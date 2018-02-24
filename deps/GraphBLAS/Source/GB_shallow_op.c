//------------------------------------------------------------------------------
// GB_shallow_op:  create a shallow copy and apply a unary operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = op (A)

// Create a shallow copy of a matrix, applying an operator to the entries.

// The values are typically not a shallow copy, unless no typecasting is needed
// and the operator is an identity operator.

// The pattern is always a shallow copy.  No errors are checked except for
// out-of-memory conditions.  This function is not user-callable.  Shallow
// matrices are never passed back to the user.

// Compare this function with GB_shallow_cast.c

#include "GB.h"

GrB_Info GB_shallow_op              // create shallow matrix and apply operator
(
    GrB_Matrix *shallow_op_handle,  // output matrix, of type op->ztype
    const GrB_UnaryOp op,           // operator to apply
    const GrB_Matrix A              // input matrix to typecast
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (shallow_op_handle != NULL && *shallow_op_handle == NULL) ;
    ASSERT_OK (GB_check (A, "A for shallow_op", 0)) ;
    ASSERT_OK (GB_check (op, "op for shallow_op", 0)) ;
    ASSERT (GB_Type_compatible (op->xtype, A->type)) ;
    ASSERT ((A->nzmax == 0) == (A->i == NULL && A->x == NULL)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // construct a shallow copy of A for the pattern of C
    //--------------------------------------------------------------------------

    // [ allocate the struct for C, but do not allocate C->p, C->i, or C->x
    GrB_Info info ;
    GB_NEW (shallow_op_handle, op->ztype, A->nrows, A->ncols, false, false) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }
    GrB_Matrix C = *shallow_op_handle ;

    //--------------------------------------------------------------------------
    // make a shallow copy of the column pointers
    //--------------------------------------------------------------------------

    ASSERT (C->magic == MAGIC2) ;   // be careful; C is not yet initialized
    C->p = A->p ;                   // C->p is of size A->ncols + 1
    C->p_shallow = true ;           // C->p will not be freed when freeing C
    C->magic = MAGIC ;              // C is now initialized ]

    //--------------------------------------------------------------------------
    // check for empty matrix
    //--------------------------------------------------------------------------

    if (A->nzmax == 0)
    {
        // C->p is shallow but the rest is empty
        C->nzmax = 0 ;
        C->i = NULL ;
        C->x = NULL ;
        C->i_shallow = false ;
        C->x_shallow = false ;
        ASSERT_OK (GB_check (C, "C = quick copy of empty (A)", 0)) ;
        return (REPORT_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the pattern
    //--------------------------------------------------------------------------

    C->i = A->i ;               // of size A->nzmax
    C->i_shallow = true ;       // C->i will not be freed when freeing C

    //--------------------------------------------------------------------------
    // apply the operator to the numerical values
    //--------------------------------------------------------------------------

    int64_t anz = NNZ (A) ;
    ASSERT (A->nzmax >= IMAX (anz,1)) ;

    if (op->opcode == GB_IDENTITY_opcode && A->type == op->xtype)
    {
        // no work is done at all.  C is a pure shallow copy
        ASSERT (op->ztype == op->xtype) ;
        C->nzmax = A->nzmax ;
        C->x = A->x ;
        C->x_shallow = true ;       // C->x will not be freed when freeing C
        ASSERT_OK (GB_check (C, "C = pure shallow (A)", 0)) ;
        return (REPORT_SUCCESS) ;
    }

    // allocate new space for the numerical values of C
    C->nzmax = IMAX (anz,1) ;
    GB_MALLOC_MEMORY (C->x, C->nzmax, C->type->size) ;
    C->x_shallow = false ;          // free C->x when freeing C
    double memory = GBYTES (C->nzmax, C->type->size) ;
    if (C->x == NULL)
    {
        // out of memory
        GB_MATRIX_FREE (shallow_op_handle) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    void *Cx = C->x ;
    void *Ax = A->x ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // For built-in types only, thus xtype == ztype, but A->type can differ
    #define WORKER(ztype,atype)                                 \
    {                                                           \
        ztype *cx = (ztype *) Cx ;                              \
        atype *ax = (atype *) Ax ;                              \
        for (int64_t p = 0 ; p < anz ; p++)                     \
        {                                                       \
            /* x = (ztype) ax [p], type casting */              \
            ztype x ;                                           \
            CAST (x, ax [p]) ;                                  \
            /* apply the unary operator */                      \
            cx [p] = OP (x) ;                                   \
        }                                                       \
        return (REPORT_SUCCESS) ;                               \
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
        GB_Type_code code2 = A->type->code ;        // defines atype

        ASSERT (code1 <= GB_UDT_code) ;
        ASSERT (code2 <= GB_UDT_code) ;

        // BOP(x) is for boolean x, IOP(x) for integer (int* and uint*),
        // and FOP(x) is for floating-point

        switch (op->opcode)
        {

            case GB_ONE_opcode:        // z = 1

                #define BOP(x) true
                #define IOP(x) 1
                #define FOP(x) 1
                #include "GB_2type_template.c"

            case GB_IDENTITY_opcode:   // z = x

                // Do not create workers when the two codes are the same,
                // C is a pure shallow copy of A, and the function has already
                // returned the result C.
                #define NSAME
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

            case GB_LNOT_opcode:       // z = ! (x != 0)

                #define BOP(x) !x
                #define IOP(x) (!(x != 0))
                #define FOP(x) (!(x != 0))
                #include "GB_2type_template.c"

            default: ;
        }

    #endif

    #undef WORKER

    //--------------------------------------------------------------------------
    // generic worker:  apply an operator, with optional typecasting
    //--------------------------------------------------------------------------

    int64_t asize = A->type->size ;
    int64_t zsize = op->ztype->size ;
    GB_cast_function
        cast_A_to_X = GB_cast_factory (op->xtype->code, A->type->code) ;
    GB_unary_function fop = op->function ;

    // scalar workspace
    char xwork [op->xtype->size] ;

    for (int64_t p = 0 ; p < anz ; p++)
    {
        // xwork = (xtype) Ax [p]
        cast_A_to_X (xwork, Ax +(p*asize), asize) ;
        // Cx [p] = fop (xwork)
        fop (Cx +(p*zsize), xwork) ;
    }

    ASSERT_OK (GB_check (C, "C = shallow (op (A))", 0)) ;
    return (REPORT_SUCCESS) ;
}

