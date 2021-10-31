//------------------------------------------------------------------------------
// GB_selector:  select entries from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
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

#define GB_FREE_WORK                        \
{                                           \
    GB_FREE_WERK (&Zp, Zp_size) ;           \
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
    GB_FREE_WORK ;                          \
}

GrB_Info GB_selector
(
    GrB_Matrix C,               // output matrix, NULL or static header
    GB_Select_Opcode opcode,    // selector opcode
    const GxB_SelectOp op,      // user operator
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    int64_t ithunk,             // (int64_t) Thunk, if Thunk is NULL
    const GxB_Scalar Thunk,     // optional input for select operator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_SELECTOP_OK_OR_NULL (op, "selectop for GB_selector", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (Thunk, "Thunk for GB_selector", GB0) ;
    ASSERT (opcode >= 0 && opcode <= GB_USER_SELECT_opcode) ;
    ASSERT_MATRIX_OK (A, "A input for GB_selector", GB_FLIP (GB0)) ;
    // positional selector (tril, triu, diag, offdiag, resize): can't be jumbled
    ASSERT (GB_IMPLIES (opcode <= GB_RESIZE_opcode ||
        opcode == GB_USER_SELECT_opcode, !GB_JUMBLED (A))) ;
    // nonzombie, nentry selector: jumbled OK
    ASSERT (GB_IMPLIES (opcode > GB_RESIZE_opcode && 
        opcode < GB_USER_SELECT_opcode, GB_JUMBLED_OK (A))) ;
    ASSERT (C == NULL || (C != NULL && C->static_header)) ;

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

    // The scalar value of Thunk is typecasted to an integer (int64_t
    // ithunk) for built-in operators (tril, triu, diag, offdiag, and resize).
    // It is also typecast to the same type as A (to the scalar athunk).  This
    // is used for gt, ge, lt, le, ne, eq to Thunk, for built-in types.

    // If Thunk is NULL, or has no entry, it is treated as a scalar value
    // of zero.

    const int64_t asize = A->type->size ;
    const GB_Type_code acode = A->type->code ;

    GB_void athunk [GB_VLA(asize)] ;
    memset (athunk, 0, asize) ;
    GB_void *restrict xthunk = athunk ;

    if (Thunk != NULL && GB_nnz ((GrB_Matrix) Thunk) > 0)
    {
        // xthunk points to Thunk->x for user-defined select operators
        xthunk = (GB_void *) Thunk->x ;
        const GB_Type_code tcode = Thunk->type->code ;
        ithunk = 0 ;
        if (tcode <= GB_FP64_code && opcode < GB_USER_SELECT_opcode)
        { 
            // ithunk = (int64_t) Thunk
            GB_cast_scalar (&ithunk, GB_INT64_code, xthunk, tcode,
                sizeof (int64_t)) ;
            // athunk = (atype) Thunk
            GB_cast_scalar (athunk, A->type->code, xthunk, tcode, asize) ;
            // xthunk now points to the typecasted (atype) Thunk
            xthunk = athunk ;
        }
    }

    //--------------------------------------------------------------------------
    // handle iso case for built-in select ops that depend only on the value
    //--------------------------------------------------------------------------

    if (A_iso && opcode >= GB_NONZERO_opcode && opcode <= GB_LE_THUNK_opcode)
    { 

        // select op is NONZERO, EQ_ZERO, GT_ZERO, GE_ZERO, LT_ZERO, LE_ZERO,
        // EQ_THUNK, GT_THUNK, GE_THUNK, LT_THUNK, or LE_THUNK.  All of these
        // select ops depend only on the value of A(i,j).  Since A is iso,
        // either all entries in A will be copied to C and thus C can be
        // created as a shallow copy of A, or no entries from A will be copied
        // to C and thus C is an empty matrix.  The select factory is not
        // needed, except to check the iso value via GB_bitmap_selector.

        ASSERT (!in_place_A) ;
        ASSERT (C != NULL && C->static_header) ;

        // construct a scalar containing the iso scalar
        struct GB_Scalar_opaque S_header ;
        GxB_Scalar S = GB_Scalar_wrap (&S_header, A->type, A->x) ;
        S->iso = false ;    // but ensure S is not iso
        ASSERT_SCALAR_OK (S, "iso scalar wrap", GB0) ;

        // apply the select operator to the iso scalar
        GB_OK (GB_bitmap_selector (C, false, opcode, NULL, false,
            (GrB_Matrix) S, ithunk, xthunk, Context)) ;
        ASSERT_MATRIX_OK (C, "C from iso scalar test", GB0) ;
        bool C_empty = (GB_nnz (C) == 0) ;
        GB_phbix_free (C) ;

        // check if C has 0 or 1 entry
        if (C_empty)
        { 
            // C is an empty matrix
            return (GB_new (&C, true, // static header
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

    //      GB_TRIL_opcode        : use GB_sel__tril_iso
    //      GB_TRIU_opcode        : use GB_sel__triu_iso
    //      GB_DIAG_opcode        : use GB_sel__diag_iso
    //      GB_OFFDIAG_opcode     : use GB_sel__offdiag_iso
    //      GB_RESIZE_opcode      : use GB_sel__resize_iso
    //      GB_NONZOMBIE_opcode   : use GB_sel__nonzombie_iso
    //      GB_USER_SELECT_opcode : use GB_sel__user_iso

    // Except for GB_USER_SELECT_opcode, the GB_sel__*_iso methods do not
    // access the values of A and C, just the pattern.

    //--------------------------------------------------------------------------
    // get the user-defined operator
    //--------------------------------------------------------------------------

    GxB_select_function user_select = NULL ;
    if (op != NULL && opcode >= GB_USER_SELECT_opcode)
    { 
        GB_BURBLE_MATRIX (A, "(user select: %s) ", op->name) ;
        user_select = (GxB_select_function) (op->function) ;
    }

    //--------------------------------------------------------------------------
    // handle the bitmap/as-if-full case
    //--------------------------------------------------------------------------

    bool use_bitmap_selector ;
    if (opcode == GB_RESIZE_opcode || opcode == GB_NONZOMBIE_opcode)
    { 
        // GB_bitmap_selector does not support these opcodes.  For the RESIZE
        // and NONZOMBIE operators, A will never be bitmap.  A is converted to
        // hypersparse first for RESIZE, and a full/bitmap matrix never has
        // zombies.
        use_bitmap_selector = false ;
    }
    else if (opcode == GB_DIAG_opcode)
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
        (opcode == GB_EQ_ZERO_opcode) ||        // C iso value is zero
        (opcode == GB_EQ_THUNK_opcode) ||       // C iso value is thunk
        (opcode == GB_NONZERO_opcode &&
         acode == GB_BOOL_code) ;               // C iso value is true

    if (C_iso)
    { 
        GB_BURBLE_MATRIX (A, "(iso select) ") ;
    }

    //--------------------------------------------------------------------------
    // bitmap/full case
    //--------------------------------------------------------------------------

    if (use_bitmap_selector)
    { 
        GB_BURBLE_MATRIX (A, "(bitmap select) ") ;
        ASSERT (C != NULL && C->static_header) ;
        return (GB_bitmap_selector (C, C_iso, opcode, user_select, flipij, A,
            ithunk, xthunk, Context)) ;
    }

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
    // determine the number of threads and tasks to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    int A_ntasks, A_nthreads ;
    double work = 8*anvec + ((opcode == GB_DIAG_opcode) ? 0 : GB_nnz_held (A)) ;
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
    // count the live entries in each vector
    //--------------------------------------------------------------------------

    // Count the number of live entries in each vector of A.  The result is
    // computed in Cp, where Cp [k] is the number of live entries in the kth
    // vector of A.

    if (opcode <= GB_RESIZE_opcode)
    {
        // allocate Zp
        Zp = GB_MALLOC_WERK (anvec, int64_t, &Zp_size) ;
        if (Zp == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // phase1: count the entries
    //--------------------------------------------------------------------------

    // define the worker for the switch factory
    #define GB_SELECT_PHASE1
    #define GB_sel1(opname,aname) GB (_sel_phase1_ ## opname ## aname)
    #define GB_SEL_WORKER(opname,aname,atype)                               \
    {                                                                       \
        GB_sel1 (opname, aname) (Zp, Cp, Wfirst, Wlast, A, flipij, ithunk,  \
            (atype *) xthunk, user_select, A_ek_slicing, A_ntasks,          \
            A_nthreads) ;                                                   \
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
    Cx = (GB_void *) GB_XALLOC (C_iso, cnz, asize, &Cx_size) ;
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
        // OFFDIAG, TRIL, TRIU, RESIZE, NONZOMBIE, and USER select operators.
        GB_iso_select (Cx, opcode, xthunk, Ax, acode, asize) ;
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
            flipij, ithunk, (atype *) xthunk, user_select, A_ek_slicing,    \
            A_ntasks, A_nthreads) ;                                         \
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

        int sparsity = (A->h != NULL) ? GxB_HYPERSPARSE : GxB_SPARSE ;
        ASSERT (C != NULL && C->static_header) ;
        info = GB_new (&C, true, // sparse or hyper (from A), static header
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

    GB_FREE_WORK ;
    return (GrB_SUCCESS) ;
}

