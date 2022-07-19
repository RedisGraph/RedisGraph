#ifndef HEAP_H
#define HEAP_H
#include <stdlib.h>

typedef struct heap_s heap_t;

// heap element compare function signature
typedef int (*heap_cmp)(const void *, const void *, void *udata);

/**
 * Create new heap and initialise it.
 *
 * malloc()s space for heap.
 *
 * @param[in] cmp Callback used to get an item's priority
 * @param[in] udata User data passed through to cmp callback
 * @return initialised heap */
heap_t *Heap_new(heap_cmp cmp, void *udata);

/**
 * Initialise heap. Use memory passed by user.
 *
 * No malloc()s are performed.
 *
 * @param[in] cmp Callback used to get an item's priority
 * @param[in] udata User data passed through to cmp callback
 * @param[in] size Initial size of the heap's array */
void Heap_init(heap_t* h, heap_cmp cmp, void *udata, unsigned int size);

void Heap_free(heap_t * hp);

/**
 * Add item
 *
 * Ensures that the data structure can hold the item.
 *
 * NOTE:
 *  realloc() possibly called.
 *  The heap pointer will be changed if the heap needs to be enlarged.
 *
 * @param[in/out] hp_ptr Pointer to the heap. Changed when heap is enlarged.
 * @param[in] item The item to be added
 * @return 0 on success; -1 on failure */
int Heap_offer(heap_t **hp_ptr, void *item);

/**
 * Add item
 *
 * An error will occur if there isn't enough space for this item.
 *
 * NOTE:
 *  no malloc()s called.
 *
 * @param[in] item The item to be added
 * @return 0 on success; -1 on error */
int Heap_offerx(heap_t * hp, void *item);

/**
 * Remove the item with the top priority
 *
 * @return top item */
void *Heap_poll(heap_t * hp);

/**
 * @return top item of the heap */
void *Heap_peek(const heap_t * hp);

/**
 * Clear all items
 *
 * NOTE:
 *  Does not free items.
 *  Only use if item memory is managed outside of heap */
void Heap_clear(heap_t * hp);

/**
 * @return number of items in heap */
int Heap_count(const heap_t * hp);

/**
 * @return size of array */
int Heap_size(const heap_t * hp);

/**
 * @return number of bytes needed for a heap of this size. */
size_t Heap_sizeof(unsigned int size);

/**
 * Remove item
 *
 * @param[in] item The item that is to be removed
 * @return item to be removed; NULL if item does not exist */
void *Heap_remove_item(heap_t * hp, const void *item);

/**
 * Test membership of item
 *
 * @param[in] item The item to test
 * @return 1 if the heap contains this item; otherwise 0 */
int Heap_contains_item(const heap_t * hp, const void *item);

#endif /* HEAP_H */
