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

#include <MAL31ServerConfig.h>
#include <AGBufferReader.h>
#include <AGBufferWriter.h>
#include <AGUtil.h>
#include <AGDigest.h>
#include <AGBase64.h>
#include <AGMD5.h>
#include <MAL31DBConfig.h>

/* Version 0 - original creation */
#define RECORD_VERSION_0 (0)

/* Version 1 - added allowSecureConnection */
#define RECORD_VERSION_1 (1)

/* This is the version number that is written to new records */
#define CURRENT_RECORD_VERSION (RECORD_VERSION_1)

/*--------------------------------------------------------------------------*/
void
MAL31ServerConfigReadData(AGServerConfig *config, AGReader *r)
{
    int32 count, i;
    int16 recordVersion;
    AGDBConfig *dbconfig;

    recordVersion = AGReadInt16(r);
    config->uid = AGReadInt32(r);
    config->status = (AGRecordStatus)AGReadInt16(r);
    config->serverName = AGReadCString(r);
    config->serverPort = AGReadInt16(r);

    config->userName = AGReadCString(r);
    config->cleartextPassword = AGReadCString(r);
    if (AGReadInt8(r))
        AGReadBytes(r, config->password, 16);

    /* read in nonce */
    if (AGReadInt8(r))
        AGReadBytes(r, config->nonce, 16);

    if (AGReadInt8(r))
        config->disabled = TRUE;
    else
        config->disabled = FALSE;

    config->friendlyName = AGReadCString(r);
    config->userUrl = AGReadCString(r);
    config->description = AGReadCString(r);
    config->serverUri = AGReadCString(r);

    config->sequenceCookieLength = AGReadInt32(r);
    if (config->sequenceCookieLength > 0) {
        config->sequenceCookie =
            (uint8*)malloc(config->sequenceCookieLength);
        AGReadBytes(r, config->sequenceCookie, config->sequenceCookieLength);
    }

    count = AGReadInt32(r);
    config->dbconfigs = AGArrayNew(AGUnownedPointerElements, count);
    for (i = 0; i < count; i++) {
        dbconfig = AGDBConfigNew(NULL, AG_SENDALL_CFG, FALSE, 0, NULL, NULL);
        MAL31DBConfigReadData(dbconfig, r);
        AGArrayAppend(config->dbconfigs, dbconfig);
    }
    if (AGReadInt8(r))
        config->sendDeviceInfo = TRUE;
    else
        config->sendDeviceInfo = FALSE;

    config->hashPassword = AGReadBoolean(r);
    /* pending(miket):  leave in until cleartext password support is in. */
    config->hashPassword = TRUE;
    config->connectTimeout = AGReadCompactInt(r);
    config->writeTimeout = AGReadCompactInt(r);
    config->readTimeout = AGReadCompactInt(r);

    config->connectSecurely = AGReadBoolean(r);

    if (recordVersion >= RECORD_VERSION_1) {
        /* Record version 1:  added allowSecureConnection. */
        config->allowSecureConnection = AGReadBoolean(r);
    } else {
        config->allowSecureConnection = FALSE;
    }
}

/*---------------------------------------------------------------------------*/
static AGBool
digestIsNull(uint8 a[16])
{
    int i;
    for(i=0;i<16;i++)
        if(a[i])
            return 0;
    return 1;
}
/*---------------------------------------------------------------------------*/
void
MAL31ServerConfigWriteData(AGServerConfig *config, AGWriter *w)
{
    int32 i, count;
    AGDBConfig *dbconfig;
    
    AGWriteInt16(w, CURRENT_RECORD_VERSION);
    AGWriteInt32(w, config->uid);
    AGWriteInt16(w, (uint16)config->status);
    AGWriteCString(w, config->serverName);
    AGWriteInt16(w, config->serverPort);

    AGWriteCString(w, config->userName);
    AGWriteCString(w, config->cleartextPassword);

    if (digestIsNull(config->password))
        AGWriteInt8(w, 0);
    else {
        AGWriteInt8(w, 16);
        AGWriteBytes(w, config->password, 16);
    }

    if (digestIsNull(config->nonce))
        AGWriteInt8(w, 0);
    else {
        AGWriteInt8(w, 16);
        AGWriteBytes(w, config->nonce, 16);
    }

    if (config->disabled)
        AGWriteInt8(w, 1);
    else
        AGWriteInt8(w, 0);

    AGWriteCString(w, config->friendlyName);
    AGWriteCString(w, config->userUrl);
    AGWriteCString(w, config->description);
    AGWriteCString(w, config->serverUri);

    AGWriteInt32(w, config->sequenceCookieLength);
    if (config->sequenceCookieLength > 0) {
        AGWriteBytes(w, config->sequenceCookie, 
                     config->sequenceCookieLength);
    }

    count = AGArrayCount(config->dbconfigs);
    AGWriteInt32(w, count);
    for (i = 0; i < count; i++) {
        dbconfig = (AGDBConfig*)AGArrayElementAt(config->dbconfigs, i);
        MAL31DBConfigWriteData(dbconfig, w);
    }
    if (config->sendDeviceInfo)
        AGWriteInt8(w, 1);
    else
        AGWriteInt8(w, 0);

    AGWriteBoolean(w, config->hashPassword);
    AGWriteCompactInt(w, config->connectTimeout);
    AGWriteCompactInt(w, config->writeTimeout);
    AGWriteCompactInt(w, config->readTimeout);

    AGWriteBoolean(w, config->connectSecurely);
    AGWriteBoolean(w, config->allowSecureConnection);

}



