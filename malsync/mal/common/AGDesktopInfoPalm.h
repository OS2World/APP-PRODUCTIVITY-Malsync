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

#ifndef __AGDESKTOPINFOPALM_H__
#define __AGDESKTOPINFOPALM_H__

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>
#include <AGArray.h>

typedef int (WINAPI * PLTGETUSERCOUNT)(void);

typedef int (WINAPI * PLTGETUSER)(unsigned int iIndex,
    TCHAR *pUserBuffer,
    short *psUserBufSize);
typedef int (WINAPI * PLTGETUSERDIRECTORY)(TCHAR *pUser,
    TCHAR *pBuffer,
    int *piBufferSize);

typedef struct {
    HMODULE hModule;
    PLTGETUSERCOUNT pGetUserCount;
    PLTGETUSER pGetUser;
    PLTGETUSERDIRECTORY pGetUserDirectory;
    AGArray * userArray;
} AGDesktopInfoPalm;

ExportFunc AGDesktopInfoPalm * AGDesktopInfoPalmNew(void);
ExportFunc void AGDesktopInfoPalmFree(AGDesktopInfoPalm * DesktopInfoPalm);
ExportFunc
uint32 AGDesktopInfoPalmGetUserCount(AGDesktopInfoPalm * DesktopInfoPalm);
ExportFunc
void AGDesktopInfoPalmGetUsername(AGDesktopInfoPalm * DesktopInfoPalm,
                                  uint32 n,
                                  char * buffer,
                                  short * bufsize);
ExportFunc
void AGDesktopInfoPalmGetUserDirectory(AGDesktopInfoPalm * DesktopInfoPalm,
                                       char * username,
                                       char * buffer,
                                       int * bufsize);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifdef _WIN32 */ 
#endif /* #ifndef __AGDESKTOPINFOPALM_H__ */