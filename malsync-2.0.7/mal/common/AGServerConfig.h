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

#ifndef __AGSERVERCONFIG_H__
#define __AGSERVERCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>
#include <AGArray.h>
#include <AGReader.h>
#include <AGWriter.h>
#include <AGRecord.h>
#include <AGDBConfig.h>

#define AGSERVERCONFIG_MAX_PASSWORD_LENGTH (14)

/* AGServerConfig resides on the desktop and the device.
 It is editable on both the desktop and the device.
 The server modifies the database configurations 
 when it needs to do so (as well as the adminURL 
 and description).
*/

enum {
    AG_HASH_PASSWORD_NO = 0,
    AG_HASH_PASSWORD_YES = 1,
    AG_HASH_PASSWORD_UNKNOWN
};
    
/*  Note:  This data structure has been published to the world.
    If anything in it changes, it must remain compatible with previous
    versions.
*/
typedef struct AGServerConfig {

    int32 uid;
    AGRecordStatus status;

    /* device/desktop modifies */
    char *serverName;
    uint16 serverPort;
    char *userName;
    char *cleartextPassword;
    uint8 password[16]; /* hash */
    AGBool disabled;
    AGBool resetCookie; /* TRUE if at next sync cookie should be reset. */
    AGBool notRemovable;

    /* server modifies */
    char *friendlyName;
    char *serverType;
    char *userUrl;
    char *description;
    char *serverUri;
    int32 sequenceCookieLength;
    uint8 *sequenceCookie;
    AGArray *dbconfigs;
    uint8 nonce[16];
    AGBool sendDeviceInfo;
    uint8 hashPassword;

    uint32 connectTimeout;
    uint32 writeTimeout;
    uint32 readTimeout;

    AGBool connectSecurely;
    AGBool allowSecureConnection;

    int32 expansion1;
    int32 expansion2;
    int32 expansion3;
    int32 expansion4;

    int32 reservedLen;
    void * reserved;

} AGServerConfig;


ExportFunc AGServerConfig *AGServerConfigNew();
ExportFunc void AGServerConfigInit(AGServerConfig *obj);
ExportFunc void AGServerConfigFinalize(AGServerConfig *obj);
ExportFunc void AGServerConfigFree(AGServerConfig *obj);

ExportFunc AGServerConfig *AGServerConfigCopy(AGServerConfig *dst, 
                                              AGServerConfig *src);
ExportFunc AGServerConfig *AGServerConfigDup(AGServerConfig *src);

ExportFunc int32 AGServerConfigReadData(AGServerConfig *obj, AGReader *r);
ExportFunc void AGServerConfigWriteData(AGServerConfig *obj, AGWriter *w);
#ifndef REMOVE_SYNCHRONIZE_FEATURE
AGServerConfig * AGServerConfigSynchronize(AGServerConfig *agreed,
                                           AGServerConfig *device,
                                           AGServerConfig *desktop,
                                           AGBool preferDesktop);
#endif /* #ifndef REMOVE_SYNCHRONIZE_FEATURE */
    
ExportFunc AGDBConfig *AGServerConfigGetDBConfigNamed(AGServerConfig *obj, 
                                                      char *dbname);
ExportFunc AGDBConfig *AGServerConfigDeleteDBConfigNamed(AGServerConfig *obj,
                                                         char *dbname);
ExportFunc void AGServerConfigAddDBConfig(AGServerConfig *obj, 
                                          AGDBConfig *dbconfig);
ExportFunc AGBool AGServerConfigIsValid(AGServerConfig *obj);
ExportFunc void AGServerConfigResetCookie(AGServerConfig *obj);
ExportFunc void AGServerConfigResetNonce(AGServerConfig *obj);
ExportFunc void AGServerConfigResetHashState(AGServerConfig *obj);
ExportFunc void AGServerConfigResetStates(AGServerConfig *obj);
ExportFunc void AGServerConfigChangePassword(AGServerConfig *obj,
                                             char * newPassword);
ExportFunc void AGServerConfigChangeHashPasswordState(AGServerConfig *obj,
                                                      uint8 newstate);
ExportFunc void AGServerConfigDupDBConfigArray(AGServerConfig *dst,
                                               AGServerConfig *src);
ExportFunc void AGServerConfigFreeDBConfigArray(AGServerConfig *obj);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGSERVERCONFIG_H__ */
