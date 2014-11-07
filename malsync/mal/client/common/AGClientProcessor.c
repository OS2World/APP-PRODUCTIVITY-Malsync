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

#include <AGClientProcessor.h>
#include <AGUtil.h>
#include <AGProtocol.h>
#include <AGNet.h>
#include <AGDigest.h>
#include <AGProxy.h>
#include <AGProxyDebug.h>
#include <AGBase64.h>

typedef enum {
    IDLE = 0,
    CONNECT, 
    PING,
    HELLO,
    DEVICEINFO,
    SEND_RECRS,
    SENDING_EXTENSIONS,
    GOODBYE,
    SEND_HEADER,
    SEND_BUFFER_LOGON,
    GET_HEADER,
    GET_MAGIC,
    RECEIVING_CMDS,
    PROCESSING_BUFFERED_CMDS,
    PROBLEM
} AGProcessorState;

static void stateChangeToHELLO(AGClientProcessor *processor);
static void stateChangeToPING(AGClientProcessor *processor);
static void stateChangeToDEVICEINFO(AGClientProcessor *processor);
static void stateChangeToRECRS(AGClientProcessor *processor);
static void processRECRS(AGClientProcessor *processor);
static void stateChangeToEXTENSION(AGClientProcessor *processor);
static void processExtensions(AGClientProcessor *processor);
static void stateChangeToGOODBYE(AGClientProcessor *processor);
static void stateChangeToSENDHEADER(AGClientProcessor *processor);
static void stateChangeToGETHEADER(AGClientProcessor *processor);
static void stateChangeToSENDBUFFERLOGON(AGClientProcessor *processor);
static void stateChangeToRECEIVING(AGClientProcessor *processor);
static void stateChangeToHELLOForReal(AGClientProcessor *processor);
static void stateChangeToPROCESSCMD(AGClientProcessor *processor);
static int32 processCMDS(AGClientProcessor *processor);
static void stateChangeToMAGIC(AGClientProcessor *processor);

static void cleanUpLogonMemory(AGClientProcessor *processor);
static int32 processCommand(AGClientProcessor *processor);
static int32 writeToLogonBuffer(void *out, AGNetCtx *ctx, AGSocket *socket, 
                                uint8 *buffer, int32 bytesToSend, AGBool block);
static void sendBuffer(AGClientProcessor *processor);
static int32 processNotComplete(AGClientProcessor *processor, 
                                int32 rc, int32 maxRetries,
                                int32 retryFailStringId);
static void syncComplete(AGClientProcessor *processor);
#ifdef _WIN32
static int32 queueCommand(AGClientProcessor *processor);
#endif // #ifdef _WIN32

/*---------------------------------------------------------------------------*/
ExportFunc
AGClientProcessor *AGClientProcessorNew(AGServerConfig *serverInfo,
                                        AGDeviceInfo *deviceInfo,
                                        AGLocationConfig *lc,
                                        AGPlatformCalls *platformCalls,
                                        AGBool bufferCommands,
                                        AGNetCtx *netctx)
{
    AGClientProcessor *processor;
    processor = (AGClientProcessor *)malloc(sizeof(AGClientProcessor));
    if(processor != NULL)
        AGClientProcessorInit(processor, 
                              serverInfo, 
                              deviceInfo,
                              lc,
                              platformCalls,
                              bufferCommands,
                              netctx);
    return processor;
}

ExportFunc
AGClientProcessor *AGClientProcessorInit(AGClientProcessor *processor, 
                                         AGServerConfig *serverInfo,
                                         AGDeviceInfo *deviceInfo,
                                         AGLocationConfig *lc,
                                         AGPlatformCalls *platformCalls,
                                         AGBool bufferCommands,
                                         AGNetCtx *netctx)
{
    char *proxyServer = NULL, *socksServer = NULL;
    int16 proxyPort = 0, socksPort = 0;
    
    memset(processor, 0, sizeof(AGClientProcessor));
    processor->state = IDLE;
    processor->serverInfo = serverInfo;
    processor->deviceInfo = deviceInfo;

    if (lc) {

        AGBool excludeProxy = FALSE;

        excludeProxy = AGProxyCheckExclusionArray(lc->exclusionServers,
            serverInfo->serverName);

        if(!excludeProxy && lc->HTTPUseProxy && lc->HTTPName && lc->HTTPPort) {
            proxyServer = lc->HTTPName;
            proxyPort   = lc->HTTPPort;
        }
        if (!excludeProxy && lc->SOCKSUseProxy && lc->SOCKSName && lc->SOCKSPort) {
            socksServer = lc->SOCKSName;
            socksPort   = lc->SOCKSPort;
        }
        processor->lc = lc;
    }
    processor->platformCalls = platformCalls;
    AGSyncProcessorInit(&processor->syncProcessor, 
                        serverInfo->serverName,
                        serverInfo->serverPort,
                        NULL,  NULL,
                        proxyServer, proxyPort,
                        socksServer, socksPort,
                        netctx);
    
    /* Added for evil 401 proxies */
    processor->syncProcessor.lc = lc;
    processor->syncProcessor.cp = processor;

    AGSyncProcessorSetTimeouts(&processor->syncProcessor, 
                               processor->serverInfo->connectTimeout,
                               processor->serverInfo->writeTimeout,
                               processor->serverInfo->readTimeout);
    AGBufferWriterInit(&processor->writer, 1024);
    processor->writerInited = TRUE;
    processor->bufferCommands = bufferCommands;
    return processor;
}

ExportFunc void AGClientProcessorFinalize(AGClientProcessor *processor)
{
    processor->state = IDLE;
    cleanUpLogonMemory(processor);
    AGSyncProcessorFinalize(&processor->syncProcessor);

#if defined(_WIN32) && !defined(_WIN32_WCE)
    if(processor->logConnectionInfo)
         proxyDebugCleanup(processor);
#endif

}

static void cleanUpLogonMemory(AGClientProcessor *processor)
{
    if(processor->writeBuffer != NULL) {
        free(processor->writeBuffer);
        processor->writeBuffer = NULL;
    }
    if(processor->writerInited) {
        AGBufferWriterFinalize(&processor->writer);
        processor->writerInited = FALSE;
    }    
    if(processor->serverCommandReader)
        AGBufferReaderFree(processor->serverCommandReader);
    processor->serverCommandReader = NULL;
    if(processor->logonBufferWriter)
        AGBufferWriterFree(processor->logonBufferWriter);
    processor->logonBufferWriter = NULL;
}

ExportFunc void AGClientProcessorSetBufferServerCommands(
                                         AGClientProcessor *processor,
                                         AGBool buffer)
{
    processor->bufferServerCommands = buffer;
}

ExportFunc void AGClientProcessorFree(AGClientProcessor *processor)
{
    AGClientProcessorFinalize(processor);
    free(processor);
}

ExportFunc void AGClientProcessorSync(AGClientProcessor *processor)
{
    AGSyncProcessorConnect(&processor->syncProcessor);
    processor->state = CONNECT;
    processor->pingRequest = FALSE;
}

ExportFunc void AGClientProcessorPing(AGClientProcessor *processor)
{
    AGSyncProcessorConnect(&processor->syncProcessor);
    processor->state = CONNECT;
    processor->pingRequest = TRUE;
}

ExportFunc int32 AGClientProcessorProcess(AGClientProcessor *processor)
{
    int32 rc = AGCLIENT_IDLE;
    int32 syncrc;

    switch (processor->state) {
    case IDLE:
        rc = AGCLIENT_IDLE;
        break;

    case CONNECT:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            processor->calcBufferPass = TRUE;
            rc = AGCLIENT_CONTINUE;
            if(processor->pingRequest) 
                stateChangeToPING(processor);
            else
                stateChangeToHELLO(processor);
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case HELLO:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            if(processor->serverInfo->sendDeviceInfo)
                stateChangeToDEVICEINFO(processor);
            else
                stateChangeToRECRS(processor);
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case PING:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            stateChangeToGOODBYE(processor);
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case DEVICEINFO:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            stateChangeToRECRS(processor);
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case SEND_RECRS:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            processRECRS(processor);
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case SENDING_EXTENSIONS:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            processExtensions(processor);
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

/*
    GOODBYE is the last state that sends 'real' MAL commands.
    There are two possiblies here.
    
    If we are buffering commands, then this is the end of the sending
    state, switch to the SENDHEADER state to send the HTTP headers and
    push the buffer on along after it. Then switch to GET_HEADER to 
    look for the HTTP headers send from the server.

    If we are NOT buffering and this is the calcBufferPass, we want to 
    send the content headers and then restart the state machine at HELLO,
    this time really sending the commands.

    If we are NOT buffering and we are not in calcBufferPass, we've actually
    just finished sending the MAL commands to the server, switch to the
    GET_HEADER state and look for the HTTP headers from the server.
    
 */
    case GOODBYE:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            if(!processor->bufferCommands) {
                if(processor->calcBufferPass) {
                    // In this state, none of the MAL commands have actually
                    // been sent, the writer functions were just counting
                    // bytes. Now we know how many bytes we're going to send,
                    // we can determine the proper content-length.

                    // This is going to send the HTTP headers,
                    // turns the calcBufferPass false, then call
                    // HELLO to actually send the MAL commands.
                    // This is the two-pass, non-buffered version
                    // it ends here in GOODBYE, with the calcBufferPass
                    // false, which switches to the GET_HEADER state.
                    stateChangeToSENDHEADER(processor);
                } else {
                    // We've just completed the second pass of 'really' sending 
                    // the MAL commands, and want to start receiving the
                    // HTTP header lines from the server
    
                    // Also note that this is almost exclusively called by the
                    // on device code, which does not have the space to buffer
                    // all the commands. So this and the state change in 
                    // SEND_BUFFER_LOGON case goto the same place.
                    stateChangeToGETHEADER(processor);
                }
            } else {
                // The MAL commands have been buffered into the processor
                // so we know the content-length and we can just send the
                // entire buffer of MAL commands to the server.

                // This will send the HTTP headers, the buffer, and then
                // switch into the GET_HEADER state.
                stateChangeToSENDHEADER(processor);
            }
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case SEND_HEADER:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            if(processor->bufferCommands)
                stateChangeToSENDBUFFERLOGON(processor);
            else
                stateChangeToHELLOForReal(processor);
            rc = AGCLIENT_CONTINUE;
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case SEND_BUFFER_LOGON:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            stateChangeToGETHEADER(processor);
            rc = AGCLIENT_CONTINUE;
         } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

/*
    GET_HEADER is the beginning of the reading phase.
 */
    case GET_HEADER:

        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            /*PENDING(klobad) if there is a problem where's the cleanup*/
            if(processor->logonBufferWriter)
                AGBufferWriterFree(processor->logonBufferWriter);
            processor->logonBufferWriter = NULL;
            stateChangeToMAGIC(processor);
            rc = AGCLIENT_CONTINUE;
        } else if (syncrc == AGSYNC_ERR || syncrc == AGSYNC_CONTINUE) {
        /*PENDING(klobad) if there is a problem where's the cleanup*/
            if(processor->logonBufferWriter)
                AGBufferWriterFree(processor->logonBufferWriter);
            processor->logonBufferWriter = NULL;
            rc = processNotComplete(processor, syncrc, 0, 0);
        } else { // YOU_PROXY_BASTARD
            processor->state = CONNECT;            
            rc = AGCLIENT_CONTINUE;
        }
        break;

    case GET_MAGIC:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
            //PENDING(klobad) minor version checking??
            if(processor->syncProcessor.magic 
                != ((AG_PROTOCOL_MAGIC_HIGH << 8) | (AG_PROTOCOL_MAGIC_LOW))) {
                processor->errStringId = AGMSGInvalidMagicStringId;
                processor->state = PROBLEM;
                rc = AGCLIENT_CONTINUE;
            } else if(processor->syncProcessor.majorVersion
                    > AG_PROTOCOL_MAJOR_VERSION) {
                processor->errStringId = AGMSGIncompatibleVersionStringId;
                processor->state = PROBLEM;
                rc = AGCLIENT_CONTINUE;
            } else {
                stateChangeToRECEIVING(processor);
                rc = AGCLIENT_CONTINUE;
            }
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

/*
    RECEIVING_CMDS is NOT the beginning of the reading phase.
    RECEIVING_CMDS is NOT the beginning of the reading phase.
 */
    case RECEIVING_CMDS:
        syncrc = AGSyncProcessorProcess(&processor->syncProcessor);
        if(syncrc == AGSYNC_IDLE) {
#ifdef _WIN32
            if(processor->threadHandle)
                rc = queueCommand(processor);
            else
#endif // #ifdef _WIN32
                rc = processCommand(processor);
            if(rc == AGCLIENT_ERR) {
                /*PENDING(klobad) error processing command*/
                processor->errStringId = AGMSGUnknownFailureStringId;
                processor->state = PROBLEM;
                rc = AGCLIENT_CONTINUE;
            } else if(rc == AGCLIENT_CONTINUE) {
                /* Tell the SyncProcessor to get the next command*/
                AGSyncProcessorGetNextCommand(&processor->syncProcessor);
            } else if(rc == AGCLIENT_IDLE) {
                /* we're done processing commands*/
                if(processor->bufferServerCommands) {
                    syncComplete(processor);
                    stateChangeToPROCESSCMD(processor);
                    rc = AGCLIENT_CONTINUE;
                } else {
                    syncComplete(processor);
                    processor->state = IDLE;
                }
            }
        } else {
            rc = processNotComplete(processor, syncrc, 0, 0);
        }
        break;

    case PROCESSING_BUFFERED_CMDS:
        rc = processCMDS(processor);
        if(rc == AGCLIENT_IDLE) {
            syncComplete(processor);
            processor->state = IDLE;
        }
        break;

    case PROBLEM:
        /* Whoever sets the state to PROBLEM, 
         should set the errStringId properly*/
        syncComplete(processor);
        rc = AGCLIENT_ERR;
        break;      
    }
    return rc;
}
/*---------------------------------------------------------------------------*/
static void syncComplete(AGClientProcessor *processor)
{
    AGSyncProcessorDisconnect(&processor->syncProcessor);
    processor->state = IDLE;
}
/*---------------------------------------------------------------------------*/
static int32 writeToLogonBuffer(void *out, AGNetCtx *ctx, AGSocket *socket, 
                                uint8 *buffer,
                                int32 bytesToSend, 
                                AGBool block)
{
    return AGWriteBytes((AGWriter *)out, buffer, bytesToSend);
}
/*---------------------------------------------------------------------------*/
static void initMALConversation(AGClientProcessor *processor)
{
    if(processor->bufferCommands) {
        processor->logonBufferWriter = AGBufferWriterNew(1024);
        AGSyncProcessorSetSendDataFunc(&processor->syncProcessor, 
                                        processor->logonBufferWriter, 
                                        writeToLogonBuffer);
    } else {
        if(processor->calcBufferPass) {
            /* init the writer to just count bytes*/
            processor->logonBufferWriter = AGBufferWriterNew(1024);
            AGWriterInit((AGWriter *)processor->logonBufferWriter, NULL, NULL);
            AGSyncProcessorSetSendDataFunc(&processor->syncProcessor, 
                                            processor->logonBufferWriter, 
                                            writeToLogonBuffer);
        } else {
            /* Let default AGSyncProcessor() sending happen*/
            AGSyncProcessorSetSendDataFunc(&processor->syncProcessor, 
                                            NULL, 
                                            NULL);
        }
    }

    AGBufferWriterReset(&processor->writer);
    AGWriteMAGIC((AGWriter *)&processor->writer);
    AGWriteMAJORVERSION((AGWriter *)&processor->writer, AG_PROTOCOL_MAJOR_VERSION);
    AGWriteMINORVERSION((AGWriter *)&processor->writer, AG_PROTOCOL_MINOR_VERSION);
}

static void stateChangeToHELLO(AGClientProcessor *processor)
{
    uint8 digestAuth[16];
    AGServerConfig *sc = processor->serverInfo;

    initMALConversation(processor);

    /*  If we were passed in a serverConfig that is requesting that
        cookies be reset, then we're doing an online sync and should
        handle that request right now rather than waiting for the
        profile synchronization code on the desktop to handle it.
    */
    if (sc->resetCookie) {
        sc->resetCookie = FALSE;
        AGDigestSetToNull(sc->nonce);
        CHECKANDFREE(sc->sequenceCookie);
        sc->sequenceCookieLength = 0;
    }
    
    bzero(digestAuth, 16);

    if(AG_HASH_PASSWORD_NO == sc->hashPassword) {

        char pwdbuf[16+1];

        /* We should send up password in cleartext format.
        Stick it in the digestAuth space. */
        bzero(pwdbuf, 16+1);
        if (NULL != sc->cleartextPassword
            && strlen(sc->cleartextPassword) > 0) {
            char * decoded = NULL;
            int32 len = 0;
            decoded = (char*)AGBase64Decode(sc->cleartextPassword, &len);
            strncpy(pwdbuf, decoded, 16);
            free(decoded);
        } else {
            /* This signifies to the server that we're trying to send a
            blank cleartext password. */
            pwdbuf[15] = (char)0xff;
        }
        memcpy(digestAuth, pwdbuf, 16);

    } else {

        /* Create the digest of the username, password and nonce.
        If the nonce is null, we will leave the digestAuth blank
        (this is what happens if we're in the AG_HASH_PASSWORD_UNKNOWN
        state) */
        if (!AGDigestNull(sc->password) && !AGDigestNull(sc->nonce))
            AGDigest(sc->userName, sc->password, sc->nonce, digestAuth);

    }

    AGWriteHELLO2((AGWriter *)&processor->writer,
                 processor->serverInfo->userName,
                 digestAuth,
                 sc->nonce,
                 (processor->deviceInfo)
                 ? processor->deviceInfo->availableBytes : 0, 
                 processor->serverInfo->sequenceCookieLength, 
                 processor->serverInfo->sequenceCookie,
                 processor->serverInfo->uid);
    
    AGSyncProcessorSendBuffer(&processor->syncProcessor, 
                          (uint8 *)AGBufferWriterGetBuffer(&processor->writer),
                          AGBufferWriterGetBufferSize(&processor->writer));
    processor->state = HELLO;
}

static void stateChangeToPING(AGClientProcessor *processor)
{    
    initMALConversation(processor);
    
    AGWritePING((AGWriter *)&processor->writer);
    AGSyncProcessorSendBuffer(&processor->syncProcessor, 
                          (uint8 *)AGBufferWriterGetBuffer(&processor->writer),
                          AGBufferWriterGetBufferSize(&processor->writer));
    processor->state = PING;
}

static void stateChangeToDEVICEINFO(AGClientProcessor *processor)
{
    AGBufferWriterReset(&processor->writer);
    AGWriteDEVICEINFO((AGWriter *)&processor->writer,
        processor->deviceInfo->osName, 
        processor->deviceInfo->osVersion,
        processor->deviceInfo->colorDepth, 
        processor->deviceInfo->screenWidth, 
        processor->deviceInfo->screenHeight,
        processor->deviceInfo->serialNumber,
        processor->deviceInfo->language,
        processor->deviceInfo->charset,
        processor->deviceInfo->platformDataLength,
        processor->deviceInfo->platformData); 
    sendBuffer(processor);
    processor->state = DEVICEINFO;
}

static void appendCLOSE(AGClientProcessor *processor)
{
    if(processor->sentOPEN) {
        AGWriteCLOSEDATABASE((AGWriter *)&processor->writer);
        processor->sentOPEN = FALSE;
        return;
    }
}

static void sendBuffer(AGClientProcessor *processor)
{
    if(AGBufferWriterGetBufferSize(&processor->writer) > 0) {
        AGSyncProcessorSendBuffer(&processor->syncProcessor, 
            (uint8 *)AGBufferWriterGetBuffer(&processor->writer),
            AGBufferWriterGetBufferSize(&processor->writer));
    }
}

static void incrementDBConfig(AGClientProcessor *processor)
{
    appendCLOSE(processor);
    processor->dbConfigIndex++;
    processor->sentOPEN = FALSE;
    processor->state = SEND_RECRS;
}

static void appendUNKNOWN(AGClientProcessor *processor, AGDBConfig *dbconfig)
{    
    AGWriteUNKNOWNDATABASE((AGWriter *)&processor->writer, 
                            dbconfig->dbname);   
}

static void stateChangeToRECRS(AGClientProcessor *processor)
{
    processor->dbConfigIndex = 0;
    processor->sentOPEN = FALSE;
    if(processor->serverInfo->dbconfigs == NULL
        || processor->serverInfo->dbconfigs->count < 1) {
        stateChangeToEXTENSION(processor);
        return;
    }

    processRECRS(processor);
}

//PENDING(klobad) I think particular slice of code might be better represented
//as some additional top level states. As is it's too complex and is really a 
//couple of states within one state.
static void processRECRS(AGClientProcessor *processor)
{
    AGRecord *nextRecord = NULL;
    AGDBConfig *dbconfig = NULL;
    int32 result = AGCLIENT_IDLE;
    int32 errCode = AGCLIENT_NO_ERR;

    processor->state = SEND_RECRS;
    AGBufferWriterReset(&processor->writer);

    /* Are there any databases to send 
        || have we sent them all
    */
    if((processor->serverInfo->dbconfigs == NULL)          
        || (processor->dbConfigIndex 
                >= processor->serverInfo->dbconfigs->count)) {
        if(processor->sentOPEN) {
            incrementDBConfig(processor);
            sendBuffer(processor);
            return;
        } else {
            stateChangeToEXTENSION(processor);
            return;
        }
    }

    dbconfig = (AGDBConfig*)
        AGArrayElementAt(processor->serverInfo->dbconfigs, 
            processor->dbConfigIndex);

    /* All callbacks are required at all times */
    if(processor->platformCalls->openDatabaseFunc == NULL
        || processor->platformCalls->nextModifiedRecordFunc == NULL
        || processor->platformCalls->nextRecordFunc == NULL) {
        incrementDBConfig(processor);
        appendUNKNOWN(processor, dbconfig);
        sendBuffer(processor);
        return;
    }


    /* NOTE: This should never happen, the AG_DONTSEND_CFG type
       should have told the system to delete this dbconfig.
    */
    if(dbconfig->type == AG_DONTSEND_CFG) {
        incrementDBConfig(processor);
        sendBuffer(processor);
        return;
    }

    /* If we've not sent the OPEN, then this is a new database. */
    if(!processor->sentOPEN) {
        result = (*processor->platformCalls->openDatabaseFunc)(
                        processor->platformCalls->out, 
                        dbconfig,
                        &errCode);
        if(result != AGCLIENT_IDLE) {
            incrementDBConfig(processor);
            appendUNKNOWN(processor, dbconfig);
            sendBuffer(processor);
            return;
        }
        //At this point we know we want to send the OPENDATABASE
        //command, but ONLY if there are records to send.
    }

    if(dbconfig->type == AG_SENDMODS_CFG) {
        result = (*processor->platformCalls->nextModifiedRecordFunc)(
                        processor->platformCalls->out, 
                        &nextRecord,
                        &errCode);
    } else { /* dbconfig->type == AG_SENDALL_CFG */
        result = (*processor->platformCalls->nextRecordFunc)(
                            processor->platformCalls->out, 
                            &nextRecord,
                            &errCode);
    }
    
    if(result == AGCLIENT_ERR
        || result == AGCLIENT_IDLE 
        || nextRecord == NULL) {
        incrementDBConfig(processor);
        sendBuffer(processor);
        return;
    }

    //if we get here, and we haven't sentOPEN, then this is the
    //first record to send, write the open and the newids first
    //then send the record bytes
    if(!processor->sentOPEN) {
        AGWriteOPENDATABASE((AGWriter *)&processor->writer, 
                             dbconfig->dbname);
        if(dbconfig->newids && AGArrayCount(dbconfig->newids) > 0) {
            AGWriteNEWIDS((AGWriter *)&processor->writer, dbconfig->newids);
        }
        processor->sentOPEN = TRUE;
    }
    
    if(dbconfig->sendRecordPlatformData) {
        AGWriteRECORD((AGWriter *)&processor->writer, 
            nextRecord->uid, 
            nextRecord->status, 
            nextRecord->recordDataLength, 
            nextRecord->recordData, 
            nextRecord->platformDataLength, 
            nextRecord->platformData);
    } else {
        AGWriteRECORD((AGWriter *)&processor->writer, 
            nextRecord->uid, 
            nextRecord->status, 
            nextRecord->recordDataLength, 
            nextRecord->recordData, 
            0, 
            NULL);
    }
    sendBuffer(processor);
}

static void processExtensions(AGClientProcessor *processor)
{
    int32 command, commandLen, result;
    void *commandBytes = NULL;

    if(processor->platformCalls->nextExpansionCommandFunc == NULL) {
        stateChangeToGOODBYE(processor);
        return;
    }

    result = (*processor->platformCalls->nextExpansionCommandFunc)(
                            processor->platformCalls->out, 
                            &command,
                            &commandLen,
                            &commandBytes);

    if(result == AGCLIENT_IDLE) {
        stateChangeToGOODBYE(processor);
        return;
    }

    AGBufferWriterReset(&processor->writer);
    AGWriteCommand((AGWriter *)&processor->writer, command, 
                        commandLen, commandBytes);
    sendBuffer(processor);
}

static void stateChangeToEXTENSION(AGClientProcessor *processor)
{
    processor->state = SENDING_EXTENSIONS;
    processExtensions(processor);
}

static void stateChangeToGOODBYE(AGClientProcessor *processor)
{
    AGBufferWriterReset(&processor->writer);
    AGWriteEND((AGWriter *)&processor->writer);
    AGSyncProcessorSendBuffer(&processor->syncProcessor, 
        (uint8 *)AGBufferWriterGetBuffer(&processor->writer),
        AGBufferWriterGetBufferSize(&processor->writer));
    processor->state = GOODBYE;
}

static void stateChangeToRECEIVING(AGClientProcessor *processor)
{
    cleanUpLogonMemory(processor);
    if(processor->bufferServerCommands) {
        AGBufferWriterInit(&processor->writer, 1024);
        processor->writerInited = TRUE;
    }
    AGSyncProcessorGetNextCommand(&processor->syncProcessor);
    processor->state = RECEIVING_CMDS;
}

static void stateChangeToGETHEADER(AGClientProcessor *processor)
{
    AGSyncProcessorGetHeader(&processor->syncProcessor);
    processor->state = GET_HEADER;
}

static void stateChangeToMAGIC(AGClientProcessor *processor)
{
    AGSyncProcessorGetMagic(&processor->syncProcessor);
    processor->state = GET_MAGIC;
}
#define CONLEN "Content-Length: "
#define POSTFORMAT "POST %s HTTP/1.0\r\nUser-Agent: Mozilla/3.0 (compatible; MAL  0.7)\r\nHost: %s\r\nContent-Type: application/x-mal-client-data\r\n"
static void stateChangeToSENDHEADER(AGClientProcessor *processor)
{
    int len = 0;
    char *path;
    char *authheader = NULL;
    char num[24];
    AGServerConfig *sc = processor->serverInfo;
    AGLocationConfig *lc = processor->lc;
    
    if (lc && lc->HTTPUseProxy && lc->HTTPName && lc->HTTPPort) {

        if (!sc->serverUri) {
            len = 24 + strlen(sc->serverName);
            path = (char*)malloc(len);
            
            if (!path) {
                processor->errStringId = AGMSGUnknownFailureStringId;
                processor->state = PROBLEM;
                return;
            }
            sprintf(path, "http://%s:%d/sync", sc->serverName, 
                    sc->serverPort);
            
        } else {
            len = strlen(sc->serverUri) + strlen(sc->serverName) + 24; 
            path = (char*)malloc(len);
            if (!path) {
                processor->errStringId = AGMSGUnknownFailureStringId;
                processor->state = PROBLEM;
                return;
            }
            sprintf(path, "http://%s:%d%s", sc->serverName, 
                    sc->serverPort,
                    sc->serverUri);
        }
        if (lc->HTTPUseAuthentication && lc->HTTPUsername &&
            lc->HTTPPassword) {
            authheader =
                AGProxyCreateAuthHeader(lc->HTTPUsername,
                                        lc->HTTPPassword,
                                        lc->proxy401);
        }
    } else {

        if (!sc->serverUri)
            path = "/sync";
        else
            path = sc->serverUri;

    }
    
    bzero(num, 24);
#if defined(_WIN32) || defined(__palmos__)
    if(processor->bufferCommands) {
        itoa(AGBufferWriterGetBufferSize(processor->logonBufferWriter),
                 num, 10);
    } else {
        itoa(((AGWriter *)processor->logonBufferWriter)->totalBytesWritten,
                num, 10);
    }

#else
    if(processor->bufferCommands) {
        sprintf(num, "%d", 
            AGBufferWriterGetBufferSize(processor->logonBufferWriter));
    } else {
        sprintf(num, "%d", 
            ((AGWriter *)processor->logonBufferWriter)->totalBytesWritten);
    }
#endif

    /* Since we have to add port to the URL for proxy stuff, guess 
    at the length here, use strlen to find actual length to send */
    len += strlen(POSTFORMAT) + 24; 
    len += strlen(sc->serverName); /* "Host: %s" in POSTFORMAT */
    len += strlen(path);
    len += strlen(num);
    len += strlen(CONLEN);

    if (authheader)
        len += strlen(authheader);
    len += 4;
    
    if(processor->writeBuffer != NULL) {
        free(processor->writeBuffer );
        processor->writeBuffer = NULL;
    }
    processor->writeBuffer = malloc(len + 1); /* + 1 NULL term*/
    sprintf((char*)processor->writeBuffer,
        POSTFORMAT, path, sc->serverName);
    if (authheader)
        strcat((char*)processor->writeBuffer, authheader);

    strcat((char*)processor->writeBuffer, CONLEN);
    strcat((char*)processor->writeBuffer, num);
    strcat((char*)processor->writeBuffer, "\r\n\r\n");
    
    len = strlen((char*)processor->writeBuffer);

#if defined(_WIN32) && !defined(_WIN32_WCE)
    if (processor->logConnectionInfo)
        proxyDebug(processor, ">>> ---- Post String\n%s>>>>----\n", 
                   processor->writeBuffer);
#endif

    AGSyncProcessorSetSendDataFunc(&processor->syncProcessor, 
                                   NULL, 
                                   NULL);
    
    AGSyncProcessorSendBuffer(&processor->syncProcessor, 
                              (uint8 *)processor->writeBuffer, len);

    processor->state = SEND_HEADER;
    return;
}


static void stateChangeToSENDBUFFERLOGON(AGClientProcessor *processor)
{
    AGSyncProcessorSendBuffer(&processor->syncProcessor, 
        (uint8 *)AGBufferWriterGetBuffer(processor->logonBufferWriter),
        AGBufferWriterGetBufferSize(processor->logonBufferWriter));
    processor->state = SEND_BUFFER_LOGON;
}

static void stateChangeToHELLOForReal(AGClientProcessor *processor)
{
    if(processor->logonBufferWriter) {
        AGBufferWriterFree(processor->logonBufferWriter);
    }
    processor->logonBufferWriter = NULL;
    processor->calcBufferPass = FALSE;
    stateChangeToHELLO(processor);
}

static void stateChangeToPROCESSCMD(AGClientProcessor *processor)
{
    // only get here when processor->bufferServerCommands
    processor->serverCommandReader = AGBufferReaderNew((uint8 *)processor->writer.buffer);
    processor->state = PROCESSING_BUFFERED_CMDS;
}

static int32 processCMDS(AGClientProcessor *processor)
{
    int32 result = AGCLIENT_ERR;
    int32 errCode;

    if(processor->platformCalls->performCommandFunc == NULL) { 
        if(processor->serverCommandReader)
            AGBufferReaderFree(processor->serverCommandReader);
        processor->serverCommandReader = NULL;
        return result;
    }

    result = (*processor->platformCalls->performCommandFunc)(
                        processor->platformCalls->performCommandOut, 
                        &errCode,
                        (AGReader *)processor->serverCommandReader);

    if(result != AGCLIENT_CONTINUE) {
        if(processor->serverCommandReader)
            AGBufferReaderFree(processor->serverCommandReader);
        processor->serverCommandReader = NULL;
    }
    return result;
}

static int32 callPerformCommand(AGClientProcessor *processor)
{
    AGBufferReader reader;
    int32 result = AGCLIENT_ERR;
    int32 errCode;

    if(processor->platformCalls->performCommandFunc == NULL)
        return result;

    AGBufferReaderInit(&reader, 
            AGSyncProcessorGetCommandBuffer(&processor->syncProcessor));

    result = (*processor->platformCalls->performCommandFunc)(
                        processor->platformCalls->performCommandOut, 
                        &errCode,
                        (AGReader *)&reader);
    
    AGBufferReaderFinalize(&reader);
    return result;
}

static int32 processCommand(AGClientProcessor *processor)
{
    AGBufferReader reader;
    int32 result = AGCLIENT_ERR;
    int32 bytesToWrite = 0;
    int32 command;

    if(!processor->bufferServerCommands) {
        return callPerformCommand(processor);
    }

    command = processor->syncProcessor.command;
    if(command == AG_TASK_CMD) {
        char *taskName = NULL;

        AGBufferReaderInit(&reader, 
                AGSyncProcessorGetCommandBuffer(&processor->syncProcessor));
        AGReadCompactInt((AGReader *)&reader); // strip the command
        AGReadCompactInt((AGReader *)&reader); // strip the commandlength
        AGReadTASK((AGReader *)&reader, &taskName, &processor->taskIsBufferable);
        if(taskName)
            free(taskName);
        AGBufferReaderFinalize(&reader);
    }

    // If this is a task or item that cannot be buffered, we
    // always want to call the performCommand immediately, and
    // not append this task or command to the buffer.
    //
    // TASK and ITEM will either be displayed directly to the
    // user in the case where these items are server progress information
    // or they will be buffered and displayed while the client is processing
    // the real commands.
    if((command == AG_TASK_CMD 
        || command == AG_ITEM_CMD)
        && !processor->taskIsBufferable) {
            return callPerformCommand(processor);
    }

    // Write the command into the command buffer for processing later
    bytesToWrite += AGCompactSize(command);
    bytesToWrite += AGCompactSize(processor->syncProcessor.commandLen);
    bytesToWrite += processor->syncProcessor.commandLen;
    AGWriteBytes((AGWriter *)&processor->writer, 
                    (uint8 *)processor->syncProcessor.buffer,
                    bytesToWrite);
    if(command != AG_END_CMD)
        result = AGCLIENT_CONTINUE;
    else
        result = AGCLIENT_IDLE;

    return result; 
}

static int32 processNotComplete(AGClientProcessor *processor, int32 rc,
                                int32 maxRetries, int32 retryFailStringId)
{
    if(rc == AGSYNC_ERR) {
        processor->errStringId = processor->syncProcessor.errStringId;
        processor->state = PROBLEM;
    } 
    
    return AGCLIENT_CONTINUE;
}

// Code within _WIN32 ifdef is multithreaded & platform-specific.
#ifdef _WIN32

/* This function runs in the SECOND thread.  It's called eventually by
   AGClientProcessorProcess(), which runs in the SECOND thread. */
static int32 queueCommand(AGClientProcessor *processor)
{
    int32 result = AGCLIENT_ERR;
    int32 bytesToWrite = 0;

    // Add the command into the buffer.
    WaitForSingleObject(processor->mutex, INFINITE);
    bytesToWrite += AGCompactSize(processor->syncProcessor.command);
    bytesToWrite += AGCompactSize(processor->syncProcessor.commandLen);
    bytesToWrite += processor->syncProcessor.commandLen;
    AGWriteBytes((AGWriter *)processor->threadWriter,
        (uint8 *)processor->syncProcessor.buffer,
        bytesToWrite);
    processor->writeIndex =
        AGBufferWriterGetBufferSize(processor->threadWriter);

    // Check the error code and see whether we've gotten into a problem
    // state. pending(miket) figure out how to do this.
    if(0) {

        result = AGCLIENT_ERR;
        processor->finished = TRUE;

    } else {

        if(processor->syncProcessor.command != AG_END_CMD)
            result = AGCLIENT_CONTINUE;
        else {
            processor->finished = TRUE;
            result = AGCLIENT_IDLE;
        }

    }

    ReleaseMutex(processor->mutex);

    return result; 
}

/* This function runs in the FIRST thread (the one that actually executes
   commands). */
static int32 dispatchQueuedCommand(AGClientProcessor *processor)
{
    AGBufferReader reader;
    int32 result = AGCLIENT_ERR;
    int32 command;
    int32 length;
    int32 fulllength;
    int32 errCode;
    uint8 *localCommand = NULL;

    // Pull a command off the queue.
    WaitForSingleObject(processor->mutex, INFINITE);

    AGBufferReaderInit(&reader,
        AGBufferWriterGetBuffer(processor->threadWriter)
            + processor->readIndex);
    command = AGReadCompactInt((AGReader *)&reader);
    length = AGReadCompactInt((AGReader *)&reader);
    fulllength = AGCompactSize(command)
        + AGCompactSize(length)
        + length;
    localCommand = malloc(fulllength);
    if (localCommand) {

        /* Make a copy of the complete MAL command (command, command length,
        and command data). I know there's a better way to do it than the
        scary code you see here, but it's late, I'm tired, it works, it works,
        it works... */

        memcpy(localCommand,
            AGBufferWriterGetBuffer(processor->threadWriter)
                + processor->readIndex,
            fulllength - length);

        AGReadBytes((AGReader *)&reader,
            &localCommand[fulllength - length],
            length);
    }
    processor->readIndex += fulllength;
    AGBufferReaderFinalize(&reader);

    ReleaseMutex(processor->mutex);

    // Dispatch the command.
    if(processor->platformCalls->performCommandFunc != NULL) {

        AGBufferReaderInit(&reader, localCommand);

        result = (*processor->platformCalls->performCommandFunc)(
                            processor->platformCalls->performCommandOut, 
                            &errCode,
                            (AGReader *)&reader);

        AGBufferReaderFinalize(&reader);

    }

    if(localCommand)
        free(localCommand);

    return result;

}

/* This function runs in the FIRST thread.  It waits for commands to
   appear in the queue, and then causes them to be executed. */
int32 AGClientProcessorBeginCommandDispatcher(AGClientProcessor *processor,
                                              void (* timeSliceFunc)(void),
                                              AGBool * cancel)
{
    AGBool go = TRUE;

    while(go) {
        
        int32 diff;
        AGBool finished;

        /* Compare the point in the command buffer where we're ready to
        start executing commands to the point in it where further commands
        end. */
        WaitForSingleObject(processor->mutex, INFINITE);
        diff = processor->readIndex - processor->writeIndex;
        finished = processor->finished;
        ReleaseMutex(processor->mutex);

        /* If the caller has given us a cancel flag, check it. */
        if (cancel && *cancel) {
            go = FALSE;
            continue;
        }

        /* If we've been signaled that no more commands will be added to the
        buffer, and if we've executed all the commands that were already in
        it, then we're done. */
        if(finished && (diff >= 0)) {
            go = FALSE;
        } else {
            /* If we haven't executed all commands in the buffer, then do
            the next one. */
            if(diff < 0) {
                dispatchQueuedCommand(processor);
            } else {
                /* There were no commands to execute, but we're supposed to
                see more, so give up our time slice and check again later.
                Implementation note:  I know this code isn't portable but
                I used the portable sleep function anyway. */
                AGSleepMillis(0);
            }
        }

        if(timeSliceFunc)
            timeSliceFunc();
    }

    return 0;
}

#endif // #ifdef _WIN32
