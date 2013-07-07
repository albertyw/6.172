#include<iostream>
#include<cstdlib>
#include<cstring>
#include "allocator_interface.h"
#include "memlib.h"
#include <math.h>
#include <assert.h>
#include <time.h>

/*
...................________
.................,.-‘”..........``~.,
..............,.-”..................“-.,
.............,/.......................”:,
..........,?...........................\,
........./............................,}
........./..........................,:`^`.}
......../.........................,:”..../
.......?...__....................:`..../
......./__.(...“~-,_..............,:`....../
....../(_..”~,_.....“~,_..........,:`...._/
......{.._$;_...”=,_....“-,_...,.-~-,},.~”;/....}
......((...*~_....”=-._...“;,,./`../”..../...../
.......\`~,....“~.,...........`...}......../
......(...`=-,,....`.............(...;_,,-”
......./.`~,....`-.................\../\
......\`~.*-,...................|,./.....\,__
,,_.....}.>-._\..................|........`=~-,
...`=~-,_\_...`\,.................\
..........`=~-,,.\,................\
................`:,,.............`\........__
...................`=-,..........,%`>--==``
...................._\......_,-%.....`\
*/


/* All blocks must have a specified minimum alignment. */
#define ALIGNMENT 8

/* Starting size of the private heap area */
#define BIN_MIN 3
#define BIN_MAX 36            // The highest number bin
#define NUMBER_OF_BINS (BIN_MAX-BIN_MIN+1)                               //TODO: OPTIMIZE 33; using 33 because we're not using bins 0,1,2
#define PRIVATE_SIZE (SIZE_T_SIZE*NUMBER_OF_BINS + MUTEX_SIZE*NUMBER_OF_BINS)                      
#define HEAP_SIZE (ALIGNMENT*2048)                          //TODO: OPTIMIZE 2048
// HEAP_SIZE and PRIVATE_SIZE should be a power of 2

/* Rounds up to the nearest multiple of ALIGNMENT. */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* The smallest aligned size that will hold a size_t value. */
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define MUTEX_SIZE (ALIGN(sizeof(pthread_mutex_t)))

/* The minimum score for coallescing to happen */
#define COALLESCE_MIN 150

#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )


/*
TODO: __FILE__ __LINE__ __FUNC__
TODO: Use scoped lock
TODO: Check Deadlock detection (add a timeout, only for testing) 
TODO: Use a large lock for making program run in serial for testing
TODO: jemalloc, tcmalloc, 
TODO: Check cache line/boundary
TODO: Create different mutexes for different operations (malloc, free, realloc) 
or for different bin sizes
TODO: Create a lookup table or caching for math functions
*/
namespace my
{
  volatile int allocator::coallesceScore = 0;
  pthread_mutex_t allocator::coallesceScoreMutex;
  size_t *allocator::heapPointer;
  //******************** MATH STUFF *********************//
  /**
   * Round up to the next highest power
   * This rounds up to 2**SIZE_T_SIZE
   **/
  inline size_t roundPowUp(size_t num){
    // See http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num |= num >> 32;
    num++;
    assert((num >> (uint64_t)log2(num)) == 1);
    return num;
  }

  /**
   * From http://aggregate.org/MAGIC/#Population%20Count%20%28Ones%20Count%29
   * 32-bit recursive reduction using SWAR but first step is mapping 2-bit values 
   * into sum of 2 1-bit values in sneaky way
   **/
  inline size_t ones(register size_t x) {
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    x += (x >> 32);
    return(x & 0x0000003f);
  }

  /**
   * Find the ceiling for the log of num  
   **/
  inline uint8_t log2(size_t x){
    // From http://aggregate.org/MAGIC/#Population%20Count%20%28Ones%20Count%29                                       
    register size_t y = (x & (x - 1));
    y |= -y;
    y >>= 63;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return (uint8_t)(ones(x >> 1) - y);
  }
 
  /**
   * Find 2^num
   **/
  inline size_t pow2(size_t num){
    return 2 << (num-1);
  }
  
  //********************* POINTER MANIPULATIONS ****************//
  /**
   * Returns a pointer to the bin number specified
   * binNumber is based on size of the blocks, not based on distance from mem_heap_lo
   * getBinPointer is a pointer to a bin
   * *getBinPointer is the pointer from the bin to the heap
   * **getBinPointer is the value in the heap
   **/
  inline size_t ** allocator::getBinPointer(uint8_t binNum){
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    size_t ** temp = (size_t**)mem_heap_lo() + (binNum-BIN_MIN);
    assert(temp >= (size_t **)mem_heap_lo());
    assert(temp < (size_t **)getHeapPointer());
    return temp;
  }
  
  /**
   * Set the pointer in a bin (i.e. the value of the bin) to a given pointer
   **/
  inline void allocator::setBinPointer(uint8_t binNum, size_t *setPointer){
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    assert(setPointer >= getHeapPointer() || setPointer == 0);
    assert(setPointer <= (size_t *)mem_heap_hi());
    size_t **binPointer = getBinPointer(binNum);
    *binPointer = setPointer;
    assert(*getBinPointer(binNum) == setPointer);
  }
  
  /**
   * Get the lock on a bin
   **/
   inline pthread_mutex_t* allocator::getBinLock(uint8_t binNum){
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    pthread_mutex_t* lowLock = (pthread_mutex_t*)((size_t*)mem_heap_lo() + BIN_MAX - BIN_MIN + 1);
    return lowLock + binNum - BIN_MIN;
  }
  
  /**
   * Lock a bin
   **/
  inline void allocator::lockBin(uint8_t binNum){
    int rc = pthread_mutex_lock(getBinLock(binNum));
    //int rc = pthread_mutex_trylock(getBinLock(binNum));
    /*if (rc!=0) {
      printf("Lock time out\n");
      assert(false);
    }*/
  }
  
  /**
   * Unlock a bin
   **/
  inline void allocator::unlockBin(uint8_t binNum){
    int rc = pthread_mutex_trylock(getBinLock(binNum));
    assert(rc!=0);
    pthread_mutex_unlock(getBinLock(binNum));
  }
  
  /**
   * Set the value of blockPointer to pointerValue
   **/
  inline void allocator::setBlockPointer(size_t *blockPointer, size_t *pointerValue){
    assert(blockPointer >= (size_t *)mem_heap_lo());
    assert(blockPointer <= (size_t *)mem_heap_hi());
    assert(pointerValue >= getHeapPointer() || pointerValue == 0);
    assert(pointerValue <= (size_t *)mem_heap_hi()+1);
    size_t **pointer = (size_t **)blockPointer;
    *pointer = pointerValue;
  }
  /**
   * Returns a pointer to the beginning of the public heap (after the bin pointers)
   **/
  inline size_t * allocator::getHeapPointer(){
    //assert(sizeAddBytes((size_t*)mem_heap_lo(), (uint64_t)PRIVATE_SIZE)<= (size_t *)mem_heap_hi());
    //return sizeAddBytes((size_t*)mem_heap_lo(), (uint64_t)PRIVATE_SIZE);
    return heapPointer;
  }
  /**
   * Returns a pointer moved by some number of bytes
   **/
  inline size_t * allocator::sizeAddBytes(size_t *pointer, uint64_t bytes){
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
   * Add a block to a bin
   * This operation is threadsafe
   **/
  inline void allocator::addBlockToBin(size_t *blockPointer, uint8_t binNum){
    assert(binNum >= BIN_MIN);
    assert(binNum <= BIN_MAX);
    assert(blockPointer >= getHeapPointer());
    assert(blockPointer <= (size_t *)mem_heap_hi());
    lockBin(binNum);
    setBlockPointer(blockPointer, *getBinPointer(binNum));
    setBinPointer(binNum, blockPointer);
    unlockBin(binNum);
  }
  
  /**
   * Remove and return a block from a bin
   * If no second argument is given, remove the first block
   * If there is a second argument, search for that block in the bin and pop that 
   *   or return 0 if that block is not found
   * This operation is not thread safe
   **/
  inline size_t * allocator::popBlockFromBin(uint8_t binNum, size_t *blockPointer = 0){
    assert(binNum >= BIN_MIN);
    assert(binNum <= BIN_MAX);
    assert(blockPointer == 0 || blockPointer >= getHeapPointer());
    assert(blockPointer <= (size_t *)((char*)(mem_heap_hi())+1));
    size_t * returnBlock;
    if(blockPointer == 0){
      // POP THE FIRST BLOCK FROM THE BIN
      returnBlock = *getBinPointer(binNum);
      assert(returnBlock >= getHeapPointer());
      assert(returnBlock <= (size_t *)mem_heap_hi());
      assert(nextBlock(returnBlock) == 0 || nextBlock(returnBlock) >= getHeapPointer());
      assert(nextBlock(returnBlock) <= (size_t *)mem_heap_hi());
      setBinPointer(binNum, nextBlock(returnBlock));
      return returnBlock;
    }else{
      // SEARCH FOR BLOCKPOINTER IN THE BIN THEN POP IT
      size_t *possibleBlockPointer = *getBinPointer(binNum);
      size_t *previousBlockPointer = (size_t *)getBinPointer(binNum);
      returnBlock = 0;
      // KEEP SEARCHING THE BIN UNTIL THE BLOCKPOINTER IS FOUND
      while(possibleBlockPointer!=0){
        assert(possibleBlockPointer >= getHeapPointer());
        assert(sizeAddBytes(possibleBlockPointer, pow2(binNum)) <= (size_t*)mem_heap_hi()+1);
        assert(previousBlockPointer >= (size_t*)mem_heap_lo());
        assert(sizeAddBytes(previousBlockPointer, pow2(binNum)) <= (size_t*)mem_heap_hi()+1);
        if(possibleBlockPointer == blockPointer){
          // FOUND THE BLOCK
          setBlockPointer(previousBlockPointer, nextBlock(possibleBlockPointer));
          assert(possibleBlockPointer == 0 || (possibleBlockPointer >= getHeapPointer() && possibleBlockPointer < (size_t *)mem_heap_hi()));
          return possibleBlockPointer;
        }
        previousBlockPointer = possibleBlockPointer;
        possibleBlockPointer = nextBlock(possibleBlockPointer);
      }
      // BLOCKPOINTER NOT IN BIN
      return 0;
    }
  }
   
  /**
   * Increase the heap size then add the extra amount to a bin
   * size should be max of current heap size and the size needed by malloc
   * size should be a power of 2 already
   * Return the binNum that the extra block was added to
   * This function is not thread safe
   **/
  inline uint8_t allocator::increaseHeapSize(size_t size)
  {
    assert(size == roundPowUp(size));
    // CALCULATE ACTUAL MAX
    size = max(2*size, roundPowUp((size_t*)mem_heap_hi() - (size_t*)getHeapPointer()));
    assert(size == roundPowUp(size));
    uint8_t binNum = log2(size);
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    assert(*getBinPointer(binNum)==0);
    void *newMemPointer = mem_sbrk(size);
    //memset(newMemPointer, 0, size);                         //TODO: REMOVE THIS
    assert((size_t *)newMemPointer >= getHeapPointer());
    assert((size_t *)newMemPointer <= (size_t *)mem_heap_hi());
    assert((char *)mem_heap_hi() - (char*)newMemPointer + 1 == size);
    addBlockToBin((size_t *)newMemPointer, binNum);
    assert(pow2(binNum) == size);
    return binNum;
  }
  
  /**
   * Split a block in largerBinNum into smaller bins
   * Every bin between largerBinNum and smallerBinNum gets one block
   * smallerBinNum gets two blocks
   * largerBinNum is the bin with a block to split
   * smallerBinNum is the bin that needs blocks
   * To allow for races outside of this function, largerBinNum does not necessarily need a block
   **/
  inline void allocator::splitBlock(uint8_t largerBinNum, uint8_t smallerBinNum)
  {
    lockBin(largerBinNum);
    // CHECK IF LARGERBINNUM ACTUALLY HAS A BLOCK
    if(*getBinPointer(largerBinNum) == 0){
      unlockBin(largerBinNum);
      return;
    }
    assert(*getBinPointer(largerBinNum)!=0);
    //assert(*getBinPointer(smallerBinNum)==0);
    assert(largerBinNum >= BIN_MIN);
    assert(largerBinNum <= BIN_MAX);
    assert(smallerBinNum >= BIN_MIN);
    assert(smallerBinNum <= BIN_MAX);
    assert(largerBinNum >= smallerBinNum);
    size_t smallerSize = pow2(smallerBinNum);
    size_t biggerSize = pow2(largerBinNum);
    
    // GET THE BLOCK TO BE SPLIT
    size_t *requiredBlock = popBlockFromBin(largerBinNum);
    unlockBin(largerBinNum);
    //size_t *endBlock = sizeAddBytes(requiredBlock, (uint64_t)biggerSize);
    //assert(endBlock >= getHeapPointer());
    // ADD THE REQUIRED BLOCK TO A BIN
    addBlockToBin(requiredBlock, smallerBinNum);
    // WHILE CURRENTSIZE IS LESS THAN BIGGERSIZE/2
    size_t *pointerInBlock = sizeAddBytes(requiredBlock, (uint64_t)smallerSize);
    uint8_t currentBin = smallerBinNum;
    
    for(size_t currentSize = smallerSize; currentSize < biggerSize; currentSize *= 2){
      assert(currentSize == pow2(currentBin));
      assert(pointerInBlock <= (size_t *)mem_heap_hi());
      assert(pointerInBlock >= getHeapPointer());
      // ADD THE NEW BLOCK TO THE BIN
      addBlockToBin(pointerInBlock, currentBin);
      // CHANGE VALUES FOR NEXT ITERATION
      pointerInBlock = sizeAddBytes(pointerInBlock, (uint64_t)currentSize);
      currentBin++;
    }
    
    assert(currentBin == largerBinNum);   // Make sure we've split up the whole block
    //assert(endBlock == pointerInBlock);
  }
  
  /**
   * Given a newly freed block at blockPointer with a current bin of binNum, 
   * recursively see if it can be combined with neighboring blocks
   * Returns the new binNum of block
   
   ****  JoinBlock was found to be too slow.  It's better to just take the utilization
   ****  penalty than to take the larger time penalty to call joinBlocks
   **/
  inline void allocator::joinBlocks(size_t *blockPointer, uint8_t binNum, bool joinBefore=true){
    // FOR EACH BIN
    /*
    uint64_t blockSize = pow2(BIN_MIN);
    size_t *block;
    size_t *blockPrevious;
    size_t *possibleJoin;
    for(uint8_t binNum = BIN_MIN; binNum <= BIN_MAX; binNum++){
      blockSize = pow2(binNum);
      lockBin(binNum);
      // COMPARE EVERY TWO BLOCKS
      block = *getBinPointer(binNum);
      blockPrevious = (size_t *)getBinPointer(binNum);
      while(block!=0){
        
        assert(block >= getHeapPointer());
        assert(sizeAddBytes(block, blockSize) <= (size_t*)mem_heap_hi()+1);
        assert(blockPrevious >= (size_t*)mem_heap_lo());
        assert(sizeAddBytes(blockPrevious, blockSize) <= (size_t*)mem_heap_hi()+1);
        
        // CHECK FOR FREE BLOCK AFTER
        //binInfo();
        //printf("%p\n", block);
        //printf("%lu\n", blockSize);
        //printf("%p\n", mem_heap_hi());
        possibleJoin = sizeAddBytes(block, (uint64_t)blockSize);
        
        //assert(possibleJoin == sizeAddBytes(block, (uint64_t)blockSize));
        // Remove possibleJoin
        if(popBlockFromBin(binNum, possibleJoin) != 0){
          assert((char*)sizeAddBytes(block, blockSize*2) <=(char*)mem_heap_hi()+1);
          assert(block >= getHeapPointer());
          assert((char*)sizeAddBytes(possibleJoin, blockSize) <=(char*)mem_heap_hi()+1);
          assert(possibleJoin >= getHeapPointer());
          setBlockPointer(blockPrevious, nextBlock(block));
          lockBin(binNum+1);
          addBlockToBin(block, binNum+1);
          unlockBin(binNum+1);
          block = nextBlock(blockPrevious);
          continue;
        }
        // CHECK FOR FREE BLOCK BEFORE
        possibleJoin = (size_t*)((char*)block + -(uint64_t)blockSize);
        //assert(block == (size_t*)((char*)block + -(uint64_t)blockSize));
        if(possibleJoin >= getHeapPointer()){
          if(popBlockFromBin(binNum, possibleJoin) != 0){
            assert((char*)sizeAddBytes(block, blockSize) <=(char*)mem_heap_hi()+1);
            assert(block >= getHeapPointer());
            assert((char*)sizeAddBytes(possibleJoin, blockSize*2) <=(char*)mem_heap_hi()+1);
            assert(possibleJoin >= getHeapPointer());
            setBlockPointer(blockPrevious, nextBlock(block));
            lockBin(binNum+1);
            addBlockToBin(possibleJoin, binNum+1);
            unlockBin(binNum+1);
            block = nextBlock(blockPrevious);
            continue;
          }
        }
        blockPrevious = nextBlock(blockPrevious);
        block = nextBlock(block);
      }
      unlockBin(binNum);
      blockSize *= 2;
    }
    /*
    size_t blockSize = pow2(binNum);
    // FIND IF CONTIGUOUS FREE BLOCKS ARE FREE
    size_t *previousFreeBlockPointer = (size_t*)getBinPointer(binNum);
    size_t *binFreeBlockPointer = *getBinPointer(binNum);
    while(binFreeBlockPointer !=0){
      if(binFreeBlockPointer == sizeAddBytes(blockPointer, (uint64_t)blockSize)){
        //binFreeBlockPointer is to be deleted
        // Delete old values
        *binFreeBlockPointer = 0;
        *blockPointer = 0;
        setBlockPointer(previousFreeBlockPointer, nextBlock(binFreeBlockPointer));
        setBinPointer(binNum, nextBlock(blockPointer));
        // ADD NEW BLOCK TO BIN
        binNum++;
        setBlockPointer(blockPointer, *getBinPointer(binNum));
        setBinPointer(binNum, blockPointer);
        return joinBlocks(blockPointer, binNum, joinBefore);
      }else if(blockPointer == sizeAddBytes(binFreeBlockPointer, (uint64_t)blockSize) && joinBefore){
        // blockpointer is to be deleted
        // Delete old values
        *binFreeBlockPointer = 0;
        *blockPointer = 0;
        setBlockPointer(previousFreeBlockPointer, nextBlock(binFreeBlockPointer));
        setBinPointer(binNum, nextBlock(blockPointer));
        // ADD NEW BLOCK TO BIN
        binNum++;
        setBlockPointer(binFreeBlockPointer, *getBinPointer(binNum));
        setBinPointer(binNum, binFreeBlockPointer);
        return joinBlocks(binFreeBlockPointer, binNum, joinBefore);
      }
      previousFreeBlockPointer = nextBlock(previousFreeBlockPointer);
      binFreeBlockPointer = nextBlock(binFreeBlockPointer);
    }
    // Don't need to care about binNum too big because memory wouldn't be able to have already allocated blocks
    assert(binNum <= BIN_MAX);
    assert(binNum >= BIN_MIN);
    
    //return binNum;*/
  }
  
  /**
   * Given a pointer to a block where the value stored in the block is also a pointer,
   * return that value as a pointer
   **/
  inline size_t * allocator::nextBlock(size_t *blockPointer){
    // The code is the same as:
    // size_t blockValue = *blockPointer;
    // return (size_t *)blockValue;
    assert((size_t *)*blockPointer <= (size_t *)mem_heap_hi());
    assert((size_t *)*blockPointer >= getHeapPointer() || (size_t *)*blockPointer == 0);
    assert((size_t *)*blockPointer != blockPointer);
    return (size_t *)*blockPointer;
  }
  
  /**
   * Given a pointer to a block that is already malloced, move the pointer back 
   * to the correct place
   **/
  inline size_t * allocator::blockHeader(void *ptr){
    // GO BACK AND FIND THE SIZE
    size_t *blockPointer = (size_t *)ptr-1;
    assert(roundPowUp(*blockPointer) == *blockPointer);
    assert(blockPointer >= getHeapPointer());
    assert(blockPointer < (size_t *)mem_heap_hi());
    return blockPointer;
  }
  
  /**
   * Display information about the bins
   * For heap checking only, not for use in final version
   **/
   void allocator::binInfo(){
     size_t *p;
     int numBlocks;
     for(int i = BIN_MIN; i <= BIN_MAX; i++){
       numBlocks = 0;
       p = *getBinPointer(i);
       printf("%i:  - ", i);
       while(p!=0){
         assert(p <= (size_t *)mem_heap_hi());
         sizeAddBytes(p, pow2(i));
         assert(p >= getHeapPointer() || p == 0);
         printf("% p ", p);
         p = nextBlock(p);
         numBlocks ++;
       }
       printf("  -  %i blocks\n", numBlocks);
     }
     printf("\n");
   }
  
  //********************** HEAP CONSISTENCY CHECKING *****************//
  /**
   * check heap consistency
   **/
  int allocator::check()
  {
    // CHECK THAT EVERY FREE BLOCK HAS A POINTER WITHIN THE HEAP OR IS 0
    // For every bin
    size_t *blockPointer;
    for(uint8_t i = BIN_MIN; i <= BIN_MAX; i++){
      blockPointer = (size_t*)getBinPointer(i);
      assert(blockPointer >= (size_t*)mem_heap_lo());
      assert(blockPointer <= getHeapPointer());
      blockPointer = nextBlock(blockPointer);
      // For every block
      while(blockPointer != 0){
        // Check that the pointer in the block does not point to somewhere wrong
        assert(blockPointer >= getHeapPointer());
        assert(blockPointer <= (size_t*)mem_heap_hi());
        blockPointer = nextBlock(blockPointer);
      }
    }
    
    // CHECK THAT THERE ARE NO LOOPS IN THE LINKED BLOCKS
    // CHECK THAT THERE ARE NO CONTIGUOUS COALESCEABLE BLOCKS
    // For every bin
    size_t blockSize;
    size_t *blockPointer2;
    for(uint8_t i = BIN_MIN; i <= BIN_MAX; i++){
      blockSize = pow2(i);
      blockPointer = *getBinPointer(i);
      // Compare every two blocks
      while(blockPointer != 0){
        blockPointer2 = nextBlock(blockPointer);
        while(blockPointer2 != 0){
          // Check whether the blocks are adjacent
          assert((size_t*)((char*)blockPointer + (char)blockSize) != blockPointer2);
          assert((size_t*)((char*)blockPointer - (char)blockSize) != blockPointer2);
          // check whether blocks are circular
          assert(blockPointer != blockPointer2);
          blockPointer2 = nextBlock(blockPointer2);
        }
        blockPointer = nextBlock(blockPointer);
      }
    }
    //  CHECK THAT THERE ARE NO OVERLAPPING ALLOCATED BLOCKS
    
    
    // Is every block in the free list marked as free?
    // Is every free block actually in the free list?
    
    return 0;
  }
  
  //******************** INIT ***************************//
  /**
   * init - Initialize the malloc package.  Called once before any other
   * calls are made.  Since this is a very simple implementation, we just
   * return success.
   * This is only called once by the parallel allocator, so no need to check for 
   * races in this function.  
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
    
    for(uint8_t binNum=BIN_MIN; binNum<BIN_MAX; binNum++){
      // INITIALIZE BINS
      setBinPointer(binNum, 0);
      
      // INITIALIZE BIN LOCKS
      pthread_mutex_init(getBinLock(binNum), NULL);
    }
    
    // SET THE HEAP POINTER
    heapPointer = sizeAddBytes((size_t*)mem_heap_lo(), (uint64_t)PRIVATE_SIZE);
    
    // ALLOCATE A PART OF THE HEAP TO MEMORIZE EMPTY BIN'S BLOCKS
    addBlockToBin(getHeapPointer(), log2(HEAP_SIZE));
    
    // INITIALIZE COALLESCING SCORE
    int coallesceScore = 0;
    return 0;
  }
  
  
  //******************** MALLOC ***************************//
  /**
   * malloc - Allocate a block by incrementing the brk pointer.
   *     Always allocate a block whose size is a multiple of the alignment.
   */
  void * allocator::malloc(size_t size){
    // Make sure that we're aligned to 8 byte boundaries
    size_t my_aligned_size = roundPowUp(ALIGN(size) + ALIGN(SIZE_T_SIZE));
    assert(size <= (my_aligned_size-8));
    assert(my_aligned_size%8 == 0);
    // FIND THE BIN (ROUND UP LG(SIZE))
    uint8_t binAllocateNum = log2(my_aligned_size);
    lockBin(binAllocateNum);
    size_t **binPtr = getBinPointer(binAllocateNum);
    assert(binAllocateNum >= BIN_MIN);
    assert(binAllocateNum <= BIN_MAX);
    assert(binPtr < (size_t **)getHeapPointer());
    assert(binPtr >= (size_t **)mem_heap_lo());
    // IF BIN IS EMPTY
    while(*binPtr == 0){                                     //TODO: USE A GLOBAL VARIABLE TO SAVE THE HIGHEST BIN NUMBER
      unlockBin(binAllocateNum);
      // SEARCH LARGER BINS FOR BLOCKS                    //TODO: ADD COALESCING IN CASE THERE ARE SMALLER BLOCKS
      uint8_t binToBreakNum;
      for(binToBreakNum = binAllocateNum+1; binToBreakNum <= BIN_MAX; binToBreakNum++){
        if(*getBinPointer(binToBreakNum) != 0) break;
      }
      // IF NO BLOCKS FOUND, ALLOCATE MORE MEMORY FOR THE HEAP
      if(binToBreakNum > BIN_MAX){
        binToBreakNum = increaseHeapSize(my_aligned_size);
      }
      // SPLIT BLOCK UP INTO SMALLER BINS
      splitBlock(binToBreakNum, binAllocateNum);
      lockBin(binAllocateNum);
    }
    
    // ASSERT BIN IS NOT EMPTY
    assert(*getBinPointer(binAllocateNum)!=0);
    // REMOVE BLOCK POINTER FROM BIN
    size_t * returnBlock = popBlockFromBin(binAllocateNum);
    unlockBin(binAllocateNum);
    
    // RECORD THE SIZE INTO THE RETURNBLOCK
    *returnBlock = my_aligned_size;
    assert(*returnBlock!=0);
    returnBlock = sizeAddBytes(returnBlock, (uint64_t)SIZE_T_SIZE);
    // RETURN BLOCK POINTER
    assert(returnBlock >= getHeapPointer());
    assert(returnBlock <= (size_t *)mem_heap_hi());
    return returnBlock;
  }
  
  
  //******************** FREE ***************************//
  /**
   *  Free a block
   **/
  void allocator::free(void *ptr)
  {
    // DON'T DO ANYTHING FOR NULL POINTERS
    if(ptr == NULL){
      return;
    }
    assert(ptr >= getHeapPointer());
    assert((size_t *)ptr <= (size_t *)mem_heap_hi());
    // GO BACK AND FIND THE SIZE
    size_t *blockPointer = blockHeader(ptr);
    freeBlock(blockPointer, *blockPointer);
  }
  
  /**
   * Given a pointer to the beginning of the block and the size of the block that 
   * is to be freed, add the block to bins
   * blockSize is not necessarily a power of 2
   **/
  inline void allocator::freeBlock(size_t *blockPointer, size_t blockSize){
    // FIND BIN THAT BLOCK IS SUPPOSED TO BELONG TO
    uint8_t binNum;
    // FIND BINNUM = min(log(blockSize))
    if(roundPowUp(blockSize)!=blockSize){
      binNum = log2(roundPowUp(blockSize)/2);
    }else{
      binNum = log2(blockSize); 
    }
    assert(binNum >= BIN_MIN);
    assert(binNum <= BIN_MAX);
    // ADD BLOCK TO BIN
    addBlockToBin(blockPointer, binNum);
    // COALESCE BLOCKS
    coallesceScore += 1;
    if(coallesceScore >= COALLESCE_MIN){
      coallesceScore = 0;
      pthread_mutex_unlock(&coallesceScoreMutex);
      joinBlocks(blockPointer, binNum); 
    }else{
      pthread_mutex_unlock(&coallesceScoreMutex);
    }
    if(pow2(binNum) < blockSize){
      //freeBlock(blockPointer + pow2(binNum), blockSize - pow2(binNum));     //TODO: RE ENABLE OR THERE WILL BE A MEM LEAK
    }
  }
  
  
  //******************** REALLOC ***************************//
  /**
   * Realloc reads information about the returned pointer, computes whether the 
   * block needs to be increased or decreased in size, then does the computations to 
   * change the block size
   */
  void * allocator::realloc(void *origPointer, size_t wantSize)
  {
    if(wantSize==0){  // if size is 0, same as free
      free(origPointer);
      return (void*)0;
    }
    if(origPointer==0){ // if null pointer, same as malloc
      return malloc(wantSize);
    }
    assert((size_t*)origPointer >= getHeapPointer());
    assert((size_t*)origPointer <= mem_heap_hi());
    // GO BACK AND FIND THE SIZE
    size_t *blockPointer = blockHeader(origPointer);
    uint64_t origSize = (uint64_t)*blockPointer;            // TODO: CONVERT TO SIZE_T INSTEAD OF UINT64_T
    uint64_t currentSize = (uint64_t)*blockPointer;
    assert(pow2(log2(origSize)) == origSize);
    assert(pow2(log2(currentSize)) == currentSize);
    // CALCULATE DIFFERENCE IN MEMORY THAT IS NEEDED FOR THE REALLOC
    wantSize = roundPowUp(ALIGN(wantSize) + ALIGN(SIZE_T_SIZE));
    if(wantSize == origSize){
      assert(origPointer == blockPointer + 1);
      return origPointer;
    }
    uint8_t binNum = log2(currentSize);
    assert(binNum >= BIN_MIN);
    assert(binNum <= BIN_MAX);
    size_t *endOfOrigBlock = sizeAddBytes(blockPointer, origSize);
    size_t *endOfCurrentBlock = endOfOrigBlock;
    assert(endOfOrigBlock >= getHeapPointer());
    assert(endOfOrigBlock <= (size_t*)mem_heap_hi()+1);
    size_t *poppedNeighborBlock;
    while(currentSize < wantSize){        //IF REQUESTED SIZE IS LARGER
      // CHECK IF JOINING NEIGHBOR BINS WORKS
      lockBin(binNum);
      poppedNeighborBlock = popBlockFromBin(binNum, sizeAddBytes(blockPointer, currentSize));
      unlockBin(binNum);
      if(poppedNeighborBlock == 0){
        break;
      }
      assert(poppedNeighborBlock == sizeAddBytes(blockPointer, currentSize));
      endOfCurrentBlock = sizeAddBytes(blockPointer, currentSize);
      currentSize *= 2;
      ++binNum;
    }
    if(currentSize > wantSize){       // IF CURRENT SIZE IS LARGER
      // FREE THE REST OF THE BLOCK
      //freeBlock(endOfCurrentBlock, currentSize - wantSize);
      // CHANGE THE BLOCK SIZE
      *blockPointer = currentSize;
      return origPointer;
    }else{                            // IF CURRENT SIZE IS STILL SMALLER THAN WANTED SIZE
      size_t *newBlock = (size_t*)malloc(wantSize);
      std::memcpy(newBlock, origPointer, origSize - SIZE_T_SIZE);
      freeBlock(blockPointer, origSize);
      return (void*)newBlock;
    }
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
