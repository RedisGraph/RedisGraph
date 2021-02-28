//------------------------------------------------------------------------------
// allktruss_mex.c:  construct all k-trusses of a graph
//------------------------------------------------------------------------------

// usage:  [stats,AllC] = allktruss_mex (A)

// This function computes the set of all k-trusses.  It is identical to
// allktruss.m; see that function for a complete description.

#include "ktruss_def.h"

static const char *stat_fields [ ] = { "kmax", "ntris", "nedges", "nsteps" } ;

mxArray *createstat (int64_t kmax, int64_t *stat)
{
    mxArray *x = mxCreateDoubleMatrix (1, kmax, mxREAL) ;
    double *p = mxGetPr (x) ;
    for (int64_t i = 1 ; i <= kmax ; i++)
    {
        p [i-1] = stat [i] ;
    }
    return (x) ;
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    
    // check inputs
    if (nargin != 1 || nargout > 2)
    {
        mexErrMsgTxt ("usage: [stats,AllC] = allktruss_mex (A)") ;
    }

    // get inputs
    int64_t n = (int64_t) mxGetN (pargin [0]) ;
    int64_t *Ap = (int64_t *) mxGetJc (pargin [0]) ;
    int64_t *Ai = (int64_t *) mxGetIr (pargin [0]) ;
    int64_t nnz = Ap [n] ;

    // create input for allktruss
    int64_t *Cp = (int64_t *) mxMalloc ((n+1) * sizeof (int64_t)) ;
    int64_t *Ci = (int64_t *) mxMalloc ((nnz+1) * sizeof (int64_t)) ;
    int64_t *Ce = (int64_t *) mxMalloc ((nnz+1) * sizeof (int64_t)) ;
    memcpy (Cp, Ap, (n+1) * sizeof (int64_t)) ;
    memcpy (Ci, Ai, (nnz) * sizeof (int64_t)) ;

    // create the output statistics
    int64_t kmax ;
    int64_t *ntris   = (int64_t *) mxMalloc ((n+1) * sizeof (int64_t)) ;
    int64_t *nedges  = (int64_t *) mxMalloc ((n+1) * sizeof (int64_t)) ;
    int64_t *nstepss = (int64_t *) mxMalloc ((n+1) * sizeof (int64_t)) ;

    // create the arrays to hold all output k-trusses
    int64_t **Cps = NULL ;
    int64_t **Cis = NULL ;
    int64_t **Cxs = NULL ;
    if (nargout > 1)
    {
        Cps = (int64_t **) mxMalloc ((n+1) * sizeof (int64_t *)) ;
        Cis = (int64_t **) mxMalloc ((n+1) * sizeof (int64_t *)) ;
        Cxs = (int64_t **) mxMalloc ((n+1) * sizeof (int64_t *)) ;
    }

    // construct all ktrusses
    allktruss (Cp, Ci, Ce, n, 1, n, &kmax, ntris, nedges, nstepss,
        Cps, Cis, Cxs) ;

    // create the MATLAB cell array for all k-trusses
    if (nargout > 1)
    {
        // printf ("create all %ld ktrusses\n", kmax) ;
        pargout [1] = mxCreateCellMatrix (1, kmax) ;

        for (int64_t k = 1 ; k <= 2 ; k++)
        {
            // there is no 1-truss or 2-truss
            mxArray *C = mxCreateDoubleMatrix (0, 0, mxREAL) ;
            mxSetCell (pargout [1], k-1, C) ;
        }

        // the kmax-truss is empty
        mxArray *C = mxCreateSparse (n, n, 0, mxREAL) ;
        mxSetCell (pargout [1], kmax-1, C) ;

        // non-empty k-trusses
        for (int64_t k = 3 ; k <= kmax-1 ; k++)
        {
            // copy the k-truss into the MATLAB cell array
            int64_t *Sp = Cps [k] ;
            int64_t *Si = Cis [k] ;
            int64_t *Se = Cxs [k] ;
            int64_t snz = Sp [n] ;
            mxArray *C = mxCreateSparse (n, n, snz, mxREAL) ;
            memcpy (mxGetJc (C), Sp, (n+1) * sizeof (int64_t)) ;
            memcpy (mxGetIr (C), Si, snz   * sizeof (int64_t)) ;
            double *Cx = mxGetPr (C) ;
            for (int64_t p = 0 ; p < snz ; p++)
            {
                Cx [p] = (double) (Se [p]) ;
            }
            mxFree (Sp) ;
            mxFree (Si) ;
            mxFree (Se) ;
            mxSetCell (pargout [1], k-1, C) ;
        }
    }

    mxFree (Cp) ;
    mxFree (Ci) ;
    mxFree (Ce) ;

    // return statistics
    pargout [0] = mxCreateStructMatrix (1, 1, 4, stat_fields) ;
    mxSetFieldByNumber (pargout [0], 0, 0,
            mxCreateDoubleScalar ((double) kmax)) ;
    mxSetFieldByNumber (pargout [0], 0, 1, createstat (kmax, ntris)) ;
    mxSetFieldByNumber (pargout [0], 0, 2, createstat (kmax, nedges)) ;
    mxSetFieldByNumber (pargout [0], 0, 3, createstat (kmax, nstepss)) ;
    mxFree (ntris) ;
    mxFree (nedges) ;
    mxFree (nstepss) ;
}

