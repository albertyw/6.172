/**
 * Matrix Multiply
 *
 *
 * Declarations here are your API specification -- we expect that your
 * code defines and correctly implements these functions and does not modify
 * the way the reference implementation uses variables mentioned here,
 * so that we may call them from testbed.c and any other testing software
 * that we write!
 *
 * Deviating from this API may cause your program to fail all of our tests.
 **/

#ifndef MATRIX_MULTIPLY_H_INCLUDED

#define MATRIX_MULTIPLY_H_INCLUDED

/* Types */

typedef struct {
  int rows;
  int cols;
  int** values;
} matrix;

/** This is called before matrix_multiply_run(), for you to perform any
 * setup tasks.
 *
 * Do not place any code that should be timed within this function!
 */
void matrix_multiply_setup(void);

/**
 * Multiply matrix A*B, store result in C.
 */
int matrix_multiply_run(const matrix* A, const matrix* B, matrix* C);

/*
 * Allocates a row-by-cols matrix and returns it
 *
 */
matrix* make_matrix(int rows, int cols);

/*
 * Frees an allocated matrix
 */
void free_matrix(matrix* m);

/*
 * Print matrix
 */
void print_matrix(const matrix* m);
#endif
