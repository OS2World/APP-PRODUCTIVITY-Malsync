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

#ifndef __AGBUFFERREADER_H__
#define __AGBUFFERREADER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGReader.h>

typedef struct AGBufferReader {
    AGReader agReader;
    uint8 *buffer;
    uint32 currentIndex;
} AGBufferReader;

ExportFunc AGBufferReader *AGBufferReaderNew(uint8 *buf);
ExportFunc AGBufferReader *AGBufferReaderInit(AGBufferReader *reader, uint8 *buf);
ExportFunc void AGBufferReaderFinalize(AGBufferReader *reader);
ExportFunc void AGBufferReaderFree(AGBufferReader *reader);
ExportFunc uint8 *AGBufferReaderPeek(AGBufferReader *reader);

ExportFunc int32 AGBufferReaderRead(void *aReader, void *dst, int32 len);
ExportFunc char * AGReadProtectedCString(AGBufferReader *reader);
ExportFunc uint32 AGReadProtectedBytes(AGBufferReader *r, void *buf, int32 len);
ExportFunc void AGBufferReaderSkipBytes(AGBufferReader *r, int32 len);
ExportFunc void AGBufferReaderSkipCString(AGBufferReader *r);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __AGBUFFERREADER_H__ */

