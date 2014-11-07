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

#ifndef __AGCOMMANDPROCESSOR_H__
#define __AGCOMMANDPROCESSOR_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGReader.h>
#include <AGServerConfig.h>
#include <AGDeviceInfo.h>
#include <AGDBConfig.h>
#include <AGRecord.h>
#include <AGClientProcessor.h>
#include <AGLocationConfig.h>
#include <AGUserConfig.h>
#include <AGProtocol.h>

typedef struct AGPlatformCommands {
    void *out;
    AGPerformTaskFunc 				performTaskFunc;
    AGPerformItemFunc 				performItemFunc;
    AGPerformDeleteDatabaseFunc 	performDeleteDatabaseFunc;
    AGPerformOpenDatabaseFunc 		performOpenDatabaseFunc;
    AGPerformCloseDatabaseFunc 		performCloseDatabaseFunc;
    AGPerformClearModsFunc 			performClearModsFunc;
    AGPerformGoodbyeFunc 			performGoodbyeFunc;
    AGPerformRecordFunc 			performRecordFunc;
    AGPerformExpansionFunc 			performExpansionFunc;
    AGPerformExpansionResourceFunc  performExpansionResourceFunc;
    AGPerformExpansionChSCFunc      performExpansionChSCFunc;

    AGPerformEndFunc				performEndFunc;
//	AGPerformSendDeviceInfoFunc  	performSendDeviceInfoFunc;
//	AGPerformDatabaseConfigFunc  	performDatabaseConfigFunc;
//	AGPerformServerConfigFunc 		performServerConfigFunc;
//	AGPerformCookieFunc 			performCookieFunc;
//	AGPerformNonceFunc 				performNonceFunc;
} AGPlatformCommands;

typedef struct AGCommandProcessor {
    AGPlatformCommands commands;
    AGServerConfig *serverConfig;
    AGDBConfig *currentDb;
    AGBool syncAgain;
} AGCommandProcessor;

ExportFunc AGCommandProcessor *AGCommandProcessorNew(AGServerConfig *server);
ExportFunc AGCommandProcessor *AGCommandProcessorInit(
                                            AGCommandProcessor *processor,
                                            AGServerConfig *server);
ExportFunc void AGCommandProcessorFinalize(AGCommandProcessor *processor);
ExportFunc void AGCommandProcessorFree(AGCommandProcessor *processor);

ExportFunc int32 AGCommandProcessorStart(AGCommandProcessor *processor);
ExportFunc AGBool AGCommandProcessorShouldSyncAgain(
                                        AGCommandProcessor *processor);

ExportFunc AGPerformCommandFunc AGCommandProcessorGetPerformFunc(
                                                AGCommandProcessor *processor);

ExportFunc int32 AGCPPerformCommand(AGCommandProcessor *processor, 
                                            int32 *errCode,
                                            AGReader *reader);


ExportFunc int32 AGCPEnd(AGCommandProcessor *out, 
                                       int32 *returnErrorCode);
ExportFunc int32 AGCPSendDeviceInfo(AGCommandProcessor *out, 
                                             int32 *returnErrorCode,
                                             AGBool send);
ExportFunc int32 AGCPDatabaseConfig(AGCommandProcessor *out, 
                                                 int32 *returnErrorCode,
                                                 char *dbname, 
                                                 AGDBConfigType config, 
                                                 AGBool sendRecordPlatformData, 
                                                 int32 platformDataLength, 
                                                 void *platformData);
ExportFunc int32 AGCPServerConfig(AGCommandProcessor *out, 
                                  int32 *returnErrorCode,
                                  char *friendlyName, 
                                  char *userUrl,
                                  char *message,
                                  char *serverUri, 
                                  AGBool clientShouldHashPasswords, 
                                  AGBool allowSecureClientConnect, 
                                  uint32 connectTimeout, 
                                  uint32 writeTimeout, 
                                  uint32 readTimeout);
ExportFunc int32 AGCPCookie(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                     int32 cookieLength,
                                     void *cookie);
ExportFunc int32 AGCPNonce(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                    uint8 nonce[16]);
ExportFunc int32 AGCPTask(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                    char *currentTask,
                                    AGBool bufferable);
ExportFunc int32 AGCPItem(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                   int32 currentItemNumber,
                                   int32 totalItemCount,
                                   char *currentItem);
ExportFunc int32 AGCPDeleteDatabase(AGCommandProcessor *out, 
                                            int32 *returnErrorCode,
                                             char *dbname);
ExportFunc int32 AGCPOpenDatabase(AGCommandProcessor *out, 
                                            int32 *returnErrorCode,
                                           char *dbname);
ExportFunc int32 AGCPCloseDatabase(AGCommandProcessor *out, 
                                                    int32 *returnErrorCode);
ExportFunc int32 AGCPClearMods(AGCommandProcessor *out, 
                                                    int32 *returnErrorCode);
ExportFunc int32 AGCPGoodbye(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                      AGSyncStatus syncStatus,
                                      int32 errorCode,
                                      char *errorMessage);
ExportFunc int32 AGCPRecord(AGCommandProcessor *out, 
                                     int32 *returnErrorCode,
                                     int32 *newUID,
                                     int32 uid,
                                     AGRecordStatus mod,
                                     int32 recordDataLength,
                                     void *recordData,
                                     int32 platformDataLength,
                                     void *platformData);

ExportFunc int32 AGCPExpansion(AGCommandProcessor *out, int32 *returnErrorCode,
                                        int32 expansionCommand, 
                                        int32 commandLength,
                                        void *commandBytes);
ExportFunc int32 AGCPExpansionResource(AGCommandProcessor *out,
                                       int32 *returnErrorCode,
                                       int32 type, 
                                       int32 resourceLen,
                                       void *resource);
ExportFunc int32 AGCPExpansionChangeServerConfig(AGCommandProcessor *out,
                                                 int32 *returnErrorCode,
                                                 AGBool disableServer,
                                                 int32 flags,
                                                 char *serverName,
                                                 int16 serverPort,
                                                 char *userName,
                                                 int32 passwordLen,
                                                 uint8 *password,
                                                 AGBool connectSecurely,
                                                 AGBool notRemovable);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __AGCOMMANDPROCESSOR_H__ */


