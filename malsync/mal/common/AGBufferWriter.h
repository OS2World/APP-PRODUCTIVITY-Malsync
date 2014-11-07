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

#ifndef __AGBUFFERWRITER_H__
#define __AGBUFFERWRITER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGWriter.h>

typedef struct AGBufferWriter {
    AGWriter agWriter;
//    uint8 *cacheBuffer;
//    uint32 cacheSize;
//    uint32 cacheIndex;
    const uint8 *buffer;
    uint32 buffersize;
    uint32 buffercapacity;
} AGBufferWriter;

ExportFunc AGBufferWriter *AGBufferWriterNew(uint32 cacheSize);
ExportFunc AGBufferWriter *AGBufferWriterInit(AGBufferWriter *writer, 
                                                uint32 cacheSize);
ExportFunc void AGBufferWriterFinalize(AGBufferWriter *writer);
ExportFunc void AGBufferWriterFree(AGBufferWriter *writer);

ExportFunc void AGBufferWriterReset(AGBufferWriter *writer);

ExportFunc int32 AGBufferWriterWrite(void *aWriter, void *src, int32 len);

ExportFunc uint8 *AGBufferWriterGetBuffer(AGBufferWriter *writer);
ExportFunc uint32 AGBufferWriterGetBufferSize(AGBufferWriter *writer);

ExportFunc uint8 *AGBufferWriterRemoveBuffer(AGBufferWriter *writer);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __AGBUFFERWRITER_H__ */

