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

#ifndef __AGDEVICEINFO_H__
#define __AGDEVICEINFO_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGReader.h>
#include <AGWriter.h>

/*  Note:  This data structure has been published to the world.
    If anything in it changes, it must remain compatible with previous
    versions.
*/
typedef struct AGDeviceInfo {

    char *osName;
    char *osVersion;
    char *serialNumber;
    int32 availableBytes;

    int32 screenWidth;
    int32 screenHeight;
    int32 colorDepth;

    char *language;
    char *charset;
    int32 platformDataLength;
    void *platformData;
} AGDeviceInfo;

ExportFunc AGDeviceInfo *AGDeviceInfoNew();
ExportFunc AGDeviceInfo *AGDeviceInfoInit(AGDeviceInfo *deviceInfo);
ExportFunc void AGDeviceInfoFinalize(AGDeviceInfo *deviceInfo);
ExportFunc void AGDeviceInfoFree(AGDeviceInfo *deviceInfo);
ExportFunc void AGDeviceInfoReadData(AGDeviceInfo *deviceInfo, AGReader *r);
ExportFunc void AGDeviceInfoWriteData(AGDeviceInfo *deviceInfo, AGWriter *w);

ExportFunc void AGDeviceInfoSetOSName(AGDeviceInfo *deviceInfo, char *osname);
ExportFunc void AGDeviceInfoSetOSVersion(AGDeviceInfo *deviceInfo, 
                                         char *osversion);
ExportFunc void AGDeviceInfoSetSerialNumber(AGDeviceInfo *deviceInfo, 
                                            char *serialNumber);
ExportFunc void AGDeviceInfoSetLanguage(AGDeviceInfo *deviceInfo, 
                                        char *language);
ExportFunc void AGDeviceInfoSetCharSet(AGDeviceInfo *deviceInfo, 
                                       char *charset);
ExportFunc void AGDeviceInfoSetPlatformData(AGDeviceInfo *deviceInfo, 
                                            int32 platformDataLength, 
                                            void *platformData);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGDEVICEINFO_H__
