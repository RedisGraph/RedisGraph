//------------------------------------------------------------------------------
// GB_transpose_op: transpose and apply an operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// R = op ((xtype) A')

// The values of A are typecasted to op->xtype and then passed to the unary
// operator.  The output is assigned to R, which must be of type op->ztype; no
// output typecasting done with the output of the operator.

// The row pointers of the output matrix have already been computed, in Rp.
// Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
// version of Rp on *input*.  On output, however, Rp has been shifted down
// by one.  Rp [0:m-1] has been over written with Rp [1:m].  They can be
// shifted back, if needed, but GraphBLAS treats this array Rp, on input
// to this function, as a throw-away copy of Rp.

// Compare with GB_transpose_ix.c and GB_apply_op.c

// PARALLEL: the bucket transpose will not be simple to parallelize.  The qsort
// method of transpose would be more parallel.  This method might remain mostly
// sequential.

#include "GB.h"

void GB_transpose_op        // transpose and apply an operator to a matrix
(
    int64_t *Rp,            // size m+1, input: row pointers, shifted on output
    int64_t *Ri,            // size cnz, output column indices
    GB_void *Rx,            // size cnz, output values, type op->ztype
    const GrB_UnaryOp op,   // operator to apply, NULL if no operator
    const GrB_Matrix A,     // input matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    ASSERT (op != NULL) ;
    ASSERT (Rp != NULL && Ri != NULL && Rx != NULL) ;
    ASSERT (op != NULL) ;
    ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    ASSERT (GB_IMPLIES (op->opcode < GB_USER_C_opcode, op->xtype == op->ztype));
    ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // get the input matrix
    //--------------------------------------------------------------------------

    const int64_t *Ai = A->i ;
    const GB_void *Ax = A->x ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // Some unary operators z=f(x) do not use the value x, like z=1.  This is
    // intentional, so the gcc warning is ignored.
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"

    // For built-in types only, thus xtype == ztype, but A->type can differ
    #define GB_WORKER(ztype,atype)                              \
    {                                                           \
        ztype *rx = (ztype *) Rx ;                              \
        atype *ax = (atype *) Ax ;                              \
        GBI_for_each_vector (A)                                 \
        {                                                       \
            GBI_for_each_entry (j, p, pend)                     \
            {                                                   \
                int64_t q = Rp [Ai [p]]++ ;                     \
                Ri [q] = j ;                                    \
                /* x = (ztype) ax [p], type casting */          \
                ztype x ;                                       \
                GB_CAST (x, ax [p]) ;                           \
                /* apply the unary operator */                  \
                rx [q] = GB_OP (x) ;                            \
            }                                                   \
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
        GB_Type_code code2 = A->type->code ;         // defines atype

        ASSERT (code1 <= GB_UDT_code) ;
        ASSERT (code2 <= GB_UDT_code) ;

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
    // generic worker: transpose and apply an operator, with optional typecast
    //--------------------------------------------------------------------------

    // The generic worker can handle any operator and any type, and it does all
    // required typecasting.  Thus the switch factory can be disabled, and the
    // code will more compact and still work.  It will just be slower.

    int64_t asize = A->type->size ;
    int64_t zsize = op->ztype->size ;
    GB_cast_function
        cast_A_to_X = GB_cast_factory (op->xtype->code, A->type->code) ;
    GxB_unary_function fop = op->function ;

    // scalar workspace
    char xwork [op->xtype->size] ;

    GBI_for_each_vector (A)
    {
        GBI_for_each_entry (j, p, pend)
        { 
            int64_t q = Rp [Ai [p]]++ ;
            Ri [q] = j ;
            // xwork = (xtype) Ax [p]
            cast_A_to_X (xwork, Ax +(p*asize), asize) ;
            // Rx [q] = fop (xwork) ; Rx is of type op->ztype
            fop (Rx +(q*zsize), xwork) ;
        }
    }
}

