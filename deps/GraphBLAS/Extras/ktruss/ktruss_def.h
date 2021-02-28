//------------------------------------------------------------------------------
// ktruss_def.h: definitions for ktruss and allktruss functions
//------------------------------------------------------------------------------

// These functions do not use GraphBLAS

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef MATLAB_MEX_FILE
#include "mex.h"
#include "matrix.h"
#define malloc mxMalloc
#define calloc mxCalloc
#define free   mxFree
#else
#include <omp.h>
#endif

#undef MIN
#undef MAX
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// select the 64-bit version
#define LP64

#if defined (MATLAB_MEX_FILE) || defined (LP64)
typedef int64_t Index ;
#define INDEX_MAX ((Index) ((1ULL << 48)-1))
#else
typedef int32_t Index ;
#define INDEX_MAX ((Index) ((1ULL << 31)-1))
#endif

#define NWORKSPACES 256

bool ktruss_read        // true if successful, false otherwise
(
    FILE *f,            // file for reading, already open (can be stdin)
    int64_t **p_Ap,     // Ap: column pointers, of size n+1
    Index **p_Ai,       // Ai: row indices, of size nz = Ap [n]
    Index *p_n          // A is n-by-n
) ;

int64_t ktruss                  // # steps taken, or <= 0 if error
(
    // input/output:
    int64_t *restrict Ap,       // column pointers, size n+1
    Index   *restrict Ai,       // row indices, size anz = Ap [n]
    // output, content not defined on inpu:
    Index   *restrict Ax,       // values
    // input, not modified:
    const Index n,              // A is n-by-n
    const Index support,        // support = (k-2) for a k-truss, must be > 0
    const int threads,          // # of threads
    const Index chunck          // scheduler chunk size
) ;

int64_t ktruss_ntriangles (const int64_t anz, const Index *Ax) ;

bool allktruss                  // true if successful, false otherwise
(
    // input/output:
    int64_t *restrict Ap,       // column pointers, size n+1
    Index   *restrict Ai,       // row indices, size anz = Ap [n]
    // output, content not defined on input:
    Index   *restrict Ax,       // values
    // input, not modified:
    const Index n,              // A is n-by-n
    const int threads,          // # of threads
    const Index chunk,          // scheduler chunk size

    // output statistics
    int64_t *restrict kmax,     // smallest k where k-truss is empty
    int64_t *restrict ntris,    // size n, ntris [k] is #triangles in k-truss
    int64_t *restrict nedges,   // size n, nedges [k] is #edges in k-truss
    int64_t *restrict nstepss,  // size n, nsteps [k] is #steps for k-truss

    // optional output k-trusses, if present
    int64_t **restrict Cps,     // size n
    Index   **restrict Cis,     // size n
    Index   **restrict Cxs      // size n
) ;

