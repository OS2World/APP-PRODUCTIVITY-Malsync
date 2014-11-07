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

#ifndef _MD5_H_
#define _MD5_H_

/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.
   
   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.
   
   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.
   
   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.
   
   These notices must be retained in any copies of any part of this
   documentation and/or software.
*/
#include <AGTypes.h> 

/* MD5 context. */
typedef struct {
    unsigned long state[4];                                   /* state (ABCD)*/
    unsigned long count[2];        /* number of bits, modulo 2^64 (lsb first)*/
    unsigned char buffer[64];                                 /* input buffer*/
} AGMD5_CTX;

ExportFunc void AGMD5Init(AGMD5_CTX *ctx);
ExportFunc void AGMD5Update(AGMD5_CTX *ctx, unsigned char *a, unsigned int b);
ExportFunc void AGMD5Final(unsigned char a[16], AGMD5_CTX *ctx);

#endif






