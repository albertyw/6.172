#include<iostream>
#include<cstdlib>
#include<cstring>
#include "allocator_interface.h"
#include "memlib.h"
#include <math.h>
#include <assert.h>

/* All blocks must have a specified minimum alignment. */
#define ALIGNMENT 8

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
  //******************** MATH STUFF *********************//
  /**
   * Round up to the next highest power
   * This rounds up to 2**SIZE_T_SIZE
   **/
  size_t roundPowUp(size_t num){                                     // See http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    num--;
    for(uint64_t power=1; power<SIZE_T_SIZE; power*=2){
      num |= num >> power;
    }
    num++;
    assert((num >> (uint64_t)log2(num)) == 1);
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
    return 2 << (num-1);
    //return pow(2, num);                                          //TODO: FIND A BITHACK
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
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    size_t ** temp = (size_t**)mem_heap_lo() + (binNum-3);
    assert(temp >= (size_t **)mem_heap_lo());
    assert(temp < (size_t **)getHeapPointer());
    
    return temp;
  }
  /**
   * Set the pointer in a bin (i.e. the value of the bin) to a given pointer
   **/
  void allocator::setBinPointer(uint8_t binNum, size_t *setPointer){
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    assert(setPointer >= getHeapPointer() || setPointer == 0);
    assert(setPointer <= (size_t *)mem_heap_hi());
    size_t **binPointer = getBinPointer(binNum);
    *binPointer = setPointer;
    assert(*getBinPointer(binNum) == setPointer);
  }
  /**
   * Set the value of blockPointer to pointerValue
   **/
  void allocator::setBlockPointer(size_t *blockPointer, size_t *pointerValue){
    assert(blockPointer >= getHeapPointer());
    assert(blockPointer <= (size_t *)mem_heap_hi());
    assert(pointerValue >= getHeapPointer() || pointerValue == 0);
    assert(pointerValue <= (size_t *)mem_heap_hi());
    size_t **pointer = (size_t **)blockPointer;
    *pointer = pointerValue;
  }
  /**
   * Returns a pointer to the beginning of the public heap (after the bin pointers)
   **/
  size_t * allocator::getHeapPointer(){
    assert(sizeAddBytes((size_t*)mem_heap_lo(), (uint64_t)PRIVATE_SIZE)<= (size_t *)mem_heap_hi());
    return sizeAddBytes((size_t*)mem_heap_lo(), (uint64_t)PRIVATE_SIZE);
  }
  /**
   * Returns a pointer moved by some number of bytes
   **/
  size_t * allocator::sizeAddBytes(size_t *pointer, uint64_t bytes){
    assert(bytes >= 8);
    assert(bytes%SIZE_T_SIZE == 0);
    assert(pointer >= mem_heap_lo());
    assert(pointer <= mem_heap_hi());
    assert((size_t*)((char*)pointer + bytes) >= mem_heap_lo());
    assert((char*)pointer + bytes <= (char*)mem_heap_hi()+1);
    return (size_t*)((char*)pointer + bytes);
  }
  
  
  //*************************** HEAP HELPER FUNCTIONS ************************//
  /**
   * Increase the heap size then add the extra amount to a bin
   * size should be max of current heap size and the size needed by malloc
   * size should be a power of 2 already
   * Return the binNum that the extra block was added to
   **/
  uint8_t allocator::increaseHeapSize(size_t size)
  {
    assert(size == roundPowUp(size));
    size = max(size, mem_heapsize());
    uint8_t binNum = log2(size);
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    assert(*getBinPointer(binNum)==0);
    void *newMemPointer = mem_sbrk(size);
    assert((size_t *)newMemPointer >= getHeapPointer());
    assert((size_t *)newMemPointer <= (size_t *)mem_heap_hi());
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
  void allocator::splitBlock(uint8_t largerBinNum, uint8_t smallerBinNum)
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
    size_t *endBlock = sizeAddBytes(pointerInBlock, (uint64_t)biggerSize);
    assert(endBlock >= getHeapPointer());
    setBinPointer(smallerBinNum, pointerInBlock);
    // WHILE CURRENTSIZE IS LESS THAN BIGGERSIZE/2
    pointerInBlock = sizeAddBytes(pointerInBlock, (uint64_t)smallerSize);
    uint8_t currentBin = smallerBinNum;
    //printf("%p -- Pointer In Block\n",pointerInBlock);
    for(size_t currentSize = smallerSize; currentSize < biggerSize; currentSize *= 2){
      assert(currentSize == pow2(currentBin));
      assert(pointerInBlock <= (size_t *)mem_heap_hi());
      assert(pointerInBlock >= getHeapPointer());
      // COPY THE BIN'S POINTER TO THE BLOCK
      setBlockPointer(pointerInBlock, *getBinPointer(currentBin));
      // CHANGE THE BIN'S POINTER TO POINT TO THE BLOCK
      setBinPointer(currentBin, pointerInBlock);
      // Change values for next iteration
      pointerInBlock = sizeAddBytes(pointerInBlock, (uint64_t)currentSize);
      currentBin++;
    }
    assert(currentBin == largerBinNum);   // Make sure we've split up the whole block
    assert(endBlock == pointerInBlock);
    setBinPointer(largerBinNum, 0);
  }
  
  /**
   * Given a pointer to a block where the value stored in the block is also a pointer,
   * return that value as a pointer
   **/
  size_t * allocator::nextBlock(size_t *blockPointer){
    // Same as:
    // size_t blockValue = *blockPointer;
    // return (size_t *)blockValue;
    assert((size_t *)*blockPointer <= (size_t *)mem_heap_hi());
    assert((size_t *)*blockPointer >= getHeapPointer());
    return (size_t *)*blockPointer;
  }
  
  //********************** HEAP CONSISTENCY CHECKING *****************//
  /**
   * check heap consistency
   **/
  int allocator::check()
  {
    /*// CHECK THAT EVERY FREE BLOCK HAS A POINTER WITHIN THE HEAP OR IS 0
    size_t *minPointer = sizeAddBytes((size_t*)mem_heap_lo(), (uint64_t)PRIVATE_SIZE);;
    size_t *maxPointer = (size_t *)mem_heap_hi();
    // For every in
    for(uint8_t i = BIN_MIN; i <= BIN_MAX; i++){
      size_t *blockPointer = *getBinPointer(i);
      // For every block
      while(blockPointer != 0){
        // Check that the pointer in the block does not point to somewhere wrong
        if(blockPointer != 0 && (blockPointer<minPointer || blockPointer>maxPointer)){
          printf("Free Block has a pointer pointing outside of range");
          return -1;
        }
        blockPointer = nextBlock(blockPointer);
      }
    }
    
    // CHECK THAT THERE ARE NO CONTIGUOUS COALESCEABLE BLOCKS
    // For every bin
    for(uint8_t i = BIN_MIN; i <= BIN_MAX; i++){
      size_t block_size = pow2(i);
      size_t *blockPointer = *getBinPointer(i);
      // Compare every two blocks
      while(*blockPointer != 0){
        size_t *block2Pointer = *getBinPointer(i);
        while(*block2Pointer != 0){
          // And check whether the blocks are adjacent
          if(blockPointer + block_size == block2Pointer || blockPointer - block_size == block2Pointer){
            printf("Contiguous Coalescable block found in bin %i", i);
            return -1;
          }
          block2Pointer = nextBlock(block2Pointer);
        }
        blockPointer = nextBlock(blockPointer);
      }
    }
    
    //  CHECK THAT THERE ARE NO OVERLAPPING ALLOCATED BLOCKS
    
    
    // Is every block in the free list marked as free?
    // Is every free block actually in the free list?
    
    */
    return 0;
  }
  
  //******************** INIT ***************************//
  /**
   * init - Initialize the malloc package.  Called once before any other
   * calls are made.  Since this is a very simple implementation, we just
   * return success.
   */
  int allocator::init()
  {
    assert(HEAP_SIZE%8 == 0);
    assert(PRIVATE_SIZE%8 == 0);
    // ALLOCATE A STARTING HEAP
    size_t *p = (size_t *)mem_sbrk(HEAP_SIZE + PRIVATE_SIZE);
    if (p == (void *)-1) {
      /* Whoops, an error of some sort occurred.  We return NULL to let
         the client code know that we weren't able to allocate memory. */
      return -1;
    }
    assert(p != (void *)-1);
    assert(((char *)mem_heap_hi() - (char *)mem_heap_lo())+1  == (HEAP_SIZE + PRIVATE_SIZE));
    
    // MAKE SURE THAT ALL POINTERS ARE SET TO 0
    memset(p,0,(HEAP_SIZE+PRIVATE_SIZE));
    assert(*p == 0);
    assert(*(sizeAddBytes(p, (uint64_t)PRIVATE_SIZE)) == 0);
    
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
    size_t my_aligned_size = roundPowUp(ALIGN(size) + ALIGN(SIZE_T_SIZE));
    assert(size <= (my_aligned_size-8));
    assert(my_aligned_size%8 == 0);
    // FIND THE BIN (ROUND UP LG(SIZE))
    uint8_t binAllocateNum = log2(my_aligned_size);
    size_t **binPtr = getBinPointer(binAllocateNum);
    assert(binAllocateNum >= BIN_MIN);
    assert(binAllocateNum <= BIN_MAX);
    assert(binPtr < (size_t **)getHeapPointer());
    assert(binPtr >= (size_t **)mem_heap_lo());
    // IF BIN IS EMPTY
    if(*binPtr == 0){                                     //TODO: USE A GLOBAL VARIABLE TO SAVE THE HIGHEST BIN NUMBER
      // SEARCH LARGER BINS FOR BLOCKS
      uint8_t binToBreakNum;
      for(binToBreakNum = binAllocateNum+1; binToBreakNum <= BIN_MAX; binToBreakNum++){
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
      // ASSERT THAT THE BIN WE JUST ADDED TO HAS 2 BLOCKS
      assert(*getBinPointer(binAllocateNum)!=0); //Assert that the bin is not empty
      assert(nextBlock(*getBinPointer(binAllocateNum))!=0); // Assert that the bin points to something
      assert(nextBlock(nextBlock(*getBinPointer(binAllocateNum)))==0); // Assert that the second block doesn't point to anything
    }
    // ASSERT BIN IS NOT EMPTY
    assert(*getBinPointer(binAllocateNum)!=0);
    // REMOVE BLOCK POINTER FROM BIN
    size_t * returnBlock = *getBinPointer(binAllocateNum);
    assert(returnBlock >= getHeapPointer());
    assert(returnBlock <= (size_t *)mem_heap_hi());
    //printf("%p %p ------- BLOCK SWAP\n",returnBlock,(size_t *)*returnBlock);
    setBinPointer(binAllocateNum, nextBlock(returnBlock));
    // RECORD THE SIZE INTO THE RETURNBLOCK
    *returnBlock = my_aligned_size;
    //printf("%lu %p --- BLOCKS \n",*returnBlock,returnBlock);
    assert(*returnBlock!=0);
    returnBlock = sizeAddBytes(returnBlock, (uint64_t)SIZE_T_SIZE);
    // RETURN BLOCK POINTER
    assert(returnBlock >= getHeapPointer());
    assert(returnBlock <= (size_t *)mem_heap_hi());
    return returnBlock;
  }
  
  
  //******************** FREE ***************************//
  /*
   * free - Freeing a block does nothing.
   */
  void allocator::free(void *ptr)
  {
    // DON'T DO ANYTHING FOR NULL POINTERS
    if(ptr == NULL) return;
    assert(ptr >= getHeapPointer());
    assert((size_t *)ptr <= (size_t *)mem_heap_hi());
    
    // GO BACK AND FIND THE SIZE
    printf("%p POINTER \n",ptr);
    size_t *blockPointer = (size_t *)ptr-1;
    size_t blockSize = *blockPointer;
    printf("%lu SIZE\n",blockSize);
    assert(roundPowUp(blockSize) == blockSize);
    assert(blockPointer > getHeapPointer());
    assert(blockPointer < (size_t *)mem_heap_hi());
    // FIND BIN THAT BLOCK IS SUPPOSED TO BELONG TO
    uint8_t binNum = log2(blockSize);
    assert(binNum >= BIN_MIN);
    assert(binNum <= BIN_MAX);
    // ERASE VALUES IN PTR
    memset(blockPointer,0,blockSize);
    // FIND IF CONTIGUOUS FREE BLOCKS ARE FREE
    bool coalesceFound = true;
    size_t *binFreeBlockPointer;
    while(coalesceFound == true){
      coalesceFound = false;
      binFreeBlockPointer = *getBinPointer(binNum);
      for(binFreeBlockPointer; binFreeBlockPointer !=0; binFreeBlockPointer = nextBlock(binFreeBlockPointer)){
        if(binFreeBlockPointer == sizeAddBytes(blockPointer, (uint64_t)blockSize)){
          // IF CONTIGUOUS THEN COMBINE FREE BLOCKS
          coalesceFound = true;
          *binFreeBlockPointer = 0;
          blockSize *=2;
          binNum++;
          break;
        }else if(binFreeBlockPointer == (size_t *)((char *)blockPointer+ blockSize)){
          // IF CONTIGUOUS THEN COMBINE FREE BLOCKS
          coalesceFound = true;
          *blockPointer = 0;
          blockPointer = binFreeBlockPointer;
          blockSize *=2;
          binNum++;
          break;
        }
      }
      // Don't need to care about binNum too big because memory wouldn't be able to have already allocated blocks
      assert(binNum < BIN_MAX);
      assert(binNum > BIN_MIN);
    }
    //ADD BLOCK TO BIN
    setBlockPointer(blockPointer, *getBinPointer(binNum));
    setBinPointer(binNum, blockPointer);
  }
  
  
  //******************** REALLOC ***************************//
  /*
   * realloc - Implemented simply in terms of malloc and free
   */
  void * allocator::realloc(void *ptr, size_t size)
  {
    if(size==0){
      free(ptr);
      ptr = 0;
      return ptr;
    }
    if(ptr==0){
      return malloc(size);
    }
    // FIND CURRENT SIZE OF ALLOCATION
    
    // CALCULATE DIFFERENCE IN MEMORY IS NEEDED FOR THE REALLOC
    
    // IF REQUESTED SIZE IS SMALLER
    // CHANGE THE CURRENT SIZE OF ALLOCATION
    // DIVIDE UP THE EXTRA MEMORY INTO BINS
    
    //IF REQUESTED SIZE IS LARGER
    
    // CREATE A POINTER TO END OF ALLOCATION
    
    // WHILE VALUE OF POINTER IS IN A BIN
      // INCREASE THE POINTER TO THE END OF THE UNALLOCATED BLOCK
      
    // IF THE POINTER IS IN AN ALLOCATED AREA
      // MALLOC FULL SIZE
      // MOVE DATA TO THE NEW MALLOC AREA
    // ELSE
      // COMBINE BLOCKS
    
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
