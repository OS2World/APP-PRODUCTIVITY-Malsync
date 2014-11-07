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
    The AGClientProcessor is able to connect and send logon commands to a
    MAL server. It hides the complexity of dealing with the network in a
    non-blocking manner. 

    You configure a AGClientProcessor with the appropriate information needed
    to connect to a specific server, ie servername, port, username, etc.

    Sometimes the logon process involves sending records up to the server. In
    this case, there is a callback function that the AGClientProcessor will
    make to ask for the records. The caller is responsible for properly
    handling these requests through the AGGetNextModifiedRecordFunc and
    AGGetNextRecordFunc functions. 

    Additionally, the AGClientProcessor has no knowledge of the commands that
    are being sent from the server. It does have knowledge of the general format
    commands so it can get them properly off the network, but will send
    this data to another callback, AGPerformCommandFunc, to actually perform the
    command action. The return value of this callback determines if the 
    AGClientProcessor continues processing the next command.

*/

#ifndef __AGCLIENTPROCESSOR_H__
#define __AGCLIENTPROCESSOR_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGSyncProcessor.h>
#include <AGBufferWriter.h>
#include <AGBufferReader.h>
#include <AGMsg.h>
#include <AGReader.h>
#include <AGServerConfig.h>
#include <AGDeviceInfo.h>
#include <AGDBConfig.h>
#include <AGRecord.h>
#include <AGLocationConfig.h>
    
typedef enum {
    /* Processor has completed the last requested action, or has nothing to do */
    AGCLIENT_IDLE = 0,
    /* Processor still has work to do to complete the last action */
    AGCLIENT_CONTINUE,
    /* An error has occured during the processing, check the errStringId */
    AGCLIENT_ERR
} AGClientProcessResult;

/* These are returned by the callbacks
 to describe the error encountered.*/
typedef enum {
    AGCLIENT_NO_ERR = 0,
    AGCLIENT_OPEN_ERR,
    AGCLIENT_READ_ERR,
    AGCLIENT_UNKNOWN_ERR
} AGClientError;

/* 
 This function is called when the client receives a command from the server.
 It is up the the client to properly parse and interpret these command
 buffers from the server. The AGReader will contain a complete command, so
 that the read calls don't block because of the network.

 inputs -
    out - pointer that was passed into the AGClientProcessorInit() method
    commandToProcess - an AGReader containing the command buffer
 outputs - 
    errCode - if you return AGCLIENT_ERR you should fill in the errCode
 return value:
    AGCLIENT_IDLE: if there are no more commands to be handled
    AGCLIENT_CONTINUE: if there are more commands to be handled
    AGCLIENT_ERR: If there was some problem with the command 
                  and you want to stop processing commands
*/
typedef int32 (*AGPerformCommandFunc)(void *out, int32 *errCode,
                                        AGReader *commandToProcess);
/*
  This function is called prior to making the AGGetNextModifiedRecordFunc
  or AGGetNextRecordFunc calls for a particular database. 
  We will open the database and ask for all the records sequentially.
  You will not have to have mutliple databases open at a single time.
  There is no corresponding close database function. You should close
  the database when one of the readRecord functions returns no more 
  records. 
*/
typedef int32 (*AGOpenDatabaseFunc)(void *out,
                                    AGDBConfig *theDatabase, 
                                    int32 *errCode);

/* 
 These functions are called during the logon process to get records that
 need to be sent up to the server. It will be interating over the AGDBConfig
 records from the AGServerConfig structure. 

 The AGGetNextModifiedRecordFunc function will be called when the AGDBConfig
 is set to only send modified records. The AGGetNextRecordFunc will be called
 when the AGDBConfig is set to send all the records.
 
 inputs -
    out - pointer that was passed into the AGClientProcessorInit() method
 outputs -
    record - the record to be sent
    errCode - is you return AGCLIENT_ERR you should fill in the errCode
 return value:
      AGCLIENT_IDLE (+ record == NULL) when there are no more records
      AGCLIENT_CONTINUE (+ record != NULL) when there is a record
      AGCLIENT_ERR (+ record == NULL) if there is an err and you want to stop
*/
typedef int32 (*AGGetNextModifiedRecordFunc)(void *out, 
                                        AGRecord **record,
                                        int32 *errCode);
typedef int32 (*AGGetNextRecordFunc)(void *out, 
                                        AGRecord **record,
                                        int32 *errCode);

typedef int32 (*AGGetNextExpansionCommandFunc)(void *out,
                                               int32 *newCommand,
                                               int32 *commandLength,
                                               void **commandBytes);
                                            

typedef struct AGPlatformCalls {
    void *out;
    AGGetNextModifiedRecordFunc nextModifiedRecordFunc;
    AGGetNextRecordFunc nextRecordFunc;
    AGOpenDatabaseFunc openDatabaseFunc;
    AGGetNextExpansionCommandFunc nextExpansionCommandFunc;

    void *performCommandOut;
    AGPerformCommandFunc performCommandFunc;
} AGPlatformCalls;

typedef struct AGClientProcessor {
    AGServerConfig *serverInfo;
    AGDeviceInfo *deviceInfo;
    AGLocationConfig *lc;
    AGPlatformCalls *platformCalls;
    AGBool bufferCommands;
    AGBool calcBufferPass;
    AGBool bufferServerCommands;
    AGBool pingRequest;
    AGBool taskIsBufferable;
    
    int16 state;
    uint32 errStringId;

    int32 dbConfigIndex;
    AGBool sentOPEN;
    void *writeBuffer;

    AGBool writerInited;
    AGBufferWriter writer;
    AGBufferWriter *logonBufferWriter;
    AGSyncProcessor syncProcessor;
    AGBufferReader *serverCommandReader;

#ifdef _WIN32
    HANDLE threadHandle;
    HANDLE mutex;
    uint32 readIndex, writeIndex;
    AGBool finished;
    AGBufferWriter * threadWriter;
    AGBool logConnectionInfo;
    HANDLE logMutex;
    uint8 *debugBuffer;
#endif // #ifdef _WIN32

} AGClientProcessor;

ExportFunc
AGClientProcessor *AGClientProcessorNew(AGServerConfig *serverInfo,
                                        AGDeviceInfo *deviceInfo,
                                        AGLocationConfig *lc,
                                        AGPlatformCalls *platformCalls,
                                        AGBool bufferCommands,
                                        AGNetCtx *netctx);
ExportFunc
AGClientProcessor *AGClientProcessorInit(AGClientProcessor *processor, 
                                         AGServerConfig *serverInfo,
                                         AGDeviceInfo *deviceInfo,
                                         AGLocationConfig *lc,
                                         AGPlatformCalls *platformCalls,
                                         AGBool bufferCommands,
                                         AGNetCtx *netctx);

ExportFunc void AGClientProcessorSetBufferServerCommands(
                                         AGClientProcessor *processor,
                                         AGBool buffer);
    
ExportFunc void AGClientProcessorFinalize(AGClientProcessor *processor);
ExportFunc void AGClientProcessorFree(AGClientProcessor *processor);

ExportFunc void AGClientProcessorPing(AGClientProcessor *processor);
ExportFunc void AGClientProcessorSync(AGClientProcessor *processor);
ExportFunc int32 AGClientProcessorProcess(AGClientProcessor *processor);
ExportFunc
int32 AGClientProcessorBeginCommandDispatcher(AGClientProcessor *processor,
                                              void (* timeSliceFunc)(void),
                                              AGBool * cancel);
#define MAX_SYNCS (10)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGCLIENTPROCESSOR_H__ */
