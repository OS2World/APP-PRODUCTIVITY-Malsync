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

#ifdef _WIN32
#include <AGUtilWin.h>

int32 index(const char *str, int8 c)
{
    int32 i;
    int8 t;
    
    i = 0;
    while ((t = (int8)str[i]) != 0) {
        if (t == c) {
            return i;
        }

        i++;
    }

    return -1;
}

int32 rindex(const char *str, int8 c)
{
    int32 i;
    int8 t;
    
    i = strlen(str) - 1;
    while ((t = (int8)str[i]) != 0) {
        if (t == c) {
            return i;
        }

        i--;
    }

    return -1;
}

ExportFunc TCHAR *AGTStrDup(char *str)
{
#ifdef _WIN32_WCE
    return (TCHAR *)ConvertAnsiToUnicode(str);
#else
    return strdup(str);
#endif
}

ExportFunc char *AGCStrDup(TCHAR *str)
{
#ifdef _WIN32_WCE
    return (char *)ConvertUnicodeToAnsi(str);
#else
    return strdup(str);
#endif
}

#ifndef _WIN32_WCE

/* Windows32 ONLY functions */

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
//#include <winsock2.h>
ExportFunc uint32 AGTime()
{
    struct timeval tP;

    gettimeofday(&tP, NULL);
    return (uint32)tP.tv_sec;
}

ExportFunc void AGTimeMicro(uint32 *sec, uint32 *usec)
{
    struct timeval tP;

    gettimeofday(&tP, NULL);
    *sec = tP.tv_sec;
    *usec = tP.tv_usec;
}

ExportFunc int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    struct _timeb tb;
    
    _ftime(&tb);
    
    if (tp != NULL) {
        tp->tv_sec = tb.time;
        tp->tv_usec = tb.millitm * 1000;
    }
    
    if (tzp != NULL) {
        tzp->tz_minuteswest = tb.timezone;
        tzp->tz_dsttime = tb.dstflag;
    }

    return 0;
}

#else /* _WIN32_WCE */

/* WindowCE ONLY Functions*/

#include <windows.h>
#include <windowsx.h>

#ifndef _TIME_T_DEFINED
typedef long time_t;        /* time value */
#define _TIME_T_DEFINED     /* avoid multiple def's of time_t */
#endif  /* _TIME_T_DEFINED */

#ifndef _TM_DEFINED
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#define _TM_DEFINED
#endif  /* _TM_DEFINED */

time_t tm2sec(const struct tm * t)
{
    int year;
    time_t days;
    static const int dayoffset[12] =
    {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};

    year = t->tm_year;

    if (year < 70 || ((sizeof(time_t) <= 4) && (year >= 138)))
    return 0;

    /* shift new year to 1st March in order to make leap year calc easy */

    if (t->tm_mon < 2)
    year--;

    /* Find number of days since 1st March 1900 (in the Gregorian calendar). */

    days = year * 365 + year / 4 - year / 100 + (year / 100 + 3) / 4;
    days += dayoffset[t->tm_mon] + t->tm_mday - 1;
    days -= 25508;      /* 1 jan 1970 is 25508 days since 1 mar 1900 */

    days = ((days * 24 + t->tm_hour) * 60 + t->tm_min) * 60 + t->tm_sec;

    if (days < 0)
    return 0;   /* must have overflowed */
    else
    return days;        /* must be a valid time */
}

ExportFunc uint32 AGTime()
{
    SYSTEMTIME st;
    struct tm time;

    // PENDING (jason) this currently doesn't accurately adjust for
    // timezone or daylight savings time...
    GetSystemTime(&st);
    time.tm_sec = st.wSecond;
    time.tm_min = st.wMinute;
    time.tm_hour = st.wHour;
    time.tm_mday = st.wDay;
    time.tm_mon = st.wMonth - 1; // SYSTEMTIME defines months as 1-12, tm as 0-11;
    time.tm_year = st.wYear - 1900;
    time.tm_wday = st.wDayOfWeek;
    time.tm_yday = 0; // Not used in tm2sec...
    time.tm_isdst = 0;

    return (uint32)tm2sec(&time);
}

#if _WIN32_WCE < 211

int sprintf( char *buffer, const char *format, ...)
{
    char *newFormat;
    WCHAR *newFormatW;
    char *newBuffer;
    WCHAR newBufferW[1000]; // PENDING (jason) Wish we new how big to make this...
    va_list p;
    int i, length;

    if (!buffer || !format)
        return -1;

    newFormat = strdup(format);

    for (i = 0, length = strlen(newFormat); i < length; i++) {
        if (newFormat[i] == '%' && i < length - 1) {
            if (newFormat[i + 1] == 's')
                newFormat[++i] = 'S';
            else if (newFormat[i + 1] == 'S')
                newFormat[++i] = 's';
        }
    }

    newFormatW = ConvertAnsiToUnicode(newFormat);
    free((void *)newFormat);

    va_start(p, format);
    length = vswprintf(newBufferW, newFormatW, p);
    free((void *)newFormatW);

    newBuffer = ConvertUnicodeToAnsi(newBufferW);
    strcpy(buffer, newBuffer);
    free((void *)newBuffer);

    return length;
}

#endif

char *strdup(const char *str)
{
    char * newStr;
    
    if (str == NULL)
        return NULL;

    newStr = (char *)malloc(strlen(str) + 1);

    strcpy(newStr, str);

    return newStr;
}

int stricmp(const char *s1, const char *s2) {
    int i = 0;

    if (s1 == s2)
        return 0;

    if (s1 == NULL)
        return -1;

    if (s2 == NULL)
        return 1;

    while (s1[i] != 0 && s2[i] != 0) {
        if (tolower(s1[i]) != tolower(s2[i]))
            return s1[i]<s2[i]?-1:1;
        i++;
    }

    if (s1[i] == 0 && s2[i] == 0)
        return 0;
    else
        return s1[i]?1:-1;
}

#if _WIN32_WCE < 300
void *calloc( size_t count, size_t size )
{
    void *retval = malloc(count * size);
    if (retval)
        memset(retval, 0, count * size);
    return retval;
}

int strnicmp(const char *s1, const char *s2, int len) {
    int i = 0;

    if (s1 == s2)
        return 0;

    if (s1 == NULL)
        return -1;

    if (s2 == NULL)
        return 1;

    while (s1[i] != 0 && s2[i] != 0 && i < len) {
        if (tolower(s1[i]) != tolower(s2[i]))
            return s1[i]<s2[i]?-1:1;
        i++;
    }

    if(i == len)
        return 0;

    if (s1[i] == 0 && s2[i] == 0)
        return 0;
    else
        return s1[i]?1:-1;
}

char *strrchr(char *s, int c)
{
    char* last = NULL;

    if (s != NULL) {
        char* p;
    
        for (p = s; *p != '\0'; p++) {
            if (*p == c)
                last = p;
        }
    }
    
    return last;
}
#endif

extern HRESULT AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW) { 
    ULONG cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (NULL == pszA)
    {
        *ppszW = NULL;
        return 0;
    }

    // Determine number of wide characters to be allocated for the
    // Unicode string.
    cCharacters =  strlen(pszA)+1;

    // Use of the OLE allocator is required if the resultant Unicode
    // string will be passed to another COM component and if that
    // component will free it. Otherwise you can use your own allocator.
    if (*ppszW == NULL) {
        *ppszW = (LPWSTR)malloc(cCharacters*2);
        if (NULL == *ppszW)
            return E_OUTOFMEMORY;
    }

    // Covert to Unicode.
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
                  *ppszW, cCharacters))
    {
        dwError = GetLastError();
        free(*ppszW);
        *ppszW = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return 0;
} 

extern HRESULT UnicodeToAnsi(LPWSTR pszW, LPCSTR* ppszA) { 
    ULONG cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (NULL == pszW)
    {
        *ppszA = NULL;
        return 0;
    }

    // Determine number of wide characters to be allocated for the
    // Unicode string.
    for (cCharacters = 0; pszW[cCharacters] != 0; cCharacters++);
    cCharacters++;

    // Use of the OLE allocator is required if the resultant Unicode
    // string will be passed to another COM component and if that
    // component will free it. Otherwise you can use your own allocator.
    if (*ppszA == NULL) {
        *ppszA = (LPCSTR)malloc(cCharacters);
        if (NULL == *ppszA)
            return E_OUTOFMEMORY;
    }

    // Covert to Unicode.
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters,
                  (LPSTR)*ppszA, cCharacters, NULL, NULL))
    {
        dwError = GetLastError();
        free((void *)*ppszA);
        *ppszA = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return 0;
}

WCHAR *ConvertAnsiToUnicode(const char *string)
{
    WCHAR *buf = NULL;
    /* (adam) I simplified the implementation of this function.  There's no need
     * to check for NULL; AnsiToUnicode() does this already.
     * There's also no need to allocate our own buffer to hold the result string,
     * since AnsiToUnicode() will allocate a buffer if we set buf to NULL before
     * calling it.
     */
    AnsiToUnicode(string, &buf);
    return buf;
}

char *ConvertUnicodeToAnsi(WCHAR *string) {
    char *buf;

    if (!string)
        return NULL;

    buf = (char *)malloc(wcslen(string) + 1);

    if (UnicodeToAnsi(string, &buf) == 0)
        return buf;

    free((void *)buf);

    return NULL;
}

void AGAssertionFailed()
{
    DebugBreak();
}

#endif /* _WIN32_WCE */
#endif /* _WIN32 */


