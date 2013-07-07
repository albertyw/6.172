#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "matrix_multiply.h"
#include "assert.h"

/*
 * Allocates a row-by-cols matrix and returns it
 */
matrix* make_matrix(int rows, int cols)
{
  matrix* new_matrix = malloc(sizeof(matrix));

  // Set the number of rows and columns
  new_matrix->rows = rows;
  new_matrix->cols = cols;

  // Allocate a buffer big enough to hold the matrix.
  new_matrix->values = (int**)malloc(sizeof(int*) * rows);
  int i;
  for (i = 0; i < rows; i++) {
    new_matrix->values[i] = (int*)malloc(sizeof(int) * cols);
  }

  return new_matrix;
}

/*
 * Frees an allocated matrix
 */
void free_matrix(matrix* m)
{
  int i;
  for (i = 0; i < m->rows; i++) {
    free(m->values[i]);
  }
  free(m->values);
  free(m);
}

/*
 * Print Matrix
 */
void print_matrix(const matrix* m)
{
  int i, j;
  printf("------------\n");
  for (i = 0; i < m->rows; i++) {
    for (j = 0; j < m->cols; j++) {
      printf("  %d  ", m->values[i][j]);
    }
    printf("\n");
  }
  printf("------------\n");
}


/**
 * Multiply matrix A*B, store result in C.
 */
int matrix_multiply_run(const matrix* A, const matrix* B, matrix* C)
{
  assert(A->cols == B->rows);
  assert(A->rows == C->rows);
  assert(B->cols == C->cols);
    
  int i, j, k;
  for (i = 0; i < A->rows; i++) {
    for (j = 0; j < A->cols; j++) {
      C->values[i][j] = 0;
    }
  }
  for (i = 0; i < A->rows; i++) {
    for (k = 0; k < A->cols; k++) {
      for (j = 0; j < B->cols; j++) {
        C->values[i][j] += A->values[i][k] * B->values[k][j];
      }
    }
  }

  return 0;
}
