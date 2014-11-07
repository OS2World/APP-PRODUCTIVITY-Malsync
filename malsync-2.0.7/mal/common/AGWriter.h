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

#ifndef __AGWRITER_H__
#define __AGWRITER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>

// The write function takes an opaque pointer as the first arg,
// the second arg is the buffer, and the third arg is the
// number of bytes to write.
//
// n = write(fd, buf, len);

typedef int32 (*AGWriteFunc)(void *, void *, int32);

// AGWriter is a stateless pass-though which translates high
// level operations like AGWriteCommand(r) in to low level write()
// operations.  The underlying output stream is stored as an
// opaque pointer, out, which is passed as the first arg to
// writeFunc.  AGWriter assumes synchronous operation.  If you
// want to do asynchronous IO, you'll need to do some additional
// work.

typedef struct AGWriter {
    void *out;
    AGWriteFunc writeFunc;
    int32 err;
    uint32 totalBytesWritten;
} AGWriter;

// Returns the size (in bytes) for a particular compact int.
#define AGCompactSize(c) \
    ((((uint32)(c)) <= 253) ? 1 : (((uint32)(c)) < (((uint32)1 << 16) - 1) ? 3 : 5))

ExportFunc AGWriter *AGWriterNew(void *in, AGWriteFunc writeFunc);
ExportFunc AGWriter *AGWriterInit(AGWriter *w, void *in, AGWriteFunc writeFunc);
ExportFunc void AGWriterFinalize(AGWriter *w);
ExportFunc void AGWriterFree(AGWriter *w);

// All of the AGWrite*() functions set w->err to non-zero when
// something bad happens.

// Returns values in the range 0 ... (2^32 - 1).
// Writes 1-5 bytes to w->out.
ExportFunc void AGWriteCompactInt(AGWriter *w, uint32 val);

// Returns values in the range 0 ... (2^32 - 1).
// Writes 4 bytes to w->out.
ExportFunc void AGWriteInt32(AGWriter *w, uint32 val);

// Returns values in the range 0 ... (2^24 - 1).
// Writes 3 bytes to w->out.
ExportFunc void AGWriteInt24(AGWriter *w, uint32 val);

// Returns values in the range 0 ... (2^16 - 1).
// Writes 2 bytes to w->out.
ExportFunc void AGWriteInt16(AGWriter *w, uint16 val);

// Writes values in the range 0 ... (2^8 - 1).
// Writes 1 byte to w->out.
ExportFunc void AGWriteInt8(AGWriter *w, uint8 val);

// Cover function - calls AGWriteInt8 with 1
// if the value is TRUE, otherwise writes 0
ExportFunc void AGWriteBoolean(AGWriter *w, AGBool value);

// This will not return without writing all the requested bytes.
// It returns the number of bytes written or -1 on error.
ExportFunc uint32 AGWriteBytes(AGWriter *w, void *buf, int32 len);

// Writes the length of the string as a compact int to the
// stream, then writes the string (without the null terminator).
ExportFunc void AGWriteString(AGWriter *w, char *str, int32 len);

// Writes a NULL terminated string
ExportFunc void AGWriteCString(AGWriter *w, char *str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGWRITER_H__
