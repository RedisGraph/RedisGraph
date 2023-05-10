//------------------------------------------------------------------------------
// GB_mex_about9: still more basic tests
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"
#include "GB_mex_errors.h"

#define USAGE "GB_mex_about10"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info, expected ;
    GrB_Matrix A = NULL ;
    GrB_Vector v = NULL ;
    int64_t pool1 [64], pool2 [64] ;

    //--------------------------------------------------------------------------
    // startup GraphBLAS
    //--------------------------------------------------------------------------

    GrB_Descriptor desc = NULL ;
    bool malloc_debug = GB_mx_get_global (true) ;

    //--------------------------------------------------------------------------
    // get/set tests
    //--------------------------------------------------------------------------

    double chunk = 0 ;
    expected = GrB_INVALID_VALUE ;
    ERR (GxB_Desc_set_INT32 (GrB_DESC_ST0, GrB_OUTP, GrB_REPLACE)) ;
    ERR (GxB_Desc_set_FP64 (GrB_DESC_ST0, GxB_DESCRIPTOR_CHUNK, 1e6)) ;
    ERR (GxB_Global_Option_set_FP64 (-1, 0)) ;
    ERR (GxB_Global_Option_set_FP64_ARRAY (-1, NULL)) ;
    ERR (GxB_Global_Option_set_INT64_ARRAY (-1, NULL)) ;
    ERR (GxB_Global_Option_set_FUNCTION (-1, NULL)) ;
    ERR (GxB_Desc_get_FP64 (NULL, -1, &chunk)) ;

    int value = -1 ;
    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_Desc_get_INT32 (desc, GxB_IMPORT, &value)) ;
    CHECK (value == GxB_DEFAULT) ;
    OK (GxB_Desc_set_INT32 (desc, GxB_IMPORT, GxB_SECURE_IMPORT)) ;
    OK (GxB_Desc_get_INT32 (desc, GxB_IMPORT, &value)) ;
    CHECK (value == GxB_SECURE_IMPORT) ;

    OK (GxB_Desc_set_FP64 (desc, GxB_DESCRIPTOR_CHUNK, 1e6)) ;
    OK (GxB_Desc_get_FP64 (desc, GxB_DESCRIPTOR_CHUNK, &chunk)) ;
    CHECK (chunk = 1e6) ;

    OK (GxB_Global_Option_set_FP64 (GxB_GLOBAL_CHUNK, 2e6)) ;
    OK (GxB_Global_Option_get_FP64 (GxB_GLOBAL_CHUNK, &chunk)) ;
    CHECK (chunk = 2e6) ;

    int32_t ver [3] ;
    char *compiler ;
    OK (GxB_Global_Option_get_INT32 (GxB_COMPILER_VERSION, ver)) ;
    OK (GxB_Global_Option_get_CHAR (GxB_COMPILER_NAME, &compiler)) ;
    printf ("compiler: %s %d.%d.%d\n", compiler, ver [0], ver [1], ver [2]) ;

    OK (GxB_Global_Option_set_INT32 (GxB_BURBLE, 1)) ;
    OK (GrB_Matrix_new (&A, GrB_FP64, 10, 10)) ;
    OK (GrB_transpose (A, NULL, NULL, A, desc)) ;
    OK (GxB_Global_Option_set_INT32 (GxB_BURBLE, 0)) ;
    OK (GrB_transpose (A, NULL, NULL, A, desc)) ;

    OK (GxB_Matrix_Option_get_INT32 (A, GxB_SPARSITY_STATUS, &value)) ;
    CHECK (value == GxB_HYPERSPARSE) ;

    double bswitch1 = 0.5, bswitch2 = 1.0 ;
    OK (GxB_Matrix_Option_set (A, GxB_BITMAP_SWITCH, bswitch1)) ;
    OK (GxB_Matrix_Option_get (A, GxB_BITMAP_SWITCH, &bswitch2)) ;
    CHECK (bswitch1 == bswitch2) ;

    for (int k = 0 ; k < 64 ; k++)
    {
        pool1 [k] = (k < 3) ? 0 : k ;
    }
    OK (GxB_Global_Option_set_INT64_ARRAY (GxB_MEMORY_POOL, pool1)) ;
    OK (GxB_Global_Option_get_INT64 (GxB_MEMORY_POOL, pool2)) ;
    for (int k = 0 ; k < 64 ; k++)
    {
        CHECK (pool1 [k] == pool2 [k]) ;
    }

    ERR (GxB_Matrix_Option_set_FP64 (A, -1, 0)) ;

    OK (GrB_Vector_new (&v, GrB_FP64, 10)) ;
    ERR (GxB_Vector_Option_set_FP64 (v, -1, 0)) ;
    ERR (GxB_Vector_Option_set_FP64 (v, -1, 0)) ;
    ERR (GxB_Vector_Option_get_FP64 (v, -1, &chunk)) ;
    ERR (GxB_Vector_Option_set_INT32 (v, -1, 0)) ;

    OK (GrB_Descriptor_free (&desc)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Vector_free (&v)) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    GB_mx_put_global (true) ;
    printf ("\nGB_mex_about10: all tests passed\n\n") ;
}

