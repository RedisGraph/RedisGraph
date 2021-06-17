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

GrB_Info GB_apply_op                // apply a unary operator, Cx = op (A)
(
    GB_void *Cx,                    // output array, of type op->ztype
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,Ax) else binop(Ax,y)
    const GrB_Matrix A,             // input matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cx != NULL) ;
    ASSERT (op1 != NULL || op2 != NULL) ;
    ASSERT_MATRIX_OK (A, "A input for GB_apply_op", GB0) ;
    ASSERT (GB_JUMBLED_OK (A)) ;        // A can be jumbled

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const GB_void *Ax = (GB_void *) A->x ;  // A->x has type A->type
    const int8_t  *Ab = A->b ;              // only if A is bitmap
    const GrB_Type Atype = A->type ;        // type of A->x
    const int64_t anz = GB_NNZ_HELD (A) ;   // size of A->x and Cx

    //--------------------------------------------------------------------------
    // determine the maximum number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // apply the operator
    //--------------------------------------------------------------------------

    GB_Opcode opcode = (op1 != NULL) ? op1->opcode : op2->opcode ;

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
        const int64_t *GB_RESTRICT Ah = A->h ;
        const int64_t *GB_RESTRICT Ap = A->p ;
        const int64_t *GB_RESTRICT Ai = A->i ;
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
            int64_t *GB_RESTRICT Cx_int = (int64_t *) Cx ;
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
            int32_t *GB_RESTRICT Cx_int = (int32_t *) Cx ;
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
    else if (op1 != NULL)
    {

        //----------------------------------------------------------------------
        // unary operator
        //----------------------------------------------------------------------

        ASSERT_UNARYOP_OK (op1, "op1 for GB_apply_op", GB0) ;

        // determine number of threads to use
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        GrB_UnaryOp op = op1 ;

        #ifndef GBCOMPACT
        if ((Atype == op->xtype)
            || (opcode == GB_IDENTITY_opcode) || (opcode == GB_ONE_opcode))
        { 

            // The switch factory is used if the op is IDENTITY or ONE, or if
            // no typecasting is being done.  The ONE operator ignores the type
            // of its input and just produces a 1 of op->ztype == op->xtype.
            // The IDENTITY operator can do arbitrary typecasting.

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_unop_apply(op,zname,aname) \
                GB_unop_apply_ ## op ## zname ## aname

            #define GB_WORKER(op,zname,ztype,aname,atype)               \
            {                                                           \
                if (GB_unop_apply (op,zname,aname) ((ztype *) Cx,       \
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
        GB_cast_function
            cast_A_to_X = GB_cast_factory (op->xtype->code, Atype->code) ;
        GxB_unary_function fop = op->function ;

        int64_t p ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (p = 0 ; p < anz ; p++)
        { 
            if (!GBB (Ab, p)) continue ;
            // xwork = (xtype) Ax [p]
            GB_void xwork [GB_VLA(xsize)] ;
            cast_A_to_X (xwork, Ax +(p*asize), asize) ;
            // Cx [p] = fop (xwork)
            fop (Cx +(p*zsize), xwork) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // binary operator
        //----------------------------------------------------------------------

        ASSERT_BINARYOP_OK (op2, "standard op2 for GB_apply_op", GB0) ;
        ASSERT_SCALAR_OK (scalar, "scalar for GB_apply_op", GB0) ;

        // determine number of threads to use
        int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

        GB_Type_code xcode, ycode, zcode ;
        bool op_is_first  = (opcode == GB_FIRST_opcode) ;
        bool op_is_second = (opcode == GB_SECOND_opcode) ;
        bool op_is_pair   = (opcode == GB_PAIR_opcode) ;

        size_t asize = Atype->size ;
        size_t ssize = scalar->type->size ;
        size_t zsize = op2->ztype->size ;
        size_t xsize = op2->xtype->size ;
        size_t ysize = op2->ytype->size ;

        GB_Type_code scode = scalar->type->code ;
        xcode = op2->xtype->code ;
        ycode = op2->ytype->code ;

        // typecast the scalar to the operator input
        bool ignore_scalar = false ;
        size_t ssize_cast ;
        GB_Type_code scode_cast ;
        if (binop_bind1st)
        { 
            ssize_cast = xsize ;
            scode_cast = xcode ;
            ignore_scalar = op_is_second || op_is_pair ;
        }
        else
        { 
            ssize_cast = ysize ;
            scode_cast = ycode ;
            ignore_scalar = op_is_first  || op_is_pair ;
        }
        GB_void swork [GB_VLA(ssize_cast)] ;
        GB_void *scalarx = (GB_void *) scalar->x ;
        if (scode_cast != scode && !ignore_scalar)
        { 
            // typecast the scalar to the operator input, in swork
            GB_cast_function cast_s = GB_cast_factory (scode_cast, scode) ;
            cast_s (swork, scalar->x, ssize) ;
            scalarx = swork ;
        }

        #ifndef GBCOMPACT

            if (binop_bind1st)
            {

                //--------------------------------------------------------------
                // z = op(scalar,Ax)
                //--------------------------------------------------------------

                if (GB_binop_builtin (
                    op2->xtype, ignore_scalar,
                    Atype,      op_is_first  || op_is_pair,
                    op2, false, &opcode, &xcode, &ycode, &zcode))
                { 

                    //----------------------------------------------------------
                    // define the worker for the switch factory
                    //----------------------------------------------------------

                    #define GB_bind1st(op,xname) GB_bind1st_ ## op ## xname

                    #define GB_BINOP_WORKER(op,xname)                        \
                    {                                                        \
                        if (GB_bind1st (op, xname) (Cx, scalarx, Ax, Ab, anz,\
                            nthreads) == GrB_SUCCESS) return (GrB_SUCCESS) ; \
                    }                                                        \
                    break ;

                    //----------------------------------------------------------
                    // launch the switch factory
                    //----------------------------------------------------------

                    #define GB_NO_SECOND
                    #define GB_NO_PAIR
                    #include "GB_binop_factory.c"
                }
            }
            else
            {

                //--------------------------------------------------------------
                // z = op(Ax,scalar)
                //--------------------------------------------------------------

                if (GB_binop_builtin (
                    Atype,      op_is_second || op_is_pair,
                    op2->ytype, ignore_scalar,
                    op2, false, &opcode, &xcode, &ycode, &zcode))
                { 

                    //----------------------------------------------------------
                    // define the worker for the switch factory
                    //----------------------------------------------------------

                    #define GB_bind2nd(op,xname) GB_bind2nd_ ## op ## xname
                    #undef  GB_BINOP_WORKER
                    #define GB_BINOP_WORKER(op,xname)                        \
                    {                                                        \
                        if (GB_bind2nd (op, xname) (Cx, Ax, scalarx, Ab, anz,\
                            nthreads) == GrB_SUCCESS) return (GrB_SUCCESS) ; \
                    }                                                        \
                    break ;

                    //----------------------------------------------------------
                    // launch the switch factory
                    //----------------------------------------------------------

                    #define GB_NO_FIRST
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

        if (binop_bind1st)
        {
            // Cx = op (scalar,Ax)
            GB_cast_function cast_A_to_Y = GB_cast_factory (ycode, acode) ;
            int64_t p ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < anz ; p++)
            { 
                if (!GBB (Ab, p)) continue ;
                // ywork = (ytype) Ax [p]
                GB_void ywork [GB_VLA(ysize)] ;
                cast_A_to_Y (ywork, Ax +(p*asize), asize) ;
                // Cx [p] = fop (xwork, ywork)
                fop (Cx +(p*zsize), scalarx, ywork) ;
            }
        }
        else
        {
            // Cx = op (Ax,scalar)
            GB_cast_function cast_A_to_X = GB_cast_factory (xcode, acode) ;
            int64_t p ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < anz ; p++)
            { 
                if (!GBB (Ab, p)) continue ;
                // xwork = (xtype) Ax [p]
                GB_void xwork [GB_VLA(xsize)] ;
                cast_A_to_X (xwork, Ax +(p*asize), asize) ;
                // Cx [p] = fop (xwork, ywork)
                fop (Cx +(p*zsize), xwork, scalarx) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

