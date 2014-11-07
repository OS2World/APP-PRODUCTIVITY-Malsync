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

#include <Pilot.h>
#include <AGResourceManager.h>
#include <AGUserConfig.h>
#include <AGBufferReader.h>
#include <AGBufferWriter.h>
/* remove this next line ASAP */
#include <AGSyncCommon.h>
#include <AGUtil.h>

typedef struct {
    int32 count;
    int32 iteration;
    void * user;
} AGServerEnumerateState;

typedef AGUserConfig * LPAGUserConfig;

#define DEFAULT_CARDNO ((UInt)0)

static void SetLastError(uint32 err)
{
    //PENDING(klobad) do something with this
}

static AGUserConfig *AGReadUserConfigFromDisk(char *unusedKey) {
    AGBufferReader r;
    LocalID id;
    DmOpenRef dbRef;
    VoidHand handle;
    void *ptr;
    AGUserConfig *userConfig = NULL;

    id = DmFindDatabase(DEFAULT_CARDNO, DEVICE_PROFILE_DB_NAME);
    if(id != 0) {
        dbRef = DmOpenDatabase(DEFAULT_CARDNO, id, dmModeReadOnly);
        handle = DmQueryRecord(dbRef, 0);
        if(!handle) {
            DmCloseDatabase(dbRef);
            return NULL;
        } 
        ptr = MemHandleLock(handle);
        AGBufferReaderInit(&r, (uint8 *)ptr);
        userConfig = AGUserConfigNew();
        if(userConfig) {
            AGUserConfigReadData(userConfig, (AGReader *)&r);
            userConfig->dirty = FALSE;
        }
        AGBufferReaderFinalize(&r);
        MemHandleUnlock(handle);
        DmCloseDatabase(dbRef);
    } 
    return userConfig;
}

static void AGWriteUserConfigToDisk(char *unusedKey, AGUserConfig *userConfig)
{
    AGBufferWriter w;
    LocalID id;
    DmOpenRef dbRef;
    VoidHand handle;
    void *ptr;
    UInt indx = 0;
    Err er;
    int32 size;
    
    id = DmFindDatabase(DEFAULT_CARDNO, DEVICE_PROFILE_DB_NAME);
    if(id == 0) {
        er = DmCreateDatabase(DEFAULT_CARDNO, 
                                DEVICE_PROFILE_DB_NAME, 
                                DEVICE_PROFILE_DB_CREATOR, 
                                DEVICE_PROFILE_DB_TYPE, 
                                false);
        id = DmFindDatabase(DEFAULT_CARDNO, DEVICE_PROFILE_DB_NAME);
    }
    if(id != 0) {
        AGBufferWriterInit(&w, 1024);
        AGUserConfigWriteData(userConfig, (AGWriter *)&w);

        dbRef = DmOpenDatabase(DEFAULT_CARDNO, id, dmModeReadWrite);
        if(DmNumRecords(dbRef) > 0) {
            DmRemoveRecord(dbRef, 0);
        }
        size = AGBufferWriterGetBufferSize(&w);
        handle = DmNewRecord(dbRef, &indx, size);
        ptr = MemHandleLock(handle);
        DmWrite(ptr, 0, AGBufferWriterGetBuffer(&w), 
                            AGBufferWriterGetBufferSize(&w));
        AGBufferWriterFinalize(&w);
        MemHandleUnlock(handle);
        DmReleaseRecord(dbRef, indx, true);
        DmCloseDatabase(dbRef);
        userConfig->dirty = FALSE;
    }
}

/* ----------------------------------------------------------------------------
*/
void AGRefreshDeviceList(void)
{
    /* Stubbed for code sharing. */
}

/* ----------------------------------------------------------------------------
*/
AGDeviceEnumerator * AGDeviceEnumeratorNew(void)
{
    /* Stubbed for code sharing. */
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
char * AGGetNextDevice(AGDeviceEnumerator * enumerator)
{
    /* Stubbed for code sharing. */
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
int32 AGDeviceEnumeratorReset(AGDeviceEnumerator * enumerator)
{
    /* Stubbed for code sharing. */
    return 0;
}

/* ----------------------------------------------------------------------------
*/
void AGDeviceEnumeratorFree(AGDeviceEnumerator * enumerator)
{
    /* Stubbed for code sharing. */
}

/* ----------------------------------------------------------------------------
*/
int32 AGGetDevice(char * key, AGRMDeviceInfo * info)
{
    /* Stubbed for code sharing. */
    if (NULL == key || NULL == info)
        return AG_ERROR_BAD_ARGUMENT;

    return AG_ERROR_NOT_IMPLEMENTED;

}

/* ----------------------------------------------------------------------------
*/
int32 AGGetCurrentDevice(char * key, int32 cbKey)
{
    /* Stubbed for code sharing. */
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
int32 AGSetCurrentDevice(char * key)
{
    /* Stubbed for code sharing. */
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
int32 AGSetCurrentDeviceFromPlatform(int32 type, char * id)
{
    /* Stubbed for code sharing. */
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
int32 AGAddServer(char * key, AGServerConfig * server, AGBool fromDevice)
{
    int32 result = AG_ERROR_NONE;
    LPAGUserConfig user = NULL;    

    if (NULL == server)
        return AG_ERROR_BAD_ARGUMENT;

    user = AGReadUserConfigFromDisk(NULL);
    if (NULL == user)
        user = AGUserConfigNew();
    if (NULL != user) {
        AGUserConfigAddServer(user, AGServerConfigDup(server), fromDevice);
        AGWriteUserConfigToDisk(NULL, user);
        AGUserConfigFree(user);
    } else
        result = AG_ERROR_OUT_OF_MEMORY;

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGRemoveServer(char * key, int32 uid)
{
    int32 result = AG_ERROR_NONE;
    LPAGUserConfig user = NULL;    

    if (0 == uid)
        return AG_ERROR_BAD_ARGUMENT;

    user = AGReadUserConfigFromDisk(NULL);
    if (NULL != user) {
        AGUserConfigRemoveServer(user, uid);
        AGWriteUserConfigToDisk(NULL, user);
        AGUserConfigFree(user);
    } else
        result = AG_ERROR_NOT_FOUND;

    return result;
}

/* ----------------------------------------------------------------------------
*/
static void modifyServerConfig(AGServerConfig * destsc,
                               AGServerConfig * srcsc,
                               AGBool * modFlags)
{
    if (modFlags[AGRM_SMOD_ALL]) {
        AGServerConfigCopy(destsc, srcsc);
        return;
    }
    if (modFlags[AGRM_SMOD_UID]) {
        destsc->uid = srcsc->uid;
    }
    if (modFlags[AGRM_SMOD_STATUS])
        destsc->status = srcsc->status;
    if (modFlags[AGRM_SMOD_SERVERNAME]) {
        CHECKANDFREE(destsc->serverName);
        if (NULL != srcsc->serverName)
            destsc->serverName = strdup(srcsc->serverName);
    }
    if (modFlags[AGRM_SMOD_SERVERPORT])
        destsc->serverPort = srcsc->serverPort;
    if (modFlags[AGRM_SMOD_USERNAME]) {
        CHECKANDFREE(destsc->userName);
        if (NULL != srcsc->userName)
            destsc->userName = strdup(srcsc->userName);
    }
    if (modFlags[AGRM_SMOD_CLEARTEXTPASSWORD]) {
        CHECKANDFREE(destsc->cleartextPassword);
        if (NULL != srcsc->cleartextPassword)
            destsc->cleartextPassword = strdup(srcsc->cleartextPassword);
    }
    if (modFlags[AGRM_SMOD_PASSWORD])
        memcpy(destsc->password, srcsc->password, 16);
    if (modFlags[AGRM_SMOD_DISABLED])
        destsc->disabled = srcsc->disabled;
    if (modFlags[AGRM_SMOD_RESETCOOKIE])
        destsc->resetCookie = srcsc->resetCookie;
    if (modFlags[AGRM_SMOD_FRIENDLYNAME]) {
        CHECKANDFREE(destsc->friendlyName);
        if (NULL != srcsc->friendlyName)
            destsc->friendlyName = strdup(srcsc->friendlyName);
    }
    if (modFlags[AGRM_SMOD_SERVERTYPE]) {
        CHECKANDFREE(destsc->serverType);
        if (NULL != srcsc->serverType)
            destsc->serverType = strdup(srcsc->serverType);
    }
    if (modFlags[AGRM_SMOD_USERURL]) {
        CHECKANDFREE(destsc->userUrl);
        if (NULL != srcsc->userUrl)
            destsc->userUrl = strdup(srcsc->userUrl);
    }
    if (modFlags[AGRM_SMOD_DESCRIPTION]) {
        CHECKANDFREE(destsc->description);
        if (NULL != srcsc->description)
            destsc->description = strdup(srcsc->description);
    }
    if (modFlags[AGRM_SMOD_SERVERURI]) {
        CHECKANDFREE(destsc->serverUri);
        if (NULL != srcsc->serverUri)
            destsc->serverUri = strdup(srcsc->serverUri);
    }
    if (modFlags[AGRM_SMOD_SEQUENCECOOKIELENGTH])
        destsc->sequenceCookieLength = srcsc->sequenceCookieLength;
    if (modFlags[AGRM_SMOD_SEQUENCECOOKIE]) {
        CHECKANDFREE(destsc->sequenceCookie);
        if (srcsc->sequenceCookieLength > 0)
            memcpy(destsc->sequenceCookie,
                srcsc->sequenceCookie,
                srcsc->sequenceCookieLength);
    }
    if (modFlags[AGRM_SMOD_DBCONFIGS]) {
        if (NULL != destsc->dbconfigs)
            AGServerConfigFreeDBConfigArray(destsc);
        AGServerConfigDupDBConfigArray(destsc, srcsc);
    }
    if (modFlags[AGRM_SMOD_NONCE])
        memcpy(destsc->nonce, srcsc->nonce, 16);
    if (modFlags[AGRM_SMOD_SENDDEVICEINFO])
        destsc->sendDeviceInfo = srcsc->sendDeviceInfo;
    if (modFlags[AGRM_SMOD_HASHPASSWORD])
        destsc->hashPassword = srcsc->hashPassword;
    if (modFlags[AGRM_SMOD_CONNECTTIMEOUT])
        destsc->connectTimeout = srcsc->connectTimeout;
    if (modFlags[AGRM_SMOD_WRITETIMEOUT])
        destsc->writeTimeout = srcsc->writeTimeout;
    if (modFlags[AGRM_SMOD_READTIMEOUT])
        destsc->readTimeout = srcsc->readTimeout;
    if (modFlags[AGRM_SMOD_CONNECTSECURELY])
        destsc->connectSecurely = srcsc->connectSecurely;
    if (modFlags[AGRM_SMOD_ALLOWSECURECONNECTION])
        destsc->allowSecureConnection = srcsc->allowSecureConnection;
}

/* ----------------------------------------------------------------------------
*/
int32 AGModifyServer(char * key,
                     int32 uid,
                     AGServerConfig * server,
                     int32 flagStructSize,
                     AGBool * modFlags)
{
    int32 result = AG_ERROR_NONE;
    LPAGUserConfig user = NULL;
    LPAGSC destsc = NULL;

    if (0 == uid
        || NULL == server
        || flagStructSize != AGRM_SMOD_NUMFLAGS
        || NULL == modFlags)
        return AG_ERROR_BAD_ARGUMENT;

    user = AGReadUserConfigFromDisk(NULL);
    if (NULL != user) {
        destsc = AGUserConfigGetServer(user, uid);
        if (NULL != destsc) {
            modifyServerConfig(destsc, server, modFlags);
            AGWriteUserConfigToDisk(NULL, user);
        }
        AGUserConfigFree(user);
    }

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGReplaceServer(char * key, AGServerConfig * server)
{
    AGBool modFlags[AGRM_SMOD_NUMFLAGS];
    modFlags[AGRM_SMOD_ALL] = TRUE;
    return AGModifyServer(key,
        server->uid,
        server,
        AGRM_SMOD_NUMFLAGS,
        modFlags);
}

/* ----------------------------------------------------------------------------
*/
AGServerConfig * AGGetServer(char * key, int32 uid)
{
    LPAGSC result = NULL;
    LPAGUserConfig user = NULL;    

    if (0 == uid) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    user = AGReadUserConfigFromDisk(NULL);
    if (NULL != user) {
        result = AGUserConfigGetServer(user, uid);
        if (NULL != result)
            result = AGServerConfigDup(result);
        AGUserConfigFree(user);
    }
    return result;
}

/* ----------------------------------------------------------------------------
*/
AGServerEnumerator * AGServerEnumeratorNew(char * key)
{
    AGServerEnumerateState * state;

    state = (AGServerEnumerateState *)malloc(sizeof(AGServerEnumerateState));
    if (NULL == state)
        return NULL;

    state->user = AGReadUserConfigFromDisk(NULL);
	if(state->user)
        state->count = AGUserConfigCount((AGUserConfig*)state->user);
	else
        state->count = 0;
    AGServerEnumeratorReset((AGServerEnumerator *)state);

    SetLastError(AG_ERROR_NONE);
    return (AGServerEnumerator *)state;
}

/* ----------------------------------------------------------------------------
*/
int32 AGGetServerCount(AGServerEnumerator * enumerator)
{
    AGServerEnumerateState * state;

    if (NULL == enumerator) {
/*pending(josh): convert to Palm SetLastError(AG_ERROR_BAD_ARGUMENT); */
        return 0;
    }

    state = (AGServerEnumerateState *)enumerator;
    
/*pending(josh): convert to Palm SetLastError(AG_ERROR_NONE); */
    return state->count;
}

/* ----------------------------------------------------------------------------
*/
AGServerConfig * AGGetNextServer(AGServerEnumerator * enumerator)
{
    AGServerEnumerateState * state;
    AGServerConfig *serverConfig;

    if (NULL == enumerator) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    state = (AGServerEnumerateState *)enumerator;

    /* First or later call.  Return one element. */
    if (state->iteration < state->count) {
        SetLastError(AG_ERROR_NONE);
        serverConfig = AGUserConfigGetServerByIndex((AGUserConfig*)state->user,
            state->iteration++);
            
        if (serverConfig != NULL) {
            return AGServerConfigDup(serverConfig);
        } else {
            SetLastError(AG_ERROR_NOT_FOUND);
            return NULL;
        }
    }

    /* Done.  Return NULL. */
    SetLastError(AG_ERROR_NONE);
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
AGServerConfig * AGGetServerAt(AGServerEnumerator * enumerator, int32 n)
{
    AGServerEnumerateState * state;
    AGServerConfig *serverConfig;

    if (NULL == enumerator) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    state = (AGServerEnumerateState *)enumerator;

    if (n >= 0 && n < state->count) {
        SetLastError(AG_ERROR_NONE);
        serverConfig = AGUserConfigGetServerByIndex((AGUserConfig*)state->user, n);
        
        if (serverConfig != NULL)
            return AGServerConfigDup(serverConfig);
    }

    SetLastError(AG_ERROR_NOT_FOUND);
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
int32 AGServerEnumeratorReset(AGServerEnumerator * enumerator)
{
    AGServerEnumerateState * state;

    if (NULL == enumerator)
        return AG_ERROR_BAD_ARGUMENT;

    state = (AGServerEnumerateState *)enumerator;

    state->iteration = 0;
    
    return AG_ERROR_NONE;
}

/* ----------------------------------------------------------------------------
*/
void AGServerEnumeratorFree(AGServerEnumerator * enumerator)
{
    AGServerEnumerateState * state;

    if (NULL == enumerator) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return;
    }

    state = (AGServerEnumerateState *)enumerator;

    if (NULL != (AGUserConfig*)state->user) {
        AGUserConfigFree((AGUserConfig*)state->user);
        state->user = NULL;
    }
    free(state);
    SetLastError(AG_ERROR_NONE);
}

/* ----------------------------------------------------------------------------
*/
AGLocationConfig * AGGetLocationConfig(void)
{
    /* Stubbed for code sharing. */
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
int32 AGModifyLocationConfig(AGLocationConfig * loc,
                             int32 flagStructSize,
                             AGBool * modFlags)
{
    if (NULL == loc
        || flagStructSize != AGRM_LMOD_NUMFLAGS
        || NULL == modFlags)
        return AG_ERROR_BAD_ARGUMENT;

    /* Stubbed for code sharing. */
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
int32 AGSyncProfiles(uint8 * out, uint8 ** in, uint32 * inSize, AGBool pd)
{
    /* Stubbed for code sharing. */
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
int32 AGRetrieveProfileBlob(char * key, uint8 ** in, uint32 * inSize)
{
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
int32 AGStoreProfileBlob(char * key, uint8 * out, uint32 outSize)
{
    return AG_ERROR_NOT_IMPLEMENTED;
}

/* ----------------------------------------------------------------------------
*/
static int32 resetOneServerCookie(char * key, LPAGSC sc)
{
    int32 result = AG_ERROR_NONE;
    AGBool * mod;
    AGServerConfigResetCookie(sc);
    mod = calloc(1, sizeof(AGBool) * AGRM_SMOD_NUMFLAGS);
    if (NULL != mod) {
        mod[AGRM_SMOD_SEQUENCECOOKIELENGTH] = TRUE;
        mod[AGRM_SMOD_SEQUENCECOOKIE] = TRUE;
        result = AGModifyServer(key,
            sc->uid,
            sc,
            AGRM_SMOD_NUMFLAGS,
            mod);
        free(mod);
    } else
        result = AG_ERROR_OUT_OF_MEMORY;
    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGResetServerCookies(char * key, uint32 uid)
{
    LPAGSC sc = NULL;
    int32 result = AG_ERROR_NONE;

    if (0 != uid) {
        sc = AGGetServer(key, uid);
        if (NULL != sc) {
            result = resetOneServerCookie(key, sc);
            AGServerConfigFree(sc);
        } else
            result = AG_ERROR_NOT_FOUND;
    } else {
        AGServerEnumerator * e = AGServerEnumeratorNew(key);
        if (NULL != e) {
            do {
                sc = AGGetNextServer(e);
                if (NULL != sc) {
                    result = resetOneServerCookie(key, sc);
                    AGServerConfigFree(sc);
                }
            } while (NULL != sc);
            AGServerEnumeratorFree(e);
        }
    }
    return result;
}

/* ----------------------------------------------------------------------------
*/
void AGEnableAllServers(char *key)
{
    AGServerEnumerator * e = AGServerEnumeratorNew(key);
    AGBool mod[AGRM_SMOD_NUMFLAGS];
    AGServerConfig *sc;

    if(NULL == e)
        return;

    bzero(mod, sizeof(AGBool) * AGRM_SMOD_NUMFLAGS);
    mod[AGRM_SMOD_DISABLED] = TRUE;
    do {
        sc = AGGetNextServer(e);
        if (NULL != sc) {
            if(sc->disabled && AGServerConfigIsValid(sc)) {
                sc->disabled = FALSE;
                AGModifyServer(key,
                               sc->uid,
                               sc,
                               AGRM_SMOD_NUMFLAGS,
                               mod);
            }
            AGServerConfigFree(sc);
        }
    } while (NULL != sc);
    AGServerEnumeratorFree(e);
}

void AGDisableAllServers(char *key)
{
    AGServerEnumerator * e = AGServerEnumeratorNew(key);
    AGBool mod[AGRM_SMOD_NUMFLAGS];
    AGServerConfig *sc;

    if(NULL == e)
        return;

    bzero(mod, sizeof(AGBool) * AGRM_SMOD_NUMFLAGS);
    mod[AGRM_SMOD_DISABLED] = TRUE;
    do {
        sc = AGGetNextServer(e);
        if (NULL != sc) {
            if(!sc->disabled && AGServerConfigIsValid(sc)) {
                sc->disabled = TRUE;
                AGModifyServer(key,
                               sc->uid,
                               sc,
                               AGRM_SMOD_NUMFLAGS,
                               mod);
            }
            AGServerConfigFree(sc);
        }
    } while (NULL != sc);
    AGServerEnumeratorFree(e);
}
