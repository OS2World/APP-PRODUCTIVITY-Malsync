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

#include <AGDBConfig.h>
#include <AGUtil.h>
#ifndef REMOVE_SYNCHRONIZE_FEATURE
#include <AGSynchronize.h>
#endif

static AGArray *dupNewIdArray(AGArray *src);

ExportFunc AGDBConfig *AGDBConfigNew(char *dbname,
                                     AGDBConfigType type, 
                                     AGBool sendRecordPlatformData, 
                                     int32 platformDataLength, 
                                     void *platformData,
                                     AGArray *newids)
{
    AGDBConfig *obj;

    obj = (AGDBConfig*)malloc(sizeof(AGDBConfig));
    if (NULL == obj)
        return NULL;
    return AGDBConfigInit(obj,
        dbname,
        type, 
        sendRecordPlatformData,
        platformDataLength,
        platformData,
        newids);
}

ExportFunc AGDBConfig *AGDBConfigInit(AGDBConfig *obj, 
                                      char *dbname, AGDBConfigType type,
                                      AGBool sendRecordPlatformData,
                                      int32 platformDataLength, 
                                      void *platformData,
                                      AGArray *newids)
{
    bzero(obj, sizeof(AGDBConfig));
    obj->type = type;
    obj->sendRecordPlatformData = sendRecordPlatformData;
    AGDBConfigSetDBName(obj, dbname);
    AGDBConfigSetPlatformData(obj, platformDataLength, platformData);
    AGDBConfigSetNewIds(obj, newids);
    if (NULL == obj->newids)
        obj->newids = AGArrayNew(AGIntegerElements, 0);

    return obj;
}

ExportFunc void AGDBConfigFinalize(AGDBConfig *obj)
{
    if (obj->dbname)
        free(obj->dbname);
    if (obj->platformData)
        free(obj->platformData);
    if (obj->newids) 
        AGArrayFree(obj->newids);
    CHECKANDFREE(obj->reserved);
    bzero(obj, sizeof(*obj));
}

ExportFunc void AGDBConfigFree(AGDBConfig *obj)
{
    AGDBConfigFinalize(obj);
    free(obj);
}

ExportFunc AGDBConfig *AGDBConfigCopy(AGDBConfig *dst,
                                      AGDBConfig *src)
{
    void * tmp;

    if (NULL == dst || NULL == src)
        return NULL;

    AGDBConfigFinalize(dst);

    if (NULL != src->platformData) {
        tmp = malloc(src->platformDataLength);
        if (NULL != tmp)
            memcpy(tmp, src->platformData, src->platformDataLength);
    }
    else
        tmp = NULL;

    AGDBConfigInit(dst,
        strdup(src->dbname),
        src->type,
        src->sendRecordPlatformData,
        src->platformDataLength,
        tmp,
        dupNewIdArray(src->newids));

    dst->expansion1 = src->expansion1;
    dst->expansion2 = src->expansion2;
    dst->expansion3 = src->expansion3;
    dst->expansion4 = src->expansion4;

    dst->reservedLen = src->reservedLen;
    if (NULL != src->reserved) {
        dst->reserved = malloc(dst->reservedLen);
        memcpy(dst->reserved, src->reserved, dst->reservedLen);
    }

    return dst;

}

ExportFunc AGDBConfig *AGDBConfigDup(AGDBConfig *src)
{
    return AGDBConfigCopy(AGDBConfigNew(NULL,
                            AG_DONTSEND_CFG,
                            FALSE,
                            0,
                            NULL,
                            NULL),
                           src);
}

ExportFunc void AGDBConfigSetDBName(AGDBConfig *obj, char *dbname)
{
    if (obj->dbname == dbname)
        return;

    if (obj->dbname != NULL)
        free(obj->dbname);

    obj->dbname = dbname;
}

ExportFunc void AGDBConfigSetPlatformData(AGDBConfig *obj, 
                                          int32 platformDataLength, 
                                          void *platformData)
{
    obj->platformDataLength = platformDataLength;
    if (obj->platformData == platformData)
        return;

    CHECKANDFREE(obj->platformData);

    obj->platformData = platformData;
}

ExportFunc void AGDBConfigSetNewIds(AGDBConfig *obj, AGArray *newids)
{
    //PENDING(klobad) This scares me, but I'm in Rome.
    if (obj->newids == newids)
        return;

    if(obj->newids)
        AGArrayFree(obj->newids);

    obj->newids = newids;
}

#define agSIGNATURE_HIGH      (0xD5)
#define agSIGNATURE_LOW       (0xAA)
#define agVERSION_MAJ_0       (0)
#define agVERSION_MIN_0       (0)
#define agCURRENT_MAJ_VER     agVERSION_MAJ_0
#define agCURRENT_MIN_VER     agVERSION_MIN_0

ExportFunc int32 AGDBConfigReadData(AGDBConfig *obj, AGReader *r)
{
    int32 i, count;
    int32 majver, minver;

    if (AGReadInt16(r) != ((agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW))
        return AG_ERROR_INVALID_SIGNATURE;

    majver = AGReadCompactInt(r);
    minver = AGReadCompactInt(r);

    CHECKANDFREE(obj->dbname);
    obj->dbname = AGReadCString(r);
    obj->type = (AGDBConfigType)AGReadCompactInt(r);
    obj->sendRecordPlatformData = AGReadBoolean(r);
    obj->platformDataLength = AGReadCompactInt(r);
    CHECKANDFREE(obj->platformData);
    obj->platformData = malloc(obj->platformDataLength);
    AGReadBytes(r, obj->platformData, obj->platformDataLength);
    count = AGReadCompactInt(r);
    AGArrayRemoveAll(obj->newids);
    for(i = 0; i < count; i++) 
        AGArrayAppend(obj->newids, (void *)AGReadInt32(r));

    obj->expansion1 = AGReadCompactInt(r);
    obj->expansion2 = AGReadCompactInt(r);
    obj->expansion3 = AGReadCompactInt(r);
    obj->expansion4 = AGReadCompactInt(r);

    obj->reservedLen = AGReadCompactInt(r);
    CHECKANDFREE(obj->reserved);
    if (obj->reservedLen > 0) {
        obj->reserved = malloc(obj->reservedLen);
        AGReadBytes(r, obj->reserved, obj->reservedLen);
    }

    if (majver > agCURRENT_MAJ_VER)
        return AG_ERROR_UNKNOWN_VERSION;

    return AG_ERROR_NONE;
}

ExportFunc void AGDBConfigWriteData(AGDBConfig *obj, AGWriter *w)
{
    int32 i, count;

    AGWriteInt16(w, (agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW);
    AGWriteCompactInt(w, agCURRENT_MAJ_VER);
    AGWriteCompactInt(w, agCURRENT_MIN_VER);
    AGWriteCString(w, obj->dbname);
    AGWriteCompactInt(w, obj->type);
    AGWriteBoolean(w, obj->sendRecordPlatformData);
    AGWriteCompactInt(w, obj->platformDataLength);
    AGWriteBytes(w, obj->platformData, obj->platformDataLength);
    if(!obj->newids || AGArrayCount(obj->newids) < 1) {
        AGWriteCompactInt(w, 0);
    } else {
        count = AGArrayCount(obj->newids);
        AGWriteCompactInt(w, count);
        for(i = 0; i < count; i++) 
            AGWriteInt32(w, (uint32)AGArrayElementAt(obj->newids, i));
    }

    AGWriteCompactInt(w, obj->expansion1);
    AGWriteCompactInt(w, obj->expansion2);
    AGWriteCompactInt(w, obj->expansion3);
    AGWriteCompactInt(w, obj->expansion4);

    AGWriteCompactInt(w, obj->reservedLen);
    if (obj->reservedLen > 0)
        AGWriteBytes(w, obj->reserved, obj->reservedLen);
}

#ifndef REMOVE_SYNCHRONIZE_FEATURE
AGDBConfig * AGDBConfigSynchronize(AGDBConfig *agreed,
                                   AGDBConfig *device,
                                   AGDBConfig *desktop)
{
    AGDBConfig * obj;
    
    obj = AGDBConfigNew(NULL, AG_DONTSEND_CFG, FALSE, 0, NULL, NULL);

    if (NULL != obj) {

        CHECKANDFREE(obj->dbname);
        obj->dbname = AGSynchronizeString(agreed->dbname,
            device->dbname,
            desktop->dbname);

        obj->type = (AGDBConfigType)AGSynchronizeInt32((int32)agreed->type,
            (int32)device->type,
            (int32)desktop->type);

        obj->sendRecordPlatformData =
            AGSynchronizeBoolean(agreed->sendRecordPlatformData,
                device->sendRecordPlatformData,
                desktop->sendRecordPlatformData);

        CHECKANDFREE(obj->platformData);
        AGSynchronizeData(&obj->platformData,
            &obj->platformDataLength,
            agreed->platformData,
            agreed->platformDataLength,
            device->platformData,
            device->platformDataLength,
            desktop->platformData,
            desktop->platformDataLength);

        AGArrayRemoveAll(obj->newids);
        AGDBConfigSetNewIds(obj, dupNewIdArray(device->newids));

        CHECKANDFREE(obj->reserved);
        obj->reservedLen = 0;
        AGSynchronizeData(&obj->reserved,
            &obj->reservedLen,
            agreed->reserved,
            agreed->reservedLen,
            device->reserved,
            device->reservedLen,
            desktop->reserved,
            desktop->reservedLen);

    }
    
    return obj;
}
#endif /*#ifndef REMOVE_SYNCHRONIZE_FEATURE*/

ExportFunc void AGDBConfigAppendNewId(AGDBConfig *obj,
                                      uint32 tmpUID,
                                      uint32 newUID)
{
    AGArrayAppend(obj->newids, (void *)tmpUID);
    AGArrayAppend(obj->newids, (void *)newUID);
}

static AGArray *dupNewIdArray(AGArray *src)
{
    AGArray *newArray;

    if(src) {
        newArray = AGArrayNew(AGIntegerElements, AGArrayCount(src));
        AGArrayAppendArray(newArray, src);
    } else
        newArray = NULL;
    return newArray;
}