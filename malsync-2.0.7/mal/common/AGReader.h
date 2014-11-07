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

#ifndef __AGREADER_H__
#define __AGREADER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>

// The read function takes an opaque pointer as the first arg,
// the second arg is the buffer, and the third arg is the
// number of bytes to read.
//
// n = read(fd, buf, len);

typedef int32 (*AGReadFunc)(void *, void *, int32);

// AGReader is a stateless pass-though which translates high
// level operations like AGReadCommand(r) in to low level read()
// operations.  The underlying input stream is stored as an
// opaque pointer, in, which is passed as the first arg to
// readFunc.  AGReader assumes synchronous operation.  If you
// want to do asynchronous IO, you'll need to do some additional
// work.

typedef struct AGReader {
    void *in;
    AGReadFunc readFunc;
    int32 err;
} AGReader;

ExportFunc AGReader *AGReaderNew(void *in, AGReadFunc readFunc);
ExportFunc AGReader *AGReaderInit(AGReader *r, void *in, AGReadFunc readFunc);
ExportFunc void AGReaderFinalize(AGReader *r);
ExportFunc void AGReaderFree(AGReader *r);

// All of the AGRead*() functions set r->err to non-zero when
// something bad happens and returns -1.

// Returns values in the range 0 ... (2^32 - 1).
// Reads 1-5 bytes from r->in (depending on value).
ExportFunc uint32 AGReadCompactInt(AGReader *r);

// Returns values in the range 0 ... (2^32 - 1).
// Reads 4 bytes from r->in.
ExportFunc uint32 AGReadInt32(AGReader *r);

// Returns values in the range 0 ... (2^24 - 1).
// Reads 3 bytes from r->in.
ExportFunc uint32 AGReadInt24(AGReader *r);

// Returns values in the range 0 ... (2^16 - 1).
// Reads 2 bytes from r->in.
ExportFunc uint16 AGReadInt16(AGReader *r);

// Returns values in the range 0 ... (2^8 - 1).
// Reads 1 byte from r->n.
ExportFunc uint8 AGReadInt8(AGReader *r);

// Calls AGReadInt8, if the resulting value is 1,
// value is set to TRUE, otherwise value is FALSE
ExportFunc AGBool AGReadBoolean(AGReader *r);

// This will not return without reading all the requested bytes.
// It returns the number of bytes read or -1 on error.
ExportFunc uint32 AGReadBytes(AGReader *r, void *buf, int32 len);

// Read past len bytes on the stream 
// Nothing is allocated, return value is -1 on error
// zero otherwise
ExportFunc uint32 AGSkipBytes(AGReader *r, int32 len);

// Reads a compact int off the stream (the length of the
// string) and then allocates space for the string, reads
// and returns the string (null terminated).  The caller
// must free.
ExportFunc char *AGReadString(AGReader *r);

// Read past a string without allocating any memory
// Reads a compact int first, then skips that
// many bytes in the stream.  Returns -1 on error, 
// zero otherwise.
ExportFunc uint32 AGSkipString(AGReader *r);

// Reads a NULL terminated string
ExportFunc char *AGReadCString(AGReader *r);

// Read past a string without allocating any memory
// Reads through the stream until it hits a NULL terminator
// Returns -1 on error, zero otherwise
ExportFunc uint32 AGSkipCString(AGReader *r);

// Interprets the bytes as a CompactInt, returning its length
ExportFunc uint32 AGCompactLenFromBuffer(uint8 *buf);

// Interprets the bytes as a CompactInt, returning its value
ExportFunc uint32 AGCompactIntFromBuffer(uint8 *buf);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGREADER_H__
