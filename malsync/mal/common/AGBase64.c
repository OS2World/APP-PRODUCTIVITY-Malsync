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

// Owner: tick

#include <AGBase64.h>
#include <AGUtil.h>

const char basis_64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*---------------------------------------------------------------------------*/
static void doonebyte(uint8 *ptr, char *p)
{
    *p++ = basis_64[ptr[0] >> 2];
    *p++ = basis_64[((ptr[0] & 0x3) << 4)];
    *p++ = '=';
    *p++ = '=';
    *p   = '\0';
}
/*---------------------------------------------------------------------------*/
static void dotwobytes(uint8 *ptr, char *p)
{
    *p++ = basis_64[ptr[0] >> 2];
    *p++ = basis_64[((ptr[0] & 0x3) << 4) | ((int) (ptr[1] & 0xF0) >> 4)];
    *p++ = basis_64[((ptr[1] & 0xF) << 2)];
    *p++ = '=';
    *p = '\0';
}
#ifdef _WIN32
/*---------------------------------------------------------------------------*/
ExportFunc void AGBase64EncodedBufferFree(uint8 *buf)
{
    free(buf);
}
#endif
/*---------------------------------------------------------------------------*/
ExportFunc char *AGBase64Encode(uint8 *ptr, int32 bytes)
{
    char *encodedDigest;
    int32 i, modx;
    char *p;
    int32 size;
    

    if (!bytes)
        bytes = strlen((char*)ptr);
            
    /* The encoding process represents 24-bit groups of input bits */
    /*  as output strings of 4 encoded characters. */
    size = ((bytes+2)/3 * 4) + 1;
    
    encodedDigest = (char *) malloc(size);
    
    p = encodedDigest;
    
    if (bytes == 1) {
        doonebyte(ptr, p);
    } else if (bytes == 2) {
        dotwobytes(ptr, p);
    } else {
//        modx  = bytes%3;
        modx = bytes - ((bytes/3) * 3);
        bytes -= modx;
        for (i = 0; i < bytes; i += 3) {
            *p++ = basis_64[ptr[i] >> 2];
            *p++ = basis_64[((ptr[i] & 0x3) << 4) | ((int) (ptr[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((ptr[i + 1] & 0xF) << 2) | ((int) (ptr[i + 2] & 0xC0) >> 6)];
            *p++ = basis_64[ptr[i + 2] & 0x3F];
        }
        ptr += bytes;
        if (modx == 1)
            doonebyte(ptr, p);
        else if (modx == 2)
            dotwobytes(ptr, p);
        else
            *p = '\0';
        
    }
    return encodedDigest;
}
/*---------------------------------------------------------------------------*/
#define BUFINC 256
ExportFunc uint8 *AGBase64Decode(char *source, int32 *len)
{
    char *retbuf, *ob;
    int ixtext;
    int lentext;
    char ch, *ptr, *tptr;
    char inbuf [3];
    int ixinbuf;
    int ignorechar;
    int endtext = 0;
    int bufsize, size = 0;

    bufsize = BUFINC;

    retbuf = ob = (char*)malloc(bufsize);
    if (!retbuf)
        return NULL;
    
    ixtext = 0;
    
    lentext = strlen(source);
    ixinbuf = 0;
    ptr = source;
    while (1) {
        
        if (ixtext >= lentext)
            break;
        
        ch = *ptr++;
        ixtext++;
             
        ignorechar = 0;
             
        if ((ch >= 'A') && (ch <= 'Z'))
            ch = ch - 'A';
        
        else if ((ch >= 'a') && (ch <= 'z'))
            ch = ch - 'a' + 26;
        
        else if ((ch >= '0') && (ch <= '9'))
            ch = ch - '0' + 52;
        
        else if (ch == '+')
            ch = 62;
        
        else if (ch == '=') /*no op -- can't ignore this one*/
            endtext = 1;
        
        else if (ch == '/')
            ch = 63;
        
        else
            ignorechar = 1; 
             
        if (!ignorechar) {
            
            int ctcharsinbuf = 3;
            int leaveLoop = 0;
            
            if (endtext) {
                
                if (ixinbuf == 0)
                    break;
                
                if ((ixinbuf == 1) || (ixinbuf == 2))
                    ctcharsinbuf = 1;
                else
                    ctcharsinbuf = 2;
                
                ixinbuf = 3;
                
                leaveLoop = 1;
            }
            
            inbuf [ixinbuf++] = ch;
            
            if (ixinbuf == 4) {
                ixinbuf = 0;                
                if (size+4 > bufsize) {
                    bufsize += BUFINC;
                    tptr = (char*)realloc(retbuf, bufsize);
                    if (!tptr) {
                        free(retbuf);
                        return NULL;
                    }
                    retbuf = tptr;
                }
                *ob++ = (inbuf [0] << 2) | ((inbuf [1] & 0x30) >> 4);
                size++;
                if (ctcharsinbuf > 1) {
                    *ob++ = ((inbuf [1] & 0x0F) << 4) | ((inbuf [2] & 0x3C) >> 2);
                    size++;
                }
                if (ctcharsinbuf > 2) {
                    *ob++ = ((inbuf [2] & 0x03) << 6) | (inbuf [3] & 0x3F);
                size++;
                }
            }
            
            if (leaveLoop)
                break;
        }
    }

    *ob = 0;
    *len = size;
    
    return (uint8 *)retbuf;
} 


