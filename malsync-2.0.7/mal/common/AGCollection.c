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

// Owner: linus

#include <AGCollection.h>
#include <AGUtil.h>

ExportFunc void AGCollectionCallbacksInit(AGCollectionCallbacks *callbacks,
                                          AGElementType elemType)
{
    bzero(callbacks, sizeof(*callbacks));

    switch (elemType) {
        case AGIntegerElements:
            break;
        case AGOwnedStringElements:
            callbacks->compareFunc = (AGCompareCallback)AGStrCmp;
            callbacks->hashFunc = (AGHashCallback)AGStrHash;
            callbacks->removeFunc = AGFreeFunc;
            break;
        case AGUnownedStringElements:
            callbacks->compareFunc = (AGCompareCallback)AGStrCmp;
            callbacks->hashFunc = (AGHashCallback)AGStrHash;
            break;
        case AGOwnedPointerElements:
            callbacks->hashFunc = AGPtrHash;
            callbacks->removeFunc = AGFreeFunc;
            break;
        case AGUnownedPointerElements:
            callbacks->hashFunc = AGPtrHash;
            break;
        case AGCustomElements:
            break;
        default:
            break;
    }
}

ExportFunc int32 AGStrCmp(char *s1, char *s2)
{
    if (s1 == s2) {
        return 0;
    }

    if (s1 == NULL) {
        return -1;
    }

    if (s2 == NULL) {
        return 1;
    }

    return strcmp(s1, s2);

}

ExportFunc int32 AGStrHash(char *str)
{
    uint32 hash, c;

    if (str == NULL) {
        return 0;
    }

    hash = 0;

    while ((c = *str++) != 0) {
        hash = (hash * 39) + c;
    }

    return hash;
}

ExportFunc int32 AGPtrHash(void *ptr)
{
    // Memory is typeically allocated 4 byte aligned
    // so throw away the bottom two bits.

    return ((uint32)ptr >> 2);
}
