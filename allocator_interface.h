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
