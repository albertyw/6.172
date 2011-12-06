#ifndef _ALLOCATOR_INTERFACE_H
#define _ALLOCATOR_INTERFACE_H

#include<iostream>
#include <tbb/mutex.h>
#include <pthread.h>

namespace my
{
  class allocator_interface
  {
  public:
    virtual void reset_brk () = 0;
    virtual void * heap_lo () = 0;
    virtual void * heap_hi () = 0;
  };

  class libc_allocator : public virtual allocator_interface
  {
  public:
    static int init ();
    static void * malloc (size_t size);
    static void * realloc (void *ptr, size_t size);
    static void free (void *ptr);
    static int check ();
    void reset_brk ();
    void * heap_lo ();
    void * heap_hi ();
  };

  class allocator : public virtual allocator_interface
  {
  public:
    static pthread_mutex_t qwer;
    
    static size_t ** getBinPointer(uint8_t binNum);
    static void setBinPointer(uint8_t binNum, size_t *setPointer);
    static pthread_mutex_t * getBinLock(uint8_t binNum);
    static void lockBin(uint8_t binNum);
    static void unlockBin(uint8_t binNum);
    static void setBlockPointer(size_t *blockPointer, size_t *pointerValue);
    static size_t * getHeapPointer();
    static size_t * sizeAddBytes(size_t *pointer, uint64_t bytes);
    
    static void addBlockToBin(size_t *blockPointer, uint8_t binNum);
    static size_t * popBlockFromBin(uint8_t binNum);
    static uint8_t increaseHeapSize(size_t size);
    static void splitBlock(uint8_t largerBinNum,uint8_t smallerBinNum);
    static uint8_t joinBlocks(size_t *blockPointer, uint8_t binNum, bool joinBefore);
    static size_t * nextBlock(size_t *blockPointer);
    static size_t * blockHeader(void *ptr);
    static void binInfo();
    
    static int init();
    static void * malloc(size_t size);
    static void * realloc(void *ptr, size_t size);
    static void free(void *ptr);
    static void freeBlock(size_t *pointer, size_t blockSize);
    static int check();
    void reset_brk();
    void * heap_lo();
    void * heap_hi();
  };


  class bad_allocator : public virtual allocator_interface
  {
  public:
    static int init ();
    static void * malloc (size_t size);
    static void * realloc (void *ptr, size_t size);
    static void free (void *ptr);
    static int check ();
    void reset_brk ();
    void * heap_lo ();
    void * heap_hi ();
  };

};
#endif /* _ALLOCATOR_INTERFACE_H */
