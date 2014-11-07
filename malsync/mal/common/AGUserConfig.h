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

/* Owner:  miket */

#ifndef __AGUSERCONFIG_H__
#define __AGUSERCONFIG_H__

#include <AGArray.h>
#include <AGServerConfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct AGUserConfig {

    AGBool dirty;
    int32 nextUID;
    AGArray * servers;
    AGArray * uidDeletes;

    int32 expansion1;
    int32 expansion2;
    int32 expansion3;
    int32 expansion4;

    int32 reservedLen;
    void * reserved;

} AGUserConfig;

typedef struct {
    int32 count, addCount;
} AGUserConfigEnumerateState;

AGUserConfig *  AGUserConfigNew();
void            AGUserConfigInit(AGUserConfig * uc);
void            AGUserConfigFree(AGUserConfig * uc);
void            AGUserConfigFinalize(AGUserConfig * uc);
AGUserConfig *  AGUserConfigCopy(AGUserConfig *dst, AGUserConfig *src);
AGUserConfig *  AGUserConfigDup(AGUserConfig *src);

int32               AGUserConfigCount(AGUserConfig * uc);
AGServerConfig *    AGUserConfigGetServer(AGUserConfig * uc, int32 uid);
AGServerConfig *    AGUserConfigGetServerByIndex(AGUserConfig * uc, int32 i);
AGServerConfig *    AGUserConfigEnumerate(AGUserConfig * uc,
                                          AGUserConfigEnumerateState ** state);

void    AGUserConfigAddServer(AGUserConfig * uc,
                              AGServerConfig * sc,
                              AGBool device);
void    AGUserConfigRemoveServer(AGUserConfig * uc, int32 uid);

int32   AGUserConfigReadData(AGUserConfig * uc, AGReader *r);
void    AGUserConfigWriteData(AGUserConfig * uc, AGWriter *w);
#ifndef REMOVE_SYNCHRONIZE_FEATURE
AGUserConfig * AGUserConfigSynchronize(AGUserConfig *agreed,
                                       AGUserConfig *device,
                                       AGUserConfig *desktop,
                                       AGBool preferDesktop);
#endif /* #ifndef REMOVE_SYNCHRONIZE_FEATURE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* of #ifndef __AGUSERCONFIG_H__ */
