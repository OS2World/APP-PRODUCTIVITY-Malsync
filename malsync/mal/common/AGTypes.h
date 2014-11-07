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

#ifndef __AGTYPES_H__
#define __AGTYPES_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum {
    AG_ERROR_NOT_FOUND,
    AG_ERROR_BAD_ARGUMENT,
    AG_ERROR_READ_FAILED,
    AG_ERROR_WRITE_FAILED,
    AG_ERROR_SYNCHRONIZATION_FAILED,
    AG_ERROR_OUT_OF_MEMORY,
    AG_ERROR_PATH_UNKNOWN,
    AG_ERROR_UNKNOWN_DEVICE_TYPE,
    AG_ERROR_INVALID_SIGNATURE,
    AG_ERROR_UNKNOWN_VERSION,
    AG_ERROR_NOT_IMPLEMENTED,
    AG_ERROR_NONE = 0
};

#ifdef _WIN32
#define ExportFunc __declspec( dllexport )
#else
#define ExportFunc
#endif /* _WIN32 */

#ifdef _WIN32
#define ImportFunc __declspec(dllimport)
#else
#define ImportFunc
#endif

#ifndef __palmos__
/* Standard scalar types */

/* Certain other packages (such as Netscape's NSPR) define fixed-size integer
 * types.  To avoid redefining int8, int16, int32, uint8, uint16 and uint32,
 * we add a test for FIXED_INT_TYPES_DEFINED here and also to the header files of
 * other packages (such as NSPR) which we have to build with.
 * It would be better if each software package (including this one) added its own prefix to
 * these type names to avoid name collisions.
 */
#ifndef FIXED_INT_TYPES_DEFINED
#define FIXED_INT_TYPES_DEFINED

typedef char             int8;
typedef short            int16;
typedef int              int32;

#ifndef _CDGLOBAL_H_
/* The SSLPlus library defines these (but oddly enough not the int*
   typedefs)
*/
typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
#endif /* _CDGLOBAL_H_ */

#endif /* FIXED_INT_TYPES_DEFINED */

typedef int 			 sword;
typedef unsigned int 	 uword;

typedef int AGBool;
#else /* __palmos__ */
/* Standard scalar types for __palmos__ */
#ifndef FIXED_INT_TYPES_DEFINED
#define FIXED_INT_TYPES_DEFINED
typedef char             int8;
typedef short            int16;
typedef long             int32;
 
#ifndef _CDGLOBAL_H_
/* The SSLPlus library defines these (but oddly enough not the int*
   typedefs)
*/
typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned long    uint32;
#endif /* _CDGLOBAL_H_ */

#endif /* FIXED_INT_TYPES_DEFINED */

typedef short 			 sword;
typedef unsigned short 	 uword;

typedef int AGBool;
#endif /* !__palmos__ */

#ifdef FALSE
#undef FALSE
#endif /* FALSE */

#define FALSE 0

#ifdef TRUE
#undef TRUE
#endif /* TRUE */

#define TRUE 1

#ifdef NULL
#undef NULL
#endif /* NULL */

#define NULL 0

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __AGTYPES_H__ */

