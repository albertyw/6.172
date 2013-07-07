#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Typedefs */

typedef uint32_t data_t;

/* Insertion sort */
void inline isort(data_t *left, data_t *right) 
{
  data_t *cur = left + 1;
  while (cur <= right) {
    data_t val = *cur;
    data_t *index = cur - 1;

    while (index >= left && *index > val) {
      *(index + 1) = *index;
      index--;
    }

    *(index + 1) = val;
    cur++;
  }
}

