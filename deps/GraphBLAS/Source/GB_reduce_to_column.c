//------------------------------------------------------------------------------
// GB_reduce_to_column: reduce a matrix to a column using a binary op
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w<mask> = accum (w,reduce(A)) where w is n-by-1

#include "GB.h"

GrB_Info GB_reduce_to_column        // w<mask> = accum (w,reduce(A))
(
    GrB_Matrix w,                   // input/output for results, size n-by-1
    const GrB_Matrix mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp reduce,      // reduce operator for t=reduce(A)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc,      // descriptor for w, mask, and A
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK2 (w, mask, A)) ;

    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (mask) ;
    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_FAULTY (desc) ;

    ASSERT_OK (GB_check (w, "w input for reduce_BinaryOp", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (mask, "mask for reduce_BinaryOp", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for reduce_BinaryOp", GB0)) ;
    ASSERT_OK (GB_check (reduce, "reduce for reduce_BinaryOp", GB0)) ;
    ASSERT_OK (GB_check (A, "A input for reduce_BinaryOp", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (desc, "desc for reduce_BinaryOp", GB0)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, xx1, xx2);

    // w and mask are n-by-1 GrB_Vector objects, typecasted to GrB_Matrix
    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (GB_IMPLIES (mask != NULL, GB_VECTOR_OK (mask))) ;

    // check domains and dimensions for w<mask> = accum (w,T)
    GrB_Type ttype = reduce->ztype ;
    info = GB_compatible (w->type, w, mask, accum, ttype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // check types of reduce
    if (reduce->xtype != reduce->ztype || reduce->ytype != reduce->ztype)
    { 
        // all 3 types of z = reduce (x,y) must be the same.  reduce must also
        // be associative but there is no way to check this in general.
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "All domains of reduction operator must be identical;\n"
            "operator is: [%s] = %s ([%s],[%s])", reduce->ztype->name,
            reduce->name, reduce->xtype->name, reduce->ytype->name))) ;
    }

    // T = reduce (T,A) must be compatible
    if (!GB_Type_compatible (A->type, reduce->ztype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for reduction operator z=%s(x,y):\n"
            "input matrix A of type [%s]\n"
            "cannot be typecast to reduction operator of type [%s]",
            reduce->name, A->type->name, reduce->ztype->name))) ;
    }

    // check the dimensions
    int64_t wlen = GB_NROWS (w) ;
    if (A_transpose)
    {
        if (wlen != GB_NCOLS (A))
        { 
            return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                "w=reduce(A'):  length of w is "GBd";\n"
                "it must match the number of columns of A, which is "GBd".",
                wlen, GB_NCOLS (A)))) ;
        }
    }
    else
    {
        if (wlen != GB_NROWS(A))
        { 
            return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
                "w=reduce(A):  length of w is "GBd";\n"
                "it must match the number of rows of A, which is "GBd".",
                wlen, GB_NROWS (A)))) ;
        }
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (w, C_replace, mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_WAIT (w) ;
    GB_WAIT (mask) ;
    GB_WAIT (A) ;

    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format of A
    //--------------------------------------------------------------------------

    // the result vector T is in CSC format
    if (!(A->is_csc))
    { 
        A_transpose = !A_transpose ;
    }

    //--------------------------------------------------------------------------
    // T = reduce (A) or reduce (A')
    //--------------------------------------------------------------------------

    // T is created below so that it can be typecasted to a GrB_Vector when
    // done: non-hypersparse wlen-by-1 matrix in CSC format.

    GrB_Matrix T = NULL ;

    // T = reduce_to_column (A) or reduce_to_column (A'), which is T = sum (A')
    // or sum (A), in MATLAB notation, except where where 'sum' is any
    // associative operator.

    // By default, T(i) = op (A (i,:)) is a column whose length is the same as
    // the number of rows of A.  T(i) is the reduction of all entries in the
    // ith row of A.  If A_transpose is true, the T is computed as if A were
    // transposed first, and thus its length is equal to the number of columns
    // of the input matrix A.  The use of A_transpose is the opposite of
    // MATLAB, since sum(A) in MATLAB sums up the columns of A, and sum(A')
    // sums up the rows of A..

    // T is an wlen-by-1 GrB_Matrix that represents the column.  It is computed
    // as a matrix so it can be passed to GB_accum_mask without typecasting.

    ASSERT (wlen == (A_transpose) ? A->vdim : A->vlen) ;

    //--------------------------------------------------------------------------
    // scalar workspace
    //--------------------------------------------------------------------------

    size_t asize = A->type->size ;
    int    acode = A->type->code ;
    const int64_t *restrict Ai = A->i ;
    const GB_void *restrict Ax = A->x ;
    int64_t anz = GB_NNZ (A) ;

    size_t zsize = reduce->ztype->size ;
    int    zcode = reduce->ztype->code ;
    char awork [zsize] ;
    char zwork [zsize] ;

    //--------------------------------------------------------------------------
    // T = reduce(A) or reduce(A')
    //--------------------------------------------------------------------------

    GxB_binary_function freduce = reduce->function ;
    GB_cast_function cast_A_to_Z = GB_cast_factory (zcode, acode) ;
    int64_t tnz = 0 ;
    bool nocasting = (A->type == reduce->ztype) ;

    if (A_transpose)
    {

        //----------------------------------------------------------------------
        // T = reduce(A'), where T(j) = reduce (A (:,j))
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // count the number of entries in the result
        //----------------------------------------------------------------------

        // nnz(T) = # of non-empty columns of A
        tnz = A->nvec_nonempty ;
        ASSERT (tnz == GB_nvec_nonempty (A)) ;

        //----------------------------------------------------------------------
        // allocate T
        //----------------------------------------------------------------------

        // since T is a GrB_Vector, it is CSC and not hypersparse
        T = NULL ;                  // allocate a new header for T
        GB_CREATE (&T, ttype, wlen, 1, GB_Ap_calloc, true,
            GB_FORCE_NONHYPER, GB_HYPER_DEFAULT, 1, tnz, true) ;
        if (info != GrB_SUCCESS)
        { 
            return (info) ;
        }
        ASSERT (GB_VECTOR_OK (T)) ;

        T->p [0] = 0 ;
        T->p [1] = tnz ;
        int64_t *restrict Ti = T->i ;
        GB_void *restrict Tx = T->x ;
        T->nvec_nonempty = (tnz > 0) ? 1 : 0 ;

        //----------------------------------------------------------------------
        // sum down each sparse vector: T (j) = reduce (A (:,j))
        //----------------------------------------------------------------------

        bool done = false ;

        tnz = 0 ;

        // define the worker for the switch factory
        #define GB_WORKER(type)                                             \
        {                                                                   \
            const type *ax = (type *) Ax ;                                  \
            type *tx = (type *) Tx ;                                        \
            GB_for_each_vector (A)                                          \
            {                                                               \
                /* w = reduce (A (:,j)) */                                  \
                type w ;                                                    \
                int64_t GBI1_initj (Iter, j, p, pend) ;                     \
                if (p >= pend) continue ;   /* skip vector j if empty */    \
                /* w = Ax [p], the first entry in vector j */               \
                w = ax [p] ;                                                \
                /* subsequent entries in vector j */                        \
                /* FUTURE: some operators can terminate this loop early */  \
                for (p++ ; p < pend ; p++)                                  \
                {                                                           \
                    /* w "+=" ax [p] ; */                                   \
                    GB_DUP (w, ax [p]) ;                                    \
                }                                                           \
                Ti [tnz] = j ;                                              \
                tx [tnz] = w ;                                              \
                tnz++ ;                                                     \
            }                                                               \
            done = true ;                                                   \
        }

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // If GB_COMPACT is defined, the switch factory is disabled and all
        // work is done by the generic worker.  The compiled code will be more
        // compact, but 3 to 4 times slower.

        #ifndef GBCOMPACT

            if (nocasting)
            { 
                // controlled by opcode and typecode.  No typecasting is done.
                GB_Opcode opcode = reduce->opcode ;
                GB_Type_code typecode = acode ;
                ASSERT (typecode <= GB_UDT_code) ;
                #include "GB_assoc_template.c"
            }

        #endif

        #undef GB_WORKER

        //----------------------------------------------------------------------
        // generic worker: with typecasting
        //----------------------------------------------------------------------

        if (!done)
        {
            GB_for_each_vector (A)
            {
                // zwork = reduce (A (:,j))
                int64_t GBI1_initj (Iter, j, p, pend) ;
                if (p >= pend) continue ;   // skip vector j if empty
                // zwork = (ztype) Ax [p], the first entry in vector j
                cast_A_to_Z (zwork, Ax +(p*asize), zsize) ;
                // subsequent entries in vector j
                for (p++ ; p < pend ; p++)
                { 
                    // awork = (ztype) Ax [p]
                    cast_A_to_Z (awork, Ax +(p*asize), zsize) ;
                    // zwork += awork
                    freduce (zwork, zwork, awork) ;         // (z x alias)
                }
                Ti [tnz] = j ;
                // Tx [tnz] = zwork ;
                memcpy (Tx +(tnz*zsize), zwork, zsize) ;
                tnz++ ;
            }
        }

        ASSERT (tnz == T->p [1]) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // T = reduce(A), where T(i) = reduce (A (i,:))
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // select the method
        //----------------------------------------------------------------------

        // When A_transpose is false (after flipping it to account for the
        // CSR/CSC format), wlen is A->vlen, the vector length of A.  This is
        // the number of rows of a CSC matrix, or the # of columns of a CSR
        // matrix.  The matrix A itself requires O(vdim+anz) memory if
        // non-hypersparse and O(anz) if hypersparse.  This does not depend on
        // A->vlen.  So if the vector length is really huge (when anz << wlen),
        // the bucket method would fail.  Thus, the qsort method, below, is
        // used when anz < wlen.

        if (anz < wlen)
        {

            //------------------------------------------------------------------
            // qsort method
            //------------------------------------------------------------------

            // memory usage is O(anz) and time is O(anz*log(anz)).  This is
            // more efficient than the bucket method, below, when A is very
            // hypersparse.  The time and memory complexity does not depend
            // on wlen.

            // since T is a GrB_Vector, it is not hypersparse
            T = NULL ;                  // allocate a new header for T
            GB_NEW (&T, ttype, wlen, 1, GB_Ap_null, true, GB_FORCE_NONHYPER,
                GB_HYPER_DEFAULT, 1) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                return (info) ;
            }

            info = GB_build (T, (GrB_Index *) Ai, NULL, Ax, anz, reduce, acode,
                false, false, Context) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                GB_MATRIX_FREE (&T) ;
                return (info) ;
            }
            ASSERT (T->nvec_nonempty == GB_nvec_nonempty (T)) ;

        }
        else
        {

            //------------------------------------------------------------------
            // bucket method: allocate workspace
            //------------------------------------------------------------------

            // memory usage is O(wlen) and time is O(wlen + anz).  This can be
            // costly if A is hypersparse, but it is only used if anz >= wlen,
            // so the time and memory usage are OK.

            bool *restrict mark = NULL ;
            GB_CALLOC_MEMORY (mark, wlen + 1, sizeof (bool)) ;

            GB_void *restrict work = NULL ;
            GB_MALLOC_MEMORY (work, wlen + 1, zsize) ;

            #define GB_REDUCE_FREE_WORK                                     \
            {                                                               \
                GB_FREE_MEMORY (mark, wlen + 1, sizeof (bool)) ;            \
                GB_FREE_MEMORY (work, wlen + 1, zsize) ;                    \
            }

            if (mark == NULL || work == NULL)
            { 
                // out of memory
                GB_REDUCE_FREE_WORK ;
                double memory = GBYTES (wlen + 1, sizeof (bool) + zsize) ;
                return (GB_OUT_OF_MEMORY (memory)) ;
            }

            //------------------------------------------------------------------
            // sum across each index: work [i] = reduce (A (i,:))
            //------------------------------------------------------------------

            bool done = false ;

            // define the worker for the switch factory
            #define GB_WORKER(type)                                         \
            {                                                               \
                const type *ax = (type *) Ax ;                              \
                type *ww = (type *) work ;                                  \
                for (int64_t p = 0 ; p < anz ; p++)                         \
                {                                                           \
                    /* get A(i,j) */                                        \
                    int64_t i = Ai [p] ;                                    \
                    if (!mark [i])                                          \
                    {                                                       \
                        /* first time row i has been seen */                \
                        ww [i] = ax [p] ;                                   \
                        mark [i] = true ;                                   \
                        tnz++ ;                                             \
                    }                                                       \
                    else                                                    \
                    {                                                       \
                        /* ww [i] "+=" ax [p] */                            \
                        GB_DUP (ww [i], ax [p]) ;                           \
                    }                                                       \
                }                                                           \
                done = true ;                                               \
            }

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            // If GB_COMPACT is defined, the switch factory is disabled and all
            // work is done by the generic worker.  The compiled code will be
            // more compact, but 3 to 4 times slower.

            #ifndef GBCOMPACT

                if (nocasting)
                { 
                    // controlled by opcode and typecode.  No typecasting
                    GB_Opcode opcode = reduce->opcode ;
                    GB_Type_code typecode = acode ;
                    ASSERT (typecode <= GB_UDT_code) ;
                    #include "GB_assoc_template.c"
                }

            #endif

            #undef GB_WORKER

            //------------------------------------------------------------------
            // generic worker
            //------------------------------------------------------------------

            if (!done)
            {
                for (int64_t p = 0 ; p < anz ; p++)
                {
                    // get A(i,j)
                    int64_t i = Ai [p] ;
                    if (!mark [i])
                    { 
                        // work [i] = (ztype) Ax [p]
                        cast_A_to_Z (work +(i*zsize), Ax +(p*asize), zsize) ;
                        mark [i] = true ;
                        tnz++ ;
                    }
                    else
                    { 
                        // awork = (ztype) Ax [p]
                        cast_A_to_Z (awork, Ax +(p*asize), zsize) ;
                        // work [i] += awork
                        GB_void *restrict worki = work +(i*zsize) ;
                        freduce (worki, worki, awork) ;     // (z x alias)
                    }
                }
            }

            //------------------------------------------------------------------
            // allocate T
            //------------------------------------------------------------------

            // if T is dense, then transplant work into T->x
            bool tdense = (tnz == wlen) ;

            // since T is a GrB_Vector, it is CSC and not hypersparse
            T = NULL ;                  // allocate a new header for T
            GB_CREATE (&T, ttype, wlen, 1, GB_Ap_calloc, true,
                GB_FORCE_NONHYPER, GB_HYPER_DEFAULT, 1, tnz, !tdense) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                GB_REDUCE_FREE_WORK ;
                return (info) ;
            }

            if (tdense)
            { 
                // T is dense, transplant work into T->x
                T->x = work ;
                // set work to NULL so it will not be freed
                work = NULL ;
            }

            T->p [0] = 0 ;
            T->p [1] = tnz ;
            int64_t *restrict Ti = T->i ;
            T->nvec_nonempty = (tnz > 0) ? 1 : 0 ;

            //------------------------------------------------------------------
            // gather the results into T
            //------------------------------------------------------------------

            if (tdense)
            {
                // construct the pattern of T
                for (int64_t i = 0 ; i < wlen ; i++)
                { 
                    Ti [i] = i ;
                }
            }
            else
            {
                // gather T from mark and work
                GB_void *restrict Tx = T->x ;
                int64_t p = 0 ;
                for (int64_t i = 0 ; i < wlen ; i++)
                {
                    if (mark [i])
                    { 
                        Ti [p] = i ;
                        // Tx [p] = work [i]
                        memcpy (Tx +(p*zsize), work +(i*zsize), zsize) ;
                        p++ ;
                    }
                }
                ASSERT (p == tnz) ;
            }

            //------------------------------------------------------------------
            // free workspace
            //------------------------------------------------------------------

            GB_REDUCE_FREE_WORK ;
        }
    }
    ASSERT_OK (GB_check (T, "T output for T = reduce (A)", GB0)) ;

    //--------------------------------------------------------------------------
    // w<mask> = accum (w,T): accumulate the results into w via the mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (w, mask, NULL, accum, &T, C_replace, Mask_comp,
        Context)) ;
}

