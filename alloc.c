/**
 * Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _metadata_t {
  unsigned int size;
  unsigned char isUsed;
  struct _metadata_t *next;
} metadata_t;

void *startOfHeap = NULL;

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
  // implement calloc:
  void *ptr = malloc(num * size);
  memset(ptr, 0, num * size);
  return ptr;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
  // implement malloc:

  if (startOfHeap == NULL) {
    startOfHeap = sbrk(0);
  }

  if (startOfHeap != sbrk(0)) {
    metadata_t *curMeta = startOfHeap;
    metadata_t *prev = startOfHeap;
    while(curMeta != NULL) {
      if (curMeta->size > size && curMeta->size > size + sizeof(metadata_t) && curMeta->isUsed == 0) {
        metadata_t *split = (void *)curMeta + sizeof(metadata_t) + size;
        split->isUsed = 0;
        split->size = curMeta->size - size - sizeof(metadata_t);
        split->next = curMeta->next;
        curMeta->size = size;
        curMeta->isUsed = 1;
        prev->next = split;
        if (curMeta != startOfHeap) {
          curMeta->next = NULL;
        }
        return (void *)curMeta + sizeof(metadata_t);
      } else if (curMeta->size >= size && curMeta->isUsed == 0) {
        curMeta->isUsed = 1;
        prev->next = curMeta->next;
        if (curMeta != startOfHeap) {
          curMeta->next = NULL;
        }
        return (void *)curMeta + sizeof(metadata_t);
      }
      prev = curMeta;
      curMeta = curMeta->next;
    }
  }

  metadata_t *meta = sbrk(sizeof(metadata_t));
  meta->size = size;
  meta->isUsed = 1;
  meta->next = NULL;

  void *ptr = sbrk(size);

  return ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
  // implement free

  if (ptr == NULL) {
    return;
  }

  metadata_t *meta = ptr - sizeof(metadata_t);

  metadata_t *curMeta = startOfHeap;
  metadata_t *prev = startOfHeap;
  while(curMeta != NULL) {
    if (curMeta->isUsed == 0 && (void *)curMeta + sizeof(metadata_t) + curMeta->size == meta) {
      curMeta->size += (sizeof(metadata_t) + meta->size);
      meta = curMeta;
    } else if (curMeta->isUsed == 0 && (void*)meta + sizeof(metadata_t) + meta->size == curMeta) {
      meta->size += (sizeof(metadata_t) + curMeta->size);
      curMeta->size = 0;
      prev->next = curMeta->next;
      curMeta->next = NULL;
      curMeta = prev;
      if (meta == startOfHeap) {
        break;
      }
    }
    prev = curMeta;
    curMeta = curMeta->next;
  }

  if (meta->isUsed == 1) {
    meta->isUsed = 0;
    metadata_t *start = startOfHeap;
    metadata_t *copy = start->next;
    start->next = meta;
    meta->next = copy;
  }

}


/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
  // implement realloc:
  if (ptr == NULL) {
    return malloc(size);
  }

  if (size == 0) {
    free(ptr);
    return NULL;
  }

  free(ptr);
  void *to_return = malloc(size);
  memcpy(to_return, ptr, size);
  return to_return;
}
