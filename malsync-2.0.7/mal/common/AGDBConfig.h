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

#ifndef __AGDBCONFIG_H__
#define __AGDBCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>
#include <AGReader.h>
#include <AGWriter.h>
#include <AGArray.h>

/* The server can configure the client to automatically
 send all of the records of a database during sync,
 send only the modified records, or to not send
 anything from that database (typically used to remove
 a database from the client's send list)
*/
typedef enum {
    AG_SENDALL_CFG = 0,
    AG_SENDMODS_CFG,
    AG_DONTSEND_CFG
} AGDBConfigType;

typedef struct AGDBConfig {
    char *dbname;
    AGDBConfigType type;
    AGBool sendRecordPlatformData;
    int32 platformDataLength;
    void *platformData;
    AGArray *newids;
    int32 expansion1;
    int32 expansion2;
    int32 expansion3;
    int32 expansion4;
    int32 reservedLen;
    void * reserved;
} AGDBConfig;

ExportFunc AGDBConfig *AGDBConfigNew(char *dbname, AGDBConfigType type, 
                                     AGBool sendRecordPlatformData,
                                     int32 platformDataLength, 
                                     void *platformData,
                                     AGArray *newids);
ExportFunc AGDBConfig *AGDBConfigInit(AGDBConfig *obj, 
                                      char *dbname, AGDBConfigType type,
                                      AGBool sendRecordPlatformData,
                                      int32 platformDataLength, 
                                      void *platformData,
                                      AGArray *newids);

ExportFunc void AGDBConfigFinalize(AGDBConfig *obj);
ExportFunc void AGDBConfigFree(AGDBConfig *obj);

ExportFunc AGDBConfig *AGDBConfigCopy(AGDBConfig *dst, AGDBConfig *src);
ExportFunc AGDBConfig *AGDBConfigDup(AGDBConfig *src);

ExportFunc void AGDBConfigSetDBName(AGDBConfig *obj, char *dbname);
ExportFunc void AGDBConfigSetPlatformData(AGDBConfig *obj, 
                                          int32 platformDataLength, 
                                          void *platformData);

ExportFunc int32 AGDBConfigReadData(AGDBConfig *obj, AGReader *r);
ExportFunc void AGDBConfigWriteData(AGDBConfig *obj, AGWriter *w);

#ifndef REMOVE_SYNCHRONIZE_FEATURE
ExportFunc AGDBConfig * AGDBConfigSynchronize(AGDBConfig *agreed,
                                              AGDBConfig *device,
                                              AGDBConfig *desktop);
#endif
ExportFunc void AGDBConfigAppendNewId(AGDBConfig *obj,
                                      uint32 tmpUID,
                                      uint32 newUID);
ExportFunc void AGDBConfigSetNewIds(AGDBConfig *obj, AGArray *newids);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGDBCONFIG_H__ */
