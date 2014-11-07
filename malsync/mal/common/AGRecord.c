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

#include <AGRecord.h>
#include <AGTypes.h>
#include <AGUtil.h>

ExportFunc AGRecord *AGRecordNew(int32 uid, AGRecordStatus status, 
                                 int32 recordDataLength, void *recordData, 
                                 int32 platformDataLength, void *platformData)
{
    AGRecord *record;

    record = (AGRecord *)malloc(sizeof(AGRecord));
    return AGRecordInit(record, uid, status, recordDataLength, 
        recordData, platformDataLength, platformData);
}

ExportFunc AGRecord *AGRecordInit(AGRecord *record, int32 uid, 
                                  AGRecordStatus status, 
                                  int32 recordDataLength, void *recordData, 
                                  int32 platformDataLength, void *platformData)
{
    bzero(record, sizeof(*record));
    record->uid = uid;
    record->status = status;
    AGRecordSetData(record, recordDataLength, recordData);
    AGRecordSetPlatformData(record, platformDataLength, platformData);

    return record;
}

ExportFunc void AGRecordFinalize(AGRecord *record)
{
    if (record->recordData)
        free(record->recordData);
    if (record->platformData)
        free(record->platformData);
    bzero(record, sizeof(*record));
}

ExportFunc void AGRecordFree(AGRecord *record)
{
    AGRecordFinalize(record);
    free(record);
}

ExportFunc void AGRecordSetData(AGRecord *record, int32 recordDataLength, 
                                void *recordData)
{
    record->recordDataLength = recordDataLength;

    if (record->recordData == recordData) {
        return;
    }

    if (record->recordData != NULL) {
        free(record->recordData);
    }

    record->recordData = recordData;
}

ExportFunc void AGRecordSetPlatformData(AGRecord *record, 
                                        int32 platformDataLength, 
                                        void *platformData)
{
    record->platformDataLength = platformDataLength;

    if (record->platformData == platformData) {
        return;
    }

    if (record->platformData != NULL) {
        free(record->platformData);
    }

    record->platformData = platformData;
}

ExportFunc AGBool AGRecordIsNew(AGRecord *record)
{
    return record->status == AG_RECORD_NEW;
}

ExportFunc AGBool AGRecordIsDeleted(AGRecord *record)
{
    return record->status == AG_RECORD_DELETED;
}

ExportFunc AGBool AGRecordIsUpdated(AGRecord *record)
{
    return record->status == AG_RECORD_UPDATED;
}

ExportFunc AGBool AGRecordIsModified(AGRecord *record)
{
    return record->status != AG_RECORD_UNMODIFIED;
}

ExportFunc void AGRecordReadData(AGRecord *record, AGReader *r)
{
    record->uid     = AGReadCompactInt(r);
    record->status  = (AGRecordStatus)AGReadCompactInt(r);
    record->recordDataLength = AGReadCompactInt(r);
    if (record->recordDataLength > 0) {
        if (NULL != record->recordData)
            free(record->recordData);
        record->recordData = malloc(record->recordDataLength);
        AGReadBytes(r, record->recordData, record->recordDataLength);
    }
    record->platformDataLength = AGReadCompactInt(r);
    if (record->platformDataLength > 0) {
        if (NULL != record->platformData)
            free(record->platformData);
        record->platformData = malloc(record->platformDataLength);
        AGReadBytes(r, record->platformData, record->platformDataLength);
    }
}

ExportFunc void AGRecordWriteData(AGRecord *record, AGWriter *w)
{
    AGWriteCompactInt(w, record->uid);
    AGWriteCompactInt(w, record->status);
    AGWriteCompactInt(w, record->recordDataLength);
    if (record->recordDataLength > 0)
        AGWriteBytes(w, record->recordData, record->recordDataLength);
    AGWriteCompactInt(w, record->platformDataLength);
    if (record->platformDataLength > 0)
        AGWriteBytes(w, record->platformData, record->platformDataLength);
}
