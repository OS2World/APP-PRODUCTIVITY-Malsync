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

#ifndef __AGCEPROTOCOL_H__
#define __AGCEPROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
   
#include <AGTypes.h>
#include <AGWriter.h>
#include <AGReader.h>
    
ExportFunc void AGCEWriteDeviceInfoPlatformData(AGWriter *w, 
                                    char *processor);
ExportFunc void AGCEReadDeviceInfoPlatformData(AGReader *r, 
                                    char **processor);

#define CE_EXPN_RSRC_FILE_EXECUTE_BIT  0x02

ExportFunc void AGCEWriteEXPN_RSRC_FILE_TYPE(AGWriter *w, uint8 flags, 
                                             char *remoteFilename);
ExportFunc void AGCEReadEXPN_RSRC_FILE_TYPE(AGReader *r, uint8 *flags, 
                                            char **remoteFilename);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* #ifndef __AGCEPROTOCOL_H__ */
