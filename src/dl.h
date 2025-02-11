#ifndef DL_H
#define DL_H

typedef enum {
    dlOk,
    dlErrorNullList,
    dlErrorNullData,
    dlErrorNullArg,
    dlErrorOutOfMemory,
    dlErrorIndexOutOfBounds,
    dlErrorInvalidResizeFactor,
    dlErrorInvalidCapacity,
} DLStatus;

/* Returns a string describing the given error status */
const char * dlGetErrorString(DLStatus s);

typedef struct {
    char *data;
    unsigned int element_size;
    unsigned int count;
    unsigned int capacity;
    unsigned int iterator;
} DynamicList;

/* Creates a dynamic list with given initial capacity and element size.
 * Returns NULL if allocation fails. */
DynamicList *newDynamicList(
    const unsigned int initial_capacity,
    const unsigned int element_size);

/* Typesafe constructor macro */
#define dlNew(T, initial_capacity) \
    newDynamicList(initial_capacity, sizeof(T))

/* Frees all of a lists associated memory */
void dlFree(DynamicList *dl);

/* Returns a pointer to first element in buffer */
const void * dlFirst(const DynamicList *dl);

/* Returns a pointer to last element counted in buffer */
const void * dlLast(const DynamicList *dl);

/* Returns a pointer to the address of the given element index*/
const void * dlGet(
    const DynamicList    *dl,
    const unsigned int element_index);


/* Clears the memory in a dynamic lists data buffer and sets
 * the count to 0; Capacity remains untouched.*/
DLStatus dlClear(DynamicList *dl);

/* Resizes a dynamic list's data buffer by the given resize factor.
 * Returns an error if factor is less or equal to zero.
 * Returns an error if no additional memory can be allocated. */
DLStatus dlResize(DynamicList *dl, const float factor);

/* Reallocates the lists data buffer to fit only the counted
 * items */
DLStatus dlShrinkToFit(DynamicList *dl);

/* Sets the value of data at the given element index
 * Index can be within the lists capacity, and will adjust the
 * count accordingly. */
DLStatus dlSet(
    DynamicList *dl,
    const unsigned int element_index,
    const void *element);

/* Pushes the given element to the next available spot
 * Returns an error if dlResize() fails. */
DLStatus dlPush(
    DynamicList *dl,
    const void      *element);

/* Removes the last element from a list. */
DLStatus dlPop(DynamicList *dl);

/* Removes an element from list without maintaining order. */
DLStatus dlRemoveUnordered(
    DynamicList    *dl,
    const unsigned int element_index);

/* Removes an element from list in while maintaining order. */
DLStatus dlRemoveOrdered(
    DynamicList    *dl,
    const unsigned int element_index);

/* Resets iterator to 0 */
DLStatus dlResetIterator(DynamicList *dl);

/* Returns 1 if next element exists, 0 otherwise */
int dlHasNext(const DynamicList *dl);

/* Gets pointer to the next element in iteration */
const void * dlNext(DynamicList *dl);

#endif
