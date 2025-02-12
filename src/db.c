#include "db.h"
#include <stdlib.h>
#include <string.h>

const char * dbGetErrorString(DBError s)
{
    switch (s)
    {
        case dbErrorOk:
            return "Success.";
        case dbErrorNullBufferObject:
            return "Dynamic list pointer is NULL.";
        case dbErrorNullBufferData:
            return "Dynamic list's data_buffer buffer pointer is NULL.";
        case dbErrorNullArgument:
            return "Given argument is NULL.";
        case dbErrorOutOfMemory:
            return "Could not allocate additional memory.";
        case dbErrorIndexOutOfBounds:
            return "Index exceeds list's bounds.";
        case dbErrorInvalidResizeFactor:
            return "Resize factor is invalid.";
        case dbErrorInvalidCapacity:
            return "Initial capacity is invalid.";
        default:
            return "Unknown.";
    }
}

DynamicBuffer *internal_dbNew(
    const unsigned int starting_cap,
    const unsigned int data_size)
{
    if (starting_cap == 0)
        return NULL;

    DynamicBuffer *db = malloc(sizeof(DynamicBuffer));
    if (!db) return NULL;

    db->capacity = starting_cap;
    db->data_size = data_size;
    db->data_buffer = malloc(starting_cap * data_size);
    if (!db->data_buffer)
    {
        free(db);
        return NULL;
    }

    return db;
}

void dbFree(DynamicBuffer *db)
{
    if (db == NULL) return;

    free(db->data_buffer);
    free(db);
}

const void * dbFirst(const DynamicBuffer *db)
{
    return db->data_buffer;
}

const void * dbLast(const DynamicBuffer *db)
{
    return db->data_buffer + (db->data_size * (db->count - 1));
}

// Read
const void * internal_dbGet(
    const DynamicBuffer *db,
    const unsigned int  index)
{
    if (!db) return NULL;
    if (!db->data_buffer) return NULL;
    if (index >= db->capacity) return NULL;

    return db->data_buffer + (db->data_size * index);
}

// Write
//

DBError dbClear(DynamicBuffer *db)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;

    memset(db->data_buffer, 0, db->capacity * db->data_size);
    db->count = 0;
    return dbErrorOk;
}

DBError dbResize(DynamicBuffer *db, const float factor)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;
    if (factor <= 0)
        return dbErrorInvalidResizeFactor;

    db->capacity *= factor;
    char *tmp = realloc(db->data_buffer, db->capacity * db->data_size);
    if (tmp == NULL)
        return dbErrorOutOfMemory;

    db->data_buffer = tmp;
    return dbErrorOk;
}

DBError dbShrinkToFit(DynamicBuffer *db)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;

    // Already shrunk bail
    if (db->count == db->capacity)
        return dbErrorOk;

    int new_buf_size = db->data_size * db->count;
    if (new_buf_size == 0)
        new_buf_size = db->data_size;

    void *t = realloc(db->data_buffer, new_buf_size);
    if (!t)
        return -1;

    db->data_buffer = t;
    db->capacity = new_buf_size / db->data_size;
    return dbErrorOk;
}

DBError dbSet(
    DynamicBuffer *db,
    const unsigned int index,
    const void *element)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;
    if (!element) return dbErrorNullArgument;
    if (index >= db->capacity)
        return dbErrorIndexOutOfBounds;

    char *dest = db->data_buffer + (db->data_size * index);
    memcpy(dest, element, db->data_size);

    if (index == db->count)
        db->count++;
    else if (index > db->count)
        db->count = index + 1;

    return dbErrorOk;
}

DBError dbPush(
    DynamicBuffer *db,
    const void      *element)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;
    if (!element) return dbErrorNullArgument;

    if (db->count >= db->capacity)
        dbResize(db, 1.6);

    char *dest = db->data_buffer + (db->data_size * db->count);

    memcpy(dest, element, db->data_size);
    db->count++;
    return dbErrorOk;
}

void shrinkIfOK(DynamicBuffer *db)
{
    if ((db->capacity - db->count) >= db-> count * 5)
        dbShrinkToFit(db);
}

DBError dbPop(DynamicBuffer *db)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;

    if (db->count == 0)
        return dbErrorOk;

    char *dest = db->data_buffer + (db->data_size * (db->count - 1));

    memset(dest, 0, db->data_size);
    db->count--;

    shrinkIfOK(db);
    return dbErrorOk;
}

DBError dbRemoveOrdered(
    DynamicBuffer    *db,
    const unsigned int index)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;
    if (index >= db->count)
        return dbErrorIndexOutOfBounds;

    // Pop last element
    if (index == db->count - 1)
    {
        dbPop(db);
        return dbErrorOk;
    }

    // Move all memory after index "left" one space;
    char *dest = dbGet(db, char, index);
    const unsigned int size = (db->count - index - 1) * db->data_size;

    memmove(dest, dest + db->data_size, size);
    memset((void *)dbLast(db), 0, db->data_size);
    db->count--;

    shrinkIfOK(db);
    return dbErrorOk;
}

DBError dbRemoveUnordered(
    DynamicBuffer    *db,
    const unsigned int index)
{
    if (!db) return dbErrorNullBufferObject;
    if (!db->data_buffer) return dbErrorNullBufferData;
    if (index >= db->count)
        return dbErrorIndexOutOfBounds;

    // Pop last element
    if (index == db->count - 1)
    {
        dbPop(db);
        return dbErrorOk;
    }

    // Copy last element to given index and remove duplicate
    char *dest = db->data_buffer + (index * db->data_size);
    char *last = (char*)dbLast(db);

    memcpy(dest, last, db->data_size);
    memset(last, 0, db->data_size);
    db->count--;

    shrinkIfOK(db);
    return dbErrorOk;
}

// Traversal
//

DBError dbResetIterator(DynamicBuffer *db)
{
    if (!db) return dbErrorNullBufferObject;
    db->iterator = 0;
    return dbErrorOk;
}

int dbHasNext(const DynamicBuffer *db)
{
    if (!db) return 0;
    return db->iterator < db->count;
}

const void * internal_dbNext(DynamicBuffer *db)
{
    if (!dbHasNext(db)) return NULL;

    const void* element = dbGet(db, void, db->iterator);
    db->iterator++;
    return element;
}
