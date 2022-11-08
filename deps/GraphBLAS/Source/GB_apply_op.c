//------------------------------------------------------------------------------
// GB_apply_op: typecast and apply a unary/binary/idxunop operator to an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Cx = op (A)

// Cx and A->x may be aliased.

// This function is CSR/CSC agnostic.  For positional ops, A is treated as if
// it is in CSC format.  The caller has already modified the op if A is in CSR
// format.

// Template/GB_positional_op_ijp can return GrB_OUT_OF_MEMORY.
// Otherwise, this function only returns GrB_SUCCESS.

#include "GB_apply.h"
#include "GB_binop.h"
#include "GB_ek_slice.h"
#include "GB_unused.h"
#ifndef GBCUDA_DEV
#include "GB_unop__include.h"
#include "GB_binop__include.h"
#endif

#define GB_FREE_ALL                         \
{                                           \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
}

GrB_Info GB_apply_op        // apply a unary op, idxunop, or binop, Cx = op (A)
(
    GB_void *Cx,                    // output array
    const GrB_Type ctype,           // type of C
    const GB_iso_code C_code_iso,   // C non-iso, or code to compute C iso value
        const GB_Operator op,       // unary/index-unary/binop to apply
        const GrB_Scalar scalar,    // scalar to bind to binary operator
        bool binop_bind1st,         // if true, C=binop(s,A), else C=binop(A,s)
        bool flipij,                // if true, flip i,j for user idxunop
    const GrB_Matrix A,             // input matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cx != NULL) ;
    ASSERT_MATRIX_OK (A, "A input for GB_apply_op", GB0) ;
    ASSERT (GB_JUMBLED_OK (A)) ;        // A can be jumbled
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;
    ASSERT (GB_IMPLIES (op != NULL, ctype == op->ztype)) ;
    ASSERT_SCALAR_OK_OR_NULL (scalar, "scalar for GB_apply_op", GB0) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    // A->x is not const since the operator might be applied in-place, if
    // C is aliased to C.

    GB_void *Ax = (GB_void *) A->x ;        // A->x has type A->type
    const int8_t *Ab = A->b ;               // only if A is bitmap
    const GrB_Type Atype = A->type ;        // type of A->x
    const int64_t anz = GB_nnz_held (A) ;   // size of A->x and Cx

    //--------------------------------------------------------------------------
    // determine the maximum number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get the operator
    //--------------------------------------------------------------------------

    GB_Opcode opcode ;
    bool op_is_unop = false ;
    bool op_is_binop = false ;
    if (op != NULL)
    { 
        opcode = op->opcode ;
        op_is_unop = GB_IS_UNARYOP_CODE (opcode) ;
        op_is_binop = GB_IS_BINARYOP_CODE (opcode) ;
    }
    else
    { 
        // C is iso, with no operator to apply; just call GB_iso_unop below.
        ASSERT (C_code_iso == GB_ISO_1 ||   // C iso value is 1
                C_code_iso == GB_ISO_S ||   // C iso value is the scalar
                C_code_iso == GB_ISO_A) ;   // C iso value is the iso value of A
        opcode = GB_NOP_code ;
    }

    //--------------------------------------------------------------------------
    // apply the operator
    //--------------------------------------------------------------------------

    if (GB_OPCODE_IS_POSITIONAL (opcode))
    {

        //----------------------------------------------------------------------
        // built-in positional unary, index_unary, or binary operator
        //----------------------------------------------------------------------

        bool is64 = (op->ztype == GrB_INT64) ;
        bool is32 = (op->ztype == GrB_INT32) ;
        ASSERT_OP_OK (op, "positional unop/idxunop/binop: GB_apply_op", GB0) ;

        // get A and C
        const int64_t *restrict Ah = A->h ;
        const int64_t *restrict Ap = A->p ;
        const int64_t *restrict Ai = A->i ;
        int64_t anvec = A->nvec ;
        int64_t avlen = A->vlen ;
        int64_t avdim = A->vdim ;

        //----------------------------------------------------------------------
        // determine number of threads to use
        //----------------------------------------------------------------------

        int nthreads = GB_nthreads (anz + anvec, chunk, nthreads_max) ;
        int ntasks = (nthreads == 1) ? 1 : (32 * nthreads) ;

        //----------------------------------------------------------------------
        // Cx = positional_op (A)
        //----------------------------------------------------------------------

        int64_t thunk = GB_positional_offset (opcode, scalar) ;

        // GB_positional_op_ijp allocates a set of tasks, which can possibly
        // fail if out of memory.

        if (is64)
        { 

            //------------------------------------------------------------------
            // int64 Cx = positional_op (A)
            //------------------------------------------------------------------

            int64_t *restrict Cz = (int64_t *) Cx ;
            switch (opcode)
            {

                case GB_POSITIONI_unop_code  : // z = position_i(A(i,j)) == i
                case GB_POSITIONI1_unop_code : // z = position_i1(A(i,j)) == i+1
                case GB_FIRSTI_binop_code    : // z = first_i(A(i,j),y) == i
                case GB_FIRSTI1_binop_code   : // z = first_i1(A(i,j),y) == i+1
                case GB_SECONDI_binop_code   : // z = second_i(x,A(i,j)) == i
                case GB_SECONDI1_binop_code  : // z = second_i1(x,A(i,j)) == i+1
                case GB_ROWINDEX_idxunop_code : // z = i+thunk
                    #define GB_APPLY(p)                     \
                        Cz [p] = (i + thunk) ;
                    #include "GB_positional_op_ip.c"
                    return (GrB_SUCCESS) ;

                case GB_POSITIONJ_unop_code  : // z = position_j(A(i,j)) == j
                case GB_POSITIONJ1_unop_code : // z = position_j1(A(i,j)) == j+1
                case GB_FIRSTJ_binop_code    : // z = first_j(A(i,j),y) == j
                case GB_FIRSTJ1_binop_code   : // z = first_j1(A(i,j),y) == j+1
                case GB_SECONDJ_binop_code   : // z = second_j(x,A(i,j)) == j
                case GB_SECONDJ1_binop_code  : // z = second_j1(x,A(i,j)) == j+1
                case GB_COLINDEX_idxunop_code : // z = j+thunk
                    #define GB_APPLY(p)                     \
                        Cz [p] = (j + thunk) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_DIAGINDEX_idxunop_code : // z = (j-(i+thunk)
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (j - (i+thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_FLIPDIAGINDEX_idxunop_code : // z = (i-(j+thunk)
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (i - (j+thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                default: ;
            }

        }
        else if (is32)
        { 

            //------------------------------------------------------------------
            // int32 Cx = positional_op (A)
            //------------------------------------------------------------------

            int32_t *restrict Cz = (int32_t *) Cx ;
            switch (opcode)
            {

                case GB_POSITIONI_unop_code  : // z = position_i(A(i,j)) == i
                case GB_POSITIONI1_unop_code : // z = position_i1(A(i,j)) == i+1
                case GB_FIRSTI_binop_code    : // z = first_i(A(i,j),y) == i
                case GB_FIRSTI1_binop_code   : // z = first_i1(A(i,j),y) == i+1
                case GB_SECONDI_binop_code   : // z = second_i(x,A(i,j)) == i
                case GB_SECONDI1_binop_code  : // z = second_i1(x,A(i,j)) == i+1
                case GB_ROWINDEX_idxunop_code : // z = i+thunk
                    #define GB_APPLY(p)                     \
                        Cz [p] = (int32_t) (i + thunk) ;
                    #include "GB_positional_op_ip.c"
                    return (GrB_SUCCESS) ;

                case GB_POSITIONJ_unop_code  : // z = position_j(A(i,j)) == j
                case GB_POSITIONJ1_unop_code : // z = position_j1(A(i,j)) == j+1
                case GB_FIRSTJ_binop_code    : // z = first_j(A(i,j),y) == j
                case GB_FIRSTJ1_binop_code   : // z = first_j1(A(i,j),y) == j+1
                case GB_SECONDJ_binop_code   : // z = second_j(x,A(i,j)) == j
                case GB_SECONDJ1_binop_code  : // z = second_j1(x,A(i,j)) == j+1
                case GB_COLINDEX_idxunop_code : // z = j+thunk
                    #define GB_APPLY(p)                     \
                        Cz [p] = (int32_t) (j + thunk) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_DIAGINDEX_idxunop_code : // z = (j-(i+thunk)
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (int32_t) (j - (i+thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_FLIPDIAGINDEX_idxunop_code : // z = (i-(j+thunk)
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (int32_t) (i - (j+thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                default: ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // bool Cx = positional_op (A)
            //------------------------------------------------------------------

            ASSERT (op->ztype == GrB_BOOL) ;
            bool *restrict Cz = (bool *) Cx ;
            switch (opcode)
            {

                case GB_TRIL_idxunop_code : // z = (j <= (i+thunk))
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (j <= (i + thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_TRIU_idxunop_code : // z = (j >= (i+thunk))
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (j >= (i + thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_DIAG_idxunop_code : // z = (j == (i+thunk))
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (j == (i + thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_OFFDIAG_idxunop_code : // z = (j != (i+thunk))
                    #define GB_APPLY(p)                     \
                        int64_t i = GBI (Ai, p, avlen) ;    \
                        Cz [p] = (j != (i + thunk)) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_COLLE_idxunop_code : // z = (j <= thunk)
                    #define GB_APPLY(p)                     \
                        Cz [p] = (j <= thunk) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_COLGT_idxunop_code : // z = (j > thunk)
                    #define GB_APPLY(p)                     \
                        Cz [p] = (j > thunk) ;
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;

                case GB_ROWLE_idxunop_code : // z = (i <= thunk)
                    #define GB_APPLY(p)                     \
                        Cz [p] = (i <= thunk) ;
                    #include "GB_positional_op_ip.c"
                    return (GrB_SUCCESS) ;

                case GB_ROWGT_idxunop_code : // z = (i > thunk)
                    #define GB_APPLY(p)                     \
                        Cz [p] = (i > thunk) ;
                    #include "GB_positional_op_ip.c"
                    return (GrB_SUCCESS) ;

                default: ;
            }
        }

    }
    else if (C_code_iso != GB_NON_ISO)
    {

        //----------------------------------------------------------------------
        // apply the unary or binary operator to the iso value
        //----------------------------------------------------------------------

        // if C is iso, this function takes O(1) time
        GBURBLE ("(iso apply) ") ;
        ASSERT_MATRIX_OK (A, "A passing to GB_iso_unop", GB0) ;
        if (anz > 0)
        { 
            // Cx [0] = unop (A), binop (scalar,A), or binop (A,scalar)
            GB_iso_unop (Cx, ctype, C_code_iso, op, A, scalar) ;
        }

    }
    else if (op_is_unop)
    {

        //----------------------------------------------------------------------
        // apply the unary operator to all entries
        //----------------------------------------------------------------------

        ASSERT_UNARYOP_OK (op, "unop for GB_apply_op", GB0) ;
        ASSERT (!A->iso) ;

        // determine number of threads to use
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        #ifndef GBCUDA_DEV
        if (Atype == op->xtype || opcode == GB_IDENTITY_unop_code)
        { 

            // The switch factory is used if the op is IDENTITY, or if no
            // typecasting is being done.  IDENTITY operator can do arbitrary
            // typecasting (it is not used if no typecasting is done).

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_unop_apply(unop,zname,aname) \
                GB (_unop_apply_ ## unop ## zname ## aname)

            #define GB_WORKER(unop,zname,ztype,aname,atype)             \
            {                                                           \
                if (GB_unop_apply (unop,zname,aname) ((ztype *) Cx,     \
                    (const atype *) Ax, Ab, anz, nthreads)              \
                    == GrB_SUCCESS) return (GrB_SUCCESS) ;              \
            }                                                           \
            break ;

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            #include "GB_unop_factory.c"
        }
        #endif

        //----------------------------------------------------------------------
        // generic worker: typecast and apply a unary operator
        //----------------------------------------------------------------------

        GB_BURBLE_N (anz, "(generic apply: %s) ", op->name) ;

        size_t asize = Atype->size ;
        size_t zsize = op->ztype->size ;
        size_t xsize = op->xtype->size ;
        GB_Type_code acode = Atype->code ;
        GB_Type_code xcode = op->xtype->code ;
        GB_cast_function cast_A_to_X = GB_cast_factory (xcode, acode) ;
        GxB_unary_function fop = op->unop_function ;

        int64_t p ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        { 
            if (!GBB (Ab, p)) continue ;
            // xwork = (xtype) Ax [p]
            GB_void xwork [GB_VLA(xsize)] ;
            cast_A_to_X (xwork, Ax +(p)*asize, asize) ;
            // Cx [p] = fop (xwork)
            fop (Cx +(p*zsize), xwork) ;
        }

    }
    else if (op_is_binop)
    {

        //----------------------------------------------------------------------
        // apply a binary operator (bound to a scalar)
        //----------------------------------------------------------------------

        ASSERT_BINARYOP_OK (op, "standard binop for GB_apply_op", GB0) ;
        ASSERT_SCALAR_OK (scalar, "scalar for GB_apply_op", GB0) ;

        GB_Type_code xcode, ycode, zcode ;
        ASSERT (opcode != GB_FIRST_binop_code) ;
        ASSERT (opcode != GB_SECOND_binop_code) ;
        ASSERT (opcode != GB_PAIR_binop_code) ;
        ASSERT (opcode != GB_ANY_binop_code) ;

        size_t asize = Atype->size ;
        size_t ssize = scalar->type->size ;
        size_t zsize = op->ztype->size ;
        size_t xsize = op->xtype->size ;
        size_t ysize = op->ytype->size ;

        GB_Type_code scode = scalar->type->code ;
        xcode = op->xtype->code ;
        ycode = op->ytype->code ;

        // typecast the scalar to the operator input
        size_t ssize_cast ;
        GB_Type_code scode_cast ;
        if (binop_bind1st)
        { 
            ssize_cast = xsize ;
            scode_cast = xcode ;
        }
        else
        { 
            ssize_cast = ysize ;
            scode_cast = ycode ;
        }
        GB_void swork [GB_VLA(ssize_cast)] ;
        GB_void *scalarx = (GB_void *) scalar->x ;
        if (scode_cast != scode)
        { 
            // typecast the scalar to the operator input, in swork
            GB_cast_function cast_s = GB_cast_factory (scode_cast, scode) ;
            cast_s (swork, scalar->x, ssize) ;
            scalarx = swork ;
        }

        // determine number of threads to use
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

        #ifndef GBCUDA_DEV
        if (binop_bind1st)
        {

            //------------------------------------------------------------------
            // z = binop (scalar,Ax)
            //------------------------------------------------------------------

            if (GB_binop_builtin (op->xtype, false, Atype, false,
                (GrB_BinaryOp) op, false, &opcode, &xcode, &ycode, &zcode))
            { 

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_bind1st(binop,xname) GB (_bind1st_ ## binop ## xname)
                #define GB_BINOP_WORKER(binop,xname)                    \
                {                                                       \
                    if (GB_bind1st (binop, xname) (Cx, scalarx, Ax,     \
                        Ab, anz, nthreads) == GrB_SUCCESS)              \
                        return (GrB_SUCCESS) ;                          \
                }                                                       \
                break ;

                //--------------------------------------------------------------
                // launch the switch factory
                //--------------------------------------------------------------

                #define GB_NO_FIRST
                #define GB_NO_SECOND
                #define GB_NO_PAIR
                #include "GB_binop_factory.c"
            }
        }
        else
        {

            //------------------------------------------------------------------
            // z = binop (Ax,scalar)
            //------------------------------------------------------------------

            if (GB_binop_builtin (Atype, false, op->ytype, false,
                (GrB_BinaryOp) op, false, &opcode, &xcode, &ycode, &zcode))
            { 

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_bind2nd(binop,xname) GB (_bind2nd_ ## binop ## xname)
                #undef  GB_BINOP_WORKER
                #define GB_BINOP_WORKER(binop,xname)                    \
                {                                                       \
                    if (GB_bind2nd (binop, xname) (Cx, Ax, scalarx,     \
                        Ab, anz, nthreads) == GrB_SUCCESS)              \
                        return (GrB_SUCCESS) ;                          \
                }                                                       \
                break ;

                //--------------------------------------------------------------
                // launch the switch factory
                //--------------------------------------------------------------

                #define GB_NO_FIRST
                #define GB_NO_SECOND
                #define GB_NO_PAIR
                #include "GB_binop_factory.c"
            }
        }
        #endif

        //----------------------------------------------------------------------
        // generic worker: typecast and apply a binary operator
        //----------------------------------------------------------------------

        GB_BURBLE_N (anz, "(generic apply: %s) ", op->name) ;

        GB_Type_code acode = Atype->code ;
        GxB_binary_function fop = op->binop_function ;
        ASSERT (!A->iso) ;

        if (binop_bind1st)
        {
            // Cx = binop (scalar,Ax)
            GB_cast_function cast_A_to_Y = GB_cast_factory (ycode, acode) ;
            int64_t p ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < anz ; p++)
            { 
                if (!GBB (Ab, p)) continue ;
                // ywork = (ytype) Ax [p]
                GB_void ywork [GB_VLA(ysize)] ;
                cast_A_to_Y (ywork, Ax +(p)*asize, asize) ;
                // Cx [p] = fop (scalarx, ywork)
                fop (Cx +((p)*zsize), scalarx, ywork) ;
            }
        }
        else
        {
            // Cx = binop (Ax,scalar)
            GB_cast_function cast_A_to_X = GB_cast_factory (xcode, acode) ;
            int64_t p ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < anz ; p++)
            { 
                if (!GBB (Ab, p)) continue ;
                // xwork = (xtype) Ax [p]
                GB_void xwork [GB_VLA(xsize)] ;
                cast_A_to_X (xwork, Ax +(p)*asize, asize) ;
                // Cx [p] = fop (xwork, scalarx)
                fop (Cx +(p*zsize), xwork, scalarx) ;
            }
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // apply a user-defined index_unary op
        //----------------------------------------------------------------------

        // All valued GrB_IndexUnaryOps (GrB_VALUE*) have already been renamed
        // to their corresponding binary op (GrB_VALUEEQ_FP32 became
        // GrB_EQ_FP32, for example).  The only remaining index unary ops are
        // positional, and user-defined.  Positional ops have been handled
        // above, so only user-defined index unary ops are left.

        // get A and C
        const int64_t *restrict Ah = A->h ;
        const int64_t *restrict Ap = A->p ;
        const int64_t *restrict Ai = A->i ;
        int64_t anvec = A->nvec ;
        int64_t avlen = A->vlen ;
        int64_t avdim = A->vdim ;

        ASSERT (opcode == GB_USER_idxunop_code) ;
        GxB_index_unary_function fop = op->idxunop_function ;

        size_t asize = Atype->size ;
        size_t ssize = scalar->type->size ;
        size_t zsize = op->ztype->size ;
        size_t xsize = op->xtype->size ;
        size_t ysize = op->ytype->size ;

        GB_Type_code scode = scalar->type->code ;
        GB_Type_code acode = Atype->code ;
        GB_Type_code xcode = op->xtype->code ;
        GB_Type_code ycode = op->ytype->code ;
        GB_cast_function cast_A_to_X = GB_cast_factory (xcode, acode) ;

        GB_void ywork [GB_VLA(ysize)] ;
        GB_void *ythunk = (GB_void *) scalar->x ;
        if (ycode != scode)
        { 
            // typecast the scalar to the operator input, in ywork
            GB_cast_function cast_s = GB_cast_factory (ycode, scode) ;
            cast_s (ywork, scalar->x, ssize) ;
            ythunk = ywork ;
        }

        #define GB_APPLY(p)                                                 \
            if (!GBB (Ab, p)) continue ;                                    \
            int64_t i = GBI (Ai, p, avlen) ;                                \
            GB_void xwork [GB_VLA(xsize)] ;                                 \
            cast_A_to_X (xwork, Ax +(p)*asize, asize) ;                     \
            fop (Cx +(p*zsize), xwork,                                      \
                flipij ? j : i, flipij ? i : j, ythunk) ;

        #include "GB_positional_op_ijp.c"
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    return (GrB_SUCCESS) ;
}

