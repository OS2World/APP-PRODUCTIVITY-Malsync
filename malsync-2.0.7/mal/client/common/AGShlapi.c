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

#include <AGShlapi.h>

static BOOL appendSlash(char * str)
{
	int len;

    if (NULL == str)
        return FALSE;

    len = strlen(str);

	if (len > 0)
		return ('\\' != str[len - 1]);

    return FALSE;
}

LPSTR PathFindExtension(LPCSTR pPath) {
    if (pPath) {
        return strrchr(pPath, '.');
    } else {
        return NULL;
    }
}

BOOL PathIsURL(LPCSTR pPath) {
    return TRUE;
}

LPSTR PathCombine(LPSTR pDest, LPCSTR pDir, LPCSTR pFile) {
    if (pDest && pDir && pFile) {
        int dirLen = strlen(pDir);

        strcpy(pDest, pDir);
        if (appendSlash(pDest)) {
            pDest[dirLen] = '\\';
            strcpy(pDest + dirLen + 1, pFile);
        } else
            strcpy(pDest + dirLen, pFile);
    }
    return pDest;
}

static BOOL isAbsolute(LPCSTR s) {
    return ((NULL != s) && (strlen(s) >= 2) && (':' == s[1]));
}

BOOL PathAppend(LPSTR pDir, LPCSTR pFile) {
    if (pDir && pFile) {
        LPSTR d;
        LPCSTR f;
        int dirLen;

        d = pDir;
        f = pFile;

        if (isAbsolute(f)) {
            while (*f && ((tolower(*d) == tolower(*f)))) {
                d++;f++;
            }
        }
        dirLen = strlen(d);
        if (appendSlash(d) && f[0] != '\\') {
            d[dirLen] = '\\';
            strcpy(d + dirLen + 1, f);
        } else
            strcpy(d + dirLen, f);
    }
    return TRUE;
}

BOOL PathRemoveFileSpec(LPSTR pPath) {
    if (pPath) {
        char *pSlash = strrchr(pPath, '\\');

        if (pSlash) {
            *pSlash = '\0';
            return TRUE;
        }
    }
    return FALSE;
}