//------------------------------------------------------------------------------
// LAGraph_Vector_to_dense: convert a vector to dense
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_Vector_to_dense: convert a vector to dense, contributed by Tim
// Davis, Texas A&M

// vdense is a dense copy of v.  Where v(i) exists, vdense(i) = v(i).  If v(i)
// is not in the pattern, it is assigned the value v(i) = id.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL \
    GrB_free (&u) ;

GrB_Info LAGraph_Vector_to_dense
(
    GrB_Vector *vdense,     // output vector
    GrB_Vector v,           // input vector
    void *id                // pointer to value to fill vdense with
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Vector u = NULL ;

    if (vdense == NULL || id == NULL)
    {
        // TODO decide on a consistent error-handling mechanism
        LAGRAPH_ERROR ("arguments missing", GrB_NULL_POINTER) ;
    }
    (*vdense) = NULL ;

    // GxB_fprint (v, 2, stderr) ;

    //--------------------------------------------------------------------------
    // get the type and size of v
    //--------------------------------------------------------------------------

    GrB_Type type ;
    GrB_Index n, nvals ;
    LAGRAPH_OK (GxB_Vector_type  (&type,  v)) ;
    LAGRAPH_OK (GrB_Vector_size  (&n,     v)) ;
    LAGRAPH_OK (GrB_Vector_nvals (&nvals, v)) ;

    // u = empty vector, the same type and size as v
    LAGRAPH_OK (GrB_Vector_new (&u, type, n)) ;

    //--------------------------------------------------------------------------
    // create u as a dense vector, all entries equal to (*id)
    //--------------------------------------------------------------------------

    // define macro to fill u with all entries equal to (*id)
    #define FILL(ctype,gtype,arg)                                           \
    {                                                                       \
        /* cast the void pointer to the right type and dereference it */    \
        ctype value = (* (ctype *) id) ;                                    \
        for (GrB_Index i = 0 ; i < n ; i++)                                 \
        {                                                                   \
            /* u (i) = value */                                             \
            LAGRAPH_OK (GrB_Vector_setElement_ ## gtype (u, arg, i)) ;      \
        }                                                                   \
        id_is_zero = (value == 0) ;                                         \
    }

    bool id_is_zero ;
    if      (type == GrB_BOOL        ) FILL (bool          , BOOL  , value)
    else if (type == GrB_INT8        ) FILL (int8_t        , INT8  , value)
    else if (type == GrB_INT16       ) FILL (int16_t       , INT16 , value)
    else if (type == GrB_INT32       ) FILL (int32_t       , INT32 , value)
    else if (type == GrB_INT64       ) FILL (int64_t       , INT64 , value)
    else if (type == GrB_UINT8       ) FILL (uint8_t       , UINT8 , value)
    else if (type == GrB_UINT16      ) FILL (uint16_t      , UINT16, value)
    else if (type == GrB_UINT32      ) FILL (uint32_t      , UINT32, value)
    else if (type == GrB_UINT64      ) FILL (uint64_t      , UINT64, value)
    else if (type == GrB_FP32        ) FILL (float         , FP32  , value)
    else if (type == GrB_FP64        ) FILL (double        , FP64  , value)
    else if (type == LAGraph_ComplexFP64 ) FILL (double complex, UDT   , id   )
    else
    {
        LAGRAPH_ERROR ("type not supported", GrB_INVALID_VALUE) ;
    }

    // finish the vector
    GrB_Index ignore ;
    LAGRAPH_OK (GrB_Vector_nvals (&ignore, u)) ;

    //--------------------------------------------------------------------------
    // scatter the sparse vector v into u
    //--------------------------------------------------------------------------

    if (nvals > 0)
    {
        if (id_is_zero && type != LAGraph_ComplexFP64)
        {
            // use v itself for the mask for built-in types, if (*id) is zero
            // u (v != 0) = v
            LAGRAPH_OK (GrB_assign (u, v, NULL, v, GrB_ALL, n, NULL)) ;
        }
        else
        {
            // TODO: not yet written
            // get the pattern of v
            // u (pattern) = v
            LAGRAPH_ERROR ("TODO", GrB_INVALID_VALUE) ;
        }
    }

    // GxB_fprint (u, 2, stderr) ;

    // finish the assignment
    LAGRAPH_OK (GrB_Vector_nvals (&ignore, u)) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*vdense) = u ;
    return (GrB_SUCCESS) ;
}

