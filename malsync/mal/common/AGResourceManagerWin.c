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
#include <AGSyncCommon.h>
#include <AGShlapi.h>
#include <AGUtil.h>
#include <AGDesktopInfoPalm.h>
#include <AGDesktopInfoWinCE.h>
#include <windows.h>
#include <crtdbg.h>

#define AG_MAL_ROOT_KEY     "Software\\Mobile Application Link"
#define AG_MAL_DEVICE_KEY   "Devices"
#define AG_MAL_DEVICE_NAME  "Name"
#define AG_MAL_DEVICE_PATH  "Path"
#define AG_MAL_DEVICE_PREFS "Preferences"
#define AG_MAL_DEVICE_PREFS_SYNCED  "Synchronized Preferences"
#define AG_MAL_PATH_DEFVAL  "MAL"
#define AG_MAL_PREFS_DEFVAL "MALConfig.cfg"
#define AG_MAL_PREFS_SYNCED_DEFVAL  "MALConfigSync.cfg"
#define AG_MAL_PREFS_DEFVAL_OLD "MALConfig.dat"
#define AG_MAL_PREFS_SYNCED_DEFVAL_OLD  "MALConfigSync.dat"
#define AG_CE_PARTNER_KEY   "Software\\Microsoft\\Windows CE Services\\Partners"
#define AG_MAL_CE_INSTALLED         "CEInstalled"
#define AG_MAL_PALM_INSTALLED       "PalmInstalled"
#define AG_MAL_DESKTOP_INSTALLED    "DesktopInstalled"
#define AG_MAL_CURRENT_DEVICE   "CurrentDevice"
#define AG_MAL_DESKTOP_NAME   "Desktop Name"
#define AG_MAL_DESKTOP_PATH   "Desktop Path"
#define agMALDesktopDefaultName "Desktop Profile"

enum {
    agrmServer = 0,
    agrmLocation,
    agrmDevice,
    agrmMAXNUMOBJECTS
};

typedef enum {
    cePlatform,
    palmPlatform,
    desktopPlatform
} platformType;

typedef struct {
    AGRMDeviceInfo pub;
    char * szPath;      /* Full path to our prefs folder for this device */
    int32 cbPath;       /* size of array pointed to by szPath */
    char * szPreferences;   /* Full path to our prefs folder for this device */
    int32 cbPreferences;    /* size of array pointed to by szPreferences */
    char * szPreferencesSynchronized;   /* Synchronized prefs path */
    int32 cbPreferencesSynchronized;    /* size of array */
} AGRMDeviceInfoPriv;

typedef struct {
    int32 count;
    int32 iteration;
    void * user;
} AGServerEnumerateState;

typedef struct {
    int32 count;
    int32 iteration;
    char ** devices;
} AGDeviceEnumerateState;

typedef AGUserConfig * LPAGUC;

static char * mutexName[agrmMAXNUMOBJECTS] = {
    "AvantGo_MAL_Server_VHNhbw==",
    "AvantGo_MAL_Connection_VHNhbw==",
    "AvantGo_MAL_Device_VHNhbw==",
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
    _ASSERT(NULL != mutex);
    ReleaseMutex(mutex);
    CloseHandle(mutex);
}

/* ----------------------------------------------------------------------------
*/
static AGBool isPlatformInstalled(platformType platform)
{
    AGBool result = FALSE;
    HKEY hkey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        AG_MAL_ROOT_KEY,
        0,
        KEY_READ,
        &hkey)) {

        DWORD size = sizeof(DWORD);
        DWORD dwType;
        DWORD value = 0;
        char * resid = NULL;

        switch (platform) {
            case cePlatform:
                resid = AG_MAL_CE_INSTALLED;
                break;
            case palmPlatform:
                resid = AG_MAL_PALM_INSTALLED;
                break;
            case desktopPlatform:
                resid = AG_MAL_DESKTOP_INSTALLED;
                break;
        }

        if (NULL != resid)
            RegQueryValueEx(hkey, resid, 0, &dwType, (LPBYTE)&value, &size);

        result = (value != 0);

        RegCloseKey(hkey);

    }

    return result;
}

/* ----------------------------------------------------------------------------
    Win9x is simpler because RegDeleteKey() works no matter whether the key
    has subkeys. NT can delete only keys containing no subkeys.  For code
    simplicity, we do the full delete for both platforms here.
*/
static void resetDeviceList()
{
    AGBool result = FALSE;
    HKEY hkey;
    DWORD dwIndex;
    AGArray * deletes;

    deletes = AGArrayNew(AGOwnedStringElements, 0);
    if (NULL == deletes)
        return;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY,
        0,
        KEY_ALL_ACCESS,
        &hkey)) {

        int n;

        /* A device list already exists.  Walk it and delete everything
        we find. */

        dwIndex = 0;
        do {
        
            TCHAR subKeyName[MAX_PATH];
            DWORD size;
            FILETIME filetime;

            size = MAX_PATH;
            result = RegEnumKeyEx(hkey,
                dwIndex,
                subKeyName,
                &size,
                NULL,
                NULL,
                NULL,
                &filetime);

            ++dwIndex;
        
            if (ERROR_SUCCESS != result)
                continue;
        
            AGArrayAppend(deletes, strdup(subKeyName));

        } while (ERROR_SUCCESS == result);

        /*  The walking and the deleting must happen separately because
            Windows chokes and dies if you delete what you're enumerating.
        */
        n = AGArrayCount(deletes);
        while (n--)
            RegDeleteKey(hkey, (char*)AGArrayElementAt(deletes, n));

        RegCloseKey(hkey);

    } else {

        /* There was no device list. Create an empty one. */

        HKEY key = NULL;
        DWORD disposition;

        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
            AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &key,
            &disposition)) {

            RegCloseKey(key);

        }

    }

    AGArrayFree(deletes);
}

/* ----------------------------------------------------------------------------
*/
static void createPalmList(char * malpath, char * prefsname, char * sprefsname)
{
    int32 i, n;
    AGDesktopInfoPalm * desktopInfo;
    
    desktopInfo = AGDesktopInfoPalmNew();
    if (NULL == desktopInfo)
        return;

    n = AGDesktopInfoPalmGetUserCount(desktopInfo);

    for (i = 0; i < n; ++i) {

        TCHAR username[MAX_PATH];
        TCHAR tempdir[MAX_PATH];
        TCHAR tempbuf[MAX_PATH];
        short bufSize;
        int   intBufSize; /* aaaaaaaaaaaaaarrrrrgh */

        bufSize = MAX_PATH;
        AGDesktopInfoPalmGetUsername(desktopInfo, i, username, &bufSize);
        if (0 != strlen(username)) {

            TCHAR keyName[MAX_PATH];
            HKEY hkey;
            LONG result;
            DWORD disposition;

            strcpy(keyName, AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY"\\p>");
            strcat(keyName, username);

            result = RegCreateKeyEx(HKEY_CURRENT_USER,
                keyName,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &hkey,
                &disposition);

            if (ERROR_SUCCESS == result) {

                RegSetValueEx(hkey,
                    AG_MAL_DEVICE_NAME,
                    0,
                    REG_SZ,
                    username,
                    strlen(username) + 1);

                intBufSize = MAX_PATH;
                AGDesktopInfoPalmGetUserDirectory(desktopInfo,
                    username,
                    tempdir,
                    &intBufSize);

                RegSetValueEx(hkey,
                    AG_MAL_DEVICE_PATH,
                    0,
                    REG_SZ,
                    tempdir,
                    strlen(tempdir) + 1);

                PathCombine(tempbuf, tempdir, malpath);
                CreateDirectory(tempbuf, NULL);
                PathAppend(tempbuf, prefsname);

                RegSetValueEx(hkey,
                    AG_MAL_DEVICE_PREFS,
                    0,
                    REG_SZ,
                    tempbuf,
                    strlen(tempbuf) + 1);

                PathCombine(tempbuf, tempdir, malpath);
                PathAppend(tempbuf, sprefsname);

                RegSetValueEx(hkey,
                    AG_MAL_DEVICE_PREFS_SYNCED,
                    0,
                    REG_SZ,
                    tempbuf,
                    strlen(tempbuf) + 1);

                RegCloseKey(hkey);
            }

        }

    }

    AGDesktopInfoPalmFree(desktopInfo);
}

/* ----------------------------------------------------------------------------
*/
static void createCEList(char * malpath, char * prefsname, char * sprefsname)
{
    HKEY ceRootKey, partnerKey;
    LONG result;
    DWORD dwIndex;
    BOOL foundDevice;

    result = RegOpenKeyEx(HKEY_CURRENT_USER,
        AG_CE_PARTNER_KEY,
        0,
        KEY_READ,
        &ceRootKey);

    if (ERROR_SUCCESS != result)
        return;

    dwIndex = 0;
    do {
        
        TCHAR subKeyName[MAX_PATH];
        TCHAR path[MAX_PATH];
        TCHAR tempbuf[MAX_PATH];
        DWORD size;
        TCHAR deviceName[MAX_PATH];
        DWORD dwType;
        FILETIME filetime;

        foundDevice = FALSE;

        size = MAX_PATH;
        result = RegEnumKeyEx(ceRootKey,
            dwIndex,
            subKeyName,
            &size,
            NULL,
            NULL,
            NULL,
            &filetime);

        ++dwIndex;
        
        if (ERROR_SUCCESS != result)
            continue;
        
        result = RegOpenKeyEx(ceRootKey,
            subKeyName,
            0,
            KEY_READ,
            &partnerKey);

        if (ERROR_SUCCESS != result)
            continue;

        foundDevice = TRUE;

        size = MAX_PATH;
        result = RegQueryValueEx(partnerKey,
            "DataFolder",
            0,
            &dwType,
            (LPBYTE)path,
            &size);

        if (ERROR_SUCCESS == result) {

            size = MAX_PATH;
            result = RegQueryValueEx(partnerKey,
                "DisplayName",
                0,
                &dwType,
                (LPBYTE)deviceName,
                &size);

            if (ERROR_SUCCESS == result) {

                TCHAR keyName[MAX_PATH];
                HKEY hkey;
                LONG result;
                DWORD disposition;

                strcpy(keyName, AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY"\\c>");
                strcat(keyName, deviceName);

                result = RegCreateKeyEx(HKEY_CURRENT_USER,
                    keyName,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hkey,
                    &disposition);

                if (ERROR_SUCCESS == result) {

                    RegSetValueEx(hkey,
                        AG_MAL_DEVICE_NAME,
                        0,
                        REG_SZ,
                        deviceName,
                        strlen(deviceName) + 1);

                    RegSetValueEx(hkey,
                        AG_MAL_DEVICE_PATH,
                        0,
                        REG_SZ,
                        path,
                        strlen(path) + 1);

                    PathCombine(tempbuf, path, malpath);
                    CreateDirectory(tempbuf, NULL);
                    PathAppend(tempbuf, prefsname);

                    RegSetValueEx(hkey,
                        AG_MAL_DEVICE_PREFS,
                        0,
                        REG_SZ,
                        tempbuf,
                        strlen(tempbuf) + 1);

                    PathCombine(tempbuf, path, malpath);
                    PathAppend(tempbuf, sprefsname);

                    RegSetValueEx(hkey,
                        AG_MAL_DEVICE_PREFS_SYNCED,
                        0,
                        REG_SZ,
                        tempbuf,
                        strlen(tempbuf) + 1);

                    RegCloseKey(hkey);
                }

            }

        }

        RegCloseKey(partnerKey);

    } while (foundDevice);

    RegCloseKey(ceRootKey);

}

/* ----------------------------------------------------------------------------
*/
static void createDesktopList(char * malpath,
                              char * prefsname,
                              char * sprefsname)
{
    char deviceName[MAX_PATH];
    char path[MAX_PATH];
    char keyName[MAX_PATH];
    HKEY hkeyMAL;
    HKEY hkey;
    LONG result;
    DWORD disposition;
    BOOL bDesktop = FALSE, bPath = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        AG_MAL_ROOT_KEY,
        0,
        KEY_READ,
        &hkeyMAL)) {

        DWORD size;
        DWORD dwType;

        size = MAX_PATH;
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyMAL,
            AG_MAL_DESKTOP_NAME,
            0,
            &dwType,
            (LPBYTE)deviceName,
            &size))
            bDesktop = TRUE;

        size = MAX_PATH;
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyMAL,
            AG_MAL_DESKTOP_PATH,
            0,
            &dwType,
            (LPBYTE)path,
            &size))
            bPath = TRUE;

        RegCloseKey(hkeyMAL);

    }

    if (!bDesktop)
        strcpy(deviceName, agMALDesktopDefaultName);
    if (!bPath)
        strcpy(path, "C:\\");

    strcpy(keyName, AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY"\\d>");
    strcat(keyName, deviceName);

    result = RegCreateKeyEx(HKEY_CURRENT_USER,
        keyName,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hkey,
        &disposition);

    if (ERROR_SUCCESS == result) {

        TCHAR tempbuf[MAX_PATH];

        RegSetValueEx(hkey,
            AG_MAL_DEVICE_NAME,
            0,
            REG_SZ,
            deviceName,
            strlen(deviceName) + 1);

        RegSetValueEx(hkey,
            AG_MAL_DEVICE_PATH,
            0,
            REG_SZ,
            path,
            strlen(path) + 1);

        PathCombine(tempbuf, path, malpath);
        CreateDirectory(tempbuf, NULL);
        PathAppend(tempbuf, prefsname);

        RegSetValueEx(hkey,
            AG_MAL_DEVICE_PREFS,
            0,
            REG_SZ,
            tempbuf,
            strlen(tempbuf) + 1);

        PathCombine(tempbuf, path, malpath);
        PathAppend(tempbuf, sprefsname);

        RegSetValueEx(hkey,
            AG_MAL_DEVICE_PREFS_SYNCED,
            0,
            REG_SZ,
            tempbuf,
            strlen(tempbuf) + 1);

        RegCloseKey(hkey);
    }

}

/* ----------------------------------------------------------------------------
*/
static void refreshDeviceList(char * malpath,
                              char * prefsname,
                              char * sprefsname)
{
    HANDLE mutex = NULL;

    if (NULL != (mutex = enterBlockingSection(agrmDevice))) {
        
        resetDeviceList();

        if (isPlatformInstalled(palmPlatform))
            createPalmList(malpath, prefsname, sprefsname);

        if (isPlatformInstalled(cePlatform))
            createCEList(malpath, prefsname, sprefsname);

        if (isPlatformInstalled(desktopPlatform))
            createDesktopList(malpath, prefsname, sprefsname);

        leaveBlockingSection(agrmDevice, mutex);

    } else
        SetLastError(AG_ERROR_SYNCHRONIZATION_FAILED);
}

/* ----------------------------------------------------------------------------
*/
void AGRefreshDeviceList(void)
{
    HANDLE mutex = NULL;
    char * malpath = NULL;
    char * prefsname = NULL;
    char * sprefsname = NULL;

    malpath = AGSyncCommonGetStringConstant(agMALSubdirectoryName, FALSE);
    if (NULL == malpath)
        malpath = strdup(AG_MAL_PATH_DEFVAL);
    prefsname = AGSyncCommonGetStringConstant(agNewPrefFilename, FALSE);
    if (NULL == prefsname)
        prefsname = strdup(AG_MAL_PREFS_DEFVAL);
    sprefsname = AGSyncCommonGetStringConstant(agNewSyncPrefFilename, FALSE);
    if (NULL == sprefsname)
        sprefsname = strdup(AG_MAL_PREFS_SYNCED_DEFVAL);

    refreshDeviceList(malpath, prefsname, sprefsname);

    CHECKANDFREE(malpath);
    CHECKANDFREE(prefsname);
    CHECKANDFREE(sprefsname);
}

/* ----------------------------------------------------------------------------
*/
void AGRefreshDeviceListOld(void)
{
    HANDLE mutex = NULL;
    char * malpath = NULL;
    char * prefsname = NULL;
    char * sprefsname = NULL;

    malpath = AGSyncCommonGetStringConstant(agMALSubdirectoryName, FALSE);
    if (NULL == malpath)
        malpath = strdup(AG_MAL_PATH_DEFVAL);
    prefsname = AGSyncCommonGetStringConstant(agPreferencesFilename, FALSE);
    if (NULL == prefsname)
        prefsname = strdup(AG_MAL_PREFS_DEFVAL_OLD);
    sprefsname =
        AGSyncCommonGetStringConstant(agSynchronizedPreferencesFilename,
            FALSE);
    if (NULL == sprefsname)
        sprefsname = strdup(AG_MAL_PREFS_SYNCED_DEFVAL_OLD);

    refreshDeviceList(malpath, prefsname, sprefsname);

    CHECKANDFREE(malpath);
    CHECKANDFREE(prefsname);
    CHECKANDFREE(sprefsname);
}

/* ----------------------------------------------------------------------------
*/
AGDeviceEnumerator * AGDeviceEnumeratorNew(void)
{
    AGDeviceEnumerateState * state;
    HANDLE mutex = NULL;
    int32 i;

    /* First time called.  Set up. */
    state = (AGDeviceEnumerateState *)malloc(sizeof(AGDeviceEnumerateState));
    if (NULL == state)
        return NULL;

    state->count = 0;
    state->iteration = 0;
    state->devices = NULL;

    if (NULL != (mutex = enterBlockingSection(agrmDevice))) {

        HKEY hkey;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
            AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY,
            0,
            KEY_READ,
            &hkey)) {

            RegQueryInfoKey(hkey,
                NULL,
                NULL,
                0,
                &state->count, 
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL);

            if (state->count > 0) {

                state->devices = (char**)malloc(sizeof(char*) * state->count);

                if (NULL != state->devices) {

                    LONG result;

                    i = 0;
                    do {

                        TCHAR subKeyName[MAX_PATH];
                        DWORD size;
                        FILETIME filetime;

                        size = MAX_PATH;
                        result = RegEnumKeyEx(hkey,
                            i,
                            subKeyName,
                            &size,
                            NULL,
                            NULL,
                            NULL,
                            &filetime);

                        if (ERROR_SUCCESS == result)
                            state->devices[i] = strdup(subKeyName);

                        ++i;

                    } while (ERROR_SUCCESS == result);


                }

            }

            RegCloseKey(hkey);
        }

        leaveBlockingSection(agrmDevice, mutex);

    }

    return (AGDeviceEnumerator*)state;
}

/* ----------------------------------------------------------------------------
*/
char * AGGetNextDevice(AGDeviceEnumerator * enumerator)
{
    AGDeviceEnumerateState * state;

    if (NULL == enumerator)
        return NULL;

    state = (AGDeviceEnumerateState *)enumerator;

    /* First or later call.  Return one element. */
    if (state->iteration < state->count) {
        return strdup(state->devices[state->iteration++]);
    }

    /* Done.  Return NULL. */
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
void AGDeviceEnumeratorFree(AGDeviceEnumerator * enumerator)
{
    int32 i;
    AGDeviceEnumerateState * state;

    if (NULL == enumerator)
        return;

    state = (AGDeviceEnumerateState *)enumerator;

    _ASSERT(NULL != state);
    for (i = 0; i < state->count; ++i) {
        if (NULL != state->devices[i]) {
            free(state->devices[i]);
            state->devices[i] = NULL;
        }
    }
    free(state->devices);
    state->devices = NULL;
    free(state);
}

/* ----------------------------------------------------------------------------
*/
int32 AGGetDevice(char * key, AGRMDeviceInfo * info)
{
    HANDLE mutex = NULL;

    if (NULL == key || NULL == info)
        return AG_ERROR_BAD_ARGUMENT;

    switch (key[0]) {
        case 'c':
            info->type = AG_WINCE;
            break;
        case 'p':
            info->type = AG_PALM;
            break;
        case 'd':
            info->type = AG_DESKTOP;
            break;
        default:
            info->type = AG_UNKNOWN_DEVICE_TYPE;
    }

    if (NULL != (mutex = enterBlockingSection(agrmDevice))) {

        HKEY hkey;
        TCHAR keyName[MAX_PATH];

        strcpy(keyName, AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY"\\");
        strcat(keyName, key);

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
            keyName,
            0,
            KEY_READ,
            &hkey)) {

            DWORD type;

            if (NULL != info->szName) {

                type = REG_SZ;
                RegQueryValueEx(hkey,
                    AG_MAL_DEVICE_NAME,
                    0,
                    &type,
                    (LPBYTE)info->szName,
                    &info->cbName);

            }

            RegCloseKey(hkey);
        }

        leaveBlockingSection(agrmDevice, mutex);

    } else
        return AG_ERROR_SYNCHRONIZATION_FAILED;

    return AG_ERROR_NONE;

}

/* ----------------------------------------------------------------------------
*/
static int32 agGetDevicePriv(char * key, AGRMDeviceInfoPriv * info)
{
    int32 result = AG_ERROR_NONE;
    HANDLE mutex = NULL;
    
    result = AGGetDevice(key, &info->pub);
    if (AG_ERROR_NONE != result)
        return result;

    if (NULL != (mutex = enterBlockingSection(agrmDevice))) {

        HKEY hkey;
        TCHAR keyName[MAX_PATH];

        strcpy(keyName, AG_MAL_ROOT_KEY"\\"AG_MAL_DEVICE_KEY"\\");
        strcat(keyName, key);

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
            keyName,
            0,
            KEY_READ,
            &hkey)) {

            DWORD type;

            if (NULL != info->szPath) {

                type = REG_SZ;
                RegQueryValueEx(hkey,
                    AG_MAL_DEVICE_PATH,
                    0,
                    &type,
                    (LPBYTE)info->szPath,
                    &info->cbPath);

            }

            if (NULL != info->szPreferences) {

                type = REG_SZ;
                RegQueryValueEx(hkey,
                    AG_MAL_DEVICE_PREFS,
                    0,
                    &type,
                    (LPBYTE)info->szPreferences,
                    &info->cbPreferences);

            }

            if (NULL != info->szPreferencesSynchronized) {

                type = REG_SZ;
                RegQueryValueEx(hkey,
                    AG_MAL_DEVICE_PREFS_SYNCED,
                    0,
                    &type,
                    (LPBYTE)info->szPreferencesSynchronized,
                    &info->cbPreferencesSynchronized);

            }

            RegCloseKey(hkey);
        }

        leaveBlockingSection(agrmDevice, mutex);

    } else
        return AG_ERROR_SYNCHRONIZATION_FAILED;

    return AG_ERROR_NONE;
}

/* ----------------------------------------------------------------------------
*/
int32 AGGetCurrentDevice(char * key, int32 cbKey)
{
    int32 result = AG_ERROR_NOT_FOUND;
    HKEY hkey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        AG_MAL_ROOT_KEY,
        0,
        KEY_READ,
        &hkey)) {

        DWORD size = cbKey;
        DWORD dwType;

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
            AG_MAL_CURRENT_DEVICE,
            0,
            &dwType,
            (LPBYTE)key,
            &size)) {
            
            result = AG_ERROR_NONE;

        }

        RegCloseKey(hkey);

    }

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGSetCurrentDevice(char * key)
{
    int32 result = AG_ERROR_NOT_FOUND;
    HKEY hkey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        AG_MAL_ROOT_KEY,
        0,
        KEY_WRITE,
        &hkey)) {

        if (ERROR_SUCCESS == RegSetValueEx(hkey,
            AG_MAL_CURRENT_DEVICE,
            0,
            REG_SZ,
            key,
            strlen(key) + 1)) {
            
            result = AG_ERROR_NONE;

        }

        RegCloseKey(hkey);

    }

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGSetCurrentDeviceFromPlatform(int32 type, char * id)
{
    char key[128];

    /* pending (miket): HACK -- $1.00 in hack bucket */
    if (AG_DESKTOP == type) {
        CHECKANDFREE(id);
        id = strdup("Desktop Profile");
    }

    if (NULL == id)
        return AG_ERROR_BAD_ARGUMENT;

    switch (type) {
        case AG_WINCE:
            strcpy(key, "c>");
            break;
        case AG_PALM:
            strcpy(key, "p>");
            break;
        case AG_DESKTOP:
            strcpy(key, "d>");
            break;
        default:
            return AG_ERROR_UNKNOWN_DEVICE_TYPE;
    }

    strcat(key, id);

    return AGSetCurrentDevice(key);
}

/* ----------------------------------------------------------------------------
*/
static char * getPrefsFilenameForDevice(char * key)
{
    AGRMDeviceInfoPriv info;
    int32 deviceresult = AG_ERROR_OUT_OF_MEMORY;

    ZeroMemory(&info, sizeof(AGRMDeviceInfoPriv));
    info.cbPreferences = MAX_PATH;
    info.szPreferences = (char*)malloc(info.cbPreferences);
    if (NULL != info.szPreferences)
        deviceresult = agGetDevicePriv(key, &info);
    if (AG_ERROR_NONE != deviceresult) {
        SetLastError(deviceresult);
        CHECKANDFREE(info.szPreferences);
        return NULL;
    }
    return info.szPreferences;
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
    AGUserConfig * AGReadUserConfigFromDiskAtomically(TCHAR * filename,
                                                      HANDLE * fileHandle)

    Reads userConfig from disk and returns a handle to a still-open,
    exclusive-access file.
*/
static AGUserConfig * AGReadUserConfigFromDiskAtomically(TCHAR * filename,
                                                  HANDLE * fileHandle)
{
    return createAndRead(filename, fileHandle, TRUE);
}

/* ----------------------------------------------------------------------------
    AGUserConfig * AGReadUserConfigFromDisk(TCHAR * filename)

    Reads userConfig from disk.
*/
static AGUserConfig * AGReadUserConfigFromDisk(TCHAR * filename)
{
    HANDLE f;
    AGUserConfig * userConfig;

    userConfig = createAndRead(filename, &f, FALSE);

    if (INVALID_HANDLE_VALUE != f)
        CloseHandle(f);

    return userConfig;
}

/* ----------------------------------------------------------------------------
    void AGWriteUserConfigToDiskAtomically(AGUserConfig * userConfig,
                                           HANDLE fileHandle)

    Writes userConfig to the supplied open file, and then closes the file.

*/
static void AGWriteUserConfigToDiskAtomically(AGUserConfig * userConfig,
                                       HANDLE fileHandle)

{
    if (INVALID_HANDLE_VALUE != fileHandle) {
        AGWriter * writer = AGWriterNew(fileHandle, writeDiskFunc);
        if (NULL != userConfig) {
            SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN);
            AGUserConfigWriteData(userConfig, writer);
        }
        AGWriterFree(writer);
        SetEndOfFile(fileHandle);
        CloseHandle(fileHandle);
    }
}

/* ----------------------------------------------------------------------------
    void AGWriteUserConfigToDisk(TCHAR * filename, AGUserConfig * userConfig)

    Writes userConfig to disk.

*/
static void AGWriteUserConfigToDisk(TCHAR * filename, AGUserConfig * userConfig)
{
    HANDLE f;

    f = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 0);
    AGWriteUserConfigToDiskAtomically(userConfig, f);

}

/* ----------------------------------------------------------------------------
*/
static AGLocationConfig * AGReadLocationConfigFromDisk(TCHAR * filename)
{
    HANDLE f;
    AGLocationConfig * locationConfig = NULL;

    f = CreateFile(filename,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);
    if (INVALID_HANDLE_VALUE != f) {
        AGReader * reader = AGReaderNew(f, readDiskFunc);
        locationConfig = AGLocationConfigNew();
        AGLocationConfigReadData(locationConfig, reader);
        AGReaderFree(reader);
    }

    if (INVALID_HANDLE_VALUE != f)
        CloseHandle(f);

    return locationConfig;
}

/* ----------------------------------------------------------------------------
*/
static void AGWriteLocationConfigToDisk(TCHAR * filename,
                                 AGLocationConfig * locationConfig)
{
    HANDLE f = CreateFile(filename,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0);
    if (INVALID_HANDLE_VALUE != f) {
        AGWriter * writer = AGWriterNew(f, writeDiskFunc);
        if (NULL != locationConfig) {
            SetFilePointer(f, 0, NULL, FILE_BEGIN);
            AGLocationConfigWriteData(locationConfig, writer);
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
    char * prefsname = NULL;

    if (NULL == key || NULL == server)
        return AG_ERROR_BAD_ARGUMENT;

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname)
        return AG_ERROR_NOT_FOUND;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        LPAGUC user = NULL;    

        user = AGReadUserConfigFromDisk(prefsname);
        if (NULL == user)
            user = AGUserConfigNew();
        if (NULL != user) {
            AGUserConfigAddServer(user, AGServerConfigDup(server), fromDevice);
            AGWriteUserConfigToDisk(prefsname, user);
            AGUserConfigFree(user);
        } else
            result = AG_ERROR_OUT_OF_MEMORY;

        leaveBlockingSection(agrmServer, mutex);

    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

    CHECKANDFREE(prefsname);

    return result;
}

/* ----------------------------------------------------------------------------
*/
int32 AGRemoveServer(char * key, int32 uid)
{
    HANDLE mutex = NULL;
    int32 result = AG_ERROR_NONE;
    char * prefsname = NULL;

    if (NULL == key || 0 == uid)
        return AG_ERROR_BAD_ARGUMENT;

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname)
        return AG_ERROR_NOT_FOUND;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        LPAGUC user = NULL;    

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

    CHECKANDFREE(prefsname);

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
    if (modFlags[AGRM_SMOD_NOTREMOVABLE])
        destsc->notRemovable = srcsc->notRemovable;
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
    char * prefsname = NULL;

    if (NULL == key
        || 0 == uid
        || NULL == server
        || flagStructSize != AGRM_SMOD_NUMFLAGS
        || NULL == modFlags)
        return AG_ERROR_BAD_ARGUMENT;

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname)
        return AG_ERROR_NOT_FOUND;

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {
        LPAGUC user = NULL;
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

    CHECKANDFREE(prefsname);

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
    char * prefsname = NULL;

    if (NULL == key || 0 == uid) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    prefsname = getPrefsFilenameForDevice(key);
    if (NULL == prefsname) {
        SetLastError(AG_ERROR_NOT_FOUND);
        return NULL;
    }

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {
        LPAGUC user = NULL;    
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
    AGRMDeviceInfoPriv info;
    int32 deviceresult = AG_ERROR_OUT_OF_MEMORY;
    HANDLE mutex = NULL;

    if (NULL == key) {
        SetLastError(AG_ERROR_BAD_ARGUMENT);
        return NULL;
    }

    state = (AGServerEnumerateState *)malloc(sizeof(AGServerEnumerateState));
    if (NULL == state)
        return NULL;

    ZeroMemory(&info, sizeof(AGRMDeviceInfoPriv));
    info.cbPreferences = MAX_PATH;
    info.szPreferences = (char*)malloc(info.cbPreferences);
    if (NULL != info.szPreferences)
        deviceresult = agGetDevicePriv(key, &info);
    if (AG_ERROR_NONE != deviceresult) {
        SetLastError(deviceresult);
        CHECKANDFREE(info.szPreferences);
        return NULL;
    }

    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        state->user = AGReadUserConfigFromDisk(info.szPreferences);
        CHECKANDFREE(info.szPreferences);
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

/* ----------------------------------------------------------------------------
*/
AGLocationConfig * AGGetLocationConfig(void)
{
    HANDLE mutex = NULL;
    char * filename = NULL;
    LPAGLC loc = NULL;

    filename = AGSyncCommonGetStringConstant(agNewConnectionFilename, FALSE);
    if (NULL == filename) {
        SetLastError(AG_ERROR_PATH_UNKNOWN);
        return NULL;
    }

    if (NULL != (mutex = enterBlockingSection(agrmLocation))) {
        loc = AGReadLocationConfigFromDisk(filename);
        leaveBlockingSection(agrmLocation, mutex);
    } else
        SetLastError(AG_ERROR_SYNCHRONIZATION_FAILED);

    if (NULL != loc)
        SetLastError(AG_ERROR_NONE);
    else
        SetLastError(AG_ERROR_NOT_FOUND);

    CHECKANDFREE(filename);

    return loc;
}

/* ----------------------------------------------------------------------------
*/
static void modifyLocationConfig(AGLocationConfig * d,
                                 AGLocationConfig * s,
                                 AGBool * modFlags)
{
     
    if (modFlags[AGRM_LMOD_ALL]) {
        AGLocationConfigCopy(d, s);
        return;
    }

    /* pending(miket) the rest aren't implemented! */
}

/* ----------------------------------------------------------------------------
*/
int32 AGModifyLocationConfig(AGLocationConfig * loc,
                             int32 flagStructSize,
                             AGBool * modFlags)
{
    HANDLE mutex = NULL;
    char * filename = NULL;

    filename = AGSyncCommonGetStringConstant(agNewConnectionFilename, FALSE);
    if (NULL == filename)
        return AG_ERROR_PATH_UNKNOWN;

    if (NULL == loc
        || flagStructSize != AGRM_LMOD_NUMFLAGS
        || NULL == modFlags)
        return AG_ERROR_BAD_ARGUMENT;

    if (NULL != (mutex = enterBlockingSection(agrmLocation))) {
        
        AGLocationConfig * dst = NULL;
        dst = AGReadLocationConfigFromDisk(filename);
        if (NULL != dst) 
            modifyLocationConfig(dst, loc, modFlags);
        else
            dst = AGLocationConfigDup(loc);
        AGWriteLocationConfigToDisk(filename, dst);
        if (NULL != dst)
            AGLocationConfigFree(dst);

        leaveBlockingSection(agrmLocation, mutex);

    } else
        return AG_ERROR_SYNCHRONIZATION_FAILED;

    CHECKANDFREE(filename);

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

/* ----------------------------------------------------------------------------
*/
int32 AGSyncProfiles(uint8 * out, uint8 ** in, uint32 * inSize, AGBool pd)
{
    HANDLE mutex = NULL;
    int32 result = AG_ERROR_NONE;
    AGRMDeviceInfoPriv info;
    int32 deviceresult = AG_ERROR_OUT_OF_MEMORY;

    ZeroMemory(&info, sizeof(AGRMDeviceInfoPriv));
    info.cbPreferences = info.cbPreferencesSynchronized = MAX_PATH;
    info.szPreferences = (char*)malloc(info.cbPreferences);
    info.szPreferencesSynchronized = 
        (char*)malloc(info.cbPreferencesSynchronized);
    if (NULL != info.szPreferences && NULL != info.szPreferencesSynchronized) {
        char curdev[MAX_PATH];
        AGGetCurrentDevice(curdev, MAX_PATH);
        deviceresult = agGetDevicePriv(curdev, &info);
    }
    if (AG_ERROR_NONE != deviceresult) {
        CHECKANDFREE(info.szPreferences);
        CHECKANDFREE(info.szPreferencesSynchronized);
        return deviceresult;
    }
    
    if (NULL != (mutex = enterBlockingSection(agrmServer))) {

        AGUserConfig * device = NULL,
            * desktop = NULL,
            * agreed = NULL,
            * newUserConfig = NULL;
        AGBufferReader * r = NULL;
        AGBufferWriter * w = NULL;

        desktop = AGReadUserConfigFromDisk(info.szPreferences);
        agreed = AGReadUserConfigFromDisk(info.szPreferencesSynchronized);
        if (NULL == agreed)
            agreed = AGUserConfigNew();
        if (NULL != out) {
            r = AGBufferReaderNew((uint8*)out);
            if (NULL != r) {
                device = AGUserConfigNew();
                if (NULL != device) {
                    result = AGUserConfigReadData(device, (AGReader*)r);
                    /* pending : report error at this point... */
                    result = AG_ERROR_NONE;
                } else
                    result = AG_ERROR_OUT_OF_MEMORY;
                AGBufferReaderFree(r);
            }
        }
        newUserConfig = AGUserConfigSynchronize(agreed, device, desktop, pd);
        
        if (NULL != device)
            AGUserConfigFree(device);
        if (NULL != desktop)
            AGUserConfigFree(desktop);
        if (NULL != agreed)
            AGUserConfigFree(agreed);
        
        AGWriteUserConfigToDisk(info.szPreferences,
            newUserConfig);
        AGWriteUserConfigToDisk(info.szPreferencesSynchronized,
            newUserConfig);
        w = AGBufferWriterNew(0);
        if (NULL != w) {
            AGUserConfigWriteData(newUserConfig, (AGWriter*)w);
            *inSize = AGBufferWriterGetBufferSize(w);
            *in = AGBufferWriterRemoveBuffer(w);
            AGBufferWriterFree(w);
        }
        
        if (NULL != newUserConfig)
            AGUserConfigFree(newUserConfig);

        leaveBlockingSection(agrmServer, mutex);

    } else
        result = AG_ERROR_SYNCHRONIZATION_FAILED;

    CHECKANDFREE(info.szPreferences);
    CHECKANDFREE(info.szPreferencesSynchronized);

    return result;
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
