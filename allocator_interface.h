#ifndef _ALLOCATOR_INTERFACE_H
#define _ALLOCATOR_INTERFACE_H

#include<iostream>
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
    static int init ();
    static size_t ** getBinPointer (uint8_t binNum);
    static void setBinPointer (uint8_t binNum, size_t *setPointer);
    static void setBlockPointer (size_t *blockPointer, size_t *pointerValue);
    static size_t * getHeapPointer ();
    static int increaseHeapSize (size_t size);
    static void splitBlock (int largerBinNum, int smallerBinNum);
    static size_t * nextBlock (size_t *blockPointer);
    static void * malloc (size_t size);
    static void * realloc (void *ptr, size_t size);
    static void free (void *ptr);
    static int check ();
    void reset_brk ();
    void * heap_lo ();
    void * heap_hi ();
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
