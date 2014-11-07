/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

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

#include <MAL31DBConfig.h>
#ifdef DARWIN
#include <stdlib.h>
#else
#include <malloc.h>
#endif

/*---------------------------------------------------------------------------*/
void
MAL31DBConfigReadData(AGDBConfig *dbconfig, AGReader *r)
{
    char *dbname;
    int32 i, count;

    dbname = AGReadCString(r);
    dbconfig->type = (AGDBConfigType)AGReadCompactInt(r);
    AGDBConfigSetDBName(dbconfig, dbname);
    dbconfig->sendRecordPlatformData = AGReadBoolean(r);
    dbconfig->platformDataLength = AGReadCompactInt(r);
    dbconfig->platformData = malloc(dbconfig->platformDataLength);
    AGReadBytes(r, dbconfig->platformData, dbconfig->platformDataLength);
    count = AGReadCompactInt(r);
    if(count > 0) {
        dbconfig->newids = AGArrayNew(AGIntegerElements, count);
        for(i = 0; i < count; i++) 
            AGArrayAppend(dbconfig->newids, (void *)AGReadInt32(r));
    }
}

/*---------------------------------------------------------------------------*/
void
MAL31DBConfigWriteData(AGDBConfig *dbconfig, AGWriter *w)
{
    int32 i, count;

    AGWriteCString(w, dbconfig->dbname);
    AGWriteCompactInt(w, dbconfig->type);
    AGWriteBoolean(w, dbconfig->sendRecordPlatformData);
    AGWriteCompactInt(w, dbconfig->platformDataLength);
    AGWriteBytes(w, dbconfig->platformData, dbconfig->platformDataLength);
    if(!dbconfig->newids || AGArrayCount(dbconfig->newids) < 1) {
        AGWriteCompactInt(w, 0);
    } else {
        count = AGArrayCount(dbconfig->newids);
        AGWriteCompactInt(w, count);
        for(i = 0; i < count; i++) 
            AGWriteInt32(w, (uint32)AGArrayElementAt(dbconfig->newids, i));
    }
}
