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

#ifndef __AGUTILMAC_H__
#define __AGUTILMAC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (defined(macintosh) && !defined(__palmos__))
#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   define snprintf _snprintf
#   define strncasecmp(s1, s2, len) strnicmp(s1, s2, len)
#   define strcasecmp(s1, s2) stricmp(s1, s2)
#   define bzero(dst, len) memset((dst), (0), (len))
#   define bcopy(src, dst, len) memmove((dst), (src), (len))
#   define AGSleepMillis(x)  /*Sleep((x))*/
#   define AGFreeFunc free
#   define isascii(c) (((c) >= 0) && ((c) <= 0x7f))
#   define isalphanum(c) (((c) >= 'a' ) && ((c) <= 'z'))\
                    || (((c) >= 'A' ) && ((c) <= 'Z'))\
                    || (((c) >= '0' ) && ((c) <= '9'))
#   define isspace(c) ((((c) >= 0x09) && ((c) <= 0x0d)) || ((c) == 0x20))
#   define hexDigit(n) ((n)<10 ? '0'+(n) : 'A'+(n)-10)
#   define htons(s) (s)
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
ExportFunc int gettimeofday(struct timeval *tp, struct timezone *tzp);
char * strdup(const char *str);
#endif /* #if (defined(macintosh) && !defined(__palmos__)) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __AGUTILMAC_H__ */