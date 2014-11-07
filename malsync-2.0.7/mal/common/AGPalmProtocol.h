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

#ifndef __AGPALMPROTOCOL_H__
#define __AGPALMPROTOCOL_H__

#include <AGTypes.h>
#include <AGWriter.h>
#include <AGReader.h>
#include <AGRecord.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//PENDING(klobad) this is probably going to be more like a function table
// where we have an extra void* and no specific arguements

ExportFunc void AGPalmWriteDBConfigPlatformData(AGWriter *w, 
                                    uint32 creator, uint32 type, uint32 flags);
ExportFunc void AGPalmReadDBConfigPlatformData(AGReader *r, 
                                    uint32 *creator, uint32 *type, uint32 *flags);

//PENDING(klobad) what attribs are needed here?
ExportFunc void AGPalmWriteRecordPlatformData(AGWriter *w, int16 recordIndex);
ExportFunc void AGPalmReadRecordPlatformData(AGReader *r, int16 *recordIndex);

//PENDING(klobad) what attribs are needed here?
ExportFunc void AGPalmWriteDeviceInfoPlatformData(AGWriter *w);
ExportFunc void AGPalmReadDeviceInfoPlatformData(AGReader *r);

/* ----------------------------------------------------------------------------
    ExportFunc uint8 AGPalmMALModToPilotAttribs(AGRecordStatus mod)

    Given a MAL-style AGRecordStatus value, map as best as possible to
    PalmPilot status bits.

    -> AGRecordStatus mod: See definition.

    <- BYTE: Pilot-style m_Attribs value.

*/
ExportFunc uint8 AGPalmMALModToPilotAttribs(AGRecordStatus mod);

/* ----------------------------------------------------------------------------
    ExportFunc AGRecordStatus AGPalmPilotAttribsToMALMod(uint8 attrib)

    Given a PalmPilot m_Attribs byte, map as best as possible to MAL
    AGRecordStatus value.

    -> uint8 attrib: Pilot-style m_Attribs byte.

    <- AGRecordStatus: See definition.

*/
ExportFunc AGRecordStatus AGPalmPilotAttribsToMALMod(uint8 attrib);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __AGPALMPROTOCOL_H__ */

