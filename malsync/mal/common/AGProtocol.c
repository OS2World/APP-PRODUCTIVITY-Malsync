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

// Owner: linus

#include <AGProtocol.h>
#include <AGBufferWriter.h>
#include <AGDigest.h>
#include <AGUtil.h>

#if !defined(__palmos__) && !defined(_WIN32_WCE)
//#define DEBUG_PROTOCOL
#endif

                                                           // We do this a lot.
#define ADD_STRING_LEN(str, strLen) { \
    if (str != NULL) {                \
        strLen = strlen(str);         \
    }                                 \
    len += AGCompactSize(strLen);     \
    len += strLen;                    \
}

//PENDING(klobad) convert all the simple commands to MACROS to this guy??
ExportFunc void AGWriteCommand(AGWriter *w, int32 command,
                                int32 commandDataLen,
                                void *commandData)
{

    AGWriteCompactInt(w, command);
    AGWriteCompactInt(w, commandDataLen);
    if(commandDataLen > 0) {
        AGWriteBytes(w, commandData, commandDataLen);
    }
#ifdef DEBUG_PROTOCOL
        printf("\t agprotocol.c - AGWriteCommand(%u, %u) - %s\n", command, commandDataLen, AGProtocolCommandName(command));
#endif


}


ExportFunc void AGWriteMAGIC(AGWriter *w)
{
    AGWriteInt8(w, AG_PROTOCOL_MAGIC_HIGH);
    AGWriteInt8(w, AG_PROTOCOL_MAGIC_LOW);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteMAGIC(%u)\n", (AG_PROTOCOL_MAGIC_HIGH << 8) | AG_PROTOCOL_MAGIC_LOW);
#endif
}

ExportFunc void AGWriteMAJORVERSION(AGWriter *w, int8 major)
{
    AGWriteInt8(w,  major);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteMAJORVERSION(%d)\n", major);
#endif
}

ExportFunc void AGWriteMINORVERSION(AGWriter *w, int8 minor)
{
    AGWriteInt8(w,  minor);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteMINORVERSION(%d)\n", minor);
#endif
}

ExportFunc void AGWriteEND(AGWriter *w)
{
    AGWriteCompactInt(w, AG_END_CMD);
    AGWriteCompactInt(w, 0);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteEND\n");
#endif
}

ExportFunc void AGReadMAGIC(AGReader *r, uint16 *magic)
{
    int32 high, low;
    
    high = AGReadInt8(r);
    low = AGReadInt8(r);
    
    *magic = (high << 8) | low;
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadMAGIC(%u)\n", *magic);
#endif
}
ExportFunc void AGReadMAJORVERSION(AGReader *r, int8 *major)
{
    *major = AGReadInt8(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadMAJORVERSION(%d)\n", *major);
#endif
}
ExportFunc void AGReadMINORVERSION(AGReader *r, int8 *minor)
{
    *minor = AGReadInt8(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadMINORVERSION(%d)\n", *minor);
#endif
}
ExportFunc void AGReadEND(AGReader *r)
{
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadEND\n");
#endif
}

ExportFunc void AGWritePING(AGWriter *w)
{
    AGWriteCompactInt(w, AG_PING_CMD);
    AGWriteCompactInt(w, 0);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWritePING\n");
#endif
}
ExportFunc void AGReadPING(AGReader *r)
{
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadPING\n");
#endif
}

ExportFunc void AGWriteXMLDATA(AGWriter *w, 
                                 int32 dataLen,
                                 void *dataBytes)
{
    int32 len = 0;

    len += AGCompactSize(dataLen);
    len += dataLen;

    AGWriteCompactInt(w, AG_XMLDATA_CMD);
    AGWriteCompactInt(w, len);

    AGWriteCompactInt(w, dataLen);
    AGWriteBytes(w, dataBytes, dataLen);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteXMLDATA(%d)\n", len);
#endif
}


ExportFunc void AGReadXMLDATA(AGReader *r, 
                                 int32 *dataLen,
                                 void **dataBytes)
{

    *dataLen = AGReadCompactInt(r);

    if (*dataLen < 0) {
        return;
    } else if (dataLen == 0) {
        *dataBytes = NULL;
    } else {
        *dataBytes = malloc(*dataLen);
        AGReadBytes(r, *dataBytes, *dataLen);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadXMLDATA(%d)\n", *dataLen);
#endif
}


ExportFunc void AGWriteHELLO(AGWriter *w, char *username, uint8 digestAuth[16],
                             uint8 nonce[16], int32 availableBytes, int32 cookieLength,
                             void *cookie)
{
    int32 len = 0, usernameLen = 0;

    ADD_STRING_LEN(username, usernameLen);

    if (!AGDigestNull(digestAuth)) {
        len += AGCompactSize(16);
        len += 16; /* Add length of digestAuth */
    } else 
        len += AGCompactSize(0);
    if (!AGDigestNull(nonce)) {
        len += AGCompactSize(16);
        len += 16; /* Add length of nonce */
    } else 
        len += AGCompactSize(0);
    len += AGCompactSize(availableBytes);
    len += AGCompactSize(cookieLength);
    len += cookieLength;

    // Write the command.
    AGWriteCompactInt(w, AG_HELLO_CMD);
    AGWriteCompactInt(w, len);

    /* Write username */
    AGWriteString(w, username, usernameLen);

    /* Write digestAuth, 0 if nonce is unknown or invalid */
    if (AGDigestNull(digestAuth))
        AGWriteCompactInt(w, 0);
    else {
        AGWriteCompactInt(w, 16);
        AGWriteBytes(w, digestAuth, 16);
    }
    
    /* Write nonce, 0 if nonce is unknown or invalid */
    if (AGDigestNull(nonce))
        AGWriteCompactInt(w, 0);
    else {
        AGWriteCompactInt(w, 16);
        AGWriteBytes(w, nonce, 16);
    }
    
    AGWriteCompactInt(w, availableBytes);
    AGWriteCompactInt(w, cookieLength);
    AGWriteBytes(w, cookie, cookieLength);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteHELLO\n");
#endif
}

ExportFunc void AGReadHELLO(AGReader *r, char **username, uint8 digestAuth[16],
                            uint8 nonce[16], int32 *availableBytes, int32 *cookieLength,
                            void **cookie)
{   
    int32 digestLen, nonceLen; /* we know the digest is 16 bytes, but if not supplied
                      we will skip it */

    *username = AGReadString(r); 

    digestLen = AGReadCompactInt(r);
    if (!digestLen)
        memset(digestAuth, 0, 16); /* this will tell server the client has no nonce */
    else 
        AGReadBytes(r, digestAuth, 16);

    nonceLen = AGReadCompactInt(r);
    if (!nonceLen)
        memset(nonce, 0, 16); /* this will tell server the client has no nonce */
    else 
        AGReadBytes(r, nonce, 16);

    *availableBytes = AGReadCompactInt(r);

    *cookieLength = AGReadCompactInt(r);
    if (*cookieLength < 0) {
        return;
    } else if (*cookieLength == 0) {
        *cookie = NULL;
    } else {
        *cookie = malloc(*cookieLength);
        AGReadBytes(r, *cookie, *cookieLength);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadHELLO cookieLen=%d\n", *cookieLength);
#endif
}

ExportFunc void AGWriteHELLO2(AGWriter *w, char *username, uint8 digestAuth[16],
                             uint8 nonce[16], int32 availableBytes, int32 cookieLength,
                             void *cookie, uint32 serveruid)
{
    //PENDING(klobad) this is annoying to not be able to reuse the
    //AGWriteHELLO() code because the length is enbedded in there
    int32 len = 0, usernameLen = 0;

    ADD_STRING_LEN(username, usernameLen);

    if (!AGDigestNull(digestAuth)) {
        len += AGCompactSize(16);
        len += 16; /* Add length of digestAuth */
    } else 
        len += AGCompactSize(0);
    if (!AGDigestNull(nonce)) {
        len += AGCompactSize(16);
        len += 16; /* Add length of nonce */
    } else 
        len += AGCompactSize(0);
    len += AGCompactSize(availableBytes);
    len += AGCompactSize(cookieLength);
    len += cookieLength;
    len += AGCompactSize(serveruid);

    // Write the command.
    AGWriteCompactInt(w, AG_HELLO_CMD);
    AGWriteCompactInt(w, len);

    /* Write username */
    AGWriteString(w, username, usernameLen);

    /* Write digestAuth, 0 if nonce is unknown or invalid */
    if (AGDigestNull(digestAuth))
        AGWriteCompactInt(w, 0);
    else {
        AGWriteCompactInt(w, 16);
        AGWriteBytes(w, digestAuth, 16);
    }
    
    /* Write nonce, 0 if nonce is unknown or invalid */
    if (AGDigestNull(nonce))
        AGWriteCompactInt(w, 0);
    else {
        AGWriteCompactInt(w, 16);
        AGWriteBytes(w, nonce, 16);
    }
    
    AGWriteCompactInt(w, availableBytes);
    AGWriteCompactInt(w, cookieLength);
    AGWriteBytes(w, cookie, cookieLength);
    AGWriteCompactInt(w, serveruid);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteHELLO2\n");
#endif
}

ExportFunc void AGReadHELLO2(AGReader *r, char **username,  uint8 digestAuth[16],
                            uint8 nonce[16], int32 *availableBytes, int32 *cookieLength,
                            void **cookie, uint32 *serveruid)
{
    AGReadHELLO(r, username, digestAuth, nonce, availableBytes, cookieLength,
                cookie);
    *serveruid = AGReadCompactInt(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadHELLO2(%d)\n", *serveruid);
#endif
}

ExportFunc void AGWriteDEVICEINFO(AGWriter *w,
    char *osName, char *osVersion,
    int32 colorDepth, int32 screenWidth, int32 screenHeight,
    char *serialNumber, char *language, char *charset, 
    int32 platformDataLength, void *platformData)
{
    int32 len = 0, osNameLen = 0, osVersionLen = 0, serialNumberLen = 0,
        languageLen = 0, charsetLen = 0;

    // compute len

    ADD_STRING_LEN(osName, osNameLen);
    ADD_STRING_LEN(osVersion, osVersionLen);
    len += AGCompactSize(colorDepth);
    len += AGCompactSize(screenWidth);
    len += AGCompactSize(screenHeight);
    ADD_STRING_LEN(serialNumber, serialNumberLen);
    ADD_STRING_LEN(language, languageLen);
    ADD_STRING_LEN(charset, charsetLen);
    len += AGCompactSize(platformDataLength);
    len += platformDataLength;

    // write the command

    AGWriteCompactInt(w, AG_DEVICEINFO_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, osName, osNameLen);
    AGWriteString(w, osVersion, osVersionLen);

    AGWriteCompactInt(w, colorDepth);
    AGWriteCompactInt(w, screenWidth);
    AGWriteCompactInt(w, screenHeight);
    AGWriteString(w, serialNumber, serialNumberLen);
    AGWriteString(w, language, languageLen);
    AGWriteString(w, charset, charsetLen);
    AGWriteCompactInt(w, platformDataLength);
    AGWriteBytes(w, platformData, platformDataLength);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteDEVICEINFO\n");
#endif
}

ExportFunc void AGReadDEVICEINFO(AGReader *r,
    char **osName, char **osVersion,
    int32 *colorDepth, int32 *screenWidth, int32 *screenHeight,
    char **serialNumber, char **language, char **charset,
    int32 *platformDataLength, void **platformData)
{
    *osName = AGReadString(r);
    *osVersion = AGReadString(r);
    *colorDepth = AGReadCompactInt(r);
    *screenWidth = AGReadCompactInt(r);
    *screenHeight = AGReadCompactInt(r);
    *serialNumber = AGReadString(r);
    *language = AGReadString(r);
    *charset = AGReadString(r);

    *platformDataLength = AGReadCompactInt(r);
    if (*platformDataLength < 0) {
        return;
    } else if (*platformDataLength == 0) {
        *platformData = NULL;
    } else {
        *platformData = malloc(*platformDataLength);
        AGReadBytes(r, *platformData, *platformDataLength);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadDEVICEINFO(%s, %s, %d, %d, %d, %s, %s, %s, %d)\n",
                *osName, *osVersion, *colorDepth, *screenWidth, *screenHeight,
                *serialNumber, *language, *charset, *platformDataLength);
#endif
}

ExportFunc void AGWriteSENDDEVICEINFO(AGWriter *w, AGBool send)
{
    AGWriteCompactInt(w, AG_SENDDEVICEINFO_CMD);
    AGWriteCompactInt(w, 1);
    AGWriteBoolean(w, send);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteSENDDEVICEINFO\n");
#endif
}

ExportFunc void AGReadSENDDEVICEINFO(AGReader *r, AGBool *send)
{
    *send = AGReadBoolean(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadSENDDEVICEINFO\n");
#endif
}

ExportFunc void AGWriteDATABASECONFIG(AGWriter *w, char *dbname,
    AGDBConfigType config, AGBool sendRecordPlatformData,
    int32 platformDataLength, void *platformData)
{
    int32 len = 0, dbnameLen = 0;

    ADD_STRING_LEN(dbname, dbnameLen);
    len += AGCompactSize(config);
    len += 1; // 1 boolean - sendRecordPlatformData
    len += AGCompactSize(platformDataLength);
    len += platformDataLength;

    // write the command
    AGWriteCompactInt(w, AG_DATABASECONFIG_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, dbname, dbnameLen);
    AGWriteCompactInt(w, config);
    AGWriteBoolean(w, sendRecordPlatformData);
    AGWriteCompactInt(w, platformDataLength);
    AGWriteBytes(w, platformData, platformDataLength);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteDATABASECONFIG\n");
#endif
}

ExportFunc void AGReadDATABASECONFIG(AGReader *r, char **dbname,
    AGDBConfigType *config, AGBool *sendRecordPlatformData, 
    int32 *platformDataLength, void **platformData)
{
    *dbname = AGReadString(r);
    *config = (AGDBConfigType)AGReadCompactInt(r);
    *sendRecordPlatformData = AGReadBoolean(r);
    *platformDataLength = AGReadCompactInt(r);
    if (*platformDataLength < 0) {
        return;
    } else if (*platformDataLength == 0) {
        *platformData = NULL;
    } else {
        *platformData = malloc(*platformDataLength);
        AGReadBytes(r, *platformData, *platformDataLength);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadDATABASECONFIG\n");
#endif
}

ExportFunc void AGWriteSERVERCONFIG(AGWriter *w, char *friendlyName, 
                                    char *userUrl, char *message, 
                                    char *serverUri, 
                                    AGBool clientShouldHashPasswords,
                                    AGBool allowSecureClientConnect,
                                    uint32 connectTimeoutSeconds, 
                                    uint32 writeTimeoutSeconds, 
                                    uint32 readTimeoutSeconds)
{
    int32 len = 0, friendlyNameLen = 0, userUrlLen = 0, 
        messageLen = 0, serverUriLen = 0;
    int8 flags = 0;
    
    ADD_STRING_LEN(friendlyName, friendlyNameLen);
    ADD_STRING_LEN(userUrl, userUrlLen);
    ADD_STRING_LEN(message, messageLen);
    ADD_STRING_LEN(serverUri, serverUriLen);
    len += sizeof(int8); /* flags */
    len += AGCompactSize(connectTimeoutSeconds);
    len += AGCompactSize(writeTimeoutSeconds);
    len += AGCompactSize(readTimeoutSeconds);

    AGWriteCompactInt(w, AG_SERVERCONFIG_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, friendlyName, friendlyNameLen);
    AGWriteString(w, userUrl, userUrlLen);
    AGWriteString(w, message, messageLen);
    AGWriteString(w, serverUri, serverUriLen);

    if (clientShouldHashPasswords)
        BIS(flags, AG_CLIENT_HASH_PASSWORD);
    if (allowSecureClientConnect)
        BIS(flags, AG_ALLOW_SECURE_CLIENT_CONNECT);
        
    AGWriteInt8(w, flags);
    AGWriteCompactInt(w, connectTimeoutSeconds);
    AGWriteCompactInt(w, writeTimeoutSeconds);
    AGWriteCompactInt(w, readTimeoutSeconds);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteSERVERCONFIG\n");
#endif
}

ExportFunc void AGReadSERVERCONFIG(AGReader *r, char **friendlyName, 
                                   char **userUrl, char **message, 
                                   char **serverUri, 
                                   AGBool *clientShouldHashPasswords, 
                                   AGBool *allowSecureClientConnect, 
                                   uint32 *connectTimeoutSeconds, 
                                   uint32 *writeTimeoutSeconds, 
                                   uint32 *readTimeoutSeconds)
{
    int8 flags;
    
    *friendlyName = AGReadString(r);
    *userUrl = AGReadString(r);
    *message = AGReadString(r);
    *serverUri = AGReadString(r);

    flags = AGReadInt8(r);

    if (BIT(flags, AG_CLIENT_HASH_PASSWORD))
        *clientShouldHashPasswords = 1;
    else
        *clientShouldHashPasswords = 0;

    if (BIT(flags, AG_ALLOW_SECURE_CLIENT_CONNECT))
        *allowSecureClientConnect = 1;
    else
        *allowSecureClientConnect = 0;

    *connectTimeoutSeconds = AGReadCompactInt(r);
    *writeTimeoutSeconds = AGReadCompactInt(r);
    *readTimeoutSeconds = AGReadCompactInt(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadSERVERCONFIG\n");
#endif
}

ExportFunc void AGWriteCOOKIE(AGWriter *w, int32 cookieLength, void *cookie)
{
    int32 len = 0;

    len += AGCompactSize(cookieLength);
    len += cookieLength;

    AGWriteCompactInt(w, AG_COOKIE_CMD);
    AGWriteCompactInt(w, len);

    AGWriteCompactInt(w, cookieLength);
    AGWriteBytes(w, cookie, cookieLength);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteCOOKIE(%d)\n", cookieLength);
#endif
}
ExportFunc void AGReadNONCE(AGReader *r, uint8 nonce[16])
{
    AGReadBytes(r, nonce, 16);

#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadNONCE");
#endif
}
ExportFunc void AGWriteNONCE(AGWriter *w, uint8 nonce[16])
{
    AGWriteCompactInt(w, AG_NONCE_CMD);
    AGWriteCompactInt(w, 16);
    AGWriteBytes(w, nonce, 16);

#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteNONCE");
#endif
}
ExportFunc void AGReadCOOKIE(AGReader *r, int32 *cookieLength, void **cookie)
{
    *cookieLength = AGReadCompactInt(r);
    if (*cookieLength < 0) {
        return;
    } else if (cookieLength == 0) {
        *cookie = NULL;
    } else {
        *cookie = malloc(*cookieLength);
        AGReadBytes(r, *cookie, *cookieLength);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadCOOKIE(%d)\n", *cookieLength);
#endif
}

ExportFunc void AGWriteTASK(AGWriter *w, char *currentTask, AGBool bufferable)
{
    int32 len = 0, currentTaskLen = 0;

    ADD_STRING_LEN(currentTask, currentTaskLen);
    len += 1;

    AGWriteCompactInt(w, AG_TASK_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, currentTask, currentTaskLen);
    AGWriteBoolean(w, bufferable);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteTASK(%s, %d)\n", currentTask, bufferable);
#endif
}

ExportFunc void AGReadTASK(AGReader *r, char **currentTask, AGBool *bufferable)
{
    *currentTask = AGReadString(r);
    *bufferable = AGReadBoolean(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadTASK(%s, %d)\n", *currentTask, *bufferable);
#endif
}

ExportFunc void AGWriteITEM(AGWriter *w, int32 currentItemNumber,
    int32 totalItemCount, char *currentItem)
{
    int32 len = 0, currentItemLen = 0;

    len += AGCompactSize(currentItemNumber);
    len += AGCompactSize(totalItemCount);
    ADD_STRING_LEN(currentItem, currentItemLen);

    AGWriteCompactInt(w, AG_ITEM_CMD);
    AGWriteCompactInt(w, len);

    AGWriteCompactInt(w, currentItemNumber);
    AGWriteCompactInt(w, totalItemCount);
    AGWriteString(w, currentItem, currentItemLen);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteITEM(%d, %d, %s)\n", currentItemNumber, totalItemCount, currentItem);
#endif
}

ExportFunc void AGReadITEM(AGReader *r, int32 *currentItemNumber,
    int32 *totalItemCount, char **currentItem)
{
    *currentItemNumber = AGReadCompactInt(r);
    *totalItemCount = AGReadCompactInt(r);
    *currentItem = AGReadString(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadITEM\n");
#endif
}

ExportFunc void AGWriteDELETEDATABASE(AGWriter *w, char *dbname)
{
    int32 len = 0, dbnameLen = 0;

    ADD_STRING_LEN(dbname, dbnameLen);

    AGWriteCompactInt(w, AG_DELETEDATABASE_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, dbname, dbnameLen);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteDELETEDATABASE(%s)\n", dbname);
#endif
}

ExportFunc void AGReadDELETEDATABASE(AGReader *r, char **dbname)
{
    *dbname = AGReadString(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadDELETEDATABASE\n");
#endif
}

ExportFunc void AGWriteOPENDATABASE(AGWriter *w, char *dbname)
{
    int32 len = 0, dbnameLen = 0;

    ADD_STRING_LEN(dbname, dbnameLen);

    AGWriteCompactInt(w, AG_OPENDATABASE_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, dbname, dbnameLen);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteOPENDATABASE(%s)\n", dbname);
#endif
}

ExportFunc void AGReadOPENDATABASE(AGReader *r, char **dbname)
{
    *dbname = AGReadString(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadOPENDATABASE(%s)\n", *dbname);
#endif
}

ExportFunc void AGWriteCLOSEDATABASE(AGWriter *w)
{
    AGWriteCompactInt(w, AG_CLOSEDATABASE_CMD);
    AGWriteCompactInt(w, 0);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteCLOSEDATABASE\n");
#endif
}

ExportFunc void AGReadCLOSEDATABASE(AGReader *r)
{
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadCLOSEDATABASE\n");
#endif
}

ExportFunc void AGWriteCLEARMODS(AGWriter *w)
{
    AGWriteCompactInt(w, AG_CLEARMODS_CMD);
    AGWriteCompactInt(w, 0);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteCLEARMODS\n");
#endif
}

ExportFunc void AGReadCLEARMODS(AGReader *r)
{
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadCLEARMODS\n");
#endif
}

ExportFunc void AGWriteGOODBYE(AGWriter *w, AGSyncStatus syncStatus, 
    int32 errorCode, char *errorMsg)
{
    int32 len = 0, errorMsgLen = 0;

    len += AGCompactSize(syncStatus);
    len += AGCompactSize(errorCode);
    ADD_STRING_LEN(errorMsg, errorMsgLen);

    AGWriteCompactInt(w, AG_GOODBYE_CMD);
    AGWriteCompactInt(w, len);

    AGWriteCompactInt(w, syncStatus);
    AGWriteCompactInt(w, errorCode);
    AGWriteString(w, errorMsg, errorMsgLen);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteGOODBYE(%d, %d, %s)\n", syncStatus, errorCode, errorMsg);
#endif
}

ExportFunc void AGReadGOODBYE(AGReader *r, AGSyncStatus *syncStatus, 
    int32 *errorCode, char **errorMsg)
{
    *syncStatus = (AGSyncStatus)AGReadCompactInt(r);
    *errorCode = AGReadCompactInt(r);
    *errorMsg = AGReadString(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadGOODBYE(%d, %d, %s)\n", *syncStatus, *errorCode, *errorMsg);
#endif
}

ExportFunc void AGWriteRECORD(AGWriter *w, int32 uid, AGRecordStatus mod,
    int32 recordDataLength, void *recordData,
    int32 platformDataLength, void *platformData)
{
    int32 len = 0;

    len += 4; // uid
    len += AGCompactSize(mod);
    len += AGCompactSize(recordDataLength);
    len += recordDataLength;
    len += AGCompactSize(platformDataLength);
    len += platformDataLength;

    AGWriteCompactInt(w, AG_RECORD_CMD);
    AGWriteCompactInt(w, len);

    AGWriteInt32(w, uid);
    AGWriteCompactInt(w, mod);
    AGWriteCompactInt(w, recordDataLength);
    AGWriteBytes(w, recordData, recordDataLength);
    AGWriteCompactInt(w, platformDataLength);
    AGWriteBytes(w, platformData, platformDataLength);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteRECORD(%d, %d, %d, %d)\n", uid, mod, recordDataLength, platformDataLength);
#endif
}

#ifdef __palmos__
ExportFunc void AGReadRECORDNoCopy(AGBufferReader *r, 
    int32 *uid, AGRecordStatus *mod,
    int32 *recordDataLength, void **recordData,
    int32 *platformDataLength, void **platformData)
{
    *uid = AGReadInt32((AGReader *)r);
    *mod = (AGRecordStatus)AGReadCompactInt((AGReader *)r);
    
    *recordDataLength = AGReadCompactInt((AGReader *)r);
    if (*recordDataLength < 0) {
        return;
    } else if (*recordDataLength == 0) {
        *recordData = NULL;
    } else {
        *recordData = r->buffer + r->currentIndex;
        AGBufferReaderSkipBytes(r, *recordDataLength);
    }

    *platformDataLength = AGReadCompactInt((AGReader *)r);
    if (*platformDataLength < 0) {
        return;
    } else if (*platformDataLength == 0) {
        *platformData = NULL;
    } else {
        *platformData = r->buffer + r->currentIndex;
        AGBufferReaderSkipBytes(r, *platformDataLength);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadRECORDNoCopy(%d, %d, %d, %d)\n", *uid, *mod, *recordDataLength, *platformDataLength);
#endif
}
#endif

ExportFunc void AGReadRECORD(AGReader *r, int32 *uid, AGRecordStatus *mod,
    int32 *recordDataLength, void **recordData,
    int32 *platformDataLength, void **platformData)
{
    *uid = AGReadInt32(r);
    *mod = (AGRecordStatus)AGReadCompactInt(r);
    
    *recordDataLength = AGReadCompactInt(r);
    if (*recordDataLength < 0) {
        return;
    } else if (*recordDataLength == 0) {
        *recordData = NULL;
    } else {
        *recordData = malloc(*recordDataLength);
        AGReadBytes(r, *recordData, *recordDataLength);
    }

    *platformDataLength = AGReadCompactInt(r);
    if (*platformDataLength < 0) {
        return;
    } else if (*platformDataLength == 0) {
        *platformData = NULL;
    } else {
        *platformData = malloc(*platformDataLength);
        AGReadBytes(r, *platformData, *platformDataLength);
    }
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadRECORD(%d, %d, %d, %d)\n", *uid, *mod, *recordDataLength, *platformDataLength);
#endif
}

ExportFunc void AGWriteUNKNOWNDATABASE(AGWriter *w, char *dbname)
{
    int32 len = 0, dbnameLen = 0;

    ADD_STRING_LEN(dbname, dbnameLen);

    AGWriteCompactInt(w, AG_UNKNOWNDATABASE_CMD);
    AGWriteCompactInt(w, len);

    AGWriteString(w, dbname, dbnameLen);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGWriteUNKNOWNDATABASE\n");
#endif
}

ExportFunc void AGReadUNKNOWNDATABASE(AGReader *r, char **dbname)
{
    *dbname = AGReadString(r);
#ifdef DEBUG_PROTOCOL
    printf("\t agprotocol.c - AGReadUNKNOWNDATABASE\n");
#endif
}

ExportFunc void AGWriteNEWIDS(AGWriter *w, AGArray *newids)
{
    int32 i, count, len = 0;

    if(!newids || AGArrayCount(newids) < 1) {
        count = 0;
    } else {
        count = AGArrayCount(newids);
    }

    len += AGCompactSize(count);
    len += (count * 4);

    AGWriteCompactInt(w, AG_NEWIDS_CMD);
    AGWriteCompactInt(w, len);

    AGWriteCompactInt(w, count);
    if(count > 0) {
        for(i = 0; i < count; i++) 
            AGWriteInt32(w, (uint32)AGArrayElementAt(newids, i));
    }
}

ExportFunc void AGReadNEWIDS(AGReader *r, AGArray **newids)
{
    int32 i, count;

    *newids = NULL;
    count = AGReadCompactInt(r);
    if(count > 0) {
        *newids = AGArrayNew(AGIntegerElements, count);
        for(i = 0; i < count; i++) 
            AGArrayAppend(*newids, (void *)AGReadInt32(r));
    }
}

ExportFunc void AGWriteEXPANSION(AGWriter *w, 
                                 int32 expansionCommand, 
                                 int32 commandLength,
                                 void *commandBytes)
{
    int32 len = 0;

    len += AGCompactSize(expansionCommand);
    len += AGCompactSize(commandLength);
    len += commandLength;

    AGWriteCompactInt(w, AG_EXPANSION_CMD);
    AGWriteCompactInt(w, len);
    AGWriteCompactInt(w, expansionCommand);
    AGWriteCompactInt(w, commandLength);
    AGWriteBytes(w, commandBytes, commandLength);

}

ExportFunc void AGReadEXPANSION(AGReader *r, 
                                 int32 *expansionCommand, 
                                 int32 *commandLength,
                                 void **commandBytes)
{
    *expansionCommand = AGReadCompactInt(r);
    *commandLength = AGReadCompactInt(r);
    if(*commandLength) {
        *commandBytes = malloc(*commandLength);
        AGReadBytes(r, *commandBytes, *commandLength);
    }
}

ExportFunc void AGWriteEXPANSION_RESOURCE(AGWriter *w, 
                                          uint32 resourceType,
                                          uint32 resourceLen,
                                          void* resource)
{
    int32 len = 0;
    AGBufferWriter * tw = NULL;

    len += AGCompactSize(resourceType);
    len += AGCompactSize(resourceLen);
    len += resourceLen;

    tw = AGBufferWriterNew(len);
    AGWriteCompactInt((AGWriter*)tw, resourceType);
    AGWriteCompactInt((AGWriter*)tw, resourceLen);
    if (resourceLen > 0)
        AGWriteBytes((AGWriter*)tw, resource, resourceLen);

    AGWriteEXPANSION(w,
        AG_EXPANSION_RESOURCE,
        len,
        AGBufferWriterGetBuffer(tw));
    AGBufferWriterFree(tw);

}

ExportFunc void AGReadEXPANSION_RESOURCE(AGReader *r,
                                         int32* resourceType,
                                         int32* resourceLen,
                                         void** resource)
{
    AGReadEXPANSION(r, resourceType, resourceLen, resource);
}

void AGWriteEXPANSION_CHANGESERVERCONFIG(AGWriter *w,
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
    int32 len = 0, serverNameLen = 0, userNameLen = 0;
    AGBufferWriter * tw = NULL;

    len += 1; /* disableServer */
    len += AGCompactSize(flags);
    ADD_STRING_LEN(serverName, serverNameLen);
    len += AGCompactSize(serverPort);
    ADD_STRING_LEN(userName, userNameLen);
    len += AGCompactSize(passwordLen);
    len += passwordLen; /* password */
    len += 1; /* connectSecurely */
    len += 1; /* notRemovable */

    tw = AGBufferWriterNew(len);
    AGWriteBoolean((AGWriter*)tw, disableServer);
    AGWriteCompactInt((AGWriter*)tw, flags);
    AGWriteString((AGWriter*)tw, serverName, serverNameLen);
    AGWriteCompactInt((AGWriter*)tw, serverPort);
    AGWriteString((AGWriter*)tw, userName, userNameLen);
    AGWriteCompactInt((AGWriter*)tw, passwordLen);
    if (passwordLen > 0)
        AGWriteBytes((AGWriter*)tw, password, passwordLen);
    AGWriteBoolean((AGWriter*)tw, connectSecurely);
    AGWriteBoolean((AGWriter*)tw, notRemovable);

    AGWriteEXPANSION(w,
        AG_EXPANSION_CHANGESERVERCONFIG,
        len,
        AGBufferWriterGetBuffer(tw));
    AGBufferWriterFree(tw);
}

ExportFunc void AGReadEXPANSION_CHANGESERVERCONFIG(AGReader *r,
                                                   AGBool *disableServer,
                                                   int32 *flags,
                                                   char **serverName,
                                                   int16 *serverPort,
                                                   char **userName,
                                                   int32 *passwordLen,
                                                   uint8 **password,
                                                   AGBool *connectSecurely,
                                                   AGBool *notRemovable)
{
    *disableServer = AGReadBoolean(r);
    *flags = AGReadCompactInt(r);
    *serverName = AGReadString(r);
    *serverPort = AGReadCompactInt(r);
    *userName = AGReadString(r);
    *passwordLen = AGReadCompactInt(r);
    if (0 == *passwordLen)
        *password = NULL;
    else {
        /* Failure is not an option! */
        *password = (uint8*)malloc(*passwordLen);
        AGReadBytes(r, *password, *passwordLen);
    }
    *connectSecurely = AGReadBoolean(r);
    *notRemovable = AGReadBoolean(r);
}

ExportFunc char *AGProtocolCommandName(AGCommand command)
{
    switch(command) {
    case AG_END_CMD:        return "AG_END_CMD";
    case AG_EXPANSION_CMD:    return "AG_EXPANSION_CMD";
    case AG_HELLO_CMD:      return "AG_HELLO_CMD";
    case AG_DEVICEINFO_CMD:    return "AG_DEVICEINFO_CMD";
    case AG_SENDDEVICEINFO_CMD:    return "AG_SENDDEVICEINFO_CMD";
    case AG_DATABASECONFIG_CMD:    return "AG_DATABASECONFIG_CMD";
    case AG_SERVERCONFIG_CMD:    return "AG_SERVERCONFIG_CMD";
    case AG_COOKIE_CMD:     return "AG_COOKIE_CMD";
    case AG_TASK_CMD:       return "AG_TASK_CMD";
    case AG_ITEM_CMD:       return "AG_ITEM_CMD";
    case AG_DELETEDATABASE_CMD:    return "AG_DELETEDATABASE_CMD";
    case AG_OPENDATABASE_CMD:    return "AG_OPENDATABASE_CMD";
    case AG_CLOSEDATABASE_CMD:    return "AG_CLOSEDATABASE_CMD";
    case AG_CLEARMODS_CMD:    return "AG_CLEARMODS_CMD";
    case AG_GOODBYE_CMD:    return "AG_GOODBYE_CMD";
    case AG_RECORD_CMD:    return "AG_RECORD_CMD";
    case AG_NONCE_CMD:    return "AG_NONCE_CMD";
    case AG_NEWIDS_CMD:    return "AG_NEWIDS_CMD";
    case AG_UNKNOWNDATABASE_CMD:    return "AG_UNKNOWNDATABASE_CMD";
    case AG_PING_CMD:    return "AG_PING_CMD";
    case AG_XMLDATA_CMD:    return "AG_XMLDATA_CMD";
    case AG_LASTCOMMAND:    return "AG_LASTCOMMAND"; // This should never happen
    }

    return NULL;
}
