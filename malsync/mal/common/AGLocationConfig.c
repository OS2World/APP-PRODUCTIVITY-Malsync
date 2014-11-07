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

#include <AGLocationConfig.h>
#include <AGUtil.h>
#include <AGBase64.h>

#define MAX_PATH 1024
/* ----------------------------------------------------------------------------
*/
char * AGDescribeExclusionArray(AGArray * array)
{

    int i, n;
    char * buf;
    AGBool appendSeparator = FALSE;

    n = AGArrayCount(array);
    if (n < 1)
        return NULL;

    buf = (char*)malloc(MAX_PATH * n);
    if (NULL == buf)
        return NULL;

    buf[0] = '\0';
    for (i = 0; i < n; ++i) {

        char * name;

        name = (char*)AGArrayElementAt(array,
            i);

        if (NULL == name)
            continue;

        if (appendSeparator)
            strcat(buf, "; ");
        else
            appendSeparator = TRUE;

        strcat(buf, name);

    }

    return buf;
}

/* ----------------------------------------------------------------------------
*/
AGArray * AGFillExclusionArray(char * list)
{

    AGArray * result;
    char * delim = "\n ;,\t";
    char *token, *ptr, *strptr, *excludeString;

    result = AGArrayNew(AGOwnedStringElements, 0);

    if (NULL == result)
        return NULL;

    token = strtok(list, delim);
        
    while (NULL != token) {

        strptr = token;
        excludeString = ptr = strdup(token);
        *ptr = 0;

        /* this is brain dead, but some people insist on putting a
           "*" in front of the exclude string. This also removes white
           space which will fuck up the string compare later */
        while(*strptr) {
            if (!isspace(*strptr) && *strptr != '*')
                *ptr++ = *strptr;
            strptr++;
        }

        *ptr = 0;

        if (!*excludeString) {
            free(excludeString);
            continue;
        }

        AGArrayAppend(result, excludeString);

        token = strtok(NULL, delim);

    }

    return result;

}

/* ----------------------------------------------------------------------------
*/
void AGLocationConfigInit(AGLocationConfig * obj)
{
    if (NULL != obj) {
        bzero(obj, sizeof(AGLocationConfig));
        obj->exclusionServers = AGArrayNew(AGOwnedStringElements, 0);
    }
}

/* ----------------------------------------------------------------------------
*/
void AGLocationConfigFinalize(AGLocationConfig * obj)
{
    if (NULL != obj) {

        CHECKANDFREE(obj->HTTPName);
        CHECKANDFREE(obj->HTTPUsername);
        CHECKANDFREE(obj->HTTPPassword);
        CHECKANDFREE(obj->SOCKSName);
        CHECKANDFREE(obj->SecureName);
        CHECKANDFREE(obj->autoConfigProxyURL);

        if (NULL != obj->exclusionServers)
            AGArrayFree(obj->exclusionServers);

        bzero(obj, sizeof(AGLocationConfig));
    }
}

/* ----------------------------------------------------------------------------
*/
AGLocationConfig * AGLocationConfigNew(void)
{
    AGLocationConfig * result =
        (AGLocationConfig *)malloc(sizeof(AGLocationConfig));
    AGLocationConfigInit(result);
    return result;
}

/* ----------------------------------------------------------------------------
*/
void AGLocationConfigFree(AGLocationConfig * obj)
{
    if (NULL != obj) {
        AGLocationConfigFinalize(obj);
        free(obj);
    }
}

#define agSIGNATURE_HIGH      (0xD5)
#define agSIGNATURE_LOW       (0xAA)
#define agVERSION_MAJ_0       (0)
#define agVERSION_MIN_0       (0)
#define agCURRENT_MAJ_VER     agVERSION_MAJ_0
#define agCURRENT_MIN_VER     agVERSION_MIN_0

/* ----------------------------------------------------------------------------
    int32 AGLocationConfigReadData(AGLocationConfig * obj, AGReader * r)
*/
int32 AGLocationConfigReadData(AGLocationConfig * obj, AGReader * r)
{
    int n;
    int32 majver, minver;
    char * tmp;
    int32 len;

    if (AGReadInt16(r) != ((agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW))
        return AG_ERROR_INVALID_SIGNATURE;

    majver = AGReadCompactInt(r);
    minver = AGReadCompactInt(r);
    obj->source = (AGLocationConfigSource)AGReadCompactInt(r);
    obj->HTTPUseProxy = AGReadBoolean(r);
    CHECKANDFREE(obj->HTTPName);
    obj->HTTPName = AGReadCString(r);
    obj->HTTPPort = AGReadInt16(r);
    obj->HTTPUseAuthentication = AGReadBoolean(r);
    CHECKANDFREE(obj->HTTPUsername);
    tmp = AGReadCString(r);
    if (NULL != tmp) {
        obj->HTTPUsername = (char *) AGBase64Decode(tmp, &len);
        free(tmp);
    }
    CHECKANDFREE(obj->HTTPPassword);
    tmp = AGReadCString(r);
    if (NULL != tmp) {
        obj->HTTPPassword = (char *) AGBase64Decode(tmp, &len);
        free(tmp);
    }
    obj->SOCKSUseProxy = AGReadBoolean(r);
    CHECKANDFREE(obj->SOCKSName);
    obj->SOCKSName = AGReadCString(r);
    obj->SOCKSPort = AGReadInt16(r);
    AGArrayRemoveAll(obj->exclusionServers);
    n = AGReadCompactInt(r);
    while (n--)
        AGArrayAppend(obj->exclusionServers, AGReadCString(r));
    obj->bypassLocal = AGReadBoolean(r);
    CHECKANDFREE(obj->autoConfigProxyURL);
    obj->autoConfigProxyURL = AGReadCString(r);

    CHECKANDFREE(obj->SecureName);
    obj->SecureName = AGReadCString(r);
    obj->SecurePort = AGReadInt16(r);

    obj->expansion1 = AGReadCompactInt(r);
    obj->expansion2 = AGReadCompactInt(r);
    obj->expansion3 = AGReadCompactInt(r);
    obj->expansion4 = AGReadCompactInt(r);

    obj->reservedLen = AGReadCompactInt(r);
    CHECKANDFREE(obj->reserved);
    if (obj->reservedLen > 0) {
        obj->reserved = malloc(obj->reservedLen);
        AGReadBytes(r, obj->reserved, obj->reservedLen);
    }

    if (majver > agCURRENT_MAJ_VER)
        return AG_ERROR_UNKNOWN_VERSION;

    return AG_ERROR_NONE;
}

void AGLocationConfigWriteData(AGLocationConfig * obj, AGWriter * w)
{
    int i, n;
    char * tmp = NULL;

    AGWriteInt16(w, ((agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW));
    AGWriteCompactInt(w, agCURRENT_MAJ_VER);
    AGWriteCompactInt(w, agCURRENT_MIN_VER);

    AGWriteCompactInt(w, obj->source);
    AGWriteBoolean(w, obj->HTTPUseProxy);
    AGWriteCString(w, obj->HTTPName);
    AGWriteInt16(w, (uint16)obj->HTTPPort);
    AGWriteBoolean(w, obj->HTTPUseAuthentication);
    if (NULL != obj->HTTPUsername)
        tmp = AGBase64Encode((uint8 *) obj->HTTPUsername, 0);
    AGWriteCString(w, tmp);
    CHECKANDFREE(tmp);
    if (NULL != obj->HTTPPassword)
        tmp = AGBase64Encode((uint8 *) obj->HTTPPassword, 0);
    AGWriteCString(w, tmp);
    CHECKANDFREE(tmp);
    AGWriteBoolean(w, obj->SOCKSUseProxy);
    AGWriteCString(w, obj->SOCKSName);
    AGWriteInt16(w, (uint16)obj->SOCKSPort);
    n = AGArrayCount(obj->exclusionServers);
    AGWriteCompactInt(w, n);
    for (i = 0; i < n; ++i)
        AGWriteCString(w, (char*)AGArrayElementAt(obj->exclusionServers, i));
    AGWriteBoolean(w, obj->bypassLocal);
    AGWriteCString(w, obj->autoConfigProxyURL);
    AGWriteCString(w, obj->SecureName);
    AGWriteInt16(w, (uint16)obj->SecurePort);

    AGWriteCompactInt(w, obj->expansion1);
    AGWriteCompactInt(w, obj->expansion2);
    AGWriteCompactInt(w, obj->expansion3);
    AGWriteCompactInt(w, obj->expansion4);

    AGWriteCompactInt(w, obj->reservedLen);
    if (obj->reservedLen > 0)
        AGWriteBytes(w, obj->reserved, obj->reservedLen);
}

AGLocationConfig * AGLocationConfigCopy(AGLocationConfig * dst,
                                        AGLocationConfig * src)
{
    if (NULL == src || NULL == dst)
        return NULL;

    dst->source = src->source;

    dst->HTTPUseProxy = src->HTTPUseProxy;
    CHECKANDFREE(dst->HTTPName);
    if (NULL != src->HTTPName)
        dst->HTTPName = strdup(src->HTTPName);
    dst->HTTPPort = src->HTTPPort;
    dst->HTTPUseAuthentication = src->HTTPUseAuthentication;
    CHECKANDFREE(dst->HTTPUsername);
    if (NULL != src->HTTPUsername)
        dst->HTTPUsername = strdup(src->HTTPUsername);
    CHECKANDFREE(dst->HTTPPassword);
    if (NULL != src->HTTPPassword)
        dst->HTTPPassword = strdup(src->HTTPPassword);

    dst->SOCKSUseProxy = src->SOCKSUseProxy;
    CHECKANDFREE(dst->SOCKSName);
    if (NULL != src->SOCKSName)
        dst->SOCKSName = strdup(src->SOCKSName);
    dst->SOCKSPort = src->SOCKSPort;

    CHECKANDFREE(dst->SecureName);
    if (NULL != src->SecureName)
        dst->SecureName = strdup(src->SecureName);
    dst->SecurePort = src->SecurePort;

    CHECKANDFREE(dst->autoConfigProxyURL);
    if (NULL != src->autoConfigProxyURL)
        dst->autoConfigProxyURL = strdup(src->autoConfigProxyURL);

    {
        int i, n;
        AGArrayRemoveAll(dst->exclusionServers);
        n = AGArrayCount(src->exclusionServers);
        for (i = 0; i < n; ++i) {
            AGArrayAppend(dst->exclusionServers,
                strdup((char*)AGArrayElementAt(src->exclusionServers, i)));
        }
    }
    dst->bypassLocal = src->bypassLocal;
    dst->proxy401 = src->proxy401;

    dst->expansion1 = src->expansion1;
    dst->expansion2 = src->expansion2;
    dst->expansion3 = src->expansion3;
    dst->expansion4 = src->expansion4;

    dst->reservedLen = src->reservedLen;
    CHECKANDFREE(dst->reserved);
    if (NULL != src->reserved) {
        dst->reserved = malloc(src->reservedLen);
        if (NULL != dst->reserved)
            memcpy(dst->reserved, src->reserved, src->reservedLen);
    }

    return dst;
}

AGLocationConfig * AGLocationConfigDup(AGLocationConfig * src)
{
    return AGLocationConfigCopy(AGLocationConfigNew(), src);
}
