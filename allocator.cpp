#include<iostream>
#include<cstdlib>
#include<cstring>
#include "allocator_interface.h"
#include "memlib.h"
#include <math.h>



/* All blocks must have a specified minimum alignment. */
#define ALIGNMENT 8

/* Starting size of the heap */

/* Starting size of the private heap area */
#define PRIVATE_SIZE (ALIGNMENT*(36-3))                      //TODO: OPTIMIZE 33; using 33 because we're not using bins 0,1,2
#define STARTING_SIZE (ALIGNMENT*1024 + PRIVATE_SIZE)        //TODO: OPTIMIZE 1024
// STARTING_SIZE and PRIVATE_SIZE should be a power of 2

/* Rounds up to the nearest multiple of ALIGNMENT. */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* The smallest aligned size that will hold a size_t value. */
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

namespace my
{
  /*
   * check - This checks our invariant that the size_t header before every
   * block points to either the beginning of the next block, or the end of the
   * heap.
   */
  int allocator::check()
  {
    char *p;
    char *lo = (char*)mem_heap_lo();
    char *hi = (char*)mem_heap_hi() + 1;
    size_t size = 0;

    p = lo;
    while (lo <= p && p < hi) {
      size = ALIGN(*(size_t*)p + SIZE_T_SIZE);
      p += size;
    }

    if (p != hi) {
      printf("Bad headers did not end at heap_hi!\n");
      printf("heap_lo: %p, heap_hi: %p, size: %lu, p: %p\n", lo, hi, size, p);
      return -1;
    }

    return 0;
  }

  /**
   * init - Initialize the malloc package.  Called once before any other
   * calls are made.  Since this is a very simple implementation, we just
   * return success.
   */
  int allocator::init()
  {
    // ALLOCATE A STARTING HEAP OF SIZE STARTING_SIZE
    size_t *p = (size_t *)mem_sbrk(STARTING_SIZE);
    
    if (p == (void *)-1) {
      /* Whoops, an error of some sort occurred.  We return NULL to let
         the client code know that we weren't able to allocate memory. */
      return -1;
    }
    size_t *startingPosition = (size_t*)mem_heap_lo();
    for(size_t *i = startingPosition; i<(PRIVATE_SIZE + startingPosition); i+=ALIGNMENT){
      *i = 0;
    }
    
    // ALLOCATE A PART OF THE HEAP TO MEMORIZE EMPTY BIN'S BLOCKS
    size_t *bin13 = p+10*ALIGNMENT;                         //TODO: Change 10 to whatever the bin of the starting size is
    *bin13 = (size_t)(p + PRIVATE_SIZE);
    
    
    return 0;
  }
  
  /**
   * Increase the heap size then add the extra amount to a bin
   * size should be max of current heap size and the size needed by malloc
   * Return a pointer to the block that is allocated
   **/
  void * allocator::increaseHeapSize(size_t size)
  {
    size = max(size, mem_heapsize());
    return mem_sbrk(size);
  }
  
  /**
   * Split a block at pointer with size biggerSize into at least one block of smallerSize
   * This will also change pointers for other free blocks to make sure they are linked
   * biggerSize is number of bytes of the big block
   * smallerSize is the number of bytes of the block size needed
   * both sizes should be a power of 2
   * return a pointer to the block size needed
   **/
  void * allocator::splitBlock(size_t *pointer, size_t biggerSize, size_t smallerSize)
  {
    /*int a = 3;
    int *b  = &a;
    int c = 5;
    int *d = &c;
    *b = d;*/
    
    // MAKE THE REQUIRED BLOCK
    size_t *returnPointer = pointer;
    pointer += ALIGNMENT * smallerSize;
    size_t currentSize = smallerSize;
    size_t ** binPtr;
    // WHILE CURRENTSIZE IS LESS THAN BIGGERSIZE/2
    for(currentSize; currentSize < (biggerSize/2); currentSize *= 2){
      // FIND THE BIN FOR THE BLOCK
      binPtr = (size_t **)mem_heap_lo() + ((uint8_t)(log2(currentSize))-3) * ALIGNMENT;
      // COPY THE BIN'S POINTER TO THE BLOCK
      *pointer = *binPtr;
      // CHANGE THE BIN'S POINTER TO POINT TO THE BLOCK
      *binPtr = pointer;
    }
    return returnPointer;
  }
  
  /**
   * malloc - Allocate a block by incrementing the brk pointer.
   *     Always allocate a block whose size is a multiple of the alignment.
   */
  void * allocator::malloc(size_t size)
  {
    // Make sure that we're aligned to 8 byte boundaries
    int my_aligned_size = ALIGN(size + SIZE_T_SIZE);
    // FIND THE BIN (ROUND UP LG(SIZE))
    uint8_t bin = (uint8_t)ceil(log2(my_aligned_size));         //TODO: USE A BITHACK
    size_t *binPtr = (size_t *)mem_heap_lo() + bin;
    // IF BIN IS EMPTY
    if(*binPtr == 0){
      // SEARCH LARGER BINS FOR BLOCKS
      size_t *searchBinPtr = binPtr;
      size_t *startingPosition = (size_t*)mem_heap_lo();
      for(*searchBinPtr; searchBinPtr < (startingPosition+PRIVATE_SIZE); *searchBinPtr+= ALIGNMENT){
        if(*searchBinPtr != 0) break;
      }
      // IF NO BLOCKS FOUND, MEM_SBRK
      if(*searchBinPtr == 0){
        searchBinPtr = (size_t *)increaseHeapSize(pow(2,bin));
      }
      // SPLIT BLOCK UP INTO SMALLER BINS
      
    }
    // ASSERT BIN IS NOT EMPTY
    
    // REMOVE BLOCK POINTER FROM BIN
    
    // RETURN BLOCK POINTER
    
    
    
    
    /* We allocate a little bit of extra memory so that we can store the
       size of the block we've allocated.  Take a look at realloc to see
       one example of a place where this can come in handy. */
    int aligned_size = ALIGN(size + SIZE_T_SIZE);

    /* Expands the heap by the given number of bytes and returns a pointer to
       the newly-allocated area.  This is a slow call, so you will want to
       make sure you don't wind up calling it on every malloc. */
    void *p = mem_sbrk(aligned_size);

    if (p == (void *)-1) {
      /* Whoops, an error of some sort occurred.  We return NULL to let
         the client code know that we weren't able to allocate memory. */
      return NULL;
    } else {
      /* We store the size of the block we've allocated in the first
         SIZE_T_SIZE bytes. */
      *(size_t*)p = size;

      /* Then, we return a pointer to the rest of the block of memory,
         which is at least size bytes long.  We have to cast to uint8_t
         before we try any pointer arithmetic because voids have no size
         and so the compiler doesn't know how far to move the pointer.
         Since a uint8_t is always one byte, adding SIZE_T_SIZE after
         casting advances the pointer by SIZE_T_SIZE bytes. */
      return (void *)((char *)p + SIZE_T_SIZE);
    }
  }

  /*
   * free - Freeing a block does nothing.
   */
  void allocator::free(void *ptr)
  {
    // FIND BIN THAT BLOCK IS SUPPOSED TO BELONG TO
    
    // FIND IF CONTIGUOUS FREE BLOCKS ARE FREE
    
      // IF CONTIGUOUS THEN COMBINE FREE BLOCKS
  }

  /*
   * realloc - Implemented simply in terms of malloc and free
   */
  void * allocator::realloc(void *ptr, size_t size)
  {
    void *newptr;
    size_t copy_size;

    /* Allocate a new chunk of memory, and fail if that allocation fails. */
    newptr = malloc(size);
    if (NULL == newptr)
      return NULL;

    /* Get the size of the old block of memory.  Take a peek at malloc(),
       where we stashed this in the SIZE_T_SIZE bytes directly before the
       address we returned.  Now we can back up by that many bytes and read
       the size. */
    copy_size = *(size_t*)((uint8_t*)ptr - SIZE_T_SIZE);

    /* If the new block is smaller than the old one, we have to stop copying
       early so that we don't write off the end of the new block of memory. */
    if (size < copy_size)
      copy_size = size;

    /* This is a standard library call that performs a simple memory copy. */
    std::memcpy(newptr, ptr, copy_size);

    /* Release the old block. */
    free(ptr);

    /* Return a pointer to the new block. */
    return newptr;
  }

  /* call mem_reset_brk. */
  void allocator::reset_brk()
  {
    mem_reset_brk() ;
  }

  /* call mem_heap_lo */
  void * allocator::heap_lo()
  {
    return mem_heap_lo() ;
  }

  /* call mem_heap_hi */
  void * allocator::heap_hi()
  {
    return mem_heap_hi() ;
  }


};
