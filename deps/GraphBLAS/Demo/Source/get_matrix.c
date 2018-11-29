//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/get_matrix.c: get matrix from file, or create random
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Creates a symmetric matrix, either from a file or by creating a random
// matrix.  If reading from a file, the file is assumed to be 0-based.

#define FREE_ALL        \
    GrB_free (&A) ;     \
    GrB_free (&desc) ;  \
    GrB_free (&Mask) ;

#include "demos.h"

GrB_Info get_matrix         // get a matrix from stdin, or create random one
(
    GrB_Matrix *A_output,   // matrix to create
    int argc,               // command-line arguments
    char **argv,
    bool no_self_edges,     // if true, ensure the matrix has no self-edges
    bool boolean            // if true, file is read as GrB_BOOL, else GrB_FP64
)
{

    GrB_Info info ;
    GrB_Index nrows = 1, ncols = 1, ntuples = 1, nvals ;
    GrB_Matrix A = NULL ;
    GrB_Matrix Mask = NULL ;
    GrB_Descriptor desc = NULL ;
    int kind = 0 ;

    double tic [2], t ;
    simple_tic (tic) ;

    if (argc > 2)
    {

        //----------------------------------------------------------------------
        // create a random matrix
        //----------------------------------------------------------------------

        kind = strtol (argv [1], NULL, 0) ;

        if (kind == 0)
        {

            //------------------------------------------------------------------
            // random pattern
            //------------------------------------------------------------------

            // usage:  ./main 0 nrows ncols ntuples method

            int method = 0 ;    //  0:setElement, 1:build

            if (argc > 2) nrows   = strtol (argv [2], NULL, 0) ;
            if (argc > 3) ncols   = strtol (argv [3], NULL, 0) ;
            if (argc > 4) ntuples = strtol (argv [4], NULL, 0) ;
            if (argc > 5) method  = strtol (argv [5], NULL, 0) ;

            OK (random_matrix (&A, true, no_self_edges,
                nrows, ncols, ntuples, method, false)) ;

            // force completion, just to check timing
            GrB_Matrix_nvals (&nvals, A) ;

            t = simple_toc (tic) ;

            // printf format warnings can vary with different compilers, so
            // punt and type cast to double
            printf (
                "random %.16g by %.16g, nz: %.16g, method %d time %.3f sec\n",
                (double) nrows, (double) ncols, (double) nvals, method, t) ;

            fprintf (stderr,
                "random %.16g by %.16g, nz: %.16g, method %d time %.3f sec\n",
                (double) nrows, (double) ncols, (double) nvals, method, t) ;

        }
        else
        {

            //------------------------------------------------------------------
            // Wathen matrix
            //------------------------------------------------------------------

            // usage:  ./main 1 nx ny method

            int method = 0 ;        // 0 to 3
            int64_t nx = 4, ny = 4 ;
            if (argc > 2) nx     = strtol (argv [2], NULL, 0) ;
            if (argc > 3) ny     = strtol (argv [3], NULL, 0) ;
            if (argc > 4) method = strtol (argv [4], NULL, 0) ;

            OK (wathen (&A, nx, ny, false, method, NULL)) ;
            GrB_Matrix_nvals (&nvals, A) ;
            GrB_Matrix_nrows (&nrows, A) ;
            t = simple_toc (tic) ;

            printf (
                "Wathen: nx %.16g ny %.16g n %.16g nz %.16g method %d,"
                " time: %.3f sec\n", (double) nx, (double) ny, (double) nrows,
                (double) nvals, method, t) ;

            fprintf (stderr,
                "Wathen: nx %.16g ny %.16g n %.16g nz %.16g method %d,"
                " time: %.3f sec\n", (double) nx, (double) ny, (double) nrows,
                (double) nvals, method, t) ;

            // remove the self edges from the matrix
            if (no_self_edges)
            {
                // Mask = speye (nrows) ;
                OK (GrB_Matrix_new (&Mask, GrB_BOOL, nrows, nrows)) ;
                for (int64_t i = 0 ; i < nrows ; i++)
                {
                    // Mask (i,i) = true
                    OK (GrB_Matrix_setElement (Mask, (bool) true, i, i)) ;
                }
                // A<~Mask> = A, thus removing the diagonal.  GrB_transpose
                // does C<Mask>=A', so setting inp0 to tran does C=A'', and
                // thus C<Mask>=A, without forming any transpose at all.
                // Replace is set, so A is cleared first.  Otherwise the
                // diagonal is not touched by C<~Mask>=A.
                OK (GrB_Descriptor_new (&desc)) ;
                OK (GrB_Descriptor_set (desc, GrB_INP0, GrB_TRAN)) ;
                OK (GrB_Descriptor_set (desc, GrB_MASK, GrB_SCMP)) ;
                OK (GrB_Descriptor_set (desc, GrB_OUTP, GrB_REPLACE)) ;
                OK (GrB_transpose (A, Mask, NULL, A, desc)) ;
                GrB_free (&Mask) ;
                GrB_free (&desc) ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // read a 0-based or 1-based matrix from stdin
        //----------------------------------------------------------------------

        // usage:  ./main   < file
        //         ./main 0 < file
        //         ./main 1 < file
        //
        // default is 0-based, for the matrices in the Matrix/ folder

        bool one_based = false ;
        if (argc > 1) one_based = strtol (argv [1], NULL, 0) ;

        OK (read_matrix (&A, stdin, true, no_self_edges, one_based, boolean,
            false)) ;

        OK (GrB_Matrix_nrows (&nrows, A)) ;
        OK (GrB_Matrix_ncols (&ncols, A)) ;
        OK (GrB_Matrix_nvals (&nvals, A)) ;

        printf ("matrix %.16g by %.16g, %.16g entries, from stdin\n",
            (double) nrows, (double) ncols, (double) nvals) ;

        fprintf (stderr, "matrix %.16g by %.16g, %.16g entries, from stdin\n",
            (double) nrows, (double) ncols, (double) nvals) ;

    }

    //--------------------------------------------------------------------------
    // print and return result
    //--------------------------------------------------------------------------

    // print a short description of the matrix (about 30 entries)
    OK (GxB_Matrix_fprint (A, "from get_matrix:", GxB_SHORT, stdout)) ;

    *A_output = A ;
    A = NULL ;
    return (GrB_SUCCESS) ;
}

