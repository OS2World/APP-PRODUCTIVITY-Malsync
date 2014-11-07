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

#ifdef _WIN32 /**** Entire file is NOOP if not on Windows. *****/
#include <AGPasswordPrompt.h>
#include <windowsx.h>
#include <stdio.h>

typedef struct {
    HINSTANCE hInstance;
    int idText;
    int idUsernameCtrl;
    int idPasswordCtrl;
    char * proxyservername;
    int proxyserverport;
    char ** username;
    char ** password;
} pwpInfo;

/* ----------------------------------------------------------------------------
*/
static char * windowTextDup(HWND hwnd)
{
    int len;
    char * result = NULL;
    len = GetWindowTextLength(hwnd) + 1;
    if (len != 0) {
        result = (char*)malloc(len);
        GetWindowText(hwnd, result, len);
    }
    return result;
}

/* ----------------------------------------------------------------------------
*/
static BOOL onInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    char proto[2048];
    char buf[2048];
    pwpInfo * pInfo;
    
    SetWindowLong(hwnd, GWL_USERDATA, lParam);
    pInfo = (pwpInfo *)GetWindowLong(hwnd, GWL_USERDATA);

    GetDlgItemText(hwnd, pInfo->idText, proto, sizeof(proto));
    _snprintf(buf,
        sizeof(proto),
        proto, 
        pInfo->proxyservername,
        pInfo->proxyserverport);
    SetDlgItemText(hwnd, pInfo->idText, buf);

    if (NULL != *pInfo->username)
        SetDlgItemText(hwnd, pInfo->idUsernameCtrl, *pInfo->username);
    if (NULL != *pInfo->password)
        SetDlgItemText(hwnd, pInfo->idPasswordCtrl, *pInfo->password);

    return TRUE;
}

/* ----------------------------------------------------------------------------
*/
static void doOK(HWND hwnd, pwpInfo * pInfo)
{
    if (NULL != *pInfo->username)
        free(*pInfo->username);
    *pInfo->username = windowTextDup(GetDlgItem(hwnd, pInfo->idUsernameCtrl));
    if (NULL != *pInfo->password)
        free(*pInfo->password);
    *pInfo->password = windowTextDup(GetDlgItem(hwnd, pInfo->idPasswordCtrl));
}

/* ----------------------------------------------------------------------------
*/
static void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    pwpInfo * pInfo = (pwpInfo *)GetWindowLong(hwnd, GWL_USERDATA);

    switch (codeNotify) {
        case BN_CLICKED:
            switch (id) {
                case IDOK: {
                    doOK(hwnd, pInfo);
                    EndDialog(hwnd, IDOK);
                    return;
                }
                case IDCANCEL: {
                    EndDialog(hwnd, IDCANCEL);
                    return;
                }
                default:
                    break;
            }
            break;
        default:
            break;

    }
}

/* ----------------------------------------------------------------------------
*/
static BOOL CALLBACK dialogProc(HWND hwnd, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(hwnd, WM_INITDIALOG, onInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, onCommand);
    }

    return 0;   /* except for WM_INITDIALOG, returning zero means
                 we didn't process the message. */
}

int AGPasswordPromptProxy(HINSTANCE hInstance,
                          HWND hwndParent,
                          int idDialog,
                          int idText,
                          int idUsernameCtrl,
                          int idPasswordCtrl,
                          char * proxyservername,
                          int proxyserverport,
                          char ** username,
                          char ** password)
{
    pwpInfo info;

    info.hInstance = hInstance;
    info.idText = idText;
    info.idUsernameCtrl = idUsernameCtrl;
    info.idPasswordCtrl = idPasswordCtrl;
    info.proxyservername = proxyservername;
    info.proxyserverport = proxyserverport;
    info.username = username;
    info.password = password;

    return DialogBoxParam(hInstance, 
        MAKEINTRESOURCE(idDialog),
        hwndParent,
        dialogProc, 
        (DWORD)&info);

}

#endif /* #ifdef _WIN32 */