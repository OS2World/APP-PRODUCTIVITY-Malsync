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

#include <AGDesktopInfoWinCE.h>

AGDesktopInfoWinCE * AGDesktopInfoWinCENew(void)
{
    AGDesktopInfoWinCE * desktopInfo;

    desktopInfo = (AGDesktopInfoWinCE *)malloc(sizeof(AGDesktopInfoWinCE));
    if (NULL != desktopInfo) {

        desktopInfo->hModule = LoadLibrary("ceutil.dll");

        if (NULL != desktopInfo->hModule) {

            desktopInfo->pCeGetDeviceID =
                (CEGETDEVICEID)GetProcAddress(
                desktopInfo->hModule, "CeGetDeviceId");

        }

    }

    return desktopInfo;
}

void AGDesktopInfoWinCEFree(AGDesktopInfoWinCE * desktopInfo)
{
    if (NULL != desktopInfo) {

        if (NULL != desktopInfo->hModule)
            FreeLibrary(desktopInfo->hModule);

        free(desktopInfo);

    }
}

uint32 AGDesktopInfoWinCEGetDeviceID(AGDesktopInfoWinCE * desktopInfo)
{
    if (NULL != desktopInfo->pCeGetDeviceID)
        return desktopInfo->pCeGetDeviceID();
    else
        return 0;
}
