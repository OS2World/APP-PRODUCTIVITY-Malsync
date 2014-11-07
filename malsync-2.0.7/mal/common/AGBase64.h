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

#ifndef __AGBASE64_H__
#define __AGBASE64_H__

#include <AGTypes.h>

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

ExportFunc char *AGBase64Encode(uint8 *ptr, int32 len);
/* If you pass in zero for len, strlen will be used to deterine
   buffer length.
   Caller is responsible freeing returned string
*/

ExportFunc uint8 *AGBase64Decode(char *ptr, int32 *len);
/* Input must be null terminated.
   Caller is responsible freeing returned string.
   Length of buffer is in len variable.
*/

#ifdef _WIN32
ExportFunc void AGBase64EncodedBufferFree(uint8 *buf);
#else
#define AGBase64EncodedBufferFree free
#endif
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
