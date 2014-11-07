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

#include <AGServerConfig.h>
#include <AGBufferReader.h>
#include <AGBufferWriter.h>
#include <AGUtil.h>
#include <AGDigest.h>
#include <AGBase64.h>
#include <AGMD5.h>

#ifndef REMOVE_SYNCHRONIZE_FEATURE
#include <AGSynchronize.h>
#endif

/*  PENDING(klobad) this is also in AGDigest, but
    I wanted to reduce the dependancies for the device */
static AGBool digestIsNull(uint8 a[16])
{
    int i;
    for(i=0;i<16;i++)
        if(a[i])
            return 0;
    return 1;
}

static void digestSetToNull(uint8 a[16])
{
    int i;
    for(i=0;i<16;i++)
        a[i]=0;
}

ExportFunc AGServerConfig *AGServerConfigNew()
{
    AGServerConfig *obj = (AGServerConfig *)malloc(sizeof(AGServerConfig));
    AGServerConfigInit(obj);
    return obj;
}

ExportFunc void AGServerConfigInit(AGServerConfig *obj)
{
    if (NULL == obj)
        return;

    bzero(obj, sizeof(*obj));

    /* A brand-new serverConfig has an unknown hash password state. */
    obj->hashPassword = AG_HASH_PASSWORD_UNKNOWN;

    /* pending(miket):  leave in until all servers send down servertype. */
    obj->serverType = strdup("AvantGo");

    obj->dbconfigs = AGArrayNew(AGUnownedPointerElements, 0);
}

void AGServerConfigFreeDBConfigArray(AGServerConfig *obj)
{
    if (NULL != obj->dbconfigs) {
        int32 n = AGArrayCount(obj->dbconfigs);
        while(n--)
            AGDBConfigFree((AGDBConfig *)AGArrayElementAt(obj->dbconfigs, n));
        AGArrayRemoveAll(obj->dbconfigs);
    }
}

void AGServerConfigDupDBConfigArray(AGServerConfig *dst,
                                    AGServerConfig *src)
{
    int32 i, n;
    n = AGArrayCount(src->dbconfigs);
    for (i = 0; i < n; ++i)
        AGArrayAppend(dst->dbconfigs,
            AGDBConfigDup((AGDBConfig *)AGArrayElementAt(src->dbconfigs, i)));
}

ExportFunc void AGServerConfigFinalize(AGServerConfig *obj)
{
    if (NULL != obj) {
        CHECKANDFREE(obj->serverName);
        CHECKANDFREE(obj->userName);
        CHECKANDFREE(obj->cleartextPassword);
        CHECKANDFREE(obj->friendlyName);
        CHECKANDFREE(obj->serverType);
        CHECKANDFREE(obj->userUrl);
        CHECKANDFREE(obj->description);
        CHECKANDFREE(obj->serverUri);
        CHECKANDFREE(obj->sequenceCookie);
        if(obj->dbconfigs) {
            AGServerConfigFreeDBConfigArray(obj);
            AGArrayFree(obj->dbconfigs);
        }
        CHECKANDFREE(obj->reserved);
        bzero(obj, sizeof(*obj));
    }
}

ExportFunc void AGServerConfigFree(AGServerConfig *obj)
{
    if (NULL != obj) {
        AGServerConfigFinalize(obj);
        free(obj);
    }
}

ExportFunc AGServerConfig *AGServerConfigCopy(AGServerConfig *dst, 
                                              AGServerConfig *src)
{
    if (NULL == dst || NULL == src)
        return NULL;

    dst->uid = src->uid;
    dst->status = src->status;

    CHECKANDFREE(dst->serverName);
    if (NULL != src->serverName)
        dst->serverName = strdup(src->serverName);
    dst->serverPort = src->serverPort;
    CHECKANDFREE(dst->userName);
    if (NULL != src->userName)
        dst->userName = strdup(src->userName);
    CHECKANDFREE(dst->cleartextPassword);
    if (NULL != src->cleartextPassword)
        dst->cleartextPassword = strdup(src->cleartextPassword);
    memcpy(dst->password, src->password, 16);
    dst->disabled = src->disabled;
    dst->resetCookie = src->resetCookie;
    dst->notRemovable = src->notRemovable;

    CHECKANDFREE(dst->friendlyName);
    if (NULL != src->friendlyName)
        dst->friendlyName = strdup(src->friendlyName);
    CHECKANDFREE(dst->serverType);
    if (NULL != src->serverType)
        dst->serverType = strdup(src->serverType);
    CHECKANDFREE(dst->userUrl);
    if (NULL != src->userUrl)
        dst->userUrl = strdup(src->userUrl);
    CHECKANDFREE(dst->description);
    if (NULL != src->description)
        dst->description = strdup(src->description);
    CHECKANDFREE(dst->serverUri);
    if (NULL != src->serverUri)
        dst->serverUri = strdup(src->serverUri);
    dst->sequenceCookieLength = src->sequenceCookieLength;
    CHECKANDFREE(dst->sequenceCookie);
    if (NULL != src->sequenceCookie) {
        dst->sequenceCookie = (uint8*)malloc(src->sequenceCookieLength);
        if (NULL != dst->sequenceCookie)
            memcpy(dst->sequenceCookie,
                src->sequenceCookie,
                src->sequenceCookieLength);
    }

    AGServerConfigFreeDBConfigArray(dst);
    AGServerConfigDupDBConfigArray(dst, src);

    memcpy(dst->nonce, src->nonce, 16);

    dst->sendDeviceInfo = src->sendDeviceInfo;

    dst->hashPassword = src->hashPassword;
    dst->connectTimeout = src->connectTimeout;
    dst->writeTimeout = src->writeTimeout;
    dst->readTimeout = src->readTimeout;

    dst->connectSecurely = src->connectSecurely;
    dst->allowSecureConnection = src->allowSecureConnection;

    dst->expansion1 = src->expansion1;
    dst->expansion2 = src->expansion2;
    dst->expansion3 = src->expansion3;
    dst->expansion4 = src->expansion4;

    dst->reservedLen = src->reservedLen;
    CHECKANDFREE(dst->reserved);
    if (NULL != src->reserved) {
        dst->reserved = malloc(src->reservedLen);
        if (NULL != dst->reserved)
            memcpy(dst->reserved, src->reserved, src->reservedLen);
    }
    return dst;
}

ExportFunc AGServerConfig *AGServerConfigDup(AGServerConfig *src)
{
    return AGServerConfigCopy(AGServerConfigNew(), src);
}

static void getDBConfigNamed(AGServerConfig *obj,  char *dbname,
                             AGDBConfig **dbconfig, uint32 *index)
{
    AGDBConfig *result = NULL;
    int32 i;
    int32 n;

    if(dbconfig)
        *dbconfig = NULL;
    if(index)
        *index = -1;

    if (NULL == obj->dbconfigs || NULL == dbname)
        return;

    n = AGArrayCount(obj->dbconfigs);
    for (i = 0; i < n; i++) {
        result = (AGDBConfig *)AGArrayElementAt(obj->dbconfigs, i);
        //pending(miket): Are database names case-sensitive?
        if (!strcmp(result->dbname, dbname)) {
            if(dbconfig)
                *dbconfig = result;
            if(index)
                *index = i;
            return;
        }
    }
    return;
}

ExportFunc AGDBConfig *AGServerConfigGetDBConfigNamed(AGServerConfig *obj, 
                                                    char *dbname)
{
    AGDBConfig *result = NULL;
    getDBConfigNamed(obj, dbname, &result, NULL);
    return result;
}

ExportFunc AGDBConfig *AGServerConfigDeleteDBConfigNamed(AGServerConfig *obj, 
                                                         char *dbname)
{
    uint32 i;
    AGDBConfig *db;

    getDBConfigNamed(obj, dbname, &db, &i);

    if (i == -1 || db == NULL)
        return NULL;

    AGArrayRemoveAt(obj->dbconfigs, i);
    return db;
}

ExportFunc void AGServerConfigAddDBConfig(AGServerConfig *obj, 
                                          AGDBConfig *dbconfig)
{
    AGDBConfig *oldConfig;

    oldConfig = AGServerConfigDeleteDBConfigNamed(obj, dbconfig->dbname);
    if(oldConfig)
        AGDBConfigFree(oldConfig);
    AGArrayAppend(obj->dbconfigs, dbconfig);
}

#ifndef REMOVE_SYNCHRONIZE_FEATURE
AGServerConfig * AGServerConfigSynchronize(AGServerConfig *agreed,
                                           AGServerConfig *device,
                                           AGServerConfig *desktop,
                                           AGBool preferDesktop)
{
    AGServerConfig * result;
    AGServerConfig * cw;

    cw = preferDesktop ? desktop : device;
    
    result = AGServerConfigNew();

    if (NULL != result) {

        result->uid = AGSynchronizeInt32(agreed->uid,
            device->uid,
            desktop->uid);

        result->status = (AGRecordStatus)AGSynchronizeInt32(
            agreed->status,
            device->status,
            desktop->status);

        CHECKANDFREE(result->serverName);
        result->serverName = AGSynchronizeString(agreed->serverName,
            device->serverName,
            desktop->serverName);

        result->serverPort = AGSynchronizeInt16(agreed->serverPort,
            device->serverPort,
            desktop->serverPort);

        CHECKANDFREE(result->userName);
        result->userName = AGSynchronizeString(agreed->userName,
            device->userName,
            desktop->userName);

        CHECKANDFREE(result->cleartextPassword);
        result->cleartextPassword =
            AGSynchronizeString(agreed->cleartextPassword,
                device->cleartextPassword,
                desktop->cleartextPassword);

        AGSynchronizeStackStruct((void*)result->password,
            (void*)agreed->password,
            (void*)device->password,
            (void*)desktop->password,
            16);

        result->disabled = AGSynchronizeBoolean(agreed->disabled,
            device->disabled,
            desktop->disabled);

        result->resetCookie = AGSynchronizeBoolean(agreed->resetCookie,
            device->resetCookie,
            desktop->resetCookie);

        result->notRemovable = AGSynchronizeBoolean(agreed->notRemovable,
            device->notRemovable,
            desktop->notRemovable);

        CHECKANDFREE(result->friendlyName);
        result->friendlyName = AGSynchronizeString(agreed->friendlyName,
            device->friendlyName,
            desktop->friendlyName);

        CHECKANDFREE(result->serverType);
        result->serverType = AGSynchronizeString(agreed->serverType,
            device->serverType,
            desktop->serverType);

        CHECKANDFREE(result->userUrl);
        result->userUrl = AGSynchronizeString(agreed->userUrl,
            device->userUrl,
            desktop->userUrl);

        CHECKANDFREE(result->description);
        result->description = AGSynchronizeString(agreed->description,
            device->description,
            desktop->description);

        CHECKANDFREE(result->serverUri);
        result->serverUri = AGSynchronizeString(agreed->serverUri,
            device->serverUri,
            desktop->serverUri);

        CHECKANDFREE(result->sequenceCookie);
        result->sequenceCookieLength = 0;
        if (!result->resetCookie) {
            if (cw->sequenceCookieLength > 0) {
                result->sequenceCookie =
                    (uint8*)malloc(cw->sequenceCookieLength);
                if (NULL != result->sequenceCookie) {
                    memcpy(result->sequenceCookie,
                        cw->sequenceCookie,
                        cw->sequenceCookieLength);
                    result->sequenceCookieLength = cw->sequenceCookieLength;
                }
            }
        }
        /*  At this point, we have already reset the cookie if we were
            supposed to do so. Clear the flag. */
        result->resetCookie = FALSE;

        AGServerConfigFreeDBConfigArray(result);
        if (NULL != cw->dbconfigs)
            AGServerConfigDupDBConfigArray(result, cw);

        if (!device->resetCookie && !desktop->resetCookie)
            AGSynchronizeStackStruct((void*)result->nonce,
                (void*)agreed->nonce,
                (void*)device->nonce,
                (void*)desktop->nonce,
                16);
        else
            digestSetToNull((uint8*)result->nonce);

        result->hashPassword = AGSynchronizeInt8(agreed->hashPassword,
            device->hashPassword,
            desktop->hashPassword);

        result->sendDeviceInfo = AGSynchronizeBoolean(agreed->sendDeviceInfo,
            device->sendDeviceInfo,
            desktop->sendDeviceInfo);

        result->connectTimeout = AGSynchronizeBoolean(agreed->connectTimeout,
            device->connectTimeout, desktop->connectTimeout);

        result->writeTimeout = AGSynchronizeBoolean(agreed->writeTimeout,
            device->writeTimeout, desktop->writeTimeout);

        result->readTimeout = AGSynchronizeBoolean(agreed->readTimeout,
            device->readTimeout, desktop->readTimeout);

        result->connectSecurely =
            AGSynchronizeBoolean(agreed->connectSecurely,
                device->connectSecurely,
                desktop->connectSecurely);

        result->allowSecureConnection =
            AGSynchronizeBoolean(agreed->allowSecureConnection,
                device->allowSecureConnection,
                desktop->allowSecureConnection);

        CHECKANDFREE(result->reserved);
        result->reservedLen = 0;
        AGSynchronizeData(&result->reserved,
            &result->reservedLen,
            agreed->reserved,
            agreed->reservedLen,
            device->reserved,
            device->reservedLen,
            desktop->reserved,
            desktop->reservedLen);
    }


    return result;

}
#endif /*#ifndef REMOVE_SYNCHRONIZE_FEATURE */

//PENDING(klobad) confirm these are valid validity checks
ExportFunc AGBool AGServerConfigIsValid(AGServerConfig *obj)
{
    if(obj == NULL)
        return FALSE;

    if(obj->friendlyName == NULL)
        return FALSE;

    if(0 == strlen(obj->friendlyName))
        return FALSE;

    if(obj->serverName == NULL)
        return FALSE;

    if(0 == strlen(obj->serverName))
        return FALSE;

    if(obj->serverPort == 0)
        return FALSE;

    return TRUE;
}

void AGServerConfigResetCookie(AGServerConfig *obj)
{
    obj->sequenceCookieLength = 0;
    if (NULL != obj->sequenceCookie) {
        free(obj->sequenceCookie);
        obj->sequenceCookie = NULL;
    }
}

void AGServerConfigResetNonce(AGServerConfig *obj)
{
    // Wanted to remove the depandancy on AGDigestSetToNull
    // so I didn't need AGDigest on the client.
    digestSetToNull(obj->nonce);
}

void AGServerConfigResetHashState(AGServerConfig *obj)
{
    digestSetToNull(obj->password);
    obj->hashPassword = AG_HASH_PASSWORD_UNKNOWN;
}

void AGServerConfigResetStates(AGServerConfig *obj)
{
    AGServerConfigResetNonce(obj);
    AGServerConfigResetCookie(obj);
    AGServerConfigResetHashState(obj);
}

void AGServerConfigChangePassword(AGServerConfig *obj,
                                  char * newPassword)
{
    if (NULL == newPassword || 0 == strlen(newPassword)) {
        CHECKANDFREE(obj->cleartextPassword);
        digestSetToNull(obj->password);
        return;
    }

    if (AG_HASH_PASSWORD_YES == obj->hashPassword) {
        AGMd5((uint8*)newPassword, strlen(newPassword), obj->password);
    } else {
        CHECKANDFREE(obj->cleartextPassword);
        obj->cleartextPassword = AGBase64Encode((uint8*)newPassword, 0);
    }
}

void AGServerConfigChangeHashPasswordState(AGServerConfig *obj,
                                           uint8 newstate)
{
    char * buf = NULL;
    int32 len = 0;

    /* If we're already hashed, nothing can change, so ignore the
    new state and return. */
    if (AG_HASH_PASSWORD_YES == obj->hashPassword)
        return;

    /* This shouldn't ever happen because the state should not go
    from a known state to an unknown one. */
    if (AG_HASH_PASSWORD_UNKNOWN == newstate)
        return;

    /* By this point, we know we're switching from an unknown state to
    a known state, so record that. */
    obj->hashPassword = newstate;

    /* If the known state is not to hash, we're done. */
    if (AG_HASH_PASSWORD_NO == newstate)
        return;

    /* If the known state is to hash, then convert the current cleartext
    password to a hashed password. */
    if (NULL == obj->cleartextPassword) {
        /*  We've purposely commented out the next line for a hack
            reason. If a user decides to edit the server address of
            an existing, non-cleartext server profile, then the UI
            resets hashPassword to AG_HASH_PASSWORD_UNKNOWN. However,
            the user may or may not re-enter the password after that.
            So the state is: cleartextPassword is NULL, password
            contains the hashed password, and hashPassword is 
            AG_HASH_PASSWORD_UNKNOWN. Then the server asks to set
            hashPassword to AG_HASH_PASSWORD_YES, which means that
            we would convert the cleartextPassword field to a hash
            value and store it in password. Problem is, there is no
            cleartext password -- there is only the hashed password.
            The hack is to assume that if cleartextPassword is NULL,
            then we're in this state, and we'd rather just reset the
            hashPassword field to the YES value and not touch either
            password or cleartextPassword. The time this won't work is
            when a user does the following:

            1. Edit an existing profile that has a non-blank password
            so that the server address has changed.

            2. Change the password on the server to a blank password.

            3. Set the password on the desktop/device to blank.

            4. Sync.

            What the user has to do in this case is either create a
            brand-new profile (which most users will do anyway) or
            reverse steps 3 and 4 and add step 5, which is sync again.
        */
        /*digestSetToNull(obj->password);*/
        return;
    }

    buf = (char*)AGBase64Decode(obj->cleartextPassword, &len);

    AGMd5((uint8*)buf, len, obj->password);

    CHECKANDFREE(obj->cleartextPassword);
}

#define agSIGNATURE_HIGH        (0xDE)
#define agSIGNATURE_LOW         (0xAA)
#define agVERSION_MAJ_0         (0)
#define agVERSION_MIN_0         (0)
#define agCURRENT_MAJ_VER       agVERSION_MAJ_0
#define agCURRENT_MIN_VER       agVERSION_MIN_0

#define agFLAG_RESET_COOKIE     (0x00000001)
#define agFLAG_NOT_REMOVABLE    (0x00000002)

int32 AGServerConfigReadData(AGServerConfig *obj, AGReader *r)
{
    int32 i, n;
    AGDBConfig *dbconfig;
    int32 majver, minver;
    int32 flags;

    if (AGReadInt16(r) != ((agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW))
        return AG_ERROR_INVALID_SIGNATURE;

    majver = AGReadCompactInt(r);
    minver = AGReadCompactInt(r);

    obj->uid = AGReadCompactInt(r);
    obj->status = (AGRecordStatus)AGReadCompactInt(r);
    CHECKANDFREE(obj->serverName);
    obj->serverName = AGReadCString(r);
    obj->serverPort = AGReadCompactInt(r);

    CHECKANDFREE(obj->userName);
    obj->userName = AGReadCString(r);
    CHECKANDFREE(obj->cleartextPassword);
    obj->cleartextPassword = AGReadCString(r);

    if (AGReadInt8(r))
        AGReadBytes(r, obj->password, 16);
    if (AGReadInt8(r))
        AGReadBytes(r, obj->nonce, 16);

    obj->disabled = AGReadBoolean(r);

    CHECKANDFREE(obj->friendlyName);
    obj->friendlyName = AGReadCString(r);
    CHECKANDFREE(obj->serverType);
    obj->serverType = AGReadCString(r);
    CHECKANDFREE(obj->userUrl);
    obj->userUrl = AGReadCString(r);
    CHECKANDFREE(obj->description);
    obj->description = AGReadCString(r);
    CHECKANDFREE(obj->serverUri);
    obj->serverUri = AGReadCString(r);

    obj->sequenceCookieLength = AGReadCompactInt(r);
    CHECKANDFREE(obj->sequenceCookie);
    if (obj->sequenceCookieLength > 0) {
        obj->sequenceCookie =
            (uint8*)malloc(obj->sequenceCookieLength);
        AGReadBytes(r, obj->sequenceCookie, obj->sequenceCookieLength);
    }

    n = AGReadCompactInt(r);
    for (i = 0; i < n; i++) {
        dbconfig = AGDBConfigNew(NULL, AG_SENDALL_CFG, FALSE, 0, NULL, NULL);
        AGDBConfigReadData(dbconfig, r);
        AGArrayAppend(obj->dbconfigs, dbconfig);
    }
    obj->sendDeviceInfo = AGReadBoolean(r);
    obj->hashPassword = AGReadInt8(r);
    obj->connectTimeout = AGReadCompactInt(r);
    obj->writeTimeout = AGReadCompactInt(r);
    obj->readTimeout = AGReadCompactInt(r);
    obj->connectSecurely = AGReadBoolean(r);
    obj->allowSecureConnection = AGReadBoolean(r);

    flags = AGReadCompactInt(r);
    obj->resetCookie = (flags & agFLAG_RESET_COOKIE);
    obj->notRemovable = (flags & agFLAG_NOT_REMOVABLE);

    obj->expansion1 = AGReadCompactInt(r);
    obj->expansion2 = AGReadCompactInt(r);
    obj->expansion3 = AGReadCompactInt(r);
    obj->expansion4 = AGReadCompactInt(r);

    obj->reservedLen = AGReadCompactInt(r);
    CHECKANDFREE(obj->reserved);
    if (obj->reservedLen > 0) {
        obj->reserved = malloc(obj->reservedLen);
        AGReadBytes(r, obj->reserved, obj->reservedLen);
    }

    if (majver > agCURRENT_MAJ_VER)
        return AG_ERROR_UNKNOWN_VERSION;

    return AG_ERROR_NONE;
}

void AGServerConfigWriteData(AGServerConfig *obj, AGWriter *w)
{
    int32 i, n;
    AGDBConfig *dbconfig;
    int32 flags = 0;

    AGWriteInt16(w, ((agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW));
    AGWriteCompactInt(w, agCURRENT_MAJ_VER);
    AGWriteCompactInt(w, agCURRENT_MIN_VER);
    AGWriteCompactInt(w, obj->uid);
    AGWriteCompactInt(w, (uint32)obj->status);
    AGWriteCString(w, obj->serverName);
    AGWriteCompactInt(w, obj->serverPort);

    AGWriteCString(w, obj->userName);
    AGWriteCString(w, obj->cleartextPassword);

    if (digestIsNull(obj->password))
        AGWriteInt8(w, 0);
    else {
        AGWriteInt8(w, 16);
        AGWriteBytes(w, obj->password, 16);
    }

    if (digestIsNull(obj->nonce))
        AGWriteInt8(w, 0);
    else {
        AGWriteInt8(w, 16);
        AGWriteBytes(w, obj->nonce, 16);
    }

    AGWriteBoolean(w, obj->disabled);

    AGWriteCString(w, obj->friendlyName);
    AGWriteCString(w, obj->serverType);
    AGWriteCString(w, obj->userUrl);
    AGWriteCString(w, obj->description);
    AGWriteCString(w, obj->serverUri);

    AGWriteCompactInt(w, obj->sequenceCookieLength);
    if (obj->sequenceCookieLength > 0) {
        AGWriteBytes(w, obj->sequenceCookie, 
            obj->sequenceCookieLength);
    }

    n = AGArrayCount(obj->dbconfigs);
    AGWriteCompactInt(w, n);
    for (i = 0; i < n; i++) {
        dbconfig = (AGDBConfig*)AGArrayElementAt(obj->dbconfigs, i);
        AGDBConfigWriteData(dbconfig, w);
    }

    AGWriteBoolean(w, obj->sendDeviceInfo);
    AGWriteInt8(w, obj->hashPassword);
    AGWriteCompactInt(w, obj->connectTimeout);
    AGWriteCompactInt(w, obj->writeTimeout);
    AGWriteCompactInt(w, obj->readTimeout);

    AGWriteBoolean(w, obj->connectSecurely);
    AGWriteBoolean(w, obj->allowSecureConnection);

    flags |= obj->resetCookie ? agFLAG_RESET_COOKIE : 0;
    flags |= obj->notRemovable ? agFLAG_NOT_REMOVABLE : 0;
    AGWriteCompactInt(w, flags);

    AGWriteCompactInt(w, obj->expansion1);
    AGWriteCompactInt(w, obj->expansion2);
    AGWriteCompactInt(w, obj->expansion3);
    AGWriteCompactInt(w, obj->expansion4);

    AGWriteCompactInt(w, obj->reservedLen);
    if (obj->reservedLen > 0)
        AGWriteBytes(w, obj->reserved, obj->reservedLen);
}