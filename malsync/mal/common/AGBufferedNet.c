/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mobile Application Link.
 *
 * The Initial Developer of the Original Code is AvantGo, Inc.
 * Portions created by AvantGo, Inc. are Copyright (C) 1997-1999
 * AvantGo, Inc. All Rights Reserved.
 *
 * Contributor(s):
 */
 
#include <AGNet.h>
#include <AGBufferedNet.h>
#include <AGProtectedMem.h>
#include <AGUtil.h>


#ifdef __unix__

    #include <errno.h>
    
#endif // __unix__

/*************************************************************************************/
#ifdef __palmos__

    #define BUFSIZE 512
    #define ALMOST_INFINITE_TIMEOUT (10 * sysTicksPerSecond)

#else

    #define BUFSIZE 4096
    
#endif

typedef struct BufferedSocket {
    AGSocket agSocket;
    uint8 *buffer;
    int32 bufferSize;
    uint8 *currentReadPos;
    int32 bytesToRead;
    int32 bytesToSend;
    int32 bytesRemaining;
    AGBool closed;
    int32 (*WriteToSocketBuffer) (struct BufferedSocket *bsoc, const uint8 *srcBuffer, 
                                    int32 bytes);
    uint8 bufferIsStorageMem;

} BufferedSocket;

/*************************************************************************************/

extern int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 bytes, 
                AGBool block);
extern int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data, 
                        int32 bytes, AGBool block);
extern sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc);

static int32 AllocateBufferedSocketBuffer(BufferedSocket *bsoc, int32 bufferSize, AGBool dynamicOnly);
static void FreeBufferedSocketBuffer(BufferedSocket *bsoc);
static int32 FlushBufferedSocketBuffer(AGNetCtx *ctx, BufferedSocket *bsoc, AGBool block);
static int32 LoadBufferedSocketBuffer(AGNetCtx *ctx, BufferedSocket *bsoc, AGBool block);
static int32 WriteToDynamicSocketBuffer(BufferedSocket *bsoc, const uint8 *srcBuffer, int32 bytes);

static int32 WriteToStorageSocketBuffer(BufferedSocket *bsoc, const uint8 *srcBuffer, int32 bytes);

#ifndef __palmos__

    static int AGNetGetError(void);
    
#endif /* __palmos__ */

/*************************************************************************************/
#ifdef __palmos__

    AGSocket *AGBufNetSocketNew(AGNetCtx *ctx)
    {
        Err error;
        BufferedSocket *bsoc;

        if(!ctx->networkLibRefNum)
            return NULL;

        bsoc = (BufferedSocket *) calloc (1, sizeof(BufferedSocket));

        if(!bsoc)
            return NULL;
        
        bsoc->agSocket.fd = NetLibSocketOpen( ctx->networkLibRefNum, netSocketAddrINET,
                                    netSocketTypeStream,
                                    htons( 6 ), // ignored...
                                    ALMOST_INFINITE_TIMEOUT, &error );
        if(error) {
            bsoc->agSocket.fd = -1;
            free(bsoc);
            return NULL;
        }

      AllocateBufferedSocketBuffer(bsoc, BUFSIZE, FALSE);
      
      return (AGSocket *) bsoc;
    }

#else    // not palm os

    AGSocket *AGBufNetSocketNew(AGNetCtx *ctx)
    {
      BufferedSocket *bsoc;

      bsoc = (BufferedSocket *) calloc (1, sizeof(BufferedSocket));
      
      if (!bsoc)
          return NULL;
          
      bsoc->agSocket.fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

      if (bsoc->agSocket.fd  < 0 ) 
      {
          free(bsoc);
          return NULL;
      }
      
      bsoc->agSocket.state = AG_SOCKET_NEW;

      AllocateBufferedSocketBuffer(bsoc, BUFSIZE, TRUE);
      
      return (AGSocket *) bsoc;
    }

#endif // __palmos__

/*************************************************************************************/

sword AGBufNetSocketClose(AGNetCtx *ctx, AGSocket *soc) 
{

    BufferedSocket *bsoc = (BufferedSocket *)soc;
    sword retval;

    retval = AGNetSocketClose(ctx, soc);

    if (bsoc->buffer)
        FreeBufferedSocketBuffer(bsoc);
    
    return retval;

}

/*************************************************************************************/

int32 AGBufNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data, int32 bytes,
                 AGBool block)
{
    BufferedSocket *bsoc = (BufferedSocket *)soc;
    int32 overflowBytes, bytesSent, bytesSentToBuffer, err;
    
    /* No buffer behaves like always */
    if (!bsoc->buffer) 
    {
        return AGNetSend(ctx, soc, data, bytes, block); 
    }

    if (bsoc->bytesRemaining == 0)
        FlushBufferedSocketBuffer(ctx, bsoc, block);

    /* Easy case, just append data to send buffer */
    if (bytes <= bsoc->bytesRemaining) 
    {
        bsoc->WriteToSocketBuffer(bsoc, data, bytes);
        
        if (bsoc->bytesRemaining == 0)
            FlushBufferedSocketBuffer(ctx, bsoc, block);
            
        return bytes;
    }

    /* Fill up the buffer */
     bytesSentToBuffer = bsoc->bytesRemaining;
    overflowBytes = bytes - bytesSentToBuffer;
    bsoc->WriteToSocketBuffer(bsoc, data, bytesSentToBuffer);
    data += bytesSentToBuffer;     // update data pointer to point to unread data
    
    err = FlushBufferedSocketBuffer(ctx, bsoc, block);

    if (err != 0)
    {
        if (bsoc->bytesRemaining > 0)
        {
            // fill in any space vacated by the flush

            if (overflowBytes > bsoc->bytesRemaining)
                overflowBytes = bsoc->bytesRemaining;

            bsoc->WriteToSocketBuffer(bsoc, data, overflowBytes);
            bytesSentToBuffer += overflowBytes;
        }
        
        if (bytesSentToBuffer == 0)
            bytesSentToBuffer = err;
            
        return bytesSentToBuffer;
    }
    
    if (overflowBytes <= bsoc->bufferSize)    // overflow can be contained in buffer
    {
        bsoc->WriteToSocketBuffer(bsoc, data, overflowBytes);

        if (bsoc->bytesRemaining == 0)
            FlushBufferedSocketBuffer(ctx, bsoc, block);

        return bytes;
    }
    else
    {
        bytesSent = AGNetSend(ctx, soc, data, bytes - bytesSentToBuffer, block);
        
        if (bytesSent < 0)
        {
            if (bytesSentToBuffer == 0)
                return bytesSent;
            else
                return bytesSentToBuffer;
        }

        return bytesSentToBuffer + bytesSent;
    }
}

/*************************************************************************************/

int32 AGBufNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 bytes,
                 AGBool block)
{
    BufferedSocket *bsoc = (BufferedSocket *)soc;
    int32 bytesRead, bytesReadFromBuffer, overflowBytes, err;

    /* No buffer behaves like always */
    if (!bsoc->buffer) {
        return AGNetRead(ctx, soc, buffer, bytes, block); 
    }

    if(!bsoc->currentReadPos) {
        
        /* If we still need to send some stuff do it now */
        if (bsoc->bytesToSend) 
        {
            err = FlushBufferedSocketBuffer(ctx, bsoc, block);
        
            if (err != 0)
            {
                if (err > 0)
                    return AG_NET_WOULDBLOCK;
                else
                    return err;
            }    
        }

        bsoc->bytesRemaining = 0;
    }

    /* If no more bytes and closed, return 0 */
    if (!bsoc->bytesRemaining && bsoc->closed)
        return 0;

    if (bsoc->bytesRemaining == 0)
    {
        bytesRead = LoadBufferedSocketBuffer(ctx, bsoc, block);
        
        if (bytesRead <= 0)
            return bytesRead;
    }
            
    if (bsoc->bytesRemaining >= bytes)
    {
        memcpy(buffer, bsoc->currentReadPos, bytes);
        bsoc->bytesRemaining -= bytes;
        bsoc->currentReadPos += bytes;
        
        if (bsoc->bytesRemaining == 0)
            LoadBufferedSocketBuffer(ctx, bsoc, block);
            
        return bytes;
    }
    else
    {
        bytesReadFromBuffer = bsoc->bytesRemaining;
        memcpy(buffer, bsoc->currentReadPos, bytesReadFromBuffer);
        bsoc->bytesRemaining = bsoc->bytesToRead = 0;
        bsoc->currentReadPos = bsoc->buffer;
        buffer += bytesReadFromBuffer;
        overflowBytes = bytes - bytesReadFromBuffer;
        
        if (overflowBytes <= bsoc->bufferSize)
        {
            bytesRead = LoadBufferedSocketBuffer(ctx, bsoc, block);
            
            if (bytesRead > 0)
            {
                if (overflowBytes > bytesRead)
                    overflowBytes = bytesRead;
                    
                memcpy(buffer, bsoc->currentReadPos, overflowBytes);
                bsoc->bytesRemaining -= overflowBytes;
                bsoc->currentReadPos += overflowBytes;
                
                if (bsoc->bytesRemaining == 0)
                    LoadBufferedSocketBuffer(ctx, bsoc, block);

                return bytesReadFromBuffer + overflowBytes;
            }
            else
            {
                return bytesReadFromBuffer;
            }
        }
        else
        {
            bytesRead = AGNetRead(ctx, soc, buffer, overflowBytes, block);
            
            if (bytesRead > 0)
            {
                LoadBufferedSocketBuffer(ctx, bsoc, block);
                return bytesRead + bytesReadFromBuffer;
            }
            else
            {
                if (bytesRead == 0)
                    bsoc->closed = TRUE;
                
                if (bytesReadFromBuffer > 0)
                {
                    return bytesReadFromBuffer;
                }
                else
                {
                    return bytesRead;
                }
            }
        }
    }
}

/*************************************************************************************/
#ifdef __palmos__

int32 AGBufNetReadProtected(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 offset, int32 bytes,
                 AGBool block)
{
    BufferedSocket *bsoc = (BufferedSocket *)soc;
    int32 bytesRead, bytesReadFromBuffer, overflowBytes, err;

    /* No buffer behaves like always */
    if (!bsoc->buffer) {
        return AGNetReadProtected(ctx, soc, buffer, offset, bytes, block); 
    }

    if(!bsoc->currentReadPos) {
        /* If we still need to send some stuff do it now */
        if (bsoc->bytesToSend) 
        {
            err = FlushBufferedSocketBuffer(ctx, bsoc, block);
        
            if (err != 0)
            {
                if (err > 0)
                    return AG_NET_WOULDBLOCK;
                else
                    return err;
            }    
        }            
        bsoc->bytesRemaining = 0;
    }
    
    /* If no more bytes and closed, return 0 */
    if (!bsoc->bytesRemaining && bsoc->closed)
        return 0;

    if (bsoc->bytesRemaining == 0)
    {
        bytesRead = LoadBufferedSocketBuffer(ctx, bsoc, block);
        
        if (bytesRead <= 0)
            return bytesRead;
    }
            
    if (bsoc->bytesRemaining >= bytes)
    {
        AGProtectedWrite(buffer, offset, bsoc->currentReadPos, bytes);
        bsoc->bytesRemaining -= bytes;
        bsoc->currentReadPos += bytes;
        
        if (bsoc->bytesRemaining == 0)
            LoadBufferedSocketBuffer(ctx, bsoc, block);
            
        return bytes;
    }
    else
    {
        bytesReadFromBuffer = bsoc->bytesRemaining;
        AGProtectedWrite(buffer, offset, bsoc->currentReadPos, bytesReadFromBuffer);
        bsoc->bytesRemaining = bsoc->bytesToRead = 0;
        bsoc->currentReadPos = bsoc->buffer;
        offset += bytesReadFromBuffer;
        overflowBytes = bytes - bytesReadFromBuffer;
        
        if (overflowBytes <= bsoc->bufferSize)
        {
            bytesRead = LoadBufferedSocketBuffer(ctx, bsoc, block);
            
            if (bytesRead > 0)
            {
                if (overflowBytes > bytesRead)
                    overflowBytes = bytesRead;
                    
                AGProtectedWrite(buffer, offset, bsoc->currentReadPos, overflowBytes);
                bsoc->bytesRemaining -= overflowBytes;
                bsoc->currentReadPos += overflowBytes;
                
                if (bsoc->bytesRemaining == 0)
                    LoadBufferedSocketBuffer(ctx, bsoc, block);

                return bytesReadFromBuffer + overflowBytes;
            }
            else
            {
                return bytesReadFromBuffer;
            }
        }
        else
        {
            bytesRead = AGNetReadProtected(ctx, soc, buffer, offset, overflowBytes, block);
            
            if (bytesRead > 0)
            {
                LoadBufferedSocketBuffer(ctx, bsoc, block);
                return bytesRead + bytesReadFromBuffer;
            }
            else
            {
                if (bytesRead == 0)
                    bsoc->closed = TRUE;
                
                if (bytesReadFromBuffer > 0)
                {
                    return bytesReadFromBuffer;
                }
                else
                {
                    return bytesRead;
                }
            }
        }
    }
}

#endif // __palmos__
/*************************************************************************************/

int32 AGBufNetGets(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 offset,
                int32 bytes, int32 *bytesRead, AGBool block)
{                
    BufferedSocket *bsoc = (BufferedSocket *)soc;
    int32 bytesReadIntoBuffer, bufferBytesInString, strLen, maxChars, err;
    uint8 *currentChar, endOfLine;

    /* No buffer behaves like always */
    if (!bsoc->buffer) {
        return AGNetGets(ctx, soc, buffer, offset, bytes, bytesRead, block); 
    }

    if (bytes <= 0)
    {
        *bytesRead = 0;
        return 0;
    }

    if(!bsoc->currentReadPos) {
        /* If we still need to send some stuff do it now */
        if (bsoc->bytesToSend) 
        {
            err = FlushBufferedSocketBuffer(ctx, bsoc, block);
        
            if (err != 0)
            {
                *bytesRead = 0;
        
                if (err > 0)
                    return AG_NET_WOULDBLOCK;
                else
                    return err;
            }    
        }            
        bsoc->bytesRemaining = 0;
    }
    
    /* If no more bytes and closed, return 0 */
    if (!bsoc->bytesRemaining && bsoc->closed)
        return 0;

    if (bsoc->bytesRemaining <= 0)
    {
        bytesReadIntoBuffer = LoadBufferedSocketBuffer(ctx, bsoc, block);
        
        if (bytesReadIntoBuffer <= 0)
        {
            *bytesRead = 0;
            return bytesReadIntoBuffer;
        }
    }
    
    endOfLine = FALSE;
    maxChars = bytes - 1;
    strLen = 0;
            
    if (bsoc->bytesRemaining >= maxChars)
    {
        currentChar = bsoc->currentReadPos;
    
        while ((strLen < maxChars) && (!endOfLine))
        {
            if (*currentChar++ == '\n')
                endOfLine = TRUE;
                
            strLen++;
        }
    
        AGProtectedWrite(buffer, offset, bsoc->currentReadPos, strLen);
        bsoc->bytesRemaining -= strLen;
        bsoc->currentReadPos += strLen;
        
        AGProtectedZero(buffer, offset + strLen, 1);
        
        if (bsoc->bytesRemaining == 0)
            LoadBufferedSocketBuffer(ctx, bsoc, block);
        
        *bytesRead = strLen;     
        return strLen;
    }
    else
    {
        bytesReadIntoBuffer = 1;    // dummy value to get loop started
        bufferBytesInString = 0;
        currentChar = bsoc->currentReadPos;
        
        while (((strLen + bufferBytesInString) < maxChars) && (!endOfLine) &&
                (bytesReadIntoBuffer > 0))
        {
            if (bufferBytesInString == bsoc->bytesRemaining)
            {
                if (bufferBytesInString > 0)
                {
                    AGProtectedWrite(buffer, offset + strLen, bsoc->currentReadPos, bufferBytesInString);
                    strLen += bufferBytesInString;
                    bsoc->bytesRemaining -= bufferBytesInString;
                    bsoc->currentReadPos += bufferBytesInString;
                    bufferBytesInString = 0;
                }
            
                bytesReadIntoBuffer = LoadBufferedSocketBuffer(ctx, bsoc, block);
                currentChar = bsoc->currentReadPos;
            }

            if (bsoc->bytesRemaining > 0)
            {
                if (*currentChar++ == '\n')
                    endOfLine = TRUE;

                bufferBytesInString++;
            }
        }

        if (bufferBytesInString > 0)
        {
            AGProtectedWrite(buffer, offset + strLen, bsoc->currentReadPos, bufferBytesInString);
            strLen += bufferBytesInString;
            bsoc->bytesRemaining -= bufferBytesInString;
            bsoc->currentReadPos += bufferBytesInString;
            bufferBytesInString = 0;
        }
        
        if ((bsoc->bytesRemaining <= 0) && (bytesReadIntoBuffer > 0))
            LoadBufferedSocketBuffer(ctx, bsoc, block);
    
        if (strLen > 0)
            AGProtectedZero(buffer, offset + strLen, 1);
        
        *bytesRead = strLen;
        
        if ((strLen < maxChars) && (!endOfLine) && (bytesReadIntoBuffer <= 0))
            return bytesReadIntoBuffer;    // if < 1, value is error code
        else
            return strLen;
    }
}

/*************************************************************************************/

static int32 AllocateBufferedSocketBuffer(BufferedSocket *bsoc, int32 bufferSize, AGBool dynamicOnly)
{
    if (bsoc == NULL)
        return -1;

  /* if bufferSize it set to zero, works like ordinary socket */
    if (bufferSize) 
    {
        bsoc->buffer = (uint8 *) malloc (bufferSize);    // Try allocating buffer in dynamic mem

        if (bsoc->buffer != NULL)    // success
        {    
            bsoc->WriteToSocketBuffer = WriteToDynamicSocketBuffer;
            bsoc->bufferIsStorageMem = FALSE;
        }
        else                    // fail - try allocating in storage mem
        {
            bsoc->buffer = (uint8 *) AGProtectedMalloc (bufferSize);
            
            if (bsoc->buffer != NULL)
            {
                bsoc->WriteToSocketBuffer = WriteToStorageSocketBuffer;
                bsoc->bufferIsStorageMem = TRUE;
            }
        }
        
        if (bsoc->buffer != NULL)
        {
            bsoc->bufferSize = bufferSize;
            bsoc->bytesRemaining = bsoc->bufferSize;
            bsoc->bytesToSend = 0;
            bsoc->bytesToRead = 0;
            bsoc->closed = FALSE;
        }
    }
    
    if (bsoc->buffer == NULL)
        return -1;
        
    return 0;
}

/*************************************************************************************/

static void FreeBufferedSocketBuffer(BufferedSocket *bsoc)
{
    if ((bsoc == NULL) || (bsoc->buffer == NULL))
        return;

    if (bsoc->bufferIsStorageMem)
    {
        AGProtectedFree(bsoc->buffer);
    }
    else
    {
        free(bsoc->buffer);
    }
    
    bsoc->buffer = NULL;
}

/*************************************************************************************/

static int32 FlushBufferedSocketBuffer(AGNetCtx *ctx, BufferedSocket *bsoc, AGBool block)
{
    int32 bytesSent = 0, bytesLeft;

    if ((bsoc == NULL) || (bsoc->buffer == NULL) || (bsoc->bytesToSend == 0))
        return 0;
        
    bytesSent = AGNetSend(ctx, &(bsoc->agSocket), bsoc->buffer, bsoc->bytesToSend, block);    

    if (bytesSent == bsoc->bytesToSend)
    {
        bsoc->bytesToSend = 0;
        bsoc->bytesRemaining = bsoc->bufferSize;
        
        return 0;
    }
    else if (bytesSent > 0)
    {
        bytesLeft = bsoc->bytesToSend - bytesSent;
        
        bsoc->bytesToSend = 0;
        bsoc->bytesRemaining = bsoc->bufferSize;
        
        bsoc->WriteToSocketBuffer(bsoc, &(bsoc->buffer[bytesSent]), bytesLeft);
        
        return AG_NET_WOULDBLOCK;
    }
    
    return bytesSent;
}

/*************************************************************************************/

static int32 LoadBufferedSocketBuffer(AGNetCtx *ctx, BufferedSocket *bsoc, AGBool block)
{
    int32 bytesRead = 0;

    if ((bsoc == NULL) || (bsoc->buffer == NULL) || (bsoc->bytesRemaining > 0))
        return 0;
    
    bsoc->currentReadPos = bsoc->buffer;


#ifdef __palmos__
    
    if (bsoc->bufferIsStorageMem)
    {
        bytesRead = AGNetReadProtected(ctx, &(bsoc->agSocket), bsoc->buffer, 0, 
                                        bsoc->bufferSize, block);
    }
    else
    {    
        bytesRead = AGNetRead(ctx, &(bsoc->agSocket), bsoc->buffer, bsoc->bufferSize, block);
    }    

#else // not palm os

        bytesRead = AGNetRead(ctx, &(bsoc->agSocket), bsoc->buffer, bsoc->bufferSize, block);

#endif // __palmos__


    if (bytesRead > 0)
    {
        bsoc->bytesRemaining = bsoc->bytesToRead = bytesRead;
    }
    else 
    {
        bsoc->bytesRemaining = bsoc->bytesToRead = 0;
    
        if (bytesRead == 0)
            bsoc->closed = TRUE;
    }
    
    return bytesRead;
}

/*************************************************************************************/

static int32 WriteToDynamicSocketBuffer(BufferedSocket *bsoc, const uint8 *srcBuffer, int32 bytes)
{
    if ((bsoc == NULL) || (bsoc->bytesRemaining < bytes))
        return -1;
        
    memmove(&(bsoc->buffer[bsoc->bytesToSend]), srcBuffer, bytes);
    
    bsoc->bytesToSend += bytes;
    bsoc->bytesRemaining -= bytes;
    
    return 0;
}
        
/*************************************************************************************/

static int32 WriteToStorageSocketBuffer(BufferedSocket *bsoc, const uint8 *srcBuffer, int32 bytes)
{
    int32 err;

    if ((bsoc == NULL) || (bsoc->bytesRemaining < bytes))
        return -1;
        
    err = AGProtectedWrite(bsoc->buffer, bsoc->bytesToSend, (void *) srcBuffer, bytes);
    
    if (err == 0)
    {
        bsoc->bytesToSend += bytes;
        bsoc->bytesRemaining -= bytes;
    }
    
    return err;
}

/*************************************************************************************/
#ifndef __palmos__

static int AGNetGetError()
{
  int err = 0;

#ifdef _WIN32
  err = WSAGetLastError();
  switch(err) {
    case WSAEWOULDBLOCK:
    case 10022:
    case WSAEALREADY:
      return AG_NET_WOULDBLOCK;
    case WSAEISCONN:
      return AG_NET_EISCONN;
    default:
      return AG_NET_ERROR;
  }

#else

  err = errno;
  switch (err) {
    case EINPROGRESS:
      return AG_NET_WOULDBLOCK;
    case EAGAIN:
      return AG_NET_WOULDBLOCK;
    case EALREADY:
      return AG_NET_WOULDBLOCK;
    case EISCONN:
      return AG_NET_EISCONN;
    default:
      return AG_NET_ERROR;
  }
#endif
}

#endif /* !__palmos__ */



