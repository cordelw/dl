#ifndef DYNAMIC_BUF_H
#define DYNAMIC_BUF_H

// TODO: Fix documentation ?
// TODO: Implement find functionality
// TODO: Implement Push range maybe

typedef enum {
    dbErrorOk,
    dbErrorNullParentObject,
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
    unsigned int stride;
    unsigned int count;
    unsigned int capacity;
    unsigned int iterator;
    float resize_factor;
} DynamicBuf;

/* Creates a dynamic list with given initial capacity and element size.
 * Returns NULL if allocation fails. */
DynamicBuf * internal_dbNew(
    const unsigned int initial_capacity,
    const unsigned int stride,
    const float resize_factor);

/* Typesafe constructor macro */
#define dbNew(T, initial_capacity) \
    internal_dbNew(initial_capacity, sizeof(T), 1.6)

/* Typesafe constructor macro with resize factor */
#define dbNewWithResize(T, initial_capacity, resize_factor) \
    internal_dbNew(initial_capacity, sizeof(T), resize_factor)

/* Frees all of a lists associated memory */
void dbFree(DynamicBuf *db);

/* Returns a pointer to first item in buffer */
const void * dbFirst(const DynamicBuf *db);

/* Returns a pointer to last item counted in buffer */
const void * dbLast(const DynamicBuf *db);

/* Returns a pointer to the address of the given item's index*/
const void * internal_dbGet(
    const DynamicBuf    *db,
    const unsigned int index);

/* Typesafe item get macro
 * get an item from buffer of type T at index */
#define dbGet(db, T, index) \
    (T *) internal_dbGet(db, index)

/* Clears the memory in a dynamic lists data buffer and sets
 * the count to 0; Capacity remains untouched.*/
DBError dbClear(DynamicBuf *db);

/* Sets a dynamic buffer's resize factor
 * Error when factor is invalid (<1) */
DBError dbChangeResizeFactor(
    DynamicBuf *db,
    float factor);

/* Resizes a dynamic list's data buffer by the given resize factor.
 * Returns an error if factor is less or equal to zero.
 * Returns an error if no additional memory can be allocated. */
DBError dbResize(DynamicBuf *db);

/* Reallocates the lists data buffer to fit only the counted
 * items */
DBError dbShrinkToFit(DynamicBuf *db);

/* Sets the value of data at the given element index
 * Index can be within the lists capacity, and will adjust the
 * count accordingly. */
DBError dbSet(
    DynamicBuf         *db,
    const unsigned int index,
    const void         *data);

/* Sets the given type of given literal data at given index */
#define dbSetLiteral(db, i, T, data) \
    dbSet(db, i, &((T){data}))

/* Pushes the given element to the next available spot
 * Returns an error if dbResize() fails. */
DBError dbPush(
    DynamicBuf *db,
    const void *data);

/* Pushes the given literal data to nex available spot */
#define dbPushLiteral(db, T, data) \
    dbPush(db, &((T){data}))

/* Removes the last element from a list. */
DBError dbPop(DynamicBuf *db);

/* Removes an element from list without maintaining order. */
DBError dbRemoveUnordered(
    DynamicBuf         *db,
    const unsigned int index);

/* Removes an element from list in while maintaining order. */
DBError dbRemoveOrdered(
    DynamicBuf         *db,
    const unsigned int index);

/* Resets iterator to 0 */
DBError dbResetIterator(DynamicBuf *db);

/* Returns 1 if next element exists, 0 otherwise */
int dbHasNext(const DynamicBuf *db);

/* Gets pointer to the next element in iteration */
const void * internal_dbNext(DynamicBuf *db);

/* Typesafe Next() macro
 * Returns next item from buffer of type*/
#define dbNext(db, T) \
    (T *) internal_dbNext(db)

#endif
