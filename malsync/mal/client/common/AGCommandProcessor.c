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

#include <AGCommandProcessor.h>
#include <AGUtil.h>
#include <AGDigest.h>

ExportFunc AGCommandProcessor *AGCommandProcessorNew(AGServerConfig *server)
{
    AGCommandProcessor *tmp;
    tmp = (AGCommandProcessor *)malloc(sizeof(AGCommandProcessor));
    if(tmp)
        AGCommandProcessorInit(tmp, server);
    return tmp;
}

ExportFunc AGCommandProcessor *AGCommandProcessorInit(
                                            AGCommandProcessor *processor,
                                            AGServerConfig *server)
{
    bzero(processor, sizeof(AGCommandProcessor));
    processor->serverConfig = server;
    return processor;
}

ExportFunc void AGCommandProcessorFinalize(AGCommandProcessor *processor)
{
    bzero(processor, sizeof(AGCommandProcessor));
}

ExportFunc void AGCommandProcessorFree(AGCommandProcessor *processor)
{
    AGCommandProcessorFinalize(processor);
    free(processor);
}

ExportFunc int32 AGCommandProcessorStart(AGCommandProcessor *processor)
{
    processor->syncAgain = FALSE;
    return 0;
}

ExportFunc AGBool AGCommandProcessorShouldSyncAgain(
                                                AGCommandProcessor *processor)
{
    return processor->syncAgain;
}

ExportFunc AGPerformCommandFunc AGCommandProcessorGetPerformFunc(
                                                AGCommandProcessor *processor)
{
    return (AGPerformCommandFunc)&AGCPPerformCommand;
}

static int32 parseDATABASECONFIG(void *out, AGReader *r, 
                                                int32 len, int32 *errCode)
{
    char *dbname;
    AGBool sendRecordPlatformData;
    AGDBConfigType config;
    int32 platformDataLength;
    void *platformData;
    int32 result;

    AGReadDATABASECONFIG(r, &dbname, 
                            &config, 
                            &sendRecordPlatformData, 
                            &platformDataLength, 
                            &platformData);

    result = AGCPDatabaseConfig((AGCommandProcessor *)out, errCode, 
                                dbname, config, sendRecordPlatformData,
                                platformDataLength, platformData);
    if(dbname)
        free(dbname);
    if(platformDataLength)
        free(platformData);
    return result;
}

ExportFunc int32 AGCPDatabaseConfig(AGCommandProcessor *out, 
                                     int32 *returnErrorCode,
                                     char *dbname, 
                                     AGDBConfigType config, 
                                     AGBool sendRecordPlatformData, 
                                     int32 platformDataLength,
                                     void *platformData)
{
    AGDBConfig *dbconfig = NULL;
    void *tmp = NULL;

    if(!dbname) {
        return AGCLIENT_ERR;
    }

    if(config == AG_DONTSEND_CFG) {
        dbconfig = AGServerConfigDeleteDBConfigNamed(out->serverConfig, dbname);
        if(dbconfig)
            AGDBConfigFree(dbconfig);
    } else {
        if(platformDataLength) {
            tmp = malloc(platformDataLength);
            bcopy(platformData, tmp, platformDataLength);
        }
    
        dbconfig = AGDBConfigNew(strdup(dbname), config, 
                                sendRecordPlatformData, 
                                platformDataLength, tmp, NULL);
        AGServerConfigAddDBConfig(out->serverConfig, dbconfig);
    }
    return AGCLIENT_CONTINUE;
}

static int32 parseSERVERCONFIG(void *out, AGReader *r, 
                                int32 len, int32 *errCode)
{
    char *friendlyName = NULL, *userUrl = NULL, *message = NULL, 
        *serverUri = NULL;
    AGBool clientShouldHashPasswords, allowSecureClientConnect;
    uint32 connectTimeout, writeTimeout, readTimeout;
    int32 result;

    AGReadSERVERCONFIG(r, &friendlyName, &userUrl,  &message, &serverUri,
                       &clientShouldHashPasswords,
                       &allowSecureClientConnect,
                       &connectTimeout,
                       &writeTimeout, 
                       &readTimeout);

    result = AGCPServerConfig((AGCommandProcessor *)out, 
                            errCode,
                            friendlyName, 
                            userUrl,
                            message,
                            serverUri, 
                            clientShouldHashPasswords, 
                            allowSecureClientConnect, 
                            connectTimeout, 
                            writeTimeout,
                            readTimeout);
    if (friendlyName)
        free(friendlyName);
    if (userUrl)
        free(userUrl);
    if (message)
        free(message);
    if (serverUri)
        free(serverUri);
    return result;
}

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
                                  uint32 readTimeout)
{
    if(!out->serverConfig) {
        return AGCLIENT_ERR;    
    }

    if (out->serverConfig->friendlyName)
        free(out->serverConfig->friendlyName);
    if (out->serverConfig->userUrl)
        free(out->serverConfig->userUrl);
    if (out->serverConfig->description)
        free(out->serverConfig->description);
    if (out->serverConfig->serverUri)
        free(out->serverConfig->serverUri);
    out->serverConfig->friendlyName = NULL;
    out->serverConfig->userUrl = NULL;
    out->serverConfig->description = NULL;
    out->serverConfig->serverUri = NULL;

    if (friendlyName)
        out->serverConfig->friendlyName = strdup(friendlyName);
    if(userUrl)
        out->serverConfig->userUrl = strdup(userUrl);
    if(message)
       out->serverConfig->description = strdup(message);
    if(serverUri)
       out->serverConfig->serverUri = strdup(serverUri);
    AGServerConfigChangeHashPasswordState(out->serverConfig,
        (uint8)(clientShouldHashPasswords
        ? AG_HASH_PASSWORD_YES
        : AG_HASH_PASSWORD_NO));
    out->serverConfig->allowSecureConnection = allowSecureClientConnect;
    out->serverConfig->connectTimeout = connectTimeout;
    out->serverConfig->writeTimeout = writeTimeout;
    out->serverConfig->readTimeout = readTimeout;
    return AGCLIENT_CONTINUE;
}

static int32 parseCOOKIE(void *out, AGReader *r, int32 len, int32 *errCode)
{
    int32 cookieLen = 0, result;
    void *cookie;

    AGReadCOOKIE(r, &cookieLen, &cookie);
    result = AGCPCookie((AGCommandProcessor *)out, errCode,
                        cookieLen, cookie);
    if(cookieLen)
        free(cookie);
    return result;
}

ExportFunc int32 AGCPCookie(AGCommandProcessor *out, 
                                     int32 *returnErrorCode,
                                     int32 cookieLength,
                                     void *cookie)
{
    void *tmp = NULL;

    if(!out->serverConfig) {
        return AGCLIENT_ERR;
    }

    if(out->serverConfig->sequenceCookie != NULL) {
        free(out->serverConfig->sequenceCookie);
        out->serverConfig->sequenceCookie = NULL;
        out->serverConfig->sequenceCookieLength = 0;
    }
    
    if(cookieLength) {
        tmp = malloc(cookieLength);
        bcopy(cookie, tmp, cookieLength);
    }
    out->serverConfig->sequenceCookie = (uint8*)tmp;
    out->serverConfig->sequenceCookieLength = cookieLength;
    return AGCLIENT_CONTINUE;
}

static int32 parseNONCE(void *out, AGReader *r, int32 len, int32 *errCode)
{
    uint8 nonce[16];

    AGReadNONCE(r, nonce);
    return AGCPNonce((AGCommandProcessor *)out, errCode, nonce);
}

ExportFunc int32 AGCPNonce(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                    uint8 nonce[16])
{
    if(!out->serverConfig)
        return AGCLIENT_ERR;

    if(AG_HASH_PASSWORD_UNKNOWN == out->serverConfig->hashPassword) {
        AGServerConfigChangeHashPasswordState(out->serverConfig,
            (uint8)(AGDigestNull(nonce)
            ? AG_HASH_PASSWORD_NO
            : AG_HASH_PASSWORD_YES));
    }

    bcopy(nonce, out->serverConfig->nonce, 16);
    return AGCLIENT_CONTINUE;
}

static int32 parseTASK(void *out, AGReader *r, int32 len, int32 *errCode)
{
    char *currentTask = NULL;
    int32 result;
    AGBool bufferable = FALSE;

    AGReadTASK(r, &currentTask, &bufferable);
    result = AGCPTask((AGCommandProcessor *)out, errCode, currentTask, bufferable);
    if(currentTask)
        free(currentTask);
    return result;
}

ExportFunc int32 AGCPTask(AGCommandProcessor *out, 
                                    int32 *returnErrorCode,
                                    char *currentTask,
                                    AGBool bufferable)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performTaskFunc) {
        rc = (*out->commands.performTaskFunc)(out->commands.out, 
                                        returnErrorCode, currentTask, bufferable);
    }
    return rc;
}


static int32 parseITEM(void *out, AGReader *r, int32 len, int32 *errCode)
{
    int32 currentItemNumber = 0;
    int32 totalItemCount = 0;
    char *currentItem = NULL;
    int32 result;

    AGReadITEM(r, &currentItemNumber, &totalItemCount, &currentItem);
    result = AGCPItem((AGCommandProcessor *)out, errCode,
                        currentItemNumber,
                        totalItemCount, 
                        currentItem);
    if(currentItem)
        free(currentItem);
    return result;
}

ExportFunc int32 AGCPItem(AGCommandProcessor *out, 
                            int32 *returnErrorCode,
                            int32 currentItemNumber,
                            int32 totalItemCount,
                            char *currentItem)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performItemFunc) {
        rc = (*out->commands.performItemFunc)(out->commands.out, returnErrorCode, 
                                currentItemNumber, totalItemCount, currentItem);
    }
    return rc;
}

static int32 parseDELETEDATABASE(void *out, AGReader *r,
                                    int32 len, int32 *errCode)
{
    char *dbname;
    int32 result;

    AGReadDELETEDATABASE(r, &dbname);
    result = AGCPDeleteDatabase((AGCommandProcessor *)out, errCode, dbname);
    if(dbname)
        free(dbname);
    return result;
}

ExportFunc int32 AGCPDeleteDatabase(AGCommandProcessor *out, 
                                            int32 *returnErrorCode,
                                             char *dbname)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performDeleteDatabaseFunc) {
        rc = (*out->commands.performDeleteDatabaseFunc)(out->commands.out, 
                                                    returnErrorCode, dbname);
    }
    return rc;
}


static int32 parseOPENDATABASE(void *out, AGReader *r, int32 len, int32 *errCode)
{
    char *dbname = NULL;
    int32 result;

    AGReadOPENDATABASE(r, &dbname);
    result = AGCPOpenDatabase((AGCommandProcessor *)out, errCode, dbname);
    if(dbname)
       free(dbname);
    return result;
}

ExportFunc int32 AGCPOpenDatabase(AGCommandProcessor *out, 
                                            int32 *returnErrorCode,
                                            char *dbname)
{
    int32 rc = AGCLIENT_CONTINUE;

    out->currentDb = AGServerConfigGetDBConfigNamed(out->serverConfig,
                                                            dbname);
    if(!out->currentDb)
        return rc;

    if(out->commands.performOpenDatabaseFunc) {
        rc = (*out->commands.performOpenDatabaseFunc)(out->commands.out, 
                                                    returnErrorCode, dbname);
    }

    // if we're getting an openDB from the server, that must mean
    // that they've gotten our last set of newids. Clear them now,
    // so that subsequent RECR commands can add them properly.
    if(out->currentDb) {
        AGDBConfigSetNewIds(out->currentDb, NULL);
    }
    return rc;
}


static int32 parseCLOSEDATABASE(void *out, AGReader *r, int32 len, int32 *errCode)
{
    AGReadCLOSEDATABASE(r);
    return AGCPCloseDatabase((AGCommandProcessor *)out, errCode);
}

ExportFunc int32 AGCPCloseDatabase(AGCommandProcessor *out, 
                                                    int32 *returnErrorCode)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performCloseDatabaseFunc) {
        rc = (*out->commands.performCloseDatabaseFunc)(out->commands.out, 
                                                            returnErrorCode);
    }
    return rc;
}

static int32 parseCLEARMODS(void *out, AGReader *r, int32 len, int32 *errCode)
{
    AGReadCLEARMODS(r);
    return AGCPClearMods((AGCommandProcessor *)out, errCode);
}

ExportFunc int32 AGCPClearMods(AGCommandProcessor *out, 
                                                    int32 *returnErrorCode)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performClearModsFunc) {
        rc = (*out->commands.performClearModsFunc)(out->commands.out, 
                                                            returnErrorCode);
    }
    return rc;
}

static int32 parseGOODBYE(void *out, AGReader *r, int32 len, int32 *errCode)
{
    AGSyncStatus syncStatus = AG_DONE_STATUS;
    int32 errorCode = 0, result;
    char *errorMessage = NULL;

    AGReadGOODBYE(r, &syncStatus, &errorCode, &errorMessage);
    result = AGCPGoodbye((AGCommandProcessor *)out, errCode, syncStatus, 
                    errorCode, errorMessage);
    if(errorMessage)
       free(errorMessage);
    return result;
}

ExportFunc int32 AGCPGoodbye(AGCommandProcessor *out, 
                                int32 *returnErrorCode,
                                AGSyncStatus syncStatus,
                                int32 errorCode,
                                char *errorMessage)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performGoodbyeFunc) {
        rc = (*out->commands.performGoodbyeFunc)(out->commands.out, 
                                returnErrorCode,
                                syncStatus, errorCode, errorMessage);
    }

    if(syncStatus == AG_CALLAGAIN_STATUS)
        out->syncAgain = TRUE;
    return rc;
}

static int32 parseSENDDEVICEINFO(void *out, AGReader *r, 
                                    int32 len, int32 *errCode)
{
    AGBool send;

    AGReadSENDDEVICEINFO(r, &send);
    return AGCPSendDeviceInfo((AGCommandProcessor *)out, errCode, send);
}

ExportFunc int32 AGCPSendDeviceInfo(AGCommandProcessor *out, 
                                             int32 *returnErrorCode,
                                             AGBool send)
{
    out->serverConfig->sendDeviceInfo = send;
    return AGCLIENT_CONTINUE;
}

static int32 parseRECORD(void *out, AGReader *r, int32 len, int32 *errCode)
{
    int32 uid, newId = 0;
    AGRecordStatus mod;
    int32 recordDataLength, result;
    void *recordData;
    int32 platformDataLength;
    void *platformData;

#ifndef __palmos__
    AGReadRECORD(r, &uid, &mod,
        &recordDataLength, &recordData,
        &platformDataLength, &platformData);
#else
    AGReadRECORDNoCopy((AGBufferReader *)r, &uid, &mod,
                    &recordDataLength, &recordData,
                    &platformDataLength, &platformData);
#endif
    result = AGCPRecord((AGCommandProcessor *)out, errCode, &newId,
                                     uid,
                                     mod,
                                     recordDataLength,
                                     recordData,
                                     platformDataLength,
                                     platformData);

#ifndef __palmos__
    if(recordData)
       free(recordData);
    if(platformData)
       free(platformData);
#else
    // __palmos__ did not copy the recordData and platformData
    // so don't free it
#endif

    return result;
}

ExportFunc int32 AGCPRecord(AGCommandProcessor *out, 
                                     int32 *returnErrorCode,
                                     int32 *newUID,
                                     int32 uid,
                                     AGRecordStatus mod,
                                     int32 recordDataLength,
                                     void *recordData,
                                     int32 platformDataLength,
                                     void *platformData)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performRecordFunc) {
        rc = (*out->commands.performRecordFunc)(out->commands.out,
                                     returnErrorCode,
                                     newUID,
                                     uid,
                                     mod,
                                     recordDataLength,
                                     recordData,
                                     platformDataLength,
                                     platformData);
    }

    if(mod == AG_RECORD_NEW_TEMPORARY_UID) {
        //NOTE: It needs to record this mapping between the temporary and
        //the real uid to the dbconfig.
        if(out->currentDb) {
            AGDBConfigAppendNewId(out->currentDb, uid, *newUID);
        }
    }
    return rc;
}


static int32 parseEXPANSION(void *out, AGReader *r, int32 len, int32 *errCode)
{
    int32 command, commandLen, result;
    void *buf = NULL;

    AGReadEXPANSION(r, &command, &commandLen, &buf);
    result = AGCPExpansion((AGCommandProcessor *)out, errCode, command, 
                                                commandLen, buf);
    if(buf)
        free(buf);
    return result;
}

static int32 parseEXPANSION_RESOURCE(void *out,
                                     AGReader *r,
                                     int32 len,
                                     int32 *errCode)
{
    int32 type, resourceLen, result;
    void *buf = NULL;

    AGReadEXPANSION_RESOURCE(r, &type, &resourceLen, &buf);
    result = AGCPExpansionResource((AGCommandProcessor *)out,
        errCode,
        type,
        resourceLen,
        buf);
    if(buf)
        free(buf);
    return result;
}

static int32 parseEXPANSION_CHANGESERVERCONFIG(void *out,
                                               AGReader *r,
                                               int32 len,
                                               int32 *errCode)
{
    int32 result;
    AGBool disableServer;
    int32 flags;
    char *serverName;
    int16 serverPort;
    char *userName;
    int32 passwordLen;
    uint8 *password;
    AGBool connectSecurely;
    AGBool notRemovable;

    AGReadEXPANSION_CHANGESERVERCONFIG(r,
        &disableServer,
        &flags,
        &serverName,
        &serverPort,
        &userName,
        &passwordLen,
        &password,
        &connectSecurely,
        &notRemovable);
    result = AGCPExpansionChangeServerConfig((AGCommandProcessor *)out,
        errCode,
        disableServer,
        flags,
        serverName,
        serverPort,
        userName,
        passwordLen,
        password,
        connectSecurely,
        notRemovable);
    if(serverName)
        free(serverName);
    if(userName)
        free(userName);
    if(password)
        free(password);
    return result;
}

static int32 performExpansionCommand(AGCommandProcessor *out, 
                                     int32 *errCode,
                                     int32 expansionCommand, 
                                     int32 commandLength,
                                     AGReader *reader)
{
    int32 rc = AGCLIENT_CONTINUE;

    switch((AGExpansionCommand)expansionCommand) {
    case AG_EXPANSION_RESOURCE:
        rc = parseEXPANSION_RESOURCE(out, reader, commandLength, errCode);
        break;
    case AG_EXPANSION_CHANGESERVERCONFIG:
        rc = parseEXPANSION_CHANGESERVERCONFIG(out,
            reader,
            commandLength,
            errCode);
        break;
    default:
        break; // ok not to understand unknown expansion commands.
    }
    return rc;
}

ExportFunc int32 AGCPExpansion(AGCommandProcessor *out, int32 *returnErrorCode,
                                        int32 expansionCommand, 
                                        int32 commandLength,
                                        void *commandBytes)
{
    int32 rc = AGCLIENT_CONTINUE;
    AGBufferReader * er = NULL;

    if(out->commands.performExpansionFunc) {
        rc = (*out->commands.performExpansionFunc)(out->commands.out,
                                returnErrorCode,
                                expansionCommand,
                                commandLength,
                                commandBytes);
    }

    /* Format of expansion commands is MAL command within MAL command,
    meaning that commandBytes should be interpreted exactly like a normal
    MAL command, with one exception:  only expansion commands should be
    embedded in an EXPANSION command (not non-expansion MAL commands). */

    er = AGBufferReaderNew((uint8*)commandBytes);
    if (NULL != er) {

        rc = performExpansionCommand(out,
            returnErrorCode,
            expansionCommand,
            commandLength,
            (AGReader*)er);

        AGBufferReaderFree(er);

    }

    return rc;
}

ExportFunc int32 AGCPExpansionResource(AGCommandProcessor *out,
                                       int32 *returnErrorCode,
                                       int32 resourceType, 
                                       int32 resourceLen,
                                       void *resource)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performExpansionResourceFunc) {
        rc = (*out->commands.performExpansionResourceFunc)(out->commands.out,
                                returnErrorCode,
                                resourceType,
                                resourceLen,
                                resource);
    }

    switch ((AGResourceType)resourceType) {

    case AG_EXPANSION_RESOURCE_SERVERTYPE: {

        if (NULL != resource) {
            
            /* Todo: probably want to move this outside case when we have
            more than one resource type. */
            AGBufferReader * r = NULL;
        
            /* If we got a SERVERTYPE command, replace the current
            serverType no matter what (even if server sent empty
            string) */
            if (NULL != out->serverConfig->serverType) {
                free(out->serverConfig->serverType);
                out->serverConfig->serverType = NULL;
            }

            r = AGBufferReaderNew((uint8*)resource);
            if (NULL != r) {
                out->serverConfig->serverType = AGReadCString((AGReader*)r);
                AGBufferReaderFree(r);
            }

        }

        break;
    }
    default:
        break; // ok not to understand unknown resources.

    }

    return rc;
}

int32 AGCPExpansionChangeServerConfig(AGCommandProcessor *out,
                                      int32 *returnErrorCode,
                                      AGBool disableServer,
                                      int32 flags,
                                      char *serverName,
                                      int16 serverPort,
                                      char *userName,
                                      int32 passwordLen,
                                      uint8 *password,
                                      AGBool connectSecurely,
                                      AGBool notRemovable)
{
    int32 rc = AGCLIENT_CONTINUE;

    if(out->commands.performExpansionChSCFunc) {
        rc = (*out->commands.performExpansionChSCFunc)(out->commands.out,
                                returnErrorCode,
                                disableServer,
                                flags,
                                serverName,
                                serverPort,
                                userName,
                                passwordLen,
                                password,
                                connectSecurely,
                                notRemovable);
    }

    if (disableServer)
        out->serverConfig->disabled = TRUE;

    if (flags & AG_CHSC_SERVERNAME) {
        if (NULL != out->serverConfig->serverName)
            free(out->serverConfig->serverName);
        out->serverConfig->serverName = (NULL != serverName)
            ? strdup(serverName) : NULL;
    }

    if (flags & AG_CHSC_SERVERPORT)
        out->serverConfig->serverPort = serverPort;

    if (flags & AG_CHSC_USERNAME) {
        if (NULL != out->serverConfig->userName)
            free(out->serverConfig->userName);
        out->serverConfig->userName = (NULL != userName)
            ? strdup(userName) : NULL;
    }

    if (flags & AG_CHSC_PASSWORD) {
        if (AG_HASH_PASSWORD_YES == out->serverConfig->hashPassword) {
            bzero((uint8*)out->serverConfig->password, 16);
            /*
            In this case passwordLen should *always* be 16, but
            for completeness we'll try not to choke if it's something else.
            */
            bcopy(password,
                out->serverConfig->password,
                min(16, passwordLen));

        } else {
            if (NULL != out->serverConfig->cleartextPassword)
                free(out->serverConfig->cleartextPassword);
            out->serverConfig->cleartextPassword = (NULL != password)
                ? strdup((char *) password) : NULL;
        }
    }

    if (flags & AG_CHSC_CONNECTSECURELY)
        out->serverConfig->connectSecurely = connectSecurely;

    if (flags & AG_CHSC_NOTREMOVABLE)
        out->serverConfig->notRemovable = notRemovable;

    return rc;
}

static int32 parseEND(void *out, AGReader *r, int32 len, int32 *errCode)
{
    AGReadEND(r);
    return AGCPEnd((AGCommandProcessor *)out, errCode);
}

ExportFunc int32 AGCPEnd(AGCommandProcessor *out,  int32 *returnErrorCode)
{
    int32 rc = AGCLIENT_IDLE;

    if(out->commands.performEndFunc) {
        rc = (*out->commands.performEndFunc)(out->commands.out, returnErrorCode);
    }
    return rc;
}


ExportFunc int32 AGCPPerformCommand(AGCommandProcessor *out, 
                                            int32 *errCode,
                                            AGReader *reader)
{
    uint32 command;
    uint32 length;
    int32 rc = AGCLIENT_CONTINUE;
#ifdef DEBUG_COMMANDS
    char *name;
#endif
    command = AGReadCompactInt(reader);
    length = AGReadCompactInt(reader);

#ifdef DEBUG_COMMANDS
{
    name = AGProtocolCommandName(command);
    if(name == NULL) {
        name = "UNKNOWN";
    }
        
    printf("%s(%u, %u)\n", name, command, length);
}
#endif

    switch((AGCommand)command) {
    case AG_END_CMD:            
        rc = parseEND(out, reader, length, errCode); break;
    case AG_EXPANSION_CMD:      
        rc = parseEXPANSION(out, reader, length, errCode); break;
    case AG_DATABASECONFIG_CMD: 
        rc = parseDATABASECONFIG(out, reader, length, errCode); break;
    case AG_SERVERCONFIG_CMD:   
        rc = parseSERVERCONFIG(out, reader, length, errCode); break;
    case AG_COOKIE_CMD:         
        rc = parseCOOKIE(out, reader, length, errCode); break;
    case AG_NONCE_CMD:          
        rc = parseNONCE(out, reader, length, errCode); break;
    case AG_TASK_CMD:           
        rc = parseTASK(out, reader, length, errCode); break;
    case AG_ITEM_CMD:           
        rc = parseITEM(out, reader, length, errCode); break;
    case AG_DELETEDATABASE_CMD: 
        rc = parseDELETEDATABASE(out, reader, length, errCode); break;
    case AG_OPENDATABASE_CMD:   
        rc = parseOPENDATABASE(out, reader, length, errCode); break;
    case AG_CLOSEDATABASE_CMD:  
        rc = parseCLOSEDATABASE(out, reader, length, errCode); break;
    case AG_CLEARMODS_CMD:      
        rc = parseCLEARMODS(out, reader, length, errCode); break;
    case AG_RECORD_CMD:         
        rc = parseRECORD(out, reader, length, errCode); break;
    case AG_GOODBYE_CMD:        
        rc = parseGOODBYE(out, reader, length, errCode); break;
    case AG_SENDDEVICEINFO_CMD: 
        rc = parseSENDDEVICEINFO(out, reader, length, errCode); break;

    case AG_HELLO_CMD:      
    case AG_DEVICEINFO_CMD:      
    case AG_NEWIDS_CMD:      
    default:
        #ifdef DEBUG_COMMANDS
        printf("\tThe server should have NEVER sent a %s command.\n", name);
        #endif
        rc = AGCLIENT_ERR; 
        break;
    }
    return rc;
}
