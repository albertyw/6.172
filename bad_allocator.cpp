#include<iostream>
#include<cstdlib>
#include "allocator_interface.h"
#include "memlib.h"
#define BAD_SIZE 4101

namespace my
{
/*
 * bad_init - Does nothing.
 */
int bad_allocator::init()
{
	return 0;
}

/*
 * bad_check - No checker.
 */
int bad_allocator::check()
{
	return 1;
}

/*
 * bad_malloc - Allocate a block by incrementing the brk pointer.
 * Always allocate a block whose size is not a multiple of the alignment,
 *     and may not fit the requested allocation size.
 */
void * bad_allocator::malloc(size_t size)
{
	size = BAD_SIZE;  /* Overrides input, and has bad aligmnent. */
	void *p = mem_sbrk(size);

	if (p == (void *)-1) {
		/* The heap is probably full, so return NULL. */
		return NULL;
	} else {
		return p;
	}
}

/*
 * bad_free - Freeing a block does nothing.
 */
void bad_allocator::free(void *ptr)
{
}

/*
 * bad_realloc - Implemented simply in terms of bad_malloc and bad_free, but lacks
 *      copy step.
 */
void * bad_allocator::realloc(void *ptr, size_t size)
{
	void *newptr;

	/* Allocate a new chunk of memory, and fail if that allocation fails. */
	newptr = malloc(size);
	if (NULL == newptr)
	  return NULL;

	/* Release the old block. */
	free(ptr);

	/* Return a pointer to the new block. */
	return newptr;
}


/* call mem_reset_brk. */
void bad_allocator::reset_brk()
{
	mem_reset_brk() ;
}

/* call mem_heap_lo */
void * bad_allocator::heap_lo()
{
	return mem_heap_lo() ;
}

/* call mem_heap_hi */ 
void * bad_allocator::heap_hi()
{
	return mem_heap_hi() ;
}


};
