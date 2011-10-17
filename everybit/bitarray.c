/*  Copyright (c) 2010 6.172 Staff

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

/* Implements the ADT specified in bitarray.h as a packed array of bits; a
 * bitarray containing bit_sz bits will consume roughly bit_sz/8 bytes of
 * memory. */

#include <assert.h>
#include <stdio.h>

#include "bitarray.h"
#include "bytefliptable.h"

/* Internal representation of the bit array. */
struct bitarray {
  /* The number of bits represented by this bit array. Need not be divisible by 8. */
  size_t bit_sz;
  /* The underlying memory buffer that stores the bits in packed form (8 per byte). */
  char *buf;
};

bitarray_t *bitarray_new(size_t bit_sz) {
  /* Allocate an underlying buffer of ceil(bit_sz/8) bytes. */
  char *buf = calloc(1, bit_sz / 8 + ((bit_sz % 8 == 0) ? 0 : 1));
  if (buf == NULL)
    return NULL;
  bitarray_t *ret = malloc(sizeof(struct bitarray));
  if (ret == NULL) {
    free(buf);
    return NULL;
  }
  ret->buf = buf;
  ret->bit_sz = bit_sz;
  return ret;
}

void bitarray_free(bitarray_t *ba) {
  if (ba == NULL)
    return;
  free(ba->buf);
  ba->buf = NULL;
  free(ba);
}

size_t bitarray_get_bit_sz(bitarray_t *ba) {
  return ba->bit_sz;
}

/* Portable modulo operation that supports negative dividends. */
static size_t modulo(ssize_t n, size_t m) {
  /* See
  http://stackoverflow.com/questions/1907565/c-python-different-behaviour-of-the-modulo-operation */
  /* Mod may give different result if divisor is signed. */
  ssize_t sm = (ssize_t) m;
  assert(sm > 0);
  ssize_t ret = ((n % sm) + sm) % sm;
  assert(ret >= 0);
  return (size_t) ret;
}

static char bitmask(size_t bit_index) {
  return 1 << (bit_index % 8);
}

bool bitarray_get(bitarray_t *ba, size_t bit_index) {
  assert(bit_index < ba->bit_sz);
  return (ba->buf[bit_index / 8] & bitmask(bit_index)) ? true : false;
}

void bitarray_set(bitarray_t *ba, size_t bit_index, bool val) {
  assert(bit_index < ba->bit_sz);
  ba->buf[bit_index / 8]
      = (ba->buf[bit_index / 8] & ~bitmask(bit_index)) | (val ? bitmask(bit_index) : 0); 
}

size_t bitarray_count_flips(bitarray_t *ba, size_t bit_off, size_t bit_len) {
  assert(bit_off + bit_len <= ba->bit_sz);
  size_t i, ret = 0;
  /* Go from the first bit in the substring to the second to last one. For each bit, count another
  transition if the bit is different from the next one. Note: do "i + 1 < bit_off + bit_len" instead
  of "i < bit_off + bit_len - 1" to prevent wraparound in the case where bit_off + bit_len == 0. */
  for (i = bit_off; i + 1 < bit_off + bit_len; i++) {
    if (bitarray_get(ba, i) != bitarray_get(ba, i + 1))
      ret++;
  }
  return ret;
}

/* Rotate substring left by one bit. */
static void bitarray_rotate_left_one(bitarray_t *ba, size_t bit_off, size_t bit_len) {
  size_t i;
  bool first_bit = bitarray_get(ba, bit_off);
  for (i = bit_off; i + 1 < bit_off + bit_len; i++)
    bitarray_set(ba, i, bitarray_get(ba, i + 1));
  bitarray_set(ba, i, first_bit);
}

/* Rotate substring left by the specified number of bits. */
static void bitarray_rotate_left(bitarray_t *ba, size_t bit_off, size_t bit_len, size_t bit_amount)
{
  size_t i;
  for (i = 0; i < bit_amount; i++)
    bitarray_rotate_left_one(ba, bit_off, bit_len);
}
/*
 * The original function that was used.  "_old" has been appended to the funtion name
 */
void bitarray_rotate_old(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount) {
  assert(bit_off + bit_len <= ba->bit_sz);
  if (bit_len == 0)
    return;
  /* Convert a rotate left or right to a left rotate only, and eliminate multiple full rotations. */
  bitarray_rotate_left(ba, bit_off, bit_len, modulo(-bit_right_amount, bit_len));
}


/*
 * BETA CODE STARTS BELOW
 */

/**
 * Reverses the bit order of the *ba[bit_off : bit_off + bit_len] substring
 * This is done bit by bit
 */
inline void bitarray_reverse_bit(bitarray_t *ba, size_t bit_off, size_t bit_len) {
  assert(bit_off + bit_len <= bitarray_get_bit_sz(ba));

  size_t start = bit_off;
  size_t end = bit_off + bit_len - 1;
  bool temp;

  while (start < end) {

    // Swaps the start and end bit
    temp = bitarray_get(ba, start);
    bitarray_set(ba, start, bitarray_get(ba, end));
    bitarray_set(ba, end, temp);

    start++;
    end--;
  } 
}

/**
 * Rotates a bitarray using the reverse swap method;
 * That is, bitarray ab is transformed to ba using the identity (a^R b^R)^R
 * This method rotates bit by bit, which is slow...
 */
void bitarray_rotate_bit(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount) {
  assert(bit_off + bit_len <= ba->bit_sz);

  if (bit_len == 0)
    return;

  // Converts rotates in either direction to a left rotate
  size_t k = modulo(-bit_right_amount, bit_len);

  if (k == 0)
    return;

  // Rotates using bit reverse
  bitarray_reverse_bit(ba, bit_off, k);
  bitarray_reverse_bit(ba, bit_off + k, bit_len - k);
  bitarray_reverse_bit(ba, bit_off, bit_len);
}

/**
 * Swaps two bytes
 * Also passing temp pointer so that a temp variable doesn't need to be created in the function
 */
inline void byte_switch( char *a, char *b, char temp) {
  temp = *a;
  *a = *b;
  *b = temp;
}


/*
 * *********************** FINAL CODE STARTS BELOW ***************************
 */

/**
 * Retrieves byte at index byte_index from bitarray
 */
char *bitarray_get_byte(bitarray_t *ba, size_t byte_index) {
  assert(byte_index < ba->bit_sz / 8);
  printf("Bitarray_get_byte got this value : %i \n", *(ba->buf + byte_index) & 255);
  return ba->buf + byte_index;
}

inline void bitarray_reverse(bitarray_t *ba, size_t bit_off, size_t bit_len){
  // Check if the reverse substring is inside one byte
  if (bit_off%8 + bit_len <= 8) bitarray_reverse_bit(ba, bit_off, bit_len);
  
  // Calculate indices of the partial bytes that are on the ends of this substring
  // I've checked the calculations for these variables
  // The bits not to be touched are between left_start (inclusive) and left_start+left_len (exclusive) like bit_off and bit_len
  size_t left_start = bit_off;
  size_t left_len = (8 - (bit_off % 8)) % 8;
  size_t right_len = (bit_off + bit_len) % 8;
  size_t right_start = bit_off + bit_len - right_len;
  
  // Reverse bytes in place
  size_t byte_start = (left_start + left_len) / 8;
  size_t byte_end = (right_start / 8); // exclusive

  for (size_t byte_index = byte_start;
      byte_index < byte_end;
      ++byte_index) {
    byte_reverse(bitarray_get_byte(ba, byte_index));
  }

  // Switch bytes
  size_t right_byte_index = byte_end-1;
  char temp;

  for (size_t left_byte_index = byte_start;
       left_byte_index < (byte_end + byte_start) / 2;
       ++left_byte_index) {
    byte_switch(bitarray_get_byte(ba, right_byte_index), bitarray_get_byte(ba, left_byte_index), temp);
    --right_byte_index;
  }

  // TODO: TAKE CARE OF ENDS
  // use left_start, left_len, right_start, right_len
  // shifts and shits... yeah!
  
}

/**
 * Main function to rotate a bitstring
 * ba -- bitstring
 * bitoff -- rotation index start (inclusive)
 * bit_len -- length of string to rotate
 * bit_right_amount -- amount of rotation
 */
void bitarray_rotate(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount) {
  // Sanity checks
  assert((bit_off + bit_len) <= ba->bit_sz);

  // Don't do anything if there's nothing to rotate
  if (bit_len <= 1)
    return;

  // Converts rotates in either direction to a left rotate
  size_t k = modulo(-bit_right_amount, bit_len);

  // Don't do anything if it's not being rotated
  if (k == 0)
    return;

  // Converts bitarray ab to ba using identity:
  // ba = (a^R b^R)^R
  // where ^R = bits in reverse order
  //  bitarray_reverse(ba, bit_off, k);
  //  bitarray_reverse(ba, bit_off + k, bit_len - k);
   bitarray_reverse(ba, bit_off, bit_len);
}
