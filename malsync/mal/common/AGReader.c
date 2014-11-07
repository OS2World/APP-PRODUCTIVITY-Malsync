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

// Owner: linus

#include <AGReader.h>
#include <AGUtil.h>

ExportFunc AGReader *AGReaderNew(void *in, AGReadFunc readFunc)
{
    AGReader *r;

    r = (AGReader *)malloc(sizeof(AGReader));
    return AGReaderInit(r, in, readFunc);
}

ExportFunc AGReader *AGReaderInit(AGReader *r, void *in, AGReadFunc readFunc)
{
    bzero(r, sizeof(*r));
    r->in = in;
    r->readFunc = readFunc;

    return r;
}

ExportFunc void AGReaderFinalize(AGReader *r)
{
    bzero(r, sizeof(*r));
}

ExportFunc void AGReaderFree(AGReader *r)
{
    AGReaderFinalize(r);
    free(r);
}

ExportFunc uint32 AGReadCompactInt(AGReader *r)
{
    uint32 val;

    val = AGReadInt8(r);
    if (val <= 253) {
        return val;
    } else if (val == 254) {
        return AGReadInt16(r);
    } else if (val == 255) {
        return AGReadInt32(r);
    }

    return -1;
}

ExportFunc uint32 AGReadInt32(AGReader *r)
{
    uint8 buf[4];
    int32 count;

    count = AGReadBytes(r, buf, 4);
    if (count != 4) {
        return -1;
    }

    return ((uint32)buf[0] << 24) 
            | ((uint32)buf[1] << 16) 
            | ((uint32)buf[2] << 8) 
            | (uint32)buf[3];
}

ExportFunc uint32 AGReadInt24(AGReader *r)
{
    uint8 buf[3];
    int32 count;

    count = AGReadBytes(r, buf, 3);
    if (count != 3) {
        return -1;
    }

    return (((uint32)buf[0] << 16) 
            | ((uint32)buf[1] << 8) 
            | (uint32)buf[2]);
}

ExportFunc uint16 AGReadInt16(AGReader *r)
{
    uint8 buf[2];
    int32 count;

    count = AGReadBytes(r, buf, 2);
    if (count != 2) {
        return -1;
    }

    return (buf[0] << 8) | buf[1];
}

ExportFunc uint8 AGReadInt8(AGReader *r)
{
    uint8 buf[1];
    int32 count;

    if (r->err) {
        return -1;
    }

    count = (*r->readFunc)(r->in, buf, 1);
    if (count != 1) {
        r->err = -1;
        return -1;
    }

    return buf[0];
}

ExportFunc AGBool AGReadBoolean(AGReader *r)
{
    int8 readValue;

    if (r->err) {
        return FALSE;
    }

    readValue = AGReadInt8(r);
    if (readValue == -1) {
        r->err = -1;
        return FALSE;
    } else if (readValue > 0)
        return TRUE;
    else
        return FALSE;
}


ExportFunc uint32 AGReadBytes(AGReader *r, void *buf, int32 len)
{
    int32 count, origLen;

    if (r->err) {
        return -1;
    }

    origLen = len;

    while (len > 0) {
        count = (*r->readFunc)(r->in, buf, len);
        if (count <= 0) {
            r->err = -1;
            return -1;
        }

        len -= count;
        buf = ((uint8 *)buf) + count;
    }

    return origLen;
}

//PENDING - should we just allocate len bytes and call read once?
ExportFunc uint32 AGSkipBytes(AGReader *r, int32 len)
{
    int32 i, count;
    uint8 buf[1];

    if (r->err) {
        return -1;
    }

    for (i = 0; i < len; i++) {
        count = (*r->readFunc)(r->in, buf, 1);
        if (count != 1) {
            r->err = -1;
            return -1;
        }
    }

    return 0;
}


ExportFunc char *AGReadString(AGReader *r)
{
    int32 len;
    char *buf;

    len = AGReadCompactInt(r);
    if (len <= 0) {
        return NULL;
    }


    buf = (char*)malloc(len + 1);
    AGReadBytes(r, buf, len);
    buf[len] = 0;

    return buf;
}

ExportFunc uint32 AGSkipString(AGReader *r)
{
    int32 len;

    if (r->err) {
        return -1;
    }

    len = AGReadCompactInt(r);
    if (len <= 0) {
        return 0;
    }

    return AGSkipBytes(r, len);
}

#define AG_STR_CHUNK_SIZE   150
ExportFunc char *AGReadCString(AGReader *r)
{
    int32 count, index, len = 0;
    char *result, *temp;
    char temp2[AG_STR_CHUNK_SIZE];
    AGBool freeTemp = FALSE;

    temp = temp2;
    len = AG_STR_CHUNK_SIZE;
    index = -1;

    do {
        index++;

        if (index >= len) {
            if (freeTemp == FALSE) {
                temp = (char*)malloc(len+AG_STR_CHUNK_SIZE);
                memcpy(temp, temp2, AG_STR_CHUNK_SIZE);
                freeTemp = TRUE;
            } else
                temp = (char*)realloc(temp, len+AG_STR_CHUNK_SIZE);
            len += AG_STR_CHUNK_SIZE;
        }

        count = (*r->readFunc)(r->in, temp+index, 1);
        if (count != 1) {
            r->err = -1;
            if (freeTemp)
                free(temp);
            return NULL;
        }
        if (index == 0 && *(temp+index) == 0) {
            // first character is NULL
            if (freeTemp)
                free(temp);
            return NULL;
        }
    } while (*(temp+index) != 0);



    result = (char*)malloc(index+1);
    memcpy(result, temp, index+1);
    if (freeTemp)
        free(temp);

    return result;
}

ExportFunc uint32 AGSkipCString(AGReader *r)
{
    int32 count;
    uint8 buf[1];

    if (r->err) {
        return -1;
    }

    do {
        count = (*r->readFunc)(r->in, buf, 1);
        if (count != 1) {
            r->err = -1;
            return -1;
        }
    } while (buf[0] != 0);

    return 0;
}

ExportFunc uint32 AGCompactLenFromBuffer(uint8 *buf)
{
    if (*buf <= 253) {
        return 1;
    } else if (*buf == 254) {
        return 3;
    } else if (*buf == 255) {
        return 5;
    }
    return -1;
}
 
ExportFunc uint32 AGCompactIntFromBuffer(uint8 *buf)
{
    uint32 len = AGCompactLenFromBuffer(buf);

    if (len == 1) {
        return (uint32)buf[0];
    } else if (len == 3) {
        return (((uint32)buf[1]) << 8) 
                | (uint32)buf[2];
    } else if (len == 5) {
        return ((uint32)buf[1] << 24) 
                | ((uint32)buf[2] << 16) 
                | ((uint32)buf[3] << 8) 
                | (uint32)buf[4];
    }
    return -1;
}

