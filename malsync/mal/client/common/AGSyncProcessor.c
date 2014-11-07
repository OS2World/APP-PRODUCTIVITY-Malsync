/* The contents of this file are subject to the Mozilla Public License
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

#include <AGSyncProcessor.h>
#include <AGBufferReader.h>
#include <AGWriter.h> /* for the AGCompactSize() call */
#include <AGUtil.h>
#include <AGProtocol.h>
#include <AGClientProcessor.h>
#include <AGProxyDebug.h>
#include <AGBufferedNet.h>

typedef enum {
    IDLE = 0,
    GET_HOST,
    CONNECT,
    INIT_GET_HEADER,
    DO_GET_HEADER,
    GET_MAGIC,
    GET_CMD,
    GET_CMD_LEN,
    GET_CMD_DATA,
    SENDING,
    PROBLEM
} AGSyncProcessorState;

#define SLEEP_TIME 5

static void resetAGSyncProcessor(AGSyncProcessor *processor);
static void setRequestedBytes(AGSyncProcessor *processor,
                                uint32 requestedByteLen);
static void requestCompactInt(AGSyncProcessor *processor);
static uint32 getCompactInt(AGSyncProcessor *processor);
static void sendBytes(AGSyncProcessor *processor, uint8 *bytes, uint32 len);
static AGBool problemReading(AGSyncProcessor *processor, int32 retval);
static void expandRequest(AGSyncProcessor *processor, uint32 requestedByteLen);
static int16 processRead(AGSyncProcessor *processor);
static int32 processHeaderSetup(AGSyncProcessor *processor);
static int32 processHeader(AGSyncProcessor *processor);
static int16 processWrite(AGSyncProcessor *processor);
static int32 processConnect(AGSyncProcessor *processor);
static int32 processTimeout(AGSyncProcessor *processor, uint32 timeout,
                                int32 retryFailStringId);
static void interpretMagic(AGSyncProcessor *processor);
static int32 defaultSendData(void *out, AGNetCtx *ctx, AGSocket *socket,
                             uint8 *buffer, int32 bytesToSend, AGBool block);

/*---------------------------------------------------------------------------*/
ExportFunc
AGSyncProcessor *AGSyncProcessorNew(char *serverName, int16 serverPort,
                                    void *out, AGSendDataFunc sendDataFunc,
                                    char *proxyServer, int16 proxyPort,
                                    char *socksServer, int16 socksPort,
                                    AGNetCtx *netctx)
{
    AGSyncProcessor *processor;
    processor = (AGSyncProcessor *)malloc(sizeof(AGSyncProcessor));
    if(processor != NULL)
        AGSyncProcessorInit(processor, serverName, serverPort, out,
                            sendDataFunc, proxyServer, proxyPort,
                            socksServer, socksPort, netctx);
    return processor;
}
/*---------------------------------------------------------------------------*/
ExportFunc
AGSyncProcessor *AGSyncProcessorInit(AGSyncProcessor *processor,
                                     char *serverName, int16 serverPort,
                                     void *out, AGSendDataFunc sendDataFunc,
                                     char *proxyServer, int16 proxyPort,
                                     char *socksServer, int16 socksPort,
                                     AGNetCtx *netctx)
{
    memset(processor, 0, sizeof(AGSyncProcessor));
    processor->serverName = strdup(serverName);
    processor->serverPort = serverPort;
    processor->state = IDLE;

    if (proxyServer) {
        processor->proxyServer = strdup(proxyServer);
        processor->proxyPort = proxyPort;
    }
    if (socksServer) {
        processor->socksServer = strdup(socksServer);
        processor->socksPort = socksPort;
    }

    AGSyncProcessorSetSendDataFunc(processor, out, sendDataFunc);
    AGSyncProcessorSetTimeouts(processor, DEFAULT_CONNECT_TIMEOUT,
                                DEFAULT_WRITE_TIMEOUT,
                                DEFAULT_READ_TIMEOUT);
    processor->maxReadSize = MAX_READ_SIZE;
    processor->netctx = netctx;
    return processor;
}
/*---------------------------------------------------------------------------*/
ExportFunc
void AGSyncProcessorFinalize(AGSyncProcessor *processor)
{
/*PENDING(klobad) better cleanup??
 */
    AGSyncProcessorDisconnect(processor);

    if(processor->serverName != NULL) {
        free(processor->serverName);
        processor->serverName = NULL;
    }

    if(processor->socksServer) {
        free(processor->socksServer);
        processor->socksServer = NULL;
    }

    if(processor->proxyServer) {
        free(processor->proxyServer);
        processor->proxyServer = NULL;
    }

    if(processor->freeBuffer && processor->buffer != NULL) {
        AGProtectedFree((void *)processor->buffer);
        processor->buffer = NULL;
        processor->freeBuffer = FALSE;
    }

}

ExportFunc void AGSyncProcessorFree(AGSyncProcessor *processor)
{
    AGSyncProcessorFinalize(processor);
    free(processor);
}

ExportFunc void AGSyncProcessorSetTimeouts(AGSyncProcessor *processor,
                                           uint32 connectTimeoutSeconds,
                                           uint32 writeTimeoutSeconds,
                                           uint32 readTimeoutSeconds)
{
    if(connectTimeoutSeconds)
        processor->connectTimeout = connectTimeoutSeconds;
    if(writeTimeoutSeconds)
        processor->writeTimeout = writeTimeoutSeconds;
    if(readTimeoutSeconds)
        processor->readTimeout = readTimeoutSeconds;
}


ExportFunc int32 AGSyncProcessorProcess(AGSyncProcessor *processor)
{
    int32 rc = AGSYNC_IDLE;
    uint32 tmp;

    // When tracking a timeoutAt, we don't want the time
    // spent outside this function as part of the timeout,
    // so move the timeoutAt forward the amount of time
    // we spent outside this function.
    if(processor->beginUserTime != 0 && processor->timeoutAt != 0) {
        processor->timeoutAt += (AGTime() - processor->beginUserTime);
    }

    switch (processor->state) {
    case IDLE:
        rc = AGSYNC_IDLE;
        break;

    case GET_HOST:
        processor->socket = AGNETSOCKETNEW(processor->netctx);
        if(NULL == processor->socket) {
            processor->errStringId = AGMSGLookupFailedStringId;
            rc = AGSYNC_ERR;
            break;
        }
        if (processor->socksServer) {
            processor->socket->addr =
                AGNetGetHostAddr(processor->netctx,
                                 processor->socksServer);
            processor->errStringId  = AGMSGSocksDNSErrorStringId;
        } else if (processor->proxyServer) {
            processor->socket->addr =
                AGNetGetHostAddr(processor->netctx,
                                 processor->proxyServer);
            processor->errStringId  = AGMSGProxyDNSErrorStringId;
        } else {
            processor->socket->addr =
                AGNetGetHostAddr(processor->netctx,
                                 processor->serverName);
            processor->errStringId  = AGMSGLookupFailedStringId;
        }
        if(0 == processor->socket->addr) {
            rc = AGSYNC_ERR;
            break;
        } else {
            processor->errStringId  = 0;
        }
        resetAGSyncProcessor(processor);
        processor->state = CONNECT;
        rc = AGSYNC_CONTINUE;
        break;

    case CONNECT:
        rc = processConnect(processor);
        break;

    case SENDING:
        processWrite(processor);
        if(processor->state == IDLE) {
            processor->errStringId = 0;
            processor->buffer = NULL;
            processor->freeBuffer = FALSE;
            processor->buffersize = 0;
            processor->offset = 0;
            processor->requestedBytes = 0;
            processor->bytesProcessed = 0;
            processor->timeoutAt = 0;
            processor->state = IDLE;
            rc = AGSYNC_IDLE;
        } else {
            rc = AGSYNC_CONTINUE;
        }
        break;

    case INIT_GET_HEADER:
        rc = processHeaderSetup(processor);

        break;

    case DO_GET_HEADER:
        rc = processHeader(processor);
        /* OK, if we have gotten a 401 error and we havne't tried
           the realm authorization, set the stuff and do it now */
        if ((processor->errStringId == AGMSG401StringId) &&
            processor->lc &&
            !processor->lc->proxy401  && processor->lc->HTTPUseAuthentication &&
            processor->lc->HTTPUsername && processor->lc->HTTPPassword &&
            processor->lc->HTTPUseProxy) {

#if defined(_WIN32) && !defined(_WIN32_WCE)
            if (processor->cp->logConnectionInfo)
                proxyDebug(processor->cp, "\nReceived a 401, trying Basic auth\n");
#endif

            /* Set a flag to tell state machine to use basic authorization */
            processor->lc->proxy401 = 1;

            /* Close the socket */
            AGSyncProcessorDisconnect(processor);

            /* reset state machine to connect to proxy again */
            resetAGSyncProcessor(processor);
            processor->state = GET_HOST;
            rc = AGSYNC_401;
        }
        break;

    case GET_MAGIC:
        processRead(processor);
        if(processor->state == IDLE) {
            interpretMagic(processor);
            processor->state = IDLE;
            rc = AGSYNC_IDLE;
        } else {
            rc = AGSYNC_CONTINUE;
        }
        break;

    case GET_CMD:
        processRead(processor);
        if(processor->state == IDLE) {
            processor->command = getCompactInt(processor);
            if(!AGIsValidCommand(processor->command)) {
                processor->state = PROBLEM;
                processor->errStringId = AGMSGReadingFailedStringId;
                rc = AGSYNC_CONTINUE;
            } else {
                requestCompactInt(processor);
                processor->state = GET_CMD_LEN;
                rc = AGSYNC_CONTINUE;
            }
        } else {
            rc = AGSYNC_CONTINUE;
        }
        break;

    case GET_CMD_LEN:
        processRead(processor);
        if(processor->state == IDLE) {
            processor->commandLen = getCompactInt(processor);
            if(processor->commandLen > AGMAL_MAX_COMMAND_LEN) {
                processor->state = PROBLEM;
                processor->errStringId = AGMSGReadingFailedStringId;
                rc = AGSYNC_CONTINUE;
            } else {
                processor->errStringId = 0;
                setRequestedBytes(processor, processor->commandLen);
                processor->state = GET_CMD_DATA;
                rc = AGSYNC_CONTINUE;
            }
        } else {
            rc = AGSYNC_CONTINUE;
        }
        break;

    case GET_CMD_DATA:
        processRead(processor);
        if(processor->state == IDLE) {
            processor->state = IDLE;
            rc = AGSYNC_IDLE;
        } else {
            rc = AGSYNC_CONTINUE;
        }
        break;

    case PROBLEM:
        /* Whoever sets the state to PROBLEM,
            should set the errStringId properly*/
        tmp = processor->errStringId;
        // This is currently calling reserAGSyncProcessor()
        // which clears the err id - HA
        /* PENDING(klobad) better cancelling??*/
        AGSyncProcessorDisconnect(processor);
        processor->errStringId = tmp;
        rc = AGSYNC_ERR;
        break;
    }

    // If we're tracking a timeout, then we want to
    // remember when we left this loop, so we can
    // subtract the time we spend outside this loop
    // from the timeout. (Actually, we're moving the
    // timeoutAt value forward.)
    if(processor->timeoutAt != 0)
        processor->beginUserTime = AGTime();
    else
        processor->beginUserTime = 0;

    return rc;
}
/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorConnect(AGSyncProcessor *processor)
{
    resetAGSyncProcessor(processor);
    processor->state = GET_HOST;
}
/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorGetHeader(AGSyncProcessor *processor)
{
    processor->state = INIT_GET_HEADER;
}
/*---------------------------------------------------------------------------*/
ExportFunc uint8 *AGSyncProcessorGetCommandBuffer(AGSyncProcessor *processor)
{
    return (uint8 *)processor->buffer;
}
/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorGetNextCommand(AGSyncProcessor *processor)
{
    resetAGSyncProcessor(processor);
    requestCompactInt(processor);
    processor->state = GET_CMD;
}

/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorGetMagic(AGSyncProcessor *processor)
{
    resetAGSyncProcessor(processor);
    // AG_PROTOCOL_MAGIC_HIGH + AG_PROTOCOL_MAGIC_LOW
    // + AGWriteMAJORVERSION() + AGWriteMINORVERION()
    setRequestedBytes(processor, 4);
    processor->state = GET_MAGIC;
}

/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorDisconnect(AGSyncProcessor *processor)
{
    if(processor->socket != NULL)
         AGNETSOCKETFREE(processor->netctx, processor->socket);
    processor->socket = NULL;
    resetAGSyncProcessor(processor);
    processor->state = IDLE;
}
/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorSendBuffer(AGSyncProcessor *processor,
                                                uint8 *buffer, uint32 len)
{
    resetAGSyncProcessor(processor);
    sendBytes(processor, buffer, len);
    processor->state = SENDING;
}
/*---------------------------------------------------------------------------*/
ExportFunc void AGSyncProcessorSetSendDataFunc(AGSyncProcessor *processor,
                                               void *out,
                                               AGSendDataFunc sendDataFunc)
{
    processor->out = out;
    if(sendDataFunc == NULL)
        processor->sendDataFunc = defaultSendData;
    else
        processor->sendDataFunc = sendDataFunc;
}

/*---------------------------------------------------------------------------*/
static int32 defaultSendData(void *out, AGNetCtx *ctx, AGSocket *socket,
                            uint8 *buffer, int32 bytesToSend,  AGBool block)
{
    return AGNETSEND(ctx, socket, buffer, bytesToSend, block);
}
/*---------------------------------------------------------------------------*/
static int32 processTimeout(AGSyncProcessor *processor, uint32 timeoutLen,
                            int32 retryFailStringId)
{

    if(processor->timeoutAt == 0) {
        processor->timeoutAt = AGTime() + timeoutLen;
    } else {
        if(AGTime() >= processor->timeoutAt) {
            processor->errStringId = retryFailStringId;
            processor->state = PROBLEM;
        }
    }
    return AGSYNC_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static void resetAGSyncProcessor(AGSyncProcessor *processor)
{
    processor->state = IDLE;
    processor->command = 0;
    processor->commandLen = 0;
    processor->timeoutAt = 0;
    processor->errStringId = 0;
    processor->offset = 0;
    setRequestedBytes(processor, 0);
}
/*---------------------------------------------------------------------------*/
static void setRequestedBytes(AGSyncProcessor *processor,
                              uint32 requestedByteLen)
{
    /* NOTE: This does not reset the offset! (this is a good thing)*/
    processor->requestedBytes = 0;
    processor->bytesProcessed = 0;
    /* expandRequest may change state to PROBLEM*/
    expandRequest(processor, requestedByteLen);
}
/*---------------------------------------------------------------------------*/
static void requestCompactInt(AGSyncProcessor *processor)
{
    setRequestedBytes(processor, 1);
    return;
}
/*---------------------------------------------------------------------------*/
static void sendBytes(AGSyncProcessor *processor, uint8 *bytes, uint32 len)
{
    processor->errStringId = 0;
    processor->freeBuffer = FALSE;
    processor->buffer = bytes;
    processor->buffersize = len;
    processor->offset = 0;
    processor->requestedBytes = 0;
    processor->bytesProcessed = 0;
}
/*---------------------------------------------------------------------------*/
static int32 processConnect(AGSyncProcessor *processor)
{
    int32 rc = AGSYNC_IDLE;
    sword netrc = 0;

    if (processor->socksServer) {
        if (processor->proxyServer)
            netrc = AGSocksConnect(processor->netctx,
                                   processor->socket,
                                   processor->socket->addr,
                                   processor->socksPort,
                                   processor->proxyServer,
                                   processor->proxyPort,
                                   FALSE);
        else
            netrc = AGSocksConnect(processor->netctx,
                                   processor->socket,
                                   processor->socket->addr,
                                   processor->socksPort,
                                   processor->serverName,
                                   processor->serverPort,
                                   FALSE);
    } else if (processor->proxyServer) {
        netrc = AGNETCONNECT(processor->netctx,
                             processor->socket,
                             processor->socket->addr,
                             processor->proxyPort,
                             FALSE);
    } else {
        netrc = AGNETCONNECT(processor->netctx,
                             processor->socket,
                             processor->socket->addr,
                             processor->serverPort,
                             FALSE);
    }

    if(netrc == 0) {
        /* Connect() call is complete, they need to LOGON now*/
        resetAGSyncProcessor(processor);
        rc = AGSYNC_IDLE;
    } else if(netrc == AG_NET_WOULDBLOCK) {
        rc = processTimeout(processor,
                            processor->connectTimeout,
                            AGMSGConnectingFailedStringId);
        AGSleepMillis(SLEEP_TIME);
    } else {
        if (processor->socksServer) {
            switch (netrc) {
            case AG_NET_SOCKS_ERROR_CONNECTTO:
                processor->errStringId =
                    AGMSGSocksConnectFailedStringId;
                break;
            case AG_NET_SOCKS_COULDNT_CONNECT:
                processor->errStringId =
                    AGMSGSocksConnectErrorStringId;
                break;
            case AG_NET_SOCKS_BAD_ID:
                processor->errStringId =
                    AGMSGSocksIdErrorStringId;
                break;
            case AG_NET_ERROR_BAD_HOSTNAME:
                if (processor->proxyServer)
                    processor->errStringId =
                        AGMSGProxyDNSErrorStringId;
                else
                    processor->errStringId =
                        AGMSGLookupFailedStringId;
                break;
            default:
                processor->errStringId =
                    AGMSGSocksErrorStringId;
                break;
            }
        } else if (processor->proxyServer) {
            processor->errStringId = AGMSGSocksConnectFailedStringId;
        } else {
            processor->errStringId = AGMSGConnectingFailedStringId;
        }
        processor->state = PROBLEM;
        rc = AGSYNC_CONTINUE;
    }
    return rc;
}
/*---------------------------------------------------------------------------*/
static int16 processRead(AGSyncProcessor *processor)
{
    uint32 bytesToGet;
    int32 retval;

    if(processor->state == GET_CMD || processor->state == GET_CMD_LEN) {
        if(processor->requestedBytes == 1 && processor->bytesProcessed == 1) {
            uint8 *buf = (uint8 *) &processor->buffer[processor->offset
                                        - processor->bytesProcessed];
            /* If this is a one byte number, the expandRequest(0)
             is a no op. Otherwise, it expands to grow the necessary
             bytes (2 or 4)
             -1 the bytesProcessed will already be one, bytesToGet needs
             to be AGCompactInt() in size*/
            if(AGCompactLenFromBuffer(buf) > 1)
                expandRequest(processor, (AGCompactLenFromBuffer(buf) - 1));
        }
        /* The expands could have failed*/
        if(processor->state == PROBLEM) {
            return AGSYNC_ERR;
        }
    }

    bytesToGet = processor->requestedBytes - processor->bytesProcessed;
    if(processor->maxReadSize != NO_MAX_READ_SIZE
        && bytesToGet > processor->maxReadSize) {
        bytesToGet = processor->maxReadSize;
    }

    /* Have we gotten all the data bytes for this command?*/
    if(bytesToGet == 0) {
        processor->state = IDLE;
        return AGSYNC_IDLE;
    }

#ifndef __palmos__
    retval = AGNETRECV(processor->netctx, processor->socket,
                        ((uint8 *)processor->buffer) + processor->offset,
                        bytesToGet, FALSE);
#else
    retval = AGNETRECVDM(processor->netctx, processor->socket,
                                ((uint8 *)processor->buffer),
                                processor->offset, bytesToGet, FALSE);
#endif
    if(retval == AG_NET_WOULDBLOCK) {
        processTimeout(processor,
                        processor->readTimeout,
                        AGMSGReadingFailedStringId);
        AGSleepMillis(SLEEP_TIME);
    } else if(problemReading(processor, retval)) {
        processor->state = PROBLEM;
        processor->errStringId = AGMSGReadingFailedStringId;
    } else {
        processor->bytesProcessed += retval;
        processor->offset += retval;
        processor->timeoutAt = 0;
    }
    return AGSYNC_CONTINUE;
}

static int16 processWrite(AGSyncProcessor *processor)
{
    uint32 bytesToSend;
    int32 retval;

    bytesToSend = processor->buffersize - processor->bytesProcessed;

    /* Have we gotten all the data bytes for this command?*/
    if(bytesToSend == 0) {
        processor->state = IDLE;
        return AGSYNC_IDLE;
    }

    retval =
        (*processor->sendDataFunc)(processor->out, processor->netctx,
                                   processor->socket,
                                   (uint8 *)processor->buffer + processor->offset,
                                   (int32)bytesToSend,
                                   FALSE);

    if(retval == AG_NET_WOULDBLOCK) {
        processTimeout(processor,
                        processor->writeTimeout,
                        AGMSGSendingFailedStringId);
        AGSleepMillis(SLEEP_TIME);
        return AGSYNC_CONTINUE;
    } else if (problemReading(processor, retval)) {
        processor->state = PROBLEM;
        processor->errStringId = AGMSGReadingFailedStringId;
        /* Need to return AGSYNC_CONTINUE here to allow the
         PROBLEM state to process*/
        return AGSYNC_CONTINUE;
    } else {
        processor->bytesProcessed += retval;
        processor->offset += retval;
        processor->timeoutAt = 0;
    }

    /* Have we sent all the data bytes for this command?*/
    bytesToSend = processor->buffersize - processor->bytesProcessed;
    if(bytesToSend == 0) {
        processor->state = IDLE;
        return AGSYNC_IDLE;
    } else {
        /* state does not change stays - SENDING*/
    }
    return AGSYNC_CONTINUE;
}

static uint32 getCompactInt(AGSyncProcessor *processor)
{
    uint32 start = processor->offset
                        - processor->bytesProcessed;
    return AGCompactIntFromBuffer((uint8 *)(processor->buffer + start));
}

static AGBool problemReading(AGSyncProcessor *processor, int32 retval)
{
    if ( retval < 0) {
        /* Network read err */
        processor->state  = PROBLEM;
        processor->errStringId = AGMSGUnknownFailureStringId;
        return TRUE;
    } else if ( retval == 0 ) {
        /* server closed the connection */
        processor->state  = PROBLEM;
        processor->errStringId = AGMSGConnectionClosedStringId;
        return TRUE;
    }
    return FALSE;
}

static void expandRequest(AGSyncProcessor *processor, uint32 requestedByteLen)
{
    processor->requestedBytes += requestedByteLen;
    if(processor->buffer != NULL && requestedByteLen > 0
        && processor->buffersize < (processor->offset + requestedByteLen)) {

        if(requestedByteLen < 50)
             requestedByteLen = 50;

        processor->buffer= (uint8*)AGProtectedRealloc((uint8 *)processor->buffer,
                                requestedByteLen + processor->offset);
        processor->buffersize = requestedByteLen + processor->offset;
        processor->freeBuffer = TRUE;
        if(processor->buffer == NULL)  {
            processor->state  = PROBLEM;
            processor->errStringId = AGMSGUnknownFailureStringId;
            return;
        }
    }

    if(processor->buffer == NULL && requestedByteLen > 0) {
        if(requestedByteLen < 50)
             requestedByteLen = 50;
        processor->offset = 0;
        processor->buffer = (uint8*)AGProtectedMalloc(requestedByteLen);
        processor->freeBuffer = TRUE;
        processor->buffersize = requestedByteLen;
    }

    if(processor->buffer == NULL && requestedByteLen > 0)  {
        processor->state = PROBLEM;
        processor->errStringId = AGMSGUnknownFailureStringId;
    }
}
/*---------------------------------------------------------------------------*/
static int32 processHeaderSetup(AGSyncProcessor *processor)
{
    resetAGSyncProcessor(processor);
    if (processor->buffersize < 1024){
        processor->buffer= (uint8*)
            AGProtectedRealloc((void *)processor->buffer,
                1024);
        if(processor->buffer == NULL)  {
            processor->state  = PROBLEM;
            processor->errStringId = AGMSGUnknownFailureStringId;
            return AGSYNC_CONTINUE;
        }
        processor->freeBuffer = TRUE;
        processor->buffersize = 1024;
    }
    processor->state = DO_GET_HEADER;
    return AGSYNC_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 processHeader(AGSyncProcessor *processor)
{
    AGSocket *soc = processor->socket;
    int32 bytestoread, bytesread, br, rc = AGSYNC_CONTINUE;
    uint8 *buf;

    bytestoread = processor->buffersize - processor->bytesProcessed;

    if(bytestoread <= 0) {
        // Looks like we might not be talking to a real server?
        // the headers being returned from the server are bigger than
        // processor->buffersize (normally 1024 bytes?)
        // get out of dodge
        processor->state = PROBLEM;
        processor->errStringId = AGMSGReadingFailedStringId;
        return AGSYNC_CONTINUE;
    }

    buf = (unsigned char *)processor->buffer + processor->bytesProcessed;
    bytesread = AGBufNetGets(processor->netctx, soc,
                          (unsigned char *) processor->buffer,
                          processor->bytesProcessed,
                          bytestoread, &br, FALSE);

    if ( bytesread < 0 ) {
        if ( bytesread == AG_NET_WOULDBLOCK ) {
            processor->bytesProcessed += br;
            rc = processTimeout(processor,
                                processor->readTimeout,
                                AGMSGReadingFailedStringId);
            if(br > 0)
                processor->timeoutAt = 0;
            AGSleepMillis(SLEEP_TIME);
        } else {
            processor->state = PROBLEM;
            processor->errStringId = AGMSGReadingFailedStringId;
        }
    } else if (bytesread == 0) {
        processor->state  = PROBLEM;
        processor->errStringId = AGMSGUnknownFailureStringId;
    } else {
#if defined(_WIN32) && !defined(_WIN32_WCE)
        if (processor->cp->logConnectionInfo)
            proxyDebug(processor->cp, ">> %s", buf);
#endif
        buf = (unsigned char *) processor->buffer;
        /* We got a full line from the net; let's see if we need to get the
           return code from it */
        if (processor->returnCode == 0) {
            /* skip the HTTP/1.x */
            while( isascii(*buf) && !isspace(*buf))
                buf++;
            /* skip space before 200, because Palm's
                atoi doesn't like leading whitespace */
            while( isascii(*buf) && isspace(*buf))
                buf++;
            processor->returnCode = atoi((const char *)buf);
            if (processor->returnCode != 200) {
#if defined(_WIN32) && !defined(_WIN32_WCE)
            /* If we are logging the connection info, we don't want to bail
                here: we want to get the whole header */
                if (!processor->cp->logConnectionInfo)
#endif
                processor->state = PROBLEM;
                switch(processor->returnCode) {
                    case 407:
                        processor->errStringId =
                            AGMSGBadProxyAuthStringId;
                        break;
                    case 401:
                        processor->errStringId =
                            AGMSG401StringId;
                        break;
                    case 502:
                        processor->errStringId =
                            AGMSGConnectingFailedStringId;
                        break;
                    default:
                        processor->errStringId =
                            AGMSGUnknownFailureStringId;
                        break;
                }
            }

        }

        /* We have the return code, just keep reading the
            header till its done */
        processor->bytesProcessed = 0;

        if (buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n')) {

#ifdef _WIN32
            /* If we are logging the connection info, we need to
               set the problem state here because we skipped it above */
            if (processor->cp->logConnectionInfo) {
                if (processor->returnCode != 200) {
                    processor->state = PROBLEM;
                    return AGSYNC_CONTINUE;
                }
            }
#endif

            /* We are done with the header */
            resetAGSyncProcessor(processor);
            processor->state = IDLE;
            rc = AGSYNC_IDLE;
        }
    }

    return rc;

}

static void interpretMagic(AGSyncProcessor *processor)
{
    AGBufferReader r;

    AGBufferReaderInit(&r, (uint8 *)processor->buffer);
    AGReadMAGIC((AGReader *)&r, &processor->magic);
    AGReadMAJORVERSION((AGReader *)&r, &processor->majorVersion);
    AGReadMINORVERSION((AGReader *)&r, &processor->minorVersion);
    AGBufferReaderFinalize(&r);

}

