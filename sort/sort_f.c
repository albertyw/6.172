#include "util.h"

/* Function prototypes */

static void merge_f(data_t *A, int p, int q, int r);
static void copy_f(data_t * source, data_t * dest, int n) ;

/* Function definitions */

/* Basic merge sort */
void sort_f(data_t *A, int p, int r) 
{
	assert (A) ;
	if (p < r)
	{
		int q = (p + r) / 2 ;
		sort_f(A, p, q);
		sort_f(A, q + 1, r);
		merge_f(A, p, q, r) ;
	}	
}

/* A merge routine. Merges the sub-arrays A [p..q] and A [q + 1..r].
 * Uses two arrays 'left' and 'right' in the merge operation.
 */
static void merge_f(data_t *A, int p, int q, int r) 
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
	
	copy_f (&(A [p]), left, n1) ;
	copy_f (&(A [q + 1]), right, n2) ; 
	left [n1] = UINT_MAX ;
	right [n2] = UINT_MAX ;

	int i = 0 ;
	int j = 0 ;
	int k = p ;	
	for ( ; k <= r ; k++)
	{
		if (left [i] <= right [j])
		{
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

static void copy_f(data_t * source, data_t * dest, int n)
{                                                                             
        assert (dest) ;                                                       
        assert (source) ;                                                     
        int i ;                                                               
        for (i = 0 ; i < n ; i++)                                             
        {                                                                     
                dest [i] = source [i] ;                                       
        }                                                                     
}
