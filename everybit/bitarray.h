#ifndef BITARRAY_H
#define BITARRAY_H

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

/* Abstract Data Type (ADT) representing an array of bits. Individual bits in the array can be
accessed through the accessor functions bitarray_get()/bitarray_set(). Additionally, some operations
that operate on substrings of bits are provided (bitarray_rotate() and bitarray_count_flips()). */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct bitarray bitarray_t;

/* Allocate a new bitarray for storing bit_sz bits. */
bitarray_t *bitarray_new(size_t bit_sz);

/* Free a bitarray allocated by bitarray_new(). */
void bitarray_free(bitarray_t *ba);

/* Return the number of bits stored a bitarray, as given by the bit_sz argument to
bitarray_new(). */
size_t bitarray_get_bit_sz(bitarray_t *ba);

/* Index into the bitarray and retrieve the bit at the specified zero-based index. */
bool bitarray_get(bitarray_t *ba, size_t bit_index);

/* Index into the bitarray and return a pointer to the byte at the specified zero-based index. */
char *bitarray_get_byte(bitarray_t *ba, size_t byte_index);

/* Index into the bitarray and set the bit at the specified zero-based index to the specified
value. */
void bitarray_set(bitarray_t *ba, size_t bit_index, bool val);


/* Count the number of bit transitions in the substring of bits at zero-based indices between
bit_off (inclusive) and bit_off+bit_len (exclusive). For instance, to count the number of bit
transitions in the entire bitarray 10010110, invoke
bitarray_count_flips(ba, 0, bitarray_get_bit_sz(ba)); this will yield 5 (one transition per dot in
"1.00.1.0.11.0"). */
size_t bitarray_count_flips(bitarray_t *ba, size_t bit_off, size_t bit_len);


/* Perform a rotate operation on the substring of bits at zero-based indices between bit_off
(inclusive) and bit_off+bit_len (exclusive). The rotate distance is specified by bit_right_amount.
Positive values of bit_right_amount will cause bits to be rotated right, negative values will cause
bits to be rotated left. For instance, to rotate an entire bitarray 10010110 (stored in the variable
ba) left by one bit, invoke bitarray_rotate(ba, 0, bitarray_get_bit_sz(ba), -1); this would yield
00101101. Instead invoking bitarray_rotate(ba, 2, 5, 2) would yield 10110100. */
void bitarray_rotate(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount);

/* Reverses a bitarray mainly using bytes, but falls back on bitarray_reverse_bit in case the
reversal length is short */
inline void bitarray_reverse(bitarray_t *ba, size_t bit_off, size_t bit_len);

/* Does the same thing as bitarray_rotate except that it does it bit by bit */
void bitarray_rotate_bit(bitarray_t *ba, size_t bit_off, size_t bit_len, ssize_t bit_right_amount);

/* Perform a reverse operation on the substring of bits at zero-based indices between bit_off
(inclusive) and bit_off+bit_len (exclusive). 
This is done by manipulating individual bits*/
inline void bitarray_reverse_bit(bitarray_t *ba, size_t bit_off, size_t b_len);

#endif /* BITARRAY_H */
