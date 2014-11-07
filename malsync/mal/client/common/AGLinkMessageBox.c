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

#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

/* WARNING (miket):  The resources for this message box are all screwed up.
    I couldn't get multiple scripts to be included in a single project, so
    the resource.h defines are duplicated in several places, and the
    resources themselves are duplicated.  I recommend grepping for things
    like IDC_LINK to find everything. */
#include <AGLinkMessageBox.h>

typedef struct {
    char * linkText;
    char * lpText;
    char * lpCaption;
    UINT uMillisecondsBeforeTimeout;
    UINT uWaitDelta;
    UINT uTimerID;
    LPTSTR szTimeBuf;
    LPTSTR szWindowTitle;
} linkBoxData;

/* ----------------------------------------------------------------------------
*/
static void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    HINSTANCE hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

    switch (codeNotify) {

        case BN_CLICKED:
            switch (id) {
            case IDOK:
            case IDC_LINK:
                EndDialog(hwnd, id);
                break;
            }
            return;
        default:
            break;
    }
}

/* ----------------------------------------------------------------------------
*/
static void onDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
    linkBoxData * lbd;

    if (lpDrawItem->CtlID != IDC_LINK)
        FORWARD_WM_DRAWITEM(hwnd, lpDrawItem, SendMessage);

    lbd = (linkBoxData *)GetWindowLong(hwnd, GWL_USERDATA);

    if (lbd->linkText) {

        COLORREF oldColor;
        COLORREF linkColor = RGB(0, 0, 255);
        RECT rcWorking;
        HPEN hpen, hpenOld;
        
        hpen = CreatePen(PS_SOLID, 1, linkColor);
        hpenOld = SelectObject(lpDrawItem->hDC, hpen);

        rcWorking = lpDrawItem->rcItem;

        DrawText(lpDrawItem->hDC,
            lbd->linkText,
            strlen(lbd->linkText),
            (const LPRECT)&rcWorking,
            DT_CENTER | DT_CALCRECT);

        OffsetRect(&rcWorking,
            ((lpDrawItem->rcItem.right - lpDrawItem->rcItem.left)
            - (rcWorking.right - rcWorking.left)) / 2, 
            0);
        oldColor = SetTextColor(lpDrawItem->hDC, linkColor);
        DrawText(lpDrawItem->hDC,
            lbd->linkText,
            strlen(lbd->linkText),
            (const LPRECT)&rcWorking,
            0);

        MoveToEx(lpDrawItem->hDC, rcWorking.left, rcWorking.bottom, NULL);
        LineTo(lpDrawItem->hDC, rcWorking.right, rcWorking.bottom);

        SetTextColor(lpDrawItem->hDC, oldColor);

        SelectObject(lpDrawItem->hDC, hpenOld);
        DeleteObject(hpen);

    }
}

/* ----------------------------------------------------------------------------
*/
static BOOL onInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    linkBoxData * lbd;
    HINSTANCE hInst;

    hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

    SetWindowLong(hwnd, GWL_USERDATA, lParam);

    lbd = (linkBoxData *)GetWindowLong(hwnd, GWL_USERDATA);
    SetWindowText(hwnd, lbd->lpCaption);
    SetWindowText(GetDlgItem(hwnd, IDC_TEXT), lbd->lpText);

    lbd->uTimerID = SetTimer(hwnd, 0, 0, NULL);

    return TRUE;
}

static UINT reduceToZero(UINT start, UINT delta)
{
    if (delta >= start)
        return 0;
    return start - delta;
}

static stuffInSeconds(LPTSTR buf, LPTSTR proto, UINT msecs)
{
    UINT secs = msecs / 1000;

    _stprintf(buf, proto, secs / 60, secs % 60);
}

static void onTimer(HWND hwnd, UINT id)
{
    linkBoxData * lbd;
    TCHAR titleBuf[MAX_PATH];

    lbd = (linkBoxData *)GetWindowLong(hwnd, GWL_USERDATA);

    lbd->uMillisecondsBeforeTimeout = reduceToZero(
        lbd->uMillisecondsBeforeTimeout,
        lbd->uWaitDelta);

    if (NULL != lbd->szTimeBuf) {

        stuffInSeconds(titleBuf,
            lbd->szTimeBuf,
            lbd->uMillisecondsBeforeTimeout);

        if (IsWindow(hwnd))
            SetWindowText(GetDlgItem(hwnd, IDC_TIME_DISPLAY), titleBuf);

    }

    if (0 == lbd->uMillisecondsBeforeTimeout && IsWindow(hwnd)) {
        EndDialog(hwnd, IDOK);
        lbd->uTimerID = 0;
    }
    else
        lbd->uTimerID = SetTimer(hwnd, 0, lbd->uWaitDelta, NULL);

}

static void onDestroy(HWND hwnd)
{
    linkBoxData * lbd;

    lbd = (linkBoxData *)GetWindowLong(hwnd, GWL_USERDATA);

    if (0 != lbd->uTimerID)
        KillTimer(hwnd, lbd->uTimerID);
}

BOOL CALLBACK mbrtProc(HWND hwnd, UINT message,
                       WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        HANDLE_MSG(hwnd, WM_COMMAND, onCommand);
        HANDLE_MSG(hwnd, WM_INITDIALOG, onInitDialog);
        HANDLE_MSG(hwnd, WM_DRAWITEM, onDrawItem);
        HANDLE_MSG(hwnd, WM_TIMER, onTimer);
        HANDLE_MSG(hwnd, WM_DESTROY, onDestroy);
    }

    return 0;   /* except for WM_INITDIALOG, returning zero means
                we didn't process the message. */
}

int MessageBoxWithLink(HINSTANCE h,
                       HWND hwnd,
                       LPSTR lpText,
                       LPSTR lpCaption,
                       BOOL bLookForURL,
                       char * url,
                       UINT uMillisecondsBeforeTimeout)
{
    int result = -1;
    HWND hwndDlg = NULL;
    linkBoxData linkData;
    char * linkPosition = NULL;
    WNDCLASS wndClass;

    memset(&linkData, 0, sizeof(linkBoxData));

    linkData.lpCaption = lpCaption;
    linkData.uMillisecondsBeforeTimeout = uMillisecondsBeforeTimeout;
    linkData.uWaitDelta = 500;
    linkData.szTimeBuf = "%.2d:%.2d";

    if (bLookForURL) {

        int chars;

        linkData.linkText = strstr(lpText, "http://");

        if (NULL != linkData.linkText) {

            chars = (linkData.linkText - lpText + 1);

            linkData.lpText = malloc(chars * sizeof(char));

            strncpy(linkData.lpText, lpText, chars - 1);
            linkData.lpText[chars - 1] = '\0';

            if (NULL != url)
                strcpy(url, linkData.linkText); 

        }

    }

    if (NULL == linkData.lpText)
        linkData.lpText = strdup(lpText);

    /* Superclass button class. */
    GetClassInfo(h, "BUTTON", &wndClass);
    wndClass.hInstance = h;
    wndClass.lpszClassName = "AGLINKCONTROL";
    wndClass.hCursor = LoadCursor(h, MAKEINTRESOURCE(IDC_LINK_HAND));
    RegisterClass(&wndClass);

    result = DialogBoxParam(h,
        MAKEINTRESOURCE(IDD_LINK_MESSAGE_BOX),
        hwnd,
        mbrtProc,
        (LPARAM)&linkData);

    if (NULL != linkData.lpText)
        free(linkData.lpText);

    return result;
}

#endif /* #ifdef _WIN32 */
