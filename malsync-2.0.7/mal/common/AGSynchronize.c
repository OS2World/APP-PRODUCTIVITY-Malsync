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

#include <AGUtil.h>
#include <AGSynchronize.h>

int8 AGSynchronizeInt8(int8 a, int8 d, int8 r)
{
    if (a != d)
        return d;
    else if (a != r)
        return r;
    return a;
}

int16 AGSynchronizeInt16(int16 a, int16 d, int16 r)
{
    if (a != d)
        return d;
    else if (a != r)
        return r;
    return a;
}

int32 AGSynchronizeInt32(int32 a, int32 d, int32 r)
{
    if (a != d)
        return d;
    else if (a != r)
        return r;
    return a;
}

AGBool AGSynchronizeBoolean(AGBool a, AGBool d, AGBool r)
{
    if (a != d)
        return d;
    else if (a != r)
        return r;
    return a;
}

char * AGSynchronizeString(char * a, char * d, char * r)
{
    if (NULL == d && NULL == r)
        return NULL;

    if (NULL == a) {
        if (NULL != d)
            return strdup(d);
        else
            return strdup(r);
    }

    if (NULL != d) {
        if (strcmp(a, d))
            return strdup(d);
        else {
            if (NULL != r) {
                if (strcmp(a, r))
                    return strdup(r);
            } else
                return NULL;
        }
    } else
        return NULL;

    return strdup(a);
}

static void ag_memdup(void ** dest, int32 * destlen,
                      void * src, int32 srclen)
{
    *dest = malloc(srclen);
    if (NULL != *dest) {
        memcpy(*dest, src, srclen);
        *destlen = srclen;
    }
    else
        *destlen = 0;
}

void AGSynchronizeData(void ** s, int32 * slen,
                       void * a, int32 alen,
                       void * d, int32 dlen,
                       void * r, int32 rlen)
{
    if (NULL == d && NULL == r) {
        *s = NULL;
        *slen = 0;
        return;
    }

    if (NULL == a) {
        if (NULL != d)
            ag_memdup(s, slen, d, dlen);
        else
            ag_memdup(s, slen, r, rlen);
        return;
    }

    if (dlen != alen) {
        if (0 == dlen) {
            *s = NULL;
            *slen = 0;
        } else
            ag_memdup(s, slen, d, dlen);
        return;
    }
    if (NULL != d) {
        if (memcmp(a, d, alen)) {
            ag_memdup(s, slen, d, dlen);
            return;
        }
    }

    if (rlen != alen) {
        if (0 == rlen) {
            *s = NULL;
            *slen = 0;
        } else
            ag_memdup(s, slen, r, rlen);
        return;
    }
    if (NULL != r) {
        if (memcmp(a, r, alen)) {
            ag_memdup(s, slen, r, rlen);
            return;
        }
    }

    ag_memdup(s, slen, a, alen);
}

void AGSynchronizeStackStruct(void * s,
                              void * a, 
                              void * d,
                              void * r,
                              int32 len)
{
    if (memcmp(a, d, len)) {
        memcpy(s, d, len);
        return;
    }

    if (memcmp(a, r, len)) {
        memcpy(s, r, len);
        return;
    }

    memcpy(s, a, len);
}