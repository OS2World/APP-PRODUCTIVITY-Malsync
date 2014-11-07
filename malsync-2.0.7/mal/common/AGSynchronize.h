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

/*  AGSynchronize.h
    Copyright 1997, 1998, 1999 AvantGo, Inc.  All rights reserved.
    Owner:  miket
 */

#ifndef __AGSYNCHRONIZE_H__
#define __AGSYNCHRONIZE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

/*
    These functions take an "agreed" value or structure, a "dominant" value
    or structure, and a "recessive" value or structure.  The agreed item
    represents the last known synchronized state between the dominant and
    recessive items.  The synchronize functions returns the agreed item if
    nothing has changed since the last synchronization (i.e., all three items
    match in value), or whichever item has been modified since the last
    synchronization (i.e., whichever of the two differs from the agreed item).
    In case both the dominant and recessive items have been modified (i.e.,
    they both differ from the agreed item), the dominant wins and its value
    is returned.

*/

int8 AGSynchronizeInt8(int8 a, int8 d, int8 r);
int16 AGSynchronizeInt16(int16 a, int16 d, int16 r);
int32 AGSynchronizeInt32(int32 a, int32 d, int32 r);
AGBool AGSynchronizeBoolean(AGBool a, AGBool d, AGBool r);
char * AGSynchronizeString(char * a, char * d, char * r);
void AGSynchronizeData(void ** s, int32 * slen,
                       void * a, int32 alen,
                       void * d, int32 dlen,
                       void * r, int32 rlen);
void AGSynchronizeStackStruct(void * s,
                              void * a, 
                              void * d,
                              void * r,
                              int32 len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGSYNCHRONIZE_H__ */
