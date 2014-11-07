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

#if (defined(macintosh) && !defined(__palmos__))

#include <AGNet.h>
#include <AGUtil.h>
#include <AGProxy.h>

#include <stdio.h>

#define AGCalloc calloc
#define AGFree free
#define AGMalloc malloc

static int AGNetGetError(void);
static sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc);
static sword AGNetConnect(AGNetCtx *ctx, AGSocket *soc, uint32 laddr,
                          int16 port, AGBool _block);
static int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data,
                       int32 bytes, AGBool block);
static int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer,
                       int32 bytes, AGBool block);
static  void AGNetSocketFree(AGNetCtx *ctx, AGSocket *soc);
static AGSocket *AGNetSocketNew(AGNetCtx *ctx);

/*---------------------------------------------------------------------------*/
ExportFunc uint32 AGNetGetHostAddr(AGNetCtx *ctx, char *name) 
{
    OSStatus err;
    InetHostInfo response;
    
    if (NULL == ctx || NULL == name)
        return 0;

    bzero(&response, sizeof(InetHostInfo));

    err = OTInetStringToAddress(ctx->inet_services, name, &response);
    
    if (err == noErr)
        return response.addrs[0];

    return 0;
}

/*---------------------------------------------------------------------------*/
static void AGNetSocketFree(AGNetCtx *ctx, AGSocket *soc)
{
    if (NULL == ctx || NULL == soc)
        return;
    
    (*ctx->close)(ctx, soc);
    AGFree(soc);
}

/*---------------------------------------------------------------------------*/
static AGSocket *AGNetSocketNew(AGNetCtx *ctx)
{
    AGSocket *soc;
    OSStatus err = noErr;    

    /* Note that in this implementation we don't actually need the
    AGNetCtx pointer. Left in for source-code compatibility. */
    
    soc = (AGSocket *)malloc(sizeof(AGSocket));
    if (NULL == soc)
        return NULL;
    bzero(soc, sizeof(AGSocket));

    soc->ep = OTOpenEndpoint(OTCreateConfiguration(kTCPName), 0, NULL, &err);
    if (noErr == err) {
        err = OTBind(soc->ep, NULL, NULL);
        if (noErr == err) {
            /* Success.  Return newly created socket. */
            soc->bound = TRUE;
            return soc;
        }
        OTCloseProvider(soc->ep);
    }
    
    /* Didn't successfully bind, so return error. */
    free(soc);
    return NULL;
}

/*---------------------------------------------------------------------------*/
static int32 AGNetMacMapError(int32 err)
{
    if (err >= 0)
        return err;
        
    /* Map Macintosh Open Transport error codes to platform-independent
    codes. */
    switch (err) {
        case kOTNoDataErr:
        case kOTFlowErr:
        case kOTStateChangeErr:
        case kOTLookErr:
            return AG_NET_WOULDBLOCK;
            break;
        case kEISCONNErr:
            return AG_NET_EISCONN;
        default:
            return AG_NET_ERROR;
    }
}

/*---------------------------------------------------------------------------*/
static sword AGNetConnect(AGNetCtx *ctx, AGSocket *soc, uint32 laddr,
                          int16 port, AGBool _block)
{
    InetAddress hostInetAddress;
    TCall sndCall;
    
    if (NULL == ctx || NULL == soc)
        return AG_NET_ERROR;

    sndCall.addr.buf = (uint8*)&hostInetAddress;
    sndCall.addr.len = sizeof(InetAddress);
    OTInitInetAddress(&hostInetAddress, port, (InetHost)laddr);
    sndCall.opt.buf = NULL;
    sndCall.opt.len = 0;
    sndCall.udata.buf = NULL;
    sndCall.udata.len = 0;
    sndCall.sequence = 0;

    return AGNetMacMapError(OTConnect(soc->ep, &sndCall, NULL));
}

/*---------------------------------------------------------------------------*/
static int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data,
                       int32 bytes, AGBool block)
{
    if (NULL == ctx || NULL == soc || NULL == data)
        return AG_NET_ERROR;
        
    return AGNetMacMapError(OTSnd(soc->ep, (void *)data, bytes, 0));
}

/*---------------------------------------------------------------------------*/
static int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer,
                       int32 bytes, AGBool block)
{
    OTFlags junkFlags = 0;

    if (NULL == ctx || NULL == soc || NULL == buffer)
        return AG_NET_ERROR;
        
    return AGNetMacMapError(OTRcv(soc->ep,
                                (void *) buffer,
                                bytes,
                                &junkFlags));
}

/*---------------------------------------------------------------------------*/
ExportFunc int32 AGNetGets(AGNetCtx *ctx, AGSocket *soc, uint8 *buf, int32 offset,
                           int32 n, int32 *bytesread, AGBool block)
{
    uint8 b;
    int c = 0;
    int d;
    *bytesread = 0;

    buf += offset;

    if (NULL == ctx || NULL == soc || NULL == buf)
        return AG_NET_ERROR;
        
    if (n > 1)
        n -= 1;

    if (n == 0)
        return 0;

    do {
        d = AGNETRECV(ctx, soc, &b, 1, block); 
        if (d == AG_NET_WOULDBLOCK) {
            *bytesread = c;
            return AG_NET_WOULDBLOCK;
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

static int AGNetIsInited = 0;

/*---------------------------------------------------------------------------*/
ExportFunc 
sword AGNetInit(AGNetCtx *ctx)
{
    OSStatus rc = 0;
    
    if (NULL == ctx)
        return AG_NET_ERROR;
        
    rc = InitOpenTransport();
    if (noErr == rc) {
        AGNetIsInited = 1;
        bzero(ctx, sizeof(AGNetCtx));
        ctx->inet_services =
            OTOpenInternetServices(kDefaultInternetServicesPath,
                0,
                &rc);
        AGNetSetIOFuncs(ctx, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
        
    return rc;
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
        ctx->send = AGNetSend;
    
    if (connect)
        ctx->connect = connect;
    else
        ctx->connect = AGNetConnect;
    
    if (recv)
        ctx->recv  = recv;
    else
        ctx->recv  = AGNetRead;
    
    if (close)
        ctx->close = close;
    else
        ctx->close = AGNetSocketClose;
    
    if (socnew)
        ctx->socnew = socnew;
    else
        ctx->socnew = AGNetSocketNew;
    
    if (close)
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
    OSStatus rc = 0;

    if (NULL == ctx)
        return AG_NET_ERROR;
        
    if (AGNetIsInited) {
        OTCloseProvider(ctx->inet_services);
        CloseOpenTransport();
    }
    return rc;
}

/*---------------------------------------------------------------------------*/
static sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc) 
{
    /* Note that in this implementation we don't actually need the
    AGNetCtx pointer. Left in for source-code compatibility. */
    
    if (NULL != soc) {
        if (soc->bound)
            OTUnbind(soc->ep);
        if (kOTInvalidEndpointRef != soc->ep)
            OTCloseProvider(soc->ep);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Right now just looks for WOULDBLOCK error. You could see it doing 
   more in the future */
static int AGNetGetError()
{
    return AG_NET_ERROR;
}

typedef struct _socksStruct {
    int32 bytesread;
    int32 bytessent;
    int32 bytestosend;
    uint8 *data;
} socksStruct;

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
    
    if (NULL == ctx || NULL == soc || NULL == destAddr)
        return AG_NET_ERROR;
        
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
            soc->userData = (uint8 *)s;
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
            rc = AGSocksGetResponse((char*)s->data);
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

#endif /* (defined(macintosh) && !defined(__palmos__)) */
