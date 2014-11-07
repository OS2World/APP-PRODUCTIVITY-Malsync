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

#include <AGPalmProtocol.h>

//PENDING(klobad) see the pendings in the .h

ExportFunc void AGPalmWriteDBConfigPlatformData(AGWriter *w, 
                                    uint32 creator, uint32 type, uint32 flags)
{
    AGWriteCompactInt((AGWriter *)w, creator);
    AGWriteCompactInt((AGWriter *)w, type);
    AGWriteCompactInt((AGWriter *)w, flags);
}

ExportFunc void AGPalmReadDBConfigPlatformData(AGReader *r, 
                                    uint32 *creator, uint32 *type, uint32 *flags)
{
    *creator = AGReadCompactInt(r);
    *type = AGReadCompactInt(r);
    *flags = AGReadCompactInt(r);
}

ExportFunc void AGPalmWriteRecordPlatformData(AGWriter *w, int16 recordIndex)
{
    AGWriteInt16(w, recordIndex);
}

ExportFunc void AGPalmReadRecordPlatformData(AGReader *r, int16 *recordIndex)
{
    *recordIndex = AGReadInt16(r);
}

ExportFunc void AGPalmWriteDeviceInfoPlatformData(AGWriter *w)
{

}

ExportFunc void AGPalmReadDeviceInfoPlatformData(AGReader *r)
{

}

ExportFunc uint8 AGPalmMALModToPilotAttribs(AGRecordStatus mod)
{
    uint8 ret = 0;

	// These values are found from [palm includes]\System\DataMgr.h
    ret |= (mod == AG_RECORD_UNMODIFIED)    ? 0x00 : 0; //duh
    ret |= (mod == AG_RECORD_UPDATED)       ? 0x40 : 0;
    ret |= (mod == AG_RECORD_NEW)           ? 0x40 : 0;
    ret |= (mod == AG_RECORD_DELETED)       ? 0x80 : 0;
    
    return ret;    
}

ExportFunc AGRecordStatus AGPalmPilotAttribsToMALMod(uint8 attrib)
{
    if (attrib & 0x80)
        return AG_RECORD_DELETED;
    if (attrib & 0x40)
        return AG_RECORD_UPDATED;

    return AG_RECORD_UNMODIFIED;
}

