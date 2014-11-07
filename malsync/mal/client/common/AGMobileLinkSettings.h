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

#ifdef _WIN32 /**** Entire file is NOOP if not on Windows. *****/

#ifndef __AGMOBILE_LINK_H__
#define __AGMOBILE_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

#define kMALConfig "Software\\Mobile Application Link"
#define kMALConfig_Use_Wizards "MobileLink Preferences"
#define kMALConfig_Current_Device_Type "Current Device Type"
#define kMALConfig_Current_Palm_Device_Name "Current Palm Device Name"
#define kMALConfig_Current_Palm_Device_Path "Current Palm Device Path"
#define kMALConfig_Current_CE_Device_Name "Current CE Device Name"
#define kMALConfig_Current_CE_Device_Path "Current CE Device Path"
#define kMALConfig_Security_DLL_Location "Security DLL"

typedef struct {
    AGBool useWizards;
} AGMobileLink;

typedef enum {
    AG_NO_DEVICE_TYPE = 0,
    AG_PALM_DEVICE_TYPE,
    AG_CE_DEVICE_TYPE
} AGDeviceType;

typedef struct {
    char * deviceName;
    AGDeviceType deviceType;
    char * prefsPath;
} AGDeviceEntry;

ExportFunc AGMobileLink * AGMobileLinkNew(void);
ExportFunc AGMobileLink * AGMobileLinkInit(AGMobileLink * mobileLink);
ExportFunc AGMobileLink *
AGMobileLinkInitWithDefaults(AGMobileLink * mobileLink);
ExportFunc void AGMobileLinkFinalize(AGMobileLink * mobileLink);
ExportFunc void AGMobileLinkFree(AGMobileLink * mobileLink);
ExportFunc AGMobileLink * AGMobileLinkNewAndReadFromRegistry(void);
ExportFunc void AGMobileLinkWriteToRegistry(AGMobileLink * mobileLink);
ExportFunc AGBool AGMobileLinkGetUseWizards(void);
ExportFunc void AGMobileLinkSetUseWizards(AGBool use);
ExportFunc AGDeviceEntry * AGMobileLinkGetCurrentDevice(void);
ExportFunc void AGMobileLinkSetCurrentDevice(AGDeviceEntry * deviceEntry);
ExportFunc AGDeviceEntry * AGMobileLinkDeviceEntryNew(char * deviceName,
                                           AGDeviceType deviceType,
                                           char * prefsPath);
ExportFunc void AGMobileLinkDeviceEntryFree(AGDeviceEntry * dle);
char *AGMobileLinkGetSecurityDLLLocation(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __AGMOBILE_LINK_H__ */

#endif /* _WIN32 */