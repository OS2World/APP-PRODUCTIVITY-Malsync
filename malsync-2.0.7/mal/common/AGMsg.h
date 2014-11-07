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

#ifndef __AGMSG_H__
#define __AGMSG_H__

#include <AGTypes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define AGMSGStartingStringId            5450
#define AGMSGRetryingStringId            5451
#define AGMSGLookupStringId              5452
#define AGMSGLookupFailedStringId        5453
#define AGMSGConnectingStringId          5454
#define AGMSGConnectingFailedStringId    5455
#define AGMSGCancellingStringId          5456
#define AGMSGDisconnectingId             5457
#define AGMSGConnectionClosedStringId    5458
#define AGMSGUnknownFailureStringId      5459
#define AGMSGSendingChangedStringId      5460
#define AGMSGSendingFailedStringId       5461
#define AGMSGWaitingForServerStringId    5462
#define AGMSGReadingFailedStringId       5463
#define AGMSGHostLookupFailedStringId    5470
#define AGMSGConnectFailedStringId       5471
#define AGMSGProxyConnectFailedStringId  5472
#define AGMSGSocksConnectFailedStringId  5473
#define AGMSGSocksConnectErrorStringId   5474
#define AGMSGSocksIdErrorStringId        5475
#define AGMSGSocksErrorStringId          5476
#define AGMSGProxyDNSErrorStringId       5477
#define AGMSGSocksDNSErrorStringId       5478
#define AGMSGBadProxyAuthStringId        5479
#define AGMSGIncompatibleVersionStringId 5480
#define AGMSGInvalidMagicStringId        5481
#define AGMSGServerReadingFailedStringId 5482
#define AGMSGServerInvalidMagicStringId  5483
#define AGMSG401StringId                 5484

ExportFunc char *AGGetMsg(uint32 msgId);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __AGMSG_H__ */

