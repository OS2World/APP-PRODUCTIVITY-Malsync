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
 
/* owner: miket <miket@avantgo.com> */

#ifdef _WIN32

#include <AGProxySearch.h>
#include <AGLocationConfig.h>
#include <windows.h>
#include <stdio.h>
#define int32 netscapeint32
#define uint32 netscapeuint32
#include <reg.h> /* mozilla */
#include <nsreg.h> /* mozilla */
#undef int32
#undef uint32

#include <wininet.h>
#include <AGBase64.h>
#include <AGUtil.h>

static const LPTSTR kIESettingsKey =
    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");

/* ----------------------------------------------------------------------------
*/
int findWindowsInetInternetInformation(AGLocationConfig * lc)
{
    typedef HINTERNET (CALLBACK* INETOPEN)(LPCSTR lpszAgent,
        DWORD dwAccessType,
        LPCSTR lpszProxyName,
        LPCSTR lpszProxyBypass,
        DWORD dwFlags);

    typedef BOOL (CALLBACK* INETQUERYOPTION)(HINTERNET hInternet,
        DWORD dwOption,
        LPVOID lpBuffer,
        LPDWORD lpdwBufferLength);

    typedef BOOL (CALLBACK* INETCLOSEHANDLE)(HINTERNET hInet);

    int ret = AG_PROXY_NONE_FOUND;
    HMODULE hInetLib = NULL;
    HINTERNET hInet = NULL;
    INETOPEN InetOpen = NULL;
    INETQUERYOPTION InetQuery = NULL;
    INETCLOSEHANDLE InetClose = NULL;
    AGBool netLibraryOpen = FALSE;

    __try {

        hInetLib = LoadLibrary(TEXT("WININET.DLL"));

        if (NULL == hInetLib)
            __leave;

        /* Load up the functions we need to find out info from Wininet. */

        InetOpen = (INETOPEN) GetProcAddress(
            hInetLib, TEXT("InternetOpenA"));

#ifdef UNICODE
        InetQuery = (INETQUERYOPTION) GetProcAddress(
            hInetLib, L"InternetQueryOptionW");
#else
        InetQuery = (INETQUERYOPTION) GetProcAddress(
            hInetLib, "InternetQueryOptionA");
#endif /* !UNICODE */

        InetClose = (INETCLOSEHANDLE) GetProcAddress(
            hInetLib, TEXT("InternetCloseHandle"));

        if (NULL == InetOpen || NULL == InetQuery || NULL == InetClose)
            __leave;

        /* Query Wininet for information. */

        hInet = InetOpen("MALCOMMON",
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL,
            NULL,
            0);

        if (NULL != hInet) {

            const int bufSize = 1024;
            char buffer[bufSize];
            DWORD bufLen;
            HKEY hkey = NULL;

            netLibraryOpen = TRUE;

            /* Find out proxy information. */
            bufLen = bufSize;
            if (!InetQuery(NULL, INTERNET_OPTION_PROXY, &buffer, &bufLen))
                __leave;
            
            LPINTERNET_PROXY_INFO inetProxyInfo =
                (LPINTERNET_PROXY_INFO)buffer;

            lc->HTTPUseProxy = FALSE;
            lc->SOCKSUseProxy = FALSE;

            /* Is there an autoproxy URL? */
            RegOpenKeyEx(HKEY_CURRENT_USER,
                kIESettingsKey,
                0,
                KEY_READ,
                &hkey);
            if (NULL != hkey) {

                DWORD dwType;
                LONG hr;

                bufLen = bufSize;
                hr = RegQueryValueEx(hkey,
                    "AutoConfigURL",
                    0,
                    &dwType,
                    (LPBYTE)buffer,
                    &bufLen);
                if (ERROR_SUCCESS == hr) {
                    if (NULL != lc->autoConfigProxyURL) {
                        free(lc->autoConfigProxyURL);
                        lc->autoConfigProxyURL = NULL;
                    }
                    if (strlen(buffer) > 0)
                        lc->autoConfigProxyURL = strdup(buffer);
                }
                RegCloseKey(hkey);
            }

            AGArrayRemoveAll(lc->exclusionServers);

            if (INTERNET_OPEN_TYPE_PROXY == inetProxyInfo->dwAccessType) {
                
                char * cptr, * dptr;
                char sptr[128];
                char buf[MAX_PATH];
                char proxylist[4096];

                /* If there's no qualifier to tell us which kind of
                proxy server this is, then we need to insert one to
                make it an http proxy server. pending(miket): this
                may not be right! It may be that Windows simply tries
                HTTP and SOCKS if one or the other isn't specified. */
                if (NULL == strstr(inetProxyInfo->lpszProxy, "="))
                    strcpy(proxylist, "http=");
                else
                    strcpy(proxylist, "");
                strcat(proxylist, inetProxyInfo->lpszProxy);

                /* Look for an HTTP proxy */
                strcpy(sptr, "http=");
                cptr = strstr(proxylist, sptr);
                if (NULL != cptr) {

                    lc->HTTPUseProxy = TRUE;

                    cptr += strlen(sptr);
                    dptr = buf;
                    while (*cptr != '\0' && *cptr != ' ' && *cptr != ':')
                        *dptr++ = *cptr++;
                    *dptr = '\0';

                    /* Get port number. */
                    if (*cptr == ':') {

                        char number[16];
                        cptr++;
                        dptr = number;
                        while (*cptr != '\0' && *cptr != ' ')
                            *dptr++ = *cptr++;
                        *dptr = '\0';

                        lc->HTTPPort = atoi(number);

                    }

                    if (lc->HTTPUseProxy && lc->HTTPPort <= 0)
                        lc->HTTPPort = 8080;

                    if (NULL != lc->HTTPName)
                        free(lc->HTTPName);
                    lc->HTTPName = strdup(buf);

                }

                /* Look for a SOCKS proxy */
                strcpy(sptr, "socks=");
                cptr = strstr(proxylist, sptr);
                if (NULL != cptr) {

                    lc->SOCKSUseProxy = TRUE;

                    cptr += strlen(sptr);
                    dptr = buf;
                    while (*cptr != '\0' && *cptr != ' ' && *cptr != ':')
                        *dptr++ = *cptr++;
                    *dptr = '\0';

                    /* Get port number. */
                    if (*cptr == ':') {

                        char number[16];
                        cptr++;
                        dptr = number;
                        while (*cptr != '\0' && *cptr != ' ')
                            *dptr++ = *cptr++;
                        *dptr = '\0';

                        lc->SOCKSPort = atoi(number);

                    }

                    if (lc->SOCKSUseProxy && lc->SOCKSPort <= 0)
                        lc->SOCKSPort = 1080;

                    if (NULL != lc->SOCKSName)
                        free(lc->SOCKSName);
                    lc->SOCKSName = strdup(buf);
                }


                if (lc->SOCKSUseProxy || lc->HTTPUseProxy) {

                    lc->exclusionServers = AGFillExclusionArray((char*)
                        inetProxyInfo->lpszProxyBypass);

                    /* pending(miket):  handle <local> */

                }

            }

            ret = AG_PROXY_NO_ERROR;

        }

    }

    __finally {

        if (NULL != hInet && NULL != InetClose && netLibraryOpen) {
            InetClose(hInet);
            netLibraryOpen = FALSE;
        }

        if (NULL != hInetLib) 
            ::FreeLibrary(hInetLib);

    }
    
    return ret;

}

/* ----------------------------------------------------------------------------
*/
void getTokenWithoutQuotes(FILE * f, char * buf)
{
    fscanf(f, "%[^\"]", buf);
    fgetc(f);
    fscanf(f, "%[^\"]", buf);
}

/* ----------------------------------------------------------------------------
*/
static void findNetscapeUserPrefFilename(char * buffer, uint32 buflen)
{
    AGBool success = FALSE;
    HREG hReg = NULL;

    __try {

        if (REGERR_OK != NR_RegOpen(NULL, &hReg))
            __leave;

        RKEY rKey;
        char buf[MAX_PATH];

        if (REGERR_OK != NR_RegGetKey(hReg, ROOTKEY_COMMON,
            "Netscape", &rKey))
            __leave;

        if (REGERR_OK != NR_RegGetKey(hReg, rKey,
            "ProfileManager", &rKey))
            __leave;

        /* Get name of last user who used Netscape. */
        if (REGERR_OK != NR_RegGetEntryString(hReg, rKey,
            "LastNetscapeUser", buf, MAX_PATH))
            __leave;

        /* Get profile location for this user. */
        if (REGERR_OK != NR_RegGetKey(hReg,
            ROOTKEY_USERS,
            buf,
            &rKey))
            __leave;

        if (REGERR_OK != NR_RegGetEntryString(hReg, rKey,
            "ProfileLocation", buffer, buflen))
            __leave;

        success = TRUE;
    }

    __finally {

        if (NULL != hReg)
            NR_RegClose(hReg);

    }

    /* Reading 4.5 registry failed.  Try looking in known registry location
    for older browsers. */
    if (!success) {

        HKEY hKey = NULL;
        HKEY hSubKey = NULL;

        __try {

            char buf[MAX_PATH];

            if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                "SOFTWARE\\Netscape\\Netscape Navigator\\Users",
                0,
                KEY_READ,
                &hKey))
                __leave;

            DWORD size = MAX_PATH;
            if (ERROR_SUCCESS != RegQueryValueEx(hKey,
                "CurrentUser",
                0, NULL, (LPBYTE)buf, &size))
                __leave;

            if (ERROR_SUCCESS != RegOpenKeyEx(hKey,
                buf,
                0,
                KEY_READ,
                &hSubKey))
                __leave;

            size = buflen;
            if (ERROR_SUCCESS != RegQueryValueEx(hSubKey,
                "DirRoot",
                0, NULL, (LPBYTE)buffer, &size))
                __leave;

            success = TRUE;
        }

        __finally {

            if (NULL != hKey)
                RegCloseKey(hKey);
            if (NULL != hSubKey)
                RegCloseKey(hSubKey);
        }

    }

    if (!success)
        strcpy(buffer, "");
}

/* ----------------------------------------------------------------------------
*/
static int findNetscapeInternetInformation(AGLocationConfig * lc)
{
    int ret = AG_PROXY_NONE_FOUND;

    char buf[MAX_PATH];

    /* Open up Netscape's proprietary registry and query it. */

    findNetscapeUserPrefFilename(buf, MAX_PATH);
    if (strlen(buf) == 0)
        return ret;
    
    /* At this point, we know the location of the prefs file for the user
    who last used Netscape. That's a reasonable guess for picking the
    right prefs file to search for proxy information. */

    strcat(buf, "\\prefs.js");
    FILE * f = fopen(buf, "r");
    if (NULL == f)
        return ret;

    int c = 0;
    int httpPort = -1;
    int socksPort = -1;
    int proxyType = 0;
    
    AGArrayRemoveAll(lc->exclusionServers);

    do {

        c = fscanf(f, "%s", &buf);

        if (!strcmp(buf,
            "user_pref(\"network.proxy.type\",")) {
            fscanf(f, "%d", &proxyType);
            continue;
        }

        if (!strcmp(buf, "user_pref(\"network.hosts.socks_server\",")) {
            getTokenWithoutQuotes(f, buf);
            if (NULL != lc->SOCKSName)
                free(lc->SOCKSName);
            lc->SOCKSName = strdup(buf);
            if (socksPort < 0)
                socksPort = 1080;
            lc->SOCKSUseProxy = TRUE;
            continue;
        }

        if (!strcmp(buf, "user_pref(\"network.proxy.http\",")) {
            getTokenWithoutQuotes(f, buf);
            if (NULL != lc->HTTPName)
                free(lc->HTTPName);
            lc->HTTPName = strdup(buf);
            if (httpPort < 0)
                httpPort = 80;
            lc->HTTPUseProxy = TRUE;
            continue;
        }

        if (!strcmp(buf, "user_pref(\"network.proxy.autoconfig_url\",")) {
            getTokenWithoutQuotes(f, buf);
            if (NULL != lc->autoConfigProxyURL)
                free(lc->autoConfigProxyURL);
            lc->autoConfigProxyURL = strdup(buf);
            continue;
        }

        if (!strcmp(buf, "user_pref(\"network.proxy.no_proxies_on\",")) {
            getTokenWithoutQuotes(f, buf);
            lc->exclusionServers = AGFillExclusionArray(buf);
            continue;
        }

        if (!strcmp(buf, "user_pref(\"network.hosts.socks_serverport\",")) {
            fscanf(f, "%d", &socksPort);
            continue;
        }

        if (!strcmp(buf, "user_pref(\"network.proxy.http_port\",")) {
            fscanf(f, "%d", &httpPort);
            continue;
        }

    } while (EOF != c);

    fclose(f);

    /* At this point, we have successfully parsed the Netscape
    user preferences. */
    ret = AG_PROXY_NO_ERROR;

    if (socksPort >= 0)
        lc->SOCKSPort = socksPort;

    if (httpPort >= 0)
        lc->HTTPPort = httpPort;

    /* Note:  Netscape apparently doesn't have a way to specify that
    all local addresses should bypass the proxy, so we just set it false
    in all cases here. */
    lc->bypassLocal = FALSE;

    /* Proxy type zero (which isn't ever put in the source file as far
    as I can tell) means don't use proxy servers.
    
    Proxy type 1 is either socks or HTTP. */
    if (proxyType != 1) {
        lc->HTTPUseProxy = FALSE;
        lc->SOCKSUseProxy = FALSE;
    }

    /* Proxy type 2 means Netscape's automatic proxy configuration. */
    if (proxyType != 2)
        CHECKANDFREE(lc->autoConfigProxyURL);

    if (!(lc->HTTPUseProxy || lc->SOCKSUseProxy))
        AGArrayRemoveAll(lc->exclusionServers);

    return ret;
}

/* ----------------------------------------------------------------------------
*/
int32 AGSearchBrowserSettings(AGLocationConfig * lc, AGBool searchAlternate)
{
    int ret = AG_PROXY_NONE_FOUND;
    HKEY hKey = NULL;

    __try {

        if (NULL == lc) {
            ret = AG_PROXY_NULL_POINTER;
            __leave;
        }

        char buf[MAX_PATH];

        /* This registry key looks for whoever's registered to handle http
         documents.  In English, this is the default browser. */
        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT,
            "http\\shell\\open\\ddeexec\\Application", 0, KEY_READ, &hKey))
            __leave;

        DWORD size = MAX_PATH;
        if (ERROR_SUCCESS != RegQueryValueEx(hKey,
            NULL, 0, NULL, (LPBYTE)buf, &size))
            __leave;

        int preferredBrowser = -1;

        /* See which browser is the default. */
        if (!strcmp(buf, "NSShell"))
            preferredBrowser = AG_PROXY_Netscape;
        else
            preferredBrowser = AG_PROXY_MSIE;

        if (searchAlternate)
            preferredBrowser = (AG_PROXY_MSIE == preferredBrowser)
                ? AG_PROXY_Netscape
                : AG_PROXY_MSIE;

        enum {
            AG_TRY_PREFERRED,
            AG_TRY_OTHER,
            AG_FOUND,
            AG_NONE
        };

        /* In the following state machine, we cycle until we've (a) found
        the proxy information, or (b) failed with every browser we know
        about.  We try in order of preference. */
        int state = AG_TRY_PREFERRED;
        while (1) {

            switch (state) {
                case AG_TRY_PREFERRED: {
                    if (AG_PROXY_MSIE == preferredBrowser) {
                        lc->source = AG_PROXY_MSIE;
                        ret = findWindowsInetInternetInformation(lc);
                    }
                    else {
                        lc->source = AG_PROXY_Netscape;
                        ret = findNetscapeInternetInformation(lc);
                    }

                    if (AG_PROXY_NO_ERROR == ret)
                    {
                        lc->source =
                            (AG_PROXY_MSIE == preferredBrowser) ?
                            AG_PROXY_MSIE : AG_PROXY_Netscape;
                        state = AG_FOUND;
                    }
                    else
                        state = AG_TRY_OTHER;
                }
                    break;
                case AG_TRY_OTHER: {
                    if (AG_PROXY_MSIE == preferredBrowser) {
                        lc->source = AG_PROXY_Netscape;
                        ret = findNetscapeInternetInformation(lc);
                    }
                    else {
                        lc->source = AG_PROXY_MSIE;
                        ret = findWindowsInetInternetInformation(lc);
                    }

                    if (AG_PROXY_NO_ERROR == ret)
                        state = AG_FOUND;
                    else
                        state = AG_NONE;
                }
                    break;
                case AG_FOUND:
                    __leave;
                    break;
                case AG_NONE:
                    lc->source = AG_PROXY_FAILED;
                    __leave;
                    break;
            }
        }
    }
    __finally {

        if (NULL != hKey)
            RegCloseKey(hKey);

        return ret;
    }
}

#endif
