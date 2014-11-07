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

#include <AGDigest.h>
#include <md5.h>
#include <AGUtil.h>

/*---------------------------------------------------------------------------*/
void AGDigest(char *username, uint8 pass[16], uint8 nonce[16],
              uint8 digest[16])
{
    
    AGMD5_CTX context;    
    AGMD5Init (&context);
    AGMD5Update (&context, (uint8 *)username, strlen(username));
    AGMD5Update (&context, pass, 16);
    AGMD5Update (&context, nonce, 16);
    AGMD5Final (digest, &context);

}
/*---------------------------------------------------------------------------*/
AGBool AGDigestCompare(uint8 a[16], uint8 b[16])
{
    int i;
    for(i=0;i<16;i++)
        if(a[i]!=b[i])
            return 0;
    return 1;
    
}
/*---------------------------------------------------------------------------*/
AGBool AGDigestNull(uint8 a[16])
{
    int i;
    for(i=0;i<16;i++)
        if(a[i])
            return 0;
    return 1;
}
/*---------------------------------------------------------------------------*/
ExportFunc void AGDigestSetToNull(uint8 a[16])
{
    int i;
    for(i=0;i<16;i++)
        a[i]=0;
}
