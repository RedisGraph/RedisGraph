//------------------------------------------------------------------------------
// tri_def.h:  definitions for tri_* triangle counting methods
//------------------------------------------------------------------------------

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifdef MATLAB_MEX_FILE
#include "mex.h"
#include "matrix.h"
#endif

#if 0
// 32-bit version
// row and column indices, and matrix dimension, must be < 2^31.  The number
// of entries in the matrix can be up to 2^63 (Lp has type int64_t).
typedef int32_t Index ;
#define INDEX_MAX INT32_MAX
#endif

// 64-bit version
typedef int64_t Index ;
#define INDEX_MAX INT64_MAX

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

// ceil(a/b) for two integers a and b
#define CEIL(a,b) ( ((a) + (b) - 1) / (b) )

// S=tril(A), S=triu(A), and permuted variants
bool tri_prep                       // true if successful, false otherwise
(
    int64_t *restrict Sp,           // column pointers, size n+1
    Index   *restrict Si,           // row indices
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index *restrict Ai,       // row indices
    const Index n,                  // A is n-by-n
    int method,
    int nthreads                    // # of threads to use
) ;


//------------------------------------------------------------------------------
// 8 versions created by tri_template:
//------------------------------------------------------------------------------

// parallel version with bool Mark array
int64_t tri_mark_parallel           // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n,                  // A is n-by-n
    const int threads,              // # of threads
    const Index chunk               // scheduler chunk size
) ;

// sequential version with bool Mark array
int64_t tri_mark                    // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n                   // A is n-by-n
) ;

// parallel version with bit Mark array
int64_t tri_bit_parallel            // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n,                  // A is n-by-n
    const int threads,              // # of threads
    const Index chunk               // scheduler chunk size
) ;

// sequential version with bit Mark array
int64_t tri_bit                     // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n                   // A is n-by-n
) ;

// parallel version with bool Mark array
int64_t tri_logmark_parallel           // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n,                  // A is n-by-n
    const int threads,              // # of threads
    const Index chunk               // scheduler chunk size
) ;

// sequential version with bool Mark array
int64_t tri_logmark                    // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n                   // A is n-by-n
) ;

// parallel version with bit Mark array
int64_t tri_logbit_parallel            // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n,                  // L is n-by-n
    const int threads,              // # of threads
    const Index chunk               // scheduler chunk size
) ;

// sequential version with bit Mark array
int64_t tri_logbit                     // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n                   // A is n-by-n
) ;

//------------------------------------------------------------------------------

// find the first and last row index in a column
static inline bool tri_lohi         // true if column has any entries
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices
    const Index j,                  // get info on column j
    Index *lo,                      // first row index in column j
    Index *hi                       // last row index in column j
)
{
    int64_t p1 = Ap [j] ;
    int64_t p2 = Ap [j+1] ;
    if (p1 < p2)
    {
        // column j has at least one entry.  Return the first and last entry.
        (*lo) = Ai [p1] ;
        (*hi) = Ai [p2-1] ;
        return (true) ;
    }
    else
    {
        // column j is empty
        return (false) ;
    }
}

bool tri_read           // true if successful, false otherwise
(
    FILE *f,            // file for reading, already open (can be stdin)
    int64_t **p_Ap,     // Ap: column pointers, of size n+1
    Index **p_Ai,       // Ai: row indices, of size nz = Ap [n]
    Index *p_n          // A is n-by-n
) ;

//------------------------------------------------------------------------------
// two versions created by tri_dot_template
//------------------------------------------------------------------------------

int64_t tri_dot                     // # of triangles
(
    const int64_t *restrict Lp,     // column pointers of L, size n+1
    const Index   *restrict Li,     // row indices of L
    const int64_t *restrict Up,     // column pointers of U, size n+1
    const Index   *restrict Ui,     // row indices of U
    const Index n                   // L and U are n-by-n
) ;

int64_t tri_dot_parallel            // # of triangles
(
    const int64_t *restrict Lp,     // column pointers of L, size n+1
    const Index   *restrict Li,     // row indices of L
    const int64_t *restrict Up,     // column pointers of U, size n+1
    const Index   *restrict Ui,     // row indices of U
    const Index n,                  // L and U are n-by-n
    const int threads,              // # of threads
    const Index chunk               // scheduler chunk size
) ;

//------------------------------------------------------------------------------
// tri_simple: simplest method, not parallel
//------------------------------------------------------------------------------

int64_t tri_simple          // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Lp,     // column pointers, size n+1
    const Index   *restrict Li,     // row indices
    const Index   n                 // L is n-by-n
) ;

