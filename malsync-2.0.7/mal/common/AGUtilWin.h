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

#ifndef __AGUTILWIN_H__
#define __AGUTILWIN_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>
#include <windows.h>

#if _WIN32_WCE >= 211
#include <windef.h>
#endif

#ifdef _WIN32
#   include <stdlib.h>
#   include <string.h>
#   include <stdarg.h>
    /*  Common functions missing from windows. */
#   define snprintf _snprintf
#   define strncasecmp(s1, s2, len) strnicmp(s1, s2, len)
#   define strcasecmp(s1, s2) stricmp(s1, s2)
#   define bzero(dst, len) memset((dst), (0), (len))
#   define bcopy(src, dst, len) memmove((dst), (src), (len))
#   define AGSleepMillis(x)  Sleep((x))
#   define AGFreeFunc free
    ExportFunc int32 index(const char *str, int8 c);
    ExportFunc int32 rindex(const char *str, int8 c);
    ExportFunc TCHAR *AGTStrDup(char *str);
    ExportFunc char *AGCStrDup(TCHAR *str);
#   ifdef _WIN32_WCE
/* map character classification tests to wide versions; this works because
 * every ASCII character can be used as a Unicode character */
#       ifndef isalnum
#       define isalnum(c) iswalnum((wint_t) (c))
#       endif
#       ifndef isalpha
#       define isalpha(c) iswalpha((wint_t) (c))
#       endif
#       ifndef isascii
#       define isascii(c) iswascii((wint_t) (c))
#       endif
#       ifndef iscntrl
#       define iscntrl(c) iswcntrl((wint_t) (c))
#       endif
#       ifndef isdigit
#       define isdigit(c) iswdigit((wint_t) (c))
#       endif
#       ifndef isgraph
#       define isgraph(c) iswgraph((wint_t) (c))
#       endif
#       ifndef islower
#       define islower(c) iswlower((wint_t) (c))
#       endif
#       ifndef isprint
#       define isprint(c) iswprint((wint_t) (c))
#       endif
#       ifndef ispunct
#       define ispunct(c) iswpunct((wint_t) (c))
#       endif
#       ifndef isspace
#       define isspace(c) iswspace((wint_t) (c))
#       endif
#       ifndef isupper
#       define isupper(c) iswupper((wint_t) (c))
#       endif
#       ifndef isxdigit
#       define isxdigit(c) iswxdigit((wint_t) (c))
#       endif

#       ifndef isalphanum
#       define isalphanum(c) isalnum(c)
#       endif

#       ifndef hexDigit
#       define hexDigit(n) ((n)<10 ? '0'+(n) : 'A'+(n)-10)
#       endif

#       define itoa _itoa
#       if _WIN32_WCE < 211
            ExportFunc int sprintf(char *buffer, const char *format,... );
#       endif
        ExportFunc char *strdup(const char *strSource );
#       if _WIN32_WCE < 300
        ExportFunc void *calloc(size_t num, size_t size );
        ExportFunc int strnicmp(const char *s1, const char *s2, int len);
        ExportFunc char *strrchr(char *s, int c);
#       endif
        ExportFunc int stricmp(const char *s1, const char *s2);
        ExportFunc HRESULT AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW);
        ExportFunc HRESULT UnicodeToAnsi(LPWSTR pszW, LPCSTR* ppszA);
        ExportFunc WCHAR *ConvertAnsiToUnicode(const char *string);
        ExportFunc char *ConvertUnicodeToAnsi(WCHAR *string);
        ExportFunc void AGAssertionFailed();
#       ifdef NDEBUG
#           define assert(x)
#       else
#           define assert(x) if (!(x)) AGAssertionFailed()
#       endif

#   else /* _WIN32_WCE */
#       include <stdio.h>
        /* Windows doesn't have gettimeofday() (but it does have struct timeval
           in winsock).  This is an implementation in terms of ftime() which
           only has millisecond resolution. */
        struct timezone {
            int tz_minuteswest;
            int tz_dsttime;
        };
        ExportFunc int gettimeofday(struct timeval *tp, struct timezone *tzp);
#   endif /* _WIN32_WCE */
#endif /* _WIN32 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGUTILWIN_H__ */






