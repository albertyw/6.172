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
inline static size_t modulo(ssize_t n, size_t m) {
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
#ifdef OLDCODE
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
  bitarray_rotate_left(ba, bit_off, bit_len, modulo(-bit_right_amount, bit_len));
}
#endif


/*
 * BETA CODE STARTS BELOW
 */

/**
 * Reverses the bit order of the *ba[bit_off : bit_off + bit_len] substring
 * This is done bit by bit
 */
void bitarray_reverse_bit(bitarray_t *ba, size_t bit_off, size_t bit_len) {
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
 /*
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
*/
/**
 * Swaps two bytes
 * Also passing temp pointer so that a temp variable doesn't need to be created in the function
 */
inline void byte_switch(unsigned char *a,unsigned char *b,unsigned char temp) {
  temp = *a;
  *a = *b;
  *b = temp;
}


/*
 * *********************** FINAL CODE STARTS BELOW ***************************
 */

/**
 * Prints out bit_length bits of the bitarray, starting at index bit_off
 */
 /*
void print_bitarray(bitarray_t *ba, size_t bit_off, size_t bit_length) {
  printf("bitarray[%llu:%llu]: ", bit_off, bit_off + bit_length);
  for (size_t i = 0; i < bit_length; i++) {
    printf("%i", bitarray_get(ba, bit_off + i));
    if((bit_off+i+1)%8 == 0) printf(" ");
  }
  printf("\n");
}
*/

/**
 * Retrieves byte at index byte_index from bitarray
 * Perf says this is very slow
 */
inline unsigned char *bitarray_get_byte(bitarray_t *ba, size_t byte_index) {
  //assert(byte_index < ba->bit_sz / 8);
  return (unsigned char *) (ba->buf + byte_index);
}

/**
 * Sets byte at index byte_index of bitarray to val
 */
 /*
inline void bitarray_set_byte(bitarray_t *ba, size_t byte_index, unsigned char val) {
  assert(byte_index < ba->bit_sz / 8);
  ba->buf[byte_index] = val;
}
*/

/**
 * Returns the bit at index bit_index from byte
 */
bool byte_get_bit(unsigned char byte, size_t bit_index) {
  assert(bit_index <= 8);
  return (byte & bitmask(bit_index)) ? true : false;
}

/**
 * Creates a mask that can parse the right k bits from a byte (by using a logical AND)
 */
inline static unsigned char bytemask(size_t k) {
  return 255 << ((8-k) % 8);
}

/**
 * Copies bit_length bits from new_byte to bitarray, starting at location bitarray[bit_off]
 */
// NEEDS TO BE ADDED TO BITARRAY.H
// CAN BE OPTIMIZED WITH MASKING, BUT BEWARE OF LITTLE ENDIAN ISSUES; Will think about later
// PERF SAYS THIS IS VERY SLOW CURRENTLY
void bitarray_set_multiple_bits(bitarray_t *ba, size_t bit_off, size_t bit_length, unsigned char new_byte) {
  assert(bit_length <= 8);
  bool bit;
  for (size_t i = 0; i < bit_length; ++i) {
    bit = byte_get_bit(new_byte, i);
    bitarray_set(ba, i+bit_off, bit);
  }
}

/**
 * Pushes in "shift" bits of carry into a desired bitarray byte and writes the overflowing bits from byte
 * back to carry. A positive shift means that the carry bits are pushed in on the left side of byte,
 * and a negative shift means they are pushed in from the right.
 */
// NEEDS TO BE ADDED TO BITARRAY.H
// NO LONGER USING THIS FUNCTION; it has been manually inlined into bitarray_shift_bytes for perf improvement
// WILL PROBABLY JUST DELETE THIS CODE ALTOGETHER, OR AT LEAST COMMENT IT OUT
/*
void bitarray_shift_byte(bitarray_t *ba, size_t byte_index, ssize_t shift, unsigned char *carry) {
  assert(byte <= ba->buf_sz/8);
  assert(shift < 8);
  assert(shift > -8);

  unsigned char byte = *bitarray_get_byte(ba, byte_index);

  // Carry bits are pushed in from the left
  if (shift > 0) {
    bitarray_set_byte(ba, byte_index, (byte << shift) | (*carry >> (8-shift)));
    *carry = byte & bytemask(shift);
    return;
  }

  // Carry bits are pushed in from the right
  else {
    bitarray_set_byte(ba, byte_index, (byte >> -shift) | (*carry << (8+shift)));
    *carry = byte & ~bytemask(8+shift);
    return;
  }
}
*/

/**
 * Shifts a "byte_length" number of bytes in bitarray (starting at byte_off) by "shift" bits to the right.
 * The carry byte pads the first byte shifted and stores the overflow bits of the last byte shifted. Note
 * that the maximum allowed shift in either direction is one byte.
 */
void bitarray_shift_bytes(bitarray_t *ba, size_t byte_off, size_t byte_length, ssize_t shift, unsigned char * carry) {
  assert(shift < 8);
  assert(shift > -8);

  // POSSIBLE OPPORTUNITY FOR OPTIMIZATION BY REMOVING BRANCHING - BUT HOW? IDK YET
  // This function is only called once, so there will only be one branch
  unsigned char * byte;
  unsigned char temp;
  size_t carry_shift;

  // Shift right
  if (shift > 0) {
    byte = bitarray_get_byte(ba, byte_off);
    carry_shift = 8-shift;
    for (size_t i = 0; i < byte_length; ++i) {
      temp = *byte & bytemask(shift); // THIS IS VERY SLOW
      *byte = (*byte << shift) | (*carry >> carry_shift);
      *carry = temp;
      ++byte;
    }
  }

  // NOT SURE IF THIS WILL IMPROVE PERF, BUT MAYBE TURN A SHIFT-LEFT INTO A SHIFT-RIGHT THAT SETS
  // TO A DIFFERENT BYTE; SHOULDN'T BE TOO HARD TO IMPLEMENT AND MIGHT GET RID OF BRANCHES... MAYBE
  // Shift left
  else {
    byte = bitarray_get_byte(ba, byte_off + byte_length - 1);
    shift = -shift;
    carry_shift = 8-shift;
    for (size_t i = 0; i < byte_length; ++i) {
      temp = *byte & ~bytemask(carry_shift);
      *byte = (*byte >> shift) | (*carry << (carry_shift));
      *carry = temp;
      --byte;
    }
  }
}

void bitarray_reverse(bitarray_t *ba, size_t bit_off, size_t bit_len){
  // Manually reverse bits if the substring is within a single byte
  if (bit_off%8 + bit_len <= 8) {
    bitarray_reverse_bit(ba, bit_off, bit_len);
    return;
  }
  
  // Calculate indices of the partial bytes one each end of the substring.
  // These bits are from index left_start (inclusive) to left_start+left_len (exclusive)
  // and from right_start to right_start+right_len.
  size_t left_start = bit_off;
  size_t left_len = (8 - bit_off) % 8;
  size_t right_len = (bit_off + bit_len) % 8;
  size_t right_start = bit_off + bit_len - right_len;

  // Reverse the order of bits within each byte
  size_t byte_start = (left_start + left_len) / 8;
  size_t byte_end = (right_start / 8); // exclusive

  // SHOULD BE ABLE TO COMBINE THIS FOR LOOP WITH THE NEXT ONE TO INCREASE PERF;
  // WHEN DOING SO, BEWARE OF REVERSING THE MIDDLE BYTE TWICE
  /*
  unsigned char * byte = bitarray_get_byte(ba, byte_start);
  for (size_t i = byte_start; i < byte_end; ++i) {
    byte_reverse(byte);
    ++byte;
  }
  */

  // Reverse the order of all bytes and the order of bits within each byte
  //size_t number_of_swaps = (byte_end + byte_start) / 2;
  unsigned char * left_byte = bitarray_get_byte(ba, byte_start);
  unsigned char * right_byte = bitarray_get_byte(ba, byte_end-1);
  unsigned char temp;
  for (left_byte; left_byte < right_byte; ++left_byte) {
    byte_switch(right_byte, left_byte, temp);
    byte_reverse(left_byte);
    byte_reverse(right_byte);
    --right_byte;
  }
  if(left_byte == right_byte) byte_reverse(left_byte);
  /*
  for (size_t i = byte_start; i < number_of_swaps; ++i) {
    byte_switch(right_byte, left_byte, temp);
    ++left_byte;
    --right_byte;
  }*/

  // Reversal of substring is complete if no partial bytes exist
  if (left_len == 0 & right_len == 0)
    return;

  // Retrieve partial bytes (at each end of the substring) by masking unwanted bits
  unsigned char left_partial_byte;
  unsigned char right_partial_byte;

  // No right partial byte
  if (right_len == 0) {
    left_partial_byte = *bitarray_get_byte(ba, byte_start-1) & bytemask(left_len);
    right_partial_byte = 0;
  }
  // No left partial byte
  else if (left_len == 0) {
    right_partial_byte = *bitarray_get_byte(ba, byte_end) & ~(bytemask(8-right_len));
    left_partial_byte = 0;
  }
  else {
    left_partial_byte = *bitarray_get_byte(ba, byte_start-1) & bytemask(left_len);
    right_partial_byte = *bitarray_get_byte(ba, byte_end) & ~(bytemask(8-right_len));
  }

  // Reverse the order of bits in the partial bytes
  byte_reverse(&left_partial_byte);
  byte_reverse(&right_partial_byte);

  // Swap partial bytes. Requires that all full bytes between the two partial bytes
  // be shifted to make room for the swapping
  ssize_t shift = right_len - left_len;
  unsigned char carry;

  // Need to shift full bytes right
  if (shift > 0) {
    // Shift full bytes
    carry = right_partial_byte;
    bitarray_shift_bytes(ba, byte_start, byte_end - byte_start, shift, &carry);

    // Swap partial bytes
    left_partial_byte = (carry >> (8-shift)) | (left_partial_byte << shift);
    right_partial_byte = right_partial_byte >> (8-right_len);

    // POSSIBLY OPTIMIZE BY IMPROVING THIS FUNCTION; IT CURRENTLY WORKS BY MANUALLY
    // SETTING EACH BIT; CAN WE SET THE ENTIRE BYTE AT ONCE BY SOME COMBINATION OF SHIFTING/MASKING?
    bitarray_set_multiple_bits(ba, bit_off+bit_len-right_len, right_len, left_partial_byte);
    bitarray_set_multiple_bits(ba, left_start, right_len, right_partial_byte);
  }

  // Need to shift full bytes left
  else if (shift < 0) {
    // Shift full bytes
    carry = left_partial_byte;
    bitarray_shift_bytes(ba, byte_start, byte_end - byte_start, shift, &carry);

    // Swap partial bytes
    right_partial_byte = (carry << (8+shift)) | (right_partial_byte >> -shift);
    right_partial_byte = right_partial_byte >> (8-left_len);

    bitarray_set_multiple_bits(ba, left_start, left_len, right_partial_byte);
    bitarray_set_multiple_bits(ba, bit_off+bit_len-left_len, left_len, left_partial_byte);
  }

  // No shift
  else {
    // Swap partial bytes
    right_partial_byte = right_partial_byte >> (8-right_len);

    bitarray_set_multiple_bits(ba, left_start, left_len, right_partial_byte);
    bitarray_set_multiple_bits(ba, bit_off+bit_len - left_len, left_len, left_partial_byte);
  }
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
  bitarray_reverse(ba, bit_off, k);
  bitarray_reverse(ba, bit_off + k, bit_len - k);
  bitarray_reverse(ba, bit_off, bit_len);
}
