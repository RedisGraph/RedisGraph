//------------------------------------------------------------------------------
// GB_reduce_to_scalar: reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// c = accum (c, reduce_to_scalar(A)), reduce entries in a matrix to a scalar.
// Does the work for GrB_*_reduce_TYPE, both matrix and vector.

// This function does not need to know if A is hypersparse or not, and its
// result is the same if A is in CSR or CSC format.

// This function is the only place in all of GraphBLAS where the identity value
// of a monoid is required, but only in one special case: it is required to be
// the return value of c when A has no entries.  The identity value is also
// used internally, in the parallel methods below, to initialize a scalar value
// in each task.  The methods could be rewritten to avoid the use of the
// identity value.  Since this function requires it anyway, for the special
// case when nvals(A) is zero, the existence of the identity value makes the
// code a little simpler.

#include "GB_reduce.h"
#include "GB_binop.h"
#include "GB_atomics.h"
#include "GB_stringify.h"
#ifndef GBCUDA_DEV
#include "GB_red__include.h"
#endif

#define GB_FREE_ALL                 \
{                                   \
    GB_WERK_POP (F, bool) ;         \
    GB_WERK_POP (W, GB_void) ;      \
}

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

    GrB_Info info ;
    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;
    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;
    GB_RETURN_IF_NULL (c) ;
    GB_WERK_DECLARE (W, GB_void) ;
    GB_WERK_DECLARE (F, bool) ;

    ASSERT_TYPE_OK (ctype, "type of scalar c", GB0) ;
    ASSERT_MONOID_OK (reduce, "reduce for reduce_to_scalar", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for reduce_to_scalar", GB0) ;
    ASSERT_MATRIX_OK (A, "A for reduce_to_scalar", GB0) ;

    // check domains and dimensions for c = accum (c,s)
    GrB_Type ztype = reduce->op->ztype ;
    GB_OK (GB_compatible (ctype, NULL, NULL, false, accum, ztype, Context)) ;

    // s = reduce (s,A) must be compatible
    if (!GB_Type_compatible (A->type, ztype))
    { 
        return (GrB_DOMAIN_MISMATCH) ;
    }

    //--------------------------------------------------------------------------
    // assemble any pending tuples; zombies are OK
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT_IF_PENDING (A) ;
    GB_BURBLE_DENSE (A, "(A %s) ") ;

    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t asize = A->type->size ;
    int64_t zsize = ztype->size ;
    int64_t anz = GB_nnz_held (A) ;
    ASSERT (anz >= A->nzombies) ;

    // s = identity
    GB_void s [GB_VLA(zsize)] ;
    memcpy (s, reduce->identity, zsize) ;   // required, if nnz(A) is zero

    #ifdef GB_DEBUGIFY_DEFN
    GB_debugify_reduce (reduce, A) ;
    #endif

    //--------------------------------------------------------------------------
    // s = reduce_to_scalar (A) on the GPU(s) or CPU
    //--------------------------------------------------------------------------

    #if defined ( GBCUDA )
    if (GB_reduce_to_scalar_cuda_branch (reduce, A, Context))
    {

        //----------------------------------------------------------------------
        // use the GPU(s)
        //----------------------------------------------------------------------

        GB_OK (GB_reduce_to_scalar_cuda (s, reduce, A, Context)) ;

    }
    else
    #endif
    {

        //----------------------------------------------------------------------
        // use OpenMP on the CPU threads
        //----------------------------------------------------------------------

        int nthreads = 0, ntasks = 0 ;
        GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
        nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
        ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;
        ntasks = GB_IMIN (ntasks, anz) ;
        ntasks = GB_IMAX (ntasks, 1) ;

        //----------------------------------------------------------------------
        // allocate workspace
        //----------------------------------------------------------------------

        GB_WERK_PUSH (W, ntasks * zsize, GB_void) ;
        GB_WERK_PUSH (F, ntasks, bool) ;
        if (W == NULL || F == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // s = reduce_to_scalar (A)
        //----------------------------------------------------------------------

        // get terminal value, if any
        GB_void *restrict terminal = (GB_void *) reduce->terminal ;

        if (anz == A->nzombies)
        { 

            //------------------------------------------------------------------
            // no live entries in A; nothing to do
            //------------------------------------------------------------------

            ;

        }
        else if (A->iso)
        { 

            //------------------------------------------------------------------
            // reduce an iso matrix to scalar
            //------------------------------------------------------------------

            // this takes at most O(log(nvals(A))) time, for any monoid
            GB_iso_reduce_to_scalar (s, reduce, A, Context) ;

        }
        else if (A->type == ztype)
        {

            //------------------------------------------------------------------
            // reduce to scalar via built-in operator
            //------------------------------------------------------------------

            bool done = false ;

            #ifndef GBCUDA_DEV

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_red(opname,aname) \
                    GB (_red_scalar_ ## opname ## aname)

                #define GB_RED_WORKER(opname,aname,atype)                   \
                {                                                           \
                    info = GB_red (opname, aname) ((atype *) s, A, W, F,    \
                        ntasks, nthreads) ;                                 \
                    done = (info != GrB_NO_VALUE) ;                         \
                }                                                           \
                break ;

                //--------------------------------------------------------------
                // launch the switch factory
                //--------------------------------------------------------------

                // controlled by opcode and typecode
                GB_Opcode opcode = reduce->op->opcode ;
                GB_Type_code typecode = A->type->code ;
                ASSERT (typecode <= GB_UDT_code) ;

                #include "GB_red_factory.c"

            #endif

            //------------------------------------------------------------------
            // generic worker: sum up the entries, no typecasting
            //------------------------------------------------------------------

            if (!done)
            { 
                GB_BURBLE_MATRIX (A, "(generic reduce to scalar: %s) ",
                    reduce->op->name) ;

                // the switch factory didn't handle this case
                GxB_binary_function freduce = reduce->op->binop_function ;

                #define GB_ATYPE GB_void

                // no panel used
                #define GB_PANEL 1
                #define GB_NO_PANEL_CASE

                // ztype t = identity
                #define GB_SCALAR_IDENTITY(t)                           \
                    GB_void t [GB_VLA(zsize)] ;                         \
                    memcpy (t, reduce->identity, zsize) ;

                // W [tid] = t, no typecast
                #define GB_COPY_SCALAR_TO_ARRAY(W, tid, t)              \
                    memcpy (W +(tid*zsize), t, zsize)

                // s += W [k], no typecast
                #define GB_ADD_ARRAY_TO_SCALAR(s,W,k)                   \
                    freduce (s, s, W +((k)*zsize))

                // break if terminal value reached
                #define GB_HAS_TERMINAL 1
                #define GB_IS_TERMINAL(s) \
                    (terminal != NULL && memcmp (s, terminal, zsize) == 0)

                // t += (ztype) Ax [p], but no typecasting needed
                #define GB_ADD_CAST_ARRAY_TO_SCALAR(t,Ax,p)             \
                    freduce (t, t, Ax +((p)*zsize))

                #include "GB_reduce_to_scalar_template.c"
            }

        }
        else
        { 

            //------------------------------------------------------------------
            // generic worker: sum up the entries, with typecasting
            //------------------------------------------------------------------

            GB_BURBLE_MATRIX (A, "(generic reduce to scalar, with typecast:"
                " %s) ", reduce->op->name) ;

            GxB_binary_function freduce = reduce->op->binop_function ;
            GB_cast_function
                cast_A_to_Z = GB_cast_factory (ztype->code, A->type->code) ;

            // t += (ztype) Ax [p], with typecast
            #undef  GB_ADD_CAST_ARRAY_TO_SCALAR
            #define GB_ADD_CAST_ARRAY_TO_SCALAR(t,Ax,p)             \
                GB_void awork [GB_VLA(zsize)] ;                     \
                cast_A_to_Z (awork, Ax +((p)*asize), asize) ;       \
                freduce (t, t, awork)

            #include "GB_reduce_to_scalar_template.c"
        }
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
        GxB_binary_function faccum = accum->binop_function ;

        GB_cast_function cast_C_to_xaccum, cast_Z_to_yaccum, cast_zaccum_to_C ;
        cast_C_to_xaccum = GB_cast_factory (accum->xtype->code, ctype->code) ;
        cast_Z_to_yaccum = GB_cast_factory (accum->ytype->code, ztype->code) ;
        cast_zaccum_to_C = GB_cast_factory (ctype->code, accum->ztype->code) ;

        // scalar workspace
        GB_void xaccum [GB_VLA(accum->xtype->size)] ;
        GB_void yaccum [GB_VLA(accum->ytype->size)] ;
        GB_void zaccum [GB_VLA(accum->ztype->size)] ;

        // xaccum = (accum->xtype) c
        cast_C_to_xaccum (xaccum, c, ctype->size) ;

        // yaccum = (accum->ytype) s
        cast_Z_to_yaccum (yaccum, s, zsize) ;

        // zaccum = xaccum "+" yaccum
        faccum (zaccum, xaccum, yaccum) ;

        // c = (ctype) zaccum
        cast_zaccum_to_C (c, zaccum, ctype->size) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    #pragma omp flush
    return (GrB_SUCCESS) ;
}

