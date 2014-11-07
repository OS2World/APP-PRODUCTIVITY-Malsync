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

#ifndef __AGPROTECTEDMEM_H__
#define __AGPROTECTEDMEM_H__

#include <AGTypes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if !((defined __palmos__) || (defined USE_CUSTOM_PROTECTED_MEMORY))

/* (adam) We sometimes need to include this header in source files which use their own
 * implementation of ANSI functions rather than the AGUtil implementation; see jspod.c.
 * So, as a hack, we avoid including AGUtil.h when NO_AG_UTIL is defined.
 */
#ifndef NO_AG_UTIL
#include "AGUtil.h"
#endif

// These do nothing off the palm platform
#define AGProtectedMemoryInit(void)
#define AGProtectedMemoryFinalize(void)

#define AGProtectedMalloc(len) (const void *)malloc((len))
#define AGProtectedRealloc(ptr, len)  (const void *)realloc((void *)(ptr), (len))
#define AGProtectedWrite(dst, offset, src, len) \
            (memmove((((uint8 *)dst)+offset), src, len), 0)
#define AGProtectedFree(ptr) free((void *)(ptr))
#define AGProtectedZero(ptr, offset, len) memset((((uint8 *)ptr)+offset), 0, (len))
#define AGProtectedStrDup(ptr) strdup(ptr)
#define AGProtectedMemCopy(dest, source, length) (memmove(dest, source, length), 0)

/* PROTECTED_NEW and PROTECTED_FREEZE are convenient for initializing a structure on the
 * stack in PalmOS and then copying it to high memory.
 */
#define PROTECTED_NEW(p, type) type *p = (type *) malloc(sizeof(type))
#define PROTECTED_FREEZE(p, type) p

#else

typedef struct AGProtectedHeader {
    uint32 magicCookie;
    uint32 recordId;
} AGProtectedHeader;

ExportFunc int16 AGProtectedMemoryInit(void);
ExportFunc void AGProtectedMemoryFinalize(void);

/* (adam) It's inconsistent tha AGProtectedMalloc() and AGProtectedRealloc() return
 * pointers of type (const void *), and yet AGProtectedWrite() and friends take a pointer
 * argument of type (void *).  We should change AGProtectedWrite(), AGProtectedFree() and
 * AGProtectedZero() to take pointers of type (const void *).  I don't want to make the change
 * right now because it might involve adding "const" at a lot of different places in the code,
 * but it should happen at some point.
 */

ExportFunc const void *AGProtectedMalloc(uint32 len);
ExportFunc const void *AGProtectedRealloc(void *ptr, uint32 len);

/* returns an error code from DmWrite(), or 0 on success */
ExportFunc uint16 AGProtectedWrite(void *dst, uint32 offset, 
                                    void *src, uint32 len);

ExportFunc void AGProtectedFree(void *ptr);
ExportFunc void AGProtectedZero(void *ptr, uint32 offset, uint32 len);
ExportFunc char *AGProtectedStrDup(char *str);
ExportFunc int16 AGProtectedMemCopy(void *dest, const void *src, uint32 len);

#define PROTECTED_NEW(p, type) type _local; type *p = &_local
#define PROTECTED_FREEZE(p, type) ( (type *) AGProtectedDup(p, sizeof(type)) )

#endif /* !__palmos__ */

ExportFunc const void *AGProtectedDup(void *ptr, uint32 nbytes);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __AGPROTECTEDMEM_H__ */

