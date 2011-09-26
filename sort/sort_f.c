#include "util.h"
#include "isort.c"

/* Function prototypes */

static void sort_fprime(data_t *A, int p, int r, data_t *l);
static inline void merge_f(data_t *A, int p, int q, int r, data_t *l);
static inline void copy_f(data_t * source, data_t * dest, int n) ;

/* Function definitions */

#define use_isort 100

/* Init for sort */
void sort_f(data_t *A, int p, int r)
{
  data_t * left = 0; 
  mem_alloc(&left, r-p);
  sort_fprime(A, p, r, left);
  mem_free(&left);
}

/* Basic merge sort */
static void sort_fprime(data_t *A, int p, int r, data_t *l) 
{
	assert (A) ;

	if (p < r) {
	  if (r-p < use_isort) {
	    isort(&(A[p]), &(A[r]));
	  }

	  else {
		int q = (p + r) / 2 ;
		sort_fprime(A, p, q, l);
		sort_fprime(A, q + 1, r, l);
		merge_f(A, p, q, r, l) ;
	    }	
	}
}

/* A merge routine. Merges the sub-arrays A [p..q] and A [q + 1..r].
 * Uses two arrays 'left' and 'right' in the merge operation.
 */
static inline void merge_f(data_t *A, int p, int q, int r, data_t *l) 
{ 
	assert(A) ;
	assert(p <= q) ;
	assert((q + 1) <= r) ;
	int n1 = q - p + 1;
	int n2 = r - q ;

	data_t * left = l;
        if (left == NULL)
        {
	  return ;
        }
	
	copy_f (&(A [p]), left, n1) ;
	left [n1] = UINT_MAX ;

	unsigned int * __restrict Aptr = &(A[p]);
	unsigned int * __restrict leftptr = left;
	unsigned int * __restrict rightptr = &(A[q+1]);

	while(n1 > 0 && n2 > 0)
	{
	  long cmp = (*leftptr <= *rightptr);
	  long min = *rightptr ^ ((*leftptr ^ *rightptr) & -(cmp));

	  *Aptr++ = min;
	  leftptr += cmp; n1 -= cmp;
	  rightptr += !cmp; n2 -= !cmp;
	}

	while (n1 > 0) {
	  *Aptr++ = *leftptr;
	  n1--;
	}
}

static inline void copy_f(data_t * source, data_t * dest, int n)
{                                                                             
        assert (dest) ;                                                       
        assert (source) ;                                                     
        int i ;                                                               
        for (i = 0 ; i < n ; i++)                                             
        {                                                                     
                dest [i] = source [i] ;                                       
        }                                                                     
}
