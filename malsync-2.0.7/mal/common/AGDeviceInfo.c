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

#include <AGDeviceInfo.h>
#include <AGUtil.h>

ExportFunc AGDeviceInfo *AGDeviceInfoNew()
{
    AGDeviceInfo *deviceInfo;

    deviceInfo = (AGDeviceInfo *)malloc(sizeof(AGDeviceInfo));
    AGDeviceInfoInit(deviceInfo);
    return deviceInfo;
}

ExportFunc AGDeviceInfo *AGDeviceInfoInit(AGDeviceInfo *deviceInfo)
{
    bzero(deviceInfo, sizeof(*deviceInfo));
    return deviceInfo;
}

ExportFunc void AGDeviceInfoFinalize(AGDeviceInfo *deviceInfo)
{
    if (deviceInfo->osName)
        free(deviceInfo->osName);
    if (deviceInfo->osVersion)
        free(deviceInfo->osVersion);
    if (deviceInfo->serialNumber)
        free(deviceInfo->serialNumber);
    if (deviceInfo->language)
        free(deviceInfo->language);
    if (deviceInfo->charset)
        free(deviceInfo->charset);
    if (deviceInfo->platformData)
        free(deviceInfo->platformData);

    bzero(deviceInfo, sizeof(*deviceInfo));
}

ExportFunc void AGDeviceInfoFree(AGDeviceInfo *deviceInfo)
{
    AGDeviceInfoFinalize(deviceInfo);
    free(deviceInfo);
}

ExportFunc void AGDeviceInfoReadData(AGDeviceInfo *deviceInfo, AGReader *r)
{
    int32 platformDataLength;
    void *platformData = NULL;

    deviceInfo->availableBytes = AGReadInt32(r);
    deviceInfo->screenWidth = AGReadInt32(r);
    deviceInfo->screenHeight = AGReadInt32(r);
    deviceInfo->colorDepth = AGReadInt32(r);
    platformDataLength = AGReadInt32(r);
    if (platformDataLength > 0) {
        platformData = malloc(platformDataLength);
        AGReadBytes(r, platformData, platformDataLength);
    }
    AGDeviceInfoSetPlatformData(deviceInfo, platformDataLength, platformData);
    AGDeviceInfoSetOSName(deviceInfo, AGReadCString(r));
    AGDeviceInfoSetOSVersion(deviceInfo, AGReadCString(r));
    AGDeviceInfoSetLanguage(deviceInfo, AGReadCString(r));
    AGDeviceInfoSetCharSet(deviceInfo, AGReadCString(r));
    AGDeviceInfoSetSerialNumber(deviceInfo, AGReadCString(r));
}

ExportFunc void AGDeviceInfoWriteData(AGDeviceInfo *deviceInfo, AGWriter *w)
{
    AGWriteInt32(w, deviceInfo->availableBytes);
    AGWriteInt32(w, deviceInfo->screenWidth);
    AGWriteInt32(w, deviceInfo->screenHeight);
    AGWriteInt32(w, deviceInfo->colorDepth);
    AGWriteInt32(w, deviceInfo->platformDataLength);
    if (deviceInfo->platformDataLength > 0) {
        AGWriteBytes(w, deviceInfo->platformData, 
            deviceInfo->platformDataLength);
    }
    AGWriteCString(w, deviceInfo->osName);
    AGWriteCString(w, deviceInfo->osVersion);
    AGWriteCString(w, deviceInfo->language);
    AGWriteCString(w, deviceInfo->charset);
    AGWriteCString(w, deviceInfo->serialNumber);
}


void AGDeviceInfoSetOSName(AGDeviceInfo *deviceInfo, char *osname)
{
    if (deviceInfo->osName == osname) {
        return;
    }

    if (deviceInfo->osName != NULL) {
        free(deviceInfo->osName);
    }

    deviceInfo->osName = osname;
}

void AGDeviceInfoSetOSVersion(AGDeviceInfo *deviceInfo, char *osversion)
{
    if (deviceInfo->osVersion == osversion) {
        return;
    }

    if (deviceInfo->osVersion != NULL) {
        free(deviceInfo->osVersion);
    }

    deviceInfo->osVersion = osversion;
}

void AGDeviceInfoSetSerialNumber(AGDeviceInfo *deviceInfo, char *serialNumber)
{
    if (deviceInfo->serialNumber == serialNumber) {
        return;
    }

    if (deviceInfo->serialNumber != NULL) {
        free(deviceInfo->serialNumber);
    }

    deviceInfo->serialNumber = serialNumber;
}

void AGDeviceInfoSetLanguage(AGDeviceInfo *deviceInfo, char *language)
{
    if (deviceInfo->language == language) {
        return;
    }

    if (deviceInfo->language != NULL) {
        free(deviceInfo->language);
    }

    deviceInfo->language = language;
}

void AGDeviceInfoSetCharSet(AGDeviceInfo *deviceInfo, char *charset)
{
    if (deviceInfo->charset == charset) {
        return;
    }

    if (deviceInfo->charset != NULL) {
        free(deviceInfo->charset);
    }

    deviceInfo->charset = charset;
}

void AGDeviceInfoSetPlatformData(AGDeviceInfo *deviceInfo, 
                             int32 platformDataLength, void *platformData)
{
    deviceInfo->platformDataLength = platformDataLength;
    if (deviceInfo->platformData == platformData) {
        return;
    }

    if (deviceInfo->platformData != NULL) {
        free(deviceInfo->platformData);
    }

    deviceInfo->platformData = platformData;
}

