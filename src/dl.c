#include "dl.h"
#include <stdlib.h>
#include <string.h>

const char * dlGetErrorString(DLStatus s)
{
    switch (s)
    {
        case dlOk:
            return "Success.";
        case dlErrorNullList:
            return "Dynamic list pointer is NULL.";
        case dlErrorNullData:
            return "Dynamic list's data buffer pointer is NULL.";
        case dlErrorNullArg:
            return "Given argument is NULL.";
        case dlErrorOutOfMemory:
            return "Could not allocate additional memory.";
        case dlErrorIndexOutOfBounds:
            return "Index exceeds list's bounds.";
        case dlErrorInvalidResizeFactor:
            return "Resize factor is not > zero.";
        case dlErrorInvalidCapacity:
            return "Invalid initial capacity.";
        default:
            return "Unknown.";
    }
}

DynamicList *newDynamicList(
    const unsigned int starting_cap,
    const unsigned int element_size)
{
    DynamicList *dl = malloc(sizeof(DynamicList));
    if (!dl) return NULL;

    dl->capacity = starting_cap;
    dl->element_size = element_size;
    dl->data = malloc(starting_cap * element_size);
    if (!dl->data)
    {
        free(dl);
        return NULL;
    }

    return dl;
}

void dlFree(DynamicList *dl)
{
    if (dl == NULL) return;

    free(dl->data);
    free(dl);
}

const void * dlFirst(const DynamicList *dl)
{
    return dl->data;
}

const void * dlLast(const DynamicList *dl)
{
    return dl->data + (dl->element_size * (dl->count - 1));
}

// Read
const void * dlGet(
    const DynamicList    *dl,
    const unsigned int element_index)
{
    if (!dl) return NULL;
    if (!dl->data) return NULL;
    if (element_index >= dl->capacity) return NULL;

    return dl->data + (dl->element_size * element_index);
}

// Write
//

DLStatus dlClear(DynamicList *dl)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;

    memset(dl->data, 0, dl->capacity * dl->element_size);
    dl->count = 0;
    return dlOk;
}

DLStatus dlResize(DynamicList *dl, const float factor)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;
    if (factor <= 0)
        return dlErrorInvalidResizeFactor;

    dl->capacity *= factor;
    char *tmp = realloc(dl->data, dl->capacity * dl->element_size);
    if (tmp == NULL)
        return dlErrorOutOfMemory;

    dl->data = tmp;
    return dlOk;
}

DLStatus dlShrinkToFit(DynamicList *dl)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;

    // Already shrunk bail
    if (dl->count == dl->capacity)
        return dlOk;

    int new_buf_size = dl->element_size * dl->count;
    if (new_buf_size == 0)
        new_buf_size = dl->element_size;

    void *t = realloc(dl->data, new_buf_size);
    if (!t)
        return -1;

    dl->data = t;
    dl->capacity = new_buf_size / dl->element_size;
    return dlOk;
}

DLStatus dlSet(
    DynamicList *dl,
    const unsigned int element_index,
    const void *element)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;
    if (!element) return dlErrorNullArg;
    if (element_index >= dl->capacity)
        return dlErrorIndexOutOfBounds;

    char *dest = dl->data + (dl->element_size * element_index);
    memcpy(dest, element, dl->element_size);

    if (element_index == dl->count)
        dl->count++;
    else if (element_index > dl->count)
        dl->count = element_index + 1;

    return dlOk;
}

DLStatus dlPush(
    DynamicList *dl,
    const void      *element)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;
    if (!element) return dlErrorNullArg;

    if (dl->count >= dl->capacity)
        dlResize(dl, 1.6);

    char *dest = dl->data + (dl->element_size * dl->count);

    memcpy(dest, element, dl->element_size);
    dl->count++;
    return dlOk;
}

void shrinkIfOK(DynamicList *dl)
{
    if ((dl->capacity - dl->count) >= dl-> count * 5)
        dlShrinkToFit(dl);
}

DLStatus dlPop(DynamicList *dl)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;

    if (dl->count == 0)
        return dlOk;

    char *dest = dl->data + (dl->element_size * (dl->count - 1));

    memset(dest, 0, dl->element_size);
    dl->count--;

    shrinkIfOK(dl);
    return dlOk;
}

DLStatus dlRemoveOrdered(
    DynamicList    *dl,
    const unsigned int element_index)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;
    if (element_index >= dl->count)
        return dlErrorIndexOutOfBounds;

    // Pop last element
    if (element_index == dl->count - 1)
    {
        dlPop(dl);
        return dlOk;
    }

    // Move all memory after index "left" one space;
    char *dest = (char *) dlGet(dl, element_index);
    const unsigned int size = (dl->count - element_index - 1) * dl->element_size;

    memmove(dest, dest + dl->element_size, size);
    memset((void *)dlLast(dl), 0, dl->element_size);
    dl->count--;

    shrinkIfOK(dl);
    return dlOk;
}

DLStatus dlRemoveUnordered(
    DynamicList    *dl,
    const unsigned int element_index)
{
    if (!dl) return dlErrorNullList;
    if (!dl->data) return dlErrorNullData;
    if (element_index >= dl->count)
        return dlErrorIndexOutOfBounds;

    // Pop last element
    if (element_index == dl->count - 1)
    {
        dlPop(dl);
        return dlOk;
    }

    // Copy last element to given index and remove duplicate
    char *dest = dl->data + (element_index * dl->element_size);
    char *last = (char*)dlLast(dl);

    memcpy(dest, last, dl->element_size);
    memset(last, 0, dl->element_size);
    dl->count--;

    shrinkIfOK(dl);
    return dlOk;
}

// Traversal
//

DLStatus dlResetIterator(DynamicList *dl)
{
    if (!dl) return dlErrorNullList;
    dl->iterator = 0;
    return dlOk;
}

int dlHasNext(const DynamicList *dl)
{
    if (!dl) return 0;
    return dl->iterator < dl->count;
}

const void * dlNext(DynamicList *dl)
{
    if (!dlHasNext(dl)) return NULL;

    const void* element = dlGet(dl, dl->iterator);
    dl->iterator++;
    return element;
}
