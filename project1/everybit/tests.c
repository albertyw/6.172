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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "bitarray.h"
#include "ktiming.h"




// Call TEST_PASS() from your test cases to mark a test as successful
//
#define TEST_PASS() TEST_PASS_WITH_NAME(__func__, __LINE__)

#define TEST_PASS_WITH_NAME(name, line) \
    fprintf(stderr, " --> %s at line %d: PASS\n", (name), (line))

// Call TEST_FAIL from your test cases to mark a test as failed. TEST_FAIL
// should print a meaningful message with the reason that the test failed.
//
// Calling it is just like calling printf().
#define TEST_FAIL(failure_msg, args...) \
    TEST_FAIL_WITH_NAME(__func__, __LINE__, failure_msg, ##args)

#define TEST_FAIL_WITH_NAME(name, line, failure_msg, args...) do { \
    fprintf(stderr, " --> %s at line %d: FAIL\n    Reason:", (name), (line)); \
    fprintf(stderr, (failure_msg), ## args); fprintf(stderr, "\n"); \
} while (0)



/* A test case is a function that takes no arguments and returns no arguments. */
typedef void (*test_case_t)(void);

/* Some global variables to make it easier to run individual tests. */
static bitarray_t *test_ba = NULL;
static bool test_verbose = true;

/* Print a string representation of a bitarray. */
static void bitarray_fprint(FILE *f, bitarray_t *ba) {
  size_t i;
  for (i = 0; i < bitarray_get_bit_sz(ba); i++)
    fprintf(f, "%d", bitarray_get(ba, i) ? 1 : 0);
}

/* Return a single random bit. */
static bool randbit(void) {
  return ((rand() % 2) == 0) ? false : true;
}

/* Create a new bitarray in test_ba of the specified size and fill it with random data based on the
seed given. For a given seed number, the pseudorandom data will be the same (at least on the same
glibc implementation). */
static void testutil_newrand(size_t bit_sz, unsigned int seed) {
  size_t i;
  if (test_ba != NULL)
    bitarray_free(test_ba);
  test_ba = bitarray_new(bit_sz);
  assert(test_ba != NULL);
  srand(seed);
  for (i = 0; i < bit_sz; i++)
    bitarray_set(test_ba, i, randbit());
  if (test_verbose) {
    bitarray_fprint(stdout, test_ba);
    fprintf(stdout, " newrand sz=%llu, seed=%u\n", (unsigned long long) bit_sz, seed);
  }
}

static bool boolfromchar(char c) {
  assert(c == '0' || c == '1');
  return c == '1';
}

/* Verify that the specified substring of the test_ba bitarray has the expected number of
flipcounts. Output FAIL or PASS as appropriate for the Python testing script to parse. */
static void testutil_expect_flips(size_t bit_off, size_t bit_len, size_t flipcount) {
  // We do not test this.
  TEST_PASS();
}

/* Verify that the test_ba bitarray has the expected content as well as the expected number
of flipcounts as returned by bitarray_count_flips(). Output FAIL or PASS as appropriate for
the Python testing script to parse. Use the testutil_expect() macro to run this function. */
static void testutil_expect_internal(const char *bitstr, size_t flipcount,
                                     const char* func_name, int line) {
  const char *bad = NULL;
  assert(test_ba != NULL);
  size_t sl = strlen(bitstr), i;
  if (sl != bitarray_get_bit_sz(test_ba))
    bad = "bitarray size";
  for (i = 0; i < sl; i++) {
    if (bitarray_get(test_ba, i) != boolfromchar(bitstr[i]))
      bad = "bitarray content";
  }
  if (bad != NULL) {
    bitarray_fprint(stdout, test_ba);
    fprintf(stdout, " expect bits=%s \n", bitstr);
    TEST_FAIL(func_name, "incorrect %s", bad);
  } else {
    TEST_PASS_WITH_NAME(func_name, line);
  }
}

/* Invoke the testutil_expect_internal() function with the right code line number specified. */
#define testutil_expect(bitstr, flipcount) \
    testutil_expect_internal((bitstr), (flipcount), __func__, __LINE__)

/* Create a new bitarray in test_ba by parsing a string of 0s and 1s, e.g. "0101011011". */
static void testutil_frmstr(const char *bitstr) {
  size_t sl = strlen(bitstr), i;
  if (test_ba != NULL)
    bitarray_free(test_ba);
  test_ba = bitarray_new(sl);
  assert(test_ba != NULL);
  size_t myflipcount = 0;
  bool curbit, prevbit = false;
  for (i = 0; i < sl; i++) {
    curbit = boolfromchar(bitstr[i]);
    if (i != 0 && curbit != prevbit)
      myflipcount++;
    bitarray_set(test_ba, i, curbit);
    prevbit = curbit;
  }
  bitarray_fprint(stdout, test_ba);
  if (test_verbose) {
    fprintf(stdout, " newstr lit=%s\n", bitstr);
    testutil_expect(bitstr, myflipcount);
  }
}

/* Peform a rotate operation on test_ba. */
static void testutil_rotate(size_t bit_off, size_t bit_len, ssize_t bit_right_amount) {
  assert(test_ba != NULL);
  bitarray_rotate(test_ba, bit_off, bit_len, bit_right_amount);
  if (test_verbose) {
    bitarray_fprint(stdout, test_ba);
    fprintf(stdout, " rotate off=%llu, len=%llu, amnt=%lld\n",
        (unsigned long long) bit_off, (unsigned long long) bit_len, (long long) bit_right_amount);
  }
}

/* A sample long-running set of rotation operations. */
double longrunning_rotation(void) {
  test_verbose = false;
  size_t bit_sz = 12 * 1024 * 8 + 471;
  testutil_newrand(bit_sz, 0);
  clockmark_t time1 = ktiming_getmark();
  testutil_rotate(0, bit_sz, -bit_sz / 4);
  testutil_rotate(0, bit_sz, bit_sz / 4);
  testutil_rotate(0, bit_sz, bit_sz / 2);
  clockmark_t time2 = ktiming_getmark();
  return ktiming_diff_usec(&time1, &time2) / 1000000000.0;
}

/* A sample long-running set of flip count operations. */
/*
double longrunning_flipcount(void) {
  test_verbose = false;
  size_t bit_sz = 128 * 1024 * 1024 * 8 + 531;
  testutil_newrand(bit_sz, 0);
  clockmark_t time1 = ktiming_getmark();
  for (int i = 0; i < 20; i++)
    bitarray_count_flips(test_ba, 0, bit_sz);
  clockmark_t time2 = ktiming_getmark();
  return ktiming_diff_usec(&time1, &time2) / 1000000000.0;
}
*/
/* ----------- Actual test methods go here ----------- */

static void test_headerexamples(void) {
  /* Verify the examples given in bitarray.h. */
  testutil_frmstr("10010110");
  testutil_expect("10010110", 5);
  testutil_rotate(0, 8, -1);
  testutil_expect("00101101", 5);
  testutil_frmstr("10010110");
  testutil_rotate(2, 5, 2);
  testutil_expect("10110100", 5);
}

static void test_8bit(void) {
  testutil_frmstr("10000101");
  testutil_rotate(0, 8, 0);
  testutil_expect("10000101", 4);
  testutil_rotate(0, 8, 1);
  testutil_expect("11000010", 3);
  testutil_rotate(0, 8, -1);
  testutil_expect("10000101", 4);
  testutil_rotate(0, 8, -1);
  testutil_expect("00001011", 3);
  testutil_rotate(0, 8, -(3 + 8));
  testutil_expect("01011000", 4);
}

static void test_moreflips(void) {
  testutil_frmstr("110001010100101101011"); /* 21 bits */
  testutil_expect_flips(0, 21, 14);
  testutil_expect_flips(0, 20, 14);
  testutil_expect_flips(0, 19, 13);
  testutil_frmstr("100000000000000101011"); /* 21 bits */
  testutil_expect_flips(0, 21, 6);
  testutil_expect_flips(0, 20, 6);
  testutil_expect_flips(0, 19, 5);
}

/* Tests for when bitarray size is not a multiple of 8 */
static void test_non8bits(void) {
  testutil_frmstr("100000110");
  testutil_rotate(2,3,-3);
  testutil_expect("100000110", 3);
  testutil_rotate(0,9, 3);
  testutil_expect("110100000", 3);
}

/* Tests for when bitarray is null */
static void test_nullbitarray(void) {
  testutil_frmstr("");
  testutil_rotate(2,3,-3);
  testutil_expect("", 0);
}

/* Tests for when bit_len is zero */
static void test_zerolength(void) {
  testutil_frmstr("101010");
  testutil_rotate(2,0,4);
  testutil_expect("101010", 5);
}

/* Tests reverse using bytes */
static void test_bytereverse(void) {
  testutil_frmstr("01100110011111111101");
  testutil_rotate(1,16,8);
  testutil_expect("01111111100110011101", 7);
}

static void test_bytereverse2(void) {
  testutil_frmstr("011101011");
  testutil_rotate(0,9,4);
  testutil_expect("110101110", 5);
}

static void test_bytereverse3(void) {
  testutil_frmstr("0000110011001111111100000000110011001111");
  testutil_rotate(2,32,3);
  testutil_expect("0011001100000000111111110011001100001111", 11);
}

static void test_bytereverse4(void) {
  testutil_frmstr("01100110011111111101");
  testutil_rotate(7,13,8);
  testutil_expect("01100111011111111100", 7);
}

static void test_byteshift(void) {
  testutil_frmstr("0000100011111111000000001111111100000000111111111000000011111111");
  testutil_rotate(4,46,0);
  testutil_expect("000000001111111100100000111111110000000011111111", 7);
}

static void test_byteshift2(void) {
  testutil_frmstr("00000000101101111100110011111111000000001111111101010101");
  testutil_rotate(4,46,0);
  testutil_expect("00000000111111110010000011111111000000001111111101010101", 7);
}


static void test_naive(void) {
  testutil_frmstr("0"); /* 1 bit */
  testutil_rotate(0, 1, 0);
  testutil_expect("0", 0);
  testutil_rotate(0, 1, 1);
  testutil_expect("0", 0);
  testutil_rotate(0, 1, -1);
  testutil_expect("0", 0);

  testutil_frmstr("1"); /* 1 bit */
  testutil_rotate(0, 1, 0);
  testutil_expect("1", 0);
  testutil_rotate(0, 1, 1);
  testutil_expect("1", 0);
  testutil_rotate(0, 1, -1);
  testutil_expect("1", 0);
}

static void test_8bit_clean(void) {
  testutil_frmstr("00000000");
  testutil_rotate(0, 1, 0);
  testutil_expect("00000000", 0);
  testutil_rotate(0, 1, 1);
  testutil_expect("00000000", 0);
  testutil_rotate(0, 1, -1);
  testutil_expect("00000000", 0);

  testutil_frmstr("11111111");
  testutil_rotate(0, 1, 0);
  testutil_expect("11111111", 0);
  testutil_rotate(0, 1, 1);
  testutil_expect("11111111", 0);
  testutil_rotate(0, 1, -1);
  testutil_expect("11111111", 0);
}

static void test_7bit_strange(void) {
  testutil_frmstr("0110010");
  testutil_rotate(0, 3, 1);
  testutil_expect("1010010", 5);
  testutil_rotate(0, 3, -1);
  testutil_expect("0110010", 4);
  testutil_rotate(0, 7, 5);
  testutil_expect("1001001", 4);
  testutil_rotate(0, 6, -3);
  testutil_expect("1001001", 4);
}

static void test_all_2bit(void) {
  testutil_frmstr("01");

  testutil_rotate(0, 1, 1);
  testutil_expect("01", 1);
  testutil_rotate(1, 1, 1);
  testutil_expect("01", 1);

  testutil_rotate(0, 2, 0);
  testutil_expect("01", 1);
  testutil_rotate(0, 2, 2);
  testutil_expect("01", 1);
  testutil_rotate(0, 2, -2);
  testutil_expect("01", 1);

  testutil_rotate(0, 2, 1);
  testutil_expect("10", 1);
  testutil_rotate(0, 2, -1);
  testutil_expect("01", 1);

  testutil_rotate(0, 2, -1);
  testutil_expect("10", 1);
  testutil_rotate(0, 2, 1);
  testutil_expect("01", 1);

  testutil_rotate(0, 2, 3);
  testutil_expect("10", 1);
  testutil_rotate(0, 2, -3);
  testutil_expect("01", 1);

  testutil_rotate(0, 2, -3);
  testutil_expect("10", 1);
  testutil_rotate(0, 2, 3);
  testutil_expect("01", 1);  
}

static void test_uneven_offset(void) {
  testutil_frmstr("0000111100001111000011110000111100001111000011110000"); //52 bits

  testutil_rotate(4, 44, 8);
  testutil_expect("0000000011111111000011110000111100001111000011110000", 12);

  testutil_rotate(4, 44, 4);
  testutil_expect("0000111100001111111100001111000011110000111100000000", 12);


  testutil_frmstr("0000111100001111000011110000111100001111000011110000");
  testutil_rotate(4, 44, 3);
  testutil_expect("0000111111100001111000011110000111100001111000010000", 12);

  testutil_frmstr("0000111100001111000011110000111100001111000011110000");
  testutil_rotate(4, 44, 7);
  testutil_expect("0000000111111110000111100001111000011110000111100000", 10);
}







test_case_t test_cases[] = {
  test_headerexamples,
  test_8bit,
  test_moreflips,
  // ADD YOUR TEST CASES HERE
  test_non8bits,
  test_nullbitarray,
  //test_bytereverse,
  //test_bytereverse2,
  //test_bytereverse3,
  //test_bytereverse4,
  //test_byteshift,
  test_naive,
  test_8bit_clean,
  test_7bit_strange,
  test_all_2bit,
  test_uneven_offset,

  NULL // This marks the end of all test cases. Don't change this!
};
