//------------------------------------------------------------------------------
// GB_AxB_saxpy_parallel: C<M>=A*B, C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Parallel matrix-matrix multiply, A*B with optional mask M, using the saxpy
// method.  This method is used by GrB_mxm, GrB_vxm, and GrB_mxv.  For both of
// the latter two methods, B on input will be an nrows-by-1 column vxector.

// The strategy is to "slice" (or partition) B, as B = [B0 B1 ... B(t-1)] if
// there are t threads.  Then each thread k computes C(k) = A*B(k), and then
// the result is concatenated, as C = [C0 C1 ... C(t-1)].

// Each thread k computes an independent output matrix C(k), doing both its
// analysis and numeric phases.

// This strategy works well for OpenMP, but it could also be written in a
// purely inspector+executor style, like the GB_AxB_dot* methods.  Those
// methods do the analysis in parallel, and first determine the size of the
// output matrix C.  Then a parallel cumulative sum is computed, and the entire
// output matrix is allocated.  Then each task of the the numeric phase
// computes its part of the result C, without the need for any memory
// allocation by individual threads.

// This function, and the matrices C, M, A, and B are all CSR/CSC agnostic.
// For this discussion, suppose they are CSC, with vlen = # of rows, and vdim =
// # of columns.

// A*B is being computed, and the vector dimension of A must be identical to
// the vector length of B (as if both A and B are CSC matrices, and the number
// of columns of A is the same as the number of rows of B).

// The output matrix C = *Chandle has not been allocated, so C is NULL on
// input.  The mask M is optional.

// The semiring defines C=A*B.  flipxy modifies how the semiring multiply
// operator is applied.  If false, then fmult(aik,bkj) is computed.  If true,
// then the operands are swapped, and fmult(bkj,aij) is done instead.

// AxB_method selects the method to use:

//      GxB_DEFAULT:        the method is selected automatically

//      GxB_AxB_GUSTAVSON:  Gustavson's method for A*B

//      GxB_AxB_HEAP:       heap method for A*B

//      GxB_AxB_HASH:       hash method for A*B (FUTURE)

// The dot product method does not use this function.

// AxB_method_used reports the method actually chosen.  This is for
// informational purposes only, so if a parallel C=A*B splits the work into
// multiple submatrix multiplications, and uses different methods on each
// submatrix, then AxB_method_used is the method chosen by thread zero.

// FUTURE:: hash-based method, and multi-phase Gustavson and Heap methods,
// which do not do any memory allocations in parallel, but instead use an
// inspector+executur style (like GB_AxB_dot*).  This should work better on the
// GPU.

#include "GB_mxm.h"
#include "GB_Sauna.h"

GrB_Info GB_AxB_saxpy_parallel      // parallel matrix-matrix multiply
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected by thread zero
    bool *mask_applied,             // if true, mask was applied
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;          // C = (*Chandle) is NULL
    ASSERT (*Chandle == NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for parallel A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for parallel A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for parallel A*B", GB0)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for parallel A*B", GB0)) ;
    ASSERT (AxB_method_used != NULL) ;

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // get A and B
    //--------------------------------------------------------------------------

    if (B->nvec_nonempty < 0)
    { 
        B->nvec_nonempty = GB_nvec_nonempty (B, NULL) ;
    }

    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, NULL) ;
    }

    int64_t anz   = GB_NNZ (A) ;

    int64_t bnvec = B->nvec ;
    int64_t bnz   = GB_NNZ (B) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    // nthreads may be reduced after the flopcount is computed.

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + bnz, chunk, nthreads_max) ;

    //==========================================================================
    // sequential C<M>=A*B
    //==========================================================================

    #define GB_FREE_ALL ;

    if (nthreads == 1)
    {
        // select the method
        int64_t bjnz_max ;
        GB_AxB_select (A, B, semiring, AxB_method, AxB_method_used, &bjnz_max) ;

        // acquire a Sauna if Gustavson's method is being used
        int Sauna_id = -2 ;
        if (*AxB_method_used == GxB_AxB_GUSTAVSON)
        { 
            GB_OK (GB_Sauna_acquire (1, &Sauna_id, AxB_method_used, Context)) ;
        }

        // C<M>=A*B
        GrB_Info info1 = GB_AxB_saxpy_sequential (Chandle, M, Mask_comp, A, B,
            semiring, flipxy, *AxB_method_used, bjnz_max, true, mask_applied,
            Sauna_id) ;

        // release the Sauna for Gustavson's method
        if (*AxB_method_used == GxB_AxB_GUSTAVSON)
        { 
            // info is reset, so info1 is used above
            GB_OK (GB_Sauna_release (1, &Sauna_id)) ;
        }
        return ((info1 == GrB_OUT_OF_MEMORY) ? GB_OUT_OF_MEMORY : info1) ;
    }

    //==========================================================================
    // parallel C<M>=A*B
    //==========================================================================

    // The # of threads may be reduced, if the problem small, even to
    // nthreads=1.  But so far, for now, nthreads > 1.

    ASSERT (nthreads > 1) ;

    //--------------------------------------------------------------------------
    // count the flops and determine # of threads to use
    //--------------------------------------------------------------------------

    int64_t total_flops ;
    bool fine_slice = (nthreads > bnvec) ;
    int64_t *restrict Bflops = NULL ;
    int64_t *restrict Bflops_per_entry = NULL ;

    if (!fine_slice)
    {

        //----------------------------------------------------------------------
        // slice B by flops
        //----------------------------------------------------------------------

        // Slice B so that each slice has a balanced amount of flops, to
        // compute its slice of C.  Each thread gets enough columns of B so
        // that it has roughly total_flops / nthreads work to do.  Individual
        // columns are not sliced, so the final step to compute C is a
        // concatenation, not as summation.  This should give a very good load
        // balance where there are enough columns of B, but at the cost of a
        // more expensive symbolic analysis, taking O(bnz) time.  The analysis
        // is itself fully parallel, however.  This method cannot parallelize
        // A*B when B is a single column (GrB_mxv or GrB_vxm).

        // thread tid will do columns Slice [tid] to Slice [tid+1]-1

        // note that Bflops is initialized to zero
        GB_CALLOC_MEMORY (Bflops, bnvec+1, sizeof (int64_t)) ;
        if (Bflops == NULL)
        { 
            // out of memory
            return (GB_OUT_OF_MEMORY) ;
        }

        // Bflops [k] = # of flops to compute A*B(:,j) where j is the kth
        // vector in B
        GB_AxB_flopcount (Bflops, NULL, (Mask_comp) ? NULL : M, A, B, 0,
            Context) ;

        // reduce # of threads, based on flop count and the chunk size
        total_flops = Bflops [bnvec] ;

    }
    else
    {

        //----------------------------------------------------------------------
        // fine slice of B by flops (split columns of B)
        //----------------------------------------------------------------------

        // Slice B so that each slice has nearly exactly balanced amount of
        // flops to compute its slice of C.  Each thread gets exactly the
        // number of entries so that it does total_flops/nthreads work (rounded
        // to the nearest number of entries in B).

        // note that Bflops_per_entry is initialized to zero
        GB_CALLOC_MEMORY (Bflops_per_entry, bnz+1, sizeof (int64_t)) ;
        if (Bflops_per_entry == NULL)
        { 
            // out of memory
            return (GB_OUT_OF_MEMORY) ;
        }

        // Bflops_per_entry [p] = # of flops to compute A(:,k)*B(k,j)
        // where B(k,j) is in Bi [p] and Bx [p].
        GB_AxB_flopcount (NULL, Bflops_per_entry, (Mask_comp) ? NULL : M,
            A, B, 0, Context) ;

        // reduce # of threads, based on flop count and the chunk size
        total_flops = Bflops_per_entry [bnz] ;
    }

    //--------------------------------------------------------------------------
    // find the size of each slice
    //--------------------------------------------------------------------------

    nthreads = GB_nthreads (total_flops, chunk, nthreads_max) ;
    int64_t Slice [nthreads+1] ;
    Slice [0] = 0 ;

    if (!fine_slice)
    { 
        // slice B by the flops needed for each vector
        GB_pslice (Slice, Bflops, bnvec, nthreads) ;
        GB_FREE_MEMORY (Bflops, bnvec+1, sizeof (int64_t)) ;
    }
    else
    { 
        // slice B by the flops needed for each entry
        GB_pslice (Slice, Bflops_per_entry, bnz, nthreads) ;
        GB_FREE_MEMORY (Bflops_per_entry, bnz+1, sizeof (int64_t)) ;
    }

    //--------------------------------------------------------------------------
    // discard the mask if it's too costly to use
    //--------------------------------------------------------------------------

    if (M != NULL && total_flops < GB_NNZ (M))
    { 
        // The mask is too dense; discard it.  mask_applied will be false.
        M = NULL ;
    }

    //--------------------------------------------------------------------------
    // construct each slice of B
    //--------------------------------------------------------------------------

    // If the problem is small enough so that nthreads has been reduced to 1,
    // B is not sliced.

    GrB_Matrix Cslice [nthreads] ;
    GrB_Matrix Bslice [nthreads] ;
    for (int tid = 0 ; tid < nthreads ; tid++)
    { 
        Cslice [tid] = NULL ;
        Bslice [tid] = NULL ;
    }
    #undef  GB_FREE_ALL
    #define GB_FREE_ALL                             \
    {                                               \
        for (int tid = 0 ; tid < nthreads ; tid++)  \
        {                                           \
            GB_MATRIX_FREE (& (Cslice [tid])) ;     \
            GB_MATRIX_FREE (& (Bslice [tid])) ;     \
        }                                           \
    }

    if (nthreads > 1)
    {
        if (fine_slice)
        { 
            GB_OK (GB_fine_slice (B, nthreads, Slice, Bslice, Context)) ;
        }
        else
        { 
            GB_OK (GB_slice (B, nthreads, Slice, Bslice, Context)) ;
        }
    }

    //--------------------------------------------------------------------------
    // select the method for each slice
    //--------------------------------------------------------------------------

    GrB_Desc_Value AxB_methods_used [nthreads] ;
    int64_t bjnz_max [nthreads] ;
    int Sauna_ids [nthreads] ;

    bool any_Gustavson = false ;
    #pragma omp parallel for num_threads(nthreads) schedule(static,1) \
        reduction(||:any_Gustavson)
    for (int tid = 0 ; tid < nthreads ; tid++)
    { 
        GrB_Desc_Value thread_method_to_use ;
        GB_AxB_select (A, (nthreads == 1) ? B : Bslice [tid], semiring,
            AxB_method, &thread_method_to_use, &(bjnz_max [tid])) ;
        AxB_methods_used [tid] = thread_method_to_use ;
        // collect all thread-specific info
        any_Gustavson = any_Gustavson ||
            (thread_method_to_use == GxB_AxB_GUSTAVSON) ;
    }

    (*AxB_method_used) = AxB_methods_used [0] ;

    //--------------------------------------------------------------------------
    // acquire the Saunas for each thread that needs it
    //--------------------------------------------------------------------------

    if (any_Gustavson)
    { 
        // at least one thread needs a Sauna
        GB_OK (GB_Sauna_acquire (nthreads, Sauna_ids, AxB_methods_used,
            Context)) ;
    }
    else
    {
        // no thread needs a Sauna
        for (int tid = 0 ; tid < nthreads ; tid++)
        { 
            Sauna_ids [tid] = -2 ;
        }
    }

    //--------------------------------------------------------------------------
    // compute each slice of C = A*B with optional mask M
    //--------------------------------------------------------------------------

    // This is the only parallel region in which each thread allocates memory.
    // The memory space is not known until the thread determines the size of
    // its own output, in its analysis phase.  Note the "reduction(&&:ok)"
    // clause.  This is the only place where a clause like that apppears in
    // SuiteSparse:GraphBLAS.  This could be removed if C=A*B were to be
    // computed with an inspector+exector style of algorithm.

    // B has been "sliced"; in MATLAB notation, B = [B0 B1 B2 ... B(t-1] if
    // there are t threads.  Then each k thread computes its own Ck = A*Bk,
    // and the results are concatenated below, as C = [C0 C1 ... C(t-1)].
    // If a 'fine slice' was used for B, then C = C0+C1+...+C(t-1) must be
    // computed.

    // for all threads in parallel, with no synchronization except for these
    // boolean reductions:
    bool ok = true ;        // false if any thread's malloc or realloc fails
    bool panic = false ;    // true if any critical section fails
    bool allmask = true ;   // true if all threads apply the mask

    #pragma omp parallel for num_threads(nthreads) schedule(static,1) \
        reduction(&&:allmask) reduction(||:panic) \
        reduction(&&:ok)
    for (int tid = 0 ; tid < nthreads ; tid++)
    { 
        // each thread allocates its output, using malloc and realloc
        bool thread_mask_applied = false ;
        GrB_Info thread_info = GB_AxB_saxpy_sequential (&(Cslice [tid]), M,
            Mask_comp, A, (nthreads == 1) ? B : Bslice [tid], semiring,
            flipxy, AxB_methods_used [tid], bjnz_max [tid],
            false, &thread_mask_applied, Sauna_ids [tid]) ;
        // collect all thread-specific info
        ok      = ok      && (thread_info == GrB_SUCCESS) ;
        allmask = allmask && (thread_mask_applied) ;
        panic   = panic   || (thread_info == GrB_PANIC) ;
    }

    //--------------------------------------------------------------------------
    // check error conditions
    //--------------------------------------------------------------------------

    // panic if a critical section fails
    if (panic) return (GrB_PANIC) ;

    // check the return info from all the threads
    if (!ok)
    { 
        // out of memory
        if (any_Gustavson)
        { 
            // at least one thread used a Sauna; free and release all Sauna
            // workspaces
            for (int tid = 0 ; tid < nthreads ; tid++)
            {
                int Sauna_id = Sauna_ids [tid] ;
                if (Sauna_id >= 0)
                { 
                    GB_Sauna_free (Sauna_id) ;
                }
            }
            GB_OK (GB_Sauna_release (nthreads, Sauna_ids)) ;
        }
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // release the Saunas
    //--------------------------------------------------------------------------

    if (any_Gustavson)
    { 
        // at least one thread used a Sauna
        GB_OK (GB_Sauna_release (nthreads, Sauna_ids)) ;
    }

    //--------------------------------------------------------------------------
    // check if all threads applied the mask
    //--------------------------------------------------------------------------

    // if all threads applied the mask to their slices, then GB_accum_mask does
    // not need to apply it to the concatenated C in GB_AxB_meta.  If just some
    // of them did, then GB_accum_mask needs to apply the mask again.
    (*mask_applied) = allmask ;

    //--------------------------------------------------------------------------
    // concatenate or sum the slices of C
    //--------------------------------------------------------------------------

    // Each slice Cslice [tid] has the same dimensions and type as C.  C is
    // stored by column.

    if (nthreads == 1)
    { 
        // one thread, so only one slice: just copy Cslice[0] to C
        (*Chandle) = Cslice [0] ;
        Cslice [0] = NULL ;
    }
    else if (fine_slice)
    { 
        // C = sum (Cslice [0..nthreads-1]).  Adjacent slices of C can share
        // columns, which must be summed.  Columns in the middle of each slice
        // are concatenated horizontally.
        GB_OK (GB_hcat_fine_slice (Chandle, nthreads, Cslice, semiring->add,
            Sauna_ids, Context)) ;
    }
    else
    { 
        // C = [Cslice(0) Cslice(1) ... Cslice(nthreads-1)] concatenatied
        // horizontally.  Each slice contains entries that appear in a unique
        // and contiguous subset of the columns of C.
        GB_OK (GB_hcat_slice (Chandle, nthreads, Cslice, Context)) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    ASSERT_OK (GB_check (*Chandle, "C for parallel A*B", GB0)) ;
    return (GrB_SUCCESS) ;
}

