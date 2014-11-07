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

#include <AGCEProtocol.h>

#define CE_DEVICE_INFO_PLATFORM_DATA_VERSION 0

ExportFunc void AGCEWriteDeviceInfoPlatformData(AGWriter *w, 
                                    char *processor)
{
    AGWriteInt8(w, CE_DEVICE_INFO_PLATFORM_DATA_VERSION);
    AGWriteCString(w, processor);
}

ExportFunc void AGCEReadDeviceInfoPlatformData(AGReader *r, 
                                    char **processor)
{
    uint8 version;

    version = AGReadInt8(r);
    *processor = AGReadCString(r);
}

#define CE_EXPN_RSRC_FILE_TYPE_VERSION 0

ExportFunc void AGCEWriteEXPN_RSRC_FILE_TYPE(AGWriter *w, uint8 flags, 
                                             char *remoteFilename)
{
    AGWriteInt8(w, CE_EXPN_RSRC_FILE_TYPE_VERSION);
    AGWriteInt8(w, flags);
    AGWriteCString(w, remoteFilename);
}

ExportFunc void AGCEReadEXPN_RSRC_FILE_TYPE(AGReader *r, uint8 *flags, 
                                            char **remoteFilename)
{
    uint8 version;

    version = AGReadInt8(r);
    *flags = AGReadInt8(r);
    *remoteFilename = AGReadCString(r);
}
