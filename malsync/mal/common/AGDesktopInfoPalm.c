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

/* Owner:  miket */

#ifdef _WIN32

#include <AGDesktopInfoPalm.h>
#include <AGUtil.h>
#include <AGShlapi.h>

typedef struct {
    char *name;
    char *dir;
    int id;
} AGUserInfo;

static void getUsersFile(char * buffer, int bufSize)
{
    HKEY key = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\U.S. Robotics\\Pilot Desktop\\Core",
        0,
        KEY_READ,
        &key)) {

        DWORD dwSize = bufSize;
        DWORD type;

        RegQueryValueEx(key,
            "Path",
            0,
            &type,
            buffer,
            &dwSize);

        PathAppend(buffer, "users.dat");

        RegCloseKey(key);

    }

}

static BOOL isASCII(char b) {
	int ib = (int)b & 0xff;
    return ib >= 32;
}

static char *findString(char *data, char *maxData)
{
    int length = *(data++);
    int i;
    char *name;

    if (length <= 0)
        return NULL;

    for (i = 0; i < length; i++) {
        if (!isASCII(data[i]))
            return NULL;
    }

    name = (char *)malloc(length + 1);
    memcpy(name, data, length);
    name[length] = '\0';
    return name;
}

static int findID(char *data)
{
    unsigned char *udata = (unsigned char *)data;

    return ((udata[3] << 24) |
            (udata[2] << 16) |
            (udata[1] << 8) |
            udata[0]);
}

static AGArray * getUserInfo()
{
    AGArray * result = NULL;
    HANDLE hFile;
    HANDLE hMapping;
    char usersFile[MAX_PATH];
    char *userData;
    char *current;
    char *maxData;
    int fileSize;
    char *name, *dir;

    usersFile[0] = '\0';

    getUsersFile(usersFile, MAX_PATH);
    hFile = CreateFile(usersFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;

    fileSize = GetFileSize(hFile, NULL);
    hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (NULL != hMapping) {

        userData = (char *)MapViewOfFile(hMapping,
            FILE_MAP_READ,
            0,
            0,
            0);

        maxData = userData + fileSize;

        result = AGArrayNew(AGUnownedPointerElements, 0);
        if (NULL != result) {

            for (current = userData; current < maxData; current++) {

                if ((name = findString(current, maxData)) &&
                    (dir = findString(current + strlen(name) + 1, maxData))) {

                    AGUserInfo * user;
                    int id;
                
                    id = findID(current - 4);
                    user = (AGUserInfo *)malloc(sizeof(AGUserInfo));

                    if (NULL != user) {

                        user->name = name;
                        user->dir = dir;
                        user->id = id;
                        current += strlen(name) + strlen(dir) + 1;

                        AGArrayAppend(result, user);

                    }

                }

            }

        }

        UnmapViewOfFile(userData);

        CloseHandle(hMapping);

    }

    CloseHandle(hFile);

    return result;
}

AGDesktopInfoPalm * AGDesktopInfoPalmNew(void)
{
    AGDesktopInfoPalm * desktopInfo;

    desktopInfo = (AGDesktopInfoPalm *)malloc(sizeof(AGDesktopInfoPalm));
    if (NULL != desktopInfo) {

        char pathName[MAX_PATH];
        HKEY key = NULL;

        pathName[0] = '\0';

        bzero(desktopInfo, sizeof(AGDesktopInfoPalm));

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
            "Software\\U.S. Robotics\\Pilot Desktop\\Core",
            0,
            KEY_READ,
            &key)) {

            DWORD dwSize = MAX_PATH;
            DWORD type;

            RegQueryValueEx(key,
                "Path",
                0,
                &type,
                pathName,
                &dwSize);

            PathAppend(pathName, "instaide.dll");

            desktopInfo->hModule = LoadLibrary(pathName);

            /* It's possible for people to move around their directories
            legitimately, so we have to look in two places for the instaide
            DLL. */
            if (NULL == desktopInfo->hModule) {

                dwSize = MAX_PATH;

                RegQueryValueEx(key,
                    "HotSyncPath",
                    0,
                    &type,
                    pathName,
                    &dwSize);

                PathRemoveFileSpec(pathName);

                PathAppend(pathName, "instaide.dll");

                desktopInfo->hModule = LoadLibrary(pathName);

            }

            RegCloseKey(key);

        }

        if (NULL != desktopInfo->hModule) {

            desktopInfo->pGetUser = (PLTGETUSER)GetProcAddress(
                desktopInfo->hModule, "PltGetUser");

            desktopInfo->pGetUserCount = (PLTGETUSERCOUNT)GetProcAddress(
                desktopInfo->hModule, "PltGetUserCount");

            desktopInfo->pGetUserDirectory = (PLTGETUSERDIRECTORY)
                GetProcAddress(desktopInfo->hModule, "PltGetUserDirectory");

        }
        else {

            desktopInfo->userArray = getUserInfo();

        }

    }

    return desktopInfo;
}

void AGDesktopInfoPalmFree(AGDesktopInfoPalm * desktopInfo)
{
    if (NULL != desktopInfo) {

        if (NULL != desktopInfo->hModule)
            FreeLibrary(desktopInfo->hModule);

        if (NULL != desktopInfo->userArray) {

            int i, n;

            n = AGArrayCount(desktopInfo->userArray);
            for (i = 0; i < n; ++i) {

                AGUserInfo * userInfo;

                userInfo =
                    (AGUserInfo *)AGArrayElementAt(desktopInfo->userArray, i);

                if (userInfo->dir)
                    free(userInfo->dir);

                if (userInfo->name)
                    free(userInfo->name);

                free(userInfo);

            }

            AGArrayRemoveAll(desktopInfo->userArray);

            AGArrayFree(desktopInfo->userArray);
        }

        free(desktopInfo);

    }
}

uint32 AGDesktopInfoPalmGetUserCount(AGDesktopInfoPalm * desktopInfo)
{
    if (NULL != desktopInfo->pGetUserCount)
        return desktopInfo->pGetUserCount();
    else
        return AGArrayCount(desktopInfo->userArray);
}

void AGDesktopInfoPalmGetUsername(AGDesktopInfoPalm * desktopInfo,
                             uint32 n,
                             char * buffer,
                             short * bufsize)
{
    if (NULL != desktopInfo->pGetUser) {

        desktopInfo->pGetUser(n, buffer, bufsize);

    }
    else {

        if (*bufsize < 1)
            return;

        if ((NULL != desktopInfo->userArray)
            && (n < (uint32)AGArrayCount(desktopInfo->userArray))) {

                AGUserInfo * userInfo;

                userInfo =
                    (AGUserInfo *)AGArrayElementAt(desktopInfo->userArray, n);

                if (NULL != userInfo->name)
                    strncpy(buffer, userInfo->name, *bufsize);

                buffer[*bufsize-1] = '\0';

                *bufsize = strlen(buffer);

        }
        else {

            buffer[0] = '\0';
            *bufsize = 0;

        }

    }

}

void AGDesktopInfoPalmGetUserDirectory(AGDesktopInfoPalm * desktopInfo,
                                       char * username,
                                       char * buffer,
                                       int * bufsize)
{
    HKEY key = NULL;
    char buf2[MAX_PATH];

    if (*bufsize < 1)
        return;

    buffer[0] = '\0';
    buf2[0] = '\0';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\U.S. Robotics\\Pilot Desktop\\Core",
        0,
        KEY_READ,
        &key)) {

        DWORD dwSize = MAX_PATH;
        DWORD type;

        RegQueryValueEx(key,
            "Path",
            0,
            &type,
            buffer,
            &dwSize);

        RegCloseKey(key);
    }

    if (NULL != desktopInfo->pGetUserDirectory) {

        desktopInfo->pGetUserDirectory(username, buf2, bufsize);

    }
    else {

        int n;

        if (NULL == desktopInfo->userArray) {
            *bufsize = 0;
            buffer[0] = '\0';
            return;
        }

        n = AGArrayCount(desktopInfo->userArray);

        while (n--) {

            AGUserInfo * userInfo;

            userInfo =
                (AGUserInfo *)AGArrayElementAt(desktopInfo->userArray, n);

            if (NULL == userInfo->name)
                continue;

            if (strcmp(username, userInfo->name))
                continue;

            if (NULL == userInfo->dir)
                continue;

            strncpy(buf2, userInfo->dir, *bufsize);

            buf2[*bufsize-1] = '\0';

            n = 0; // signal end of loop

        }

    }

    PathAppend(buffer, buf2);

    *bufsize = strlen(buffer);

}

#endif /* #ifdef _WIN32 */