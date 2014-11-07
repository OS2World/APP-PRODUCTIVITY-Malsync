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

#ifndef __AGRESOURCEMANAGER_H__
#define __AGRESOURCEMANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGServerConfig.h>
#include <AGLocationConfig.h>
#include <AGTypes.h>

/* ----------------------------------------------------------------------------
    Interface to discover information about devices.
---------------------------------------------------------------------------- */

/*  Scan known device profiles and create an internal representation of
    those devices.  All calls to AGResourceManager will refer to the
    device list created at the last time this function was called.
*/
ExportFunc void AGRefreshDeviceList(void);

/*  Identical to AGRefreshDeviceList() but populates internal representation
    with data for old (3.0/3.1) profiles.
*/
ExportFunc void AGRefreshDeviceListOld(void);

typedef AGDeviceEnumerator;

/*  Create a handle to a DeviceEnumerator. Creating the enumerator also
    creates an internal list of devices on the system at that moment.
    Call AGDeviceEnumeratorFree() when done with the enumerator.
*/
ExportFunc AGDeviceEnumerator * AGDeviceEnumeratorNew(void);

/*  Given a handle to a DeviceEnumerator, return a pointer to the next
    device identifier in this enumerator.  The caller obtains ownership
    of this identifier and must free it when done with it.

    If the function returns NULL, there are no more device identifiers in this
    enumerator.
*/
ExportFunc char * AGGetNextDevice(AGDeviceEnumerator * enumerator);

/*  Given a handle to a DeviceEnumerator, reset the enumerator's
    internal counter so that the next call to AGGetNextDevice() for this
    enumerator will return the first device in the enumerator.
    
    Note that AGDeviceEnumeratorReset() does not generate a new list of
    devices.  It only starts the internal counter over again.
*/
ExportFunc int32 AGDeviceEnumeratorReset(AGDeviceEnumerator * enumerator);

/*  Given a handle to a DeviceEnumerator, clean up the resources used by
    the enumerator.
*/
ExportFunc void AGDeviceEnumeratorFree(AGDeviceEnumerator * enumerator);

/* Used in type field of AGRMDeviceInfo and AGSetCurrentDevice. */
enum {
    AG_PALM = 0,
    AG_WINCE,
    AG_DESKTOP,
    AG_DEVICE_TYPE_NUM,
    AG_UNKNOWN_DEVICE_TYPE = 0x7ffffff
};

typedef struct {
    int32 type;           /* AG_PALM, AG_WINCE, AG_UNKNOWN_DEVICE_TYPE */
    char * szName;      /* Platform-determined identifier of device */
    int32 cbName;       /* size of array pointed to by szName */
} AGRMDeviceInfo;

/*  Given a device identifier and a pointer to a AGRMDeviceInfo structure,
    fill in the structure for that device.  Note that it is the caller's
    responsibility to allocate the array pointed to by AGRMDeviceInfo.szName
    and to indicate in cbName how large that array is.
*/
ExportFunc int32 AGGetDevice(char * key, AGRMDeviceInfo * info);

/*  Set the current device to the given device identifier.  No checks for
    proper syntax are made.  To maintain code compatibility, it is advised
    that the caller pass only device identifiers returned by AGGetNextDevice().

    The "current" device is simply an internal persistent state maintained
    as a way for different clients of MAL to communicate which device the
    user last worked with.
*/
ExportFunc int32 AGSetCurrentDevice(char * key);

/*  Set the current device to a particular device identifier based on the
    specified platform and the identifier that's presumed unique within that
    platform.
*/
ExportFunc int32 AGSetCurrentDeviceFromPlatform(int32 type, char * id);

/*  Given a pointer to an array and an int32 indicating how large that array
    is, fill the array with the a device identifier of the current device.
*/
ExportFunc int32 AGGetCurrentDevice(char * key, int32 cbKey);

/* ----------------------------------------------------------------------------
    Interface to manipulate serverConfigs.
---------------------------------------------------------------------------- */

/*  Shorthand for an otherwise long type. */
typedef AGServerConfig * LPAGSC;

/*  Given a valid serverConfig and a device identifier, add the serverConfig
    to the device's profile.  Insert a new uid for the serverConfig in the
    serverConfig structure.  If a desktop component is calling this function,
    fromDevice should be FALSE; device components should set it to TRUE. The
    difference is that uids are allocated from different spaces to help
    ensure that no uid will collide.

    The caller retains ownership of the given serverConfig.
*/
ExportFunc int32 AGAddServer(char * key,
                             AGServerConfig * server,
                             AGBool fromDevice);

/*  Given a uid and a deviceidentifier, remove the serverConfig having that
    uid in the device profile.
*/
ExportFunc int32 AGRemoveServer(char * key, int32 uid);

/*  For modFlags field of AGModifyServer:  Allocate an array of AGBool of
    size AGRM_SMOD_NUMFLAGS.  Initially set each element to FALSE.  For each
    field in the serverConfig that you wish to be archived, set the
    corresponding element in the array to TRUE.  For example, to change
    the user's password:

    AGBool * mod;
    mod = calloc(1, sizeof(AGBool) * AGRM_SMOD_NUMFLAGS);
    if (NULL == mod)
        choke_and_die();
    mod[AGRM_SMOD_PASSWORD] = TRUE;
    err = AGModifyServer(device, sc->uid, sc, AGRM_SMOD_NUMFLAGS, mod);
    free(mod);
    if (AG_ERROR_NONE != err)
        choke_and_die();
*/
enum {
    AGRM_SMOD_ALL = 0,  /* To write out every field, set this to TRUE. */
    AGRM_SMOD_UID,
    AGRM_SMOD_STATUS,
    AGRM_SMOD_SERVERNAME,
    AGRM_SMOD_SERVERPORT,
    AGRM_SMOD_USERNAME,
    AGRM_SMOD_CLEARTEXTPASSWORD,
    AGRM_SMOD_PASSWORD,
    AGRM_SMOD_DISABLED,
    AGRM_SMOD_RESETCOOKIE,
    AGRM_SMOD_NOTREMOVABLE,
    AGRM_SMOD_FRIENDLYNAME,
    AGRM_SMOD_SERVERTYPE,
    AGRM_SMOD_USERURL,
    AGRM_SMOD_DESCRIPTION,
    AGRM_SMOD_SERVERURI,
    AGRM_SMOD_SEQUENCECOOKIELENGTH,
    AGRM_SMOD_SEQUENCECOOKIE,
    AGRM_SMOD_DBCONFIGS,
    AGRM_SMOD_NONCE,
    AGRM_SMOD_SENDDEVICEINFO,
    AGRM_SMOD_HASHPASSWORD,
    AGRM_SMOD_CONNECTTIMEOUT,
    AGRM_SMOD_WRITETIMEOUT,
    AGRM_SMOD_READTIMEOUT,
    AGRM_SMOD_CONNECTSECURELY,
    AGRM_SMOD_ALLOWSECURECONNECTION,
    AGRM_SMOD_NUMFLAGS
};

/*  Given a device identifier, a uid, a serverConfig structure, and
    "modflags," modify the serverConfig having the uid in the given device
    profile according to the modflags.  Only the serverConfig fields
    indicated by the modFlags will be changed.  flagStructSize must always
    be set to AGRM_SMOD_NUMFLAGS; if not, AGModifyServer returns
    AG_ERROR_BAD_ARGUMENT.
*/
ExportFunc int32 AGModifyServer(char * key,
                                int32 uid,
                                AGServerConfig * server,
                                int32 flagStructSize,
                                AGBool * modFlags);

/*  Given a device identifier and a serverConfig structure, replace the
    serverConfig in the device profile having the same uid as the uid of 
    the given serverConfig with the current serverConfig.  A server having this
    uid must exist in the device profile already; otherwise, AGReplaceServer()
    returns AG_ERROR_NOT_FOUND.
*/
ExportFunc int32 AGReplaceServer(char * key, AGServerConfig * server);

/*  Given a uid and a device identifier, return a copy of the serverConfig
    having that uid in the device profile.  The caller obtains ownership of
    this serverConfig (i.e., it is the caller's responsibility to free it).
*/
ExportFunc AGServerConfig * AGGetServer(char * key, int32 uid);

typedef AGServerEnumerator;

/*  Given a device identifier, create a handle to a ServerEnumerator.
    Creating the enumerator also creates an internal list of servers in the
    given device's profile at that moment.

    Call AGServerEnumeratorFree() when done with the enumerator.
*/
ExportFunc AGServerEnumerator * AGServerEnumeratorNew(char * key);

/*  Given a handle to a ServerEnumerator, return the number of serverConfigs
    in the enumerator.
*/
ExportFunc int32 AGGetServerCount(AGServerEnumerator * enumerator);

/*  Given a handle to a ServerEnumerator, return a copy of the next
    serverConfig in this enumerator.  The caller obtains ownership of this
    serverConfig (i.e., it is the caller's responsibility to free it).

    If the function returns NULL, there are no more serverConfigs in this
    enumerator.
*/
ExportFunc AGServerConfig * AGGetNextServer(AGServerEnumerator * enumerator);

/*  Given a handle to a ServerEnumerator, return a copy of the nth
    serverConfig in this enumerator.  The caller obtains ownership of this
    serverConfig (i.e., it is the caller's responsibility to free it).

    Returns NULL if there's no such serverConfig.

    Beware of relying on indexes to identify particular servers.  It's possible
    on most platforms for the number of servers to change at any time. At that
    point the nth server on the system will not necessarily match the nth
    server in the enumerator.  It's better to iterate through the list of
    servers once using AGGetNextServer() and record serverConfig uids, and then
    refer to particular servers from then on by uid only.
*/
ExportFunc AGServerConfig * AGGetServerAt(AGServerEnumerator * enumerator,
                                          int32 n);

/*  Given a handle to a ServerEnumerator, reset the enumerator's
    internal counter so that the next call to AGGetNextServer() for this
    enumerator will return the first server in the enumerator.
    
    Note that AGServerEnumeratorReset() does not generate a new list of
    servers.  It only starts the internal counter over again.
*/
ExportFunc int32 AGServerEnumeratorReset(AGServerEnumerator * enumerator);

/*  Given a handle to a ServerEnumerator, clean up the resources used by
    by the enumerator.
*/
ExportFunc void AGServerEnumeratorFree(AGServerEnumerator * enumerator);

/*  Given a serialized stream of profile data, synchronize the current device's
    profile and the stream's profile, and return a new serialized stream
    containing the synchronized profile.

    pd is "prefer desktop."  Normally we resolve conflicts by preferring
    the device's version.  In some cases (after a conduit sync, for example)
    we know that the desktop has the better information, so then we'll
    choose the desktop's version.
*/
ExportFunc int32 AGSyncProfiles(uint8 * out,
                                uint8 ** in,
                                uint32 * inSize,
                                AGBool pd);

/*  The blob functions are used on devices during a synchronization, where
    the desktop merges its and the device's profiles and returns an opaque
    blob to the device.  Since the desktop is running the show during a 
    sync, the device doesn't treat the blob as profile data.  That's why
    it treats it as a blob.
*/

/*  Return a pointer to a blob of data that represents this device's profile
    data.
*/
ExportFunc int32 AGRetrieveProfileBlob(char * key,
                                       uint8 ** in,
                                       uint32 * inSize);

/*  Given a pointer to a blob of data that represents this device's profile
    data, store it.
*/
ExportFunc int32 AGStoreProfileBlob(char * key,
                                    uint8 * out,
                                    uint32 outSize);

/*  Given a uid referring to a single serverConfig, reset the sequence cookie
    for that server.  Sending in zero as the uid resets all serverConfigs'
    sequence cookies.
*/
ExportFunc int32 AGResetServerCookies(char * key, uint32 uid);

/*  Convience method that iterates over all server configs and
    marks all valid ones enabled.
*/
ExportFunc void AGEnableAllServers(char *key);

/*  Convience method that iterates over all server configs and
    marks all valid ones disabled.
*/
ExportFunc void AGDisableAllServers(char *key);

/* ----------------------------------------------------------------------------
    Interface to manipulate locationConfigs.
---------------------------------------------------------------------------- */

/*  Shorthand for an otherwise long type. */
typedef AGLocationConfig * LPAGLC;

/*  Return a pointer to the current locationConfig on this system.  The caller
    owns this pointer (i.e., it has the responsibility of freeing it).
*/
ExportFunc AGLocationConfig * AGGetLocationConfig(void);

enum {
    AGRM_LMOD_ALL = 0,  /* To write out every field, set this to TRUE. */
    AGRM_LMOD_USEPROXY,
    AGRM_LMOD_Y,
    AGRM_LMOD_NUMFLAGS
};

/*  Given a locationConfig structure and "modflags," modify the current
    system's locationConfig according to the modflags.  Only the fields
    indicated by the modFlags will be changed.  flagStructSize must always
    be set to AGRM_LMOD_NUMFLAGS; if not, AGModifyLocationConfig returns
    AG_ERROR_BAD_ARGUMENT.
*/
ExportFunc int32 AGModifyLocationConfig(AGLocationConfig * loc,
                                        int32 flagStructSize,
                                        AGBool * modFlags);

/*  Given a locationConfig structure, replace the current system's
    locationConfig with the given one.
*/
ExportFunc int32 AGReplaceLocationConfig(AGLocationConfig * loc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGRESOURCEMANAGER_H__ */
