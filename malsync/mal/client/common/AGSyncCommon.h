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


#ifndef __AGSYNCCOMMON_H__
#define __AGSYNCCOMMON_H__

#define DEVICE_USERCONFIG_DB_NAME       "MBlnUserConfig"
#define DEVICE_USERCONFIG_DB_CREATOR    0x4D426C6e /* 'MBln' */
#define DEVICE_USERCONFIG_DB_TYPE       0x75736572 /* 'user' */

#define DEVICE_PROFILE_DB_NAME       "MBlnProfile"
#define DEVICE_PROFILE_DB_CREATOR    0x4D426C6e /* 'MBln' */
#define DEVICE_PROFILE_DB_TYPE       0x75736572 /* 'user' */

#ifdef _WIN32 /**** Entire file is NOOP if not on Windows. *****/
#include <windows.h>
#include <tchar.h>
#include <AGTypes.h>
#include <AGArray.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    agRAPIDLLName = 0,
    agRAPIFunctionName,
    agLocationConfigPath,
    agMobileLinkPath,
    agPreferencesFilename,
    agSynchronizedPreferencesFilename,
    agMALSubdirectoryName,
    agExtensionDLLPath,
    agExtensionDLLName,
    agHelpString,
    agNewPrefFilename,
    agNewSyncPrefFilename,
    agNewConnectionFilename,
} agStringConstants;

typedef enum {
    agstProgressBitmapFilename = 0,
    agstDisplayIconFilename,
} agstStringConstants;

typedef enum {
    agNone = 0, /* pending(miket) this isn't currently used */
} agIntegerConstants;

typedef enum {
    agstProgressBitmapFrameCount = 0
} agstIntegerConstants;

ExportFunc void * AGSyncCommonGetStringConstant(agStringConstants strNum,
                                                AGBool forceWide);
ExportFunc DWORD AGSyncCommonGetIntegerConstant(agIntegerConstants intNum);

#if !defined(_WIN32_WCE)
ExportFunc char * AGSyncCommonGetStringForServerType(char * serverType,
    agstStringConstants strNum);
ExportFunc DWORD AGSyncCommonGetIntegerForServerType(char * serverType,
    agIntegerConstants intNum);
ExportFunc AGArray * AGSyncCommonLoadGraphics(HINSTANCE h, char * serverType);
ExportFunc void AGSyncCommonFreeGraphics(AGArray * array);
#endif /* #if !defined(_WIN32_WCE) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */

#endif /* __AGSYNCCOMMON_H__ */

