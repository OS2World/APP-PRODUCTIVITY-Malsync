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

#if (defined(macintosh) && !defined(__palmos__))

#include <AGUtil.h>
#include <AGTypes.h>

ExportFunc uint32 AGTime()
{
    return 0; /* pending(miket) do this */
}

ExportFunc void AGTimeMicro(uint32 *sec, uint32 *usec)
{
    return; /* pending(miket) do this */
}

ExportFunc int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    return 0; /* pending(miket) do this */
}

char *strdup(const char *str)
{
    char * newStr;
    
    if (str == NULL)
        return NULL;

    newStr = (char *)malloc(strlen(str) + 1);

    strcpy(newStr, str);

    return newStr;
}

#endif /* #if (defined(macintosh) && !defined(__palmos__)) */