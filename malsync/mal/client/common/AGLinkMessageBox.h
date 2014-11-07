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

#ifndef __AGLINKMESSAGEBOX_H__
#define __AGLINKMESSAGEBOX_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <windows.h>

/* WARNING (miket):  The resources for this message box are all screwed up.
    I couldn't get multiple scripts to be included in a single project, so
    the resource.h defines are duplicated in several places, and the
    resources themselves are duplicated.  I recommend grepping for things
    like IDC_LINK to find everything. */
#include <AGLinkMBResource.h>

#define kTimeoutValue 15000

int MessageBoxWithLink(HINSTANCE h,
                       HWND hWnd,
                       LPSTR lpText,
                       LPSTR lpCaption,
                       BOOL bLookForURL,
                       char * url,
                       UINT uMillisecondsBeforeTimeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __AGLINKMESSAGEBOX_H__ */

#endif /* #ifdef _WIN32 */