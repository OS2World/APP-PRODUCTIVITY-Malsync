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

#ifndef __AGUTILPALMOS_H__
#define __AGUTILPALMOS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

#ifdef __palmos__

#include <Pilot.h>

#include <sys_socket.h>
#include <sys_types.h>
#include <unix_string.h>
#include <unix_stdio.h>
#include <unix_stdlib.h>
/* (adam) We don't include unix_stdarg.h, because unix_string.h includes stdarg.h (which gets
 * pulled in from the Metrowerks Standard Library, at least when building with CodeWarrior) and the
 * definitions in unix_stdarg.h conflict with the ones in stdarg.h.  I'm not sure why
 * unix_string.h includes stdarg.h and not unix_stdarg.h, especially since unix_string.h comes
 * from the Palm OS 3.0 SDK, which itself contains unix_stdarg.h but not stdarg.h.
 */

/* According to the ANSI C standard, free() is supposed to take no action when passed a null pointer
 * (see Harbison and Steele, 16.2) but unfortunately unix_stdlib.h maps free to MemPtrFree(), which
 * dies when it's passed a NULL.  We use a do...while(0) in the macro below to avoid evaluating
 * the argument twice.
 */
#undef free
#define free(p) do { void *_p = (p); if (_p) MemPtrFree(_p); } while (0)

ExportFunc char *AGStrDup(char *str);
#define strdup(str) AGStrDup((str))

#define strncasecmp(s1, s2, len) StrNCaselessCompare((const char *)s1, (const char *)s2, len)
#define strcasecmp(s1, s2) StrCaselessCompare((const char *)s1, (const char *)s2)
#define stricmp(a, b) StrCaselessCompare((a), (b))
#define strncmp(a, b, n) StrNCompare((a), (b), (n))
#define itoa(i, s, radix) StrIToA((s), (i))

ExportFunc void AGFreeFunc(void *ptr);
/* realloc definition in palm\unix_stdlib.h needs to be fixed. */
#ifdef realloc
#   undef realloc
#endif /* realloc */
ExportFunc void *AGRealloc(void *p, int32 s);
#define realloc(p, s) AGRealloc((p), (s))

#ifdef calloc
#   undef calloc
#endif /* calloc */
ExportFunc void *AGCalloc(size_t num, size_t size);
#define calloc(n, s) AGCalloc((n), (s))

/* Character classification functions.
 * The Palm header file CharAttr.h defines macros such as IsDigit()
 * and IsAlpha(), but the Palm macros take two arguments, the first of
 * which is an array "attr"; it's not clear what value the caller should
 * provide for "attr", and the Palm macros are not mentioned in the Palm
 * documentation.  So we define our own macros instead.
 *
 * PENDING(adam) Some of the macros below, in particular isalnum(), will
 * expand to a fair amount of code.  We should use a function instead.
 */
#define isascii(c) (((c) >= 0) && ((c) <= 0x7f))
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define islower(c) ((c) >= 'a' && (c) <= 'z')
#define isspace(c) ((((c) >= 0x09) && ((c) <= 0x0d)) || ((c) == 0x20))
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')

#define isalpha(c) (islower(c) || isupper(c))
#define isalnum(c) (isalpha(c) || isdigit(c))

/*  The memcpy defined by palm would have the side effect of mallocing 
    twice and leaking in this case: bar = memcpy(malloc(5), foo, 5)
*/
#ifdef memcpy
#   undef memcpy
#endif /* memcpy */
#define memcpy(x,y,z) AGMemCopy(x,y,z)
ExportFunc void *AGMemCopy(void *dst, const void *src, size_t len);

/* memmove is defined as AGMemCopy, since AGMemCopy handles overlaps */
#define memmove(x,y,z) AGMemCopy(x,y,z)

#ifdef NDEBUG
#define assert(x)
#else
#define assert(x) if (!x) DbgBreak();
#endif /* NDEBUG */

/* We define offsetof here since it isn't defined in any Palm headers
 * (although it is defined in the MSL header file stddef.h). */
#define offsetof(s,m) ((size_t)&(((s*)0)->m))

ExportFunc char *AGStrRChr(const char* str, int8 chr);
#define strrchr(s, c) AGStrRChr(s, c)

#endif /* __palmos__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGUTILPALMOS_H__ */






