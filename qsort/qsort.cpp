/*
 * qsort.cilk
 *
 * Copyright (c) 2007, 2008 Cilk Arts, Inc.  All rights reserved.
 *
 * An implementation of quicksort using Cilk parallelization.
 */

#include <iostream>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cstdlib>
#include <ctime>

#include <cilktools/cilkview.h>
#include <cilk/cilk.h>

// Uncomment this to enable intentional race
// #define INTENTIONAL_RACE

using namespace std;

// Sort the range between bidirectional iterators begin and end.
// end is one past the final element in the range.
// Use the Quick Sort algorithm, using recursive divide and conquer.
template <typename Iter>
void sample_qsort(Iter begin, Iter end) {

    typedef typename iterator_traits<Iter>::value_type T;

    if (begin != end) {

        // get last element
        T last = *(end - 1);

        // Partition array using last element of array as pivot
        // (move elements less than last to lower partition
        // and elements not less than last to upper partition
        // return middle = the first element not less than last
        Iter middle = std::partition(begin, end - 1,
                                     bind2nd(less<T>(), last));

        // move pivot to middle
        std::swap(*(end - 1), *middle);

        // sort lower partition
#ifdef INTENTIONAL_RACE
        // INTENTIONAL RACE: Ranges overlap
        cilk_spawn sample_qsort(begin, std::min(middle + 2, end - 1));
#else
        cilk_spawn sample_qsort(begin, middle);
#endif
        // sort upper partition (excluding pivot)
        sample_qsort(middle + 1, end);
        cilk_sync;
    }
}

void printArray(const int *a, size_t n)
{
    assert(a > 0);
    cout << "a: (" << a[0];
    for (int i = 1; i < n; ++i) {
        cout << ", " << a[i];
    }
    cout << ")" << endl;
}

// A simple test harness.  Program takes 2 optional arguments:
//   First argument specifies the length of the array to sort.
//     Defaults to 10 million.
//   Second argument specifies the number of trials to run.
//     Defaults to 1.
int main(int argc, char **argv)
{
    int *a = NULL, failCount = 0;
    bool failFlag;

    // get number of integers to sort, default 50 million
    int n = 50*1000*1000;
    if (argc > 1) {
      n = atoi(argv[1]);
    }
    cout << "Sorting " << n << " integers" << endl;

    // get number of trials, default to 1
    int numTrials = 1;
    if (argc > 2) {
        numTrials = atoi(argv[2]);
    }
    cout << "Running " << numTrials << " trials" << endl;

    // check arguments
    if (n < 1 || numTrials < 1) {
        cerr << "array length and number of trials must be positive" << endl;
        exit(-1);
    }

    // allocate memory for array
    a = new int[n];
    if (!a) {
        cerr << "array allocation failed" << endl;
        exit(-1);
    }

    // initialize to 0 to n-1
    for (int i = 0; i < n; ++i) {
      a[i] = i;
    }

    long long int total_time = 0;
    cilkview_data_t start, end;
    // for each trial, shuffle data and sort it
    for (int j = 0; j < numTrials; ++j) {
        // shuffle the data
        std::random_shuffle(a, a + n);

#ifdef DEBUG
        printArray(a, n);
#endif

        // run quicksort algorithm
        __cilkview_query(start);
        sample_qsort(a, a + n);
        __cilkview_query(end);
        total_time += (end.time - start.time);

#ifdef DEBUG
        printArray(a, n);
#endif

        // Confirm that a is sorted and that each element contains the index.
        failFlag = false;
        for (int i = 0; i < n; ++i) {
            if ( a[i] != i ) {
#ifdef DEBUG
                cout << "Sort failed at location i=" << i << " a[i] = "
                    << a[i] << endl;
#endif
                failFlag = true;
            }
        }
        if (failFlag) {
            ++failCount;
        }
    }

    if (failCount == 0) {
        cout << "All sorts succeeded" << endl;
    } else {
        cout << failCount << " sorts failed" << endl;
    }

    // report time
    //same problem as nbodies for report writing TODO
    cout << "Sort time: " << total_time / 1000.0
              << " seconds" << endl;
    __cilkview_do_report(&start, &end, "qsort", 
        CV_REPORT_WRITE_TO_LOG | CV_REPORT_WRITE_TO_RESULTS);
    
    // free integer array
    delete[] a;

    return failCount;
}
