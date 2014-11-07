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

#ifndef __AGHASHTABLE_H__
#define __AGHASHTABLE_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <AGTypes.h>
#include <AGCollection.h>
#include <AGArray.h>

// AGHashTable holds a collection of key value pairs.  The API
// is in terms of void pointers, but the keys and values can
// easily be integer values as well.  The hash table will grow
// as pairs as added, but will never shrink.
//
// The hash table has callbacks for insertion, removal, hash,
// and compare.  The insert function is called whenever an
// element is added to the hash table.  The return value of
// the insert function is what is actually stored in the table.
// The remove function is called whenever an element is removed
// from the table (including when the table itself is finalized).
// Neither insert nor remove is called when an element is
// replaced by itself.  When elements are different, insert is
// called before remove.
//
// See AGCollection.h for more details on the callbacks.
//
// WARNINGS:
//
// You can put NULLs in AGHashTables, but you'll probably regret
// it.  The table itself can handle NULLs, but you have to make
// sure the callbacks handle them as well.  It is safe to pass
// NULL to AGStrCmp(), AGStrHash(), and AGFree().
//
// If your removeFunc is AGFree() and you put the same element
// in the table more than once, it will get freed more than
// once.
//
// Memory allocation in C collections is really tricky. Be
// careful.

typedef struct AGHashTable {
    int32 count;      // number of pairs actually in the table
    int32 totalCount; // including REMOVED markers
    int32 power;      // size of the table as a power of 2

    int32 *hashCodes;
    void **keys;
    void **values;

    AGCollectionCallbacks keyCallbacks;
    AGCollectionCallbacks valueCallbacks;
} AGHashTable;

// Opaque cover for enumerating the values of the table.

typedef int32 AGHashEnumerator;

// Used to allocate a new hash table on the heap and initialize
// the capacity.  This should be paired with AGHashFree().  Don't
// call AGHashInit() on a table created with AGHashNew().
//
// Equivalent to:
//     table = AGMalloc(sizeof(AGHashTable));
//     table = AGHashInit(table, keyType, valueType, initialCapacity);

ExportFunc AGHashTable *AGHashNew(AGElementType keyType,
                                  AGElementType valueType,
                                  int32 initialCapacity);

// Creates a HashTable which can hold initialCapacity elements without
// resizing.  A capacity of 0 will defer allocating any space for
// elements until needed.  Returns NULL on error (which should only
// happen if we are out of memory).

ExportFunc AGHashTable *AGHashInit(AGHashTable *table,
                                   AGElementType keyType,
                                   AGElementType valueType,
                                   int32 initialCapacity);

// Cleans up all memeory associated with the hash table, but not
// the table struct itself.

ExportFunc void AGHashFinalize(AGHashTable *table);

// Frees a hash table created with AGHashNew().  Don't call
// AGHashFinalize() then AGHashFree().
//
// Equivalent to:
//     AGHashFinalize(table);
//     AGFree(table);

ExportFunc void AGHashFree(AGHashTable *table);

// Sets the callback functions for the keys.

ExportFunc void AGHashSetKeyCallbacks(AGHashTable *table,
                                      AGCompareCallback compareFunc,
                                      AGHashCallback hashFunc,
                                      AGInsertCallback insertFunc,
                                      AGRemoveCallback removeFunc);

// Sets the callback functions for the values.

ExportFunc void AGHashSetValueCallbacks(AGHashTable *table,
                                      AGCompareCallback compareFunc,
                                      AGHashCallback hashFunc,
                                      AGInsertCallback insertFunc,
                                      AGRemoveCallback removeFunc);

// Number of key value pairs in the table.

ExportFunc int32 AGHashCount(AGHashTable *table);

// Returns TRUE if the key is in the table.

ExportFunc AGBool AGHashContainsKey(AGHashTable *table, void *key);

// Same as AGHashContainsKey but if the key is in the table,
// its value is placed in 'value' and the original key is
// placed in 'key'

ExportFunc AGBool AGHashContainsKeyAndGet(AGHashTable *table, void **key, void **value);

// Gets the value for a given key.  Returns NULL if the key
// is not in the table.  Be careful if you put NULLs in the
// table.

ExportFunc void *AGHashGet(AGHashTable *table, void *key);

// Puts a key value pair in the table.  If the key is already in
// the table the old value is replaced by the new value.

ExportFunc void AGHashInsert(AGHashTable *table, void *key, void *value);

// Removed a key value pair from the table.

ExportFunc void AGHashRemove(AGHashTable *table, void *key);

// Removes all elements from the table while perserving the capacity.

ExportFunc void AGHashRemoveAll(AGHashTable *table);

// Begins the enumeration of the table.  Don't change the
// table while you are enumerating it.

ExportFunc AGHashEnumerator AGHashGetEnumerator(AGHashTable *table);

// Gets the next key value pair in the table.  It is OK to pass NULL
// for key or value if you don't care about the return.

ExportFunc AGBool AGHashNextPair(AGHashTable *table, AGHashEnumerator *e,
                                 void **key, void **value);

// Appends all the keys of the table to an array.  AGHashGetKeys()
// followed by AGHashGetValues() will produce parallel arrays.

ExportFunc void AGHashGetKeys(AGHashTable *table, AGArray *array);

// Appends all of the values of the table to an array.

ExportFunc void AGHashGetValues(AGHashTable *table, AGArray *array);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __AGHASHTABLE_H__
