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

#ifndef __AGDESKTOPINFOWINCE_H__
#define __AGDESKTOPINFOWINCE_H__

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

typedef DWORD (__stdcall * CEGETDEVICEID)( void );

typedef struct {
    HMODULE hModule;
    CEGETDEVICEID pCeGetDeviceID;
} AGDesktopInfoWinCE;

ExportFunc AGDesktopInfoWinCE * AGDesktopInfoWinCENew(void);
ExportFunc
void AGDesktopInfoWinCEFree(AGDesktopInfoWinCE * desktopInfo);
ExportFunc
uint32 AGDesktopInfoWinCEGetDeviceID(AGDesktopInfoWinCE * desktopInfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifdef _WIN32 */ 
#endif /* #ifndef __AGDESKTOPINFOWINCE_H__ */