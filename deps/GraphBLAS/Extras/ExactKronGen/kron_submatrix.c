//------------------------------------------------------------------------------
// kron_submatrix: construct a submatrix of C=kron(A,B)
//------------------------------------------------------------------------------

// if np > 1, the output Cfilename is prepended with "<pid>_"

#include "kron.h"

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                        \
    GrB_free (&my_A) ;                  \
    GrB_free (&B) ;                     \
    GrB_free (&my_C) ;                  \
    if (Afile != NULL) fclose (Afile) ; \
    if (Bfile != NULL) fclose (Bfile) ; \
    if (Cfile != NULL) fclose (Cfile) ; \
    if (Ai != NULL) free (Ai) ;         \
    if (Aj != NULL) free (Aj) ;         \
    if (Ax != NULL) free (Ax) ;         \
    if (Bi != NULL) free (Bi) ;         \
    if (Bj != NULL) free (Bj) ;         \
    if (Bx != NULL) free (Bx) ;         \
    if (my_Ci != NULL) free (my_Ci) ;   \
    if (my_Cj != NULL) free (my_Cj) ;   \
    if (my_Cx != NULL) free (my_Cx) ;   \
    GrB_finalize ( ) ;

// OK(method) is a macro that calls a GraphBLAS method and checks the status;
// if a failure occurs, it prints the detailed error message, frees all
// allocated workspace and returns the error code.
#define OK(method)                                          \
{                                                           \
    info = method ;                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))    \
    {                                                       \
        fprintf (stderr, "file %s line %d\n", __FILE__, __LINE__) ;  \
        fprintf (stderr, "%s\n", GrB_error ( )) ;                    \
        FREE_ALL ;                                          \
        return (1) ;                                        \
    }                                                       \
}

int kron_submatrix      // 0: success, 1: failure
(
    char *Afilename,    // filename with triplets of A 
    char *Bfilename,    // filename with triplets of B
    char *Cfilename,    // filename with output triplets C
    int np,             // # of threads being used, must be > 0
    int pid,            // id of this thread (in range 0 to np-1)
    int btype           // 0: no triangles, 1: lots, 2: some
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    fprintf (stderr, "\nkron: np:%d pid:%d btype:%d\n", np, pid, btype) ;
    if (Afilename == NULL || Bfilename == NULL || Cfilename == NULL)
    {
        fprintf (stderr, "one or more filenames are null\n") ;
        return (1) ;
    }
    if (np < 1 || pid < 0 || pid >= np)
    {
        fprintf (stderr, "np must be >= 1, and pid in range 0 to np-1\n") ;
        return (1) ;
    }
    if (btype < 0 || btype > 2)
    {
        fprintf (stderr, "invalid btype, must be 0, 1, or 2\n") ;
        return (1) ;
    }

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Matrix my_A = NULL, B = NULL, my_C = NULL ;
    GrB_Index *my_Ci = NULL, *my_Cj = NULL ;
    FILE *Afile = NULL, *Bfile = NULL, *Cfile = NULL ;
    double *my_Cx = NULL ;
    GrB_Info info ;
    double tic [2], t_read, t_convert, t_kron, t_write ;
    GrB_Index *Ai = NULL, *Aj = NULL, A_ntuples = 0, A_len = 0 ;
    GrB_Index *Bi = NULL, *Bj = NULL, B_ntuples = 0, B_len = 0 ;
    double *Ax = NULL ;
    double *Bx = NULL ;

    #define CLEN 2048
    char Cfilename2 [CLEN+1] ;

    // adjust the my_C output file name if np > 1
    if (np > 1)
    {
        snprintf (Cfilename2, CLEN, "%d_%s", pid, Cfilename) ;
        Cfilename = Cfilename2 ;
    }

    OK (GrB_init (GrB_NONBLOCKING)) ;

    //--------------------------------------------------------------------------
    // get A and B tuples from input files
    //--------------------------------------------------------------------------

    fprintf (stderr, "    A file: %s\n", Afilename) ;
    fprintf (stderr, "    B file: %s\n", Bfilename) ;
    fprintf (stderr, "    C file: %s\n", Cfilename) ;

    simple_tic (tic) ;
    Afile = fopen (Afilename, "r") ;
    Bfile = fopen (Bfilename, "r") ;
    Cfile = fopen (Cfilename, "w") ;
    if (Afile == NULL || Bfile == NULL || Cfile == NULL)
    {
        fprintf (stderr, "unable to read input files or create output file\n") ;
        FREE_ALL ;
        return (1) ;
    }

    GrB_Index A_nrows, A_ncols, B_nrows, B_ncols ;
    OK (read_tuples (Afile,
        &Ai, &Aj, &Ax, &A_ntuples, &A_len, &A_nrows, &A_ncols)) ;
    OK (read_tuples (Bfile,
        &Bi, &Bj, &Bx, &B_ntuples, &B_len, &B_nrows, &B_ncols)) ;
    fclose (Afile) ;
    fclose (Bfile) ;
    Afile = NULL ;
    Bfile = NULL ;
    t_read = simple_toc (tic) ;
    fprintf (stderr, "    kron [pid:%d]: time to read A and B: %g sec\n",
        pid, t_read) ;

    //--------------------------------------------------------------------------
    // determine my_A, the submatrix of A for this thread
    //--------------------------------------------------------------------------

    int64_t ntuples_per_thread = A_ntuples / np ;
    int64_t my_first_A_tuple   = pid * ntuples_per_thread ;
    int64_t my_last_A_tuple    = (pid == np-1) ?
        (A_ntuples-1) : ((pid+1) * ntuples_per_thread - 1) ;
    int64_t my_A_ntuples = my_last_A_tuple - my_first_A_tuple + 1 ;

    // get the tuples of my submatrix of A
    GrB_Index *my_Ai = Ai + my_first_A_tuple ;
    GrB_Index *my_Aj = Aj + my_first_A_tuple ;
    double    *my_Ax = Ax + my_first_A_tuple ;

    // find the first and last column of my submatrix of A
    int64_t my_Aj_min = -1, my_Aj_max = -2 ;
    if (np == 1)
    {
        my_Aj_min = 0 ;
        my_Aj_max = A_ncols-1 ;
    }
    else
    {
        // note that if my_A_ntuples == 0 then range is [-1,-2]
        for (int64_t k = 0 ; k < my_A_ntuples ; k++)
        {
            int64_t j = my_Aj [k] ;
            if (k == 0)
            {
                my_Aj_min = j ;
                my_Aj_max = j ;
            }
            else
            {
                my_Aj_min = MIN (my_Aj_min, j) ;
                my_Aj_max = MAX (my_Aj_max, j) ;
            }
        }
    }

    // my_A_ncols is zero if my_A has no tuples
    int64_t my_A_ncols = my_Aj_max - my_Aj_min + 1 ;

    // shift the column indices of my submatrix of A to start at column zero
    if (my_Aj_min > 0)
    {
        for (int64_t k = 0 ; k < my_A_ntuples ; k++)
        {
            my_Aj [k] -= my_Aj_min ;
        }
    }

    //--------------------------------------------------------------------------
    // convert my_A and B to GraphBLAS matrices
    //--------------------------------------------------------------------------

    simple_tic (tic) ;
    OK (GrB_Matrix_new (&my_A, GrB_FP64, A_nrows, my_A_ncols)) ;
    OK (GrB_Matrix_new (&B, GrB_FP64, B_nrows, B_ncols)) ;

    OK (GrB_Matrix_build (my_A, my_Ai, my_Aj, my_Ax, my_A_ntuples,
        GrB_PLUS_FP64)) ;
    OK (GrB_Matrix_build (B, Bi, Bj, Bx, B_ntuples, GrB_PLUS_FP64)) ;

    // free the A and B tuples
    free (Ai) ; Ai = NULL ;
    free (Aj) ; Aj = NULL ;
    free (Ax) ; Ax = NULL ;
    free (Bi) ; Bi = NULL ;
    free (Bj) ; Bj = NULL ;
    free (Bx) ; Bx = NULL ;
    t_convert = simple_toc (tic) ;
    fprintf (stderr, "    kron [pid:%d]: time to convert A and B: %g sec\n",
        pid, t_convert) ;

    //--------------------------------------------------------------------------
    // handle the btype
    //--------------------------------------------------------------------------

    if (btype == 1)
    {
        if (pid == 0)
        {
            // add a single self-loop: my_A (0,0) = 1
            fprintf (stderr, "    add my_A (0,0)=1\n") ;
            OK (GrB_Matrix_setElement (my_A, 1, 0, 0)) ;
        }
        // add a single self-loop: B (0,0) = 1
        fprintf (stderr, "    add B (0,0)=1\n") ;
        OK (GrB_Matrix_setElement (B, 1, 0, 0)) ;
    }
    else if (btype == 2)
    {
        if (pid == np-1)
        {
            // add a single self-loop: my_A (end,end) = 1
            fprintf (stderr, "    add my_A (end,end)=1\n") ;
            OK (GrB_Matrix_setElement (my_A, 1, A_nrows-1, my_A_ncols-1)) ;
        }
        // add a single self-loop: B (end,end) = 1
        fprintf (stderr, "    add B (end,end)=1\n") ;
        OK (GrB_Matrix_setElement (B, 1, B_nrows-1, B_ncols-1)) ;
    }

    GrB_Index my_A_nvals, B_nvals ;
    OK (GrB_Matrix_nvals (&my_A_nvals, my_A)) ;
    OK (GrB_Matrix_nvals (&B_nvals, B)) ;

    //--------------------------------------------------------------------------
    // my_C = kron (my_A,B)
    //--------------------------------------------------------------------------

    simple_tic (tic) ;
    OK (GrB_Matrix_new (&my_C,
        GrB_FP64, A_nrows * B_nrows, my_A_ncols * B_ncols));
    OK (GxB_kron (my_C, NULL, NULL, GrB_TIMES_FP64, my_A, B, NULL)) ;
    t_kron = simple_toc (tic) ;

    OK (GrB_free (&my_A)) ;
    OK (GrB_free (&B)) ;

    //--------------------------------------------------------------------------
    // report results
    //--------------------------------------------------------------------------

    GrB_Index C_nrows, my_C_ncols, my_C_nvals ;
    OK (GrB_Matrix_nrows (&C_nrows, my_C)) ;
    OK (GrB_Matrix_ncols (&my_C_ncols, my_C)) ;
    OK (GrB_Matrix_nvals (&my_C_nvals, my_C)) ;

    fprintf (stderr, "    GraphBLAS GxB_kron [pid:%d]:\n"
    "    my_A: %lld-by-%lld, %lld entries.\n"
    "    B: %lld-by-%lld, %lld entries.\n"
    "    my_C: %lld-by-%lld, %lld entries.\n"
    "    time: %g seconds, rate: nval(C)/t = %g million/sec\n", pid,
    A_nrows, my_A_ncols, my_A_nvals,
    B_nrows,    B_ncols,    B_nvals,
    C_nrows, my_C_ncols, my_C_nvals,
    t_kron, 1e-6*((double) my_C_nvals) / t_kron) ;

    //--------------------------------------------------------------------------
    // write my_C to the output file
    //--------------------------------------------------------------------------

    simple_tic (tic) ;
    my_Ci = (GrB_Index *) malloc ((my_C_nvals+1) * sizeof (GrB_Index)) ;
    my_Cj = (GrB_Index *) malloc ((my_C_nvals+1) * sizeof (GrB_Index)) ;
    my_Cx = (double    *) malloc ((my_C_nvals+1) * sizeof (double   )) ;
    if (my_Ci == NULL || my_Cj == NULL || my_Cx == NULL)
    {
        fprintf (stderr, "out of memory\n") ;
        FREE_ALL ;
        return (1) ;
    }

    OK (GrB_Matrix_extractTuples (my_Ci, my_Cj, my_Cx, &my_C_nvals, my_C)) ;

    // shift the column indices of my_C to reflect columns in C instead
    GrB_Index my_Cj_min = my_Aj_min * B_ncols ;

    for (int64_t k = 0 ; k < my_C_nvals ; k++)
    {
        int64_t ci = my_Ci [k] ;
        int64_t cj = my_Cj [k] ;

        // handle the btype
        if (btype != 0)
        {
            if (my_Aj_min == 0)
            {
                // remove self-loop C (0,0)
                if (ci == 0 && cj == 0) continue ;
            }
            if (my_Aj_max == A_ncols-1)
            {
                // remove self-loop C (end,end)
                if (ci == C_nrows-1 && cj == my_C_ncols-1) continue ; 
            }
        }

        fprintf (Cfile, "%lld\t%lld\t%.17g\n",
            1 + ci, 1 + cj + my_Cj_min, my_Cx [k]) ;
    }

    // success
    FREE_ALL ;
    t_write = simple_toc (tic) ;
    fprintf (stderr, "    kron [pid:%d]: time to write C: %g sec\n",
        pid, t_write) ;
    return (0) ;
}

