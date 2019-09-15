//------------------------------------------------------------------------------
// GB_transpose:  C=A' or C=op(A'), with typecasting
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// CALLS:     GB_builder

// Transpose a matrix, C=A', and optionally apply a unary operator and/or
// typecast the values.  The transpose may be done in place, in which case C or
// A are modified in place.  If the matrix to be transposed has more than one
// vector, it may have jumbled indices in its vectors, which must be sorted.
// If the input matrix has a single vector, it must be already sorted on input.
// The input matrix may have shallow components (even if in place), and the
// output may also have shallow components (even in the input matrix is not
// shallow).

// This function is CSR/CSC agnostic; it sets the output matrix format from
// C_is_csc but otherwise ignores the CSR/CSC type of A and C.

// If A_in is NULL, then C = (*Chandle) is transposed in place.  If out of
// memory, (*Chandle) is always returned as NULL, which frees the input matrix
// C if the transpose is done in place.

// If A_in is not NULL and Chandle is NULL, then A is modified in place, and
// the A_in matrix is not freed when done.

// The bucket sort is parallel, but not highly scalable.  If e=nnz(A) and A is
// m-by-n, then at most O(e/n) threads are used.  For many matrices, e is O(n),
// although the constant can be high.  The qsort method is more scalable, but
// not as fast with a modest number of threads.

#include "GB_transpose.h"
#include "GB_build.h"
#include "GB_apply.h"

GrB_Info GB_transpose           // C=A', C=(ctype)A or C=op(A')
(
    GrB_Matrix *Chandle,        // output matrix C, possibly modified in place
    GrB_Type ctype,             // desired type of C; if NULL use A->type.
                                // ignored if op is present (cast to op->ztype)
    const bool C_is_csc,        // desired CSR/CSC format of C
    const GrB_Matrix A_in,      // input matrix
    const GrB_UnaryOp op_in,    // optional operator to apply to the values
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs and determine if transpose is done in place
    //--------------------------------------------------------------------------

    bool in_place_C, in_place_A ;

    GrB_Matrix A, C ;

    if (A_in == NULL)
    { 

        //----------------------------------------------------------------------
        // C = C' ; &C is transposed in place
        //----------------------------------------------------------------------

        // GB_transpose (&C, ctype, csc, NULL, op) ;
        // C=A' is transposed in place, in the matrix C.
        // The matrix C is freed if an error occurs and C is set to NULL.

        ASSERT (Chandle != NULL) ;  // at least &C or A must be non-NULL
        A = (*Chandle) ;
        C = A ;                     // C must be freed if an error occurs
        in_place_C = true ;         // C is modified in place
        in_place_A = false ;
        ASSERT (A == C && A == (*Chandle)) ;

    }
    else if (Chandle == NULL || (*Chandle) == A_in)
    { 

        //----------------------------------------------------------------------
        // A = A' ; A is transposed in place; reuse the header of A
        //----------------------------------------------------------------------

        // GB_transpose (NULL, ctype, csc, A, op) ;
        // GB_transpose (&A, ctype, csc, A, op) ;
        // C=A' is transposed in place, in the matrix A.
        // The matrix A_in is not freed if an error occurs.

        A = A_in ;
        Chandle = &A ;              // C must not be freed if an error occurs
        C = A ;
        in_place_C = false ;
        in_place_A = true ;         // A is modified in place
        ASSERT (A == C && A == (*Chandle)) ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // C = A' ; C and A are different
        //----------------------------------------------------------------------

        // GB_transpose (&C, ctype, csc, A, op) ;
        // C and A are both non-NULL, and not aliased.
        // C=A' where C is a new matrix constructed here.
        // The matrix C is freed if an error occurs, and C is set to NULL.

        A = A_in ;
        C = NULL ;
        (*Chandle) = NULL ;         // C must be allocated; freed on error
        in_place_C = false ;        // C and A are different matrices
        in_place_A = false ;
        ASSERT (A != C && A != (*Chandle)) ;
    }

    bool in_place = (in_place_A || in_place_C) ;

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A input for GB_transpose", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (ctype, "ctype for GB_transpose", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (op_in, "op for GB_transpose", GB0)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use here
    //--------------------------------------------------------------------------

    int64_t anz   = GB_NNZ (A) ;
    int64_t anvec = A->nvec ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + anvec, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    GrB_Info info ;

    GrB_Type atype = A->type ;
    size_t asize = atype->size ;
    GB_Type_code acode = atype->code ;

    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;
    int64_t aplen = A->plen ;

    bool A_is_hyper = A->is_hyper ;
    double A_hyper_ratio = A->hyper_ratio ;

    int64_t anzmax = A->nzmax ;

    // if in place, these must be freed when done, whether successful or not
    int64_t *restrict Ap = A->p ;
    int64_t *restrict Ah = A->h ;
    int64_t *restrict Ai = A->i ;
    GB_void *restrict Ax = A->x ;

    bool Ap_shallow = A->p_shallow ;
    bool Ah_shallow = A->h_shallow ;
    bool Ai_shallow = A->i_shallow ;
    bool Ax_shallow = A->x_shallow ;

    // free prior content of A, if transpose is done in place
    #define GB_FREE_IN_PLACE_A                                               \
    {                                                                        \
        if (in_place)                                                        \
        {                                                                    \
            /* A is being transposed in placed */                            \
            /* free prior content of A but not &A itself */                  \
            if (!Ap_shallow) GB_FREE_MEMORY (Ap, aplen+1, sizeof (int64_t)) ;\
            if (!Ah_shallow) GB_FREE_MEMORY (Ah, aplen  , sizeof (int64_t)) ;\
            if (!Ai_shallow) GB_FREE_MEMORY (Ai, anzmax , sizeof (int64_t)) ;\
            if (!Ax_shallow) GB_FREE_MEMORY (Ax, anzmax , asize) ;           \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            /* A is not modified; it is purely an input matrix */            \
            ;                                                                \
        }                                                                    \
    }

    // free the new C matrix, unless C=A' is being done in place of A
    #define GB_FREE_C                                               \
    {                                                               \
        if (!in_place_A)                                            \
        {                                                           \
            /* free all of C and all its contents &C */             \
            GB_MATRIX_FREE (Chandle) ;                              \
        }                                                           \
    }

    // free both A (if in place) and C (if not in place of A)
    #define GB_FREE_A_AND_C                                         \
    {                                                               \
        GB_FREE_IN_PLACE_A ;                                        \
        GB_FREE_C ;                                                 \
    }

    //--------------------------------------------------------------------------
    // determine the type of C and get the unary operator
    //--------------------------------------------------------------------------

    GrB_UnaryOp op ;

    if (op_in == NULL)
    {
        // no operator
        op = NULL ;
        if (ctype == NULL)
        { 
            // no typecasting if ctype is NULL
            ctype = atype ;
        }
    }
    else
    {
        // If a unary operator z=op(x) is present, C is always returned as
        // op->ztype.  The input ctype is ignored.
        if (op_in->opcode == GB_IDENTITY_opcode && atype == op_in->xtype)
        { 
            // op is a built-in identity operator, with the same type as A, so
            // do not apply the operator and do not typecast.
            ASSERT (op_in->ztype == op_in->xtype) ;
            op = NULL ;
            ctype = atype ;
        }
        else
        { 
            // apply the operator, z=op(x)
            op = op_in ;
            ctype = op->ztype ;
        }
    }

    GB_Type_code ccode = ctype->code ;
    size_t csize = ctype->size ;

    //--------------------------------------------------------------------------
    // C = A'
    //--------------------------------------------------------------------------

    ASSERT (GB_IMPLIES (avlen == 0 || avdim == 0, anz == 0)) ;

    bool allocate_new_Cx = (ctype != atype) || (op != NULL) ;

    if (anz == 0)
    {

        //======================================================================
        // quick return if A is empty
        //======================================================================

        GB_FREE_IN_PLACE_A ;

        // A is empty; create a new empty matrix C, with the new type and
        // dimensions.  C is hypersparse for now but may convert when
        // returned.
        GB_CREATE (Chandle, ctype, avdim, avlen, GB_Ap_calloc,
            C_is_csc, GB_FORCE_HYPER, A_hyper_ratio, 1, 1, true, Context) ;

        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_FREE_C ;
            return (info) ;
        }
        ASSERT_OK (GB_check (*Chandle, "C transpose empty", GB0)) ;

    }
    else if (avdim == 1)
    {

        //======================================================================
        // transpose a "column" vector into a "row"
        //======================================================================

        // transpose a vector (avlen-by-1) into a "row" matrix (1-by-avlen).
        // A must be already sorted on input
        ASSERT_OK (GB_check (A, "the vector A must already be sorted", GB0)) ;

        //----------------------------------------------------------------------
        // allocate space
        //----------------------------------------------------------------------

        // Allocate the header of C, with no C->p, C->h, C->i, or C->x
        // content, and initialize the type and dimension of C.   If in
        // place, A->p, A->h, A->i, and A->x are all NULL.  The new matrix
        // is hypersparse, but can be CSR or CSC.  This step does not
        // allocate anything if in place.

        // if *Chandle == NULL, allocate a new header; otherwise reuse existing
        GB_NEW (Chandle, ctype, 1, avlen, GB_Ap_null, C_is_csc,
            GB_FORCE_HYPER, A_hyper_ratio, 0, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            ASSERT (!in_place) ;    // cannot fail if in place
            GB_FREE_C ;
            return (info) ;
        }

        if (!in_place)
        { 
            C = (*Chandle) ;
        }
        else
        { 
            ASSERT (A == C && A == (*Chandle)) ;
        }

        // allocate new space for the values and pattern
        GB_void *restrict Cx = NULL ;
        int64_t *restrict Cp ;
        int64_t *restrict Ci ;
        GB_MALLOC_MEMORY (Cp, anz+1, sizeof (int64_t)) ;
        GB_CALLOC_MEMORY (Ci, anz  , sizeof (int64_t)) ;
        if (allocate_new_Cx)
        { 
            // allocate new space for the new typecasted numerical values of C
            GB_MALLOC_MEMORY (Cx, anz, ctype->size) ;
        }
        if (Cp == NULL || Ci == NULL || (allocate_new_Cx && (Cx == NULL)))
        { 
            // out of memory
            GB_FREE_MEMORY (Cp, anz+1, sizeof (int64_t)) ;
            GB_FREE_MEMORY (Ci, anz  , sizeof (int64_t)) ;
            GB_FREE_MEMORY (Cx, anz  , csize) ;
            GB_FREE_A_AND_C ;
            return (GB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // the transpose will now succeed; fill the content of C
        //----------------------------------------------------------------------

        // numerical values: apply the operator, typecast, or make shallow copy
        if (op != NULL)
        { 
            // Cx = op ((op->xtype) Ax)
            C->x = Cx ; C->x_shallow = false ;
            GB_apply_op (Cx, op, Ax, atype, anz, Context) ;
            // prior Ax will be freed
        }
        else if (ctype != atype)
        { 
            // copy the values from A into C and cast from atype to ctype
            C->x = Cx ; C->x_shallow = false ;
            GB_cast_array (Cx, ccode, Ax, acode, anz, Context) ;
            // prior Ax will be freed
        }
        else // ctype == atype
        { 
            // no type change; numerical values of C are a shallow copy of A.
            C->x = Ax ; C->x_shallow = (in_place) ? Ax_shallow : true ;
            Ax = NULL ;  // do not free prior Ax
        }

        // each entry in A becomes a non-empty vector in C
        C->h = Ai ; C->h_shallow = (in_place) ? Ai_shallow : true ;
        Ai = NULL ;     // do not free prior Ai

        C->nzmax = anz ;

        // C->p = 0:anz and C->i = zeros (1,anz), newly allocated
        C->plen = anz ;
        C->nvec = anz ;
        C->nvec_nonempty = anz ;

        C->i = Ci ; C->i_shallow = false ;
        C->p = Cp ; C->p_shallow = false ;

        // fill the vector pointers C->p
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (int64_t k = 0 ; k <= anz ; k++)
        { 
            Cp [k] = k ;
        }
        C->magic = GB_MAGIC ;

        //----------------------------------------------------------------------
        // free prior space
        //----------------------------------------------------------------------

        GB_FREE_IN_PLACE_A ;

    }
    else if (avlen == 1)
    {

        //======================================================================
        // transpose a "row" into a "column" vector
        //======================================================================

        // transpose a "row" matrix (1-by-avdim) into a vector (avdim-by-1).
        // if A->vlen is 1, all vectors of A are implicitly sorted
        ASSERT_OK (GB_check (A, "1-by-n input A already sorted", GB0)) ;

        //----------------------------------------------------------------------
        // allocate space
        //----------------------------------------------------------------------

        // Allocate the header of C, with no C->p, C->h, C->i, or C->x
        // content, and initialize the type and dimension of C.   If in
        // place, A->p, A->h, A->i, and A->x are all NULL.  The new matrix
        // is NON-hypersparse, but can be CSR or CSC.  This step does not
        // allocate anything if in place.

        // if *Chandle == NULL, allocate a new header; otherwise reuse existing
        GB_NEW (Chandle, ctype, avdim, 1, GB_Ap_null, C_is_csc,
            GB_FORCE_NONHYPER, A_hyper_ratio, 0, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            ASSERT (!in_place) ;        // cannot fail if in place
            GB_FREE_C ;
            return (info) ;
        }

        if (!in_place)
        { 
            C = (*Chandle) ;
        }
        else
        { 
            ASSERT (A == C && A == (*Chandle)) ;
        }

        // allocate new space for the values and pattern
        GB_void *restrict Cx = NULL ;
        int64_t *restrict Cp ;
        int64_t *restrict Ci = NULL ;
        GB_CALLOC_MEMORY (Cp, 2, sizeof (int64_t)) ;

        bool allocate_new_Ci = (!A_is_hyper) ;

        if (allocate_new_Ci)
        { 
            // A is not hypersparse, so new space is needed for Ci
            GB_MALLOC_MEMORY (Ci, anz, sizeof (int64_t)) ;
        }

        if (allocate_new_Cx)
        { 
            // allocate new space for the new typecasted numerical values of C
            GB_MALLOC_MEMORY (Cx, anz, ctype->size) ;
        }

        if (Cp == NULL || (allocate_new_Cx && (Cx == NULL))
                       || (allocate_new_Ci && (Ci == NULL)))
        { 
            // out of memory
            GB_FREE_MEMORY (Cp, 2    , sizeof (int64_t)) ;
            GB_FREE_MEMORY (Ci, anz  , sizeof (int64_t)) ;
            GB_FREE_MEMORY (Cx, anz  , csize) ;
            GB_FREE_A_AND_C ;
            return (GB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // numerical values of C: apply the op, typecast, or make shallow copy
        //----------------------------------------------------------------------

        if (op != NULL)
        { 
            // Cx = op ((op->xtype) Ax)
            C->x = Cx ; C->x_shallow = false ;
            GB_apply_op (Cx, op, Ax, atype, anz, Context) ;
            // prior Ax will be freed
        }
        else if (ctype != atype)
        { 
            // copy the values from A into C and cast from atype to ctype
            C->x = Cx ; C->x_shallow = false ;
            GB_cast_array (Cx, ccode, Ax, acode, anz, Context) ;
            // prior Ax will be freed
        }
        else // ctype == atype
        { 
            // no type change; numerical values of C are a shallow copy of A
            C->x = Ax ; C->x_shallow = (in_place) ? Ax_shallow : true ;
            Ax = NULL ;  // do not free prior Ax
        }

        //----------------------------------------------------------------------
        // pattern of C
        //----------------------------------------------------------------------

        if (A_is_hyper)
        { 

            //------------------------------------------------------------------
            // each non-empty vector in A becomes an entry in C
            //------------------------------------------------------------------

            ASSERT (!allocate_new_Ci) ;
            C->i = Ah ; C->i_shallow = (in_place) ? Ah_shallow : true ;
            ASSERT (anvec == anz) ;
            Ah = NULL ;  // do not free prior Ah

        }
        else
        {

            //------------------------------------------------------------------
            // find the non-empty vectors of A, which become entries in C
            //------------------------------------------------------------------

            ASSERT (allocate_new_Ci) ;
            ASSERT (Ah == NULL) ;

            int nth = GB_nthreads (avdim, chunk, nthreads_max) ;

            if (nth == 1)
            {

                //--------------------------------------------------------------
                // construct Ci with a single thread
                //--------------------------------------------------------------

                int64_t k = 0 ;
                for (int64_t j = 0 ; j < avdim ; j++)
                {
                    if (Ap [j] < Ap [j+1])
                    { 
                        Ci [k++] = j ;
                    }
                }
                ASSERT (k == anz) ;

            }
            else
            {

                //--------------------------------------------------------------
                // construct Ci in parallel
                //--------------------------------------------------------------

                int ntasks = (nth == 1) ? 1 : (8 * nth) ;
                ntasks = GB_IMIN (ntasks, avdim) ;
                ntasks = GB_IMAX (ntasks, 1) ;
                int64_t Count [ntasks+1] ;

                #pragma omp parallel for num_threads(nth) schedule(dynamic,1)
                for (int tid = 0 ; tid < ntasks ; tid++)
                {
                    int64_t jstart, jend, k = 0 ;
                    GB_PARTITION (jstart, jend, avdim, tid, ntasks) ;
                    for (int64_t j = jstart ; j < jend ; j++)
                    {
                        if (Ap [j] < Ap [j+1])
                        { 
                            k++ ;
                        }
                    }
                    Count [tid] = k ;
                }

                GB_cumsum (Count, ntasks, NULL, 1) ;
                ASSERT (Count [ntasks] == anz) ;

                #pragma omp parallel for num_threads(nth) schedule(dynamic,1)
                for (int tid = 0 ; tid < ntasks ; tid++)
                {
                    int64_t jstart, jend, k = Count [tid] ;
                    GB_PARTITION (jstart, jend, avdim, tid, ntasks) ;
                    for (int64_t j = jstart ; j < jend ; j++)
                    {
                        if (Ap [j] < Ap [j+1])
                        { 
                            Ci [k++] = j ;
                        }
                    }
                }
            }

            #ifdef GB_DEBUG
            int64_t k = 0 ;
            for (int64_t j = 0 ; j < avdim ; j++)
            {
                if (Ap [j] < Ap [j+1])
                {
                    ASSERT (Ci [k] == j) ;
                    k++ ;
                }
            }
            ASSERT (k == anz) ;
            #endif

            C->i = Ci ; C->i_shallow = false ;
        }

        //----------------------------------------------------------------------
        // vector pointers of C
        //----------------------------------------------------------------------

        C->nzmax = anz ;

        // C->p = [0 anz] and C->h = NULL
        ASSERT (C->plen == 1) ;
        ASSERT (C->nvec == 1) ;
        ASSERT (C->h == NULL) ;
        C->p = Cp ; C->p_shallow = false ;

        C->nvec_nonempty = (anz == 0) ? 0 : 1 ;

        // fill the vector pointers C->p
        Cp [0] = 0 ;
        Cp [1] = anz ;
        C->magic = GB_MAGIC ;

        //----------------------------------------------------------------------
        // free prior space
        //----------------------------------------------------------------------

        GB_FREE_IN_PLACE_A ;

    }
    else
    {

        //======================================================================
        // transpose a general matrix
        //======================================================================

        ASSERT_OK_OR_JUMBLED (GB_check (A, "A GB_transpose jumbled ok", GB0)) ;
        ASSERT (avdim > 1 && avlen > 1) ;

        // T=A' with optional typecasting, or T=op(A')

        //----------------------------------------------------------------------
        // select the method
        //----------------------------------------------------------------------

        // for the qsort method, if the transpose is done in place and A->i is
        // not shallow, A->i can be used and then freed.  Otherwise, A->i is
        // not modified at all.
        bool recycle_Ai = (in_place && !Ai_shallow) ;
        bool use_qsort ;

        if (A_is_hyper)
        { 

            //------------------------------------------------------------------
            // always use qsort for hypersparse matrices
            //------------------------------------------------------------------

            use_qsort = true ;

        }
        else
        { 

            //------------------------------------------------------------------
            // select qsort if the transpose will likely be hypersparse
            //------------------------------------------------------------------

            use_qsort = GB_CHOOSE_QSORT_INSTEAD_OF_BUCKET (anz, avlen) ;

        }

        //----------------------------------------------------------------------
        // transpose the matrix with the selected method
        //----------------------------------------------------------------------

        if (use_qsort)
        {

            //==================================================================
            // transpose via quicksort
            //==================================================================

            //------------------------------------------------------------------
            // allocate and create iwork
            //------------------------------------------------------------------

            // allocate iwork of size anz
            int64_t *iwork ;
            GB_MALLOC_MEMORY (iwork, anz, sizeof (int64_t)) ;

            if (iwork == NULL)
            { 
                // out of memory
                GB_FREE_C ;
                return (GB_OUT_OF_MEMORY) ;
            }

            // Construct the "row" indices of C, which are "column" indices of
            // A.  This array becomes the permanent T->i on output.  This phase
            // must be done before Chandle is created below, since that step
            // destroys A.

            GB_extract_vector_list (iwork, A, nthreads) ;

            //------------------------------------------------------------------
            // allocate the output matrix and additional space (jwork and S)
            //------------------------------------------------------------------

            // Allocate the header of C, with no C->p, C->h, C->i, or C->x
            // content, and initialize the type and dimension of C.   If in
            // place, A->p, A->h, A->i, and A->x are all NULL.  The new matrix
            // is hypersparse, but can be CSR or CSC.  This step does not
            // allocate anything if in place.

            // if *Chandle == NULL, allocate a new header; otherwise reuse
            GB_NEW (Chandle, ctype, avdim, avlen, GB_Ap_null, C_is_csc,
                GB_FORCE_HYPER, A_hyper_ratio, 0, Context) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                ASSERT (!in_place) ;        // cannot fail if in place
                GB_FREE_MEMORY (iwork, anz, sizeof (int64_t)) ;
                GB_FREE_C ;
                return (info) ;
            }

            if (!in_place)
            { 
                C = (*Chandle) ;
            }
            else
            { 
                ASSERT (A == C && A == (*Chandle)) ;
            }

            // if in_place, the prior Ap and Ah can now be freed
            if (in_place)
            { 
                if (!Ap_shallow) GB_FREE_MEMORY (Ap, aplen+1, sizeof (int64_t));
                if (!Ah_shallow) GB_FREE_MEMORY (Ah, aplen  , sizeof (int64_t));
            }

            int64_t *jwork = NULL ;
            GB_Type_code scode ;
            GB_void *S = NULL ;
            GB_void *Swork = NULL ;

            if (!recycle_Ai)
            { 
                // allocate jwork of size anz
                GB_MALLOC_MEMORY (jwork, anz, sizeof (int64_t)) ;
            }

            if (op != NULL)
            { 
                // allocate Swork of size anz * csize
                GB_MALLOC_MEMORY (Swork, anz, csize) ;
            }

            if ((!recycle_Ai && (jwork == NULL))
            || ((op != NULL) && (Swork == NULL)))
            { 
                // out of memory
                GB_FREE_MEMORY (iwork, anz, sizeof (int64_t)) ;
                GB_FREE_MEMORY (jwork, anz, sizeof (int64_t)) ;
                GB_FREE_MEMORY (Swork, anz, csize) ;
                GB_FREE_A_AND_C ;
                return (GB_OUT_OF_MEMORY) ;
            }

            //------------------------------------------------------------------
            // construct jwork and Swork
            //------------------------------------------------------------------

            // "row" indices of A become "column" indices of C
            if (recycle_Ai)
            { 
                // Ai is used as workspace for the "column" indices of C.
                // jwork is a shallow copy of Ai, and is freed by GB_builder.
                jwork = Ai ;
                ASSERT (in_place) ;
                // set Ai to NULL so it is not freed by GB_FREE_IN_PLACE_A
                Ai = NULL ;
            }
            else
            { 
                // jwork = Ai, making a deep copy.  jwork is freed by
                // GB_builder.  A->i is not modified, even if out of memory.
                GB_memcpy (jwork, Ai, anz * sizeof (int64_t), nthreads) ;
            }

            // numerical values: apply the op, typecast, or make shallow copy
            if (op != NULL)
            { 
                // Swork = op ((op->xtype) Ax)
                GB_apply_op (Swork, op, Ax, atype, anz, Context) ;
                // GB_builder will not need to typecast Swork to T->x, and it
                // may choose to transplant it into T->x
                scode = ccode ;
                #if 0
                if (in_place && !Ax_shallow)
                {
                    // A is being transposed in place so A->x is no longer
                    // needed.  If A->x is shallow this can be skipped.  T->x
                    // will not be shallow if the op is present.  A->x should
                    // be freed early to free up space for GB_builder.
                    // However, in the current usage, when op is used, A is not
                    // transposed in place, so this step is not needed.
                    ASSERT (GB_DEAD_CODE) ;
                    GB_FREE_MEMORY (Ax, anzmax , asize) ;
                }
                #endif
            }
            else
            { 
                // GB_builder will typecast S from atype to ctype if needed.
                // S is a shallow copy of Ax, and must not be modified.
                S = Ax ;
                scode = acode ;
            }

            //------------------------------------------------------------------
            // build the matrix: T = (ctype) A' or op ((xtype) A')
            //------------------------------------------------------------------

            // internally, jwork is freed and then T->x is allocated, so the
            // total high-water memory usage is anz * max (csize,
            // sizeof(int64_t)).  T is always hypersparse.

            // If op is not NULL, then Swork can be transplanted into T in
            // GB_builder, instead.  However, this requires the tuples to be
            // sorted on input, which is possible but rare for GB_transpose.

            GrB_Matrix T ;
            info = GB_builder
            (
                &T,         // create T
                ctype,      // T is of type ctype
                avdim,      // T->vlen = A->vdim, always > 1
                avlen,      // T->vdim = A->vlen, always > 1
                C_is_csc,   // T has the same CSR/CSC format as C
                &iwork,     // iwork_handle, becomes T->i on output
                &jwork,     // jwork_handle, freed on output
                &Swork,     // Swork_handle, freed on output
                false,      // tuples are not sorted on input
                true,       // tuples have no duplicates
                anz,        // size of iwork, jwork, and Swork
                true,       // is_matrix: unused
                false,      // ijcheck: unused
                NULL, NULL, // original I,J indices: not used here
                S,          // array of values of type scode, not modified
                anz,        // number of tuples
                NULL,       // no dup operator needed (input has no duplicates)
                scode,      // type of S or Swork
                Context
            ) ;

            // GB_builder always frees jwork, and either frees iwork or
            // transplants it in to T->i and sets iwork to NULL.  So iwork and
            // jwork are always NULL on output.  GB_builder does not modify S.
            ASSERT (iwork == NULL && jwork == NULL && Swork == NULL) ;

            //------------------------------------------------------------------
            // free prior space and transplant T into C
            //------------------------------------------------------------------

            // Free the prior content of the input matrix, if done in place.
            // Ap, Ah, and Ai have already been freed, but Ax has not.
            GB_FREE_IN_PLACE_A ;

            if (info != GrB_SUCCESS)
            { 
                // out of memory in GB_builder
                GB_FREE_A_AND_C ;
                return (info) ;
            }

            // Transplant T in to the result C.  The matrix T is not shallow
            // and no typecasting is done, so this will always succeed.
            info = GB_transplant (*Chandle, ctype, &T, Context) ;
            ASSERT (info == GrB_SUCCESS) ;

        }
        else
        {

            //==================================================================
            // transpose via bucket sort
            //==================================================================

            // This method does not operate on the matrix in place, so it must
            // create a temporary matrix T.  Then the input matrix is freed and
            // replaced with the new matrix T.

            ASSERT (!A_is_hyper) ;

            // T is also typecasted to ctype, if not NULL
            GrB_Matrix T ;
            info = GB_transpose_bucket (&T, ctype, C_is_csc, A, op, Context) ;

            // free prior content, if C=A' is being done in place
            if (in_place_A)
            { 
                // free all content of A, but not the header, if in place of A
                GB_PHIX_FREE (A) ;   // transpose in-place
            }
            else if (in_place_C)
            { 
                // free all of C, including the header, if done in place of C
                GB_MATRIX_FREE (Chandle) ;
            }

            if (info != GrB_SUCCESS)
            { 
                // out of memory in GB_transpose_bucket
                GB_FREE_C ;
                return (info) ;
            }

            ASSERT_OK (GB_check (T, "T from bucket", GB0)) ;

            if (in_place_A)
            { 
                // The header of A has not been freed, since it is used for the
                // output.  Transplant T back into A and free T.  T is not
                // shallow and no typecast is done so this will always succeed.
                info = GB_transplant (A, ctype, &T, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
            else
            { 
                // If C=A' is done in place of C, then the header and content
                // of the input C has been freed.  The output T can now be
                // moved to the Chandle.
                ASSERT (*Chandle == NULL) ;
                (*Chandle) = T ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // conform the result to the desired hypersparsity of A
    //--------------------------------------------------------------------------

    // get the output matrix
    C = (*Chandle) ;

    // transplant the hyper_ratio from A to C
    C->hyper_ratio = A_hyper_ratio ;

    ASSERT_OK (GB_check (C, "C to conform in GB_transpose", GB0)) ;

    info = GB_to_hyper_conform (C, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_C ;
        return (info) ;
    }

    ASSERT_OK (GB_check (*Chandle, "Chandle conformed in GB_transpose", GB0)) ;
    return (GrB_SUCCESS) ;
}

