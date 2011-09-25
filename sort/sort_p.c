#include "util.h"

/* Function prototypes */

static void merge_p(data_t *A, int p, int q, int r);
static void copy_p(data_t * source, data_t * dest, int n) ;

/* Function definitions */

/* Basic merge sort */
void sort_p(data_t *A, int p, int r) 
{
	assert (A) ;
	if (p < r)
	{
		int q = (p + r) / 2 ;
		sort_p(A, p, q);
		sort_p(A, q + 1, r);
		merge_p(A, p, q, r) ;
	}	
}

/* A merge routine. Merges the sub-arrays A [p..q] and A [q + 1..r].
 * Uses two arrays 'left' and 'right' in the merge operation.
 */
inline static void merge_p(data_t *A, int p, int q, int r) 
{ 
	assert(A) ;
	assert(p <= q) ;
	assert((q + 1) <= r) ;
	int n1 = q - p + 1;
	int n2 = r - q ;

	data_t * left = 0, * right = 0 ; 
	mem_alloc(&left, n1 + 1) ;
	mem_alloc(&right, n2 + 1) ;
        if (left == NULL || right == NULL)
        {
                mem_free (&left) ;
                return ;
        }
	
	copy_p (&(A [p]), left, n1) ;
	copy_p (&(A [q + 1]), right, n2) ; 
	left [n1] = UINT_MAX ;
	right [n2] = UINT_MAX ;

	int i = 0 ;
	int j = 0 ;
	unsigned int *ptr;
	unsigned int *ptr2;
	int k = p ;	
	for ( ; k <= r ; k++)
	{
		if (left [i] <= right [j])
		{
			/*
			ptr = &A[k];
			ptr2 = &left[i];
			*ptr = *ptr2;
			*/
			A [k] = left [i] ;
			i++ ;
		}
		else
		{
			A [k] = right [j] ;
			j++ ;
		}
	}

	mem_free(&left) ;
	mem_free(&right);
}

inline static void copy_p(data_t * source, data_t * dest, int n)                              
{                                                                             
        assert (dest) ;                                                       
        assert (source) ;                                                     
        int i ;                                                               
        for (i = 0 ; i < n ; i++)                                             
        {                                                                     
                dest [i] = source [i] ;                                       
        }                                                                     
}
