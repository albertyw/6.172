#ifndef MM_MDRIVER_H
#define MM_MDRIVER_H

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "fsecs.h"
#include "memlib.h"
#include "allocator_interface.h"

/**********************
 * Constants and macros
 **********************/

/* Misc */
#define MAXLINE     1024 /* max string size */
#define HDRLINES       4 /* number of header lines in a trace file */
#define LINENUM(i) (i+5) /* cnvt trace request nums to linenums (origin 1) */

enum traceop_type {ALLOC, FREE, REALLOC} ; /* type of request */
/******************************
 * The key compound data types
 *****************************/

/* Characterizes a single trace operation (allocator request) */
typedef struct {
  traceop_type  type; /* type of request */
  int index;                        /* index for free() to use later */
  int size;                         /* byte size of alloc/realloc request */
} traceop_t;

/* Holds the information for one trace file*/
typedef struct {
  int sugg_heapsize;   /* suggested heap size (unused) */
  int num_ids;         /* number of alloc/realloc ids */
  int num_ops;         /* number of distinct requests */
  int weight;          /* weight for this trace (unused) */
  traceop_t *ops;      /* array of requests */
  char **blocks;       /* array of ptrs returned by malloc/realloc... */
  size_t *block_sizes; /* ... and a corresponding array of payload sizes */
} trace_t;

/* Function pointers for a malloc implementation.  This is used to allow a
 * single validator to operate on both libc malloc, a buggy malloc, and the
 * student "mm" malloc.
 */
typedef struct {
  int (*init)(void);
  void *(*malloc)(size_t size);
  void *(*realloc)(void *ptr, size_t size);
  void (*free)(void *ptr);
  int (*check)();
  void (*reset_brk)(void);
  void *(*heap_lo)(void);
  void *(*heap_hi)(void);
} malloc_impl_t;

/*********************
 * Function prototypes
 *********************/

void malloc_error(int tracenum, int opnum, char *msg);
void unix_error(char *msg);
void app_error(char *msg);

#endif /* MM_MDRIVER_H */
