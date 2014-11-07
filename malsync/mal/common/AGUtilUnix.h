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

#ifndef __AGUTILUNIX_H__
#define __AGUTILUNIX_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <AGTypes.h>

#ifdef __unix__
#define AGFreeFunc free
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif /* !min */

#ifdef __sun__
#ifndef MALSYNC
#include <httpd.h>
#include <ap.h>
#endif
#define snprintf ap_snprintf
#define vsnprintf ap_vsnprintf
#endif

#endif /* __unix__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGUTILUNIX_H__ */






