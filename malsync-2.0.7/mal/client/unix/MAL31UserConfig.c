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

#include <MAL31UserConfig.h>
#include <MAL31ServerConfig.h>
#include <AGServerConfig.h>
#include <AGUtil.h>

typedef struct MAL31UserConfig {

    AGBool dirty;
    int32 nextUID;
    AGBool resetCookiesAtNextSync;
    AGArray *servers;
    AGArray *serversToAdd;
    AGArray *serversToDelete;

} MAL31UserConfig;

/* Version 0 - original creation */
#define RECORD_VERSION_0 (0)

/* Version 1 - reset cookies at next sync */
#define RECORD_VERSION_1 (1)

/* This is the version number that is written to new records */
#define CURRENT_RECORD_VERSION (RECORD_VERSION_1)


/*--------------------------------------------------------------------------*/
static void
MAL31UserConfigInit(MAL31UserConfig *userConfig)
{
    bzero(userConfig, sizeof(MAL31UserConfig));
    if (NULL != userConfig) {
        userConfig->nextUID = 1;
        userConfig->servers = AGArrayNew(AGUnownedPointerElements, 1);
        userConfig->serversToAdd = NULL;
        userConfig->serversToDelete = NULL;
        userConfig->dirty = TRUE;
    }
}
/*--------------------------------------------------------------------------*/
static void
finalizeServerGroup(AGArray * array)
{
    if (NULL != array) {
        int i, n;
        n = AGArrayCount(array);
        for (i = 0; i < n; ++i) {
            AGServerConfig * sc;
            sc = (AGServerConfig*)AGArrayElementAt(array, i);
            AGServerConfigFree(sc);
        }
        AGArrayFree(array);
    }
}
/*--------------------------------------------------------------------------*/
static void
MAL31UserConfigFinalize(MAL31UserConfig * userConfig)
{
    if (NULL != userConfig) {
        finalizeServerGroup(userConfig->servers);
        finalizeServerGroup(userConfig->serversToAdd);
        finalizeServerGroup(userConfig->serversToDelete);
        bzero(userConfig, sizeof(MAL31UserConfig));
    }
}
/*--------------------------------------------------------------------------*/
static void
MAL31UserConfigFree(MAL31UserConfig * userConfig)
{
    if (NULL != userConfig) {
        MAL31UserConfigFinalize(userConfig);
        free(userConfig);
        userConfig = NULL;
    }
}
/*--------------------------------------------------------------------------*/
static AGServerConfig *
getServerByIndex(AGArray * array, int32 index)
{
    int32 n;
    n = AGArrayCount(array);
    return (index < 0 || index >= n)
        ? NULL
        : (AGServerConfig *)AGArrayElementAt(array, index);
}
/*--------------------------------------------------------------------------*/
AGServerConfig *
MAL31UserConfigGetServerByIndex(MAL31UserConfig * userConfig,
                                int32 index)
{
    return getServerByIndex(userConfig->servers, index);
}
/*--------------------------------------------------------------------------*/
int32
MAL31UserConfigCount(MAL31UserConfig * userConfig)
{
    if (NULL != userConfig)
        return AGArrayCount(userConfig->servers);
    else
        return 0;
}
/*--------------------------------------------------------------------------*/
static void
readServerGroup(AGArray **array, AGReader *r)
{
    int32 i, n;
    n = AGReadCompactInt(r);
    *array = AGArrayNew(AGUnownedPointerElements, 1);
    for (i = 0; i < n; ++i) {
        AGServerConfig *sc = AGServerConfigNew();
        MAL31ServerConfigReadData(sc, r);
        AGArrayAppend(*array, sc);
    }
}
/*--------------------------------------------------------------------------*/
static void
MAL31UserConfigReadData(MAL31UserConfig *userConfig, AGReader *r)
{
    int16 version;
    version = AGReadCompactInt(r);
    userConfig->nextUID = AGReadCompactInt(r);
    readServerGroup(&userConfig->servers, r);
    readServerGroup(&userConfig->serversToAdd, r);
    readServerGroup(&userConfig->serversToDelete, r);
    userConfig->dirty = FALSE;
    if (RECORD_VERSION_0 == version)
        userConfig->resetCookiesAtNextSync = FALSE;
    else
        userConfig->resetCookiesAtNextSync = AGReadBoolean(r);
    if (RECORD_VERSION_1 == version)
        return;
}
/*--------------------------------------------------------------------------*/
static MAL31UserConfig *
MAL31UserConfigNewAndReadData(AGReader *r)
{
    MAL31UserConfig * result;

    result = (MAL31UserConfig *)malloc(sizeof(MAL31UserConfig));

    bzero(result, sizeof(MAL31UserConfig));
    MAL31UserConfigReadData(result, r);
    return result;
}
/*--------------------------------------------------------------------------*/
static void
writeServerGroup(AGArray * array, AGWriter * w)
{
    int32 i, n;
    
    n = AGArrayCount(array);
    AGWriteCompactInt(w, n);
    for (i = 0; i < n; ++i) {
        MAL31ServerConfigWriteData((AGServerConfig *)
                                   AGArrayElementAt(array, i), w);
    }
}
/*--------------------------------------------------------------------------*/
static void
MAL31UserConfigWriteData(MAL31UserConfig *userConfig, AGWriter *w)
{
    AGWriteCompactInt(w, CURRENT_RECORD_VERSION);
    AGWriteCompactInt(w, userConfig->nextUID);
    writeServerGroup(userConfig->servers, w);
    writeServerGroup(userConfig->serversToAdd, w);
    writeServerGroup(userConfig->serversToDelete, w);
    AGWriteBoolean(w, userConfig->resetCookiesAtNextSync);
    userConfig->dirty = FALSE;
}
/*---------------------------------------------------------------------------*/
void
MAL31ReadUserData(AGUserConfig *uc, AGReader *r)
{
    /* Read in the 3.1 data from the pilot */
    MAL31UserConfig *uc31 = MAL31UserConfigNewAndReadData(r);

    /* Copy the stuff we need from the 3.1 structure to the 3.2 */
    uc->dirty = uc31->dirty;
    uc->nextUID = uc31->nextUID;

    /* Set the server array from the 3.1 structure into the 3.2 */
    if (uc->servers)
        AGArrayFree(uc->servers);
    uc->servers = uc31->servers;
    uc31->servers = NULL;

    /* Free the 3.1 memory */
    MAL31UserConfigFree(uc31);

}
/*---------------------------------------------------------------------------*/
void
MAL31WriteUserData(AGUserConfig *uc, AGWriter *w)
{
    MAL31UserConfig uc31;

    /* Init a 3.1 User config structure */
    MAL31UserConfigInit(&uc31);

    /* Copy the stuff we need from the 3.2 structure to the 3.1 */
    uc31.dirty   = uc->dirty;
    uc31.nextUID = uc->nextUID;

    /* Set the 3.1 server group from the 3.2 structure */
    if (uc31.servers)
        AGArrayFree(uc31.servers);
    uc31.servers = uc->servers;
    uc->servers = NULL;

    /* Write the data */
    MAL31UserConfigWriteData(&uc31, w);

    /* Free up the 3.1 server */
    MAL31UserConfigFinalize(&uc31);

}

