//------------------------------------------------------------------------------
// GB_transpose: C=A' or C=op(A'), with typecasting
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// CALLS:     GB_builder

// Transpose a matrix, C=A', and optionally apply a unary operator and/or
// typecast the values.  The transpose may be done in-place, in which case C or
// A are modified in-place.

// There are two ways to use this method:
//  C = A'      C and A are different
//  C = C'      C is transposed in-place, (C==A aliased)

// In both cases, the header for C and A must already be allocated (either
// static or dynamic).   A is never modified, unless C==A.  C and A cannot be
// NULL on input.  If in place (C == A) then C and A is a valid matrix on input
// (the input matrix A).  If C != A, the contents of C are not defined on input,
// and any prior content is freed.  Either header may be static or dynamic.

// The input matrix A may have shallow components (even if in-place), and the
// output C may also have shallow components (even if the input matrix is not
// shallow).

// This function is CSR/CSC agnostic; it sets the output matrix format from
// C_is_csc but otherwise ignores the CSR/CSC type of A and C.

// The bucket sort is parallel, but not highly scalable.  If e=nnz(A) and A is
// m-by-n, then at most O(e/n) threads are used.  The GB_builder method is more
// scalable, but not as fast with a modest number of threads.

#include "GB_transpose.h"
#include "GB_build.h"
#include "GB_apply.h"

#define GB_FREE_WORK                    \
{                                       \
    GB_FREE (&iwork, iwork_size) ;      \
    GB_FREE (&jwork, jwork_size) ;      \
    GB_FREE (&Swork, Swork_size) ;      \
    GB_WERK_POP (Count, int64_t) ;      \
}

#define GB_FREE_ALL                     \
{                                       \
    GB_FREE_WORK ;                      \
    GB_phbix_free (T) ;                 \
    /* freeing C also frees A if transpose is done in-place */ \
    GB_phbix_free (C) ;                 \
}

//------------------------------------------------------------------------------
// GB_transpose
//------------------------------------------------------------------------------

GrB_Info GB_transpose           // C=A', C=(ctype)A' or C=op(A')
(
    GrB_Matrix C,               // output matrix C, possibly modified in-place
    GrB_Type ctype,             // desired type of C; if NULL use A->type.
                                // ignored if op is present (cast to op->ztype)
    const bool C_is_csc,        // desired CSR/CSC format of C
    const GrB_Matrix A,         // input matrix; C == A if done in place
        // no operator is applied if both op1 and op2 are NULL
        const GrB_UnaryOp op1_in,       // unary operator to apply
        const GrB_BinaryOp op2_in,      // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs and determine if transpose is done in-place
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (C != NULL) ;
    ASSERT (A != NULL) ;
    bool in_place = (A == C) ;
    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = GB_clear_static_header (&T_header) ;
    GB_WERK_DECLARE (Count, int64_t) ;
    int64_t *iwork = NULL ; size_t iwork_size = 0 ;
    int64_t *jwork = NULL ; size_t jwork_size = 0 ;
    GB_void *Swork = NULL ; size_t Swork_size = 0 ;

    ASSERT_MATRIX_OK (A, "A input for GB_transpose", GB0) ;
    ASSERT_TYPE_OK_OR_NULL (ctype, "ctype for GB_transpose", GB0) ;
    ASSERT_UNARYOP_OK_OR_NULL (op1_in, "unop for GB_transpose", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (op2_in, "binop for GB_transpose", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (scalar, "scalar for GB_transpose", GB0) ;

    if (in_place)
    { 
        GBURBLE ("(in-place transpose) ") ;
    }

    // get the current sparsity control of A
    float A_hyper_switch = A->hyper_switch ;
    float A_bitmap_switch = A->bitmap_switch ;
    int A_sparsity_control = A->sparsity_control ;
    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;

    // wait if A has pending tuples or zombies; leave jumbled unless avdim == 1
    if (GB_PENDING (A) || GB_ZOMBIES (A) || (avdim == 1 && GB_JUMBLED (A)))
    { 
        GB_OK (GB_wait (A, "A", Context)) ;
    }
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_IMPLIES (avdim == 1, !GB_JUMBLED (A))) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    GrB_Type atype = A->type ;
    size_t asize = atype->size ;
    GB_Type_code acode = atype->code ;

    bool A_is_bitmap = GB_IS_BITMAP (A) ;
    bool A_is_hyper  = GB_IS_HYPERSPARSE (A) ;

    int64_t anz = GB_nnz (A) ;
    int64_t anz_held = GB_nnz_held (A) ;
    int64_t anvec = A->nvec ;
    int64_t anvals = A->nvals ;

    //--------------------------------------------------------------------------
    // determine the max number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // determine the type of C and get the unary or binary operator
    //--------------------------------------------------------------------------

    // If a unary or binary operator is present, C is always returned as
    // the ztype of the operator.  The input ctype is ignored.

    GrB_UnaryOp  op1 = NULL ;
    GrB_BinaryOp op2 = NULL ;
    GB_Opcode opcode = GB_NOP_opcode ;

    if (op1_in != NULL)
    {
        // get the unary operator
        opcode = op1_in->opcode ;
        if (atype == op1_in->xtype && opcode == GB_IDENTITY_opcode)
        { 
            // op1 is a built-in identity operator, with the same type as A, so
            // do not apply the operator and do not typecast.  op1 is NULL.
            ctype = atype ;
        }
        else
        { 
            // apply the operator, z=op1(x)
            op1 = op1_in ;
            ctype = op1->ztype ;
        }
    }
    else if (op2_in != NULL)
    { 
        // get the binary operator
        GrB_Type op2_intype = binop_bind1st ? op2_in->xtype : op2_in->ytype ;
        opcode = op2_in->opcode ;
        // only GB_apply calls GB_transpose with op2_in, and it ensures this
        // condition holds: first(A,y), second(x,A) have been renamed to
        // identity(A), and PAIR has been renamed one(A), so these cases do not
        // occur here.
        ASSERT (!((opcode == GB_PAIR_opcode) ||
                  (opcode == GB_FIRST_opcode  && !binop_bind1st) ||
                  (opcode == GB_SECOND_opcode &&  binop_bind1st))) ;
        // apply the operator, z=op2(A,y) or op2(x,A)
        op2 = op2_in ;
        ctype = op2->ztype ;
    }
    else
    {
        // no operator.  both op1 and op2 are NULL
        if (ctype == NULL)
        { 
            // no typecasting if ctype is NULL
            ctype = atype ;
        }
    }

    GB_Type_code ccode = ctype->code ;
    size_t csize = ctype->size ;

    //--------------------------------------------------------------------------
    // check for positional operators
    //--------------------------------------------------------------------------

    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    GrB_UnaryOp  save_op1 = op1 ;
    GrB_BinaryOp save_op2 = op2 ;
    if (op_is_positional)
    { 
        // do not apply the op until after the transpose
        op1 = NULL ;
        op2 = NULL ;
        // replace op1 with the ONE operator, as a placeholder.  C will be
        // constructed as iso, and needs to be expanded to non-iso when done.
        ASSERT (ctype == GrB_INT64 || ctype == GrB_INT32) ;
        op1 = (ctype == GrB_INT64) ? GxB_ONE_INT64 : GxB_ONE_INT32 ;
    }

    //--------------------------------------------------------------------------
    // determine the iso status of C
    //--------------------------------------------------------------------------

    ASSERT (GB_IMPLIES (avlen == 0 || avdim == 0, anz == 0)) ;
    GB_iso_code C_code_iso = GB_iso_unop_code (A, op1, op2, binop_bind1st) ;
    bool C_iso = (C_code_iso != GB_NON_ISO) ;

    ASSERT (GB_IMPLIES (A->iso, C_iso)) ;

    if (C_iso && !op_is_positional)
    { 
        GBURBLE ("(iso transpose) ") ;
    }
    else
    {
        GBURBLE ("(transpose) ") ;
    }

    //==========================================================================
    // T = A', T = (ctype) A', or T = op (A')
    //==========================================================================

    if (anz == 0)
    { 

        //----------------------------------------------------------------------
        // A is empty
        //----------------------------------------------------------------------

        // create a new empty matrix T, with the new type and dimensions.
        // set T->iso = false   OK
        GB_OK (GB_new_bix (&T, true, // hyper, static header
            ctype, avdim, avlen, GB_Ap_calloc, C_is_csc, GxB_HYPERSPARSE,
            true, A_hyper_switch, 1, 1, true, false, Context)) ;

    }
    else if (A_is_bitmap || GB_as_if_full (A))
    {

        //----------------------------------------------------------------------
        // transpose a bitmap/as-if-full matrix or vector
        //----------------------------------------------------------------------

        // A is either bitmap or as-is-full (full, or sparse or hypersparse
        // with all entries present, no zombies, no pending tuples, and not
        // jumbled).  T = A' is either bitmap or full.

        int T_sparsity = (A_is_bitmap) ? GxB_BITMAP : GxB_FULL ;
        bool T_cheap =                  // T can be done quickly if:
            (avlen == 1 || avdim == 1)      // A is a row or column vector,
            && op1 == NULL && op2 == NULL   // no operator to apply,
            && atype == ctype ;             // and no typecasting

        // allocate T
        if (T_cheap)
        { 
            // just initialize the static header of T, not T->b or T->x
            info = GB_new (&T, true,  // bitmap or full, static header
                ctype, avdim, avlen, GB_Ap_null, C_is_csc,
                T_sparsity, A_hyper_switch, 1, Context) ;
            ASSERT (info == GrB_SUCCESS) ;
        }
        else
        { 
            // allocate all of T, including T->b and T->x
            // set T->iso = C_iso   OK
            GB_OK (GB_new_bix (&T, true,  // bitmap or full, static header
                ctype, avdim, avlen, GB_Ap_null, C_is_csc, T_sparsity, true,
                A_hyper_switch, 1, anz_held, true, C_iso, Context)) ;
        }

        T->magic = GB_MAGIC ;
        if (T_sparsity == GxB_BITMAP)
        { 
            T->nvals = anvals ;     // for bitmap case only
        }

        //----------------------------------------------------------------------
        // T = A'
        //----------------------------------------------------------------------

        int nthreads = GB_nthreads (anz_held + anvec, chunk, nthreads_max) ;

        if (T_cheap)
        {
            // no work to do.  Transposing does not change A->b or A->x
            T->b = A->b ; T->b_size = A->b_size ;
            T->x = A->x ; T->x_size = A->x_size ;
            if (in_place)
            { 
                // transplant A->b and A->x into T
                T->b_shallow = A->b_shallow ;
                T->x_shallow = A->x_shallow ;
                A->b = NULL ;
                A->x = NULL ;
            }
            else
            { 
                // T is a purely shallow copy of A 
                T->b_shallow = (A->b != NULL) ;
                T->x_shallow = true ;
            }
            T->iso = A->iso ;   // OK
        }
        else if (op1 == NULL && op2 == NULL)
        { 
            // do not apply an operator; optional typecast to T->type
            GB_transpose_ix (T, A, NULL, NULL, 0, nthreads) ;
        }
        else
        { 
            // apply an operator, T has type op->ztype
            GB_transpose_op (T, C_code_iso, op1, op2, scalar, binop_bind1st, A,
                NULL, NULL, 0, nthreads) ;
        }

        ASSERT_MATRIX_OK (T, "T dense/bitmap", GB0) ;
        ASSERT (!GB_JUMBLED (T)) ;

    }
    else if (avdim == 1)
    {

        //----------------------------------------------------------------------
        // transpose a "column" vector into a "row"
        //----------------------------------------------------------------------

        // transpose a vector (avlen-by-1) into a "row" matrix (1-by-avlen).
        // A must be sorted first.

        ASSERT_MATRIX_OK (A, "the vector A must already be sorted", GB0) ;
        ASSERT (!GB_JUMBLED (A)) ;

        //----------------------------------------------------------------------
        // allocate T
        //----------------------------------------------------------------------

        // Initialized the header of T, with no content, and initialize the
        // type and dimension of T.  T is hypersparse.

        info = GB_new (&T, true, // hyper; static header
            ctype, 1, avlen, GB_Ap_null, C_is_csc,
            GxB_HYPERSPARSE, A_hyper_switch, 0, Context) ;
        ASSERT (info == GrB_SUCCESS) ;

        // allocate T->p, T->i, and optionally T->x, but not T->h
        T->p = GB_MALLOC (anz+1, int64_t, &(T->p_size)) ;
        T->i = GB_MALLOC (anz  , int64_t, &(T->i_size)) ;
        bool allocate_Tx = (op1 != NULL || op2 != NULL || C_iso) ||
                           (ctype != atype) ;
        if (allocate_Tx)
        { 
            // allocate new space for the new typecasted numerical values of T
            T->x = GB_XALLOC (C_iso, anz, csize, &(T->x_size)) ;
        }
        if (T->p == NULL || T->i == NULL || (allocate_Tx && T->x == NULL))
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // numerical values of T: apply the op, typecast, or make shallow copy
        //----------------------------------------------------------------------

        // numerical values: apply the operator, typecast, or make shallow copy
        if (op1 != NULL || op2 != NULL || C_iso)
        { 
            // T->x = op1 (A), op2 (A,scalar), or op2 (scalar,A), or
            // compute the iso value of T = 1, A, or scalar, without any op
            info = GB_apply_op ((GB_void *) T->x, ctype, C_code_iso, op1, op2,
                scalar, binop_bind1st, A, Context) ;
            ASSERT (info == GrB_SUCCESS) ;
        }
        else if (ctype != atype)
        { 
            // copy the values from A into T and cast from atype to ctype
            GB_cast_matrix (T, A, Context) ;
        }
        else
        { 
            // no type change; numerical values of T are a shallow copy of A.
            ASSERT (!allocate_Tx) ;
            T->x = A->x ; T->x_size = A->x_size ;
            if (in_place)
            {
                // transplant A->x as T->x
                T->x_shallow = A->x_shallow ;
                A->x = NULL ;
            }
            else
            {
                // T->x is a shallow copy of A->x
                T->x_shallow = true ;
            }
        }

        // each entry in A becomes a non-empty vector in T;
        // T is a hypersparse 1-by-avlen matrix

        // transplant or shallow-copy A->i as the new T->h
        T->h = A->i ; T->h_size = A->i_size ;
        if (in_place)
        { 
            // transplant A->i as T->h
            T->h_shallow = A->i_shallow ;
            A->i = NULL ;
        }
        else
        { 
            // T->h is a shallow copy of A->i
            T->h_shallow = true ;
        }

        // T->p = 0:anz and T->i = zeros (1,anz), newly allocated
        T->plen = anz ;
        T->nvec = anz ;
        T->nvec_nonempty = anz ;

        // fill the vector pointers T->p
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        int64_t k ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (k = 0 ; k < anz ; k++)
        { 
            T->i [k] = 0 ;
            T->p [k] = k ;
        }
        T->p [anz] = anz ;

        T->iso = C_iso ;    // OK
        T->magic = GB_MAGIC ;

    }
    else if (avlen == 1)
    {

        //----------------------------------------------------------------------
        // transpose a "row" into a "column" vector
        //----------------------------------------------------------------------

        // transpose a "row" matrix (1-by-avdim) into a vector (avdim-by-1).
        // if A->vlen is 1, all vectors of A are implicitly sorted
        ASSERT_MATRIX_OK (A, "1-by-n input A already sorted", GB0) ;

        //----------------------------------------------------------------------
        // allocate workspace, if needed
        //----------------------------------------------------------------------

        int ntasks = 0 ;
        int nth = GB_nthreads (avdim, chunk, nthreads_max) ;
        if (nth > 1 && !A_is_hyper)
        {
            // ntasks and Count are not needed if nth == 1
            ntasks = 8 * nth ;
            ntasks = GB_IMIN (ntasks, avdim) ;
            ntasks = GB_IMAX (ntasks, 1) ;
            GB_WERK_PUSH (Count, ntasks+1, int64_t) ;
            if (Count == NULL)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GrB_OUT_OF_MEMORY) ;
            }
        }

        // Allocate the header of T, with no content
        // and initialize the type and dimension of T.
        info = GB_new (&T, true, // sparse; static header
            ctype, avdim, 1, GB_Ap_null, C_is_csc,
            GxB_SPARSE, A_hyper_switch, 0, Context) ;
        ASSERT (info == GrB_SUCCESS) ;

        T->iso = C_iso ;    // OK

        // allocate new space for the values and pattern
        T->p = GB_CALLOC (2, int64_t, &(T->p_size)) ;
        if (!A_is_hyper)
        { 
            // A is sparse, so new space is needed for T->i
            T->i = GB_MALLOC (anz, int64_t, &(T->i_size)) ;
        }
        bool allocate_Tx = (op1 != NULL || op2 != NULL || C_iso) ||
                           (ctype != atype) ;
        if (allocate_Tx)
        { 
            // allocate new space for the new typecasted numerical values of T
            T->x = GB_XALLOC (C_iso, anz, csize, &(T->x_size)) ;
        }

        if (T->p == NULL || (T->i == NULL && !A_is_hyper) ||
            (T->x == NULL && allocate_Tx))
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // numerical values of T: apply the op, typecast, or make shallow copy
        //----------------------------------------------------------------------

        // numerical values: apply the operator, typecast, or make shallow copy
        if (op1 != NULL || op2 != NULL || C_iso)
        { 
            // T->x = op1 (A), op2 (A,scalar), or op2 (scalar,A), or
            // compute the iso value of T = 1, A, or scalar, without any op
            info = GB_apply_op ((GB_void *) T->x, ctype, C_code_iso, op1, op2,
                scalar, binop_bind1st, A, Context) ;
            ASSERT (info == GrB_SUCCESS) ;
        }
        else if (ctype != atype)
        { 
            // copy the values from A into T and cast from atype to ctype
            GB_cast_matrix (T, A, Context) ;
        }
        else
        { 
            // no type change; numerical values of T are a shallow copy of A.
            ASSERT (!allocate_Tx) ;
            T->x = A->x ; T->x_size = A->x_size ;
            if (in_place)
            { 
                // transplant A->x as T->x
                T->x_shallow = A->x_shallow ;
                A->x = NULL ;
            }
            else
            { 
                // T->x is a shallow copy of A->x
                T->x_shallow = true ;
            }
        }

        //----------------------------------------------------------------------
        // compute T->i
        //----------------------------------------------------------------------

        if (A_is_hyper)
        { 

            //------------------------------------------------------------------
            // each non-empty vector in A becomes an entry in T
            //------------------------------------------------------------------

            T->i = A->h ; T->i_size = A->h_size ;
            if (in_place)
            { 
                // transplant A->h as T->i
                T->i_shallow = A->h_shallow ;
                A->h = NULL ;
            }
            else
            { 
                // T->i is a shallow copy of A->h
                T->i_shallow = true ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // find the non-empty vectors of A, which become entries in T
            //------------------------------------------------------------------

            if (nth == 1)
            {

                //--------------------------------------------------------------
                // construct T->i with a single thread
                //--------------------------------------------------------------

                int64_t k = 0 ;
                for (int64_t j = 0 ; j < avdim ; j++)
                {
                    if (A->p [j] < A->p [j+1])
                    { 
                        T->i [k++] = j ;
                    }
                }
                ASSERT (k == anz) ;

            }
            else
            {

                //--------------------------------------------------------------
                // construct T->i in parallel
                //--------------------------------------------------------------

                int tid ;
                #pragma omp parallel for num_threads(nth) schedule(dynamic,1)
                for (tid = 0 ; tid < ntasks ; tid++)
                {
                    int64_t jstart, jend, k = 0 ;
                    GB_PARTITION (jstart, jend, avdim, tid, ntasks) ;
                    for (int64_t j = jstart ; j < jend ; j++)
                    {
                        if (A->p [j] < A->p [j+1])
                        { 
                            k++ ;
                        }
                    }
                    Count [tid] = k ;
                }

                GB_cumsum (Count, ntasks, NULL, 1, NULL) ;
                ASSERT (Count [ntasks] == anz) ;

                #pragma omp parallel for num_threads(nth) schedule(dynamic,1)
                for (tid = 0 ; tid < ntasks ; tid++)
                {
                    int64_t jstart, jend, k = Count [tid] ;
                    GB_PARTITION (jstart, jend, avdim, tid, ntasks) ;
                    for (int64_t j = jstart ; j < jend ; j++)
                    {
                        if (A->p [j] < A->p [j+1])
                        { 
                            T->i [k++] = j ;
                        }
                    }
                }
            }

            #ifdef GB_DEBUG
            int64_t k = 0 ;
            for (int64_t j = 0 ; j < avdim ; j++)
            {
                if (A->p [j] < A->p [j+1])
                {
                    ASSERT (T->i [k] == j) ;
                    k++ ;
                }
            }
            ASSERT (k == anz) ;
            #endif
        }

        //---------------------------------------------------------------------
        // vector pointers of T
        //---------------------------------------------------------------------

        // T->p = [0 anz]
        ASSERT (T->plen == 1) ;
        ASSERT (T->nvec == 1) ;
        T->nvec_nonempty = (anz == 0) ? 0 : 1 ;
        T->p [1] = anz ;
        T->magic = GB_MAGIC ;
        ASSERT (!GB_JUMBLED (T)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // transpose a general sparse or hypersparse matrix
        //----------------------------------------------------------------------

        ASSERT_MATRIX_OK (A, "A for GB_transpose", GB0) ;

        // T=A' with optional typecasting, or T=op(A')

        //----------------------------------------------------------------------
        // select the method
        //----------------------------------------------------------------------

        int nworkspaces_bucket, nthreads_bucket ;
        bool use_builder = GB_transpose_method (A,
            &nworkspaces_bucket, &nthreads_bucket, Context) ;

        //----------------------------------------------------------------------
        // transpose the matrix with the selected method
        //----------------------------------------------------------------------

        if (use_builder)
        {

            //------------------------------------------------------------------
            // transpose via GB_builder
            //------------------------------------------------------------------

            //------------------------------------------------------------------
            // allocate and create iwork
            //------------------------------------------------------------------

            // allocate iwork of size anz
            iwork = GB_MALLOC (anz, int64_t, &iwork_size) ;
            if (iwork == NULL)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GrB_OUT_OF_MEMORY) ;
            }

            // Construct the "row" indices of C, which are "column" indices of
            // A.  This array becomes the permanent T->i on output.

            GB_OK (GB_extract_vector_list (iwork, A, Context)) ;

            //------------------------------------------------------------------
            // allocate the output matrix and additional space (jwork and Swork)
            //------------------------------------------------------------------

            // initialize the header of T, with no content
            // content, and initialize the type and dimension of T.

            info = GB_new (&T, true, // hyper, static header
                ctype, avdim, avlen, GB_Ap_null, C_is_csc,
                GxB_HYPERSPARSE, A_hyper_switch, 0, Context) ;
            ASSERT (info == GrB_SUCCESS) ;

            // if in_place, the prior A->p and A->h can now be freed
            if (in_place)
            { 
                if (!A->p_shallow) GB_FREE (&A->p, A->p_size) ;
                if (!A->h_shallow) GB_FREE (&A->h, A->h_size) ;
            }

            GB_void *S_input = NULL ;

            // for the GB_builder method, if the transpose is done in-place and
            // A->i is not shallow, A->i can be used and then freed.
            // Otherwise, A->i is not modified at all.
            bool ok = true ;
            bool recycle_Ai = (in_place && !A->i_shallow) ;
            if (!recycle_Ai)
            { 
                // allocate jwork of size anz
                jwork = GB_MALLOC (anz, int64_t, &jwork_size) ;
                ok = ok && (jwork != NULL) ;
            }

            if ((op1 != NULL || op2 != NULL) && !C_iso)
            { 
                Swork = (GB_void *) GB_XALLOC (C_iso, anz, csize, &Swork_size) ;
                ok = ok && (Swork != NULL) ;
            }

            if (!ok)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GrB_OUT_OF_MEMORY) ;
            }

            //------------------------------------------------------------------
            // construct jwork and Swork
            //------------------------------------------------------------------

            // "row" indices of A become "column" indices of C
            if (recycle_Ai)
            { 
                // A->i is used as workspace for the "column" indices of C.
                // jwork is A->i, and is freed by GB_builder.
                jwork = A->i ;
                jwork_size = A->i_size ;
                A->i = NULL ;
                ASSERT (in_place) ;
            }
            else
            { 
                // copy A->i into jwork, making a deep copy.  jwork is freed by
                // GB_builder.  A->i is not modified, even if out of memory.
                GB_memcpy (jwork, A->i, anz * sizeof (int64_t), nthreads_max) ;
            }

            // numerical values: apply the op, typecast, or make shallow copy
            GrB_Type stype ;
            GB_void sscalar [GB_VLA(csize)] ;
            if (C_iso)
            { 
                // apply the op to the iso scalar
                GB_iso_unop (sscalar, ctype, C_code_iso, op1, op2, A, scalar) ;
                S_input = sscalar ;     // S_input is used instead of Swork
                Swork = NULL ;
                stype = ctype ;
            }
            else if (op1 != NULL || op2 != NULL)
            { 
                // Swork = op (A)
                info = GB_apply_op (Swork, ctype, C_code_iso, op1, op2, scalar,
                    binop_bind1st, A, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
                // GB_builder will not need to typecast Swork to T->x, and it
                // may choose to transplant it into T->x
                S_input = NULL ;        // Swork is used instead of S_input
                stype = ctype ;
            }
            else
            { 
                // GB_builder will typecast S_input from atype to ctype if
                // needed.  S_input is a shallow copy of Ax, and must not be
                // modified.
                ASSERT (!C_iso) ;
                ASSERT (!A->iso) ;
                S_input = (GB_void *) A->x ; // S_input is used instead of Swork
                Swork = NULL ;
                stype = atype ;
            }

            //------------------------------------------------------------------
            // build the matrix: T = (ctype) A' or op ((xtype) A')
            //------------------------------------------------------------------

            // internally, jwork is freed and then T->x is allocated, so the
            // total memory usage is anz * max (csize, sizeof(int64_t)).  T is
            // always hypersparse.  Either T, Swork, and S_input are all iso,
            // or all non-iso, depending on C_iso.

            GB_OK (GB_builder (
                T,          // create T using a static header
                ctype,      // T is of type ctype
                avdim,      // T->vlen = A->vdim, always > 1
                avlen,      // T->vdim = A->vlen, always > 1
                C_is_csc,   // T has the same CSR/CSC format as C
                &iwork,     // iwork_handle, becomes T->i on output
                &iwork_size,
                &jwork,     // jwork_handle, freed on output
                &jwork_size,
                &Swork,     // Swork_handle, freed on output
                &Swork_size,
                false,      // tuples are not sorted on input
                true,       // tuples have no duplicates
                anz,        // size of iwork, jwork, and Swork
                true,       // is_matrix: unused
                NULL, NULL, // original I,J indices: not used here
                S_input,    // array of values of type stype, not modified
                C_iso,      // iso property of T is the same as C->iso
                anz,        // number of tuples
                NULL,       // no dup operator needed (input has no duplicates)
                stype,      // type of S_input or Swork
                Context
            )) ;

            // GB_builder always frees jwork, and either frees iwork or
            // transplants it in to T->i and sets iwork to NULL.  So iwork and
            // jwork are always NULL on output.  GB_builder does not modify
            // S_input.
            ASSERT (iwork == NULL && jwork == NULL && Swork == NULL) ;
            ASSERT (!GB_JUMBLED (T)) ;

        }
        else
        { 

            //------------------------------------------------------------------
            // transpose via bucket sort
            //------------------------------------------------------------------

            // T = A' and typecast to ctype
            GB_OK (GB_transpose_bucket (T, C_code_iso, ctype, C_is_csc, A,
                op1, op2, scalar, binop_bind1st,
                nworkspaces_bucket, nthreads_bucket, Context)) ;

            ASSERT_MATRIX_OK (T, "T from bucket", GB0) ;
            ASSERT (GB_JUMBLED_OK (T)) ;
        }
    }

    //==========================================================================
    // free workspace, apply positional op, and transplant/conform T into C
    //==========================================================================

    GB_FREE_WORK ;

    // free prior space of A, if transpose is done in-place
    if (in_place)
    {
        GB_phbix_free (A) ;
    }

    // transplant the control settings from A to C
    C->hyper_switch = A_hyper_switch ;
    C->bitmap_switch = A_bitmap_switch ;
    C->sparsity_control = A_sparsity_control ;

    // transplant T into the result C
    GB_OK (GB_transplant (C, ctype, &T, Context)) ;

    // apply a positional operator, after transposing the matrix
    if (op_is_positional)
    {
        if (C->iso)
        { 
            // If C was constructed as iso; it needs to be expanded first,
            // but do not initialize the values.  These are computed by
            // GB_apply_op below.
            // set C->iso = false    OK: no need to burble
            GB_OK (GB_convert_any_to_non_iso (C, false, Context)) ;
        }

        // the positional operator is applied in-place to the values of C
        // Cx = op (C)
        GB_OK (GB_apply_op ((GB_void *) C->x, ctype, GB_NON_ISO, // positional
            save_op1, save_op2, scalar, binop_bind1st, C, Context)) ;
    }

    // conform the result to the desired sparsity structure of A
    ASSERT_MATRIX_OK (C, "C to conform in GB_transpose", GB0) ;
    GB_OK (GB_conform (C, Context)) ;
    ASSERT_MATRIX_OK (C, "C output of GB_transpose", GB0) ;
    return (GrB_SUCCESS) ;
}

