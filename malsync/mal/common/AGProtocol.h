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

#ifndef __AGPROTOCOL_H__
#define __AGPROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGReader.h>
#include <AGWriter.h>
#include <AGDBConfig.h>
#include <AGRecord.h>
#include <AGArray.h>
#ifdef __palmos__
#include <AGBufferReader.h> // Only needed for AGReadRECORDNoCopy()
#endif

// The protocol magic is sent as the first two bytes
// of the stream

#define AG_PROTOCOL_MAGIC_HIGH          0xDA
#define AG_PROTOCOL_MAGIC_LOW           0x7E

// The major version number of the protocol is a one
// byte value which is incremented each time an
// incompatible change is made to the protocol.
// The minor version of the protocol is incremented
// each time a change is made to the protocol which
// can be handled by older version in a compatible
// way.  Adding a command which can be safely ignored
// is an example of a compatible change.  Changing
// the format of an existing command or adding a command
// which the other side must understand to function
// correctly is an example of an incompatible change.

#define AG_PROTOCOL_MAJOR_VERSION   1
#define AG_PROTOCOL_MINOR_VERSION   0

// These are the values of the commands in the current
// version of the protocol.

typedef enum {
    AG_END_CMD = 0,
    AG_EXPANSION_CMD,
    AG_HELLO_CMD,
    AG_DEVICEINFO_CMD,
    AG_SENDDEVICEINFO_CMD,
    AG_DATABASECONFIG_CMD,
    AG_SERVERCONFIG_CMD,
    AG_COOKIE_CMD,
    AG_NONCE_CMD,
    AG_TASK_CMD,
    AG_ITEM_CMD,
    AG_DELETEDATABASE_CMD,
    AG_OPENDATABASE_CMD,
    AG_CLOSEDATABASE_CMD,
    AG_CLEARMODS_CMD,
    AG_GOODBYE_CMD,
    AG_RECORD_CMD,
    AG_UNKNOWNDATABASE_CMD,
    AG_NEWIDS_CMD,
    AG_PING_CMD,
    AG_XMLDATA_CMD,

    AG_LASTCOMMAND  // This is not a real command, just used for AGIsValidCommand
} AGCommand;

// These are the values of the expansion commands in the current
// version of the protocol.

typedef enum {
    AG_EXPANSION_RESOURCE = 0,
    AG_EXPANSION_CHANGESERVERCONFIG
} AGExpansionCommand;

// These are the values of the types of resources that can be sent within
// a RESOURCE expansion command.
    
typedef enum {
    AG_EXPANSION_RESOURCE_SERVERTYPE = 0,
    AG_EXPANSION_RESOURCE_FILE
} AGResourceType;

//These bits can be ORed into to a AGResourceType to use chunking
#define AG_RES_PARTIAL   (1<<31)
#define AG_RES_FINAL     (1<<30)
#define AG_RES_MASK (AG_RES_PARTIAL|AG_RES_FINAL)

/*  Set these flags to indicate which fields are requested to be changed in
    a CHANGESERVERCONFIG expansion command.
*/
#define AG_CHSC_SERVERNAME          (0x00000001)
#define AG_CHSC_SERVERPORT          (0x00000002)
#define AG_CHSC_USERNAME            (0x00000004)
#define AG_CHSC_PASSWORD            (0x00000008)
#define AG_CHSC_CONNECTSECURELY     (0x00000010)
#define AG_CHSC_NOTREMOVABLE        (0x00000020)
#define AG_CHSC_ALL                 (0xffffffff)

// When saying goodbye the server can ask the client
// to call back immediately.

typedef enum {
    AG_DONE_STATUS = 0,
    AG_CALLAGAIN_STATUS
} AGSyncStatus;

#define AG_CLIENT_HASH_PASSWORD 0x01
#define AG_ALLOW_SECURE_CLIENT_CONNECT 0x02
    
#define AGIsValidCommand(c) (((int32)(c) >= AG_END_CMD) && ((int32)(c) < AG_LASTCOMMAND))

// 4194304 bytes 
// Used by AGSyncProcessor to sanity check the length of a mal command
#define AGMAL_MAX_COMMAND_LEN ((uint32)1 << 22)

// When sending records to the server, the client should
// indicate the state of the record.  The client should not
// send up unmodified records unless the server has
// configured that database as SENDALL.

// These functions read and write the protocol using
// an AGReader/Writer.  The write versions write the
// command, length, and data to the stream.  The read
// functions expect the command, but not the length
// to have already been read.  The caller must free
// strings and void *s returned from the reads.

// These functions set the err field on the reader/writer
// on error.

// PENDING(linus) Allocation issues on the pilot?

ExportFunc void AGWriteMAGIC(AGWriter *w);
ExportFunc void AGReadMAGIC(AGReader *r, uint16 *magic);

ExportFunc void AGWriteMAJORVERSION(AGWriter *w, int8 major);
ExportFunc void AGReadMAJORVERSION(AGReader *r, int8 *major);

ExportFunc void AGWriteMINORVERSION(AGWriter *w, int8 minor);
ExportFunc void AGReadMINORVERSION(AGReader *r, int8 *minor);

ExportFunc void AGWriteEND(AGWriter *w);
ExportFunc void AGReadEND(AGReader *r);
typedef int32 (*AGPerformEndFunc)(void *out, int32 *returnErrorCode);

ExportFunc void AGWriteCommand(AGWriter *w, int32 command,
                                int32 commandDataLen,
                                void *commandData);

// Client to Server commands

ExportFunc void AGWriteHELLO(AGWriter *w, char *username, uint8 digestAuth[16],
                             uint8 nonce[16], int32 availableBytes, int32 cookieLength,
                             void *cookie);
ExportFunc void AGReadHELLO(AGReader *r, char **username,  uint8 digestAuth[16],
                            uint8 nonce[16], int32 *availableBytes, int32 *cookieLength,
                            void **cookie);
ExportFunc void AGWriteHELLO2(AGWriter *w, char *username, uint8 digestAuth[16],
                             uint8 nonce[16], int32 availableBytes, int32 cookieLength,
                             void *cookie, uint32 serveruid);
ExportFunc void AGReadHELLO2(AGReader *r, char **username,  uint8 digestAuth[16],
                            uint8 nonce[16], int32 *availableBytes, int32 *cookieLength,
                            void **cookie, uint32 *serveruid);

ExportFunc void AGWriteDEVICEINFO(AGWriter *w,
    char *osName, char *osVersion,
    int32 colorDepth, int32 screenWidth, int32 screenHeight,
    char *serialNumber, char *language, char *charset, 
    int32 platformDataLength, void *platformData);

ExportFunc void AGReadDEVICEINFO(AGReader *r,
    char **osName, char **osVersion,
    int32 *colorDepth, int32 *screenWidth, int32 *screenHeight,
    char **serialNumber, char **language, char **charset,
    int32 *platformDataLength, void **platformData);

ExportFunc void AGWriteUNKNOWNDATABASE(AGWriter *w, char *dbname);
ExportFunc void AGReadUNKNOWNDATABASE(AGReader *r, char **dbname);

ExportFunc void AGWriteNEWIDS(AGWriter *w, AGArray *newids);
ExportFunc void AGReadNEWIDS(AGReader *r, AGArray **newids);

ExportFunc void AGWritePING(AGWriter *w);
ExportFunc void AGReadPING(AGReader *r);

ExportFunc void AGWriteXMLDATA(AGWriter *w, 
                                 int32 dataLen,
                                 void *dataBytes);
ExportFunc void AGReadXMLDATA(AGReader *r, 
                                 int32 *dataLen,
                                 void **dataBytes);

// Server to Client commands

ExportFunc void AGWriteSENDDEVICEINFO(AGWriter *w, AGBool send);
ExportFunc void AGReadSENDDEVICEINFO(AGReader *r, AGBool *send);
typedef int32 (*AGPerformSendDeviceInfoFunc)(void *out, int32 *returnErrorCode,
                                             AGBool send);

ExportFunc void AGWriteDATABASECONFIG(AGWriter *w, char *dbname,
    AGDBConfigType config, AGBool sendRecordPlatformData, 
    int32 platformDataLength, void *platformData);
ExportFunc void AGReadDATABASECONFIG(AGReader *r, char **dbname,
    AGDBConfigType *config, AGBool *sendRecordPlatformData,
    int32 *platformDataLength, void **platformData);
typedef int32 (*AGPerformDatabaseConfigFunc)(void *out, int32 *returnErrorCode,
                                             char *dbname, 
                                             AGDBConfigType config, 
                                             AGBool sendRecordPlatformData, 
                                             int32 platformDataLength, 
                                             void *platformData);

ExportFunc void AGWriteSERVERCONFIG(AGWriter *w, char *friendlyName, 
                                    char *userUrl, char *message, 
                                    char *serverUri, 
                                    AGBool clientShouldHashPasswords,
                                    AGBool allowSecureClientConnect, 
                                    uint32 connectTimeoutSeconds, 
                                    uint32 writeTimeoutSeconds, 
                                    uint32 readTimeoutSeconds);
ExportFunc void AGReadSERVERCONFIG(AGReader *r, char **friendlyName, 
                                   char **userUrl, char **message, 
                                   char **serverUri, 
                                   AGBool *clientShouldHashPasswords,
                                   AGBool *allowSecureClientConnect, 
                                   uint32 *connectTimeoutSeconds, 
                                   uint32 *writeTimeoutSeconds, 
                                   uint32 *readTimeoutSeconds);
typedef int32 (*AGPerformServerConfigFunc)(void *out, int32 *returnErrorCode,
                                           char *friendlyName, 
                                           char *userUrl,
                                           char *message,
                                           char *serverUri, 
                                           AGBool clientShouldHashPasswords,
                                           uint32 connectTimeoutSeconds, 
                                           uint32 writeTimeoutSeconds, 
                                           uint32 readTimeoutSeconds);

ExportFunc void AGWriteCOOKIE(AGWriter *w, int32 cookieLength, void *cookie);
ExportFunc void AGReadCOOKIE(AGReader *r, int32 *cookieLength, void **cookie);
typedef int32 (*AGPerformCookieFunc)(void *out, int32 *returnErrorCode,
                                     int32 cookieLength,
                                     void *cookie);

ExportFunc void AGWriteNONCE(AGWriter *w, uint8 nonce[16]);
ExportFunc void AGReadNONCE(AGReader *r, uint8 nonce[16]);
typedef int32 (*AGPerformNonceFunc)(void *out, int32 *returnErrorCode,
                                    uint8 nonce[16]);

// Commands for the UI.  The client should be able to
// tell the server not to send these.

ExportFunc void AGWriteTASK(AGWriter *w, char *currentTask, AGBool bufferable);
ExportFunc void AGReadTASK(AGReader *r, char **currentTask, AGBool *bufferable);
typedef int32 (*AGPerformTaskFunc)(void *out, int32 *returnErrorCode,
                                   char *currentTask, AGBool bufferable);

ExportFunc void AGWriteITEM(AGWriter *w, int32 currentItemNumber,
    int32 totalItemCount, char *currentItem);
ExportFunc void AGReadITEM(AGReader *r, int32 *currentItemNumber,
    int32 *totalItemCount, char **currentItem);
typedef int32 (*AGPerformItemFunc)(void *out, int32 *returnErrorCode,
                                   int32 currentItemNumber,
                                   int32 totalItemCount,
                                   char *currentItem);

ExportFunc void AGWriteDELETEDATABASE(AGWriter *w, char *dbname);
ExportFunc void AGReadDELETEDATABASE(AGReader *r, char **dbname);
typedef int32 (*AGPerformDeleteDatabaseFunc)(void *out, int32 *returnErrorCode,
                                             char *dbname);

ExportFunc void AGWriteOPENDATABASE(AGWriter *w, char *dbname);
ExportFunc void AGReadOPENDATABASE(AGReader *r, char **dbname);
typedef int32 (*AGPerformOpenDatabaseFunc)(void *out, int32 *returnErrorCode,
                                           char *dbname);

// Closes the current database.

ExportFunc void AGWriteCLOSEDATABASE(AGWriter *w);
ExportFunc void AGReadCLOSEDATABASE(AGReader *r);
typedef int32 (*AGPerformCloseDatabaseFunc)(void *out, int32 *returnErrorCode);

// PENDING(linus) Clears the mod bits and purges deleted
// records.  Should this include a cookie of some kind?
// This command in only valid inside and OPENDATABASE/CLOSEDATABASE

ExportFunc void AGWriteCLEARMODS(AGWriter *w);
ExportFunc void AGReadCLEARMODS(AGReader *r);
typedef int32 (*AGPerformClearModsFunc)(void *out, int32 *returnErrorCode);

// PENDING(linus) What are the defined error codes?

ExportFunc void AGWriteGOODBYE(AGWriter *w, AGSyncStatus syncStatus, 
    int32 errorCode, char *errorMsg);
ExportFunc void AGReadGOODBYE(AGReader *r, AGSyncStatus *syncStatus, 
    int32 *errorCode, char **errorMsg);
typedef int32 (*AGPerformGoodbyeFunc)(void *out, int32 *returnErrorCode,
                                      AGSyncStatus syncStatus,
                                      int32 errorCode,
                                      char *errorMessage);

// Commands used by Client and Server

ExportFunc void AGWriteRECORD(AGWriter *w, int32 uid, AGRecordStatus mod,
    int32 recordDataLength, void *recordData,
    int32 platformDataLength, void *platformData);
ExportFunc void AGReadRECORD(AGReader *r, int32 *uid, AGRecordStatus *mod,
    int32 *recordDataLength, void **recordData,
    int32 *platformDataLength, void **platformData);
#ifdef __palmos__
// Identical to the AGReadRECORD, but does not create copies of the
// recordData and platformData, the pointers are the pointers into the
// buffer in the AGBufferReader structure.
ExportFunc void AGReadRECORDNoCopy(AGBufferReader *r, 
    int32 *uid, AGRecordStatus *mod,
    int32 *recordDataLength, void **recordData,
    int32 *platformDataLength, void **platformData);
#endif
typedef int32 (*AGPerformRecordFunc)(void *out, int32 *returnErrorCode,
                                     int32 *newUID,
                                     int32 uid,
                                     AGRecordStatus mod,
                                     int32 recordDataLength,
                                     void *recordData,
                                     int32 platformDataLength,
                                     void *platformData);

ExportFunc void AGWriteEXPANSION(AGWriter *w, 
                                 int32 expansionCommand, 
                                 int32 commandLength,
                                 void *commandBytes);
ExportFunc void AGReadEXPANSION(AGReader *r, 
                                 int32 *expansionCommand, 
                                 int32 *commandLength,
                                 void **commandBytes);
typedef int32 (*AGPerformExpansionFunc)(void *out, int32 *returnErrorCode,
                                        int32 expansionCommand, 
                                        int32 commandLength,
                                        void *commandBytes);

ExportFunc void AGWriteEXPANSION_RESOURCE(AGWriter *w, 
                                          uint32 resourceType,
                                          uint32 resourceLen,
                                          void* resource);
ExportFunc void AGReadEXPANSION_RESOURCE(AGReader *r,
                                         int32* resourceType,
                                         int32* resourceLen,
                                         void** resource);
typedef int32 (*AGPerformExpansionResourceFunc)(void *out,
                                                int32 *returnErrorCode,
                                                int32 resourceType, 
                                                int32 resourceLen,
                                                void *resource);

/*  disableServer is intentionally separated from the other arguments
    to these functions because the flags field tells whether the other
    arguments have been filled in, but disableServer is more like a
    command (disable this server) rather than a request to modify a field
    in the serverConfig.  Thus it doesn't need a separate flag to indicate
    that the argument has not been filled in.
*/
ExportFunc void AGWriteEXPANSION_CHANGESERVERCONFIG(AGWriter *w,
                                                    AGBool disableServer,
                                                    int32 flags,
                                                    char *serverName,
                                                    int16 serverPort,
                                                    char *userName,
                                                    int32 passwordLen,
                                                    uint8 *password,
                                                    AGBool connectSecurely,
                                                    AGBool notRemovable);
ExportFunc void AGReadEXPANSION_CHANGESERVERCONFIG(AGReader *r,
                                                   AGBool *disableServer,
                                                   int32 *flags,
                                                   char **serverName,
                                                   int16 *serverPort,
                                                   char **userName,
                                                   int32 *passwordLen,
                                                   uint8 **password,
                                                   AGBool *connectSecurely,
                                                   AGBool *notRemovable);
typedef int32 (*AGPerformExpansionChSCFunc)(void *out,
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

// Generic Function to return the string representation of a command
ExportFunc char *AGProtocolCommandName(AGCommand command);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGPROTOCOL_H__
