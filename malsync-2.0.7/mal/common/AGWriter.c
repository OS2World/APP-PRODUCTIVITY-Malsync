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

#include <AGWriter.h>
#include <AGUtil.h>

ExportFunc AGWriter *AGWriterNew(void *out, AGWriteFunc writeFunc)
{
    AGWriter *w;

    w = (AGWriter *)malloc(sizeof(AGWriter));
    return AGWriterInit(w, out, writeFunc);
}

ExportFunc AGWriter *AGWriterInit(AGWriter *w, void *out, AGWriteFunc writeFunc)
{
    bzero(w, sizeof(*w));
    w->out = out;
    w->writeFunc = writeFunc;

    return w;
}

ExportFunc void AGWriterFinalize(AGWriter *w)
{
    bzero(w, sizeof(*w));
}

ExportFunc void AGWriterFree(AGWriter *w)
{
    AGWriterFinalize(w);
    free(w);
}

ExportFunc void AGWriteCompactInt(AGWriter *w, uint32 val)
{
    if (val <= 253) {
        AGWriteInt8(w, (uint8)val);
    } else if (val >= 254 && val < (((uint32)1) << 16)) {
        AGWriteInt8(w, 254);
        AGWriteInt16(w, (uint16)val);
    } else {
        AGWriteInt8(w, 255);
        AGWriteInt32(w, val);
    }
}

ExportFunc void AGWriteInt32(AGWriter *w, uint32 val)
{
    uint8 buf[4];

    buf[0] = (val >> 24) & 0xff;
    buf[1] = (val >> 16) & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[3] = val & 0xff;

    AGWriteBytes(w, buf, 4);
}

ExportFunc void AGWriteInt24(AGWriter *w, uint32 val)
{
    uint8 buf[3];

    buf[0] = (val >> 16) & 0xff;
    buf[1] = (val >> 8) & 0xff;
    buf[2] = val & 0xff;

    AGWriteBytes(w, buf, 3);
}

ExportFunc void AGWriteInt16(AGWriter *w, uint16 val)
{
    uint8 buf[2];

    buf[0] = (val >> 8) & 0xff;
    buf[1] = val & 0xff;

    AGWriteBytes(w, buf, 2);
}

ExportFunc void AGWriteInt8(AGWriter *w, uint8 val)
{
    uint8 buf[1];
    int32 count;

    if (w->err) {
        return;
    }

    buf[0] = val & 0xff;

    if (w->writeFunc != NULL) {
        count = (*w->writeFunc)(w->out, buf, 1);
        if (count != 1) {
            w->err = -1;
            return;
        }
    }
    w->totalBytesWritten++;
}

ExportFunc void AGWriteBoolean(AGWriter *w, AGBool value)
{
    if (value)
        AGWriteInt8(w, 1);
    else
        AGWriteInt8(w, 0);
}

ExportFunc uint32 AGWriteBytes(AGWriter *w, void *buf, int32 len)
{
    int32 count, origLen;

    if (w->err) {
        return -1;
    }

    origLen = len;

    if (w->writeFunc != NULL) {
        while (len > 0) {
            count = (*w->writeFunc)(w->out, buf, len);
            if (count <= 0) {
                w->err = -1;
                return -1;
            }

            len -= count;
            buf = ((uint8 *)buf) + count;
        }
    }
    w->totalBytesWritten += origLen;

    return origLen;
}

ExportFunc void AGWriteString(AGWriter *w, char *str, int32 len)
{
    if (len < 0 || str == NULL) {
        len = 0;
    }
    AGWriteCompactInt(w, len);

    if (len > 0) {
        AGWriteBytes(w, str, len);
    }
}

ExportFunc void AGWriteCString(AGWriter *w, char *str)
{
    if (str == NULL) {
        AGWriteInt8(w, 0);
        return;
    } else {
        AGWriteBytes(w, str, strlen(str)+1);
    }
}
