/* Copyright (C) 2006 Bradley C. Kuszmaul.
 * Modified for Cilk++ by Pablo Halpern, May 2009
 * This code is licensed under the Gnu General Public License (GPL).
 */

/* Matrices are laid out in row-major order.
 * For example,  if C is an N by M matrix, then
 *               C_{1,1} is stored at C[0]
 *               C_{1,2} is stored at C[1]
 *               C_{2,1} is stored at C[M]
 * and in general C_{i,j} is stored at C[ (i-1)*M + j-1 ].
 *
 * Most matrix multiplication routines are written on column-major
 * order to be compatible with FORTRAN.  This one is in row-major
 * order, which is compatible with C and C++ arrays.
 *
 * In C, arrays are indexed from 0 to N^2-1.
 * In Math, matrices are indexed from 1 to N in each axis.
 *
 * This code works even if the matrices are not a power of 2 in size.
 * This code could be adapted to work for non-square matrices.
 */

#include <iostream>
#include <cstring>
#include <cstdlib>

#include <cilk/cilk.h>
#include <cilktools/cilkview.h>

/* mm_loop_serial is the standard triply nested loop implementation.
 *   C += A*B
 * n is the matrix size.  Note that C is not set to zero -- its initial value
 * is added to the matrix-product of B and C */
template <typename T>
void mm_loop_serial(T *C, const T *A, const T *B, int n)
{
    /* DO NOT MODIFY THIS CODE.  THIS IS USED TO VERIFY THAT YOUR
     * CODE IS PRODUCING THE RIGHT ANSWER. */
    for (int i=0; i<n; i++)
        for (int j=0; j<n; j++)
            for (int k=0; k<n; k++)
                C[i*n+j] += A[i*n+k] * B[k*n+j];
}

/* mm_loop_parallel is the parallel implementation of mm_loop_serial */
template <typename T>
void mm_loop_parallel(T *C, const T *A, const T *B, int n)
{
    /* MODIFY THIS CODE TO MAKE IT PARALLEL */
    for (int i=0; i<n; i++)
        cilk_for (int j=0; j<n; j++)
            for (int k=0; k<n; k++)
                C[i*n+j] += A[i*n+k] * B[k*n+j];
}


/* Here are some tests. */

int test_status = 0;

template <typename T>
void copy_matrix(T *dest, const T *src, int n)
{
    for (int i=0; i < n*n; ++i)
        dest[i] = src[i];
}

template <typename T>
bool are_equal_matrices(const T *a, const T *b, int n)
{
    for (int i=0; i < n*n; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}

/* Test to see if mm_loop_parallel and mm_loop_serial do the same thing. */
template <typename T>
void test_mm(const T *C, const T *A, const T *B, int n,
             const char* test_name, bool verify = true, bool timer = true)
{
    T *Cp = new T[n*n];
    copy_matrix(Cp, C, n);

    if (timer)
    {

        cilkview_data_t start, end;
        __cilkview_query(start);
        //do work
        mm_loop_parallel(Cp,A,B,n);
        
        __cilkview_query(end);
        __cilkview_do_report(&start, &end, (char *)test_name, CV_REPORT_WRITE_TO_LOG | CV_REPORT_WRITE_TO_RESULTS);

        std::cout << test_name << " time: " << end.time - start.time
                  << "ms" << std::endl;
    }
    else
        mm_loop_parallel(Cp,A,B,n);

    if (verify)
    {
        T *Cs = new T[n*n];
        copy_matrix(Cs, C, n);
        mm_loop_serial(Cs,A,B,n);
        if (!are_equal_matrices(Cp,Cs,n))
        {
            std::cout << ">>>>> " << test_name << " failed <<<<<" << std::endl;
            ++test_status;
        }
        delete [] Cs;
    }

    delete [] Cp;
}


/* Do a simple check and make sure it is right. */ 
void smoke_test1()
{
    // Second element of each array is never used and should not change
    double C[] = {1.,-1};
    double A[] = {5.,-1};
    double B[] = {7.,-1};

    mm_loop_parallel(C,A,B,1);

    if (C[0]!=36. || C[1]!=-1
     || A[0]!=5.  || A[1]!=-1
     || B[0]!=7.  || B[1]!=-1)
    {
        std::cout << ">>>>> " << "smoke_test1 failed <<<<<" << std::endl;
        ++test_status;
    }
}

/* Another simple check. */
void smoke_test2()
{
    // last element of each array is never used and should not change
    double c[] = {5,6,7,8,-1};
    double a[] = {9,10,11,12,-1};
    double b[] = {13,14,15,16,-1};

    mm_loop_parallel(c,a,b,2);

    if (c[4]!=-1|| a[4]!=-1|| b[4]!=-1
     || c[0]!=5+ 9*13+10*15 || c[1]!=6+ 9*14+10*16
     || c[2]!=7+11*13+12*15 || c[3]!=8+11*14+12*16)
    {
        std::cout << ">>>>> " << "smoke_test2 failed <<<<<" << std::endl;
        ++test_status;
    }
}

/* A bigger test. */
void random_test(int n, const char *test_name, bool verify, bool timer)
{
    double *a = new double[n*n];
    double *b = new double[n*n];
    double *c = new double[n*n];

    for (int i=0; i<n*n; i++)
    {
	// Range of Random doubles chosen to avoid roundoff error.
        c[i] = (std::rand() & 0x1ffff) / 4.0;
        a[i] = (std::rand() & 0x1ffff) / 4.0;
        b[i] = (std::rand() & 0x1ffff) / 4.0;
    }

    test_mm(c,a,b,n,test_name,verify,timer);

    delete [] c;
    delete [] b;
    delete [] a;
}

/* return true iff n = 2^k. */
bool is_power_of_2 (int n)
{
    bool match = false; /* whether a bit has been matched. */
    int field = 0x1;    /* field for bit matching. */
    for (int i = sizeof(int) * 8; i > 0; --i, field <<= 1)
    {
        if (field & n)
        {
            if (match) return false;
            match = true;
        }
    }
    return match;
}

/* The arguments to mm:
 *   --verify    run the verification tests.
 *   --notime    don't measure run times.
 *   --pause     pause at the end of the run
 *   N           (a number) run a matrix multiply on an
 *               NxN matrix, without checking that the
 *               answer is correct.  If N is absent, run a
 *               a standard test suite.
 */
int main (int argc, char *argv[])
{
    int N=-1;
    bool do_verify=false, do_time=true, do_pause=false;
    const char *N_str=NULL;
    while (argc>1)
    {
        char *arg = argv[argc-1];
        if (std::strcmp(arg,"--verify")==0)
        {
            do_verify=true;
        }
        else if (std::strcmp(arg,"--notime")==0)
        {
            do_time=false;
        }
        else if (std::strcmp(arg,"--pause")==0)
        {
            do_pause=true;
        }
        else if ('0'<=arg[0] && arg[0]<='9')
        {
            N_str=arg;
            /* Doesn't check the syntax of arg. */
            N = std::atoi(arg);
            if (N <= 0)
            {
                std::cout << "Illegal option: " << N_str << std::endl;
                return 1;
            }
            else if (!is_power_of_2(N))
            {
                std::cout << "N must be a power of 2" << std::endl;
                return 1;
            }
        }
        argc--;
    }

    /* Smoke tests. */
    smoke_test1();
    smoke_test2();

    /* The third arg says verify the answer.  */
    random_test(4,"smoke_test4",true,false);
    random_test(64,"smoke_test64",true,false);

    if (N_str)
    {
        char test_name[25] = "matrix";
        std::strncat(test_name, N_str, 12);
        random_test(N, test_name, do_verify, do_time);
    }
    else
    {
        random_test(2, "matrix2", do_verify, do_time);
        random_test(256, "matrix256", do_verify, do_time);
        random_test(1024, "matrix1024", do_verify, do_time);
    }

    std::cout << "Test " << (test_status ? "failed" : "succeeded")
              << std::endl;

    if (do_pause)
    {
        // Prevent Microsoft Visual Studio output window from disappearing.
        std::cout << "Press <ENTER> to continue..." << std::endl;
        std::cin.get();
    }

    return test_status;
}
