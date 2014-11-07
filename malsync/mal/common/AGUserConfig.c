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

/* Owner:  miket */

#include <AGUserConfig.h>
#include <AGServerConfig.h>
#include <AGUtil.h>
#ifndef REMOVE_SYNCHRONIZE_FEATURE
#include <AGSynchronize.h>
#endif

#define AG_UC_DESKTOP_OFFSET (0x40000000)

typedef AGServerConfig * LPAGSC;

/* ----------------------------------------------------------------------------
*/
static int32 getNextUID(AGUserConfig * uc, AGBool device)
{
    if (device)
        return uc->nextUID++;
    else
        return AG_UC_DESKTOP_OFFSET + uc->nextUID++;
}

#ifdef DEBUG
/* ----------------------------------------------------------------------------
*/
static AGBool serverConfigUidIsUnique(AGArray * array, int32 uid)
{
    int32 i, n;
    n = AGArrayCount(array);
    for (i = 0; i < n; ++i) {
        if (((LPAGSC)AGArrayElementAt(array, i))->uid == uid)
            return FALSE;
    }
    return TRUE;
}
#endif

/* ----------------------------------------------------------------------------
*/
void AGUserConfigAddServer(AGUserConfig * uc, LPAGSC sc, AGBool device)
{
    if (0 == sc->uid)
        sc->uid = getNextUID(uc, device);
#ifdef DEBUG
#ifndef _WIN32_WCE
    assert(serverConfigUidIsUnique(uc->servers, sc->uid));
#endif
#endif
    AGArrayAppend(uc->servers, sc);
    uc->dirty = TRUE;
}

/* ----------------------------------------------------------------------------
*/
LPAGSC AGUserConfigGetServer(AGUserConfig * uc, int32 uid)
{
    LPAGSC result = NULL;

    int32 n = AGArrayCount(uc->servers);
    while (n--) {
        result = (LPAGSC)AGArrayElementAt(uc->servers, n);
        if (uid == result->uid)
            return result;
    }
    return NULL;
}

/* ----------------------------------------------------------------------------
*/
static void addToDeleteList(AGUserConfig * uc, int32 uid)
{
    AGArrayAppend(uc->uidDeletes, (void*)uid);
}

/* ----------------------------------------------------------------------------
*/
void AGUserConfigRemoveServer(AGUserConfig * uc, int32 uid)
{
    LPAGSC sc = AGUserConfigGetServer(uc, uid);
    if (NULL != sc) {
        AGArrayRemoveAt(uc->servers, AGArrayIndexOf(uc->servers, sc, 0));
        AGServerConfigFree(sc);
        if (uid < AG_UC_DESKTOP_OFFSET)
            addToDeleteList(uc, uid);
        uc->dirty = TRUE;
    }
}

#ifndef REMOVE_SYNCHRONIZE_FEATURE
/* ----------------------------------------------------------------------------
*/
static void syncExistingServers(AGUserConfig * result,
                                AGUserConfig * agreed,
                                AGUserConfig * device,
                                AGUserConfig * desktop,
                                AGBool preferDesktop)
{
    int n = AGUserConfigCount(device);
    while (n--) {
        LPAGSC sc1 = NULL, sc2 = NULL;
        sc1 = AGUserConfigGetServerByIndex(device, n);
        sc2 = AGUserConfigGetServer(desktop, sc1->uid);
        if (NULL != sc2) {
            LPAGSC scAgreed = NULL, scResult = NULL;
            if (NULL != agreed)
                scAgreed = AGUserConfigGetServer(agreed, sc1->uid);
            if (NULL != scAgreed)
                scResult = AGServerConfigSynchronize(scAgreed,
                    sc1,
                    sc2,
                    preferDesktop);
            else
                scResult = AGServerConfigDup(sc1);
            AGUserConfigAddServer(result, scResult, TRUE);
        }
    }
}

/* ----------------------------------------------------------------------------
*/
static void addNewServers(AGUserConfig * result,
                          AGUserConfig * first,
                          AGUserConfig * second)
{
    int n = AGUserConfigCount(first);
    while (n--) {
        LPAGSC sc = NULL;
        sc = AGUserConfigGetServerByIndex(first, n);
        if (NULL == AGUserConfigGetServer(second, sc->uid)) {
            LPAGSC tsc = AGServerConfigDup(sc);
            if (NULL != tsc) {
                if (tsc->uid >= AG_UC_DESKTOP_OFFSET)
                    tsc->uid = 0;
                AGUserConfigAddServer(result, tsc, TRUE);
            }
        }
    }
}

/* ----------------------------------------------------------------------------
*/
static void mergeUserConfigs(AGUserConfig * result,
                             AGUserConfig * agreed,
                             AGUserConfig * device,
                             AGUserConfig * desktop,
                             AGBool preferDesktop)
{
    syncExistingServers(result, agreed, device, desktop, preferDesktop);
    addNewServers(result, device, desktop);
    addNewServers(result, desktop, device);
}

/* ----------------------------------------------------------------------------
*/
static void deleteMarkedServerConfigs(AGUserConfig * uc,
                                      AGArray * list)
{
    int n = AGArrayCount(list);
    while (n--) {
        AGUserConfigRemoveServer(uc, (int32)AGArrayElementAt(list, n));
    }
}

/* ----------------------------------------------------------------------------
*/
static void handleDeletes(AGUserConfig * result,
                          AGUserConfig * device,
                          AGUserConfig * desktop)
{
    deleteMarkedServerConfigs(result, device->uidDeletes);
    deleteMarkedServerConfigs(result, desktop->uidDeletes);
}

/* ----------------------------------------------------------------------------
*/
static void resetDeleteList(AGUserConfig * uc)
{
    AGArrayRemoveAll(uc->uidDeletes);
}

/* ----------------------------------------------------------------------------
*/
static void convertTempUIDs(AGUserConfig * obj)
{
    int n;

    n = AGArrayCount(obj->servers);
    while (n--) {
        AGServerConfig * sc = AGUserConfigGetServerByIndex(obj, n);
        if (sc->uid >= AG_UC_DESKTOP_OFFSET)
            sc->uid -= AG_UC_DESKTOP_OFFSET;
    }
}

/* ----------------------------------------------------------------------------
*/
static void checkForCookieReset(AGUserConfig * obj)
{
    int n;

    n = AGArrayCount(obj->servers);
    while (n--) {
        AGServerConfig * sc = AGUserConfigGetServerByIndex(obj, n);
        if (sc->resetCookie) {
            AGServerConfigResetCookie(sc);
            AGServerConfigResetNonce(sc);
            sc->resetCookie = FALSE;
        }
    }
}

/* ----------------------------------------------------------------------------
*/
AGUserConfig * AGUserConfigSynchronize(AGUserConfig *agreed,
                                       AGUserConfig *device,
                                       AGUserConfig *desktop,
                                       AGBool preferDesktop)
{
    AGUserConfig * result = NULL;
    AGUserConfig * cw = NULL;

    cw = preferDesktop ? desktop : device;

    /* If we were handed nothing, return a new uc. */
    if (NULL == device && NULL == desktop)
        return AGUserConfigNew();

    /* If either one is empty, return the other one. */
    if (NULL == device)
        result = desktop;
    if (NULL == desktop)
        result = device;
    if (NULL != result) {
        result = AGUserConfigDup(result);
        if (NULL != result) {

            /*  We're just duplicating one of the two userConfigs
                because we don't have another to sync against.  That's
                fine, but we should also do whatever processing the
                sync would have done. pending(miket): this is poorly
                structured. All syncs should go through the same pipeline
                regardless of any special cases. So eliminate this special
                case and the following code.
            */
            convertTempUIDs(result);
            checkForCookieReset(result);
            resetDeleteList(result);
        }
        return result;
    }

    /* Have two UserConfigs, so we need to merge them.
    Begin with a brand-new UserConfig structure. */
    result = AGUserConfigNew();

    if (NULL == result)
        return NULL;

    /* Put in the easy fields. */
    result->dirty = FALSE;
    result->nextUID = (desktop->nextUID > device->nextUID)
        ? desktop->nextUID : device->nextUID;
    result->reservedLen = cw->reservedLen;
    AGSynchronizeData(&result->reserved,
        &result->reservedLen,
        agreed->reserved,
        agreed->reservedLen,
        device->reserved,
        device->reservedLen,
        desktop->reserved,
        desktop->reservedLen);

    /* Merge the two userConfigs. */
    mergeUserConfigs(result, agreed, device, desktop, preferDesktop);

    /* Delete the serverConfigs marked for deletion. */
    handleDeletes(result, device, desktop);

    return result;
}
#endif /* #ifndef REMOVE_SYNCHRONIZE_FEATURE */

/* ----------------------------------------------------------------------------
*/
int32 AGUserConfigCount(AGUserConfig * uc)
{
    if (NULL != uc)
        return AGArrayCount(uc->servers);
    else
        return 0;
}

/* ----------------------------------------------------------------------------
*/
static void finalizeServers(AGUserConfig * uc)
{
    int i, n;

    if (!uc->servers)
        return;
    
    /* Free the server list. */
    n = AGArrayCount(uc->servers);
    for (i = 0; i < n; ++i) {
        AGServerConfigFree((AGServerConfig*)
            AGArrayElementAt(uc->servers, i));
    }
    AGArrayRemoveAll(uc->servers);
}

/* ----------------------------------------------------------------------------
*/
AGUserConfig *AGUserConfigCopy(AGUserConfig *dst, AGUserConfig *src)
{
    int32 i, n;

    if (NULL == src || NULL == dst)
        return NULL;

    dst->dirty = src->dirty;
    dst->nextUID = src->nextUID;

    finalizeServers(dst);
    n = AGArrayCount(src->servers);
    for (i = 0; i < n; ++i)
        AGArrayAppend(dst->servers,
            AGServerConfigDup((LPAGSC)AGArrayElementAt(src->servers, i)));
    AGArrayRemoveAll(dst->uidDeletes);
    n = AGArrayCount(src->uidDeletes);
    for (i = 0; i < n; ++i)
        AGArrayAppend(dst->uidDeletes, AGArrayElementAt(src->uidDeletes, i));

    dst->expansion1 = src->expansion1;
    dst->expansion2 = src->expansion2;
    dst->expansion3 = src->expansion3;
    dst->expansion4 = src->expansion4;

    dst->reservedLen = src->reservedLen;
    CHECKANDFREE(dst->reserved);
    if (NULL != src->reserved) {
        dst->reserved = malloc(dst->reservedLen);
        memcpy(dst->reserved, src->reserved, dst->reservedLen);
    }

    return dst;
}

/* ----------------------------------------------------------------------------
*/
AGUserConfig *AGUserConfigDup(AGUserConfig *src)
{
    return AGUserConfigCopy(AGUserConfigNew(), src);
}

/* ----------------------------------------------------------------------------
*/
LPAGSC AGUserConfigGetServerByIndex(AGUserConfig * uc,
                                    int32 index)
{
    int32 n;
    n = AGArrayCount(uc->servers);
    return (index < 0 || index >= n)
        ? NULL
        : (LPAGSC)AGArrayElementAt(uc->servers, index);
}

/* ----------------------------------------------------------------------------
*/
void AGUserConfigInit(AGUserConfig * uc)
{
    bzero(uc, sizeof(AGUserConfig));
    uc->nextUID = 1;
    uc->dirty = TRUE;
    uc->servers = AGArrayNew(AGUnownedPointerElements, 0);
    uc->uidDeletes = AGArrayNew(AGIntegerElements, 0);
    uc->reservedLen = 0;
    uc->reserved = NULL;
}

/* ----------------------------------------------------------------------------
*/
AGUserConfig * AGUserConfigNew()
{
    AGUserConfig * result = (AGUserConfig *)malloc(sizeof(AGUserConfig));
    if (NULL != result)
        AGUserConfigInit(result);
    return result;
}

/* ----------------------------------------------------------------------------
*/
void AGUserConfigFinalize(AGUserConfig * uc)
{
    /* Free the server list. */
    finalizeServers(uc);
    if (uc->servers)
        AGArrayFree(uc->servers);

    /* Free the delete list. */
    AGArrayFree(uc->uidDeletes);

    /* Free the future stuff. */
    CHECKANDFREE(uc->reserved);

    /* Clear out everything. */
    bzero(uc, sizeof(AGUserConfig));
}

/* ----------------------------------------------------------------------------
*/
void AGUserConfigFree(AGUserConfig * uc)
{
    AGUserConfigFinalize(uc);
    free(uc);
}

#define agSIGNATURE_HIGH      (0xDE)
#define agSIGNATURE_LOW       (0xAA)
#define agVERSION_MAJ_0       (0)
#define agVERSION_MIN_0       (0)
#define agCURRENT_MAJ_VER     agVERSION_MAJ_0
#define agCURRENT_MIN_VER     agVERSION_MIN_0

/* Define future flag constants here. For example:

    #define agFLAG_SAY_HELLO (0x00000001)
*/

/* ----------------------------------------------------------------------------
*/
int32 AGUserConfigReadData(AGUserConfig * obj, AGReader *r)
{
    int32 flags;
    int32 i, n;
    int32 majver, minver;

    if (AGReadInt16(r) != ((agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW))
        return AG_ERROR_INVALID_SIGNATURE;

    majver = AGReadCompactInt(r);
    minver = AGReadCompactInt(r);

    /* Read UID. */
    obj->nextUID = AGReadCompactInt(r);

    /*  Read bit flags. Note that we don't currently do anything
        with these flags.*/
    flags = AGReadCompactInt(r);

    /* Read uid deletes. */
    AGArrayRemoveAll(obj->uidDeletes);
    n = AGReadCompactInt(r);
    for (i = 0; i < n; ++i)
        AGArrayAppend(obj->uidDeletes, (void*)AGReadCompactInt(r));

    /* Read servers. */
    finalizeServers(obj);
    n = AGReadCompactInt(r);
    for (i = 0; i < n; ++i) {
        AGServerConfig * sc;
        sc = AGServerConfigNew();
        if (NULL == sc)
            return AG_ERROR_OUT_OF_MEMORY;
        AGServerConfigReadData(sc, r);
        AGArrayAppend(obj->servers, sc);
    }

    obj->dirty = FALSE;

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

void AGUserConfigWriteData(AGUserConfig * obj, AGWriter *w)
{
    int32 flags;
    int32 i, n;

    AGWriteInt16(w, (agSIGNATURE_HIGH << 8) | agSIGNATURE_LOW);
    AGWriteCompactInt(w, agCURRENT_MAJ_VER);
    AGWriteCompactInt(w, agCURRENT_MIN_VER);
    AGWriteCompactInt(w, obj->nextUID);

    /*  Future use: set bit flags here.
    */
    flags = 0;
    AGWriteCompactInt(w, flags);
    n = AGArrayCount(obj->uidDeletes);
    AGWriteCompactInt(w, n);
    for (i = 0; i < n; ++i) {
        AGWriteCompactInt(w,
            (uint32)AGArrayElementAt(obj->uidDeletes, i));
    }
    n = AGArrayCount(obj->servers);
    AGWriteCompactInt(w, n);
    for (i = 0; i < n; ++i) {
        AGServerConfigWriteData(
            (AGServerConfig*)AGArrayElementAt(obj->servers, i), w);
    }
    obj->dirty = FALSE;

    AGWriteCompactInt(w, obj->expansion1);
    AGWriteCompactInt(w, obj->expansion2);
    AGWriteCompactInt(w, obj->expansion3);
    AGWriteCompactInt(w, obj->expansion4);

    AGWriteCompactInt(w, obj->reservedLen);
    if (obj->reservedLen > 0)
        AGWriteBytes(w, obj->reserved, obj->reservedLen);
}
