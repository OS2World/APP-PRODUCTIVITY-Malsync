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

#ifdef __unix__

#include <time.h>

#include <os2.h>

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


ExportFunc uint32 AGSleepMillis(uint32 milliseconds)
{
#ifdef __EMX__
	DosSleep(milliseconds);
#else
    usleep(milliseconds*1000);
#endif
    return 0;
}

#endif /* __unix__ */
