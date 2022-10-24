//------------------------------------------------------------------------------
// GB_selector:  select entries from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_selector does the work for GB_select and the GxB_*select methods.  It
// also deletes zombies for GB_wait using the NONZOMBIE operator, and deletes
// entries outside a smaller matrix for GxB_*resize.

// TODO: GB_selector does not exploit the mask.

// If C is NULL on input, A is modified in-place.
// Otherwise, C is an uninitialized static header.

#include "GB_select.h"
#include "GB_ek_slice.h"
#include "GB_sel__include.h"
#include "GB_scalar.h"
#include "GB_transpose.h"

#define GB_FREE_WORKSPACE                   \
{                                           \
    GB_FREE_WORK (&Zp, Zp_size) ;           \
    GB_WERK_POP (Work, int64_t) ;           \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
    GB_FREE (&Cp, Cp_size) ;                \
    GB_FREE (&Ch, Ch_size) ;                \
    GB_FREE (&Ci, Ci_size) ;                \
    GB_FREE (&Cx, Cx_size) ;                \
}

#define GB_FREE_ALL                         \
{                                           \
    GB_phbix_free (C) ;                     \
    GB_FREE_WORKSPACE ;                     \
}

GrB_Info GB_selector
(
    GrB_Matrix C,               // output matrix, NULL or existing header
    GB_Opcode opcode,           // selector opcode
    const GB_Operator op,       // user operator, NULL for resize/nonzombie
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    int64_t ithunk,             // (int64_t) Thunk, if Thunk is NULL
    const GrB_Scalar Thunk,     // optional input for select operator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_OP_OK_OR_NULL (op, "selectop/idxunop for GB_selector", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (Thunk, "Thunk for GB_selector", GB0) ;
    ASSERT (GB_IS_SELECTOP_CODE (opcode) || GB_IS_INDEXUNARYOP_CODE (opcode)) ;
    ASSERT_MATRIX_OK (A, "A input for GB_selector", GB_FLIP (GB0)) ;
    // positional selector (tril, triu, diag, offdiag, resize, rowindex, ...):
    // can't be jumbled.  nonzombie, entry-valued op, user op: jumbled OK
    ASSERT (GB_IMPLIES (GB_OPCODE_IS_POSITIONAL (opcode), !GB_JUMBLED (A))) ;
    ASSERT (C == NULL || (C != NULL && (C->static_header || GBNSTATIC))) ;

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

    bool in_place_A = (C == NULL) ; // GrB_wait and GB_resize only
    int64_t *restrict Zp = NULL ; size_t Zp_size = 0 ;
    GB_WERK_DECLARE (Work, int64_t) ;
    int64_t *restrict Wfirst = NULL ;
    int64_t *restrict Wlast = NULL ;
    int64_t *restrict Cp_kfirst = NULL ;
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;

    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;
    const bool A_iso = A->iso ;

    int64_t *restrict Cp = NULL ; size_t Cp_size = 0 ;
    int64_t *restrict Ch = NULL ; size_t Ch_size = 0 ;
    int64_t *restrict Ci = NULL ; size_t Ci_size = 0 ;
    GB_void *restrict Cx = NULL ; size_t Cx_size = 0 ;

    //--------------------------------------------------------------------------
    // get Thunk
    //--------------------------------------------------------------------------

    // The scalar value of Thunk has already been typecasted to an integer
    // (int64_t ithunk).

    // It is also now typecast to the same type as A (to the scalar athunk)
    // which is required for GxB_SelectOps, and to the op->ytype (the scalar
    // ythunk) for GrB_IndexUnaryOps.

    // If Thunk is NULL, or has no entry, it is treated as a scalar value
    // of zero.

    const size_t asize = A->type->size ;
    const GB_Type_code acode = A->type->code ;

    GrB_Type ytype = NULL, xtype = NULL ;
    GB_Type_code ycode = GB_ignore_code, xcode = GB_ignore_code ;
    size_t ysize = 1, xsize = 1 ;

    if (op != NULL)
    {
        if (op->ytype != NULL)
        { 
            // get the type of the thunk input of the operator
            ytype = op->ytype ;
            ycode = ytype->code ;
            ysize = ytype->size ;
        }
        if (op->xtype != NULL)
        { 
            // get the type of the A input of the operator
            xtype = op->xtype ;
            xcode = xtype->code ;
            xsize = xtype->size ;
        }
    }

    // athunk = (A->type) Thunk, for selectop thunk comparators only
    GB_void athunk [GB_VLA(asize)] ;
    memset (athunk, 0, asize) ;

    // ythunk = (op->ytype) Thunk, for idxnunop
    GB_void ythunk [GB_VLA(ysize)] ;
    memset (ythunk, 0, ysize) ;

    bool op_is_selectop = GB_IS_SELECTOP_CODE (opcode) ;
    bool op_is_idxunop  = GB_IS_INDEXUNARYOP_CODE (opcode) ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;

    if (Thunk != NULL)
    {
        // Thunk is passed to GB_selector only if it is non-empty
        ASSERT (GB_nnz ((GrB_Matrix) Thunk) > 0) ;
        const GB_Type_code tcode = Thunk->type->code ;
        if (op_is_selectop && opcode != GB_USER_selop_code)
        { 
            // athunk = (atype) Thunk, for built-in GxB_SelectOps only
            GB_cast_scalar (athunk, acode, Thunk->x, tcode, asize) ;
        }
        if (ytype != NULL)
        { 
            // ythunk = (op->ytype) Thunk
            GB_cast_scalar (ythunk, ycode, Thunk->x, tcode, ysize) ;
        }
    }

    //--------------------------------------------------------------------------
    // handle iso case for built-in select ops that depend only on the value
    //--------------------------------------------------------------------------

    bool op_is_select_valued =
        opcode >= GB_NONZERO_selop_code && opcode <= GB_LE_THUNK_selop_code ;

    bool op_is_idxunop_valued =
        opcode >= GB_VALUENE_idxunop_code && opcode <= GB_VALUELE_idxunop_code ;

    if (A_iso && (op_is_select_valued || op_is_idxunop_valued))
    { 

        // select op is NONZERO, EQ_ZERO, GT_ZERO, GE_ZERO, LT_ZERO, LE_ZERO,
        // EQ_THUNK, GT_THUNK, GE_THUNK, LT_THUNK, or LE_THUNK, or the idxunop
        // VALUE* operators.  All of these select/idxunop ops depend only on
        // the value of A(i,j).  Since A is iso, either all entries in A will
        // be copied to C and thus C can be created as a shallow copy of A, or
        // no entries from A will be copied to C and thus C is an empty matrix.
        // The select factory is not needed, except to check the iso value via
        // GB_bitmap_selector.

        ASSERT (!in_place_A) ;
        ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;

        // construct a scalar containing the iso scalar of A

        // xscalar = (op->xtype) A->x for idxunops
        GB_void xscalar [GB_VLA(xsize)] ;
        memset (xscalar, 0, xsize) ;

        struct GB_Scalar_opaque S_header ;
        GrB_Scalar S ;
        if (op_is_select_valued)
        { 
            // wrap the iso-value of A in the scalar S, with no typecasting
            S = GB_Scalar_wrap (&S_header, A->type, A->x) ;
        }
        else
        { 
            // wrap the iso-value of A in the scalar S, typecasted to xtype
            // xscalar = (op->xtype) A->x
            GB_cast_scalar (xscalar, xcode, A->x, acode, asize) ;
            S = GB_Scalar_wrap (&S_header, xtype, xscalar) ;
        }
        S->iso = false ;    // but ensure S is not iso
        ASSERT_SCALAR_OK (S, "iso scalar wrap", GB0) ;

        // apply the select operator to the iso scalar S
        GB_OK (GB_bitmap_selector (C, false, opcode, op, false,
            (GrB_Matrix) S, ithunk, athunk, ythunk, Context)) ;
        ASSERT_MATRIX_OK (C, "C from iso scalar test", GB0) ;
        bool C_empty = (GB_nnz (C) == 0) ;
        GB_phbix_free (C) ;

        // check if C has 0 or 1 entry
        if (C_empty)
        { 
            // C is an empty matrix
            return (GB_new (&C, // existing header
                A->type, avlen, avdim, GB_Ap_calloc, true,
                GxB_SPARSE + GxB_HYPERSPARSE, GB_Global_hyper_switch_get ( ),
                1, Context)) ;
        }
        else
        { 
            // C is a shallow copy of A with all the same entries as A
            // set C->iso = A->iso  OK
            return (GB_shallow_copy (C, true, A, Context)) ;
        }
    }

    // now if A is iso, the following operators still need to be handled:

    //      GB_TRIL_selop_code        : use GB_sel__tril_iso
    //      GB_TRIU_selop_code        : use GB_sel__triu_iso
    //      GB_DIAG_selop_code        : use GB_sel__diag_iso
    //      GB_OFFDIAG_selop_code     : use GB_sel__offdiag_iso
    //      GB_NONZOMBIE_selop_code   : use GB_sel__nonzombie_iso
    //      GB_USER_selop_code        : use GB_sel__user_iso
    //      GB_ROWINDEX_idxunop_code  : use GB_sel__rowindex_iso
    //      GB_ROWLE_idxunop_code     : use GB_sel__rowle_iso
    //      GB_ROWGT_idxunop_code     : use GB_sel__rowle_iso
    //      all other idxunop         : use GB_sel__idxunop_iso

    // column selectors are handled below:
    //      GB_COLINDEX_idxunop_code  : 
    //      GB_COLLE_idxunop_code     : 
    //      GB_COLGT_idxunop_code     : 

    // Except for GB_USER_selop_code and idxunop, the GB_sel__*_iso methods do
    // not access the values of A and C, just the pattern.

    //--------------------------------------------------------------------------
    // handle the bitmap/as-if-full case
    //--------------------------------------------------------------------------

    bool use_bitmap_selector ;
    if (opcode == GB_NONZOMBIE_selop_code || in_place_A)
    { 
        // GB_bitmap_selector does not support the nonzombie opcode, nor does
        // it support operating on A in place.  For the NONZOMBIE operator, A
        // will never be bitmap.
        use_bitmap_selector = false ;
    }
    else if (opcode == GB_DIAG_selop_code)
    { 
        // GB_bitmap_selector supports the DIAG operator, but it is currently
        // not efficient (GB_bitmap_selector should return a sparse diagonal
        // matrix, not bitmap).  So use the sparse case if A is not bitmap,
        // since the sparse case below does not support the bitmap case.
        use_bitmap_selector = GB_IS_BITMAP (A) ;
    }
    else
    { 
        // For bitmap, full, or as-if-full matrices (sparse/hypersparse with
        // all entries present, not jumbled, no zombies, and no pending
        // tuples), use the bitmap selector for all other operators (TRIL,
        // TRIU, OFFDIAG, NONZERO, EQ*, GT*, GE*, LT*, LE*, and user-defined
        // operators).
        use_bitmap_selector = GB_IS_BITMAP (A) || GB_as_if_full (A) ;
    }

    //--------------------------------------------------------------------------
    // determine if C is iso for a non-iso A
    //--------------------------------------------------------------------------

    bool C_iso = A_iso ||                       // C iso value is Ax [0]
        (opcode == GB_EQ_ZERO_selop_code) ||        // C iso value is zero
        (opcode == GB_EQ_THUNK_selop_code) ||       // C iso value is thunk
        (opcode == GB_NONZERO_selop_code &&
         acode == GB_BOOL_code) ;               // C iso value is true

    if (C_iso)
    { 
        GB_BURBLE_MATRIX (A, "(iso select) ") ;
    }

    //==========================================================================
    // bitmap/full case
    //==========================================================================

    if (use_bitmap_selector)
    { 
        GB_BURBLE_MATRIX (A, "(bitmap select) ") ;
        ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;
        return (GB_bitmap_selector (C, C_iso, opcode, op,                  
            flipij, A, ithunk, athunk, ythunk, Context)) ;
    }

    //==========================================================================
    // sparse/hypersparse case
    //==========================================================================

    //--------------------------------------------------------------------------
    // determine the max number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get A: sparse, hypersparse, or full
    //--------------------------------------------------------------------------

    // the case when A is bitmap is always handled above by GB_bitmap_selector
    ASSERT (!GB_IS_BITMAP (A)) ;

    int64_t *restrict Ap = A->p ; size_t Ap_size = A->p_size ;
    int64_t *restrict Ah = A->h ;
    int64_t *restrict Ai = A->i ; size_t Ai_size = A->i_size ;
    GB_void *restrict Ax = (GB_void *) A->x ; size_t Ax_size = A->x_size ;
    int64_t anvec = A->nvec ;
    bool A_jumbled = A->jumbled ;
    bool A_is_hyper = (Ah != NULL) ;

    //==========================================================================
    // column selector
    //==========================================================================

    // The column selectors can be done in a single pass.

    if (opcode == GB_COLINDEX_idxunop_code ||
        opcode == GB_COLLE_idxunop_code ||
        opcode == GB_COLGT_idxunop_code)
    {

        //----------------------------------------------------------------------
        // find column j in A
        //----------------------------------------------------------------------

        ASSERT_MATRIX_OK (A, "A for col selector", GB_FLIP (GB0)) ;
        int nth = nthreads_max ;
        ASSERT (!in_place_A) ;
        ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;
        ASSERT (GB_JUMBLED_OK (A)) ;

        int64_t j = (opcode == GB_COLINDEX_idxunop_code) ? (-ithunk) : ithunk ;

        int64_t k = 0 ;
        bool found ;
        if (j < 0)
        { 
            // j is outside the range of columns of A
            k = 0 ;
            found = false ;
        }
        else if (j >= avdim)
        { 
            // j is outside the range of columns of A
            k = anvec ;
            found = false ;
        }
        else if (A_is_hyper)
        { 
            // find the column j in the hyperlist of A
            int64_t kright = anvec-1 ;
            GB_SPLIT_BINARY_SEARCH (j, Ah, k, kright, found) ;
            // if found is true the Ah [k] == j
            // if found is false, then Ah [0..k-1] < j and Ah [k..anvec-1] > j
        }
        else
        { 
            // j appears as the jth column in A; found is always true
            k = j ;
            found = true ;
        }

        //----------------------------------------------------------------------
        // determine the # of entries and # of vectors in C
        //----------------------------------------------------------------------

        int64_t pstart = Ap [k] ;
        int64_t pend = found ? Ap [k+1] : pstart ;
        int64_t ajnz = pend - pstart ;
        int64_t cnz, cnvec ;
        int64_t anz = Ap [anvec] ;

        if (opcode == GB_COLINDEX_idxunop_code)
        { 
            // COLINDEX: delete column j:  C = A (:, [0:j-1 j+1:end])
            cnz = anz - ajnz ;
            cnvec = (A_is_hyper && found) ? (anvec-1) : anvec ;
        }
        else if (opcode == GB_COLLE_idxunop_code)
        { 
            // COLLE: C = A (:, 0:j)
            cnz = pend ;
            cnvec = (A_is_hyper) ? (found ? (k+1) : k) : anvec ;
        }
        else // (opcode == GB_COLGT_idxunop_code)
        { 
            // COLGT: C = A (:, j+1:end)
            cnz = anz - pend ;
            cnvec = anvec - ((A_is_hyper) ? (found ? (k+1) : k) : 0) ;
        }

        if (cnz == anz)
        { 
            // C is the same as A: return it a pure shallow copy
            return (GB_shallow_copy (C, true, A, Context)) ;
        }
        else if (cnz == 0)
        { 
            // return C as empty
            return (GB_new (&C, // auto (sparse or hyper), existing header
                A->type, avlen, avdim, GB_Ap_calloc, true,
                GxB_HYPERSPARSE, GB_Global_hyper_switch_get ( ), 1, Context)) ;
        }

        //----------------------------------------------------------------------
        // allocate C
        //----------------------------------------------------------------------

        int sparsity = (A_is_hyper) ? GxB_HYPERSPARSE : GxB_SPARSE ;
        GB_OK (GB_new_bix (&C, // sparse or hyper (from A), existing header
            A->type, avlen, avdim, GB_Ap_malloc, true, sparsity, false,
            A->hyper_switch, cnvec, cnz, true, A_iso, Context)) ;

        ASSERT (info == GrB_SUCCESS) ;
        int nth2 = GB_nthreads (cnvec, chunk, nth) ;

        int64_t *restrict Cp = C->p ;
        int64_t *restrict Ch = C->h ;
        int64_t *restrict Ci = C->i ;
        GB_void *restrict Cx = (GB_void *) C->x ;
        int64_t kk ;

        //----------------------------------------------------------------------
        // construct C
        //----------------------------------------------------------------------

        if (A_iso)
        { 
            // Cx [0] = Ax [0]
            memcpy (Cx, Ax, asize) ;
        }

        if (opcode == GB_COLINDEX_idxunop_code)
        {

            //------------------------------------------------------------------
            // COLINDEX: delete the column j
            //------------------------------------------------------------------

            if (A_is_hyper)
            { 
                ASSERT (found) ;
                // Cp [0:k-1] = Ap [0:k-1]
                GB_memcpy (Cp, Ap, k * sizeof (int64_t), nth) ;
                // Cp [k:cnvec] = Ap [k+1:anvec] - ajnz
                #pragma omp parallel for num_threads(nth2)
                for (kk = k ; kk <= cnvec ; kk++)
                { 
                    Cp [kk] = Ap [kk+1] - ajnz ;
                }
                // Ch [0:k-1] = Ah [0:k-1]
                GB_memcpy (Ch, Ah, k * sizeof (int64_t), nth) ;
                // Ch [k:cnvec-1] = Ah [k+1:anvec-1]
                GB_memcpy (Ch + k, Ah + (k+1), (cnvec-k) * sizeof (int64_t),
                    nth) ;
            }
            else
            { 
                // Cp [0:k] = Ap [0:k]
                GB_memcpy (Cp, Ap, (k+1) * sizeof (int64_t), nth) ;
                // Cp [k+1:anvec] = Ap [k+1:anvec] - ajnz
                #pragma omp parallel for num_threads(nth2)
                for (kk = k+1 ; kk <= cnvec ; kk++)
                { 
                    Cp [kk] = Ap [kk] - ajnz ;
                }
            }
            // Ci [0:pstart-1] = Ai [0:pstart-1]
            GB_memcpy (Ci, Ai, pstart * sizeof (int64_t), nth) ;
            // Ci [pstart:cnz-1] = Ai [pend:anz-1]
            GB_memcpy (Ci + pstart, Ai + pend,
                (cnz - pstart) * sizeof (int64_t), nth) ;
            if (!A_iso)
            { 
                // Cx [0:pstart-1] = Ax [0:pstart-1]
                GB_memcpy (Cx, Ax, pstart * asize, nth) ;
                // Cx [pstart:cnz-1] = Ax [pend:anz-1]
                GB_memcpy (Cx + pstart * asize, Ax + pend * asize,
                    (cnz - pstart) * asize, nth) ;
            }

        }
        else if (opcode == GB_COLLE_idxunop_code)
        {

            //------------------------------------------------------------------
            // COLLE: C = A (:, 0:j)
            //------------------------------------------------------------------

            if (A_is_hyper)
            { 
                // Cp [0:cnvec] = Ap [0:cnvec]
                GB_memcpy (Cp, Ap, (cnvec+1) * sizeof (int64_t), nth) ;
                // Ch [0:cnvec-1] = Ah [0:cnvec-1]
                GB_memcpy (Ch, Ah, (cnvec) * sizeof (int64_t), nth) ;
            }
            else
            {
                // Cp [0:k+1] = Ap [0:k+1]
                ASSERT (found) ;
                GB_memcpy (Cp, Ap, (k+2) * sizeof (int64_t), nth) ;
                // Cp [k+2:cnvec] = cnz
                #pragma omp parallel for num_threads(nth2)
                for (kk = k+2 ; kk <= cnvec ; kk++)
                { 
                    Cp [kk] = cnz ;
                }
            }
            // Ci [0:cnz-1] = Ai [0:cnz-1]
            GB_memcpy (Ci, Ai, cnz * sizeof (int64_t), nth) ;
            if (!A_iso)
            { 
                // Cx [0:cnz-1] = Ax [0:cnz-1]
                GB_memcpy (Cx, Ax, cnz * asize, nth) ;
            }

        }
        else // (opcode == GB_COLGT_idxunop_code)
        {

            //------------------------------------------------------------------
            // COLGT: C = A (:, j+1:end)
            //------------------------------------------------------------------

            if (A_is_hyper)
            { 
                // Cp [0:cnvec] = Ap [k+found:anvec] - pend
                #pragma omp parallel for num_threads(nth2)
                for (kk = 0 ; kk <= cnvec ; kk++)
                { 
                    Cp [kk] = Ap [kk + k + found] - pend ;
                }
                // Ch [0:cnvec-1] = Ah [k+found:anvec-1]
                GB_memcpy (Ch, Ah + k + found, cnvec * sizeof (int64_t), nth) ;
            }
            else
            {
                ASSERT (found) ;
                // Cp [0:k] = 0
                GB_memset (Cp, 0, (k+1) * sizeof (int64_t), nth) ;
                // Cp [k+1:cnvec] = Ap [k+1:cnvec] - pend
                #pragma omp parallel for num_threads(nth2)
                for (kk = k+1 ; kk <= cnvec ; kk++)
                { 
                    Cp [kk] = Ap [kk] - pend ;
                }
            }
            // Ci [0:cnz-1] = Ai [pend:anz-1]
            GB_memcpy (Ci, Ai + pend, cnz * sizeof (int64_t), nth) ;
            if (!A_iso)
            { 
                // Cx [0:cnz-1] = Ax [pend:anz-1]
                GB_memcpy (Cx, Ax + pend * asize, cnz * asize, nth) ;
            }
        }

        //----------------------------------------------------------------------
        // finalize the matrix, free workspace, and return result
        //----------------------------------------------------------------------

        C->nvec = cnvec ;
        C->magic = GB_MAGIC ;
        C->jumbled = A_jumbled ;    // C is jumbled if A is jumbled
        C->iso = C_iso ;            // OK: burble already done above
        C->nvec_nonempty = GB_nvec_nonempty (C, Context) ;
        ASSERT_MATRIX_OK (C, "C output for GB_selector (column select)", GB0) ;
        return (GrB_SUCCESS) ;
    }

    //==========================================================================
    // all other select/idxunop operators
    //==========================================================================

    #undef  GB_FREE_ALL
    #define GB_FREE_ALL                         \
    {                                           \
        GB_phbix_free (C) ;                     \
        GB_FREE_WORKSPACE ;                     \
    }

    //--------------------------------------------------------------------------
    // allocate the new vector pointers of C
    //--------------------------------------------------------------------------

    int64_t cnz = 0 ;

    Cp = GB_CALLOC (anvec+1, int64_t, &Cp_size) ;
    if (Cp == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    int A_ntasks, A_nthreads ;
    double work = 8*anvec
        + ((opcode == GB_DIAG_selop_code) ? 0 : GB_nnz_held (A)) ;
    GB_SLICE_MATRIX_WORK (A, 8, chunk, work) ;

    //--------------------------------------------------------------------------
    // allocate workspace for each task
    //--------------------------------------------------------------------------

    GB_WERK_PUSH (Work, 3*A_ntasks, int64_t) ;
    if (Work == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }
    Wfirst    = Work ;
    Wlast     = Work + A_ntasks ;
    Cp_kfirst = Work + A_ntasks * 2 ;

    //--------------------------------------------------------------------------
    // allocate workspace for phase1
    //--------------------------------------------------------------------------

    // phase1 counts the number of live entries in each vector of A.  The
    // result is computed in Cp, where Cp [k] is the number of live entries in
    // the kth vector of A.  Zp [k] is the location of the A(i,k) entry, for
    // positional operators.

    if (op_is_positional)
    {
        // allocate Zp
        Zp = GB_MALLOC_WORK (anvec, int64_t, &Zp_size) ;
        if (Zp == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // phase1: count the live entries in each column
    //--------------------------------------------------------------------------

    // define the worker for the switch factory
    #define GB_SELECT_PHASE1
    #define GB_sel1(opname,aname) GB (_sel_phase1_ ## opname ## aname)
    #define GB_SEL_WORKER(opname,aname,atype)                               \
    {                                                                       \
        GB_sel1 (opname, aname) (Zp, Cp, Wfirst, Wlast, A,                  \
            flipij, ithunk, (atype *) athunk, ythunk, op,                   \
            A_ek_slicing, A_ntasks, A_nthreads) ;                           \
    }                                                                       \
    break ;

    // launch the switch factory
    const GB_Type_code typecode = (A_iso) ? GB_ignore_code : acode ;
    #include "GB_select_factory.c"

    #undef  GB_SELECT_PHASE1
    #undef  GB_SEL_WORKER

    //--------------------------------------------------------------------------
    // cumulative sum of Cp and compute Cp_kfirst
    //--------------------------------------------------------------------------

    int64_t C_nvec_nonempty ;
    GB_ek_slice_merge2 (&C_nvec_nonempty, Cp_kfirst, Cp, anvec,
        Wfirst, Wlast, A_ek_slicing, A_ntasks, A_nthreads, Context) ;

    //--------------------------------------------------------------------------
    // allocate new space for the compacted Ci and Cx
    //--------------------------------------------------------------------------

    cnz = Cp [anvec] ;
    cnz = GB_IMAX (cnz, 1) ;
    Ci = GB_MALLOC (cnz, int64_t, &Ci_size) ;
    // use calloc since C is sparse, not bitmap
    Cx = (GB_void *) GB_XALLOC (false, C_iso, cnz, asize, &Cx_size) ; // x:OK
    if (Ci == NULL || Cx == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // set the iso value of C
    //--------------------------------------------------------------------------

    if (C_iso)
    { 
        // The pattern of C is computed by the worker below, for the DIAG,
        // OFFDIAG, TRIL, TRIU, NONZOMBIE, and USER select operators.
        GB_iso_select (Cx, opcode, athunk, Ax, acode, asize) ;
    }

    //--------------------------------------------------------------------------
    // phase2: select the entries
    //--------------------------------------------------------------------------

    // define the worker for the switch factory
    #define GB_SELECT_PHASE2
    #define GB_sel2(opname,aname) GB (_sel_phase2_ ## opname ## aname)
    #define GB_SEL_WORKER(opname,aname,atype)                               \
    {                                                                       \
        GB_sel2 (opname, aname) (Ci, (atype *) Cx, Zp, Cp, Cp_kfirst, A,    \
            flipij, ithunk, (atype *) athunk, ythunk, op,                   \
            A_ek_slicing, A_ntasks, A_nthreads) ;                           \
    }                                                                       \
    break ;

    // launch the switch factory
    #include "GB_select_factory.c"

    //--------------------------------------------------------------------------
    // create the result
    //--------------------------------------------------------------------------

    if (in_place_A)
    {

        //----------------------------------------------------------------------
        // transplant Cp, Ci, Cx back into A
        //----------------------------------------------------------------------

        // TODO: this is not parallel: use GB_hyper_prune
        if (A->h != NULL && C_nvec_nonempty < anvec)
        {
            // prune empty vectors from Ah and Ap
            int64_t cnvec = 0 ;
            for (int64_t k = 0 ; k < anvec ; k++)
            {
                if (Cp [k] < Cp [k+1])
                { 
                    Ah [cnvec] = Ah [k] ;
                    Ap [cnvec] = Cp [k] ;
                    cnvec++ ;
                }
            }
            Ap [cnvec] = Cp [anvec] ;
            A->nvec = cnvec ;
            ASSERT (A->nvec == C_nvec_nonempty) ;
            GB_FREE (&Cp, Cp_size) ;
        }
        else
        { 
            // free the old A->p and transplant in Cp as the new A->p
            GB_FREE (&Ap, Ap_size) ;
            A->p = Cp ; Cp = NULL ; A->p_size = Cp_size ;
            A->plen = anvec ;
        }

        ASSERT (Cp == NULL) ;

        GB_FREE (&Ai, Ai_size) ;
        GB_FREE (&Ax, Ax_size) ;
        A->i = Ci ; Ci = NULL ; A->i_size = Ci_size ;
        A->x = Cx ; Cx = NULL ; A->x_size = Cx_size ;
        A->nvec_nonempty = C_nvec_nonempty ;
        A->jumbled = A_jumbled ;        // A remains jumbled (in-place select)
        A->iso = C_iso ;                // OK: burble already done above

        // the NONZOMBIE opcode may have removed all zombies, but A->nzombie
        // is still nonzero.  It is set to zero in GB_wait.
        ASSERT_MATRIX_OK (A, "A output for GB_selector", GB_FLIP (GB0)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // create C and transplant Cp, Ch, Ci, Cx into C
        //----------------------------------------------------------------------

        int sparsity = (A_is_hyper) ? GxB_HYPERSPARSE : GxB_SPARSE ;
        ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;
        info = GB_new (&C, // sparse or hyper (from A), existing header
            A->type, avlen, avdim, GB_Ap_null, true,
            sparsity, A->hyper_switch, anvec, Context) ;
        ASSERT (info == GrB_SUCCESS) ;

        if (A->h != NULL)
        {

            //------------------------------------------------------------------
            // A and C are hypersparse: copy non-empty vectors from Ah to Ch
            //------------------------------------------------------------------

            Ch = GB_MALLOC (anvec, int64_t, &Ch_size) ;
            if (Ch == NULL)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GrB_OUT_OF_MEMORY) ;
            }

            // TODO: do in parallel: use GB_hyper_prune
            int64_t cnvec = 0 ;
            for (int64_t k = 0 ; k < anvec ; k++)
            {
                if (Cp [k] < Cp [k+1])
                { 
                    Ch [cnvec] = Ah [k] ;
                    Cp [cnvec] = Cp [k] ;
                    cnvec++ ;
                }
            }
            Cp [cnvec] = Cp [anvec] ;
            C->nvec = cnvec ;
            ASSERT (C->nvec == C_nvec_nonempty) ;
        }

        C->p = Cp ; Cp = NULL ; C->p_size = Cp_size ;
        C->h = Ch ; Ch = NULL ; C->h_size = Ch_size ;
        C->i = Ci ; Ci = NULL ; C->i_size = Ci_size ;
        C->x = Cx ; Cx = NULL ; C->x_size = Cx_size ;
        C->plen = anvec ;
        C->magic = GB_MAGIC ;
        C->nvec_nonempty = C_nvec_nonempty ;
        C->jumbled = A_jumbled ;    // C is jumbled if A is jumbled
        C->iso = C_iso ;            // OK: burble already done above

        ASSERT_MATRIX_OK (C, "C output for GB_selector", GB0) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORKSPACE ;
    return (GrB_SUCCESS) ;
}

