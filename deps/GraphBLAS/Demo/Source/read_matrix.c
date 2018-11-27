//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/read_matrix.c: read a matrix from stdin
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reads a matrix from stdin.  For sample inputs, see the Matrix/* files.
// Each line has the form:
//
//      i j x
//
// where i and j are the row and column indices, and x is the value.
// The matrix is read in double precision.

// free all workspace; this used by the OK(...) macro if an error occurs
#define FREE_ALL                    \
    if (I  != NULL) free (I) ;      \
    if (J  != NULL) free (J) ;      \
    if (X  != NULL) free (X) ;      \
    if (I2 != NULL) free (I2) ;     \
    if (J2 != NULL) free (J2) ;     \
    if (X2 != NULL) free (X2) ;     \
    GrB_free (&scale2_op) ;         \
    GrB_free (&dt2) ;               \
    GrB_free (&dt1) ;               \
    GrB_free (&A) ;                 \
    GrB_free (&B) ;                 \
    GrB_free (&C) ;

#include "demos.h"

//------------------------------------------------------------------------------
// unary operator to divide by 2
//------------------------------------------------------------------------------

void scale2 (double *z, const double *x)
{
    (*z) = (*x) / 2.0 ;
}

//------------------------------------------------------------------------------
// read a matrix from a file
//------------------------------------------------------------------------------

GrB_Info read_matrix        // read a double-precision or boolean matrix
(
    GrB_Matrix *A_output,   // handle of matrix to create
    FILE *f,                // file to read the tuples from
    bool make_symmetric,    // if true, return A as symmetric
    bool no_self_edges,     // if true, then remove self edges from A
    bool one_based,         // if true, input matrix is 1-based
    bool boolean,           // if true, input is GrB_BOOL, otherwise GrB_FP64
    bool pr                 // if true, print status to stdout
)
{

    int64_t len = 256 ;
    int64_t ntuples = 0 ;
    double x ;
    GrB_Index nvals ;

    //--------------------------------------------------------------------------
    // set all pointers to NULL so that FREE_ALL can free everything safely
    //--------------------------------------------------------------------------

    GrB_Matrix C = NULL, A = NULL, B = NULL ;
    GrB_Descriptor dt1 = NULL, dt2 = NULL ;
    GrB_UnaryOp scale2_op = NULL ;

    //--------------------------------------------------------------------------
    // allocate initial space for tuples
    //--------------------------------------------------------------------------

    size_t xsize = ((boolean) ? sizeof (bool) : sizeof (double)) ;
    GrB_Index *I = malloc (len * sizeof (int64_t)), *I2 = NULL ;
    GrB_Index *J = malloc (len * sizeof (int64_t)), *J2 = NULL ;
    void *X = malloc (len * xsize) ;
    bool *Xbool ;
    double *Xdouble ;
    void *X2 = NULL ;
    if (I == NULL || J == NULL || X == NULL)
    {
        // out of memory
        if (pr) printf ("out of memory for initial tuples\n") ;
        FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    Xbool   = (bool   *) X ;
    Xdouble = (double *) X ;

    //--------------------------------------------------------------------------
    // read in the tuples from stdin, one per line
    //--------------------------------------------------------------------------

    // format warnings vary with compilers, so read in as double
    double i2, j2 ;
    while (fscanf (f, "%lg %lg %lg\n", &i2, &j2, &x) != EOF)
    {
        int64_t i = (int64_t) i2 ;
        int64_t j = (int64_t) j2 ;
        if (ntuples >= len)
        {
            I2 = realloc (I, 2 * len * sizeof (int64_t)) ;
            J2 = realloc (J, 2 * len * sizeof (int64_t)) ;
            X2 = realloc (X, 2 * len * xsize) ;
            if (I2 == NULL || J2 == NULL || X2 == NULL)
            {
                if (pr) printf ("out of memory for tuples\n") ;
                FREE_ALL ;
                return (GrB_OUT_OF_MEMORY) ;
            }
            I = I2 ; I2 = NULL ;
            J = J2 ; J2 = NULL ;
            X = X2 ; X2 = NULL ;
            len = len * 2 ;
            Xbool   = (bool   *) X ;
            Xdouble = (double *) X ;
        }
        if (one_based)
        {
            i-- ;
            j-- ;
        }
        I [ntuples] = i ;
        J [ntuples] = j ;
        if (boolean)
        {
            Xbool [ntuples] = (x != 0) ;
        }
        else
        {
            Xdouble [ntuples] = x ;
        }
        ntuples++ ;
    }

    //--------------------------------------------------------------------------
    // find the dimensions
    //--------------------------------------------------------------------------

    if (pr) printf ("ntuples: %.16g\n", (double) ntuples) ;
    int64_t nrows = 0 ;
    int64_t ncols = 0 ;
    for (int64_t k = 0 ; k < ntuples ; k++)
    {
        nrows = MAX (nrows, I [k]) ;
        ncols = MAX (ncols, J [k]) ;
    }
    nrows++ ;
    ncols++ ;

    if (pr) printf ("nrows %.16g ncols %.16g\n",
        (double) nrows, (double) ncols) ;

    //--------------------------------------------------------------------------
    // prune self edges
    //--------------------------------------------------------------------------

    // but not if creating the augmented system aka a bipartite graph
    double tic [2], t1 ;
    simple_tic (tic) ;
    if (no_self_edges && ! (make_symmetric && nrows != ncols))
    {
        int64_t ntuples2 = 0 ;
        for (int64_t k = 0 ; k < ntuples ; k++)
        {
            if (I [k] != J [k])
            {
                // keep this off-diagonal edge
                I [ntuples2] = I [k] ;
                J [ntuples2] = J [k] ;
                if (boolean)
                {
                    Xbool [ntuples2] = Xbool [k] ;
                }
                else
                {
                    Xdouble [ntuples2] = Xdouble [k] ;
                }
                ntuples2++ ;
            }
        }
        ntuples = ntuples2 ;
    }
    t1 = simple_toc (tic) ;
    if (pr) printf ("time to prune self-edges: %12.6f\n", t1) ;

    //--------------------------------------------------------------------------
    // build the matrix, summing up duplicates, and then free the tuples
    //--------------------------------------------------------------------------

    GrB_Type xtype ;
    GrB_BinaryOp xop, xop2 ;
    if (boolean)
    {
        xtype = GrB_BOOL ;
        xop   = GrB_LOR ;
        xop2  = GrB_FIRST_BOOL ;
    }
    else
    {
        xtype = GrB_FP64 ;
        xop   = GrB_PLUS_FP64 ;
        xop2  = GrB_FIRST_FP64 ;
    }

    simple_tic (tic) ;
    GrB_Info info ;
    OK (GrB_Matrix_new (&C, xtype, nrows, ncols)) ;

    if (boolean)
    {
        OK (GrB_Matrix_build (C, I, J, Xbool, ntuples, xop)) ;
    }
    else
    {
        OK (GrB_Matrix_build (C, I, J, Xdouble, ntuples, xop)) ;
    }
    t1 = simple_toc (tic) ;
    if (pr) printf ("time to build the graph with GrB_Matrix_build: %12.6f\n",
        t1) ;

#ifdef TEST_SETELEMENT
    {
        // This is just for testing performance of GrB_setElement and comparing
        // with GrB_build.  It is not needed if this function is used in 
        // production.

        // setElement will be just about as fast as build (perhaps 10% to 50%
        // more time) with non-blocking mode.  If blocking mode is enabled,
        // setElement will be extremely and painfully slow since the matrix is
        // rebuilt every time a single entry is added.

        simple_tic (tic) ;
        OK (GrB_Matrix_new (&B, xtype, nrows, ncols)) ;
        for (int64_t k = 0 ; k < ntuples ; k++)
        {
            // B (I[k], J[k]) = X [k]
            GrB_Matrix_setElement (B, X [k], I [k], J [k]) ;
        }
        // force completion of B
        GrB_Matrix_nvals (&nvals, B) ;
        double t2 = simple_toc (tic) ;
        if (pr) printf ("time to build the graph with GrB_setElement:"
            "   %12.6f\n", t2) ;
        GrB_free (&B) ;
    }
#endif

    free (I) ; I = NULL ;
    free (J) ; J = NULL ;
    free (X) ; X = NULL ;

    //--------------------------------------------------------------------------
    // construct the descriptors
    //--------------------------------------------------------------------------

    // descriptor dt2: transpose the 2nd input
    OK (GrB_Descriptor_new (&dt2)) ;
    OK (GrB_Descriptor_set (dt2, GrB_INP1, GrB_TRAN)) ;

    // descriptor dt1: transpose the 1st input
    OK (GrB_Descriptor_new (&dt1)) ;
    OK (GrB_Descriptor_set (dt1, GrB_INP0, GrB_TRAN)) ;

    //--------------------------------------------------------------------------
    // create the output matrix
    //--------------------------------------------------------------------------

    if (make_symmetric)
    {

        //----------------------------------------------------------------------
        // ensure the matrix is symmetric
        //----------------------------------------------------------------------

        if (pr) printf ("make symmetric\n") ;
        if (nrows == ncols)
        {

            //------------------------------------------------------------------
            // A = (C+C')/2
            //------------------------------------------------------------------

            if (pr) printf ("A = (C+C')/2\n") ;
            double tic [2], t ;
            simple_tic (tic) ;
            OK (GrB_Matrix_new (&A, xtype, nrows, nrows)) ;
            OK (GrB_eWiseAdd (A, NULL, NULL, xop, C, C, dt2)) ;
            OK (GrB_free (&C)) ;

            if (boolean)
            {
                *A_output = A ;
                A = NULL ;
            }
            else
            {
                OK (GrB_Matrix_new (&C, xtype, nrows, nrows)) ;
                OK (GrB_UnaryOp_new (&scale2_op, scale2, xtype, xtype)) ;
                OK (GrB_apply (C, NULL, NULL, scale2_op, A, NULL)) ;
                OK (GrB_free (&A)) ;
                OK (GrB_free (&scale2_op)) ;
                *A_output = C ;
                C = NULL ;
            }

            t = simple_toc (tic) ;
            if (pr) printf ("A = (C+C')/2 time %12.6f\n", t) ;

        }
        else
        {

            //------------------------------------------------------------------
            // A = [0 C ; C' 0], a bipartite graph
            //------------------------------------------------------------------

            // no self edges will exist
            if (pr) printf ("A = [0 C ; C' 0], a bipartite graph\n") ;

            double tic [2], t ;
            simple_tic (tic) ;

            int64_t n = nrows + ncols ;
            OK (GrB_Matrix_new (&A, xtype, n, n)) ;

            GrB_Index I_range [3], J_range [3] ;

            I_range [GxB_BEGIN] = 0 ;
            I_range [GxB_END  ] = nrows-1 ;

            J_range [GxB_BEGIN] = nrows ;
            J_range [GxB_END  ] = ncols+nrows-1 ;

            // A (nrows:n-1, 0:nrows-1) += C'
            OK (GrB_assign (A, NULL, xop2, // or NULL,
                C, J_range, GxB_RANGE, I_range, GxB_RANGE, dt1)) ;

            // A (0:nrows-1, nrows:n-1) += C
            OK (GrB_assign (A, NULL, xop2, // or NULL,
                C, I_range, GxB_RANGE, J_range, GxB_RANGE, NULL)) ;

            // force completion; if this statement does not appear, the
            // timing will not account for the final build, which would be
            // postponed until A is used by the caller in another GraphBLAS
            // operation.
            GrB_Matrix_nvals (&nvals, A) ;
            t = simple_toc (tic) ;

            if (pr) printf ("time to construct augmented system: %12.6f\n", t) ;
            *A_output = A ;
            // set A to NULL so the FREE_ALL macro does not free *A_output
            A = NULL ;

        }
    }
    else
    {

        //----------------------------------------------------------------------
        // return the matrix as-is
        //----------------------------------------------------------------------

        if (pr) printf ("leave A as-is\n") ;
        *A_output = C ;
        // set C to NULL so the FREE_ALL macro does not free *A_output
        C = NULL ;
    }

    //--------------------------------------------------------------------------
    // success: free everything except the result, and return it to the caller
    //--------------------------------------------------------------------------

    FREE_ALL ;
    if (pr) printf ("\nMatrix from file:\n") ;
    GxB_print (*A_output, pr ? GxB_SHORT : GxB_SILENT) ;
    return (GrB_SUCCESS) ;
}

