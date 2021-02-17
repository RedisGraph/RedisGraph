//------------------------------------------------------------------------------
// GB_transpose:  C=A' or C=op(A'), with typecasting
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// CALLS:     GB_builder

// Transpose a matrix, C=A', and optionally apply a unary operator and/or
// typecast the values.  The transpose may be done in-place, in which case C or
// A are modified in-place.

// If the input matrix has a single vector, it must be already sorted on input.
// The input matrix may have shallow components (even if in-place), and the
// output may also have shallow components (even if the input matrix is not
// shallow).

// This function is CSR/CSC agnostic; it sets the output matrix format from
// C_is_csc but otherwise ignores the CSR/CSC type of A and C.

// If A_in is NULL, then C = (*Chandle) is transposed in-place.  If out of
// memory, (*Chandle) is always returned as NULL, which frees the input matrix
// C if the transpose is done in-place.

// If A_in is not NULL and Chandle is NULL, then A is modified in-place, and
// the A_in matrix is not freed when done.

// The bucket sort is parallel, but not highly scalable.  If e=nnz(A) and A is
// m-by-n, then at most O(e/n) threads are used.  The GB_builder method is more
// scalable, but not as fast with a modest number of threads.

#include "GB_transpose.h"
#include "GB_build.h"
#include "GB_apply.h"

#define GB_FREE_ALL ;

// free prior content of A, if transpose is done in-place
#define GB_FREE_IN_PLACE_A                                                  \
{                                                                           \
    if (in_place)                                                           \
    {                                                                       \
        /* A is being transposed in-place */                                \
        /* free prior content of A but not &A itself */                     \
        if (!Ap_shallow) GB_FREE (Ap) ;                                     \
        if (!Ah_shallow) GB_FREE (Ah) ;                                     \
        if (!Ab_shallow) GB_FREE (Ab) ;                                     \
        if (!Ai_shallow) GB_FREE (Ai) ;                                     \
        if (!Ax_shallow) GB_FREE (Ax) ;                                     \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* A is not modified; it is purely an input matrix */               \
        ;                                                                   \
    }                                                                       \
}

// free the new C matrix, unless C=A' is being done in-place of A
#define GB_FREE_C                                                           \
{                                                                           \
    if (!in_place_A)                                                        \
    {                                                                       \
        /* free all of C and all its contents &C */                         \
        GB_Matrix_free (Chandle) ;                                          \
    }                                                                       \
}

// free both A (if in-place) and C (if not in-place of A)
#define GB_FREE_A_AND_C                                                     \
{                                                                           \
    GB_FREE_IN_PLACE_A ;                                                    \
    GB_FREE_C ;                                                             \
}

//------------------------------------------------------------------------------
// GB_transpose
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_transpose           // C=A', C=(ctype)A or C=op(A')
(
    GrB_Matrix *Chandle,        // output matrix C, possibly modified in-place
    GrB_Type ctype,             // desired type of C; if NULL use A->type.
                                // ignored if op is present (cast to op->ztype)
    const bool C_is_csc,        // desired CSR/CSC format of C
    const GrB_Matrix A_in,      // input matrix
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
    GBURBLE ("(transpose) ") ;
    GrB_Matrix A, C ;
    bool in_place_C, in_place_A ;

    if (A_in == NULL)
    { 

        //----------------------------------------------------------------------
        // C = C' ; &C is transposed in-place
        //----------------------------------------------------------------------

        // GB_transpose (&C, ctype, csc, NULL, op) ;
        // C=A' is transposed in-place, in the matrix C.
        // The matrix C is freed if an error occurs and C is set to NULL.

        ASSERT (Chandle != NULL) ;  // at least &C or A must be non-NULL
        A = (*Chandle) ;
        C = A ;                     // C must be freed if an error occurs
        in_place_C = true ;         // C is modified in-place
        in_place_A = false ;
        ASSERT (A == C && A == (*Chandle)) ;

    }
    else if (Chandle == NULL || (*Chandle) == A_in)
    { 

        //----------------------------------------------------------------------
        // A = A' ; A is transposed in-place; reuse the header of A
        //----------------------------------------------------------------------

        // GB_transpose (NULL, ctype, csc, A, op) ;
        // GB_transpose (&A, ctype, csc, A, op) ;
        // C=A' is transposed in-place, in the matrix A.
        // The matrix A_in is not freed if an error occurs.

        A = A_in ;
        Chandle = &A ;              // C must not be freed if an error occurs
        C = A ;
        in_place_C = false ;
        in_place_A = true ;         // A is modified in-place
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

    ASSERT_MATRIX_OK (A, "A input for GB_transpose", GB0) ;
    ASSERT_TYPE_OK_OR_NULL (ctype, "ctype for GB_transpose", GB0) ;
    ASSERT_UNARYOP_OK_OR_NULL (op1_in, "unop for GB_transpose", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (op2_in, "binop for GB_transpose", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (scalar, "scalar for GB_transpose", GB0) ;

    // get the current sparsity control of A
    float A_hyper_switch = A->hyper_switch ;
    int A_sparsity = A->sparsity ;

    // wait if A has pending tuples or zombies, but leave it jumbled
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (A) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    GrB_Type atype = A->type ;
    size_t asize = atype->size ;
    GB_Type_code acode = atype->code ;

    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;
    int64_t anzmax = A->nzmax ;

    // if in-place, these must be freed when done, whether successful or not
    int64_t *GB_RESTRICT Ap = A->p ;
    int64_t *GB_RESTRICT Ah = A->h ;
    int64_t *GB_RESTRICT Ai = A->i ;
    int8_t  *GB_RESTRICT Ab = A->b ;
    GB_void *GB_RESTRICT Ax = (GB_void *) A->x ;

    bool A_is_bitmap  = GB_IS_BITMAP (A) ;
    bool A_is_packed  = GB_is_packed (A) ;
    bool A_is_hyper   = GB_IS_HYPERSPARSE (A) ;

    bool Ap_shallow = A->p_shallow ;
    bool Ah_shallow = A->h_shallow ;
    bool Ai_shallow = A->i_shallow ;
    bool Ax_shallow = A->x_shallow ;
    bool Ab_shallow = A->b_shallow ;

    int64_t anz = GB_NNZ (A) ;
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
        // condition holds: the first(A,y), second(x,A), and any(...) have
        // been renamed to identity(A), so these cases do not occur here.
        ASSERT (!
           ((opcode == GB_ANY_opcode) ||
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
        // replace op1 with the ONE operator, as a placeholder
        ASSERT (ctype == GrB_INT64 || ctype == GrB_INT32) ;
        op1 = (ctype == GrB_INT64) ? GxB_ONE_INT64 : GxB_ONE_INT32 ;
    }

    //--------------------------------------------------------------------------
    // C = A'
    //--------------------------------------------------------------------------

    ASSERT (GB_IMPLIES (avlen == 0 || avdim == 0, anz == 0)) ;

    bool allocate_new_Cx = (ctype != atype) || (op1 != NULL) || (op2 != NULL) ;

    if (anz == 0)
    { 

        //======================================================================
        // quick return if A is empty
        //======================================================================

        // free prior space of A, if transpose is done in-place
        GB_FREE_IN_PLACE_A ;

        // A is empty; create a new empty matrix C, with the new type and
        // dimensions.  C is hypersparse for now but may convert when
        // returned.
        info = GB_new_bix (Chandle, // hyper, old or new header
            ctype, avdim, avlen, GB_Ap_calloc, C_is_csc,
            GxB_HYPERSPARSE, true, A_hyper_switch, 1, 1, true, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_FREE_C ;
            return (info) ;
        }
        ASSERT_MATRIX_OK (*Chandle, "C transpose empty", GB0) ;
        ASSERT (!GB_JUMBLED (*Chandle)) ;

    }
    else if (A_is_packed)
    {

        //======================================================================
        // transpose a packed or bitmap matrix or vector
        //======================================================================

        // A is packed if it is either: (a) bitmap, (b) full, or (c) sparse or
        // hypersparse with all entries present, no zombies, no pending tuples,
        // and not jumbled.  For (c), the matrix A can be treated as if it was
        // full, and the pattern (A->p, A->h, and A->i) can be ignored.

        int sparsity = (A_is_bitmap) ? GxB_BITMAP : GxB_FULL ;
        bool T_cheap =                  // T can be done quickly if:
            (avlen == 1 || avdim == 1)      // A is a row or column vector,
            && op1 == NULL && op2 == NULL   // no operator to apply, and
            && atype == ctype ;             // no typecasting

        // allocate T
        GrB_Matrix T = NULL ;
        if (T_cheap)
        { 
            // allocate just the header of T, not T->b or T->x
            info = GB_new (&T,  // bitmap or full, new header
                ctype, avdim, avlen, GB_Ap_null, C_is_csc,
                sparsity, A_hyper_switch, 1, Context) ;
        }
        else
        { 
            // allocate all of T, including T->b and T->x
            info = GB_new_bix (&T,  // bitmap or full, new header
                ctype, avdim, avlen, GB_Ap_null, C_is_csc,
                sparsity, true, A_hyper_switch, 1, anzmax, true, Context) ;
        }

        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_FREE_C ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        T->magic = GB_MAGIC ;
        if (sparsity == GxB_BITMAP)
        { 
            T->nvals = anvals ;     // for bitmap case only
        }

        //----------------------------------------------------------------------
        // T = A'
        //----------------------------------------------------------------------

        // Since A is full, # threads to use is nthreads, and the
        // nworkspaces parameter is not used

        int64_t anz_held = GB_NNZ_HELD (A) ;
        int nthreads = GB_nthreads (anz_held + anvec, chunk, nthreads_max) ;

        if (T_cheap)
        {
            // no work to do.  Transposing does not change A->b or A->x
            T->b = Ab ;
            T->x = Ax ;
            T->nzmax = A->nzmax ;
            if (in_place)
            { 
                // transplant A->b and A->x into T
                T->b_shallow = Ab_shallow ;
                T->x_shallow = Ax_shallow ;
                Ab = NULL ;     // do not free prior Ab
                Ax = NULL ;     // do not free prior Ax
                A->b = NULL ;
                A->x = NULL ;
            }
            else
            { 
                // T is a purely shallow copy of A 
                T->b_shallow = (Ab != NULL) ;
                T->x_shallow = true ;
            }
        }
        else if (op1 == NULL && op2 == NULL)
        { 
            // do not apply an operator; optional typecast to C->type
            GB_transpose_ix (T, A, NULL, NULL, 0, nthreads) ;
        }
        else
        { 
            // apply an operator, C has type op->ztype
            GB_transpose_op (T, op1, op2, scalar, binop_bind1st, A,
                NULL, NULL, 0, nthreads) ;
        }

        ASSERT_MATRIX_OK (T, "T dense/bitmap", GB0) ;
        ASSERT (!GB_JUMBLED (T)) ;

        // free prior space of A, if transpose is done in-place
        GB_FREE_IN_PLACE_A ;

        //----------------------------------------------------------------------
        // transplace T into C
        //----------------------------------------------------------------------

        // allocate the output matrix C as a full or bitmap matrix
        // if *Chandle == NULL, allocate a new header; otherwise reuse existing
        info = GB_new (Chandle, // bitmap or full, old or new header
            ctype, avdim, avlen, GB_Ap_null, C_is_csc,
            sparsity, A_hyper_switch, 0, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            ASSERT (!in_place) ;    // cannot fail if in-place
            GB_FREE_C ;
            GB_Matrix_free (&T) ;
            return (info) ;
        }

        // Transplant T into the result C, making a copy if T is shallow
        info = GB_transplant (*Chandle, ctype, &T, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_FREE_A_AND_C ;
            return (GrB_OUT_OF_MEMORY) ;
        }
        ASSERT_MATRIX_OK (*Chandle, "Chandle, GB_transpose, bitmap/full", GB0) ;

    }
    else if (avdim == 1)
    {

        //======================================================================
        // transpose a "column" vector into a "row"
        //======================================================================

        // transpose a vector (avlen-by-1) into a "row" matrix (1-by-avlen).
        // A must be sorted first.
        ASSERT_MATRIX_OK (A, "the vector A must already be sorted", GB0) ;
        GB_MATRIX_WAIT (A) ;
        ASSERT (!GB_JUMBLED (A)) ;

        //----------------------------------------------------------------------
        // allocate space
        //----------------------------------------------------------------------

        // Allocate the header of C, with no C->p, C->h, C->i, C->b, or C->x
        // content, and initialize the type and dimension of C.  The new matrix
        // is hypersparse.  This step does not allocate anything if in-place.

        // if *Chandle == NULL, allocate a new header; otherwise reuse existing
        info = GB_new (Chandle, // hyper; old or new header
            ctype, 1, avlen, GB_Ap_null, C_is_csc,
            GxB_HYPERSPARSE, A_hyper_switch, 0, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            ASSERT (!in_place) ;    // cannot fail if in-place
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
        int64_t *GB_RESTRICT Cp = NULL ;
        int64_t *GB_RESTRICT Ci = NULL ;
        GB_void *GB_RESTRICT Cx = NULL ;
        bool ok = true ;
        Cp = GB_MALLOC (anz+1, int64_t) ;
        Ci = GB_CALLOC (anz  , int64_t) ;
        ok = (Cp != NULL && Ci != NULL) ;

        if (allocate_new_Cx)
        { 
            // allocate new space for the new typecasted numerical values of C
            Cx = GB_MALLOC (anz * ctype->size, GB_void) ;
            ok = ok && (Cx != NULL) ;
        }

        if (!ok)
        { 
            // out of memory
            GB_FREE (Cp) ;
            GB_FREE (Ci) ;
            GB_FREE (Cx) ;
            GB_FREE_A_AND_C ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // fill the content of C
        //----------------------------------------------------------------------

        // numerical values: apply the operator, typecast, or make shallow copy
        if (op1 != NULL || op2 != NULL)
        { 
            // Cx = op (A)
            info = GB_apply_op ( // op1 != identity of same types
                (GB_void *) Cx, op1, op2, scalar, binop_bind1st, A, Context) ;
            // GB_apply_op can only fail if op1/op2 are positional
            ASSERT (!GB_OP_IS_POSITIONAL (op1)) ;
            ASSERT (!GB_OP_IS_POSITIONAL (op2)) ;
            ASSERT (info == GrB_SUCCESS) ;
            C->x = Cx ;
            C->x_shallow = false ;
            // prior Ax will be freed
        }
        else if (ctype != atype)
        { 
            // copy the values from A into C and cast from atype to ctype
            C->x = Cx ;
            C->x_shallow = false ;
            GB_cast_array (Cx, ccode, Ax, acode, Ab, asize, anz, 1) ;
            // prior Ax will be freed
        }
        else // ctype == atype
        { 
            // no type change; numerical values of C are a shallow copy of A.
            C->x = Ax ;
            C->x_shallow = (in_place) ? Ax_shallow : true ;
            Ax = NULL ;  // do not free prior Ax
        }

        // each entry in A becomes a non-empty vector in C
        // C is a hypersparse 1-by-avlen matrix
        C->h = Ai ;
        C->h_shallow = (in_place) ? Ai_shallow : true ;
        Ai = NULL ;     // do not free prior Ai
        // C->p = 0:anz and C->i = zeros (1,anz), newly allocated
        C->plen = anz ;
        C->nvec = anz ;
        C->nvec_nonempty = anz ;
        C->i = Ci ;
        C->p = Cp ;
        // fill the vector pointers C->p
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        int64_t k ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (k = 0 ; k <= anz ; k++)
        { 
            Cp [k] = k ;
        }

        C->nzmax = anz ;
        C->magic = GB_MAGIC ;

        // free prior space of A, if transpose is done in-place
        GB_FREE_IN_PLACE_A ;

    }
    else if (avlen == 1)
    {

        //======================================================================
        // transpose a "row" into a "column" vector
        //======================================================================

        // transpose a "row" matrix (1-by-avdim) into a vector (avdim-by-1).
        // if A->vlen is 1, all vectors of A are implicitly sorted
        ASSERT_MATRIX_OK (A, "1-by-n input A already sorted", GB0) ;

        //----------------------------------------------------------------------
        // allocate workspace, if needed
        //----------------------------------------------------------------------

        int ntasks = 0 ;
        int nth = GB_nthreads (avdim, chunk, nthreads_max) ;
        int64_t *GB_RESTRICT Count = NULL ;
        if (nth > 1 && !A_is_hyper)
        {
            // ntasks and Count are not needed if nth == 1
            ntasks = 8 * nth ;
            ntasks = GB_IMIN (ntasks, avdim) ;
            ntasks = GB_IMAX (ntasks, 1) ;
            Count = GB_CALLOC (ntasks+1, int64_t) ;
            if (Count == NULL)
            { 
                // out of memory
                GB_FREE_C ;
                return (GrB_OUT_OF_MEMORY) ;
            }
        }

        // Allocate the header of C, with no C->p, C->h, C->i, or C->x content,
        // and initialize the type and dimension of C.   If in-place, A->p,
        // A->h, A->i, and A->x are all NULL.  The new matrix is sparse, but
        // can be CSR or CSC.  This step does not allocate anything if in
        // place.

        // if *Chandle == NULL, allocate a new header; otherwise reuse existing
        info = GB_new (Chandle, // sparse; old or new header
            ctype, avdim, 1, GB_Ap_null, C_is_csc,
            GxB_SPARSE, A_hyper_switch, 0, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            ASSERT (!in_place) ;        // cannot fail if in-place
            GB_FREE_C ;
            GB_FREE (Count) ;
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
        GB_void *GB_RESTRICT Cx = NULL ;
        int64_t *GB_RESTRICT Cp = NULL ;
        int64_t *GB_RESTRICT Ci = NULL ;
        bool ok = true ;
        Cp = GB_CALLOC (2, int64_t) ;
        ok = ok && (Cp != NULL) ;
        if (!A_is_hyper)
        { 
            // A is sparse, so new space is needed for Ci
            Ci = GB_MALLOC (anz, int64_t) ;
            ok = ok && (Ci != NULL) ;
        }

        if (allocate_new_Cx)
        { 
            // allocate new space for the new typecasted numerical values of C
            Cx = GB_MALLOC (anz * ctype->size, GB_void) ;
            ok = ok && (Cx != NULL) ;
        }

        if (!ok)
        { 
            // out of memory
            GB_FREE (Cp) ;
            GB_FREE (Ci) ;
            GB_FREE (Cx) ;
            GB_FREE_A_AND_C ;
            GB_FREE (Count) ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // numerical values of C: apply the op, typecast, or make shallow copy
        //----------------------------------------------------------------------

        // numerical values: apply the operator, typecast, or make shallow copy
        if (op1 != NULL || op2 != NULL)
        { 
            // Cx = op (A)
            info = GB_apply_op ( // op1 != identity of same types
                (GB_void *) Cx, op1, op2, scalar, binop_bind1st, A, Context) ;
            // GB_apply_op can only fail if op1/op2 are positional
            ASSERT (!GB_OP_IS_POSITIONAL (op1)) ;
            ASSERT (!GB_OP_IS_POSITIONAL (op2)) ;
            ASSERT (info == GrB_SUCCESS) ;
            C->x = Cx ;
            C->x_shallow = false ;
            // prior Ax will be freed
        }
        else if (ctype != atype)
        { 
            // copy the values from A into C and cast from atype to ctype
            C->x = Cx ;
            C->x_shallow = false ;
            GB_cast_array (Cx, ccode, Ax, acode, Ab, asize, anz, 1) ;
            // prior Ax will be freed
        }
        else // ctype == atype
        { 
            // no type change; numerical values of C are a shallow copy of A
            C->x = Ax ;
            C->x_shallow = (in_place) ? Ax_shallow : true ;
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

            C->i = Ah ;
            C->i_shallow = (in_place) ? Ah_shallow : true ;
            ASSERT (anvec == anz) ;
            Ah = NULL ;     // do not free prior Ah

        }
        else
        {

            //------------------------------------------------------------------
            // find the non-empty vectors of A, which become entries in C
            //------------------------------------------------------------------

            ASSERT (Ah == NULL) ;

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

                int tid ;
                #pragma omp parallel for num_threads(nth) schedule(dynamic,1)
                for (tid = 0 ; tid < ntasks ; tid++)
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
                for (tid = 0 ; tid < ntasks ; tid++)
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

            C->i = Ci ;
            C->i_shallow = false ;
        }

        //---------------------------------------------------------------------
        // vector pointers of C
        //---------------------------------------------------------------------

        // C->p = [0 anz] and C->h = NULL
        ASSERT (C->plen == 1) ;
        ASSERT (C->nvec == 1) ;
        ASSERT (C->h == NULL) ;
        C->p = Cp ;
        C->p_shallow = false ;
        C->nvec_nonempty = (anz == 0) ? 0 : 1 ;
        // fill the vector pointers C->p
        Cp [0] = 0 ;
        Cp [1] = anz ;
        C->nzmax = anz ;
        C->magic = GB_MAGIC ;
        ASSERT (!GB_JUMBLED (C)) ;

        // free prior space of A, if transpose done in-place, and free workspace
        GB_FREE_IN_PLACE_A ;
        GB_FREE (Count) ;

    }
    else
    {

        //======================================================================
        // transpose a general sparse or hypersparse matrix
        //======================================================================

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

            //==================================================================
            // transpose via GB_builder
            //==================================================================

            //------------------------------------------------------------------
            // allocate and create iwork
            //------------------------------------------------------------------

            // allocate iwork of size anz
            int64_t *iwork = GB_MALLOC (anz, int64_t) ;
            if (iwork == NULL)
            { 
                // out of memory
                GB_FREE_C ;
                return (GrB_OUT_OF_MEMORY) ;
            }

            // Construct the "row" indices of C, which are "column" indices of
            // A.  This array becomes the permanent T->i on output.  This phase
            // must be done before Chandle is created below, since that step
            // destroys A.

            int nthreads = GB_nthreads (anz + anvec, chunk, nthreads_max) ;
            GB_extract_vector_list (iwork, A, nthreads) ;

            //------------------------------------------------------------------
            // allocate the output matrix and additional space (jwork and S)
            //------------------------------------------------------------------

            // Allocate the header of C, with no C->p, C->h, C->i, or C->x
            // content, and initialize the type and dimension of C.   If in
            // place, A->p, A->h, A->i, and A->x are all NULL.  The new matrix
            // is hypersparse, but can be CSR or CSC.  This step does not
            // allocate anything if in-place.

            // if *Chandle == NULL, allocate a new header; otherwise reuse
            info = GB_new (Chandle, // hyper, old or new header
                ctype, avdim, avlen, GB_Ap_null, C_is_csc,
                GxB_HYPERSPARSE, A_hyper_switch, 0, Context) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                ASSERT (!in_place) ;        // cannot fail if in-place
                GB_FREE (iwork) ;
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
                if (!Ap_shallow) GB_FREE (Ap) ;
                if (!Ah_shallow) GB_FREE (Ah) ;
            }

            int64_t *jwork = NULL ;
            GB_Type_code scode ;
            GB_void *S = NULL ;
            GB_void *Swork = NULL ;

            // for the GB_builder method, if the transpose is done in-place and
            // A->i is not shallow, A->i can be used and then freed.
            // Otherwise, A->i is not modified at all.
            bool ok = true ;
            bool recycle_Ai = (in_place && !Ai_shallow) ;
            if (!recycle_Ai)
            { 
                // allocate jwork of size anz
                jwork = GB_MALLOC (anz, int64_t) ;
                ok = ok && (jwork != NULL) ;
            }

            if (op1 != NULL || op2 != NULL)
            { 
                // allocate Swork of size anz * csize
                Swork = GB_MALLOC (anz * csize, GB_void) ;
                ok = ok && (Swork != NULL) ;
            }

            if (!ok)
            { 
                // out of memory
                GB_FREE (iwork) ;
                GB_FREE (jwork) ;
                GB_FREE (Swork) ;
                GB_FREE_A_AND_C ;
                return (GrB_OUT_OF_MEMORY) ;
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
            if (op1 != NULL || op2 != NULL)
            { 
                // Swork = op (A)
                info = GB_apply_op ( // op1 != identity of same types
                    (GB_void *) Swork, op1, op2, scalar, binop_bind1st,
                    A, Context) ;
                // GB_apply_op can only fail if op1/op2 are positional
                ASSERT (!GB_OP_IS_POSITIONAL (op1)) ;
                ASSERT (!GB_OP_IS_POSITIONAL (op2)) ;
                ASSERT (info == GrB_SUCCESS) ;
                // GB_builder will not need to typecast Swork to T->x, and it
                // may choose to transplant it into T->x
                scode = ccode ;
                #if 0
                if (in_place && !Ax_shallow)
                {
                    // A is being transposed in-place so A->x is no longer
                    // needed.  If A->x is shallow this can be skipped.  T->x
                    // will not be shallow if the op is present.  A->x should
                    // be freed early to free up space for GB_builder.
                    // However, in the current usage, when op is used, A is not
                    // transposed in-place, so this step is not needed.
                    ASSERT (GB_DEAD_CODE) ;
                    GB_FREE (Ax) ;
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

            GrB_Matrix T = NULL ;
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

            // Free the prior content of the input matrix, if done in-place.
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
            ASSERT (!GB_JUMBLED (T)) ;
            info = GB_transplant (*Chandle, ctype, &T, Context) ;
            ASSERT (info == GrB_SUCCESS) ;

        }
        else
        {

            //==================================================================
            // transpose via bucket sort
            //==================================================================

            // This method does not operate on the matrix in-place, so it must
            // create a temporary matrix T.  Then the input matrix is freed and
            // replaced with the new matrix T.

            // T is also typecasted to ctype, if not NULL
            GrB_Matrix T = NULL ;
            info = GB_transpose_bucket (&T, ctype, C_is_csc, A,
                op1, op2, scalar, binop_bind1st,
                nworkspaces_bucket, nthreads_bucket, Context) ;

            // free prior content, if C=A' is being done in-place
            if (in_place_A)
            { 
                // free all content of A, but not the header, if in-place of A
                GB_phbix_free (A) ;   // transpose in-place
            }
            else if (in_place_C)
            { 
                // free all of C, including the header, if done in-place of C
                GB_Matrix_free (Chandle) ;
            }

            if (info != GrB_SUCCESS)
            { 
                // out of memory in GB_transpose_bucket
                GB_FREE_C ;
                return (info) ;
            }

            ASSERT_MATRIX_OK (T, "T from bucket", GB0) ;
            ASSERT (GB_JUMBLED_OK (T)) ;

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
                // If C=A' is done in-place of C, then the header and content
                // of the input C has been freed.  The output T can now be
                // moved to the Chandle.
                ASSERT (*Chandle == NULL) ;
                (*Chandle) = T ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // get the output matrix
    //--------------------------------------------------------------------------

    C = (*Chandle) ;
    ASSERT (GB_JUMBLED_OK (C)) ;

    //--------------------------------------------------------------------------
    // apply a positional operator, after transposing the matrix
    //--------------------------------------------------------------------------

    if (op_is_positional)
    { 
        // the positional operator is applied in-place to the values of C
        op1 = save_op1 ;
        op2 = save_op2 ;
        // Cx = op (C)
        info = GB_apply_op (C->x, op1,  // positional unary/binary op only
            op2, scalar, binop_bind1st, C, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_FREE_C ;
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // conform the result to the desired sparisty structure of A
    //--------------------------------------------------------------------------

    // transplant the hyper_switch and sparsity structure from A to C
    C->hyper_switch = A_hyper_switch ;
    C->sparsity = A_sparsity ;  // transplant sparsity control into C

    ASSERT_MATRIX_OK (C, "C to conform in GB_transpose", GB0) ;

    info = GB_conform (C, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_C ;
        return (info) ;
    }

    ASSERT_MATRIX_OK (*Chandle, "Chandle conformed in GB_transpose", GB0) ;
    return (GrB_SUCCESS) ;
}

