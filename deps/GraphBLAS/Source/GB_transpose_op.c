//------------------------------------------------------------------------------
// GB_transpose_op: transpose and apply an operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input matrix is m-by-n with cnz nonzeros, with column pointers Ap of
// size n+1.  The pattern of column j is in Ai [Ap [j] ... Ap [j+1]] and thus
// cnz is equal to Ap [n].

// The values of the input matrix are in Ax, of type A_type.  The values of A
// are typecasted to op->xtype and then passed to the unary operator.  The
// output is assigned to R, which must be of type op->ztype; no output
// typecasting done with the output of the operator.

// The row pointers of the output matrix have already been computed, in Rp.
// Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
// version of Rp on *input*.  On output, however, Rp has been shifted down
// by one.  Rp [0:m-1] has been over written with Rp [1:m].  They can be
// shifted back, if needed, but GraphBLAS treats this array Rp, on input
// to this function, as a throw-away copy of Rp.

// Compare with GB_transpose_ix.c.

// No memory allocations are done, and the caller has checked all error
// conditions.  So this function cannot fail.

#include "GB.h"

void GB_transpose_op        // transpose and apply an operator to a matrix
(
    const int64_t *Ap,      // size n+1, input column pointers
    const int64_t *Ai,      // size cnz, input row indices
    const void *Ax,         // size cnz, input numerical values
    const GrB_Type A_type,  // type of input A
    int64_t *Rp,            // size m+1, input: row pointers, shifted on output
    int64_t *Ri,            // size cnz, output column indices
    void *Rx,               // size cnz, output values, type op->ztype
    const int64_t n,        // number of columns in input
    const GrB_UnaryOp op    // operator to apply, NULL if no operator
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A_type, "A type for transpose_op", 0)) ;
    ASSERT_OK (GB_check (op, "op for transpose_op", 0)) ;
    ASSERT (Ap != NULL && Ai != NULL && Ax != NULL) ;
    ASSERT (Rp != NULL && Ri != NULL && Rx != NULL) ;
    ASSERT (n >= 0) ;

    ASSERT (op != NULL) ;
    ASSERT (GB_Type_compatible (A_type, op->xtype)) ;
    ASSERT (IMPLIES (op->opcode != GB_USER_opcode, op->xtype == op->ztype)) ;

    // no zombies are tolerated
    #ifndef NDEBUG
    // there is no A->nzombies flag to check, so check the whole pattern
    int64_t anz = Ap [n] ;
    for (int64_t p = 0 ; p < anz ; p++)
    {
        ASSERT (IS_NOT_ZOMBIE (Ai [p])) ;
    }
    #endif

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // For built-in types only, thus xtype == ztype, but A_type can differ
    #define WORKER(ztype,atype)                                 \
    {                                                           \
        ztype *rx = (ztype *) Rx ;                              \
        atype *ax = (atype *) Ax ;                              \
        for (int64_t j = 0 ; j < n ; j++)                       \
        {                                                       \
            for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)       \
            {                                                   \
                int64_t q = Rp [Ai [p]]++ ;                     \
                Ri [q] = j ;                                    \
                /* x = (ztype) ax [p], type casting */          \
                ztype x ;                                       \
                CAST (x, ax [p]) ;                              \
                /* apply the unary operator */                  \
                rx [q] = OP (x) ;                               \
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
        GB_Type_code code2 = A_type->code ;         // defines atype

        ASSERT (code1 <= GB_UDT_code) ;
        ASSERT (code2 <= GB_UDT_code) ;

        switch (op->opcode)
        {

            case GB_ONE_opcode:        // z = 1

                #define BOP(x) true
                #define IOP(x) 1
                #define FOP(x) 1
                #include "GB_2type_template.c"

            case GB_IDENTITY_opcode:   // z = x

                #define BOP(x) x
                #define IOP(x) x
                #define FOP(x) x
                #include "GB_2type_template.c"

            case GB_AINV_opcode:       // z = -x

                #define BOP(x)  x
                #define IOP(x) -x
                #define FOP(x) -x
                #include "GB_2type_template.c"

            case GB_ABS_opcode:        // z = abs(x)

                #define BOP(x)  x
                #define IOP(x)  IABS(x)
                #define FOP(x)  FABS(x)
                #include "GB_2type_template.c"

            case GB_MINV_opcode:       // z = 1/x

                // see Source/GB.h discussion on boolean and integer division
                #define BOP(x) true
                #define IOP(x) IMINV(x)
                #define FOP(x) 1./x
                #include "GB_2type_template.c"

            case GB_LNOT_opcode:       // z = !x

                #define BOP(x) !x
                #define IOP(x) (!(x != 0))
                #define FOP(x) (!(x != 0))
                #include "GB_2type_template.c"

            default: ;
        }

    #endif

    #undef WORKER

    // If the switch factory has no worker for the opcode or type, then it
    // falls through to the generic worker below.

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    // The generic worker can handle any operator and any type, and it does all
    // required typecasting.  Thus the switch factory can be disabled, and the
    // code will more compact and still work.  It will just be slower.

    int64_t asize = A_type->size ;
    int64_t zsize = op->ztype->size ;
    GB_cast_function
        cast_A_to_X = GB_cast_factory (op->xtype->code, A_type->code) ;
    GB_unary_function fop = op->function ;

    // scalar workspace
    char xwork [op->xtype->size] ; 

    for (int64_t j = 0 ; j < n ; j++)
    {
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
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

