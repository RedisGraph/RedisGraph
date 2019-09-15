//------------------------------------------------------------------------------
// kron.h: include file for kron.c main program
//------------------------------------------------------------------------------

// Timothy A. Davis, (c) 2018, All Rights Reserved.  License: Apache 2.0
// (same as GraphBLAS)

#include <stdio.h>
#include "GraphBLAS.h"
#include "simple_timer.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

GrB_Info read_tuples      // read a file of tuples
(
    // input: file must be already open
    FILE *f,                // file to read the tuples from

    // output: not defined on input
    GrB_Index **I_handle,           // row indices (in range 0 to nrows-1)
    GrB_Index **J_handle,           // column indices (in range 0 to ncols-1)
    double **X_handle,              // values
    GrB_Index *p_ntuples,           // number of tuples read in
    GrB_Index *p_len,               // length of I, J, X
    GrB_Index *p_nrows,             // 1 + max (I)
    GrB_Index *p_ncols              // 1 + max (J)
) ;

int kron_submatrix      // 0: success, 1: failure
(
    char *Afilename,    // filename with triplets of A 
    char *Bfilename,    // filename with triplets of B
    char *Cfilename,    // filename with output triplets C
    int np,             // # of threads being used, must be > 0
    int pid,            // id of this thread (in range 0 to np-1)
    int btype           // 0: no triangles, 1: lots, 2: some
) ;

