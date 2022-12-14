//------------------------------------------------------------------------------
// GB_emult_02: C = A.*B where A is sparse/hyper and B is bitmap/full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = A.*B where A is sparse/hyper and B is bitmap/full constructs C with
// the same sparsity structure as A.  This method can also be called with
// the two input matrices swapped, with flipxy true, to handle the case
// where A is bitmap/full and B is sparse/hyper.

// When no mask is present, or the mask is applied later, this method handles
// the following cases:

        //      ------------------------------------------
        //      C       =           A       .*      B
        //      ------------------------------------------
        //      sparse  .           sparse          bitmap
        //      sparse  .           sparse          full  
        //      sparse  .           bitmap          sparse
        //      sparse  .           full            sparse

// If M is sparse/hyper and complemented, it is not passed here:

        //      ------------------------------------------
        //      C       <!M>=       A       .*      B
        //      ------------------------------------------
        //      sparse  sparse      sparse          bitmap  (mask later)
        //      sparse  sparse      sparse          full    (mask later)
        //      sparse  sparse      bitmap          sparse  (mask later)
        //      sparse  sparse      full            sparse  (mask later)

// If M is present, it is bitmap/full:

        //      ------------------------------------------
        //      C      <M> =        A       .*      B
        //      ------------------------------------------
        //      sparse  bitmap      sparse          bitmap
        //      sparse  bitmap      sparse          full  
        //      sparse  bitmap      bitmap          sparse
        //      sparse  bitmap      full            sparse

        //      ------------------------------------------
        //      C      <M> =        A       .*      B
        //      ------------------------------------------
        //      sparse  full        sparse          bitmap
        //      sparse  full        sparse          full  
        //      sparse  full        bitmap          sparse
        //      sparse  full        full            sparse

        //      ------------------------------------------
        //      C      <!M> =        A       .*      B
        //      ------------------------------------------
        //      sparse  bitmap      sparse          bitmap
        //      sparse  bitmap      sparse          full  
        //      sparse  bitmap      bitmap          sparse
        //      sparse  bitmap      full            sparse

        //      ------------------------------------------
        //      C      <!M> =        A       .*      B
        //      ------------------------------------------
        //      sparse  full        sparse          bitmap
        //      sparse  full        sparse          full  
        //      sparse  full        bitmap          sparse
        //      sparse  full        full            sparse

#include "GB_ewise.h"
#include "GB_emult.h"
#include "GB_binop.h"
#include "GB_unused.h"
#include "GB_stringify.h"
#ifndef GBCUDA_DEV
#include "GB_binop__include.h"
#endif

#define GB_FREE_WORKSPACE                   \
{                                           \
    GB_WERK_POP (Work, int64_t) ;           \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
}

#define GB_FREE_ALL                         \
{                                           \
    GB_FREE_WORKSPACE ;                     \
    GB_phybix_free (C) ;                    \
}

GrB_Info GB_emult_02        // C=A.*B when A is sparse/hyper, B bitmap/full
(
    GrB_Matrix C,           // output matrix, static header
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix M,     // optional mask, unused if NULL
    const bool Mask_struct, // if true, use the only structure of M
    const bool Mask_comp,   // if true, use !M
    const GrB_Matrix A,     // input A matrix (sparse/hyper)
    const GrB_Matrix B,     // input B matrix (bitmap/full)
    GrB_BinaryOp op,        // op to perform C = op (A,B)
    bool flipxy,            // if true use fmult(y,x) else fmult(x,y)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (C != NULL && (C->static_header || GBNSTATIC)) ;

    ASSERT_MATRIX_OK_OR_NULL (M, "M for emult_02", GB0) ;
    ASSERT_MATRIX_OK (A, "A for emult_02", GB0) ;
    ASSERT_MATRIX_OK (B, "B for emult_02", GB0) ;
    ASSERT_BINARYOP_OK (op, "op for emult_02", GB0) ;
    ASSERT_TYPE_OK (ctype, "ctype for emult_02", GB0) ;

    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_IS_BITMAP (B) || GB_IS_FULL (B)) ;
    ASSERT (M == NULL || GB_IS_BITMAP (B) || GB_IS_FULL (B)) ;

    int C_sparsity = GB_sparsity (A) ;

    if (M == NULL)
    { 
        GBURBLE ("emult_02:(%s=%s.*%s)",
            GB_sparsity_char (C_sparsity),
            GB_sparsity_char_matrix (A),
            GB_sparsity_char_matrix (B)) ;
    }
    else
    { 
        GBURBLE ("emult_02:(%s<%s%s%s>=%s.*%s) ",
            GB_sparsity_char (C_sparsity),
            Mask_comp ? "!" : "",
            GB_sparsity_char_matrix (M),
            Mask_struct ? ",struct" : "",
            GB_sparsity_char_matrix (A),
            GB_sparsity_char_matrix (B)) ;
    }

    //--------------------------------------------------------------------------
    // revise the operator to handle flipxy
    //--------------------------------------------------------------------------

    // Replace the ANY operator with SECOND.  ANY and SECOND give the same
    // result if flipxy is false.  However, SECOND is changed to FIRST if
    // flipxy is true.  This ensures that the results do not depend on the
    // sparsity structures of A and B.

    if (op->opcode == GB_ANY_binop_code)
    {
        switch (op->xtype->code)
        {
            case GB_BOOL_code   : op = GrB_SECOND_BOOL   ; break ;
            case GB_INT8_code   : op = GrB_SECOND_INT8   ; break ;
            case GB_INT16_code  : op = GrB_SECOND_INT16  ; break ;
            case GB_INT32_code  : op = GrB_SECOND_INT32  ; break ;
            case GB_INT64_code  : op = GrB_SECOND_INT64  ; break ;
            case GB_UINT8_code  : op = GrB_SECOND_UINT8  ; break ;
            case GB_UINT16_code : op = GrB_SECOND_UINT16 ; break ;
            case GB_UINT32_code : op = GrB_SECOND_UINT32 ; break ;
            case GB_UINT64_code : op = GrB_SECOND_UINT64 ; break ;
            case GB_FP32_code   : op = GrB_SECOND_FP32   ; break ;
            case GB_FP64_code   : op = GrB_SECOND_FP64   ; break ;
            case GB_FC32_code   : op = GxB_SECOND_FC32   ; break ;
            case GB_FC64_code   : op = GxB_SECOND_FC64   ; break ;
            default: ;
        }
    }

    // handle the flipxy
    op = GB_flip_binop (op, true, &flipxy) ;
    ASSERT_BINARYOP_OK (op, "final op for emult_02", GB0) ;

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

    GB_WERK_DECLARE (Work, int64_t) ;
    int64_t *restrict Wfirst    = NULL ;
    int64_t *restrict Wlast     = NULL ;
    int64_t *restrict Cp_kfirst = NULL ;
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;

    //--------------------------------------------------------------------------
    // get M, A, and B
    //--------------------------------------------------------------------------

    const int8_t  *restrict Mb = (M == NULL) ? NULL : M->b ;
    const GB_void *restrict Mx = (M == NULL || Mask_struct) ? NULL :
        (const GB_void *) M->x ;
    const size_t msize = (M == NULL) ? 0 : M->type->size ;

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ai = A->i ;
    const int64_t vlen = A->vlen ;
    const int64_t vdim = A->vdim ;
    const int64_t nvec = A->nvec ;
    const int64_t anz = GB_nnz (A) ;

    const int8_t *restrict Bb = B->b ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;

    //--------------------------------------------------------------------------
    // check if C is iso and compute its iso value if it is
    //--------------------------------------------------------------------------

    const size_t csize = ctype->size ;
    GB_void cscalar [GB_VLA(csize)] ;
    bool C_iso = GB_iso_emult (cscalar, ctype, A, B, op) ;

    #ifdef GB_DEBUGIFY_DEFN
    GB_debugify_ewise (C_iso, C_sparsity, ctype, M,
        Mask_struct, Mask_comp, op, flipxy, A, B) ;
    #endif

    //--------------------------------------------------------------------------
    // allocate C->p and C->h
    //--------------------------------------------------------------------------

    GB_OK (GB_new (&C, // sparse or hyper (same as A), existing header
        ctype, vlen, vdim, GB_Ap_calloc, C_is_csc,
        C_sparsity, A->hyper_switch, nvec, Context)) ;
    int64_t *restrict Cp = C->p ;

    //--------------------------------------------------------------------------
    // slice the input matrix A
    //--------------------------------------------------------------------------

    int A_nthreads, A_ntasks ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    GB_SLICE_MATRIX (A, 8, chunk) ;

    //--------------------------------------------------------------------------
    // count entries in C
    //--------------------------------------------------------------------------

    C->nvec_nonempty = A->nvec_nonempty ;
    C->nvec = nvec ;
    const bool C_has_pattern_of_A = !B_is_bitmap && (M == NULL) ;

    if (!C_has_pattern_of_A)
    {

        //----------------------------------------------------------------------
        // allocate workspace
        //----------------------------------------------------------------------

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

        //----------------------------------------------------------------------
        // count entries in C
        //----------------------------------------------------------------------

        // This phase is very similar to GB_select_phase1 (GB_ENTRY_SELECTOR).

        if (M == NULL)
        {

            //------------------------------------------------------------------
            // Method2(a): C = A.*B where A is sparse/hyper and B is bitmap
            //------------------------------------------------------------------

            ASSERT (B_is_bitmap) ;

            int tid ;
            #pragma omp parallel for num_threads(A_nthreads) schedule(dynamic,1)
            for (tid = 0 ; tid < A_ntasks ; tid++)
            {
                int64_t kfirst = kfirst_Aslice [tid] ;
                int64_t klast  = klast_Aslice  [tid] ;
                Wfirst [tid] = 0 ;
                Wlast  [tid] = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // count the entries in C(:,j)
                    int64_t j = GBH (Ah, k) ;
                    int64_t pB_start = j * vlen ;
                    int64_t pA, pA_end ;
                    GB_get_pA (&pA, &pA_end, tid, k,
                        kfirst, klast, pstart_Aslice, Ap, vlen) ;
                    int64_t cjnz = 0 ;
                    for ( ; pA < pA_end ; pA++)
                    { 
                        cjnz += Bb [pB_start + Ai [pA]] ;
                    }
                    if (k == kfirst)
                    { 
                        Wfirst [tid] = cjnz ;
                    }
                    else if (k == klast)
                    { 
                        Wlast [tid] = cjnz ;
                    }
                    else
                    { 
                        Cp [k] = cjnz ; 
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Method2(c): C<#M> = A.*B; M, B bitmap/full, A is sparse/hyper
            //------------------------------------------------------------------

            ASSERT (M != NULL) ;

            int tid ;
            #pragma omp parallel for num_threads(A_nthreads) schedule(dynamic,1)
            for (tid = 0 ; tid < A_ntasks ; tid++)
            {
                int64_t kfirst = kfirst_Aslice [tid] ;
                int64_t klast  = klast_Aslice  [tid] ;
                Wfirst [tid] = 0 ;
                Wlast  [tid] = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // count the entries in C(:,j)
                    int64_t j = GBH (Ah, k) ;
                    int64_t pB_start = j * vlen ;
                    int64_t pA, pA_end ;
                    GB_get_pA (&pA, &pA_end, tid, k,
                        kfirst, klast, pstart_Aslice, Ap, vlen) ;
                    int64_t cjnz = 0 ;
                    for ( ; pA < pA_end ; pA++)
                    { 
                        int64_t i = Ai [pA] ;
                        int64_t pB = pB_start + i ;
                        bool mij = GBB (Mb, pB) && GB_mcast (Mx, pB, msize) ;
                        mij = mij ^ Mask_comp ;
                        cjnz += (mij && GBB (Bb, pB)) ;
                    }
                    if (k == kfirst)
                    { 
                        Wfirst [tid] = cjnz ;
                    }
                    else if (k == klast)
                    { 
                        Wlast [tid] = cjnz ;
                    }
                    else
                    { 
                        Cp [k] = cjnz ; 
                    }
                }
            }
        }

        //----------------------------------------------------------------------
        // finalize Cp, cumulative sum of Cp and compute Cp_kfirst
        //----------------------------------------------------------------------

        GB_ek_slice_merge1 (Cp, Wfirst, Wlast, A_ek_slicing, A_ntasks) ;
        GB_ek_slice_merge2 (&(C->nvec_nonempty), Cp_kfirst, Cp, nvec,
            Wfirst, Wlast, A_ek_slicing, A_ntasks, A_nthreads, Context) ;
    }

    //--------------------------------------------------------------------------
    // allocate C->i and C->x
    //--------------------------------------------------------------------------

    int64_t cnz = (C_has_pattern_of_A) ? anz : Cp [nvec] ;
    // set C->iso = C_iso   OK
    GB_OK (GB_bix_alloc (C, cnz, GxB_SPARSE, false, true, C_iso, Context)) ;

    //--------------------------------------------------------------------------
    // copy pattern into C
    //--------------------------------------------------------------------------

    // TODO: could make these components of C shallow instead of memcpy

    if (GB_IS_HYPERSPARSE (A))
    { 
        // copy A->h into C->h
        GB_memcpy (C->h, Ah, nvec * sizeof (int64_t), A_nthreads) ;
    }

    if (C_has_pattern_of_A)
    { 
        // Method2(b): B is full and no mask present, so the pattern of C is
        // the same as the pattern of A
        GB_memcpy (Cp, Ap, (nvec+1) * sizeof (int64_t), A_nthreads) ;
        GB_memcpy (C->i, Ai, cnz * sizeof (int64_t), A_nthreads) ;
    }

    C->nvals = cnz ;
    C->jumbled = A->jumbled ;
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // get the opcode
    //--------------------------------------------------------------------------

    // if flipxy was true on input and the op is positional, FIRST, SECOND, or
    // PAIR, the op has already been flipped, so these tests do not have to
    // consider that case.

    GB_Opcode opcode = op->opcode ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    bool op_is_first  = (opcode == GB_FIRST_binop_code) ;
    bool op_is_second = (opcode == GB_SECOND_binop_code) ;
    bool op_is_pair   = (opcode == GB_PAIR_binop_code) ;
    GB_Type_code ccode = ctype->code ;

    //--------------------------------------------------------------------------
    // check if the values of A and/or B are ignored
    //--------------------------------------------------------------------------

    // With C = ewisemult (A,B), only the intersection of A and B is used.
    // If op is SECOND or PAIR, the values of A are never accessed.
    // If op is FIRST  or PAIR, the values of B are never accessed.
    // If op is PAIR, the values of A and B are never accessed.
    // Contrast with ewiseadd.

    // A is passed as x, and B as y, in z = op(x,y)
    bool A_is_pattern = op_is_second || op_is_pair || op_is_positional ;
    bool B_is_pattern = op_is_first  || op_is_pair || op_is_positional ;

    //--------------------------------------------------------------------------
    // using a built-in binary operator (except for positional operators)
    //--------------------------------------------------------------------------

    #define GB_PHASE_2_OF_2

    bool done = false ;

    if (C_iso)
    { 

        //----------------------------------------------------------------------
        // C is iso
        //----------------------------------------------------------------------

        // Cx [0] = cscalar = op (A,B)
        GB_BURBLE_MATRIX (C, "(iso emult) ") ;
        memcpy (C->x, cscalar, csize) ;

        // pattern of C = set intersection of pattern of A and B
        // flipxy is ignored since the operator is not applied
        #define GB_ISO_EMULT
        #include "GB_emult_02_template.c"
        done = true ;

    }
    else
    {

        #ifndef GBCUDA_DEV

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_AemultB_02(mult,xname) GB (_AemultB_02_ ## mult ## xname)

            #define GB_BINOP_WORKER(mult,xname)                         \
            {                                                           \
                info = GB_AemultB_02(mult,xname) (C,                    \
                    M, Mask_struct, Mask_comp, A, B, flipxy,            \
                    Cp_kfirst, A_ek_slicing, A_ntasks, A_nthreads) ;    \
                done = (info != GrB_NO_VALUE) ;                         \
            }                                                           \
            break ;

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            // flipxy is not passed to GB_binop_builtin, since the unflippable
            // binary ops (atan2, pow, etc) handle the flip themselves.
            // See for example Generated2/GB_binop__atan2_fp32.c.

            GB_Type_code xcode, ycode, zcode ;
            if (!op_is_positional &&
                GB_binop_builtin (A->type, A_is_pattern, B->type, B_is_pattern,
                op, false, &opcode, &xcode, &ycode, &zcode) && ccode == zcode)
            { 
                #define GB_NO_PAIR
                #include "GB_binop_factory.c"
            }

        #endif
    }

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    if (!done)
    { 
        GB_BURBLE_MATRIX (C, "(generic emult_02: %s) ", op->name) ;
        int ewise_method = flipxy ? GB_EMULT_METHOD3 : GB_EMULT_METHOD2 ;
        GB_ewise_generic (C, op, NULL, 0, 0,
            NULL, NULL, NULL, C_sparsity, ewise_method, Cp_kfirst,
            NULL, 0, 0, A_ek_slicing, A_ntasks, A_nthreads, NULL, 0, 0,
            M, Mask_struct, Mask_comp, A, B, Context) ;
    }

    //--------------------------------------------------------------------------
    // remove empty vectors from C, if hypersparse
    //--------------------------------------------------------------------------

    GB_OK (GB_hypermatrix_prune (C, Context)) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORKSPACE ;
    ASSERT_MATRIX_OK (C, "C output for emult_02", GB0) ;
    return (GrB_SUCCESS) ;
}

