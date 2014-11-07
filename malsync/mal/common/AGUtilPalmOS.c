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

#include <AGUtil.h>

#ifdef __palmos__

static SWord ag_gettimeofday(struct timeval* tP, struct timezone* tzP);

ExportFunc uint32 AGTime()
{
    struct timeval tP;
    ag_gettimeofday(&tP, NULL);
    return (uint32)tP.tv_sec;
}

ExportFunc void AGTimeMicro(uint32 *sec, uint32 *usec)
{
    struct timeval tP;

    ag_gettimeofday(&tP, NULL);
    *sec = tP.tv_sec;
    *usec = tP.tv_usec;
}

ExportFunc void AGFreeFunc(void *ptr)
{
    /* ANSI C allows free(NULL), but MemPtrFree() does not. */
    if (ptr)
        MemPtrFree(ptr);    
}

ExportFunc void *AGRealloc(void *p, int32 s)
{
    void *ptr = NULL;
    
    if(!p)
        return MemPtrNew(s);
    
    ptr = MemPtrNew(s);
    if(!ptr)
        return NULL;
        
    MemMove(ptr, p, MemPtrSize(p));
    MemPtrFree(p);
    return ptr;
}

ExportFunc void *AGCalloc(size_t num, size_t size)
{
    void *retval = malloc(num * size);
    if (retval)
        memset(retval, 0, num * size);
    return retval;
}

ExportFunc void *AGMemCopy(void *dst, const void *src, size_t len)
{
    if(dst != NULL && src != NULL)
        MemMove(dst, (void *)src, len);
    return dst;
}

ExportFunc char *AGStrDup(char *str)
{
    char *result;
    uint32 len;

    if(str == NULL) {
        str = ""; 
        len = 0;
    } else {
        len = strlen(str);
    }

    result = malloc(len+1);
    if(result != NULL)
        bcopy(str, result, len+1);

    return result;
}

ExportFunc char *AGStrRChr(const char* str, int8 chr)
{
    char c = (char)chr;
    int32 len;

    if(!str)
        return NULL;

    len = strlen(str);
    str = str+len;
    while(len >= 0) {
        if(*str == c)
            return (char *)str;
        str--;
        len--;      
    }
    return NULL;
}

/***********************************************************************
 *
 * FUNCTION:    ag_gettimeofday
 *
 * DESCRIPTION: Return the time of day and current timezone information
 *              The time of day is returned in Greenwich time and is specified
 *              in seconds and microseconds since Midnite, Jan 1, 1970. 
 *
 *              The time zone information is for informative purposes only
 *              and can be used by the caller to calculate the time in the
 *              local area accounting for daylight savings and time zone
 *              differences. 
 *
 * CALLED BY:   Applications.
 *
 * PARAMETERS:  tp - current time is returned here.
 *              tzp - current time zone information is returned here
 *                      This parameter can be null.
 *
 * RETURNED:    0 if no error
 *              -1 on error, errno contains error code
 *
 ***********************************************************************/
static  DWord       PrvSecondsOffset = 0;
#define   offset1970  0x7C25B080
static SWord        ag_gettimeofday(struct timeval* tP, struct timezone* tzP)
{
    DWord       ticks, secondsOn, subTicksOn, estSeconds;
    DWord       actSeconds;
    Long        dSecs;
    
    
    // If tzP is non-nil, get the time zone info
    if (tzP) {
        tzP->tz_minuteswest = PrefGetPreference(prefMinutesWestOfGMT);
        tzP->tz_dsttime = DST_NONE;
        }
        
    // If no tP, do nothing
    if (!tP) return 0;
    
    
    // Get the current value from our real-time clock and our current
    //  tick count.
    actSeconds = TimGetSeconds();
    ticks = TimGetTicks();

    
    //---------------------------------------------------------------
    // We use ticks to estimate the time since we desire microseconds
    //  granularity, not 1 second granularity like our real-time clock
    //  provides.
    //---------------------------------------------------------------
    secondsOn = ticks / sysTicksPerSecond;
    subTicksOn = ticks % sysTicksPerSecond;
    
    
    // If we have a calculated offset (this is not the first invocation),
    //   add that to our current seconds calculation to get the seconds
    //   since 1970.
    estSeconds = secondsOn + PrvSecondsOffset;
        
        
        
    //---------------------------------------------------------------
    // Now, make sure we're in the ballpark by comparing our tick time
    //  with our real-time clock time.
    //---------------------------------------------------------------
    // If way off, re-calculate PrvSecondsOffset;
    dSecs = actSeconds - estSeconds;
    if (dSecs < 0) dSecs = -dSecs;
    if (dSecs > 10) {
        PrvSecondsOffset = actSeconds - secondsOn;
        estSeconds = actSeconds;
        subTicksOn = 0;
        }
        
        
    // Use subTicksOn to estimate microseconds and return the time
    //  since 1970, not 1904 like our native code does.
    tP->tv_sec = estSeconds - offset1970;
    tP->tv_usec = (subTicksOn * 1000000L) / sysTicksPerSecond;

    // Exit 
    return 0;
}


ExportFunc uint32 AGSleepMillis(uint32 milliseconds)
{
#if 0
    /*
     * SysTaskDelay's argument is ticks. sysTicksPerSecond defines
     * ticks/second (as 100 on the device).
     */
    SysTaskDelay(milliseconds/(1000/sysTicksPerSecond));
#else
    /*
     * PENDING(klobad)
     * This is incorrect, but other code seems to depend on this for now.
     */
    SysTaskDelay(milliseconds);
#endif
    return 0;
}


#endif

