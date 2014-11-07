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

#ifndef __AGLOCATIONCONFIG_H__
#define __AGLOCATIONCONFIG_H__

#include <AGTypes.h>
#include <AGReader.h>
#include <AGWriter.h>
#include <AGArray.h>

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

typedef enum {
    AG_PROXY_INCOMPLETE = 0,
    AG_PROXY_FAILED,
    AG_PROXY_USER_SUPPLIED,
    AG_PROXY_MSIE,
    AG_PROXY_Netscape,
    AG_PROXY_Opera       /* pending(miket): not currently supported */
} AGLocationConfigSource;

typedef enum {
    AG_PROXY_NO_ERROR = 0,
    AG_PROXY_NULL_POINTER,
    AG_PROXY_NONE_FOUND
} AGLocationConfigResult;

typedef struct AGLocationConfig {

    AGLocationConfigSource source;

    AGBool HTTPUseProxy;
    char * HTTPName;
    int32 HTTPPort;
    AGBool HTTPUseAuthentication;
    char * HTTPUsername;
    char * HTTPPassword;

    AGBool SOCKSUseProxy;
    char * SOCKSName;
    int32 SOCKSPort;

    char * SecureName;
    int32 SecurePort;

    char * autoConfigProxyURL;

    AGArray *exclusionServers;
    AGBool bypassLocal;

    AGBool proxy401;

    int32 expansion1;
    int32 expansion2;
    int32 expansion3;
    int32 expansion4;

    int32 reservedLen;
    void * reserved;

} AGLocationConfig;

ExportFunc AGLocationConfig * AGLocationConfigNew(void);
ExportFunc void AGLocationConfigInit(AGLocationConfig * obj);
ExportFunc void AGLocationConfigFree(AGLocationConfig * obj);
ExportFunc void AGLocationConfigFinalize(AGLocationConfig * obj);
ExportFunc AGLocationConfig * AGLocationConfigCopy(AGLocationConfig * dst,
                                                   AGLocationConfig * src);
ExportFunc AGLocationConfig * AGLocationConfigDup(AGLocationConfig * src);

ExportFunc int32 AGLocationConfigReadData(AGLocationConfig * obj, AGReader * r);
ExportFunc void AGLocationConfigWriteData(AGLocationConfig * obj, AGWriter * w);
ExportFunc AGArray * AGFillExclusionArray(char * list);
ExportFunc char * AGDescribeExclusionArray(AGArray * array);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __AGLOCATIONCONFIG_H__ */
