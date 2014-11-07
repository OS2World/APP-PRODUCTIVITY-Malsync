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

#include <AGResourceManager.h>
#include <AGUserConfig.h>
#include <AGBufferReader.h>
#include <AGBufferWriter.h>
#include <AGUtil.h>
#include <windows.h>

#define AG_MAL_ROOT_KEY     "Software\\Mobile Application Link"
#define AG_MAL_DEVICE_KEY   "Devices"
#define AG_MAL_DEVICE_NAME  "Name"
#define AG_MAL_DEVICE_PATH  "Path"
#define AG_MAL_DEVICE_PREFS "Preferences"
#define AG_MAL_DEVICE_PREFS_SYNCED  "Synchronized Preferences"
#define AG_CE_PARTNER_KEY   "Software\\Microsoft\\Windows CE Services\\Partners"
#define AG_MAL_CE_INSTALLED     "CEInstalled"
#define AG_MAL_PALM_INSTALLED   "PalmInstalled"
#define AG_MAL_CURRENT_DEVICE   "CurrentDevice"

WCHAR * PROFILE_NAME = L"\\windows\\malconfig.cfg";

enum {
    agrmServer = 0,
    agrmLocation,
    agrmMAXNUMOBJECTS
};

typedef enum {
    cePlatform,
    palmPlatform
} platformType;

typedef struct {
    int32 count;
    int32 iteration;
    void * user;
} AGServerEnumerateState;

typedef AGUserConfig * LPAGUserConfig;

static TCHAR * mutexName[agrmMAXNUMOBJECTS] = {
    L"AvantGo_MAL_Server_VHNhbw==",
    L"AvantGo_MAL_Connection_VHNhbw==",
};

/* ----------------------------------------------------------------------------
*/
static HANDLE enterBlockingSection(int32 objectType)
{
    HANDLE result = NULL;

    if (objectType < 0 || objectType >= agrmMAXNUMOBJECTS)
        return NULL;

    result = CreateMutex(NULL, FALSE, mutexName[objectType]);
    if (NULL != result)
        WaitForSingleObject(result, INFINITE);
    return result;
}

/* ----------------------------------------------------------------------------
*/
static void leaveBlockingSection(int32 objectType, HANDLE mutex)
{
    ReleaseMutex(mutex);
    CloseHandle(mutex);
}

/* ----------------------------------------------------------------------------
*/
static TCHAR * getPrefsFilenameForDevice(char * key)
{
    return L"\\windows\\malconfig.cfg";
}

/* ----------------------------------------------------------------------------
    static int32 readDiskFunc(void * info, void * data, int32 len)

    Wrapper for AGReader read function. Reads from file on disk.

*/
static int32 readDiskFunc(void * info, void * data, int32 len)
{
    DWORD bytes = 0;
    ReadFile((HANDLE)info, data, len, &bytes, NULL);
    return bytes;
}

static AGUserConfig * createAndRead(TCHAR * filename,
                                    HANDLE * fileHandle,
                                    AGBool writeAccess)
{
    AGUserConfig * userConfig = NULL;

    *fileHandle =
        CreateFile(filename,
        GENERIC_READ | ((writeAccess) ? GENERIC_WRITE : 0),
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (INVALID_HANDLE_VALUE != *fileHandle) {
        AGReader * reader = AGReaderNew(*fileHandle, readDiskFunc);
        userConfig = AGUserConfigNew();
        AGUserConfigReadData(userConfig, reader);
        AGReaderFree(reader);
    }
    else {
        if (writeAccess)
            *fileHandle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    }

    return userConfig;
}

/* ----------------------------------------------------------------------------
    AGUserConfig * AGReadUserConfigFromDisk(TCHAR * filename)

    Reads userConfig from disk.
*/
AGUserConfig * AGReadUserConfigFromDisk(TCHAR * filename)
{
    HANDLE f;
    AGUserConfig * userConfig;

    userConfig = createAndRead(filename, &f, FALSE);

    if (INVALID_HANDLE_VALUE != f)
        CloseHandle(f);

    return userConfig;
}

/* ----------------------------------------------------------------------------
    static int32 writeDiskFunc(void * info, void * data, int32 len)

    Wrapper for AGWriter write function. Writes to file on disk.

*/
static int32 writeDiskFunc(void * info, void * data, int32 len)
{
    DWORD bytes = 0;
    WriteFile((HANDLE)info, data, len, &bytes, NULL);
    return bytes;
}

/* ----------------------------------------------------------------------------
    void AGWriteUserConfigToDisk(TCHAR * filename, AGUserConfig * userConfig)

    Writes userConfig to disk.

*/
void AGWriteUserConfigToDisk(TCHAR * filename, AGUserConfig * userConfig)
{
    HANDLE f = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 0);
    if (INVALID_HANDLE_VALUE != f) {
        AGWriter * writer = AGWriterNew(f, writeDiskFunc);
        if (NULL != userConfig) {
            SetFilePointer(f, 0, NULL, FILE_BEGIN);
            AGUserConfigWriteData(userConfig, writer);
        }
        AGWriterFree(writer);
        SetEndOfFile(f);
        CloseHandle(f);
    }
}

/* ----------------------------------------------------------------------------
*/
int32 AGAddServer(char * key, AGServerConfig * server, AGBool fromDevice)
{
    HANDLE mutex = NULL;
    int32 result = AG_ERROR_NONE;
    TCHAR * prefsname = NULL;

    if (NULL == server)
        return AG_ERROR_BAD_ARGUMENT;

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname)
        return AG_ERROR_NOT_FOUND;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        LPAGUserConfig user = NULL;    

        user = AGReadUserConfigFromDisk(prefsname);
        if (NULL == user)
            user = AGUserConfigNew();
        if (NULL != user) {
            AGUserConfigAddServer(user, AGServerConfigDup(server), fromDevice);
            AGWriteUserConfigToDisk(prefsname, user);
            AGUserConfigFree(user);
        } else
            result = AG_ERROR_NOT_FOUND;

        leaveBlockingSection(agrmServer, mutex);

    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGRemoveServer(char * key, int32 uid)
{
    HANDLE mutex = NULL;
    int32 result = AG_ERROR_NONE;
    TCHAR * prefsname = NULL;

    if (0 == uid)
        return AG_ERROR_BAD_ARGUMENT;

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname)
        return AG_ERROR_NOT_FOUND;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        LPAGUserConfig user = NULL;    

        user = AGReadUserConfigFromDisk(prefsname);
        if (NULL != user) {
            AGUserConfigRemoveServer(user, uid);
            AGWriteUserConfigToDisk(prefsname, user);
            AGUserConfigFree(user);
        } else
            result = AG_ERROR_NOT_FOUND;

        leaveBlockingSection(agrmServer, mutex);

    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

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
    HANDLE mutex = NULL;
    int32 result = AG_ERROR_NONE;
    int32 deviceresult = AG_ERROR_OUT_OF_MEMORY;
    TCHAR * prefsname = NULL;

    if (0 == uid
        || NULL == server
        || flagStructSize != AGRM_SMOD_NUMFLAGS
        || NULL == modFlags)
        return AG_ERROR_BAD_ARGUMENT;

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname)
        return AG_ERROR_NOT_FOUND;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {
        LPAGUserConfig user = NULL;
        LPAGSC destsc = NULL;
        user = AGReadUserConfigFromDisk(prefsname);
        if (NULL != user) {
            destsc = AGUserConfigGetServer(user, uid);
            if (NULL != destsc) {
                modifyServerConfig(destsc, server, modFlags);
                AGWriteUserConfigToDisk(prefsname, user);
            }
            AGUserConfigFree(user);
        }
        leaveBlockingSection(agrmServer, mutex);
    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

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
    HANDLE mutex = NULL;
    LPAGSC result = NULL;
    int32 deviceresult = AG_ERROR_OUT_OF_MEMORY;
    TCHAR * prefsname = NULL;

    if (0 == uid) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname) {
        SetLastError(AG_ERROR_NOT_FOUND);
        return NULL;
    }

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {
        LPAGUserConfig user = NULL;    
        user = AGReadUserConfigFromDisk(prefsname);
        leaveBlockingSection(agrmServer, mutex);
        CHECKANDFREE(prefsname);
        if (NULL != user) {
            result = AGUserConfigGetServer(user, uid);
            if (NULL != result)
                result = AGServerConfigDup(result);
            AGUserConfigFree(user);
        }
    } else {
        SetLastError(AG_ERROR_SYNCHRONIZATION_FAILED);
        return NULL;
    }

    return result;
}

/* ----------------------------------------------------------------------------
*/
AGServerEnumerator * AGServerEnumeratorNew(char * key)
{
    AGServerEnumerateState * state;
    int32 deviceresult = AG_ERROR_OUT_OF_MEMORY;
    HANDLE mutex = NULL;

    state = (AGServerEnumerateState *)malloc(sizeof(AGServerEnumerateState));
    if (NULL == state)
        return NULL;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        state->user =
            AGReadUserConfigFromDisk(getPrefsFilenameForDevice(NULL));
        state->count = AGUserConfigCount((AGUserConfig*)state->user);
        AGServerEnumeratorReset((AGServerEnumerator *)state);

        leaveBlockingSection(agrmServer, mutex);

    } else
        SetLastError(AG_ERROR_SYNCHRONIZATION_FAILED);

    SetLastError(AG_ERROR_NONE);
    return (AGServerEnumerator *)state;
}

/* ----------------------------------------------------------------------------
*/
int32 AGGetServerCount(AGServerEnumerator * enumerator)
{
    AGServerEnumerateState * state;

    if (NULL == enumerator) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return 0;
    }

    state = (AGServerEnumerateState *)enumerator;
    
    SetLastError(AG_ERROR_NONE);
    return state->count;
}

/* ----------------------------------------------------------------------------
*/
AGServerConfig * AGGetNextServer(AGServerEnumerator * enumerator)
{
    AGServerEnumerateState * state;

    if (NULL == enumerator) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    state = (AGServerEnumerateState *)enumerator;

    /* First or later call.  Return one element. */
    if (state->iteration < state->count) {
        LPAGSC result = NULL;
        SetLastError(AG_ERROR_NONE);
        result = AGUserConfigGetServerByIndex((AGUserConfig*)state->user,
            state->iteration++);
        if (NULL != result)
            return AGServerConfigDup(result);
        else
            return NULL;
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

    if (NULL == enumerator) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    state = (AGServerEnumerateState *)enumerator;

    if (n >= 0 && n < state->count) {
        LPAGSC result = NULL;
        SetLastError(AG_ERROR_NONE);
        result = AGUserConfigGetServerByIndex((AGUserConfig*)state->user, n);
        if (NULL != result)
            return AGServerConfigDup(result);
        return NULL;
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

#if !defined(_WIN32_WCE)
/* ----------------------------------------------------------------------------
*/
AGLocationConfig * AGGetLocationConfig(void)
{
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
int32 AGModifyLocationConfig(AGLocationConfig * loc,
                             int32 flagStructSize,
                             AGBool * modFlags)
{
    HANDLE mutex = NULL;

    if (NULL == loc
        || flagStructSize != AGRM_LMOD_NUMFLAGS
        || NULL == modFlags)
        return AG_ERROR_BAD_ARGUMENT;

    if (NULL != (mutex = enterBlockingSection(agrmLocation))) {
        
        leaveBlockingSection(agrmLocation, mutex);

    } else
        return AG_ERROR_SYNCHRONIZATION_FAILED;

    return AG_ERROR_NONE;
}

/* ----------------------------------------------------------------------------
*/
int32 AGReplaceLocationConfig(AGLocationConfig * loc)
{
    AGBool modFlags[AGRM_LMOD_NUMFLAGS];
    modFlags[AGRM_LMOD_ALL] = TRUE;
    return AGModifyLocationConfig(loc, AGRM_LMOD_NUMFLAGS, modFlags);
}
#endif

/* ----------------------------------------------------------------------------
*/
int32 AGRetrieveProfileBlob(char * key, uint8 ** in, uint32 * inSize)
{
    int32 result = AG_ERROR_NONE;
    HANDLE mutex = NULL;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        HANDLE file = CreateFile(PROFILE_NAME,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (INVALID_HANDLE_VALUE != file) {
            *inSize = GetFileSize(file, NULL);
            if (*inSize > 0) {
                *in = (uint8*)malloc(*inSize);
                if (NULL != *in)
                    ReadFile(file, *in, *inSize, (LPDWORD)inSize, NULL);
            }
            CloseHandle(file);
        } else
            result = GetLastError();

        leaveBlockingSection(agrmServer, mutex);

    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGStoreProfileBlob(char * key, uint8 * out, uint32 outSize)
{
    int32 result = AG_ERROR_NONE;
    HANDLE mutex = NULL;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        HANDLE file = CreateFile(PROFILE_NAME,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (INVALID_HANDLE_VALUE != file) {
            DWORD cbWritten = 0;
            if (outSize > 0)
                WriteFile(file, out, outSize, (LPDWORD)&cbWritten, NULL);
            CloseHandle(file);
        } else
            result = GetLastError();

        leaveBlockingSection(agrmServer, mutex);

    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

    return result;
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
