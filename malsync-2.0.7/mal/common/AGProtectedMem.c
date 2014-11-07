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

#ifndef USE_CUSTOM_PROTECTED_MEMORY

#include <AGProtectedMem.h>

#ifdef __palmos__

#include <AGUtil.h>

#define MALPROT_DBTYPE             'PrMm'
#define MALPROT_DBNAME             "MBlnProtMem"
#define MOBILE_LINK_CREATOR_ID     'MBln'
#define DEFAULT_CARD_NUM            0

#define PROT_MAGIC_COOKIE 0xFEEDBABE

//PENDING(klobad) this may be slow, but I wanted to remove a global.
//look at some other way to do this. Also reexamine the memory usage of 
//MAL, think that we may not alloc/realloc/free often enough for this to
//be a problem.
static DmOpenRef getDbRef()
{
    DmOpenRef ref = NULL;
    Boolean found = false; 
    LocalID id;
    ULong type, creator;

    do {
        ref = DmNextOpenDatabase(ref);
        type = creator = 0;
        if(ref != NULL) {
            DmOpenDatabaseInfo(ref, &id, NULL, NULL, NULL, NULL);
            DmDatabaseInfo(0, id, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                            NULL, &type, &creator);
        }
        if(type == MALPROT_DBTYPE && creator == MOBILE_LINK_CREATOR_ID)
            found = true;
    } while(!found && ref != NULL);

    return ref;
}

ExportFunc int16 AGProtectedMemoryInit() 
{
    int16 er;
    DmOpenRef protectedDbRef = NULL;

    protectedDbRef = NULL;
    AGProtectedMemoryFinalize();
    
    er = DmCreateDatabase(DEFAULT_CARD_NUM, 
                            MALPROT_DBNAME, 
                            MOBILE_LINK_CREATOR_ID, 
                            MALPROT_DBTYPE, 
                            false);
    if (er) {
        return er;
    }

    protectedDbRef = DmOpenDatabaseByTypeCreator(MALPROT_DBTYPE, 
                            MOBILE_LINK_CREATOR_ID, 
                            dmModeReadWrite);
    if (protectedDbRef == NULL) {
        return -1;
    }

    return 0;
}

ExportFunc void AGProtectedMemoryFinalize() 
{
    int recordCount;
    LocalID dbId;
    UInt attr;
    DmOpenRef protectedDbRef = getDbRef();

    // Release all the records in the tmp database and close the open Db
    if (protectedDbRef != NULL) {
        recordCount = DmNumRecords(protectedDbRef);
        while (recordCount-- > 0) {
            DmReleaseRecord(protectedDbRef, recordCount, false);
        }
        DmCloseDatabase(protectedDbRef);
    }

    dbId = DmFindDatabase(DEFAULT_CARD_NUM, MALPROT_DBNAME);
    if (dbId == 0)
        return;

    DmDatabaseInfo(DEFAULT_CARD_NUM, dbId, NULL, &attr,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if(attr) {
        attr = 0;
        DmSetDatabaseInfo(DEFAULT_CARD_NUM, dbId, NULL, &attr,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    DmDeleteDatabase(DEFAULT_CARD_NUM, dbId);

    return;
}

ExportFunc const void *AGProtectedMalloc(uint32 size)
{
    UInt      index = -1;
    VoidPtr   memoryCacheRecord;
    VoidPtr   memPtr;
    AGProtectedHeader header;
    DmOpenRef protectedDbRef = getDbRef();

    if(protectedDbRef == NULL)
        AGProtectedMemoryInit();
    if(protectedDbRef == NULL)
        return (const void *)NULL;
    
    if(size < 1)
        return (const void *)NULL;

    MemSet(&header, sizeof(AGProtectedHeader), 0);
    memoryCacheRecord = DmNewRecord(protectedDbRef, &index, 
                                size + sizeof(AGProtectedHeader));
    if (memoryCacheRecord == 0) {
        return (const void *)NULL;
    }

    DmRecordInfo(protectedDbRef, index, NULL, &header.recordId, NULL);

    memPtr = MemHandleLock(memoryCacheRecord);
    if (memPtr == NULL) {
        return (const void *)NULL;
    }
    
    header.magicCookie = PROT_MAGIC_COOKIE;
 
    DmWrite(memPtr, 0, &header, sizeof(AGProtectedHeader)); 
    return (const void *)(((uint8 *)memPtr) + sizeof(AGProtectedHeader));
}


ExportFunc const void *AGProtectedRealloc(void* ptr, uint32 newSize)
{
    UInt     curRecordIndex;
    Err      er;
    AGProtectedHeader header;
    VoidHand handle;
    VoidPtr newPtr;
    ULong oldSize;
    DmOpenRef protectedDbRef = getDbRef();

    if(ptr == NULL)
        return AGProtectedMalloc(newSize);

    MemMove(&header, (((uint8 *)ptr) - sizeof(AGProtectedHeader)), 
                sizeof(AGProtectedHeader));
    if(header.magicCookie != PROT_MAGIC_COOKIE)
        return NULL; 

    er = DmFindRecordByID(protectedDbRef, header.recordId, &curRecordIndex);
    if(er != 0)
        return (const void *)NULL; 

    oldSize = MemPtrSize(((uint8 *)ptr) - sizeof(AGProtectedHeader)) 
                - sizeof(AGProtectedHeader);
    MemPtrUnlock((((uint8 *)ptr) - sizeof(AGProtectedHeader)));
    handle = DmResizeRecord(protectedDbRef, curRecordIndex, 
                newSize + sizeof(AGProtectedHeader));
    if(handle == NULL)
        return (const void *)NULL;
    newPtr = MemHandleLock(handle);
    if(newPtr == NULL)
        return (const void *)NULL;
    if(newSize > oldSize) {
        DmSet(newPtr, oldSize + sizeof(AGProtectedHeader), newSize - oldSize, 0);
    }

    return (const void *)(((uint8 *)newPtr) + sizeof(AGProtectedHeader));

}

ExportFunc uint16 AGProtectedWrite(void* dst, uint32 offset, void *src, uint32 len)
{
    return DmWrite(((uint8 *)dst) - sizeof(AGProtectedHeader), 
                    offset + sizeof(AGProtectedHeader), 
                    src, len);

}

ExportFunc void AGProtectedZero(void *ptr, uint32 offset, uint32 len)
{
    DmSet(((uint8 *)ptr) - sizeof(AGProtectedHeader), 
            offset + sizeof(AGProtectedHeader), len, 0);
}

ExportFunc void AGProtectedFree(void* ptr)
{
    UInt     curRecordIndex;
    Err      er;
    AGProtectedHeader header;
    DmOpenRef protectedDbRef = getDbRef();

    if(ptr == NULL)
        return;

    MemMove(&header, (((uint8 *)ptr) - sizeof(AGProtectedHeader)), 
                sizeof(AGProtectedHeader));
    if(header.magicCookie != PROT_MAGIC_COOKIE)
        return;

    er = DmFindRecordByID(protectedDbRef, header.recordId, &curRecordIndex);
    if(er != 0)
        return;
    MemPtrUnlock((((uint8 *)ptr) - sizeof(AGProtectedHeader)));
    DmRemoveRecord(protectedDbRef, curRecordIndex); 
    return;
}

ExportFunc char *AGProtectedStrDup(char *str)
{
    char *ptr;
    int32 len;

    if(!str || *str == '\0')
        return NULL;

    len = strlen(str)+1;
    ptr = (char *)AGProtectedMalloc(len);
    AGProtectedWrite(ptr, 0, str, len);
    return ptr;
}

#endif /* __palmos__ */

ExportFunc const void *AGProtectedDup(void *ptr, uint32 nbytes)
{
    const void *q = AGProtectedMalloc(nbytes);
    AGProtectedWrite((void *) q, 0, ptr, nbytes);
    return q;
}

#endif /* !USE_CUSTOM_PROTECTED_MEMORY*/
