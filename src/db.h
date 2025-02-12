#ifndef DYNAMIC_BUFFER_H
#define DYNAMIC_BUFFER_H

typedef enum {
    dbErrorOk,
    dbErrorNullBufferObject,
    dbErrorNullBufferData,
    dbErrorNullArgument,
    dbErrorOutOfMemory,
    dbErrorIndexOutOfBounds,
    dbErrorInvalidResizeFactor,
    dbErrorInvalidCapacity,
} DBError;

/* Returns a string describing the given error status */
const char * dbGetErrorString(DBError s);

typedef struct {
    char *data_buffer;
    unsigned int data_size;
    unsigned int count;
    unsigned int capacity;
    unsigned int iterator;
} DynamicBuffer;

/* Creates a dynamic list with given initial capacity and element size.
 * Returns NULL if allocation fails. */
DynamicBuffer * internal_dbNew(
    const unsigned int initial_capacity,
    const unsigned int data_size);

/* Typesafe constructor macro */
#define dbNew(T, initial_capacity) \
    internal_dbNew(initial_capacity, sizeof(T))

/* Frees all of a lists associated memory */
void dbFree(DynamicBuffer *db);

/* Returns a pointer to first item in buffer */
const void * dbFirst(const DynamicBuffer *db);

/* Returns a pointer to last item counted in buffer */
const void * dbLast(const DynamicBuffer *db);

/* Returns a pointer to the address of the given item's index*/
const void * internal_dbGet(
    const DynamicBuffer    *db,
    const unsigned int index);

/* Typesafe item get macro
 * get an item from buffer of type T at index */
#define dbGet(db, T, index) \
    (T*)internal_dbGet(db, index)

/* Clears the memory in a dynamic lists data buffer and sets
 * the count to 0; Capacity remains untouched.*/
DBError dbClear(DynamicBuffer *db);

/* Resizes a dynamic list's data buffer by the given resize factor.
 * Returns an error if factor is less or equal to zero.
 * Returns an error if no additional memory can be allocated. */
DBError dbResize(DynamicBuffer *db, const float factor);

/* Reallocates the lists data buffer to fit only the counted
 * items */
DBError dbShrinkToFit(DynamicBuffer *db);

/* Sets the value of data at the given element index
 * Index can be within the lists capacity, and will adjust the
 * count accordingly. */
DBError dbSet(
    DynamicBuffer      *db,
    const unsigned int index,
    const void         *data);

/* Pushes the given element to the next available spot
 * Returns an error if dbResize() fails. */
DBError dbPush(
    DynamicBuffer *db,
    const void    *data);


#define dbPushLiteral(db, T, data) \
    dbPush(db, &((T){data}))

/* Removes the last element from a list. */
DBError dbPop(DynamicBuffer *db);

/* Removes an element from list without maintaining order. */
DBError dbRemoveUnordered(
    DynamicBuffer    *db,
    const unsigned int index);

/* Removes an element from list in while maintaining order. */
DBError dbRemoveOrdered(
    DynamicBuffer    *db,
    const unsigned int index);

/* Resets iterator to 0 */
DBError dbResetIterator(DynamicBuffer *db);

/* Returns 1 if next element exists, 0 otherwise */
int dbHasNext(const DynamicBuffer *db);

/* Gets pointer to the next element in iteration */
const void * internal_dbNext(DynamicBuffer *db);

/* Typesafe Next() macro
 * Returns next item from buffer of type*/
#define dbNext(db, T) \
    (T *) internal_dbNext(db)

#endif
