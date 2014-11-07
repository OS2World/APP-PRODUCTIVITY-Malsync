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

#include <AGNet.h>
#include <AGBufferedNet.h>
#include <AGUtil.h>
#include <AGProxy.h>

#ifndef __palmos__ /**** Entire file is NOOP under PalmOS ****/
//#include <stdio.h>	(adam) temporary hack - comment out so it will build on ce

#ifndef _WIN32
#include <ctype.h> //PENDING(klobad) not needed on palm or win remove??
#include <errno.h>
#endif

typedef struct _socksStruct {
    int32 bytesread;
    int32 bytessent;
    int32 bytestosend;
    uint8 *data;
} socksStruct;

#define AGCalloc calloc
#define AGFree free
#define AGMalloc malloc

static int AGNetGetError(void);
sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc);
static sword AGNetConnect(AGNetCtx *ctx, AGSocket *soc, uint32 laddr,
                          int16 port, AGBool _block);
int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data,
                       int32 bytes, AGBool block);
int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer,
                       int32 bytes, AGBool block);
static  void AGNetSocketFree(AGNetCtx *ctx, AGSocket *soc);
static AGSocket *AGNetSocketNew(AGNetCtx *ctx);

/*---------------------------------------------------------------------------*/
ExportFunc uint32 AGNetGetHostAddr(AGNetCtx *ctx, char *name) 
{
    uint32 retval;
    struct hostent *hinfo;
    char *inname = name;
    AGBool allNum = TRUE;


    if (name == NULL)
        return 0;

    while(*inname) {
        if (!isdigit((int)*inname) && *inname != '.') {
            allNum = FALSE;
            break;
        }
        inname++;
    }

    if (allNum) {
        retval = inet_addr(name);
    } else {
        hinfo = gethostbyname(name);    
        if ( !hinfo ) 
            return 0;
        memcpy(&retval, hinfo->h_addr, hinfo->h_length);
    }
    return retval;

}
/*---------------------------------------------------------------------------*/
static void AGNetSocketFree(AGNetCtx *ctx, AGSocket *soc)
{
  
    if (!soc)
        return;
    
    (*ctx->close)(ctx, soc);
    AGFree(soc);

}
/*---------------------------------------------------------------------------*/
static AGSocket *AGNetSocketNew(AGNetCtx *ctx)
{


  AGSocket *ret;

  ret = (AGSocket *)AGCalloc(1, sizeof(AGSocket));
  ret->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if ( ret->fd  < 0 ) {
      AGFree(ret);
      ret = 0;
      return NULL;
  }
  ret->state = AG_SOCKET_NEW;

  return ret;

}
/*---------------------------------------------------------------------------*/
static sword AGNetConnect(AGNetCtx *ctx, AGSocket *soc, uint32 laddr,
                          int16 port, AGBool _block)
{
    // int ret = 0;
    int rc;
    uint32 block = 1;

    if (soc->state == AG_SOCKET_NEW) {
        soc->saddr.sin_family      = AF_INET;
        soc->saddr.sin_port        = htons(port);
        memcpy(&soc->saddr.sin_addr.s_addr, &laddr, sizeof(unsigned long));

#ifdef _WIN32
        ioctlsocket(soc->fd, FIONBIO, &block);
#else
        ioctl(soc->fd, FIONBIO, &block);
#endif
        soc->state = AG_SOCKET_CONNECTING;
    }

    do {

        rc = connect(soc->fd, 
                     (struct sockaddr *)&soc->saddr, 
                     sizeof(struct sockaddr_in));
        
        if ( rc < 0 ) {
            
            rc = AGNetGetError();
            
            if ( rc == AG_NET_EISCONN ) { 
                soc->state = AG_SOCKET_CONNECTED;
                rc = 0;
                break;
            }

            else if (rc == AG_NET_WOULDBLOCK) {

                if (_block) {
                    AGSleepMillis(30);
                } else {
                    rc = AG_NET_WOULDBLOCK;
                    break;
                }
            } else {
#ifdef _WIN32
                closesocket(soc->fd);
#else
                close(soc->fd);
#endif
                soc->state = AG_SOCKET_ERROR;
                soc->fd = -1;
                rc  = AG_NET_ERROR_NO_CONNECT;
                break;
            }
            
        } else {
            soc->state = AG_SOCKET_CONNECTED;
            rc = 0;
            break;
        }        
    } while (_block);

    return rc;
}
/*---------------------------------------------------------------------------*/
int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data,
                       int32 bytes, AGBool block)
{
    int bs=0, bts;
    int rc;
    uint8 *buf;

    do {
        
        buf = (uint8 *)data + bs;
        bts = bytes - bs;

        if (bts == 0) {
            return bs;
        }

        rc = send(soc->fd, buf, bts, 0);
        
        if ( rc < 0 ) {
            rc = AGNetGetError();
            if (rc == AG_NET_WOULDBLOCK) {
                AGSleepMillis(30);
                if (!block)
                    return rc;
            } else {
                soc->state = AG_SOCKET_ERROR;
                return rc;
            }
        } else {
            bs += rc;
        }

    } while(block);

    return bs;
}
/*---------------------------------------------------------------------------*/
int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer,
                       int32 bytes, AGBool block)
{

    uint8 *ptr;
    int32 rc;
    int32 br = 0, btr;
    

    do {

        ptr = buffer + br;
        btr = bytes - br;
    
        if (btr == 0)
            return br;
        
        rc = recv(soc->fd, ptr, btr, 0);
        
        if ( rc < 0 ) {
            rc = AGNetGetError();
            if (rc == AG_NET_WOULDBLOCK) {
                if (block) {
                    AGSleepMillis(30);
                    continue;
                } else {
                    return rc;
                }
            } else {
                soc->state = AG_SOCKET_ERROR;
                return rc;
            }
        } else {
            br += rc;
        }
        
        if (rc == 0)
            return br;
        
    } while (block);

    return br;
}
/*---------------------------------------------------------------------------*/
ExportFunc int32 AGNetGets(AGNetCtx *ctx, AGSocket *soc, uint8 *buf, int32 offset,
                           int32 n, int32 *bytesread, AGBool block)
{
    char b;
    int c = 0;
    int d;
    *bytesread = 0;

    buf += offset;

    if (n > 1)
        n -= 1;

    if (n == 0)
        return 0;

    do {
        d = AGNETRECV(ctx, soc, &b, 1, block); 
        if (d == AG_NET_WOULDBLOCK) {
            *bytesread = c;
            return AG_NET_WOULDBLOCK;
        } else if (d == 0) {
            buf[ c]='\0';
            return c;
        } else if (d < 0) {
            soc->state = AG_SOCKET_ERROR;
            return d;
        }
        buf[c++] = b;
        
    } while(c < n && b != '\n');
    
    if (n > 1)
        buf[c] = 0;
    
    return c;
}

#ifdef _WIN32
static int AGNetIsInited = 0;
#endif

/*---------------------------------------------------------------------------*/
ExportFunc 
sword AGNetInit(AGNetCtx *ctx)
{
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData; 
    int err; 
    
    if (AGNetIsInited) 
        return 0;

    wVersionRequested = MAKEWORD( 1, 1 ); 
    err = WSAStartup( wVersionRequested, &wsaData );
    
    if ( err != 0 ) {
    /* Tell the user that we couldn't find a usable */
    /* WinSock DLL.                                  */    
#ifndef _WIN32_WCE
        printf("WSAStartup failed\n");
#endif
        return -1;
    } 
    /* Confirm that the WinSock DLL supports 1.1.*/
    /* Note that if the DLL supports versions greater    */
    /* than 1.1 in addition to 1.1, it will still return */
    /* 1.1 in wVersion since that is the version we      */
    /* requested.                                        */ 

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
         HIBYTE( wsaData.wVersion ) != 1 ) {
    /* Tell the user that we couldn't find a usable */
    /* WinSock DLL.                                 */    
#ifndef _WIN32_WCE
        printf("WSAStartup failed #2 with version %d.%d\n",
            LOBYTE( wsaData.wVersion ),
            HIBYTE( wsaData.wVersion ));
#endif
        WSACleanup();
        return -1; 
    } 

    AGNetIsInited = 1;
#endif
    bzero(ctx, sizeof(AGNetCtx));
    AGNetSetIOFuncs(ctx, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    /* The WinSock DLL is acceptable. Proceed. */
    return 0;
}
/*---------------------------------------------------------------------------*/
ExportFunc 
void AGNetSetIOFuncs(AGNetCtx *ctx, AGNetSendFunc send,
                     AGNetConnectFunc connect,
                     AGNetReadFunc recv,
                     AGNetCloseFunc close,
                     AGNetSocketNewFunc socnew,
                     AGNetSocketFreeFunc socfree,
                     AGNetReadProtectedFunc recvdm)
{
    if (send)
        ctx->send = send;
    else
        ctx->send = AGBufNetSend;
    
    if (connect)
        ctx->connect = connect;
    else
        ctx->connect = AGNetConnect;
    
    if (recv)
        ctx->recv  = recv;
    else
        ctx->recv  = AGBufNetRead;
    
    if (close)
        ctx->close = close;
    else
        ctx->close = AGBufNetSocketClose;
    
    if (socnew)
        ctx->socnew = socnew;
    else
        ctx->socnew = AGBufNetSocketNew;
    
    if (socfree)
        ctx->socfree = socfree;
    else
        ctx->socfree = AGNetSocketFree;

     if (recvdm)
        ctx->recvdm = recvdm;
    else 
        ctx->recvdm = NULL; /* AGNetPilot.c will fill this out on __palmos__ */
   
}
/*---------------------------------------------------------------------------*/
ExportFunc sword AGNetClose(AGNetCtx *ctx)
{

    bzero(ctx, sizeof(AGNetCtx));   
#ifdef _WIN32
    AGNetIsInited = 0;
    return WSACleanup();
#else
    return 0;
#endif
}
/*---------------------------------------------------------------------------*/
sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc) 
{

    if (!soc || soc->fd < 0 ) 
        return 0;

#ifdef _WIN32
    closesocket(soc->fd);
#else
    close(soc->fd);
#endif
    soc->fd = -1;

    return 0;

}
/*---------------------------------------------------------------------------*/
/* Right now just looks for WOULDBLOCK error. You could see it doing 
   more in the future */
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
#ifdef _HPUX_SOURCE
   #ifndef DARWIN
      case EWOULDBLOCK:
   #endif
#endif
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
/*---------------------------------------------------------------------------*/
ExportFunc
sword  AGSocksConnect(AGNetCtx *ctx, AGSocket *soc, uint32 socksLaddr, 
                      int16 socksServerPort, char *destAddr, int16 destHostPort, 
                      AGBool block)
{
    sword rc = 0;
    uint32 laddr;
    uint8 *socksBuffer = NULL;
    int32 len;
    uint8 *buf;
    socksStruct *s;
    
    if (soc->state != AG_SOCKET_CONNECTED) {
        rc = (*ctx->connect) (ctx, soc, socksLaddr, socksServerPort, block);
        if (rc == AG_NET_WOULDBLOCK)
            return rc;
        if (rc < 0) {
            return AG_NET_SOCKS_ERROR_CONNECTTO;
        }
        if (rc == 0) {
            laddr = AGNetGetHostAddr(ctx, destAddr);
            if (!laddr)
                return AG_NET_ERROR_BAD_HOSTNAME;
            
            socksBuffer = (uint8 *)AGSocksBufCreate(laddr, destHostPort, &len);
            if (!socksBuffer)
                return AG_NET_SOCKS_ERROR;
            s = (socksStruct *)malloc(sizeof(socksStruct));
            if (!s) {
                free(socksBuffer);
                return AG_NET_SOCKS_ERROR;
            }
            s->bytestosend = len;
            s->bytessent    = 0;
            s->bytesread    = 0;
            s->data = socksBuffer;
            soc->userData = (void *)s;
            return AG_NET_WOULDBLOCK;
        }
    }

    /* If we are here and have no socks buffer, things have
       gone very wrong, bail */
    if (!soc->userData)
        return AG_NET_SOCKS_ERROR;

    s = (socksStruct *)soc->userData;
    
    if (s->bytessent == s->bytestosend) {
        int bytestoread = 8 - s->bytesread;

        buf = s->data + s->bytesread;
        rc = AGNETRECV(ctx, soc, buf, bytestoread, block); 
        if (rc == AG_NET_WOULDBLOCK)
            return rc;
        if (rc < 0) {
            free(s->data);
            free(s);
            return AG_NET_SOCKS_ERROR;
        }
        s->bytesread += rc;
        if (s->bytesread == 8) {
            rc = AGSocksGetResponse(s->data);
            free(s->data);
            free(s);
            soc->userData = NULL;
            return rc;
        }
    } else {
        int32 bytestosend = s->bytestosend - s->bytessent;
        buf = s->data + s->bytessent;
        rc = AGNETSEND(ctx, soc, buf, bytestosend, block);
        if (rc == AG_NET_WOULDBLOCK)
            return rc;
        if (rc < 0) {
            free(s->data);
            free(s);
            return AG_NET_SOCKS_ERROR;
        }
        s->bytessent += rc;
        return AG_NET_WOULDBLOCK;
    }
    return 0;    
}

#endif /* !__palmos__ */



