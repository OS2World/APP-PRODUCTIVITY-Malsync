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

#include <AGBufferWriter.h>
#include <AGTypes.h>
#include <AGUtil.h>
#include <AGProtectedMem.h>

#define MIN_ALLOC_SIZE 50

ExportFunc AGBufferWriter *AGBufferWriterNew(uint32 cacheSize)
{
    AGBufferWriter *r;
    r = (AGBufferWriter *)malloc(sizeof(AGBufferWriter));
    if(r == NULL)
        return NULL;
    return AGBufferWriterInit(r, cacheSize);
}

ExportFunc AGBufferWriter *AGBufferWriterInit(AGBufferWriter *writer, 
                                                uint32 cacheSize)
{
    if(writer == NULL)
        return NULL;
    bzero(writer, sizeof(*writer));
    AGWriterInit((AGWriter *)writer, writer, AGBufferWriterWrite);
    
    // no cache right now
    writer->buffer = (uint8*)AGProtectedMalloc(cacheSize);
    if(writer->buffer == NULL)
        return NULL; //PENDING(klobad) more cleanup needed
    writer->buffersize = 0;
    writer->buffercapacity = cacheSize;
    return writer;
}

ExportFunc void AGBufferWriterFinalize(AGBufferWriter *writer)
{
    if(writer == NULL)
        return;
    
    AGProtectedFree((void *)writer->buffer);

    AGWriterFinalize((AGWriter *)writer);
}

ExportFunc void AGBufferWriterFree(AGBufferWriter *writer)
{
    if(writer == NULL)
        return;
    AGBufferWriterFinalize(writer);
    free(writer);
}

ExportFunc void AGBufferWriterReset(AGBufferWriter *writer)
{
    writer->buffersize = 0;
}

ExportFunc uint8 *AGBufferWriterGetBuffer(AGBufferWriter *writer)
{
    return (uint8 *)writer->buffer;
}

ExportFunc uint32 AGBufferWriterGetBufferSize(AGBufferWriter *writer)
{
    return writer->buffersize;
}

ExportFunc int32 AGBufferWriterWrite(void *aWriter, void *src, int32 len)
{
    int32 spaceLeft, growLen;
    AGBufferWriter *writer = (AGBufferWriter *)aWriter;
    
    spaceLeft = writer->buffercapacity - writer->buffersize;
    if(spaceLeft < len) {
        // Don't want to realloc for every single byte write
        growLen = len;
        if(growLen < MIN_ALLOC_SIZE)
            growLen = MIN_ALLOC_SIZE;

        writer->buffer = (uint8*)AGProtectedRealloc((void *)writer->buffer, 
                                    writer->buffercapacity + growLen);
        if(writer->buffer == NULL)
            return -1; //PENDING(klobad) badness and hurting
        writer->buffercapacity += growLen;
    }
    
    AGProtectedWrite((void *)writer->buffer, writer->buffersize, src, len);
    writer->buffersize += len;
    return len;
}

ExportFunc uint8 *AGBufferWriterRemoveBuffer(AGBufferWriter *writer)
{
     uint8* toReturn = (uint8 *)writer->buffer;
     writer->buffer = NULL;
     return toReturn;     
}