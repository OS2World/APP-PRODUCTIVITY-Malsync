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

#include <AGHashTable.h>
#include <AGUtil.h>

#define HASH_EMPTY 0
#define HASH_REMOVED 1
#define HASH_DEFAULT 2

#define HASH_MIN_POWER 3

#define HASH_MULTIPLIER 0x9e3779b9

// PENDING(linus) We use a 2/3rds load factor for the hash table.
// We should do a number of tests on the hash table to make sure
// it is functioning well for URLs.

#define HASH_LOAD_NUMERATOR 2
#define HASH_LOAD_DENOMINATOR 3

#define HASH_SHIFT(p) (32 - (p))
#define HASH_INDEX_MASK(p) ((1 << (p)) - 1)
#define HASH_CAPACITY(p) (HASH_LOAD_NUMERATOR * (1 << (p)) / HASH_LOAD_DENOMINATOR)
#define HASH_TABLE_SIZE(p) (1 << (p))

static int32 tableIndexFor(AGHashTable *table, void *key, int32 hashCode)
{
    uint32 product;
    int32 testHash, index, step, removedIndex, probeCount, power;
    int32 *hashCodes;
    void *testKey;
    AGCompareCallback compareFunc;

    product = hashCode * HASH_MULTIPLIER;
    power = table->power;
    index = product >> HASH_SHIFT(power);

    // Probe the first slot in the table.  We keep track of the first
    // index where we found a REMOVED marker so we can return that index
    // as the first available slot if the key is not already in the table.

    compareFunc = table->keyCallbacks.compareFunc;
    hashCodes = table->hashCodes;
    testHash = hashCodes[index];

    if (testHash == hashCode) {
        testKey = table->keys[index];
        if (compareFunc != NULL) {
            if ((*compareFunc)(key, testKey) == 0) {
                return index;
            }
        } else if (key == testKey) {
            return index;
        }

        removedIndex = -1;
    } else if (testHash == HASH_EMPTY)
        return index;
    else if (testHash == HASH_REMOVED)
        removedIndex = index;
    else
        removedIndex = -1;

    // Our first probe has failed, so now we need to start looking
    // elsewhere in the table.

    step = ((product >> (2 * HASH_SHIFT(power) - 32)) & HASH_INDEX_MASK(power)) | 1;
    probeCount = 1;

    do {
        probeCount++;
        index = (index + step) & HASH_INDEX_MASK(power);

        testHash = hashCodes[index];

        if (testHash == hashCode) {
            testKey = table->keys[index];
            if (compareFunc != NULL) {
                if ((*compareFunc)(key, testKey) == 0) {
                    return index;
                }
            } else if (key == testKey) {
                return index;
            }
        } else if (testHash == HASH_EMPTY) {
            if (removedIndex < 0)
                return index;
            else
                return removedIndex;
        } else if (testHash == HASH_REMOVED && removedIndex == -1) {
            removedIndex = index;
        }
    } while (probeCount <= table->totalCount);

    // Something very bad has happened.

    return 0;
}

static void grow(AGHashTable *table, int32 power)
{
    int32 i, index, hashCode, oldPower;
    int32 *oldHashCodes, *hashCodes;
    void *key, **keys, **oldKeys, **values, **oldValues;

    oldHashCodes = table->hashCodes;
    oldKeys = table->keys;
    oldValues = table->values;
    oldPower = table->power;

    // The table size needs to be a power of two, and it should double
    // when it grows.

    hashCodes = (int32*)calloc(HASH_TABLE_SIZE(power), sizeof(int32));
    keys = (void**)calloc(HASH_TABLE_SIZE(power), sizeof(void *));
    values = (void**)calloc(HASH_TABLE_SIZE(power), sizeof(void *));
 
    // Reinsert the old elements into the new table if there are any.  Be
    // sure to reset the counts and increment them as the old entries are
    // put back in the table.

    table->totalCount = 0;
    table->power = power;
    table->values = values;
    table->keys = keys;
    table->hashCodes = hashCodes;

    if (table->count > 0) {
        table->count = 0;

        for (i = 0; i < HASH_TABLE_SIZE(oldPower); i++) {
            hashCode = oldHashCodes[i];

            if (hashCode != HASH_EMPTY && hashCode != HASH_REMOVED) {
                key = oldKeys[i];
                index = tableIndexFor(table, key, hashCode);

                hashCodes[index] = hashCode;
                keys[index] = key;
                values[index] = oldValues[i];

                table->count++;
                table->totalCount++;
            }
        }
    }

    if (oldHashCodes != NULL) {
        free(oldHashCodes);
        free(oldKeys);
        free(oldValues);
    }
}

static void initCapacity(AGHashTable *table, int32 capacity)
{
    int32 tableSize, power;

    // Find the lowest power of 2 size for the table which will allow
    // us to insert capacity number of objects before having to
    // grow.

    tableSize = (capacity * HASH_LOAD_DENOMINATOR) / HASH_LOAD_NUMERATOR;

    power = HASH_MIN_POWER;
    while (HASH_TABLE_SIZE(power) < tableSize) {
        power++;
    }

    grow(table, power);
}

static int32 computeHash(AGHashTable *table, void *key)
{
    int32 hashCode;
    AGHashCallback hashFunc;

    hashFunc = table->keyCallbacks.hashFunc;
    if (hashFunc != NULL) {
        hashCode = (*hashFunc)(key);
    } else {
        hashCode = (int32)key;
    }

    if (hashCode == HASH_EMPTY || hashCode == HASH_REMOVED) {
        hashCode = HASH_DEFAULT;
    }
    return hashCode;
}

ExportFunc AGHashTable *AGHashNew(AGElementType keyType,
                                  AGElementType valueType,
                                  int32 initialCapacity)
{
    AGHashTable *table;

    table = (AGHashTable *)malloc(sizeof(AGHashTable));
    return AGHashInit(table, keyType, valueType, initialCapacity);
}

ExportFunc AGHashTable *AGHashInit(AGHashTable *table,
                                   AGElementType keyType,
                                   AGElementType valueType,
                                   int32 initialCapacity)
{
    bzero(table, sizeof(*table));

    AGCollectionCallbacksInit(&(table->keyCallbacks), keyType);
    AGCollectionCallbacksInit(&(table->valueCallbacks), valueType);

    if (initialCapacity != 0) {
        initCapacity(table, initialCapacity);
    } else {
        table->power = 3;
    }

    return table;
}

ExportFunc void AGHashFinalize(AGHashTable *table)
{
    AGHashRemoveAll(table);

    free(table->hashCodes);
    free(table->keys);
    free(table->values);

    bzero(table, sizeof(*table));
}

ExportFunc void AGHashFree(AGHashTable *table)
{
    AGHashFinalize(table);
    free(table);
}

ExportFunc void AGHashSetKeyCallbacks(AGHashTable *table,
                                      AGCompareCallback compareFunc,
                                      AGHashCallback hashFunc,
                                      AGInsertCallback insertFunc,
                                      AGRemoveCallback removeFunc)
{
    table->keyCallbacks.compareFunc = compareFunc;
    table->keyCallbacks.hashFunc = hashFunc;
    table->keyCallbacks.insertFunc = insertFunc;
    table->keyCallbacks.removeFunc = removeFunc;
}

ExportFunc void AGHashSetValueCallbacks(AGHashTable *table,
                                        AGCompareCallback compareFunc,
                                        AGHashCallback hashFunc,
                                        AGInsertCallback insertFunc,
                                        AGRemoveCallback removeFunc)
{
    table->valueCallbacks.compareFunc = compareFunc;
    table->valueCallbacks.hashFunc = hashFunc;
    table->valueCallbacks.insertFunc = insertFunc;
    table->valueCallbacks.removeFunc = removeFunc;
}

ExportFunc int32 AGHashCount(AGHashTable *table)
{
    return table->count;
}

ExportFunc AGBool AGHashContainsKey(AGHashTable *table, void *key)
{
    int32 index;
    AGCompareCallback compareFunc;

    if (table->count == 0) {
        return FALSE;
    }

    // This will return the index in the table for the key.  If a
    // value is there, return TRUE, else return FALSE.

    index = tableIndexFor(table, key, computeHash(table, key));

    compareFunc = table->keyCallbacks.compareFunc;
    if (compareFunc) {
        if ((*compareFunc)(table->keys[index], key) == 0) {
            return TRUE;
        }
    } else {
        if (table->keys[index] == key) {
            return TRUE;
        }
    }

    return FALSE;
}

ExportFunc AGBool AGHashContainsKeyAndGet(AGHashTable *table, void **key, void **value)
{
    int32 index;
    AGCompareCallback compareFunc;

    if (table->count == 0 || key == NULL) {
        return FALSE;
    }

    // This will return the index in the table for the key.  If a
    // value is there, return TRUE, else return FALSE.

    index = tableIndexFor(table, *key, computeHash(table, *key));

    compareFunc = table->keyCallbacks.compareFunc;
    if (compareFunc) {
        if ((*compareFunc)(table->keys[index], *key) == 0) {
            *key = table->keys[index];
            if (value != NULL) {
                *value = table->values[index];
            }
            return TRUE;
        }
    } else {
        if (table->keys[index] == *key) {
            *key = table->keys[index];
            if (value != NULL) {
                *value = table->values[index];
            }
            return TRUE;
        }
    }

    return FALSE;
}


ExportFunc void *AGHashGet(AGHashTable *table, void *key)
{
    int32 index;

    // We delay allocation of the arrays.

    if (table->count == 0) {
        return NULL;
    }

    // This will return the index in the table for the key.  If a
    // value is there, great.  If not, this will return NULL.

    index = tableIndexFor(table, key, computeHash(table, key));
    return table->values[index];
}

ExportFunc void AGHashInsert(AGHashTable *table, void *key, void *value)
{
    int32 index, hash, oldHash;
    void *oldKey, *oldValue;
    AGInsertCallback keyInsertFunc, valueInsertFunc;
    AGRemoveCallback keyRemoveFunc, valueRemoveFunc;

    // We delay allocation of the arrays.

    if (table->hashCodes == NULL) {
        grow(table, HASH_MIN_POWER);
    }

    hash = computeHash(table, key);
    index = tableIndexFor(table, key, hash);
    oldHash = table->hashCodes[index];

    // If the total number of occupied slots (either with a real element or
    // a removed marker) gets too big, grow the table.

    if (oldHash == HASH_EMPTY || oldHash == HASH_REMOVED) {
        if (oldHash == HASH_EMPTY) {
            if (table->totalCount >= HASH_CAPACITY(table->power)) {
                grow(table, table->power + 1);

                // We've had to grow the table.  Everything has been
                // rehashed, so we have to start again.

                AGHashInsert(table, key, value);
                return;
            }

            table->totalCount++;
        }

        table->count++;

        // We need to call the insert functions since we are putting
        // a new key/value in to the table.

        keyInsertFunc = table->keyCallbacks.insertFunc;
        if (keyInsertFunc != NULL) {
            key = (*keyInsertFunc)(key);
        }

        valueInsertFunc = table->valueCallbacks.insertFunc;
        if (valueInsertFunc != NULL) {
            value = (*valueInsertFunc)(value);
        }
    } else {
        // If we are replacing a key, don't remove the old
        // one if it is the same as the new one.

        oldKey = table->keys[index];
        if (key != oldKey) {
            keyInsertFunc = table->keyCallbacks.insertFunc;
            if (keyInsertFunc != NULL) {
                key = (*keyInsertFunc)(key);
            }

            keyRemoveFunc = table->keyCallbacks.removeFunc;
            if (keyRemoveFunc != NULL) {
                (*keyRemoveFunc)(oldKey);
            }
        }

        oldValue = table->values[index];
        if (value != oldValue) {
            valueInsertFunc = table->valueCallbacks.insertFunc;
            if (valueInsertFunc != NULL) {
                value = (*valueInsertFunc)(value);
            }

            valueRemoveFunc = table->valueCallbacks.removeFunc;
            if (valueRemoveFunc != NULL) {
                (*valueRemoveFunc)(oldValue);
            }
        }
    }

    table->hashCodes[index] = hash;
    table->keys[index] = key;
    table->values[index] = value;
}

ExportFunc void AGHashRemove(AGHashTable *table, void *key)
{
    int32 index, oldHash;
    AGRemoveCallback keyRemoveFunc, valueRemoveFunc;

    // We need to short-circuit here since the data arrays may not have
    // been allocated yet.

    if (table->count == 0) {
        return;
    }

    index = tableIndexFor(table, key, computeHash(table, key));
    oldHash = table->hashCodes[index];

    if (oldHash == HASH_EMPTY || oldHash == HASH_REMOVED) {
        return;
    }

    keyRemoveFunc = table->keyCallbacks.removeFunc;
    if (keyRemoveFunc != NULL) {
        (*keyRemoveFunc)(table->keys[index]);
    }

    valueRemoveFunc = table->valueCallbacks.removeFunc;
    if (valueRemoveFunc != NULL) {
        (*valueRemoveFunc)(table->values[index]);
    }

    table->count--;
    table->hashCodes[index] = HASH_REMOVED;
    table->keys[index] = NULL;
    table->values[index] = NULL;
}

ExportFunc void AGHashRemoveAll(AGHashTable *table)
{
    int32 i, tableSize, oldHash;
    AGRemoveCallback keyRemoveFunc, valueRemoveFunc;

    if (table->count == 0) {
        return;
    }

    tableSize = HASH_TABLE_SIZE(table->power);

    for (i = 0; i < tableSize; i++) {
        oldHash = table->hashCodes[i];
        if (oldHash == HASH_EMPTY || oldHash == HASH_REMOVED) {
            continue;
        }

        keyRemoveFunc = table->keyCallbacks.removeFunc;
        if (keyRemoveFunc != NULL) {
            (*keyRemoveFunc)(table->keys[i]);
        }

        valueRemoveFunc = table->valueCallbacks.removeFunc;
        if (valueRemoveFunc != NULL) {
            (*valueRemoveFunc)(table->values[i]);
        }
    }

    table->count = 0;
    table->totalCount = 0;

    bzero(table->hashCodes, tableSize * sizeof(int32));
    bzero(table->keys, tableSize * sizeof(void *));
    bzero(table->values, tableSize * sizeof(void *));
}

ExportFunc AGHashEnumerator AGHashGetEnumerator(AGHashTable *table)
{
    // Can you believe this is actually usefull?
    return 0;
}

ExportFunc AGBool AGHashNextPair(AGHashTable *table, AGHashEnumerator *e,
                                 void **key, void **value)
{
    int32 i, tableSize, hashCode;

    if (table->count == 0) {
        return FALSE;
    }

    tableSize = HASH_TABLE_SIZE(table->power);

    for (i = *e; i < tableSize; i++) {
        hashCode = table->hashCodes[i];

        if (hashCode != HASH_EMPTY && hashCode != HASH_REMOVED) {
            if (key != NULL) {
                *key = table->keys[i];
            }

            if (value != NULL) {
                *value = table->values[i];
            }

            *e = i + 1;
            return TRUE;
        }
    }

    *e = tableSize;

    if (key != NULL) {
        *key = NULL;
    }

    if (value != NULL) {
        *value = NULL;
    }

    return FALSE;
}

ExportFunc void AGHashGetKeys(AGHashTable *table, AGArray *array)
{
    int32 i, tableSize, hashCode;

    if (table->count == 0) {
        return;
    }

    tableSize = HASH_TABLE_SIZE(table->power);

    for (i = 0; i < tableSize; i++) {
        hashCode = table->hashCodes[i];

        if (hashCode != HASH_EMPTY && hashCode != HASH_REMOVED) {
            AGArrayAppend(array, table->keys[i]);
        }
    }
}

ExportFunc void AGHashGetValues(AGHashTable *table, AGArray *array)
{
    int32 i, tableSize, hashCode;

    if (table->count == 0) {
        return;
    }

    tableSize = HASH_TABLE_SIZE(table->power);

    for (i = 0; i < tableSize; i++) {
        hashCode = table->hashCodes[i];

        if (hashCode != HASH_EMPTY && hashCode != HASH_REMOVED) {
            AGArrayAppend(array, table->values[i]);
        }
    }
}
