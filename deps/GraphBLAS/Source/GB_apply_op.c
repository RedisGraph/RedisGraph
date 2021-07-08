//------------------------------------------------------------------------------
// GB_apply_op: typecast and apply a unary or binary operator to an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
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
#ifndef GBCOMPACT
#include "GB_unop__include.h"
#include "GB_binop__include.h"
#endif

#define GB_FREE_ALL                         \
{                                           \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
}

GrB_Info GB_apply_op        // apply a unary or binary operator, Cx = op (A)
(
    GB_void *Cx,                    // output array
    const GrB_Type ctype,           // type of C
    const GB_iso_code C_code_iso,   // C non-iso, or code to compute C iso value
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, C=op2(s,A) else C=op2(A,s)
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
    ASSERT (GB_IMPLIES (op1 != NULL, ctype == op1->ztype)) ;
    ASSERT (GB_IMPLIES (op2 != NULL, ctype == op2->ztype)) ;
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
    if (op1 != NULL)
    { 
        opcode = op1->opcode ;
    }
    else if (op2 != NULL)
    { 
        opcode = op2->opcode ;
    }
    else
    { 
        // C is iso, with no operator to apply; just call GB_iso_unop below.
        ASSERT (C_code_iso == GB_ISO_1 ||   // C iso value is 1
                C_code_iso == GB_ISO_S ||   // C iso value is the scalar
                C_code_iso == GB_ISO_A) ;   // C iso value is the iso value of A
        opcode = GB_NOP_opcode ;
    }

    //--------------------------------------------------------------------------
    // apply the operator
    //--------------------------------------------------------------------------

    if (GB_OPCODE_IS_POSITIONAL (opcode))
    {

        //----------------------------------------------------------------------
        // built-in positional unary or binary operator
        //----------------------------------------------------------------------

        bool is64 ;
        if (op1 != NULL)
        { 
            ASSERT_UNARYOP_OK (op1, "positional op1 for GB_apply_op", GB0) ;
            is64 = (op1->ztype == GrB_INT64) ;
        }
        else // if (op2 != NULL)
        { 
            ASSERT_BINARYOP_OK (op2, "positional op2 for GB_apply_op", GB0) ;
            is64 = (op2->ztype == GrB_INT64) ;
        }

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

        int64_t offset = GB_positional_offset (opcode) ;

        // GB_positional_op_ijp allocates a set of tasks, which can possibly
        // fail if out of memory.

        if (is64)
        { 
            int64_t *restrict Cx_int = (int64_t *) Cx ;
            switch (opcode)
            {
                case GB_POSITIONI_opcode  : // z = position_i(A(i,j)) == i
                case GB_POSITIONI1_opcode : // z = position_i1(A(i,j)) == i+1
                case GB_FIRSTI_opcode     : // z = first_i(A(i,j),y) == i
                case GB_FIRSTI1_opcode    : // z = first_i1(A(i,j),y) == i+1
                case GB_SECONDI_opcode    : // z = second_i(x,A(i,j)) == i
                case GB_SECONDI1_opcode   : // z = second_i1(x,A(i,j)) == i+1
                    #define GB_POSITION i + offset
                    #include "GB_positional_op_ip.c"
                    return (GrB_SUCCESS) ;
                case GB_POSITIONJ_opcode  : // z = position_j(A(i,j)) == j
                case GB_POSITIONJ1_opcode : // z = position_j1(A(i,j)) == j+1
                case GB_FIRSTJ_opcode     : // z = first_j(A(i,j),y) == j
                case GB_FIRSTJ1_opcode    : // z = first_j1(A(i,j),y) == j+1
                case GB_SECONDJ_opcode    : // z = second_j(x,A(i,j)) == j
                case GB_SECONDJ1_opcode   : // z = second_j1(x,A(i,j)) == j+1
                    #define GB_POSITION j + offset
                    #include "GB_positional_op_ijp.c"
                    return (GrB_SUCCESS) ;
                default: ;
            }
        }
        else
        { 
            int32_t *restrict Cx_int = (int32_t *) Cx ;
            switch (opcode)
            {
                case GB_POSITIONI_opcode  : // z = position_i(A(i,j)) == i
                case GB_POSITIONI1_opcode : // z = position_i1(A(i,j)) == i+1
                case GB_FIRSTI_opcode     : // z = first_i(A(i,j),y) == i
                case GB_FIRSTI1_opcode    : // z = first_i1(A(i,j),y) == i+1
                case GB_SECONDI_opcode    : // z = second_i(x,A(i,j)) == i
                case GB_SECONDI1_opcode   : // z = second_i1(x,A(i,j)) == i+1
                    #define GB_POSITION (int32_t) (i + offset)
                    #include "GB_positional_op_ip.c"
                    return (GrB_SUCCESS) ;
                case GB_POSITIONJ_opcode  : // z = position_j(A(i,j)) == j
                case GB_POSITIONJ1_opcode : // z = position_j1(A(i,j)) == j+1
                case GB_FIRSTJ_opcode     : // z = first_j(A(i,j),y) == j
                case GB_FIRSTJ1_opcode    : // z = first_j1(A(i,j),y) == j+1
                case GB_SECONDJ_opcode    : // z = second_j(x,A(i,j)) == j
                case GB_SECONDJ1_opcode   : // z = second_j1(x,A(i,j)) == j+1
                    #define GB_POSITION (int32_t) (j + offset)
                    #include "GB_positional_op_ijp.c"
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
            // Cx [0] = op1 (A), op2 (scalar,A), or op2 (A,scalar)
            GB_iso_unop (Cx, ctype, C_code_iso, op1, op2, A, scalar) ;
        }

    }
    else if (op1 != NULL)
    {

        //----------------------------------------------------------------------
        // apply the unary operator to all entries
        //----------------------------------------------------------------------

        ASSERT_UNARYOP_OK (op1, "op1 for GB_apply_op", GB0) ;
        ASSERT (!A->iso) ;

        // determine number of threads to use
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        #ifndef GBCOMPACT
        if (Atype == op1->xtype || opcode == GB_IDENTITY_opcode)
        { 

            // The switch factory is used if the op1 is IDENTITY, or if no
            // typecasting is being done.  IDENTITY operator can do arbitrary
            // typecasting (it is not used if no typecasting is done).

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_unop_apply(op1,zname,aname) \
                GB (_unop_apply_ ## op1 ## zname ## aname)

            #define GB_WORKER(op1,zname,ztype,aname,atype)              \
            {                                                           \
                if (GB_unop_apply (op1,zname,aname) ((ztype *) Cx,      \
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

        GB_BURBLE_N (anz, "(generic apply: %s) ", op1->name) ;

        size_t asize = Atype->size ;
        size_t zsize = op1->ztype->size ;
        size_t xsize = op1->xtype->size ;
        GB_Type_code acode = Atype->code ;
        GB_Type_code xcode = op1->xtype->code ;
        GB_cast_function cast_A_to_X = GB_cast_factory (xcode, acode) ;
        GxB_unary_function fop = op1->function ;

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
    else
    {

        //----------------------------------------------------------------------
        // apply a binary operator (bound to a scalar)
        //----------------------------------------------------------------------

        ASSERT_BINARYOP_OK (op2, "standard op2 for GB_apply_op", GB0) ;
        ASSERT_SCALAR_OK (scalar, "scalar for GB_apply_op", GB0) ;

        GB_Type_code xcode, ycode, zcode ;
        ASSERT (opcode != GB_FIRST_opcode) ;
        ASSERT (opcode != GB_SECOND_opcode) ;
        ASSERT (opcode != GB_PAIR_opcode) ;
        ASSERT (opcode != GB_ANY_opcode) ;

        size_t asize = Atype->size ;
        size_t ssize = scalar->type->size ;
        size_t zsize = op2->ztype->size ;
        size_t xsize = op2->xtype->size ;
        size_t ysize = op2->ytype->size ;

        GB_Type_code scode = scalar->type->code ;
        xcode = op2->xtype->code ;
        ycode = op2->ytype->code ;

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

        #ifndef GBCOMPACT
        if (binop_bind1st)
        {

            //------------------------------------------------------------------
            // z = op2(scalar,Ax)
            //------------------------------------------------------------------

            if (GB_binop_builtin (op2->xtype, false, Atype, false,
                op2, false, &opcode, &xcode, &ycode, &zcode))
            { 

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_bind1st(op2,xname) GB (_bind1st_ ## op2 ## xname)
                #define GB_BINOP_WORKER(op2,xname)                  \
                {                                                   \
                    if (GB_bind1st (op2, xname) (Cx, scalarx, Ax,   \
                        Ab, anz, nthreads) == GrB_SUCCESS)          \
                        return (GrB_SUCCESS) ;                      \
                }                                                   \
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
            // z = op2(Ax,scalar)
            //------------------------------------------------------------------

            if (GB_binop_builtin (Atype, false, op2->ytype, false,
                op2, false, &opcode, &xcode, &ycode, &zcode))
            { 

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_bind2nd(op2,xname) GB (_bind2nd_ ## op2 ## xname)
                #undef  GB_BINOP_WORKER
                #define GB_BINOP_WORKER(op2,xname)                  \
                {                                                   \
                    if (GB_bind2nd (op2, xname) (Cx, Ax, scalarx,   \
                        Ab, anz, nthreads) == GrB_SUCCESS)          \
                        return (GrB_SUCCESS) ;                      \
                }                                                   \
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

        GB_BURBLE_N (anz, "(generic apply: %s) ", op2->name) ;

        GB_Type_code acode = Atype->code ;
        GxB_binary_function fop = op2->function ;
        ASSERT (!A->iso) ;

        if (binop_bind1st)
        {
            // Cx = op2 (scalar,Ax)
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
            // Cx = op2 (Ax,scalar)
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

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    return (GrB_SUCCESS) ;
}

