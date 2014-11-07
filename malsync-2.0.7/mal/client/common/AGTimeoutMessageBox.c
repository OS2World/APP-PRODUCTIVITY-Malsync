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

#include <AGTimeoutMessageBox.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

const UINT kMillisecondsToWaitForDialog = 500;

typedef struct {
    DWORD mainThreadId;
    UINT uMillisecondsBeforeTimeout;
    LPTSTR szWindowTitle;
    LPTSTR szStartTitle;
    HANDLE finishEvent;
} messageBoxData;

typedef struct {
    HWND hwndFound;
    LPTSTR szWindowTitle;
} enumData;

static BOOL CALLBACK enumThreadWndProc(HWND hwnd, LPARAM lParam)
{
    TCHAR classname[MAX_PATH];
    BOOL result = TRUE;
    enumData * enumResult = (enumData *)lParam;

    GetClassName(hwnd, classname, MAX_PATH);
    if (!_tcscmp(classname, _T("#32770")))
    {
        TCHAR windowtitle[MAX_PATH];
        GetWindowText(hwnd, windowtitle, MAX_PATH);
        if (!_tcscmp(windowtitle, enumResult->szWindowTitle))
        enumResult->hwndFound = hwnd;
        result = FALSE;
    }
    return result;
}

HWND findMessageBox(DWORD threadId, LPTSTR title)
{
    enumData enumResult;

    enumResult.hwndFound = NULL;
    enumResult.szWindowTitle = title;

    EnumThreadWindows(threadId, enumThreadWndProc, (LPARAM)&enumResult);

    return enumResult.hwndFound;
}

static UINT reduceToZero(UINT start, UINT delta)
{
    if (delta >= start)
        return 0;
    return start - delta;
}

static stuffInSeconds(LPTSTR buf, LPTSTR proto, UINT secs)
{
    _stprintf(buf, proto, secs / 1000);
}

static unsigned int _stdcall messageBoxTimeout(void * out)
{
    messageBoxData * mbData;
    HWND hwndMB;
    LPTSTR titleBuf;
    DWORD waitresult;

    mbData = (messageBoxData *)out;

    /* Wait for the message box to come up */
    Sleep(kMillisecondsToWaitForDialog);

    /* Get handle to the message box. */
    hwndMB = findMessageBox(mbData->mainThreadId,
        mbData->szStartTitle);

    if (NULL == hwndMB)
        return 0;

    titleBuf = (LPTSTR)malloc((_tcslen(mbData->szWindowTitle) + 16)
        * sizeof(TCHAR));

    if (NULL != titleBuf) {

        UINT waitDelta = 1000;

        while (mbData->uMillisecondsBeforeTimeout > 0) {

            waitresult = WaitForSingleObject(mbData->finishEvent,
                min(waitDelta, mbData->uMillisecondsBeforeTimeout));

            if (WAIT_TIMEOUT == waitresult) {

                mbData->uMillisecondsBeforeTimeout = reduceToZero(
                    mbData->uMillisecondsBeforeTimeout,
                    waitDelta);

                stuffInSeconds(titleBuf,
                    mbData->szWindowTitle,
                    mbData->uMillisecondsBeforeTimeout);

                if (IsWindow(hwndMB))
                    SetWindowText(hwndMB, titleBuf);

            }
            else
                mbData->uMillisecondsBeforeTimeout = 0;

        }

        if (IsWindow(hwndMB))
            EndDialog(hwndMB, IDOK);

        free(titleBuf);

    }

    return 0;
}

int MessageBoxTimeout(HWND hWnd,
                      LPTSTR lpText,
                      LPTSTR lpCaption,
                      UINT uType,
                      UINT uMillisecondsBeforeTimeout)
{
    int result = 0;
    DWORD threadId;
    LPTSTR titleProto = TEXT("%s (%s seconds until dismissed)");
    TCHAR titleProtoBuf[MAX_PATH];
    TCHAR startBuf[MAX_PATH];
    messageBoxData mbData;
    HANDLE threadHandle = NULL;

    memset(&mbData, 0, sizeof(mbData));

    _stprintf(titleProtoBuf, titleProto, lpCaption, TEXT("%d"));

    mbData.mainThreadId = GetCurrentThreadId();
    mbData.uMillisecondsBeforeTimeout = uMillisecondsBeforeTimeout;
    mbData.szWindowTitle = titleProtoBuf;
    mbData.finishEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    threadHandle = CreateThread(NULL,
        0,
        messageBoxTimeout,
        &mbData,
        0,
        &threadId);

    stuffInSeconds(startBuf,
        titleProtoBuf,
        mbData.uMillisecondsBeforeTimeout);
    mbData.szStartTitle = startBuf;
    result = MessageBox(hWnd, lpText, startBuf, uType);
    
    SetEvent(mbData.finishEvent);
    CloseHandle(mbData.finishEvent);

    WaitForSingleObject(threadHandle, INFINITE);

    return result;
}

#endif /* #ifdef _WIN32 */