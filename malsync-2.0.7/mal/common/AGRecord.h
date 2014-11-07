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

#ifndef __AGRECORD_H__
#define __AGRECORD_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGReader.h>
#include <AGWriter.h>

typedef enum {
    AG_RECORD_UNMODIFIED = 0,
    AG_RECORD_UPDATED,
    AG_RECORD_NEW,
    AG_RECORD_DELETED,
    AG_RECORD_NEW_TEMPORARY_UID
} AGRecordStatus;


typedef struct AGRecord {
    int32 uid;
    AGRecordStatus status;

    int32 recordDataLength;
    void *recordData;

    int32 platformDataLength;
    void *platformData;
} AGRecord;


ExportFunc AGRecord *AGRecordNew(int32 uid, AGRecordStatus status, 
                                 int32 recordDataLength, void *recordData, 
                                 int32 platformDataLength, void *platformData);

ExportFunc AGRecord *AGRecordInit(AGRecord *record, int32 uid, 
                                  AGRecordStatus status, 
                                  int32 recordDataLength, void *recordData, 
                                  int32 platformDataLength, void *platformData);

ExportFunc void AGRecordFinalize(AGRecord *record);
ExportFunc void AGRecordFree(AGRecord *record);

ExportFunc void AGRecordSetData(AGRecord *record, int32 recordDataLength, 
                                void *recordData);

ExportFunc void AGRecordSetPlatformData(AGRecord *record, 
                                        int32 platformDataLength, 
                                        void *platformData);

ExportFunc void AGRecordReadData(AGRecord *record, AGReader *r);
ExportFunc void AGRecordWriteData(AGRecord *record, AGWriter *w);

ExportFunc AGBool AGRecordIsNew(AGRecord *record);
ExportFunc AGBool AGRecordIsDeleted(AGRecord *record);
ExportFunc AGBool AGRecordIsUpdated(AGRecord *record);

// new OR deleted OR updated
ExportFunc AGBool AGRecordIsModified(AGRecord *record);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGRECORD_H__
