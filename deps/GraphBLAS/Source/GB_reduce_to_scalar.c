//------------------------------------------------------------------------------
// GB_reduce_to_scalar: reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// c = accum (c, reduce_to_scalar(A)), reduce entries in a matrix to a scalar.
// Does the work for GrB_*_reduce_TYPE, both matrix and vector.

// This function does not need to know if A is hypersparse or not, and its
// result is the same if A is in CSR or CSC format.

#include "GB_reduce.h"
#ifndef GBCOMPACT
#include "GB_red__include.h"
#endif

GrB_Info GB_reduce_to_scalar    // s = reduce_to_scalar (A)
(
    void *c,                    // result scalar
    const GrB_Type ctype,       // the type of scalar, c
    const GrB_BinaryOp accum,   // for c = accum(c,s)
    const GrB_Monoid reduce,    // monoid to do the reduction
    const GrB_Matrix A,         // matrix to reduce
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;
    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL (c) ;

    ASSERT_OK (GB_check (ctype, "type of scalar c", GB0)) ;
    ASSERT_OK (GB_check (reduce, "reduce for reduce_to_scalar", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for reduce_to_scalar", GB0)) ;
    ASSERT_OK (GB_check (A, "A for reduce_to_scalar", GB0)) ;

    // check domains and dimensions for c = accum (c,s)
    GrB_Type ztype = reduce->op->ztype ;
    GrB_Info info = GB_compatible (ctype, NULL, NULL, accum, ztype, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // s = reduce (s,A) must be compatible
    if (!GB_Type_compatible (A->type, ztype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for reduction operator z=%s(x,y):\n"
            "input of type [%s]\n"
            "cannot be typecast to reduction operator of type [%s]",
            reduce->op->name, A->type->name, reduce->op->ztype->name))) ;
    }

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    GB_WAIT (A) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t asize = A->type->size ;
    int64_t zsize = ztype->size ;
    int64_t anz = GB_NNZ (A) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;
    ntasks = GB_IMIN (ntasks, anz) ;
    ntasks = GB_IMAX (ntasks, 1) ;

    //--------------------------------------------------------------------------
    // s = reduce_to_scalar (A)
    //--------------------------------------------------------------------------

    // s = identity
    GB_void s [zsize] ;
    memcpy (s, reduce->identity, zsize) ;

    // get terminal value, if any
    GB_void *restrict terminal = reduce->terminal ;

    if (anz == 0)
    { 

        //----------------------------------------------------------------------
        // nothing to do
        //----------------------------------------------------------------------

        ;

    }
    else if (A->type == ztype)
    {

        //----------------------------------------------------------------------
        // sum up the entries; no typecasting needed
        //----------------------------------------------------------------------

        // There are 44 common cases of this function for built-in types and
        // operators.  Four associative operators: MIN, MAX, PLUS, and TIMES
        // with 10 types (int*, uint*, float, and double), and four logical
        // operators (OR, AND, XOR, EQ) with a boolean type of C.  All 44 are
        // hard-coded below via a switch factory.  If the case is not handled
        // by the switch factory, 'done' remains false.  The hard-coded workers
        // do no typecasting at all.

        bool done = false ;

        // define the worker for the switch factory

        #define GB_red(opname,aname) GB_red_scalar_ ## opname ## aname

        #define GB_RED_WORKER(opname,aname,atype)                       \
        {                                                               \
            info = GB_red (opname, aname) ((atype *) s, A, ntasks, nthreads) ; \
            done = (info != GrB_NO_VALUE) ;                             \
        }                                                               \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        #ifndef GBCOMPACT

            // controlled by opcode and typecode
            GB_Opcode opcode = reduce->op->opcode ;
            GB_Type_code typecode = A->type->code ;
            ASSERT (typecode <= GB_UDT_code) ;

            #include "GB_red_factory.c"

        #endif

        //----------------------------------------------------------------------
        // generic worker: sum up the entries, no typecasting
        //----------------------------------------------------------------------

        if (!done)
        { 

            // the switch factory didn't handle this case
            GxB_binary_function freduce = reduce->op->function ;

            #define GB_ATYPE GB_void

            // no panel used
            #define GB_PANEL 1

            // workspace for each thread
            #define GB_REDUCTION_WORKSPACE(W, ntasks)               \
                GB_void W [ntasks*zsize]

            // ztype t = identity
            #define GB_SCALAR_IDENTITY(t)                           \
                GB_void t [zsize] ;                                 \
                memcpy (t, reduce->identity, zsize) ;

            // t = W [tid], no typecast
            #define GB_COPY_ARRAY_TO_SCALAR(t, W, tid)              \
                memcpy (t, W +(tid*zsize), zsize)

            // W [tid] = t, no typecast
            #define GB_COPY_SCALAR_TO_ARRAY(W, tid, t)              \
                memcpy (W +(tid*zsize), t, zsize)

            // s += W [k], no typecast
            #define GB_ADD_ARRAY_TO_SCALAR(s,W,k)                   \
                freduce (s, s, W +((k)*zsize))

            // break if terminal value reached
            #define GB_BREAK_IF_TERMINAL(s)                         \
                if (terminal != NULL)                               \
                {                                                   \
                    if (memcmp (s, terminal, zsize) == 0) break ;   \
                }

            // skip the work for this task if early exit is reached
            #define GB_IF_NOT_EARLY_EXIT                            \
                bool my_exit ;                                      \
                GB_PRAGMA (omp atomic read)                         \
                my_exit = early_exit ;                              \
                if (!my_exit)

            // break if terminal value reached, inside parallel task
            #define GB_PARALLEL_BREAK_IF_TERMINAL(s)                \
                if (terminal != NULL)                               \
                {                                                   \
                    if (memcmp (s, terminal, zsize) == 0)           \
                    {                                               \
                        /* tell the other tasks to exit early */    \
                        GB_PRAGMA (omp atomic write)                \
                        early_exit = true ;                         \
                        break ;                                     \
                    }                                               \
                }

            // ztype t ;
            #define GB_SCALAR(t)                                    \
                GB_void t [zsize]

            // t = (ztype) Ax [p], but no typecasting needed
            #define GB_CAST_ARRAY_TO_SCALAR(t,Ax,p)                 \
                memcpy (t, Ax +((p)*zsize), zsize)

            // t += (ztype) Ax [p], but no typecasting needed
            #define GB_ADD_CAST_ARRAY_TO_SCALAR(t,Ax,p)             \
                freduce (t, t, Ax +((p)*zsize))

            #include "GB_reduce_to_scalar_template.c"
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

            // t = (ztype) Ax [p], with typecast
            #undef  GB_CAST_ARRAY_TO_SCALAR
            #define GB_CAST_ARRAY_TO_SCALAR(t,Ax,p)                 \
                cast_A_to_Z (t, Ax +((p)*asize), asize)

            // t += (ztype) Ax [p], with typecast
            #undef  GB_ADD_CAST_ARRAY_TO_SCALAR
            #define GB_ADD_CAST_ARRAY_TO_SCALAR(t,Ax,p)             \
                GB_void awork [zsize] ;                             \
                cast_A_to_Z (awork, Ax +((p)*asize), asize) ;       \
                freduce (t, t, awork)

          #include "GB_reduce_to_scalar_template.c"
    }

    //--------------------------------------------------------------------------
    // c = s or c = accum (c,s)
    //--------------------------------------------------------------------------

    // This operation does not use GB_accum_mask, since c and s are
    // scalars, not matrices.  There is no scalar mask.

    if (accum == NULL)
    { 
        // c = (ctype) s
        GB_cast_function
            cast_Z_to_C = GB_cast_factory (ctype->code, ztype->code) ;
        cast_Z_to_C (c, s, ctype->size) ;
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

        // yaccum = (accum->ytype) s
        cast_Z_to_yaccum (yaccum, s, zsize) ;

        // zaccum = xaccum "+" yaccum
        faccum (zaccum, xaccum, yaccum) ;

        // c = (ctype) zaccum
        cast_zaccum_to_C (c, zaccum, ctype->size) ;
    }

    return (GrB_SUCCESS) ;
}

