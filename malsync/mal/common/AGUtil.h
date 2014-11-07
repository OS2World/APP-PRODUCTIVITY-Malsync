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

#ifndef __AGUTIL_H__
#define __AGUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

/* time is in elapsed seconds from Jan 1, 1970 UTC*/
ExportFunc uint32 AGTime(void);
ExportFunc void AGTimeMicro(uint32 *sec, uint32 *usec);
ExportFunc uint32 AGSleepMillis(uint32 milliseconds);

// Bit test, set, and clear
#define BIT(flags, b)  ((flags) & b)
#define BIS(flags, b)  ((flags) = (flags) | b)
#define BIC(flags, b)  ((flags) = (flags) & ~b)

#define CHECKANDFREE(x) if(NULL!=x){free(x);x=NULL;}

#ifdef _WIN32
#include <AGUtilWin.h>
#endif

#ifdef __palmos__
#include <AGUtilPalmOS.h>
#endif

#ifdef __unix__
#include <AGUtilUnix.h>
#endif

#if (defined(macintosh) && !defined(__palmos__))
#include <AGUtilMac.h>
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGUTIL_H__ */






