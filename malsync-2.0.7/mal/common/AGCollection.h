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

#ifndef __AGCOLLECTION_H__
#define __AGCOLLECTION_H__

#include <AGTypes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// This defines a new type called AGCompareCallback, members of
// which are functions taking two void pointers and returning
// an int.  Compare callbacks must return a number less than
// zero if the first arg is less than the second, a number greater
// than zero if the first arg is greater than the second, and return
// zero if the two args are equal.  Same returns as strcmp().

typedef int32 (*AGCompareCallback)(void *, void *);

// AGHashCallbacks are functions taking a void pointer and
// returning an int.  When paired with a compare function
// following invariant MUST be maintained:
//     if compare(a, b) == 0 then hash(a) == hash(b)

typedef int32 (*AGHashCallback)(void *);

// AGInsertCallbacks are functions taking a void pointer
// and returning a void pointer.  Collections call this
// function with the newly inserted element as the arg.
// The return value of this function is what is actually
// stored in the collection.  The most common use for this
// callback is to make a copy of the element for storage
// in the collection.

typedef void *(*AGInsertCallback)(void *);

// AGRemoveCallbacks are functions which take a void
// pointer and have no return value.  This function is called
// whenever an element is removed from the collection or
// the collection is finalized.  Most often the remove
// callback for a collection is set to NULL (do nothing)
// or AGFree().

typedef void (*AGRemoveCallback)(void *);

// This is a convenience type to hold a group of callbacks.  This
// is used by implementors of collections.

typedef struct AGCollectionCallbacks {
    AGCompareCallback compareFunc;
    AGHashCallback hashFunc;
    AGInsertCallback insertFunc;
    AGRemoveCallback removeFunc;
} AGCollectionCallbacks;

// The two principal collection classes, AGArray and AGHashTable,
// know about the common kinds of elements which they store.
// This enum is a convenience for specifing the callback
// functions for the standard element types:
//
// AGIntegerElements
//     compareFunc = NULL;
//     hashFunc    = NULL;
//     insertFunc  = NULL;
//     removeFunc  = NULL;
//
// AGOwnedStringElements
//     compareFunc = AGStrCmp;
//     hashFunc    = AGStrHash;
//     insertFunc  = NULL;
//     removeFunc  = AGFree;
//
// AGUnownedStringElements
//     compareFunc = AGStrCmp;
//     hashFunc    = AGStrHash;
//     insertFunc  = NULL;
//     removeFunc  = NULL;
//
// AGOwnedPointerElements
//     compareFunc = NULL;
//     hashFunc    = AGPtrHash;
//     insertFunc  = NULL;
//     removeFunc  = AGFree;
//
// AGUnownedPointerElements
//     compareFunc = NULL;
//     hashFunc    = AGPtrHash;
//     insertFunc  = NULL;
//     removeFunc  = NULL;
//
// AGCustomElements
//     compareFunc = NULL;
//     hashFunc    = NULL;
//     insertFunc  = NULL;
//     removeFunc  = NULL;
//
// Note that for "owned" strings and pointers the insertFunc is NULL,
// so ownership is tranferred from the caller to the collection when
// the element is inserted.  The collection will free the element
// when the element is removed or the collection is finalized.  The
// caller MUST NOT free the element.  Very often the caller will want
// to copy the element before passing it to the collection or set
// a non-null insert function to have the copy made automatically.
// If a function is NULL a default action is taken:
//
// compareFunc == NULL
//     signed integer comparision.
// hashFunc == NULL
//     signed integer hash
// insertFunc == NULL
//     do nothing
// removeFunc == NULL
//     do nothing
//
// By convention the by convention neither the insert nor the remove
// function is called when an element is being replaced by itself.

typedef enum {
    AGIntegerElements,
    AGOwnedStringElements,
    AGUnownedStringElements,
    AGOwnedPointerElements,
    AGUnownedPointerElements,
    AGCustomElements
} AGElementType;

// This convenience for collection implementors sets up the
// callbacks struct from the element type enum.

ExportFunc void AGCollectionCallbacksInit(AGCollectionCallbacks *callbacks,
                                          AGElementType elemType);

// Common functions needed by the collections.  We cover
// strcmp() to make sure we get the behavior we want
// with NULL elements.

ExportFunc int32 AGStrCmp(char *s1, char *s2);
ExportFunc int32 AGStrHash(char *str);
ExportFunc int32 AGPtrHash(void *ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGCOLLECTION_H__
