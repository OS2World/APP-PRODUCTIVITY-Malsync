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

/*
    This is a simple state machine that goes through the following states   
    in order to get the data from commands from a MAL server.

    You will almost never need to deal with this directly. The AGClientProcessor
    uses this to get it's network processing down for it. You will normally
    be talking to the AGClientProcessor.

    This is intended to be a non-blocking loop, so the way this is used is
    to request an action, say AGSyncProcessorConnect(). You should then call
    AGSyncProcessorProcess() to allow the machine to do work. You should 
    continue to call AGSyncProcessorProcess() while it returns AGSYNC_CONTINUE.
    Note The current system does not allow you to be receiving and sending at 
    the same time. In MAL, you send all the stuff up, then you switch directions
    and get all the commands coming down.

        IDLE
            It's completed the last rquest successfully. 
            It will stay in this state until the user requests another action.

        GET_HOST
            We go into this state when AGSyncProcessorConnect() is called.
            This will attempt to resolve the serverName. 
            When it is successful, the machine will go into the CONNECT state
        CONNECT
            In this state we're trying to open a socket to the server.
            When this completes we will switch to the IDLE state.
        GET_HTTP_HEADER
            Reads an http header from the socket.
        GET_MAGIC
            Reads the expected MAGIC and VERSION info from the socket
        GET_CMD
            AGSyncProcessorGetNextCommand()  puts the machine in this state.
            This state is looking to read a CINT off the network.
            When it's got one, it changes to the GET_CMD_LEN state
        GET_CMD_LEN
            This is the second state of the AGSyncProcessorGetNextCommand() 
            action. We're looking for a second CINT to determine the length
            of the command.
            When we have this we switch to the GET_CMD_DATA state.
        GET_CMD_DATA
            Finally we get the data that represents a command.
            When this completes we switch to the IDLE state.

        SENDING
            This state is entered from AGSyncProcessorSendBuffer().
            We'll stay in this state until all the data has been set to the
            server, at which time it will switch to the IDLE state.
            
        PROBLEM
            This state occurs whenever there is a problem. In general, the 
            connection to the server will be terminated if there is a problem.
*/

#ifndef __AGSYNCPROCESSOR_H__
#define __AGSYNCPROCESSOR_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGNet.h>
#include <AGMsg.h>
#include <AGProtectedMem.h>
#include <AGUtil.h>
#include <AGLocationConfig.h>

typedef enum {
    /* Processor has completed the last requested action, or has nothing to do */
    AGSYNC_IDLE = 0,
    /* Processor still has work to do to complete the last action */
    AGSYNC_CONTINUE,
    /* An error has occured during the processing, check the errStringId */
    AGSYNC_ERR,
    /* We received an 401 connecting to a proxy server */
    AGSYNC_401

} AGSyncProcessResults;

#define NO_TIMEOUT          0
#define NO_MAX_READ_SIZE    0

// These are in seconds
#define DEFAULT_CONNECT_TIMEOUT     30
#define DEFAULT_WRITE_TIMEOUT       30
#define DEFAULT_READ_TIMEOUT        60

#ifndef __palmos__
#define MAX_READ_SIZE       NO_MAX_READ_SIZE
#else
#define MAX_READ_SIZE       1024
#endif  /* !__palmos__ */

typedef int32 (*AGSendDataFunc)(void *out, AGNetCtx *ctx, AGSocket *socket, 
                                uint8 *buffer,
                                int32 bytesToSend, 
                                AGBool block);
struct AGClientProcessor;
typedef struct AGSyncProcessor {
    char *serverName;
    int16 serverPort;

    int16 state;
    uint32 command;
    uint32 commandLen;
    uint32 errStringId;

    AGSocket *socket; 
    AGBool freeBuffer;
    const uint8 *buffer;
    uint32 buffersize;
    uint32 offset;
    uint32 requestedBytes;
    uint32 bytesProcessed;

    AGSendDataFunc sendDataFunc;
    void *out;

    char *proxyServer;
    char *socksServer;
    int16 proxyPort;
    int16 socksPort;

    int32 returnCode;

    uint32 timeoutAt;
    uint32 beginUserTime;
   
    uint32 connectTimeout;
    uint32 writeTimeout;
    uint32 readTimeout;
    uint32 maxReadSize;

    uint16 magic;
    int8 majorVersion;
    int8 minorVersion;

    AGNetCtx *netctx;
    
    int trying401;
    AGLocationConfig *lc;
    struct AGClientProcessor *cp;


} AGSyncProcessor;

ExportFunc
AGSyncProcessor *AGSyncProcessorNew(char *serverName, int16 serverPort,
                                    void *out, AGSendDataFunc sendDataFunc,
                                    char *proxyServer, int16 proxyPort,
                                    char *socksServer, int16 socksPort,
                                    AGNetCtx *ctx);
ExportFunc
AGSyncProcessor *AGSyncProcessorInit(AGSyncProcessor *processor, 
                                     char *serverName, int16 serverPort,
                                     void *out, AGSendDataFunc sendDataFunc,
                                     char *proxyServer, int16 proxyPort,
                                     char *socksServer, int16 socksPort,
                                     AGNetCtx *ctx);
                                                
ExportFunc void AGSyncProcessorFinalize(AGSyncProcessor *processor);
ExportFunc void AGSyncProcessorFree(AGSyncProcessor *processor);

ExportFunc int32 AGSyncProcessorProcess(AGSyncProcessor *processor);
ExportFunc void AGSyncProcessorSetTimeouts(AGSyncProcessor *processor,
                                           uint32 connectTimeoutSeconds,
                                           uint32 writeTimeoutSeconds,
                                           uint32 readTimeoutSeconds);
/* After calling these functions you need to call AGSyncProcessorProcess()
 until it returns AGSYNC_IDLE */
ExportFunc void AGSyncProcessorConnect(AGSyncProcessor *processor);
ExportFunc void AGSyncProcessorGetHeader(AGSyncProcessor *processor);
ExportFunc void AGSyncProcessorGetMagic(AGSyncProcessor *processor);
ExportFunc void AGSyncProcessorSendBuffer(AGSyncProcessor *processor,
                                        uint8 *buffer, uint32 len);
ExportFunc void AGSyncProcessorGetNextCommand(AGSyncProcessor *processor);

/* These calls are 'blocking' */
ExportFunc uint8 *AGSyncProcessorGetCommandBuffer(AGSyncProcessor *processor);
ExportFunc void AGSyncProcessorDisconnect(AGSyncProcessor *processor);

ExportFunc void AGSyncProcessorSetSendDataFunc(AGSyncProcessor *processor, 
                                      void *out, AGSendDataFunc sendDataFunc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGSYNCPROCESSOR_H__ */
