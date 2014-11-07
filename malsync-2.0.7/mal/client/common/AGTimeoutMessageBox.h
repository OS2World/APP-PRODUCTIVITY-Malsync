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

#ifndef __AGTIMEOUTMESSAGEBOX_H__
#define __AGTIMEOUTMESSAGEBOX_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <windows.h>

#define kTimeoutValue 15000

int MessageBoxTimeout(HWND hWnd,
                      LPTSTR lpText,
                      LPTSTR lpCaption,
                      UINT uType,
                      UINT uMillisecondsBeforeTimeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __AGTIMEOUTMESSAGEBOX_H__ */

#endif /* #ifdef _WIN32 */