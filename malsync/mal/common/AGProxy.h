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

#ifndef __AGPROXY_H__
#define __AGPROXY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <AGTypes.h>
#include <AGArray.h>

ExportFunc char *AGSocksBufCreate(unsigned long laddr, short _port, int *buflen);
ExportFunc char *AGProxyCreateAuthHeader(char *user, char *pass, AGBool dobasic);
ExportFunc int AGSocksGetResponse(char *buffer);
ExportFunc AGBool AGProxyCheckExclusionArray(AGArray *array, char *addrString);

#ifdef __cplusplus
}
#endif


#endif /* __AGPROXY_H__ */
