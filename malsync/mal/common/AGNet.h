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

#ifndef __AG_NET_H__
#define __AG_NET_H__

#include <AGTypes.h>

#ifdef _WIN32
#   ifdef _WIN32_WCE
#       include <windows.h>
#       include <winsock.h>
#   else /* _WIN32_WCE */
/* 
   If this causes you compilation issues, you need to 
   move the includes around so that this file is included
   BEFORE the windows.h header.
 */
#       define FD_SETSIZE 512
#       include <winsock2.h>
#   endif /* _WIN32_WCE */
    typedef struct sockaddr_in AGSockAddr;
    typedef int AGFd;
#else /* _WIN32 */
#   ifdef __palmos__
#       include <Pilot.h>
#       include <sys_socket.h>
        typedef NetSocketAddrINType AGSockAddr;
        typedef Word AGFd;
#   else /* __palmos__ */
#       if (defined(macintosh))
#           if (!defined(__palmos__)) /* Added here for maintainability */
#               include <OpenTransport.h>
#               include <OpenTptInternet.h>
                typedef int AGSockAddr;
                typedef int AGFd;
#           endif /* if (!defined(__palmos__)) */
#       else /* defined(macintosh) */
#           include <sys/types.h>
#           include <sys/socket.h>
#           include <unistd.h>
#           include <netdb.h>
#           include <netinet/in.h>
#           ifdef __sun__
#               include <sys/filio.h>
#               include <arpa/inet.h>
#           else
#               if defined(__FreeBSD__) || defined(_HPUX_SOURCE) || defined(__EMX__)
#                   include <sys/ioctl.h>
#                   include <arpa/inet.h>
#               else
#                   include <asm/ioctls.h>
#               endif /* __FreeBSD__ */
#           endif /* __sun__ */
            typedef struct sockaddr_in AGSockAddr;
            typedef int AGFd;
#       endif /* !defined(macintosh) */
#   endif /* __palmos__ */
#endif /* _WIN32 */


#define AG_NET_UNKNOWN_PROXY_ERROR   -97
#define AG_NET_PROXY_AUTH_ERROR      -98
#define AG_NET_PROXY_FORBIDDEN       -96
#define AG_NET_SOCKS_ERROR_CONNECTTO -99
#define AG_NET_SOCKS_ERROR           -100
#define AG_NET_SOCKS_BAD_ID          -101
#define AG_NET_SOCKS_COULDNT_CONNECT -102
#define AG_NET_WOULDBLOCK            -30
#define AG_NET_IOERR                 -13
#define AG_NET_MEMORYERROR           -12
#define AG_NET_ERROR_BAD_PROXYNAME   -11
#define AG_NET_ERROR_BAD_HOSTNAME    -10
#define AG_NET_ERROR_NO_SOCKET       -9
#define AG_NET_ERROR_NO_CONNECT      -8
#define AG_NET_ERROR_RECV            -7
#define AG_NET_EISCONN               -6
#define AG_NET_ERROR                 -5

struct AGSocket;
struct AGNetCtx;

typedef enum AGSocketState {
    AG_SOCKET_ERROR = 1,
    AG_SOCKET_NEW,
    AG_SOCKET_CONNECTING,
    AG_SOCKET_CONNECTED
} AGSocketState;


#ifdef __cplusplus
extern "C" {
#endif

typedef struct AGSocket *(*AGNetSocketNewFunc)(struct AGNetCtx *ctx);
/*
 * Name:        AGNetSocketNew
 * Function:    Creates a new socket. 
 * Input:       None.
 * Output:      None.
 * Return:      Pointer to socket struture. Must be free 
 *              with AGNetSocketFree
 */
typedef void (*AGNetSocketFreeFunc)(struct AGNetCtx *ctx, 
                                    struct AGSocket *soc);
/*
 * Name:        AGNetSocketFree
 * Function:    Frees memory associated with socket, also
 *              closes socket if necessary. 
 * Input:       None.
 * Output:      None.
 * Return:      None.
 */
typedef sword (*AGNetConnectFunc)(struct AGNetCtx *ctx, 
                                  struct AGSocket *soc, uint32 laddr, short port,  
                                  AGBool block);
/*
 * Name:        AGNetConnect
 * Function:    Function that connects to internet host. For
 *              a blocking call or the first time this is 
 *              laddr and port must be valid. Subsequent calls 
 *              for a non blocking 
 *              connect ignore the name and port an just 
 *              operate on the information in the AGSocket
 *              structure. 
 * Input:       laddr  - address of host to connect to. This
 *                       is typically found by calling 
 *                       AGNetGetHostAddr
 *              port   - port on host to connect to.
 *              block - if this function should block.
 * Output:      None.
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              0 on sucessful connection.
 */
typedef int32 (*AGNetReadFunc)(struct AGNetCtx *ctx, struct AGSocket *soc, 
                               uint8 *buffer, int32 bytes, 
                               AGBool block);
/*
 * Name:        AGNetRead
 * Function:    Read data from socket into passed in buffer.
 * Input:       soc    - socket
 *              bytes  - bytes to read
 *              block  - if this function should block.
 * Output:      buffer - data from scoket 
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              0 if no more data to read, postive numbers
 *              equal number of bytes read into buffer.
 */
typedef int32 (*AGNetSendFunc)(struct AGNetCtx *ctx, struct AGSocket *soc, 
                               const uint8 *data, int32 bytes,
                               AGBool block);
/*
 * Name:        AGNetSend
 * Function:    write data from buffer into socket.
 * Input:       soc    - socket
 *              bytes  - bytes to write
 *              block  - if this function should block.
 *              data - data to write 
 * Output:      None.
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              anything else is number of bytes written.
 */
typedef sword (*AGNetCloseFunc)(struct AGNetCtx *ctx, struct AGSocket *soc);
/*
 * Name:        AGNetSocketClose
 * Function:    Close a socket
 * Input:       soc    - socket
 * Output:      None.
 * Return:      0 on success, anyting else is an error.
 */

typedef int32 (*AGNetReadProtectedFunc)(struct AGNetCtx *ctx, 
                                        struct AGSocket *soc, 
                                        uint8 *buffer, int32 offset,
                                        int32 bytes, AGBool block);
/*
 * Name:        AGNetReadProtected
 * Function:    Read data from socket into passed in protectedbuffer.
 * Input:       soc    - socket
 *              offset - offset within buffer to start writing to
 *              bytes  - bytes to read
 *              block  - if this function should block.
 * Output:      buffer - data from scoket 
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              0 if no more data to read, postive numbers
 *              equal number of bytes read into buffer.
 */

typedef struct AGSocket {
    AGSocketState state;
    AGFd fd;
    uint32 addr;
    AGSockAddr saddr;
    uint8 *userData; 
#if (defined(macintosh) && !defined(__palmos__))
    AGBool bound;
    EndpointRef ep;
    TCall sndCall;
#endif /* #if (defined(macintosh) && !defined(__palmos__)) */
} AGSocket;

typedef struct AGNetCtx {
    AGNetSendFunc send;
    AGNetConnectFunc connect;
    AGNetReadFunc recv;
    AGNetCloseFunc close;
    AGNetSocketNewFunc socnew;
    AGNetSocketFreeFunc socfree;
    AGNetReadProtectedFunc recvdm;
    uint8 *userData;
#ifdef __palmos__
    Word networkLibRefNum; 
#endif
#if (defined(macintosh) && !defined(__palmos__))
    InetSvcRef inet_services;
#endif /* #if (defined(macintosh) && !defined(__palmos__)) */

} AGNetCtx;

#define AGNETSEND(ctx, a, b, c, d) ( (*(ctx)->send)((ctx), (a), (b), (c), (d)) )
#define AGNETRECV(ctx, a, b, c, d) ( (*(ctx)->recv)((ctx), (a), (b), (c), (d)) )
#define AGNETCONNECT(ctx, a, b, c, d) ( (*(ctx)->connect)((ctx), (a), (b), (c), (d)) )
#define AGNETSOCKETCLOSE(ctx, a) ((*(ctx)->close)((ctx), (a)))
#define AGNETSOCKETNEW(ctx) ((*(ctx)->socnew)(ctx)) 
#define AGNETSOCKETFREE(ctx, a) ((*(ctx)->socfree)((ctx), (a))) 
#define AGNETRECVDM(ctx, a, b, c, d, e) ( (*(ctx)->recvdm)((ctx), (a), (b), (c), (d), (e)) )

ExportFunc 
sword AGNetInit(AGNetCtx *ctx);
/*
 * Name:        AGNetInit
 * Function:    Tells OS that we are going to want to do
 *              network IO
 * Input:       None.
 * Output:      None.
 * Return:      0 on Successs or error code
 */
ExportFunc
void AGNetSetIOFuncs(AGNetCtx *ctx, AGNetSendFunc send,
                     AGNetConnectFunc connect,
                     AGNetReadFunc recv,
                     AGNetCloseFunc close,
                     AGNetSocketNewFunc socnew,
                     AGNetSocketFreeFunc socfree,
                     AGNetReadProtectedFunc recvdm);
ExportFunc 
sword AGNetClose(AGNetCtx *ctx);
/*
 * Name:        AGNetInit
 * Function:    Tells OS that we are going no longer need to
 *              do any network IO
 * Input:       None.
 * Output:      None.
 * Return:      0 on Successs or error code
 */
ExportFunc 
sword  AGSocksConnect(AGNetCtx *ctx, AGSocket *soc, uint32 saddr,
                      int16 socksPort,
                      char *destAddr, int16 destHostPort,
                      AGBool block);
/*
 * Name:        AGNetSOCKSConnect
 * Function:    Function that can tunnel through a SOCKS proxy.
 *              After the first call all other info 
 *              except that in the soc struct is ignored.
 *              After the connect is successful the
 *              standard AGNetRead/AGNetWrite can be used
 *              on soc.
 * Input:       soc    - Socket opened by AGNetSocketNew
 *              destHost - destination of data
 *              destHostPort - port of destination server
 *              totalBytesToSend - size of data to send
 *              socksServer - HTTP server to tunnel through
 *              SocksServerPort - HTTP server port
 *              destHostPort - port of destination server
 *              userid - Pass in NULL for default.
 *              block - should call block
 * Output:      None.
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              0 on sucessful connection.
 */
ExportFunc 
uint32 AGNetGetHostAddr(AGNetCtx *ctx, char *name);
/*
 * Name:        AGNetGetHostAddr
 * Function:    Get the 4 byte network address of the 
 *              passed in string
 * Input:       name - string to lookup
 * Output:      None.
 * Return:      O is error else network address.
 */
ExportFunc 
int32 AGNetGets(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 offset,
                int32 bytes, int32 *bytesread, AGBool block);
/*
 * Name:        AGNetGets
 * Function:    Read a line form a socket. Will read from 
 *              socket till a '\n' is reached or until
 *              it has read in the passed in buffer limit.
 * Input:       soc    - socket
 *              bytes  - size of buffer
 * Output:      buffer - data from socket
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              0 if no more data to read, postive numbers
 *              equal number of bytes read into buffer.
 *              If if couldnt read a whole line and 
 *              the network returned a would block, the 
 *              amount of data read is retuned in the 
 *              bytesread var.
 */

#ifdef __palmos__
ExportFunc
int32 AGNetReadProtected(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, 
                         int32 offset, int32 bytes, AGBool block);
/*
 * Name:        AGNetReadProtected
 * Function:    Read data from socket into passed in protectedbuffer.
 * Input:       soc    - socket
 *              offset - offset within buffer to start writing to
 *              bytes  - bytes to read
 *              block  - if this function should block.
 * Output:      buffer - data from scoket 
 * Return:      for non blocking call will return 
 *              AG_NET_WOULDBLOCK if call would block.
 *              Any other negative number is an error.
 *              0 if no more data to read, postive numbers
 *              equal number of bytes read into buffer.
 */
ExportFunc

#ifdef __palmos__
sword AGNetCanDoSecure(AGNetCtx *ctx);
int32 AGNetGetCtxSize(void);
void AGNetToggle(AGNetCtx *ctx, AGBool use);
#endif
    
#endif



#ifdef __cplusplus
}
#endif


#endif /* __AG_NET_H__ */




