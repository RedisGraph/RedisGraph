//------------------------------------------------------------------------------
// GB_reduce_to_scalar: reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// c = accum (c, reduce_to_scalar(A)), reduce entries in a matrix
// to a scalar.  Not user-callable.  Does the work for GrB_*_reduce_TYPE,
// both matrix and vector.  This funciton tolerates zombies and does not
// delete them.  It does not tolerate pending tuples, so if they are present,
// all zombies are deleted and all pending tuples are assembled.

// This function does not need to know if A is hypersparse or not, and its
// result is the same if A is in CSR or CSC format.

#include "GB.h"

GrB_Info GB_reduce_to_scalar    // twork = reduce_to_scalar (A)
(
    void *c,                    // result scalar
    const GrB_Type ctype,       // the type of scalar, c
    const GrB_BinaryOp accum,   // for c = accum(c,twork)
    const GrB_Monoid reduce,    // monoid to do the reduction
    const GrB_Matrix A,         // matrix to reduce
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // Zombies are an opaque internal detail of the GrB_Matrix data structure
    // that do not depend on anything outside the matrix.  Thus, Table 2.4 of
    // the GrapBLAS spec, version 1.1.0, does not require their deletion.
    // Pending tuples are different, since they rely on another object outside
    // the matrix: the pending operator, which might be user-defined.  Per
    // Table 2.4, the user can expect that GrB_reduce applies the pending
    // operator, which can then be deleted by the user.  Thus, if the pending
    // operator is user-defined it must be applied here.  Assembling pending
    // tuples requires zombies to be deleted first.  Note that if the pending
    // operator is built-in, then the updates could in principle be skipped,
    // but this could be done only if the reduce monoid is the same as the
    // pending operator.

    GB_WAIT_PENDING (A) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;       // Zombies are tolerated, and not deleted
    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;
    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL (c) ;

    ASSERT_OK (GB_check (ctype, "type of scalar c", GB0)) ;
    ASSERT_OK (GB_check (reduce, "reduce for reduce_to_scalar", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for reduce_to_scalar", GB0)) ;
    ASSERT_OK (GB_check (A, "A for reduce_to_scalar", GB0)) ;

    // check domains and dimensions for c = accum (c,twork)
    GrB_Type ztype = reduce->op->ztype ;
    GrB_Info info = GB_compatible (ctype, NULL, NULL, accum, ztype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // twork = reduce (twork,A) must be compatible
    if (!GB_Type_compatible (A->type, ztype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for reduction operator z=%s(x,y):\n"
            "input of type [%s]\n"
            "cannot be typecast to reduction operator of type [%s]",
            reduce->op->name, A->type->name, reduce->op->ztype->name))) ;
    }

    //--------------------------------------------------------------------------
    // scalar workspace
    //--------------------------------------------------------------------------

    int64_t asize = A->type->size ;
    int64_t anz = GB_NNZ (A) ;
    const int64_t *restrict Ai = A->i ;

    int64_t zsize = ztype->size ;

    char awork [zsize] ;
    char twork [zsize] ;

    //--------------------------------------------------------------------------
    // twork = reduce_to_scalar (A)
    //--------------------------------------------------------------------------

    // twork = 0
    memcpy (twork, reduce->identity, zsize) ;

    // reduce all the entries in the matrix, but skip any zombies

    if (A->type == ztype)
    {

        //----------------------------------------------------------------------
        // sum up the entries; no casting needed
        //----------------------------------------------------------------------

        // There are 44 common cases of this function for built-in types and
        // operators.  Four associative operators: min, max, plus, and times
        // with 10 types (int*, uint*, float, and double), and three logical
        // operators (or, and, xor, eq) with a boolean type of C.  All 44 are
        // hard-coded below via a switch factory.  If the case is not handled
        // by the switch factory, 'done' remains false.

        // FUTURE: some operators can terminate early

        bool done = false ;

        // define the worker for the switch factory
        #define GB_WORKER(type)                                             \
        {                                                                   \
            const type *restrict Ax = (type *) A->x ;                       \
            type s ;                                                        \
            memcpy (&s, twork, zsize) ;                                     \
            if (A->nzombies == 0)                                           \
            {                                                               \
                for (int64_t p = 0 ; p < anz ; p++)                         \
                {                                                           \
                    /* s += A(i,j) */                                       \
                    ASSERT (GB_IS_NOT_ZOMBIE (Ai [p])) ;                    \
                    GB_DUP (s, Ax [p]) ;                                    \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                for (int64_t p = 0 ; p < anz ; p++)                         \
                {                                                           \
                    /* s += A(i,j) if the entry is not a zombie */          \
                    if (GB_IS_NOT_ZOMBIE (Ai [p])) GB_DUP (s, Ax [p]) ;     \
                }                                                           \
            }                                                               \
            memcpy (twork, &s, zsize) ;                                     \
            done = true ;                                                   \
        }

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // If GBCOMPACT is defined, the switch factory is disabled and all
        // work is done by the generic worker.  The compiled code will be more
        // compact, but 3 to 4 times slower.

        #ifndef GBCOMPACT

            // controlled by opcode and typecode
            GB_Opcode opcode = reduce->op->opcode ;
            GB_Type_code typecode = A->type->code ;
            ASSERT (typecode <= GB_UDT_code) ;
            #include "GB_assoc_template.c"

        #endif

        //----------------------------------------------------------------------
        // generic worker: sum up the entries, no typecasting
        //----------------------------------------------------------------------

        if (!done)
        {
            // the switch factory didn't handle this case
            GxB_binary_function freduce = reduce->op->function ;
            const GB_void *Ax = A->x ;
            if (A->nzombies == 0)
            {
                for (int64_t p = 0 ; p < anz ; p++)
                { 
                    // twork += A(i,j)
                    ASSERT (GB_IS_NOT_ZOMBIE (Ai [p])) ;
                    // twork += Ax [p]
                    freduce (twork, twork, Ax +(p*asize)) ; // (z x alias)
                }
            }
            else
            {
                for (int64_t p = 0 ; p < anz ; p++)
                {
                    // twork += A(i,j) if not a zombie
                    if (GB_IS_NOT_ZOMBIE (Ai [p]))
                    { 
                        // twork += Ax [p]
                        freduce (twork, twork, Ax +(p*asize)) ; // (z x alias)
                    }
                }
            }
        }
    }
    else
    {

        //----------------------------------------------------------------------
        // generic worker: sum up the entries, with typecasting
        //----------------------------------------------------------------------

        GxB_binary_function freduce = reduce->op->function ;
        GB_cast_function
            cast_A_to_Z = GB_cast_factory (ztype->code, A->type->code) ;

        const GB_void *Ax = A->x ;
        if (A->nzombies == 0)
        {
            for (int64_t p = 0 ; p < anz ; p++)
            { 
                // twork += (ztype) A(i,j)
                ASSERT (GB_IS_NOT_ZOMBIE (Ai [p])) ;
                // awork = (ztype) Ax [p]
                cast_A_to_Z (awork, Ax +(p*asize), zsize) ;
                // twork += awork
                freduce (twork, twork, awork) ; // (z x alias)
            }
        }
        else
        {
            for (int64_t p = 0 ; p < anz ; p++)
            {
                // twork += (ztype) A(i,j) if not a zombie
                if (GB_IS_NOT_ZOMBIE (Ai [p]))
                { 
                    // awork = (ztype) Ax [p]
                    cast_A_to_Z (awork, Ax +(p*asize), zsize) ;

                    // twork += awork
                    freduce (twork, twork, awork) ;     // (z x alias)
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // c = twork or c = accum (c,twork)
    //--------------------------------------------------------------------------

    // This operation does not use GB_accum_mask, since c and twork are
    // scalars, not matrices.  There is no scalar mask.

    if (accum == NULL)
    { 
        // c = (ctype) twork
        GB_cast_function
            cast_Z_to_C = GB_cast_factory (ctype->code, ztype->code) ;
        cast_Z_to_C (c, twork, ctype->size) ;
    }
    else
    { 
        GxB_binary_function faccum = accum->function ;

        GB_cast_function cast_C_to_xaccum, cast_Z_to_yaccum, cast_zaccum_to_C ;
        cast_C_to_xaccum = GB_cast_factory (accum->xtype->code, ctype->code) ;
        cast_Z_to_yaccum = GB_cast_factory (accum->ytype->code, ztype->code) ;
        cast_zaccum_to_C = GB_cast_factory (ctype->code, accum->ztype->code) ;

        // scalar workspace
        char xaccum [accum->xtype->size] ;
        char yaccum [accum->ytype->size] ;
        char zaccum [accum->ztype->size] ;

        // xaccum = (accum->xtype) c
        cast_C_to_xaccum (xaccum, c, ctype->size) ;

        // yaccum = (accum->ytype) twork
        cast_Z_to_yaccum (yaccum, twork, zsize) ;

        // zaccum = xaccum "+" yaccum
        faccum (zaccum, xaccum, yaccum) ;

        // c = (ctype) zaccum
        cast_zaccum_to_C (c, zaccum, ctype->size) ;
    }

    return (GrB_SUCCESS) ;
}

