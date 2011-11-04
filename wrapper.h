/*
 * A wrappper for allocator.cpp.
 *
 * Do not modify this file.
 */

#ifndef WRAPPER_H
#define WRAPPER_H

#include <unistd.h>

#ifdef MYMALLOC
#define CUSTOM_FREE(ptr) my_free(ptr)
#define CUSTOM_MALLOC(size) my_malloc(size)
#define CUSTOM_REALLOC(ptr, size) my_realloc(ptr, size)
#else
#define CUSTOM_FREE(ptr) free(ptr)
#define CUSTOM_MALLOC(size) malloc(size)
#define CUSTOM_REALLOC(ptr, size) realloc(ptr, size)
#endif

void my_malloc_init();
void my_free(void *ptr);
void *my_malloc(size_t size);
void *my_realloc(void* ptr, size_t size);

void end_thread();
void end_program();

#endif /* WRAPPER_H */
