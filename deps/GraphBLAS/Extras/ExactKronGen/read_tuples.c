//------------------------------------------------------------------------------
// read_tuples:  reads in a set if I,J,X tuples
//------------------------------------------------------------------------------

// Timothy A. Davis, (c) 2018, All Rights Reserved.  License: Apache 2.0
// (same as GraphBLAS)

// Usage:
//
//      GrB_Index *I, *J, ntuples, len, nrows, ncols ;
//      double *X ;
//      GrB_Info info ;
//      FILE *f = stdin ;    // for example
//      info = read_tuples (f, &I, &J, &X, &ntuples, &len, &nrows, &ncols) ;
//
// I, J, X and undefined on input.  On output, they are NULL if an error
// occurred.  If info == GrB_SUCCESS, then I, J, and X point to newly allocated
// space containing the list of tuples read in from the file f.  ntuples are
// the number of tuples read in, and len is the size of I, J, and X (in terms
// of the # of tuples they could hold).  The tuples in the file are assumed
// to be 1-based, but they are returned in I,J,X as zero-based.   On input, all
// arguments must be non-NULL pointers.

#include "kron.h"

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
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (f == NULL || I_handle == NULL || J_handle == NULL || X_handle == NULL ||
        p_ntuples == NULL || p_len == NULL || p_nrows == NULL ||
        p_ncols == NULL)
    {
        return (GrB_NULL_POINTER) ;
    }

    *I_handle = NULL ;
    *J_handle = NULL ;
    *X_handle = NULL ;
    (*p_ntuples) = 0 ;
    (*p_len) = 0 ;
    (*p_nrows) = 0 ;
    (*p_ncols) = 0 ;

    //--------------------------------------------------------------------------
    // allocate initial space for tuples
    //--------------------------------------------------------------------------

    int64_t len = 256 ;
    GrB_Index *I = malloc (len * sizeof (int64_t)), *I2 = NULL ;
    GrB_Index *J = malloc (len * sizeof (int64_t)), *J2 = NULL ;
    double    *X = malloc (len * sizeof (double )), *X2 = NULL ;
    if (I == NULL || J == NULL || X == NULL)
    {
        // out of memory
        if (I != NULL) free (I) ;
        if (J != NULL) free (J) ;
        if (X != NULL) free (X) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // read in the tuples from stdin, one per line
    //--------------------------------------------------------------------------

    // format warnings vary with compilers, so read in as double
    double ii, jj, x ;
    GrB_Index nrows = 0 ;
    GrB_Index ncols = 0 ;
    GrB_Index ntuples = 0 ;
    while (fscanf (f, "%lg %lg %lg\n", &ii, &jj, &x) != EOF)
    {
        int64_t i = (int64_t) ii ;
        int64_t j = (int64_t) jj ;
        if (ntuples >= len)
        {
            I2 = realloc (I, 2 * len * sizeof (int64_t)) ;
            J2 = realloc (J, 2 * len * sizeof (int64_t)) ;
            X2 = realloc (X, 2 * len * sizeof (double)) ;
            if (I2 == NULL || J2 == NULL || X2 == NULL)
            {
                free (I) ;
                free (J) ;
                free (X) ;
                return (GrB_OUT_OF_MEMORY) ;
            }
            I = I2 ; I2 = NULL ;
            J = J2 ; J2 = NULL ;
            X = X2 ; X2 = NULL ;
            len = len * 2 ;
        }
        if (i > nrows) nrows = i ;
        if (j > ncols) ncols = j ;
        // tuples in file are 1-based so convert to 0-based
        i-- ;
        j-- ;
        I [ntuples] = i ;
        J [ntuples] = j ;
        X [ntuples] = x ;
        ntuples++ ;
    }

    //--------------------------------------------------------------------------
    // return results
    //--------------------------------------------------------------------------

    *I_handle = I ;
    *J_handle = J ;
    *X_handle = X ;
    (*p_ntuples) = ntuples ;
    (*p_len) = len ;
    (*p_nrows) = nrows ;
    (*p_ncols) = ncols ;
    return (GrB_SUCCESS) ;
}

