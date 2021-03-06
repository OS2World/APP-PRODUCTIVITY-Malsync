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

#ifndef __AGDIGEST_H__
#define __AGDIGEST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

ExportFunc void AGDigest(char *username, uint8 pass[16], uint8 nonce[16], 
                         uint8 digest[16]);
ExportFunc AGBool AGDigestCompare(uint8 a[16], uint8 b[16]);
ExportFunc AGBool AGDigestNull(uint8 a[16]);
ExportFunc void AGDigestSetToNull(uint8 a[16]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
