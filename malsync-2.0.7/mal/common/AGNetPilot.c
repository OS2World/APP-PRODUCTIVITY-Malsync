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

#ifdef __palmos__

#include <AGTypes.h>
#include <AGNet.h>
#include <AGBufferedNet.h>
#include <AGProxy.h>
#include <AGUtil.h>
#include <AGProtectedMem.h>

#define MAX_CONNECT_RETRIES 15

static void handleNetworkErr(Err err);
static Boolean pilotSupportsNetwork(void);

int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data, 
                        int32 bytes, AGBool block);
sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc);
static sword AGNetConnect(AGNetCtx *ctx, AGSocket *soc, uint32 laddr, short port,  
                              AGBool block);
int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 bytes, 
                         AGBool block);
static  void AGNetSocketFree(AGNetCtx *ctx, AGSocket *soc);
static AGSocket *AGNetSocketNew(AGNetCtx *ctx);


#define MAXTIMEOUT  (3 * sysTicksPerSecond)
#define ALMOST_INFINITE_TIMEOUT (10 * sysTicksPerSecond)

/*---------------------------------------------------------------------------*/
ExportFunc sword AGNetInit(AGNetCtx *ctx)
{
    
    Word err;
    Word ifErrs;
    Boolean allInterfacesUp;

    bzero(ctx, sizeof(AGNetCtx));

    // Get a refernce to the net library
    // AppNetRefnum will be zero if the pilotSupportsNetwork() != true
    SysLibFind("Net.lib", &ctx->networkLibRefNum);

    if(!ctx->networkLibRefNum)
        return 1;

    if(!pilotSupportsNetwork())
        return 1;

    // Connect
    err = NetLibOpen(ctx->networkLibRefNum, &ifErrs);
    
    // If we are already connected it is not a bad thing.
    if (err != netErrAlreadyOpen) {
        // This is a bad thing
        if (err == netErrOutOfMemory) {
            return 1;
        }        
        if (err || ifErrs) {
            NetLibClose(ctx->networkLibRefNum, true);
            handleNetworkErr(err);
            return 1;
        }
    }
    
    err = ifErrs = 0;
    err = NetLibConnectionRefresh (ctx->networkLibRefNum,
                                   true,
                                   &allInterfacesUp,
                                   &ifErrs);
    
    if (err || ifErrs) {
        NetLibClose(ctx->networkLibRefNum, true);
        handleNetworkErr(err);
        return 1;       
    }
    
    AGNetSetIOFuncs(ctx, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
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
    
    if (close)
        ctx->socfree = socfree;
    else
        ctx->socfree = AGNetSocketFree;

    if (recvdm)
        ctx->recvdm = recvdm;
    else
        ctx->recvdm = AGBufNetReadProtected;

}

/*---------------------------------------------------------------------------*/
ExportFunc sword AGNetClose(AGNetCtx *ctx)
{
    if(!ctx->networkLibRefNum)
        return 0;

    if(!pilotSupportsNetwork())
        return 0;

    NetLibClose(ctx->networkLibRefNum, false);
    return 0;
}

/*---------------------------------------------------------------------------*/
ExportFunc uint32 AGNetGetHostAddr(AGNetCtx *ctx, char *name)
{
    Err error;
    uint32 ret = 0;
    NetHostInfoPtr hinfo;
    NetHostInfoBufType HostInfoBuffer;
    NetIPAddr addr;
    
    if(!ctx->networkLibRefNum)
        return 0;

    hinfo = NetLibGetHostByName( ctx->networkLibRefNum,
                                 name, &HostInfoBuffer,
                                 ALMOST_INFINITE_TIMEOUT,
                                 &error );
    
    if(!error) {
        if ( hinfo )
            MemMove((void *)&ret, (void *)hinfo->addrListP[0], hinfo->addrLen); 
        return ret;
    }

    //So NetLibGetHostByName failed. Try "name" as an ip address.
    addr = NetLibAddrAToIN(ctx->networkLibRefNum, name);
    if (addr != -1) {
        MemMove((void *)&ret, &addr, sizeof(addr));     
        return ret;
    }
    
    return 0;
}
/*---------------------------------------------------------------------------*/
int32 AGNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data, 
                        int32 bytes, AGBool block)
{
    
    uint8 *buffer, *buf;
    int32 bytesToSend;
    int32 retval=0, bytesSent=0;
    Err error;
    buf = (uint8 *)data;

    do {
        
        buffer      = buf + bytesSent;
        bytesToSend = bytes - bytesSent;
        if ( bytesToSend <= 0 )
            break;
        
        retval = NetLibSend( ctx->networkLibRefNum, soc->fd, buffer,
                             bytesToSend, netIOFlagDontRoute,  // flags
                             &soc->saddr, sizeof(NetSocketAddrINType),
                             MAXTIMEOUT, &error );
        
        
        if ( retval > 0 )
            bytesSent += retval;
        
        if (retval < 0) {
            
            if ((error == netErrWouldBlock) || (error == netErrTimeout)) {
                if (block)
                    continue;
                else
                    return AG_NET_WOULDBLOCK;
            } else {
                handleNetworkErr(error);
                return -1;
            }
        } 

    } while (block && (bytesSent < bytes));

    return bytesSent;
}

/*---------------------------------------------------------------------------*/
sword AGNetSocketClose(AGNetCtx *ctx, AGSocket *soc)
{
    
    Err error;
    sword retval = 0;
    
    if(!soc || soc->fd < 0)
        return -1;
    
    retval = NetLibSocketClose( ctx->networkLibRefNum, soc->fd, 
                                ALMOST_INFINITE_TIMEOUT, &error );
    
    soc->fd = -1;


    if(error)
        return -1;
    
    return retval;
    
}    
/*---------------------------------------------------------------------------*/
static AGSocket *AGNetSocketNew(AGNetCtx *ctx)
{
    Err error;
    AGSocket *soc = malloc(sizeof(AGSocket));

    if(!soc)
        return NULL;

    if(!ctx->networkLibRefNum)
        return NULL;

    bzero(soc, sizeof(AGSocket));
    
    soc->fd = NetLibSocketOpen( ctx->networkLibRefNum, netSocketAddrINET,
                                netSocketTypeStream,
                                htons( 6 ), // ignored...
                                ALMOST_INFINITE_TIMEOUT, &error );
    if(error) {
        soc->fd = -1;
        free(soc);
        return NULL;
    }

    return soc;

}
/*---------------------------------------------------------------------------*/
static void AGNetSocketFree(AGNetCtx *ctx, AGSocket *soc)
{
    if (!soc)
        return;
    AGNETSOCKETCLOSE(ctx, soc);
    free(soc);

}
/*---------------------------------------------------------------------------*/
static sword AGNetConnect(AGNetCtx *ctx, AGSocket *soc, uint32 laddr, short port,  
                              AGBool block)
{
    
    Err error;
    Err retval;
    Err tmpErr;
    sword retries = 0;
    NetSocketLingerType linger;
    sword   result;
    
    if ( soc->fd < 0 ) {
        return 1;
    }
    
    soc->saddr.family = AF_INET;
    soc->saddr.port   = htons(port);
    soc->saddr.addr   = laddr;

    linger.onOff = 1;
    linger.time = 0;
    result = NetLibSocketOptionSet(ctx->networkLibRefNum, soc->fd, 
                                   netSocketOptLevelSocket,
                                   netSocketOptSockLinger,
                                   &linger, sizeof(NetSocketLingerType),
                                   ALMOST_INFINITE_TIMEOUT, &error);

	if(error) {
		/* Should we bail here ? */
	}
		
    do {
        
        /* This connect will cause a hard reset if you timeout then do not
           re try the connect in a reasonable time so I am going to up the
           time out on the connect....
        */
        retval = NetLibSocketConnect( ctx->networkLibRefNum, soc->fd, 
                                      (NetSocketAddrType *)&soc->saddr,
                                      sizeof(NetSocketAddrINType), 
                                      MAXTIMEOUT * 3,
                                      &error );
        retries++;
        
    } while ( retries < MAX_CONNECT_RETRIES && retval != 0 
              && ((error == netErrSocketBusy) || (error == netErrTimeout)) );
				
    /* The mal server is not responding to us, we need to shut down our
       connection */
    if(error) {
        tmpErr = 0;
        if(soc->fd > 0) {
            NetLibSocketClose( ctx->networkLibRefNum, soc->fd, 
                               ALMOST_INFINITE_TIMEOUT, &tmpErr );
            soc->fd = -1;
        }
        return 1;
    }

    return 0;
}
/*---------------------------------------------------------------------------*/
ExportFunc int32 AGNetReadProtected(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 offset,
                         int32 bytes, AGBool block)
{

    Err error = 0;
    int32 rc = 1;
    int32 bytesread = 0;
    int32 bytestoread = bytes;
    int32 timeout;
 
    timeout = MAXTIMEOUT;
    
    do {
        rc = NetLibDmReceive( ctx->networkLibRefNum, soc->fd, 
                              buffer - sizeof(AGProtectedHeader), 
                              offset + bytesread + sizeof(AGProtectedHeader), 
                              bytestoread,
                              0, NULL, NULL, timeout,
                              &error);
        if ( rc > 0 ) {
            bytesread += rc;
            bytestoread -= rc;
        }

        if ( error ) {
            if ( (error == netErrWouldBlock) || (error == netErrTimeout) ) {
                if ( !block ) {
                    return AG_NET_WOULDBLOCK;
                }
            } else {
                handleNetworkErr(error);
                return -1;
            }
        }
        
    } while (block && (bytestoread > 0)); 
    
    return bytesread;
}
//#ifdef KOLBADNEEDSTODECIDEWHATTODOWITHTHIS
//Boolean NetlibConnectionIsLive(void)
//{
//    Word count = 0;
//    Word networkLibRefNum = 0;
//
//    networkLibRefNum = getNetworkLibRefNum();
//    if(!networkLibRefNum)
//        return false;
//
//    if(!pilotSupportsNetwork())
//        return false;
//
//    NetLibOpenCount(AppNetRefnum, &count);
//
//    // If we think it's open, make sure
//    if(count > 0) {
//        Word err = 0;
//        Word ifErrs = 0;
//        Boolean allInterfacesUp;
//    
//        err = NetLibConnectionRefresh (networkLibRefNum,
//                                        false,
//                                        &allInterfacesUp,
//                                        &ifErrs);
//        if(ifErrs == 0 && err == 0 && allInterfacesUp != NULL)
//            return true;
//    }
//    
//    // If we think it's down and it's really up, that's ok (I think)
//    return false;
//}
//#endif
/*---------------------------------------------------------------------------*/
static void handleNetworkErr(Err err)
{
//    char msg[10];
//    if(err < 1)
//       return;
//    bzerp(msg, 10);
//    itoa(msg, err&~netErrorClass);
//    FrmCustomAlert(errorGettingDataAlertId, msg, " ", " ");
    return;
}
/*---------------------------------------------------------------------------*/
int32 AGNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 bytes, 
                AGBool block)
{
    
    Err err;
    int32 rc = 1;
    int32 bytesread = 0;
    int32 bytestoread = bytes;
    uint8 *buf = buffer;

    do {
        
        rc = NetLibReceive( ctx->networkLibRefNum, soc->fd, buf, bytestoread,
                            0, NULL, NULL, MAXTIMEOUT,
                            &err);
        
        if ( rc > 0 ) {
            bytesread += rc;
            buf = buffer + bytesread;
            bytestoread = bytes - bytesread;
        }
        
        if ( err ) {
            if ( (err == netErrWouldBlock) || (err == netErrTimeout) ) {
                if ( !block ) {
                    return AG_NET_WOULDBLOCK;
                }
            } else {
                return -1;
            }
        }
        
    } while (block && (bytestoread > 0));
    
    return bytesread;
    
}

/*---------------------------------------------------------------------------*/
ExportFunc
int32 AGNetGets(AGNetCtx *ctx, AGSocket *soc, uint8 *buf, int32 offset,
                int32 bytes, int32 *bytesread, AGBool block)
{
    
    int32 rc = 0;
    unsigned char b;
    *bytesread = 0;
   
    if (bytes > 1)
        bytes -= 1;  

    if (bytes == 0)
        return 0;       
    
    do {
        //PENDING(bill) should read in chunks for efficiency 
        //   (see AGNetReadProtected)
        rc = AGNETRECV(ctx, soc, &b, 1, block);

        if ( rc > 0 ) {
            AGProtectedWrite (buf, offset + *bytesread, &b, 1); //buf[*bytesread] = b;
            *bytesread += 1;
        } else {
            return rc;
        }
        
    } while (*bytesread < bytes && b != '\n');
    
    if (bytes > 1)
        AGProtectedZero(buf, offset + *bytesread, 1); //buf[*bytesread] = 0;
    
    return *bytesread;    
}

/*---------------------------------------------------------------------------*/
static Boolean pilotSupportsNetwork() 
{
    Word tmp = 0;
    Word networkSupported = 0;

    if(networkSupported == 0) {
        // See if we have a pilot that can do net stuff.
        if (SysLibFind("Net.lib", &tmp)) {
            networkSupported = 1;
        } else {
            networkSupported = 2;
        }
    }
    return (networkSupported == 2);
}
ExportFunc
sword  AGSocksConnect(AGNetCtx *ctx, AGSocket *soc, uint32 socksLaddr, int16 socksServerPort,
                      char *destAddr, int16 destHostPort, AGBool block)
{
    //PENDING(bill) This should never happen on the Palm.
    return 0;
}
/*---------------------------------------------------------------------------*/
sword AGNetCanDoSecure(AGNetCtx *ctx)
{
    return 0;
}
/*---------------------------------------------------------------------------*/
int32 AGNetGetCtxSize(void)
{
    return (int32)sizeof(AGNetCtx);
}
/*---------------------------------------------------------------------------*/
void AGNetToggle(AGNetCtx *ctx, AGBool use)
{
    /* Does nothing */
}

#endif /* __palmos__ */

