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

#include <AGSyncCommon.h>
#if defined(_WIN32) /**** Entire file is NOOP if not on Windows. *****/

#include <stdlib.h>
#include <AGUtil.h>

TCHAR * HKEY_MAL_CURRENT_USER_ROOT = TEXT("Software\\Mobile Application Link");

TCHAR * HKEY_MAL_SERVERS_ROOT = TEXT("\\Servers");

TCHAR * HKEY_MAL_SERVERS_DEFAULT = TEXT("Default");

TCHAR * stringConstants[13] = {
    TEXT("RAPI Server DLL Name"),
    TEXT("RAPI Server Name"),
    TEXT("Location Configuration"),
    TEXT("Mobile Link Location"),
    TEXT("Preferences Filename"),
    TEXT("Synchronized Preferences Filename"),
    TEXT("MAL Subdirectory Name"),
    TEXT("Extension DLL Path"),
    TEXT("Extension DLL Name"),
    TEXT("Help URL"),
    TEXT("New Pref Filename"),
    TEXT("New Sync Pref Filename"),
    TEXT("New Connection Settings Filename"),
};

TCHAR * integerConstants[1] = {
    TEXT("") // pending(miket): not currently used
};

TCHAR * stStringConstants[2] = {
    TEXT("Progress Bitmap Filename"),
    TEXT("Display Icon"),
};

TCHAR * stIntegerConstants[1] = {
    TEXT("Number of Progress Bitmap Frames")
};

/* ----------------------------------------------------------------------------
*/
static void * getStringConstant(HKEY key, TCHAR * value, AGBool unicode)
{
    void * result = NULL;
    DWORD bufsize = 0;
    HRESULT hr = RegQueryValueEx(key,
        value,
        NULL,
        NULL,
        NULL,
        &bufsize);

    if (ERROR_SUCCESS == hr) {

        if (bufsize > 0) {
            result = malloc(bufsize);

            if (NULL != result)
                RegQueryValueEx(key,
                    value,
                    NULL,
                    NULL,
                    (BYTE*)result,
                    &bufsize);
            if (unicode) {
                int cbWChars =
                    MultiByteToWideChar(CP_ACP, 0, result, -1, NULL, 0);
                int cbWStr = sizeof(WCHAR) * cbWChars;
                void * wresult = malloc(cbWStr);
                MultiByteToWideChar(CP_ACP, 0, result, -1, wresult, cbWChars);
                free(result);
                result = wresult;
            }
        }
    }
    return result;
}

/* ----------------------------------------------------------------------------
*/
static DWORD getIntegerConstant(HKEY key,
                                TCHAR * value)
{
    DWORD result = 0;
    DWORD bufsize = sizeof(DWORD);
    DWORD valueType = 0;

    RegQueryValueEx(key,
        value,
        0,
        &valueType,
        (uint8*)&result,
        &bufsize);
        
    return result;
}

/* ----------------------------------------------------------------------------
*/
void * AGSyncCommonGetStringConstant(agStringConstants strNum,
                                     AGBool forceWide)
{
    HKEY rootKey = NULL;
    HRESULT hr;
    void * result = NULL;

    hr = RegOpenKeyEx(HKEY_CURRENT_USER,
        HKEY_MAL_CURRENT_USER_ROOT,
        0,
        KEY_READ,
        &rootKey);

    if (ERROR_SUCCESS != hr)
        return NULL;
    
    result = getStringConstant(rootKey, stringConstants[strNum], forceWide);

    RegCloseKey(rootKey);

    return result;
}

/* ----------------------------------------------------------------------------
*/
static DWORD AGSyncCommonGetIntegerConstant(agIntegerConstants intNum)
{
    HKEY rootKey = NULL;
    HRESULT hr;
    DWORD result = 0;

    hr = RegOpenKeyEx(HKEY_CURRENT_USER,
        HKEY_MAL_CURRENT_USER_ROOT,
        0,
        KEY_READ,
        &rootKey);

    if (ERROR_SUCCESS != hr)
        return 0;
    
    result = getIntegerConstant(rootKey, integerConstants[intNum]);

    RegCloseKey(rootKey);

    return result;

}

static void setupbuf(TCHAR * buf, const TCHAR * serverType)
{
    strcpy(buf, HKEY_MAL_CURRENT_USER_ROOT);
    strcat(buf, HKEY_MAL_SERVERS_ROOT);
    strcat(buf, TEXT("\\"));
    strcat(buf, serverType);
}

/* ----------------------------------------------------------------------------
    ExportFunc char *
    AGSyncCommonGetStringForServerType(char * serverType,
                                       agstStringConstants strNum)

*/
ExportFunc char *
AGSyncCommonGetStringForServerType(char * serverType,
                                   agstStringConstants strNum)
{
    HKEY rootKey = NULL;
    HRESULT hr = 0;
    char * result = NULL;
    char buf[MAX_PATH];

    setupbuf(buf,
        (NULL != serverType) ? serverType : HKEY_MAL_SERVERS_DEFAULT);

    hr = RegOpenKeyEx(HKEY_CURRENT_USER, buf, 0, KEY_READ, &rootKey);

    if (ERROR_SUCCESS != hr) {
        setupbuf(buf, HKEY_MAL_SERVERS_DEFAULT);
        hr = RegOpenKeyEx(HKEY_CURRENT_USER, buf, 0, KEY_READ, &rootKey);
        if (ERROR_SUCCESS != hr)
            return NULL;
    }
    
    result = (char*)getStringConstant(rootKey,
        stStringConstants[strNum],
        FALSE);

    RegCloseKey(rootKey);

    return result;

}

/* ----------------------------------------------------------------------------
    ExportFunc DWORD
    AGSyncCommonGetIntegerForServerType(char * serverType,
                                        agstIntegerConstants intNum)
*/
ExportFunc DWORD
AGSyncCommonGetIntegerForServerType(char * serverType,
                                    agstIntegerConstants intNum)
{
    HKEY rootKey = NULL;
    DWORD result = 0;
    HRESULT hr = 0;
    char buf[MAX_PATH];

    setupbuf(buf,
        (NULL != serverType) ? serverType : HKEY_MAL_SERVERS_DEFAULT);

    hr = RegOpenKeyEx(HKEY_CURRENT_USER, buf, 0, KEY_READ, &rootKey);

    if (ERROR_SUCCESS != hr) {

        setupbuf(buf, HKEY_MAL_SERVERS_DEFAULT);
        hr = RegOpenKeyEx(HKEY_CURRENT_USER, buf, 0, KEY_READ, &rootKey);
        if (ERROR_SUCCESS != hr)
            return 0;

    }
    
    result = getIntegerConstant(rootKey, stIntegerConstants[intNum]);

    RegCloseKey(rootKey);

    return result;

}

/* ----------------------------------------------------------------------------
*/
ExportFunc AGArray * AGSyncCommonLoadGraphics(HINSTANCE h,
                                              char * serverType)
{
    AGArray * result;

    result = AGArrayNew(AGUnownedPointerElements, 0);

    if (NULL != result) {

        LPTSTR prototype = NULL;
        int n = AGSyncCommonGetIntegerForServerType(serverType,
            agstProgressBitmapFrameCount);

        if (n > 0) {

            prototype = (LPTSTR)AGSyncCommonGetStringForServerType(serverType,
                agstProgressBitmapFilename);

        }

        if (NULL != prototype) {

            int i;

            for (i = 0; i < n; i++) {

                TCHAR filename[MAX_PATH];

                sprintf(filename, prototype, i);

                AGArrayAppend(result,
                    LoadImage(NULL,
                        filename,
                        IMAGE_BITMAP,
                        0,
                        0,
                        LR_LOADFROMFILE |
                        LR_LOADMAP3DCOLORS |
                        LR_LOADTRANSPARENT));
            }

            free(prototype);

        }

    }

    return result;
}

/* ----------------------------------------------------------------------------
*/
ExportFunc void AGSyncCommonFreeGraphics(AGArray * array)
{
    while (AGArrayCount(array)) {
        DeleteObject((HBITMAP)AGArrayElementAt(array, 0));
        AGArrayRemoveAt(array, 0);
    }

    if (NULL != array)
        AGArrayFree(array);

}

#endif
