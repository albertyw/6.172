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

void bitarray_rotate2(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount) {
  assert(bit_off + bit_len <= ba->bit_sz);
  if (bit_len == 0)
    return;
  /* Convert a rotate left or right to a left rotate only, and eliminate multiple full rotations. */
  bitarray_rotate_left(ba, bit_off, bit_len, modulo(-bit_right_amount, bit_len));
}

/*
  Albert/Lekha version of rotate; Will eventually replace bitarray_rotate above;
  Rotates a bitarray using the reverse swap method in the last paragraph of 3.1;
  That is, to transform bitarray ab to ba, we perform (a^R b^R)^R
*/
void bitarray_rotate(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount) {
  assert(bit_off + bit_len <= ba->bit_sz);

  if (bit_len == 0)
    return;

  // Converts rotates in either direction to a left rotate
  size_t k = modulo(-bit_right_amount, bit_len);

  // Rotates using bit reverse
  bitarray_reverse(ba, bit_off, k);
  bitarray_reverse(ba, bit_off + k, bit_len - k);
  bitarray_reverse(ba, bit_off, bit_len);

  //TODO
  
  // Chop off and save ends that aren't rotated
  
  //bitarray_full_rotate(CHOPPED_NEW_BIT_ARRAY, ssize_t bit_right_amount)
  
  // Add ends that are rotated
}

/*
  Rotate the full bitarray recursively, assuming that the bitarray is a multiple of 8
*/
inline static void bitarray_full_rotate(bitarray_t *ba, ssize_t bit_right_amount) {
  //TODO
  
  // Split ba into left (a) and right (b) sections
  
  // Add or remove padding on left and right sections to make it a multiple of 16
  
  // Fix or remove padding to correct
  
  
}

/*
 * Reverses the bit order of the *ba[bit_off : bit_off + bit_len] substring
  // (Maybe in the future) Reverse a bitarray of any length that is a multiple of 16
*/
inline void bitarray_reverse(bitarray_t *ba, size_t bit_off, size_t bit_len) {
  assert(bit_off + bit_len <= ba->bitarray_get_bit_sz);

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

  //TODO (Maybe in the future)
  //assert(bitarray_get_bit_sz()%16 == 0);
  // int mirror;
  // int size = bitarray_get_bit_sz();
  // for(int i; i < size/2; i+=8){
  //   mirror = i + (size/2 - i)*2;
  //   byte_reverse(ba[i]);
  //   byte_reverse(ba[mirror]);
  //   byte_switch(ba[i], ba[mirror]);
  // }
}

/* Takes two bytes and switches them in place using a bithack
*/
inline static void byte_switch(int *a, int *b) {
  *a = *a ^ *b;
  *b = *a ^ *b;
  *a = *a ^ *b;
}

/* Takes a single byte (expressed as an int, and reverses the bits in the byte 
using a bithack.  
http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64BitsDiv
*/
inline static void byte_reverse(int *b) {
  *b = (*b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}
