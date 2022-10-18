//------------------------------------------------------------------------------
// GB_transpose_op: transpose, typecast, and apply an operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = op (A')

// The values of A are typecasted to op->xtype and then passed to the unary
// operator.  The output is assigned to C, which must be of type op->ztype; no
// output typecasting done with the output of the operator.

// If the op is positional, it has been replaced with the unary op
// GxB_ONE_INT64, as a placeholder, and C_code_iso is GB_ISO_1.  The true op
// is applied later, in GB_transpose.

// If A is sparse or hypersparse
//      The pattern of C is constructed.  C is sparse.
//      Workspaces and A_slice are non-NULL.
//      This method is parallel, but not highly scalable.  It uses only
//      nthreads = nnz(A)/(A->vlen) threads.

// If A is full or as-if-full:
//      The pattern of C is not constructed.  C is full.
//      Workspaces and A_slice are NULL.
//      This method is parallel and fully scalable.

// If A is bitmap:
//      C->b is constructed.  C is bitmap.
//      Workspaces and A_slice are NULL.
//      This method is parallel and fully scalable.

#include "GB_transpose.h"
#include "GB_binop.h"
#ifndef GBCUDA_DEV
#include "GB_unop__include.h"
#include "GB_binop__include.h"
#endif

void GB_transpose_op    // transpose, typecast, and apply operator to a matrix
(
    GrB_Matrix C,                       // output matrix
    const GB_iso_code C_code_iso,       // iso code for C
        // no operator is applied if op is NULL
        const GB_Operator op,           // unary/idxunop/binop to apply
        const GrB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const GrB_Matrix A,                 // input matrix
    // for sparse or hypersparse case:
    int64_t *restrict *Workspaces,      // Workspaces, size nworkspaces
    const int64_t *restrict A_slice,    // how A is sliced, size nthreads+1
    int nworkspaces,                    // # of workspaces to use
    // for all cases:
    int nthreads                        // # of threads to use
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    GrB_Info info ;
    GrB_Type Atype = A->type ;
    ASSERT (op != NULL) ;
    GB_Opcode opcode = op->opcode ;

    // positional operators and user idxunop are applied after the transpose
    ASSERT (!GB_OPCODE_IS_POSITIONAL (opcode)) ;
    ASSERT (!GB_IS_INDEXUNARYOP_CODE (opcode)) ;

    //--------------------------------------------------------------------------
    // transpose the matrix and apply the operator
    //--------------------------------------------------------------------------

    if (C->iso)
    { 

        //----------------------------------------------------------------------
        // apply the operator to the iso value and transpose the pattern
        //----------------------------------------------------------------------

        // if C is iso, only the pattern is transposed.  The numerical work
        // takes O(1) time

        // Cx [0] = unop (A), binop (scalar,A), or binop (A,scalar)
        GB_iso_unop ((GB_void *) C->x, C->type, C_code_iso, op, A, scalar) ;

        // C = transpose the pattern
        #define GB_ISO_TRANSPOSE
        #include "GB_unop_transpose.c"

    }
    else if (GB_IS_UNARYOP_CODE (opcode))
    {

        //----------------------------------------------------------------------
        // apply the unary operator to all entries
        //----------------------------------------------------------------------

        ASSERT_UNARYOP_OK (op, "op for transpose", GB0) ;

        //----------------------------------------------------------------------
        // transpose the matrix; apply the unary op to all values if non-iso
        //----------------------------------------------------------------------

        #ifndef GBCUDA_DEV
        if (Atype == op->xtype || opcode == GB_IDENTITY_unop_code)
        { 

            // The switch factory is used if the unop is IDENTITY, or if no
            // typecasting is being done.  The IDENTITY operator can do
            // arbitrary typecasting.

            //------------------------------------------------------------------
            // define the worker for the switch factory
            //------------------------------------------------------------------

            #define GB_unop_tran(opname,zname,aname) \
                GB (_unop_tran_ ## opname ## zname ## aname)

            #define GB_WORKER(opname,zname,ztype,aname,atype)               \
            {                                                               \
                info = GB_unop_tran (opname,zname,aname)                    \
                    (C, A, Workspaces, A_slice, nworkspaces, nthreads) ;    \
                if (info == GrB_SUCCESS) return ;                           \
            }                                                               \
            break ;

            //------------------------------------------------------------------
            // launch the switch factory
            //------------------------------------------------------------------

            #include "GB_unop_factory.c"
        }
        #endif

        //----------------------------------------------------------------------
        // generic worker: transpose, typecast, and apply unary operator
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (A, "(generic transpose: %s) ", op->name) ;

        size_t asize = Atype->size ;
        size_t zsize = op->ztype->size ;
        size_t xsize = op->xtype->size ;
        GB_cast_function
            cast_A_to_X = GB_cast_factory (op->xtype->code, Atype->code) ;
        GxB_unary_function fop = op->unop_function ;

        ASSERT_TYPE_OK (op->ztype, "unop ztype", GB0) ;
        ASSERT_TYPE_OK (op->xtype, "unop xtype", GB0) ;
        ASSERT_TYPE_OK (C->type, "C type", GB0) ;
        ASSERT (C->type->size == zsize) ;
        ASSERT (C->type == op->ztype) ;

        // Cx [pC] = unop (cast (Ax [pA]))
        #undef  GB_CAST_OP
        #define GB_CAST_OP(pC,pA)                                       \
        {                                                               \
            /* xwork = (xtype) Ax [pA] */                               \
            GB_void xwork [GB_VLA(xsize)] ;                             \
            cast_A_to_X (xwork, Ax +((pA)*asize), asize) ;              \
            /* Cx [pC] = fop (xwork) ; Cx is of type op->ztype */       \
            fop (Cx +((pC)*zsize), xwork) ;                             \
        }

        #define GB_ATYPE GB_void
        #define GB_CTYPE GB_void
        #include "GB_unop_transpose.c"

    }
    else
    {

        //----------------------------------------------------------------------
        // apply a binary operator (bound to a scalar)
        //----------------------------------------------------------------------

        ASSERT_BINARYOP_OK (op, "binop for transpose", GB0) ;

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

        GB_Type_code acode = Atype->code ;
        GxB_binary_function fop = op->binop_function ;
        GB_cast_function cast_A_to_Y = GB_cast_factory (ycode, acode) ;
        GB_cast_function cast_A_to_X = GB_cast_factory (xcode, acode) ;

        //----------------------------------------------------------------------
        // transpose the matrix; apply the binary op to all values if non-iso
        //----------------------------------------------------------------------

        #ifndef GBCUDA_DEV
        if (binop_bind1st)
        {

            //------------------------------------------------------------------
            // C = op(scalar,A')
            //------------------------------------------------------------------

            if (GB_binop_builtin (op->xtype, false, Atype, false,
                (GrB_BinaryOp) op, false, &opcode, &xcode, &ycode, &zcode))
            { 

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_bind1st_tran(op,xname) \
                    GB (_bind1st_tran_ ## op ## xname)

                #define GB_BINOP_WORKER(op,xname)                           \
                {                                                           \
                    if (GB_bind1st_tran (op, xname) (C, scalarx, A,         \
                        Workspaces, A_slice, nworkspaces, nthreads)         \
                        == GrB_SUCCESS) return ;                            \
                }                                                           \
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
            // C = op(A',scalar)
            //------------------------------------------------------------------

            if (GB_binop_builtin (Atype, false, op->ytype, false,
                (GrB_BinaryOp) op, false, &opcode, &xcode, &ycode, &zcode))
            { 

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_bind2nd_tran(op,xname) \
                    GB (_bind2nd_tran_ ## op ## xname)
                #undef  GB_BINOP_WORKER
                #define GB_BINOP_WORKER(op,xname)                           \
                {                                                           \
                    if (GB_bind2nd_tran (op, xname) (C, A, scalarx,         \
                        Workspaces, A_slice, nworkspaces, nthreads)         \
                        == GrB_SUCCESS) return ;                            \
                }                                                           \
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
        // generic worker: transpose, typecast and apply a binary operator
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (A, "(generic transpose: %s) ", op->name) ;

        #define GB_CAST_OP_BIND_1ST(pC,pA)                              \
        {                                                               \
            /* ywork = (ytype) Ax [pA] */                               \
            GB_void ywork [GB_VLA(ysize)] ;                             \
            cast_A_to_Y (ywork, Ax +(pA)*asize, asize) ;                \
            /* Cx [pC] = fop (xwork) ; Cx is of type op->ztype */       \
            fop (Cx +((pC)*zsize), scalarx, ywork) ;                    \
        }

        #define GB_CAST_OP_BIND_2ND(pC,pA)                              \
        {                                                               \
            /* xwork = (xtype) Ax [pA] */                               \
            GB_void xwork [GB_VLA(xsize)] ;                             \
            cast_A_to_X (xwork, Ax +(pA)*asize, asize) ;                \
            /* Cx [pC] = fop (xwork) ; Cx is of type op->ztype */       \
            fop (Cx +(pC*zsize), xwork, scalarx) ;                      \
        }

        if (binop_bind1st)
        { 
            // Cx [pC] = op (cast (scalar), cast (Ax [pA]))
            #undef  GB_CAST_OP
            #define GB_CAST_OP(pC,pA) GB_CAST_OP_BIND_1ST(pC,pA)
            #include "GB_unop_transpose.c"
        }
        else
        { 
            // Cx [pC] = op (cast (Ax [pA]), cast (scalar))
            #undef  GB_CAST_OP
            #define GB_CAST_OP(pC,pA) GB_CAST_OP_BIND_2ND(pC,pA)
            #include "GB_unop_transpose.c"
        }
    }
}

