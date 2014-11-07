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

#include <AGBufferReader.h>
#include <AGTypes.h>
#include <AGUtil.h>
#include <AGProtectedMem.h>

ExportFunc AGBufferReader *AGBufferReaderNew(uint8 *buf)
{
    AGBufferReader *r;
    r = (AGBufferReader *)malloc(sizeof(AGBufferReader));
    if(r == NULL)
        return NULL;
    return AGBufferReaderInit(r, buf);
}

ExportFunc AGBufferReader *AGBufferReaderInit(AGBufferReader *reader, uint8 *buf)
{
    if(reader == NULL)
        return NULL;
    bzero(reader, sizeof(*reader));
    AGReaderInit((AGReader *)reader, reader, AGBufferReaderRead);
    reader->buffer = buf;
    return reader;
}

ExportFunc void AGBufferReaderFinalize(AGBufferReader *reader)
{
    if(reader == NULL)
        return;
    AGReaderFinalize((AGReader *)reader);
    bzero(reader, sizeof(*reader));
}

ExportFunc void AGBufferReaderFree(AGBufferReader *reader)
{
    if(reader == NULL)
        return;
    AGBufferReaderFinalize(reader);
    free(reader);
}

ExportFunc int32 AGBufferReaderRead(void *aReader, void *dst, int32 len)
{
    AGBufferReader *reader = (AGBufferReader *)aReader;
    bcopy((void *)(reader->buffer + reader->currentIndex), dst, len);
    reader->currentIndex += len;
    return len;
}

ExportFunc uint8 *AGBufferReaderPeek(AGBufferReader *reader)
{
    return reader->currentIndex + reader->buffer;
}

ExportFunc char * AGReadProtectedCString(AGBufferReader *reader)
{
    uint8 *ptr;
    char *mallocPtr;
    int32 strLen;

    ptr = AGBufferReaderPeek(reader);
    if(ptr && *ptr != '\0') {
       strLen = strlen((char*)ptr);
    } else  {
        AGBufferReaderSkipBytes(reader, 1);
        return NULL;
    }

    mallocPtr = AGProtectedStrDup((char *)ptr);
    AGBufferReaderSkipBytes(reader, strLen+1);   
    return mallocPtr;
}

ExportFunc uint32 AGReadProtectedBytes(AGBufferReader *r, void *buf, int32 len)
{
    uint8 *ptr;
    
    if(!buf) {
        AGBufferReaderSkipBytes(r, len);
        return len;
    }
    
    ptr = AGBufferReaderPeek(r);
    AGProtectedWrite(buf, 0, ptr, len);
    AGBufferReaderSkipBytes(r, len);
    return len;
}


ExportFunc void AGBufferReaderSkipBytes(AGBufferReader *r, int32 len)
{
    r->currentIndex += len;
}

ExportFunc void AGBufferReaderSkipCString(AGBufferReader *r)
{
    if(!r->buffer)
        return;
        
    while(r->buffer[r->currentIndex++] != '\0')
    {}
}

