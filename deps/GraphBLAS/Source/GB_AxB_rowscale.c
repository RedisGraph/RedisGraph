//------------------------------------------------------------------------------
// GB_AxB_rowscale: C = D*B, row scale with diagonal matrix D
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"
#endif

GrB_Info GB_AxB_rowscale            // C = D*B, row scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=D*A
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (Chandle != NULL) ;
    ASSERT_MATRIX_OK (D, "D for rowscale A*D", GB0) ;
    ASSERT_MATRIX_OK (B, "B for rowscale A*D", GB0) ;
    ASSERT (!GB_PENDING (D)) ; ASSERT (!GB_ZOMBIES (D)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_SEMIRING_OK (semiring, "semiring for numeric D*A", GB0) ;
    ASSERT (D->vlen == D->vdim) ;
    ASSERT (D->vlen == B->vlen) ;
    ASSERT (GB_is_diagonal (D, Context)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (GB_NNZ (B) + B->nvec, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    ASSERT (mult->ztype == semiring->add->op->ztype) ;

    bool op_is_first  = mult->opcode == GB_FIRST_opcode ;
    bool op_is_second = mult->opcode == GB_SECOND_opcode ;
    bool op_is_pair   = mult->opcode == GB_PAIR_opcode ;
    bool D_is_pattern = false ;
    bool B_is_pattern = false ;

    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        D_is_pattern = op_is_first  || op_is_pair ;
        B_is_pattern = op_is_second || op_is_pair ;
        ASSERT (GB_IMPLIES (!D_is_pattern,
            GB_Type_compatible (D->type, mult->ytype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->xtype))) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        D_is_pattern = op_is_second || op_is_pair ;
        B_is_pattern = op_is_first  || op_is_pair ;
        ASSERT (GB_IMPLIES (!D_is_pattern,
            GB_Type_compatible (D->type, mult->xtype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->ytype))) ;
    }

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // copy the pattern of B into C
    //--------------------------------------------------------------------------

    info = GB_dup (Chandle, B, false, mult->ztype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    GrB_Matrix C = (*Chandle) ;

    //--------------------------------------------------------------------------
    // C = D*B, row scale
    //--------------------------------------------------------------------------

    bool done = false ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_DxB(mult,xyname) GB_DxB_ ## mult ## xyname

    #define GB_BINOP_WORKER(mult,xyname)                                    \
    {                                                                       \
        info = GB_DxB(mult,xyname) (C, D, D_is_pattern, B, B_is_pattern,    \
            nthreads) ;                                                     \
        done = (info != GrB_NO_VALUE) ;                                     \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        GB_Opcode opcode ;
        GB_Type_code xycode, zcode ;

        if (GB_binop_builtin (D->type, D_is_pattern, B->type, B_is_pattern,
            mult, flipxy, &opcode, &xycode, &zcode))
        { 
            #include "GB_binop_factory.c"
        }

    #endif

    //--------------------------------------------------------------------------
    // C = D*B, row scale, with typecasting or user-defined operator
    //--------------------------------------------------------------------------

    if (!done)
    {
        GB_BURBLE_MATRIX (C, "generic ") ;

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of D, B, and C
        //----------------------------------------------------------------------

        GxB_binary_function fmult = mult->function ;

        size_t csize = C->type->size ;
        size_t dsize = D_is_pattern ? 0 : D->type->size ;
        size_t bsize = B_is_pattern ? 0 : B->type->size ;

        size_t xsize = mult->xtype->size ;
        size_t ysize = mult->ytype->size ;

        // scalar workspace: because of typecasting, the x/y types need not
        // be the same as the size of the D and B types.
        // flipxy false: dii = (xtype) D(i,i) and bij = (ytype) B(i,j)
        // flipxy true:  dii = (ytype) D(i,i) and bij = (xtype) B(i,j)
        size_t dii_size = flipxy ? ysize : xsize ;
        size_t bij_size = flipxy ? xsize : ysize ;

        GB_void *GB_RESTRICT Cx = C->x ;

        GB_cast_function cast_D, cast_B ;
        if (flipxy)
        { 
            // D is typecasted to y, and B is typecasted to x
            cast_D = D_is_pattern ? NULL : 
                     GB_cast_factory (mult->ytype->code, D->type->code) ;
            cast_B = B_is_pattern ? NULL : 
                     GB_cast_factory (mult->xtype->code, B->type->code) ;
        }
        else
        { 
            // D is typecasted to x, and B is typecasted to y
            cast_D = D_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, D->type->code) ;
            cast_B = B_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, B->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = D*B via function pointers, and typecasting
        //----------------------------------------------------------------------

        // dii = D(i,i), located in Dx [i]
        #define GB_GETA(dii,Dx,i)                                           \
            GB_void dii [GB_VLA(dii_size)] ;                                \
            if (!D_is_pattern) cast_D (dii, Dx +((i)*dsize), dsize) ;

        // bij = B(i,j), located in Bx [pB]
        #define GB_GETB(bij,Bx,pB)                                          \
            GB_void bij [GB_VLA(bij_size)] ;                                \
            if (!B_is_pattern) cast_B (bij, Bx +((pB)*bsize), bsize) ;

        // C(i,j) = D(i,i) * B(i,j)
        #define GB_BINOP(cij, dii, bij)                                     \
            GB_BINARYOP (cij, dii, bij) ;                                   \

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        // no vectorization
        #define GB_PRAGMA_VECTORIZE
        #define GB_PRAGMA_VECTORIZE_DOT

        if (flipxy)
        { 
            #define GB_BINARYOP(z,x,y) fmult (z,y,x)
            #include "GB_AxB_rowscale_meta.c"
            #undef GB_BINARYOP
        }
        else
        { 
            #define GB_BINARYOP(z,x,y) fmult (z,x,y)
            #include "GB_AxB_rowscale_meta.c"
            #undef GB_BINARYOP
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "rowscale: C = D*B output", GB0) ;
    ASSERT (*Chandle == C) ;
    return (GrB_SUCCESS) ;
}

