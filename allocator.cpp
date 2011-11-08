#include<iostream>
#include<cstdlib>
#include<cstring>
#include "allocator_interface.h"
#include "memlib.h"

#include <math.h>
#include <assert.h>



/* All blocks must have a specified minimum alignment. */
#define ALIGNMENT 8

/* Starting size of the heap */

/* Starting size of the private heap area */
#define BIN_MIN 3
#define BIN_MAX 36            // The highest number bin
#define NUMBER_OF_BINS (BIN_MAX-BIN_MIN)                               //TODO: OPTIMIZE 33; using 33 because we're not using bins 0,1,2
#define PRIVATE_SIZE (ALIGNMENT*NUMBER_OF_BINS)                      
#define HEAP_SIZE (ALIGNMENT*1024)                          //TODO: OPTIMIZE 1024
// HEAP_SIZE and PRIVATE_SIZE should be a power of 2

/* Rounds up to the nearest multiple of ALIGNMENT. */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* The smallest aligned size that will hold a size_t value. */
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

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
  
  //******************** MATH STUFF *********************//
  /**
   * Round up to the next highest power
   * This rounds up to 
   **/
  size_t roundPowUp(size_t num){                                     // See http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    num--;                                                       //TODO: INCREASE THE MAX POWER THAT IT ROUNDS UP TO
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
  }
   
  /**
   * Find the ceiling for the log of num
   **/
  size_t log2(size_t num){
    return ceil(log(num)/log(2));                                       //TODO: FIND A BITHACK
  }
  /**
   * Find 2^num
   **/
  size_t pow2(size_t num){
    return pow(2, num);                                          //TODO: FIND A BITHACK
  }
  
  
  //********************* POINTER MANIPULATIONS ****************//
  /**
   * Returns a pointer to the bin number specified
   * binNumber is based on size of the blocks, not based on distance from mem_heap_lo
   * getBinPointer is a pointer to a bin
   * *getBinPointer is the pointer from the bin to the heap
   * **getBinPointer is the value in the heap
   **/
  size_t ** allocator::getBinPointer(uint8_t binNum){
    size_t ** temp = (size_t**)mem_heap_lo() + (binNum-3);
    //printf("%d %p %p\n", binNum, temp, *temp);
    return (size_t**)mem_heap_lo() + (binNum-3);
  }
  /**
   * Set the pointer in a bin (i.e. the value of the bin) to a given pointer
   **/
  void allocator::setBinPointer(uint8_t binNum, size_t *setPointer){
    size_t **binPointer = getBinPointer(binNum);
    /*if (((size_t)*binPointer < (size_t)0x2000) and (size_t)*binPointer!=0){
      int temp = 1;
    }*/
    *binPointer = setPointer;
    //printf("%d %p\n",binNum,setPointer);
  }
  /**
   * Set the value of blockPointer to pointerValue
   **/
  void allocator::setBlockPointer(size_t *blockPointer, size_t *pointerValue){
    size_t **pointer = (size_t **)blockPointer;
    *pointer = pointerValue;
  }
  /**
   * Returns a pointer to the beginning of the public heap (after the bin pointers)
   **/
  size_t * allocator::getHeapPointer(){
    return (size_t*)mem_heap_lo() + PRIVATE_SIZE;
  }
  
  
  //*************************** HEAP HELPER FUNCTIONS ************************//
  /**
   * Increase the heap size then add the extra amount to a bin
   * size should be max of current heap size and the size needed by malloc
   * size should be a power of 2 already
   * Return a pointer to the block that is allocated
   **/
  uint8_t allocator::increaseHeapSize(size_t size)
  {
    assert(size == roundPowUp(size));
    size = max(size, mem_heapsize());
    uint8_t binNum = log2(size);
    assert(*getBinPointer(binNum)==0);
    void *newMemPointer = mem_sbrk(size);
    setBinPointer(binNum, (size_t *)newMemPointer);
    return binNum;
  }
  
  /**
   * Split a block at pointer with size biggerSize into at least one block of smallerSize
   * This will also change pointers for other free blocks to make sure they are linked
   * biggerSize is number of bytes of the big block
   * smallerSize is the number of bytes of the block size needed
   * both sizes should be a power of 2
   * return a pointer to the block size needed
   **/
  void allocator::splitBlock(uint8_t largerBinNum,uint8_t smallerBinNum)
  {
    assert(largerBinNum >= BIN_MIN);
    assert(largerBinNum <= BIN_MAX);
    assert(smallerBinNum >= BIN_MIN);
    assert(smallerBinNum <= BIN_MAX);
    assert(largerBinNum >= smallerBinNum);
    size_t smallerSize = pow2(smallerBinNum);
    size_t biggerSize = pow2(largerBinNum);
    
    // MAKE THE REQUIRED BLOCK
    size_t *pointerInBlock = *getBinPointer(largerBinNum);
    size_t *returnPointer = pointerInBlock;
    pointerInBlock += smallerSize;
    // WHILE CURRENTSIZE IS LESS THAN BIGGERSIZE/2
    size_t currentSize = smallerSize;
    uint8_t currentBin = smallerBinNum;
    printf("%p -- Pointer In Block\n",pointerInBlock);
    for(currentSize; currentSize < biggerSize; currentSize *= 2){
      assert(currentSize == pow2(currentBin));
      // COPY THE BIN'S POINTER TO THE BLOCK
      setBlockPointer(pointerInBlock, *getBinPointer(currentBin));
      // CHANGE THE BIN'S POINTER TO POINT TO THE BLOCK
      setBinPointer(currentBin, pointerInBlock);
      // Change values for next iteration
      pointerInBlock = (size_t *)((char *) pointerInBlock + currentSize);
      currentBin++;
    }
    printf("DONE ----\n");
    setBinPointer(largerBinNum, 0);
  }
  
  //******************** INIT ***************************//
  /**
   * init - Initialize the malloc package.  Called once before any other
   * calls are made.  Since this is a very simple implementation, we just
   * return success.
   */
  int allocator::init()
  {
    // ALLOCATE A STARTING HEAP
    size_t *p = (size_t *)mem_sbrk(HEAP_SIZE + PRIVATE_SIZE);
    memset(p,0,HEAP_SIZE+PRIVATE_SIZE);
    if (p == (void *)-1) {
      /* Whoops, an error of some sort occurred.  We return NULL to let
         the client code know that we weren't able to allocate memory. */
      return -1;
    }
    // MAKE SURE THAT ALL POINTERS IN THE PRIVATE AREA IS SET TO 0
    for(uint8_t i=BIN_MIN; i<BIN_MAX; i++){
      setBinPointer(i, 0);
    }
    // ALLOCATE A PART OF THE HEAP TO MEMORIZE EMPTY BIN'S BLOCKS
    setBinPointer(log2(HEAP_SIZE), getHeapPointer());
    
    return 0;
  }
  
  
  //******************** MALLOC ***************************//
  /**
   * malloc - Allocate a block by incrementing the brk pointer.
   *     Always allocate a block whose size is a multiple of the alignment.
   */
  void * allocator::malloc(size_t size)
  {
    // Make sure that we're aligned to 8 byte boundaries
    int alig = ALIGN(size);
    int alig1 = ALIGN(SIZE_T_SIZE);
    size_t my_aligned_size = roundPowUp(ALIGN(size) + ALIGN(SIZE_T_SIZE));
    // FIND THE BIN (ROUND UP LG(SIZE))
    uint8_t binAllocateNum = log2(my_aligned_size);
    size_t **binPtr = getBinPointer(binAllocateNum);
    // IF BIN IS EMPTY
    void* mem_low = mem_heap_lo();
    if(*binPtr == 0){                                     //TODO: USE A GLOBAL VARIABLE TO SAVE THE HIGHEST BIN NUMBER
      // SEARCH LARGER BINS FOR BLOCKS
      uint8_t binToBreakNum;
      for(binToBreakNum = binAllocateNum+1; binToBreakNum < BIN_MAX; binToBreakNum++){
        if(*getBinPointer(binToBreakNum) != 0) break;
      }
      // IF NO BLOCKS FOUND, ALLOCATE MORE MEMORY FOR THE HEAP
      if(*getBinPointer(binToBreakNum) == 0){
        binToBreakNum = increaseHeapSize(my_aligned_size);
      }
      // SPLIT BLOCK UP INTO SMALLER BINS
      splitBlock(binToBreakNum, binAllocateNum);
      // ASSERT THAT THE BIN WE JUST BROKE UP IS EMPTY
      assert(*getBinPointer(binToBreakNum)==0);
    }
    // ASSERT BIN IS NOT EMPTY
    assert(*getBinPointer(binAllocateNum)!=0);
    // REMOVE BLOCK POINTER FROM BIN
    size_t * returnBlock = (size_t *)*getBinPointer(binAllocateNum);
    printf("aaaaaaaaa\n");
    setBinPointer(binAllocateNum, (size_t *)*returnBlock);
    // RECORD THE SIZE INTO THE RETURNBLOCK
    *returnBlock = my_aligned_size;
    returnBlock += 1;
    // RETURN BLOCK POINTER
    return returnBlock;
    
    
  }
  
  
  //******************** FREE ***************************//
  /*
   * free - Freeing a block does nothing.
   */
  void allocator::free(void *ptr)
  {
    // FIND BIN THAT BLOCK IS SUPPOSED TO BELONG TO
    
    // FIND IF CONTIGUOUS FREE BLOCKS ARE FREE
    
      // IF CONTIGUOUS THEN COMBINE FREE BLOCKS
  }

  
  //******************** REALLOC ***************************//
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
