//------------------------------------------------------------------------------
// GB_apply_op:  apply a unary operator to an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Cx = op ((xtype) Ax)

// Compare with GB_transpose_op.c

// PARALLEL: do it here, but it is easy.  Might want to split into separate
// files like Generated/GB_AxB*, so worker is not in a macro but in a function.

#include "GB.h"

void GB_apply_op            // apply a unary operator, Cx = op ((xtype) Ax)
(
    GB_void *Cx,            // output array, of type op->ztype
    const GrB_UnaryOp op,   // operator to apply
    const GB_void *Ax,      // input array, of type atype
    const GrB_Type atype,   // type of Ax
    const int64_t anz,      // size of Ax and Cx
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cx != NULL) ;
    ASSERT (Ax != NULL) ;
    ASSERT (anz >= 0) ;
    ASSERT (atype != NULL) ;
    ASSERT (op != NULL) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // Some unary operators z=f(x) do not use the value x, like z=1.  This is
    // intentional, so the gcc warning is ignored.
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"

    // For built-in types only, thus xtype == ztype, but atype can differ
    #define GB_WORKER(ztype,atype)                              \
    {                                                           \
        ztype *cx = (ztype *) Cx ;                              \
        atype *ax = (atype *) Ax ;                              \
        for (int64_t p = 0 ; p < anz ; p++)                     \
        {                                                       \
            /* x = (ztype) ax [p], type casting */              \
            ztype x ;                                           \
            GB_CAST (x, ax [p]) ;                               \
            /* apply the unary operator */                      \
            cx [p] = GB_OP (x) ;                                \
        }                                                       \
        return ;                                                \
    }

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    // If GB_COMPACT is defined, the switch factory is disabled and all
    // work is done by the generic worker.  The compiled code will be more
    // compact, but 3 to 4 times slower.

    #ifndef GBCOMPACT

        // switch factory for two types, controlled by code1 and code2
        GB_Type_code code1 = op->ztype->code ;      // defines ztype
        GB_Type_code code2 = atype->code ;          // defines atype

        ASSERT (code1 <= GB_UDT_code) ;
        ASSERT (code2 <= GB_UDT_code) ;

        // GB_BOP(x) is for boolean x, GB_IOP(x) for integer (int* and uint*),
        // and GB_FOP(x) is for float, and GB_DOP(x) is for double.

        // NOTE: some of these operators z=f(x) do not depend on x, like z=1.
        // x is read anyway, but the compiler can remove that as dead code if
        // it is able to.  gcc with -Wunused-but-set-variable will complain,
        // but there's no simple way to silence this spurious warning.
        // Ignore it.

        switch (op->opcode)
        {

            case GB_ONE_opcode :       // z = 1

                #define GB_BOP(x) true
                #define GB_IOP(x) 1
                #define GB_FOP(x) 1
                #define GB_DOP(x) 1
                #include "GB_2type_template.c"
                break ;

            case GB_IDENTITY_opcode :  // z = x

                #define GB_BOP(x) x
                #define GB_IOP(x) x
                #define GB_FOP(x) x
                #define GB_DOP(x) x
                #include "GB_2type_template.c"
                break ;

            case GB_AINV_opcode :      // z = -x

                #define GB_BOP(x)  x
                #define GB_IOP(x) -x
                #define GB_FOP(x) -x
                #define GB_DOP(x) -x
                #include "GB_2type_template.c"
                break ;

            case GB_ABS_opcode :       // z = abs(x)

                #define GB_BOP(x) x
                #define GB_IOP(x) GB_IABS(x)
                #define GB_FOP(x) fabsf(x)
                #define GB_DOP(x) fabs(x)
                #include "GB_2type_template.c"
                break ;

            case GB_MINV_opcode :      // z = 1/x

                // see Source/GB.h discussion on boolean and integer division
                #define GB_BOP(x) true
                #define GB_IOP(x) GB_IMINV(x)
                #define GB_FOP(x) 1./x
                #define GB_DOP(x) 1./x
                #include "GB_2type_template.c"
                break ;

            case GB_LNOT_opcode :      // z = ! (x != 0)

                #define GB_BOP(x) !x
                #define GB_IOP(x) (!(x != 0))
                #define GB_FOP(x) (!(x != 0))
                #define GB_DOP(x) (!(x != 0))
                #include "GB_2type_template.c"
                break ;

            default: ;
        }

    #endif

    // If the switch factory has no worker for the opcode or type, then it
    // falls through to the generic worker below.

    //--------------------------------------------------------------------------
    // generic worker:  apply an operator, with optional typecasting
    //--------------------------------------------------------------------------

    // The generic worker can handle any operator and any type, and it does all
    // required typecasting.  Thus the switch factory can be disabled, and the
    // code will more compact and still work.  It will just be slower.

    int64_t asize = atype->size ;
    int64_t zsize = op->ztype->size ;
    GB_cast_function
        cast_A_to_X = GB_cast_factory (op->xtype->code, atype->code) ;
    GxB_unary_function fop = op->function ;

    // scalar workspace
    char xwork [op->xtype->size] ;

    for (int64_t p = 0 ; p < anz ; p++)
    { 
        // xwork = (xtype) Ax [p]
        cast_A_to_X (xwork, Ax +(p*asize), asize) ;
        // Cx [p] = fop (xwork)
        fop (Cx +(p*zsize), xwork) ;
    }
}

