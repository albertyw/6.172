#include "util.h"
#include "isort.c"

/* Function prototypes */

static inline void merge_m(data_t *A, int p, int q, int r);
static inline void copy_m(data_t * source, data_t * dest, int n) ;

/* Function definitions */

#define sort_1 100

/* Basic merge sort */
void sort_m(data_t *A, int p, int r) 
{
	assert (A) ;

	if (p < r) {
	  if (r-p < sort_1) {
	    isort(&(A[p]),&(A[r]));
	  }
	  else {
	    int q = (p + r) / 2 ;
	    sort_m(A, p, q);
	    sort_m(A, q + 1, r);
	    merge_m(A, p, q, r) ;
	  }	
	}
}

/* A merge routine. Merges the sub-arrays A [p..q] and A [q + 1..r].
 * Uses two arrays 'left' and 'right' in the merge operation.
 */
static inline void merge_m(data_t *A, int p, int q, int r) 
{ 
	assert(A) ;
	assert(p <= q) ;
	assert((q + 1) <= r) ;
	int n1 = q - p + 1;
	int n2 = r - q;

	data_t * left = 0; 
     	mem_alloc(&left, n1 + 1) ;
        if (left == NULL)
        {
                mem_free (&left) ;
                return ;
        }
	
	copy_m (&(A [p]), left, n1) ;
	left [n1] = UINT_MAX ;

	unsigned int * __restrict Aptr = &(A[p]);
	unsigned int * __restrict leftptr = left;
	unsigned int * __restrict rightptr = &(A[q+1]);

	while (n1 > 0 && n2 > 0) {

	  long cmp = (*leftptr <= *rightptr);
	  long min = *rightptr ^ ((*leftptr ^ *rightptr) & -(cmp));

	  *Aptr++ = min;
	  leftptr += cmp; n1 -= cmp;
	  rightptr += !cmp; n2 -= !cmp;
	}

	while (n1 > 0) {
	  *Aptr++ = *leftptr++;
	  n1--;
	}

	mem_free(&left) ;
}

static inline void copy_m(data_t * source, data_t * dest, int n)
{                                                                             
        assert (dest) ;                                                       
        assert (source) ;                                                     
        int i ;                                                               
        for (i = 0 ; i < n ; i++)                                             
        {                                                                     
                dest [i] = source [i] ;                                       
        }                                                                     
}
