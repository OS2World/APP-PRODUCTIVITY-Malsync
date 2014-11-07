/*
 * The contents of this file are subject to the Mozilla Public License
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
 
#ifndef __AGBUFFEREDNET_H__
#define __AGBUFFEREDNET_H__

#include <AGNet.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

AGSocket *AGBufNetSocketNew(AGNetCtx *ctx);
int32 AGBufNetSend(AGNetCtx *ctx, AGSocket *soc, const uint8 *data, int32 bytes,
                 AGBool block);
int32 AGBufNetRead(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 bytes,
                 AGBool block);
int32 AGBufNetReadProtected(AGNetCtx *ctx, AGSocket *soc, uint8 *buffer, int32 offset, int32 bytes,
                 AGBool block);
int32 AGBufNetGets(AGNetCtx *ctx, AGSocket *soc, uint8 *buf, int32 offset,
                int32 bytes, int32 *bytesread, AGBool block);
sword AGBufNetSocketClose(AGNetCtx *ctx, AGSocket *soc);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGBUFFEREDNET_H__
