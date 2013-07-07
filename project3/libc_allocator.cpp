#include<iostream>
#include<cstdlib>
#include "allocator_interface.h"

namespace my
{
/* Libc needs no initialization. */
int libc_allocator::init ()
{
	return 0;
}

/* Libc has no heap checker, so we just return true. */
int libc_allocator::check()
{
  return 0 ;
}

/* Libc can't be reset. */
void libc_allocator::reset_brk()
{
}

/* Return NULL for the minimum pointer value.*/
void * libc_allocator::heap_lo()
{
	return NULL;
}

/* Return NULL. 
   This probably isn't portable. */
void * libc_allocator::heap_hi()
{
	//return NULL - 1;
	return NULL ;
}

/*call default malloc */
void * libc_allocator::malloc (size_t size)
{
	return std::malloc(size) ;
}

/*call default realloc */
void * libc_allocator::realloc (void *ptr, size_t size)
{
	return std::realloc(ptr, size) ;
}


/*call default realloc */
void libc_allocator::free (void *ptr)
{
	std::free(ptr) ;
}

};
