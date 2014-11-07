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

#include <AGArray.h>
#include <AGUtil.h>

#define MIN_CAPACITY 8

ExportFunc AGArray *AGArrayNew(AGElementType elemType,
                               int32 initialCapacity)
{
    AGArray *array;

    array = (AGArray *)malloc(sizeof(AGArray));
    return AGArrayInit(array, elemType, initialCapacity);
}

ExportFunc AGArray *AGArrayInit(AGArray *array,
                                AGElementType elemType,
                                int32 initialCapacity)
{
    bzero(array, sizeof(*array));

    if (initialCapacity > 0) {
        AGArrayEnsureCapacity(array, initialCapacity);
    }

    AGCollectionCallbacksInit(&(array->callbacks), elemType);

    return array;
}

ExportFunc void AGArrayFinalize(AGArray *array)
{
    AGArrayRemoveAll(array);
    if(array->elements)
        free(array->elements);
    bzero(array, sizeof(*array));
}

ExportFunc void AGArrayFree(AGArray *array)
{
    AGArrayFinalize(array);
    free(array);
}

ExportFunc void AGArrayEnsureCapacity(AGArray *array, int32 minCapacity)
{
    int32 count, capacity, newCapacity;
    void **newElements;

    capacity = array->capacity;
    if (capacity >= minCapacity) {
        return;
    }

    newCapacity = capacity;
    if (newCapacity < MIN_CAPACITY) {
        newCapacity = MIN_CAPACITY;
    }

    while (newCapacity < minCapacity) {
        newCapacity *= 2;
    }

    newElements = (void**)malloc(newCapacity * sizeof(void *));

    count = array->count;
    if (count > 0) {
        bcopy(array->elements, newElements, count * sizeof(void *));
        free(array->elements);
    }

    bzero(newElements + count, (newCapacity - count) * sizeof(void *));
    array->elements = newElements;
    array->capacity = newCapacity;
}

ExportFunc void AGArraySetCallbacks(AGArray *array,
                                    AGCompareCallback compareFunc,
                                    AGInsertCallback insertFunc,
                                    AGRemoveCallback removeFunc)
{
    array->callbacks.compareFunc = compareFunc;
    array->callbacks.hashFunc = NULL;
    array->callbacks.insertFunc = insertFunc;
    array->callbacks.removeFunc = removeFunc;
}

ExportFunc int32 AGArrayCount(AGArray *array)
{
    if (NULL == array)
        return 0;
    else
        return array->count;
}

ExportFunc void *AGArrayElementAt(AGArray *array, int32 index)
{
    if (index >= array->count) {
        return NULL;
    }

    return array->elements[index];
}

ExportFunc void AGArrayInsertAt(AGArray *array, int32 index, void *elem)
{
    int32 count = array->count;
    void **elements;
    AGInsertCallback insertFunc;

    if (index >= count + 1) {
        return;
    }

    if (count >= array->capacity) {
        AGArrayEnsureCapacity(array, count + 1);
    }

    elements = array->elements;

    if (count - index > 0) {
        bcopy(elements + index, elements + index + 1, (count - index) * sizeof(void *));
    }

    insertFunc = array->callbacks.insertFunc;
    if (insertFunc != NULL) {
        elem = (*insertFunc)(elem);
    }

    elements[index] = elem;
    array->count = count + 1;
}

ExportFunc void AGArrayAppend(AGArray *array, void *elem)
{
    int32 count = array->count;
    AGInsertCallback insertFunc;

    if (count >= array->capacity) {
        AGArrayEnsureCapacity(array, count + 1);
    }

    insertFunc = array->callbacks.insertFunc;
    if (insertFunc != NULL) {
        elem = (*insertFunc)(elem);
    }

    array->elements[count] = elem;
    array->count++;
}

ExportFunc void AGArrayRemoveAt(AGArray *array, int32 index)
{
    int32 count = array->count;
    void **elements;
    AGRemoveCallback removeFunc;

    if (index >= count) {
        return;
    }

    elements = array->elements;

    removeFunc = array->callbacks.removeFunc;
    if (removeFunc != NULL) {
        (*removeFunc)(elements[index]);
    }

    if (count - index - 1 > 0) {
        bcopy(elements + index + 1, elements + index, 
            (count - index - 1) * sizeof(void *));
    }

    count--;
    elements[count] = NULL;
    array->count = count;
}

ExportFunc void AGArrayRemoveAll(AGArray *array)
{
    int32 i, count = array->count;
    void **elements;
    AGRemoveCallback removeFunc;

    if (count <= 0) {
        return;
    }

    elements = array->elements;

    removeFunc = array->callbacks.removeFunc;
    if (removeFunc != NULL) {
        for (i = 0; i < count; i++) {
            (*removeFunc)(elements[i]);
        }
    }

    bzero(elements, count * sizeof(void *));
    array->count = 0;
}

ExportFunc AGBool AGArrayAppendIfAbsent(AGArray *array, void *elem)
{
    int32 index;

    index = AGArrayIndexOf(array, elem, 0);
    if (index < 0) {
        AGArrayAppend(array, elem);
        return TRUE;
    }

    return FALSE;
}

ExportFunc void AGArrayAppendArray(AGArray *array, AGArray *other)
{
    int32 i, count = other->count;
    void **elements;

    elements = other->elements;

    for (i = 0; i < count; i++) {
        AGArrayAppend(array, elements[i]);
    }
}

ExportFunc void AGArrayReplaceAt(AGArray *array, int32 index, void *elem)
{
    int32 count = array->count;
    void **elements;
    AGRemoveCallback removeFunc;
    AGInsertCallback insertFunc;

    if (index >= count) {
        return;
    }

    elements = array->elements;

    // If we are replacing an element with itself, don't
    // invoke the callbacks.

    if (elem != elements[index]) {
        insertFunc = array->callbacks.insertFunc;
        if (insertFunc != NULL) {
            elem = (*insertFunc)(elem);
        }

        removeFunc = array->callbacks.removeFunc;
        if (removeFunc != NULL) {
            (*removeFunc)(elements[index]);
        }
    }

    elements[index] = elem;
}

ExportFunc int32 AGArrayIndexOf(AGArray *array, void *elem, int32 startIndex)
{
    int32 i, count = array->count;
    void **elements;
    AGCompareCallback compareFunc;

    elements = array->elements;
    compareFunc = array->callbacks.compareFunc;

    if (compareFunc != NULL) {
        for (i = startIndex; i < count; i++) {
            if ((*compareFunc)(elem, elements[i]) == 0) {
                return i;
            }
        }
    } else {
        for (i = startIndex; i < count; i++) {
            if (elem == elements[i]) {
                return i;
            }
        }
    }

    return -1;
}

ExportFunc int32 AGArrayLastIndexOf(AGArray *array, void *elem, int32 startIndex)
{
    int32 i, count = array->count;
    void **elements;
    AGCompareCallback compareFunc;

    if (startIndex >= count) {
        return -1;
    }

    elements = array->elements;
    compareFunc = array->callbacks.compareFunc;

    if (compareFunc != NULL) {
        for (i = startIndex; i >= 0; i--) {
            if ((*compareFunc)(elem, elements[i]) == 0) {
                return i;
            }
        }
    } else {
        for (i = startIndex; i >= 0; i--) {
            if (elem == elements[i]) {
                return i;
            }
        }
    }

    return -1;
}

