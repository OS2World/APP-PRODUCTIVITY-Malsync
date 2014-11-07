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

#ifndef __AGARRAY_H__
#define __AGARRAY_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGCollection.h>

// AGArray holds a collection of elements ordered by index.  In
// the API the elements are void pointers, but they can easily
// be integer values as well.  The array will grow as elements
// are added, but will never shrink.
//
// The array has callbacks for element insertion, removal, and
// comparison.  The remove callback is invoked for each element
// in the array when the array is finalized.  Neither the insert
// nor remove function is called when an element is replaced by
// itself. When the elements are different, insert is called
// before remove.  The return value of the insert callback is
// what is stored in the array.
//
// See AGCollection.h for more discussion of the callbacks.
//
// WARNINGS:
//
// You can put NULLs in AGArrays, but you'll probably regret it.
// The array itself can handle NULLs, but you have to make sure
// the callbacks handle them as well.  It is safe to pass NULL
// to AGStrCmp(), AGStrHash(), and AGFree().
//
// If your removeFunc is AGFree() and you put the same element
// in the array more than once, it will get freed more than
// once.
//
// Memory allocation in C collections is really tricky. Be
// careful.

typedef struct AGArray {
    int32 count;
    int32 capacity;
    void **elements;
    AGCollectionCallbacks callbacks;
} AGArray;


// Used to allocate a new array on the heap and initialize
// the capacity.  This should be paired with AGArrayFree().  Don't
// call AGArrayInit() on an array created with AGArrayNew().
//
// Equivalent to:
//     array = AGMalloc(sizeof(AGArray));
//     array = AGArrayInit(array, elemType, initialCapacity);

ExportFunc AGArray *AGArrayNew(AGElementType elemType,
                               int32 initialCapacity);

// Creates an array which can hold initialCapacity elements without
// resizing.  A capacity of 0 will defer allocating any space for
// elements until needed.  Returns NULL on error (which should only
// happen if we are out of memory).

ExportFunc AGArray *AGArrayInit(AGArray *array,
                                AGElementType elemType,
                                int32 initialCapacity);

// Cleans up all memory associated with the array, but not
// the array struct itself.

ExportFunc void AGArrayFinalize(AGArray *array);

// Frees an array created with AGArrayNew().  Don't call
// AGArrayFinalize() then AGArrayFree().
//
// Equivalent to:
//     AGArrayFinalize(array);
//     AGFree(array);

ExportFunc void AGArrayFree(AGArray *array);

// Makes sure the array can hold at least minCapacity elements
// without growing.

ExportFunc void AGArrayEnsureCapacity(AGArray *array, int32 minCapacity);

// Sets the callbacks for compare, insert, and remove.  See
// AGCollection.h for details.

ExportFunc void AGArraySetCallbacks(AGArray *array,
                                    AGCompareCallback compareFunc,
                                    AGInsertCallback insertFunc,
                                    AGRemoveCallback removeFunc);

// Returns the number of elements in the array.

ExportFunc int32 AGArrayCount(AGArray *array);

// Hopefully, this one is obvious.

ExportFunc void *AGArrayElementAt(AGArray *array, int32 index);

// Inserts the element at index.  If there are any elements at
// or after index, they are moved up one index.  If index is
// greater than the array's count, the call is ignored.  Don't
// do that.  The insert function is called and the return value
// in put in the array.

ExportFunc void AGArrayInsertAt(AGArray *array, int32 index, void *elem);

// Appends an element to the end of the array, growing it
// if necessary.  Same as: AGArrayInsertAt(a, count, elem);

ExportFunc void AGArrayAppend(AGArray *array, void *elem);

// Removes the element at index.  The remove function is called,
// and any elements to the right are shifted left by one index.

ExportFunc void AGArrayRemoveAt(AGArray *array, int32 index);

// Removes all elements from the table while perserving the
// capacity.  The remove function is called for each element.

ExportFunc void AGArrayRemoveAll(AGArray *array);

// Appends the value to the array if it isn't already there.
// The compare function is used to find the matching element.
// Returns whether or not the element was added.
//
// WARNING: If the element was not added to the array, the
// caller may need to free the element.

ExportFunc AGBool AGArrayAppendIfAbsent(AGArray *array, void *elem);

// Appends all the elements of other on to the array.

ExportFunc void AGArrayAppendArray(AGArray *array, AGArray *other);

// Once again, I hope this is clear.

ExportFunc void AGArrayReplaceAt(AGArray *array, int32 index, void *elem);

// Looks for a matching element using the compare function,
// or pointer equality if the compare function is NULL.

ExportFunc int32 AGArrayIndexOf(AGArray *array, void *elem, int32 startIndex);

// Same as before, but works backwards.

ExportFunc int32 AGArrayLastIndexOf(AGArray *array, void *elem, int32 startIndex);

// PENDING(linus) Comming soon...
// ExportFunc void AGArraySort(AGArray *array, AGBool ascending);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGARRAY_H_
