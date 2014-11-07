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


#ifndef __AGPASSWORDPROMPT_H__
#define __AGPASSWORDPROMPT_H__

#ifdef _WIN32 /**** Entire file is NOOP if not on Windows. *****/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>
#include <windows.h>

ExportFunc int AGPasswordPromptProxy(HINSTANCE hInstance,
                                     HWND hwndParent,
                                     int idDialog,
                                     int idText,
                                     int idUsernameCtrl,
                                     int idPasswordCtrl,
                                     char * proxyservername,
                                     int proxyserverport,
                                     char ** username,
                                     char ** password);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */

#endif /* __AGPASSWORDPROMPT_H__ */

