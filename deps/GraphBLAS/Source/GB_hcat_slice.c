//------------------------------------------------------------------------------
// GB_hcat_slice: horizontal concatenation of the slices of C
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Horizontal concatenation of slices into the matrix C.

#include "GB_mxm.h"

GrB_Info GB_hcat_slice      // horizontal concatenation of the slices of C
(
    GrB_Matrix *Chandle,    // output matrix C to create
    int nthreads,           // # of slices to concatenate
    GrB_Matrix *Cslice,     // array of slices of size nthreads
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (nthreads > 1) ;
    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;
    ASSERT (Cslice != NULL) ;
    for (int tid = 0 ; tid < nthreads ; tid++)
    {
        ASSERT_OK (GB_check (Cslice [tid], "a slice of C", GB0)) ;
        ASSERT (!GB_PENDING (Cslice [tid])) ;
        ASSERT (!GB_ZOMBIES (Cslice [tid])) ;
        ASSERT ((Cslice [tid])->is_hyper) ;
        // each Cslice [tid] is constructed as its own matrix, with Cslice
        // [tid] = A * Bslice [tid].  It is not a slice of an other matrix, so
        // Cslice [tid]->is_slice is false.
        ASSERT (!(Cslice [tid])->is_slice) ;
        ASSERT ((Cslice [tid])->type == (Cslice [0])->type) ;
        ASSERT ((Cslice [tid])->vlen == (Cslice [0])->vlen) ;
        ASSERT ((Cslice [tid])->vdim == (Cslice [0])->vdim) ;
    }

    //--------------------------------------------------------------------------
    // find the size and type of C
    //--------------------------------------------------------------------------

    // Let cnz_slice [tid] be the number of entries in Cslice [tid], and let
    // cnvec_slice [tid] be the number vectors in Cslice [tid].  Then Cnzs and
    // Cnvecs are cumulative sums of cnz_slice and cnvec_slice, respectively:

    // Cnzs   [tid] = sum of cnz_slice   [0:tid-1]
    // Cnvecs [tid] = sum of cnvec_slice [0:tid-1]

    // both arrays are size nthreads+1.  Thus, both Cnzs [0] and Cnvecs [0] are
    // zero, and their last entries are the total # entries and vectors in C,
    // respectively.

    int64_t Cnzs   [nthreads+1] ;
    int64_t Cnvecs [nthreads+1] ;

    // all the slices have the same type and dimension
    GrB_Type ctype = (Cslice [0])->type ;
    int64_t  cvlen = (Cslice [0])->vlen ;
    int64_t  cvdim = (Cslice [0])->vdim ;

    int64_t cnz = 0 ;
    int64_t cnvec = 0 ;
    int64_t cnvec_nonempty = 0 ;

    for (int tid = 0 ; tid < nthreads ; tid++)
    { 
        // compute the cumulative sum of the # entries and # vectors
        Cnzs   [tid] = cnz ;
        Cnvecs [tid] = cnvec ;
        cnz   += GB_NNZ (Cslice [tid]) ;
        cnvec += (Cslice [tid])->nvec ;
        // also sum the total number of non-empty vectors in all the slices
        cnvec_nonempty += (Cslice [tid])->nvec_nonempty ;
    }

    Cnzs   [nthreads] = cnz ;       // total # entries in C
    Cnvecs [nthreads] = cnvec ;     // total # vectors in C

    //--------------------------------------------------------------------------
    // create C and allocate all of its space
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_CREATE (Chandle, ctype, cvlen, cvdim, GB_Ap_malloc, true,
        GB_FORCE_HYPER, GB_Global_hyper_ratio_get ( ), cnvec, cnz, true,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }

    GrB_Matrix C = (*Chandle) ;

    int64_t *restrict Ch = C->h ;
    int64_t *restrict Cp = C->p ;
    int64_t *restrict Ci = C->i ;
    GB_void *restrict Cx = C->x ;
    size_t csize = ctype->size ;

    C->nvec_nonempty = cnvec_nonempty ;
    C->nvec = cnvec ;
    Cp [cnvec] = cnz ;

    //--------------------------------------------------------------------------
    // copy each slice into C
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(static,1)
    for (int tid = 0 ; tid < nthreads ; tid++)
    {
        // get the Cslice [tid] and its position in C
        int64_t *restrict Csliceh = (Cslice [tid])->h ;
        int64_t *restrict Cslicep = (Cslice [tid])->p ;
        int64_t *restrict Cslicei = (Cslice [tid])->i ;
        GB_void *restrict Cslicex = (Cslice [tid])->x ;
        int64_t cnz         = Cnzs   [tid] ;
        int64_t cnz_slice   = Cnzs   [tid+1] - cnz ;
        int64_t cnvec       = Cnvecs [tid] ;
        int64_t cnvec_slice = Cnvecs [tid+1] - cnvec ;

        // copy the row indices and values of Cslice [tid] into Ci and Cx
        memcpy (Ci + cnz        , Cslicei, cnz_slice * sizeof (int64_t)) ;
        memcpy (Cx + cnz * csize, Cslicex, cnz_slice * csize) ;

        // copy the column indices of Cslice into Ch
        memcpy (Ch + cnvec, Csliceh, cnvec_slice * sizeof (int64_t)) ;

        // construct the column pointers of C (shift upwards by cnz)
        for (int64_t k = 0 ; k < cnvec_slice ; k++)
        { 
            Cp [cnvec + k] = Cslicep [k] + cnz ;
        }
    }

    //--------------------------------------------------------------------------
    // finalize the matrix
    //--------------------------------------------------------------------------

    C->magic = GB_MAGIC ;
    ASSERT_OK (GB_check (C, "C from horizontal concatenation", GB0)) ;
    return (GrB_SUCCESS) ;
}

