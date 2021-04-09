//------------------------------------------------------------------------------
// LAGraph_alloc_global:  allocate all global objects for LAGraph
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

// LAGraph_alloc_global:  allocate all global objects for LAGraph.
// Contributed by Tim Davis, Texas A&M.

// Define and allocate global types and operators for LAGraph.  This function
// is not meant to be user-callable.  TODO: move to LAGraph_init?

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL                    \
{                                           \
    LAGraph_free_global ( ) ;               \
}

// binary operators to test for symmetry, skew-symmetry and Hermitian property
GrB_BinaryOp LAGraph_SKEW_INT8    = NULL ;
GrB_BinaryOp LAGraph_SKEW_INT16   = NULL ;
GrB_BinaryOp LAGraph_SKEW_INT32   = NULL ;
GrB_BinaryOp LAGraph_SKEW_INT64   = NULL ;
GrB_BinaryOp LAGraph_SKEW_FP32    = NULL ;
GrB_BinaryOp LAGraph_SKEW_FP64    = NULL ;
GrB_BinaryOp LAGraph_LOR_UINT32   = NULL ;
GrB_BinaryOp LAGraph_LOR_INT64    = NULL ;

void LAGraph_skew_int8
(
    bool *z,
    const int8_t *x,
    const int8_t *y
)
{
    (*z) = (*x) == -(*y) ;
}

void LAGraph_skew_int16
(
    bool *z,
    const int16_t *x,
    const int16_t *y
)
{
    (*z) = (*x) == -(*y) ;
}

void LAGraph_skew_int32
(
    bool *z,
    const int32_t *x,
    const int32_t *y
)
{
    (*z) = (*x) == -(*y) ;
}

void LAGraph_skew_int64
(
    bool *z,
    const int64_t *x,
    const int64_t *y
)
{
    (*z) = (*x) == -(*y) ;
}

void LAGraph_skew_float
(
    bool *z,
    const float *x,
    const float *y
)
{
    (*z) = (*x) == -(*y) ;
}

void LAGraph_skew_double
(
    bool *z,
    const double *x,
    const double *y
)
{
    (*z) = (*x) == -(*y) ;
}

void LAGraph_lor_uint32
(
    uint32_t *z,
    const uint32_t *x,
    const uint32_t *y
)
{
    (*z) = (((*x) != 0) || ((*y) != 0)) ;
}

void LAGraph_lor_int64
(
    int64_t *z,
    const int64_t *x,
    const int64_t *y
)
{
    (*z) = (((*x) != 0) || ((*y) != 0)) ;
}

// unary operators to check if the entry is equal to 1
GrB_UnaryOp LAGraph_ISONE_INT8     = NULL ;
GrB_UnaryOp LAGraph_ISONE_INT16    = NULL ;
GrB_UnaryOp LAGraph_ISONE_INT32    = NULL ;
GrB_UnaryOp LAGraph_ISONE_INT64    = NULL ;
GrB_UnaryOp LAGraph_ISONE_UINT8    = NULL ;
GrB_UnaryOp LAGraph_ISONE_UINT16   = NULL ;
GrB_UnaryOp LAGraph_ISONE_UINT32   = NULL ;
GrB_UnaryOp LAGraph_ISONE_UINT64   = NULL ;
GrB_UnaryOp LAGraph_ISONE_FP32     = NULL ;
GrB_UnaryOp LAGraph_ISONE_FP64     = NULL ;

void LAGraph_isone_int8
(
    bool *z,
    const int8_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_int16
(
    bool *z,
    const int16_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_int32
(
    bool *z,
    const int32_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_int64
(
    bool *z,
    const int64_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_uint8
(
    bool *z,
    const uint8_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_uint16
(
    bool *z,
    const uint16_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_uint32
(
    bool *z,
    const uint32_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_uint64
(
    bool *z,
    const uint64_t *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_float
(
    bool *z,
    const float *x
)
{
    (*z) = ((*x) == 1) ;
}

void LAGraph_isone_double
(
    bool *z,
    const double *x
)
{
    (*z) = ((*x) == 1) ;
}

// unary operator to check if the entry is equal to 2
GrB_UnaryOp LAGraph_ISTWO_UINT32 = NULL ;
GrB_UnaryOp LAGraph_ISTWO_INT64 = NULL ;

void LAGraph_istwo_uint32
(
    bool *z,
    const uint32_t *x
)
{
    (*z) = ((*x) == 2) ;
}

void LAGraph_istwo_int64
(
    bool *z,
    const int64_t *x
)
{
    (*z) = ((*x) == 2) ;
}

// unary operators that return boolean true
GrB_UnaryOp LAGraph_TRUE_BOOL = NULL ;

void LAGraph_true_bool
(
    bool *z,
    const bool *x       // ignored
)
{
    (*z) = true ;
}

// unary operators that return 1
GrB_UnaryOp LAGraph_ONE_UINT32 = NULL ;
GrB_UnaryOp LAGraph_ONE_FP64 = NULL ;
GrB_UnaryOp LAGraph_ONE_INT64 = NULL ;

void LAGraph_one_uint32
(
    uint32_t *z,
    const uint32_t *x       // ignored
)
{
    (*z) = 1 ;
}

void LAGraph_one_int64
(
    int64_t *z,
    const int64_t *x       // ignored
)
{
    (*z) = 1 ;
}

void LAGraph_one_fp64
(
    double *z,
    const double *x       // ignored
)
{
    (*z) = 1 ;
}

// unary ops to check if greater than zero
GrB_UnaryOp LAGraph_GT0_FP32 = NULL ;
GrB_UnaryOp LAGraph_GT0_FP64 = NULL ;

void LAGraph_gt0_fp32
(
    bool *z,
    const float *x
)
{
    (*z) = ((*x) > 0) ;
}

void LAGraph_gt0_fp64
(
    bool *z,
    const double *x
)
{
    (*z) = ((*x) > 0) ;
}

// unary operators to threshold a max value for DNN
GrB_UnaryOp LAGraph_YMAX_FP32 = NULL ;
GrB_UnaryOp LAGraph_YMAX_FP64 = NULL ;

void LAGraph_ymax_fp32
(
    float *z,
    const float *x
)
{
    (*z) = fminf ((*x), (float) 32.0) ;
}

void LAGraph_ymax_fp64
(
    double *z,
    const double *x
)
{
    (*z) = fmin ((*x), (double) 32.0) ;
}


// integer decrement
GrB_UnaryOp LAGraph_DECR_INT32 = NULL ;
GrB_UnaryOp LAGraph_DECR_INT64 = NULL ;

void LAGraph_decr_int32
(
    int32_t *z,
    const int32_t *x
)
{
    (*z) = ((*x) - 1) ;
}

void LAGraph_decr_int64
(
    int64_t *z,
    const int64_t *x
)
{
    (*z) = ((*x) - 1) ;
}

// z = x * (x - 1), used by LAGraph_lcc.
// This operator calculates the 2-permutation of d(v).
void LAGraph_comb_dir_fp64
(
    void *z,
    const void *x
)
{
    double xd = *(double *) x ;
    double *zd = (double *) z ;
    (*zd) = ((xd) * (xd - 1)) ;
}

// z = x * (x - 1) / 2, used by LAGraph_lcc.
// This operator calculates the 2-combination of d(v).
void LAGraph_comb_undir_fp64
(
    void *z,
    const void *x
)
{
    double xd = *(double *) x ;
    double *zd = (double *) z ;
    (*zd) = ((xd) * (xd - 1)) / 2;
}


GrB_UnaryOp LAGraph_COMB_DIR_FP64 = NULL ;
GrB_UnaryOp LAGraph_COMB_UNDIR_FP64 = NULL ;

// monoids
GrB_Monoid LAGraph_PLUS_INT64_MONOID = NULL ;
GrB_Monoid LAGraph_MAX_INT32_MONOID = NULL ;
GrB_Monoid LAGraph_LAND_MONOID = NULL ;
GrB_Monoid LAGraph_LOR_MONOID = NULL ;
GrB_Monoid LAGraph_MIN_INT32_MONOID = NULL ;
GrB_Monoid LAGraph_MIN_INT64_MONOID = NULL ;
GrB_Monoid LAGraph_PLUS_UINT32_MONOID = NULL ;
GrB_Monoid LAGraph_PLUS_FP64_MONOID = NULL ;
GrB_Monoid LAGraph_PLUS_FP32_MONOID = NULL ;
GrB_Monoid LAGraph_DIV_FP64_MONOID = NULL ;

// semirings
GrB_Semiring LAGraph_LOR_LAND_BOOL = NULL ;
GrB_Semiring LAGraph_LOR_SECOND_BOOL = NULL ;
GrB_Semiring LAGraph_LOR_FIRST_BOOL = NULL ;
GrB_Semiring LAGraph_MIN_SECOND_INT32 = NULL ;
GrB_Semiring LAGraph_MIN_FIRST_INT32 = NULL ;
GrB_Semiring LAGraph_MIN_SECOND_INT64 = NULL ;
GrB_Semiring LAGraph_MIN_FIRST_INT64 = NULL ;
GrB_Semiring LAGraph_PLUS_TIMES_UINT32 = NULL ;
GrB_Semiring LAGraph_PLUS_TIMES_INT64 = NULL ;
GrB_Semiring LAGraph_PLUS_TIMES_FP64 = NULL ;
GrB_Semiring LAGraph_PLUS_PLUS_FP64 = NULL ;
GrB_Semiring LAGraph_PLUS_TIMES_FP32 = NULL ;
GrB_Semiring LAGraph_PLUS_PLUS_FP32 = NULL ;

// all 16 descriptors
// syntax: 4 characters define the following.  'o' is the default:
// 1: o or t: A transpose
// 2: o or t: B transpose
// 3: o or c: complemented mask
// 4: o or r: replace
GrB_Descriptor

    LAGraph_desc_oooo = NULL ,   // default (NULL)
    LAGraph_desc_ooor = NULL ,   // replace
    LAGraph_desc_ooco = NULL ,   // compl mask
    LAGraph_desc_oocr = NULL ,   // compl mask, replace

    LAGraph_desc_tooo = NULL ,   // A'
    LAGraph_desc_toor = NULL ,   // A', replace
    LAGraph_desc_toco = NULL ,   // A', compl mask
    LAGraph_desc_tocr = NULL ,   // A', compl mask, replace

    LAGraph_desc_otoo = NULL ,   // B'
    LAGraph_desc_otor = NULL ,   // B', replace
    LAGraph_desc_otco = NULL ,   // B', compl mask
    LAGraph_desc_otcr = NULL ,   // B', compl mask, replace

    LAGraph_desc_ttoo = NULL ,   // A', B'
    LAGraph_desc_ttor = NULL ,   // A', B', replace
    LAGraph_desc_ttco = NULL ,   // A', B', compl mask
    LAGraph_desc_ttcr = NULL ;   // A', B', compl mask, replace

//------------------------------------------------------------------------------
// LAGraph_support_function:  select function for GxB_SelectOp and GxB_select
//------------------------------------------------------------------------------

#if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
    && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,0,1) )
// requires SuiteSparse:GraphBLAS v3.0.1
GxB_SelectOp LAGraph_support = NULL ;
#endif

bool LAGraph_support_function (const GrB_Index i, const GrB_Index j, const uint32_t *x, const uint32_t *support) ;

bool LAGraph_support_function (const GrB_Index i, const GrB_Index j, const uint32_t *x, const uint32_t *support)
{
    return ((*x) >= (*support)) ;
}

//------------------------------------------------------------------------------
// LAGraph_alloc_global
//------------------------------------------------------------------------------

#define F_BINARY(f) ((void (*)(void *, const void *, const void *)) f)
#define F_UNARY(f)  ((void (*)(void *, const void *)) f)
#define F_SELECT(f) ((bool (*)(const GrB_Index, const GrB_Index, const void *, const void *)) f)

GrB_Info LAGraph_alloc_global ( )
{
    GrB_Info info ;

    //--------------------------------------------------------------------------
    // create the binary operators
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_SKEW_INT8,
        F_BINARY (LAGraph_skew_int8),
        GrB_BOOL, GrB_INT8, GrB_INT8)) ;

    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_SKEW_INT16,
        F_BINARY (LAGraph_skew_int16),
        GrB_BOOL, GrB_INT16, GrB_INT16)) ;

    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_SKEW_INT32,
        F_BINARY (LAGraph_skew_int32),
        GrB_BOOL, GrB_INT32, GrB_INT32)) ;

    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_SKEW_INT64,
        F_BINARY (LAGraph_skew_int64),
        GrB_BOOL, GrB_INT64, GrB_INT64)) ;

    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_SKEW_FP32,
        F_BINARY (LAGraph_skew_float),
        GrB_BOOL, GrB_FP32, GrB_FP32)) ;

    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_SKEW_FP64,
        F_BINARY (LAGraph_skew_double),
        GrB_BOOL, GrB_FP64, GrB_FP64)) ;

    #ifdef GxB_SUITESPARSE_GRAPHBLAS
    // use the built-in binary operator
    LAGraph_LOR_UINT32 = GxB_LOR_UINT32 ;
    LAGraph_LOR_INT64  = GxB_LOR_INT64  ;
    #else

    // create a new built-in binary operator using LAGraph_lor_uint32
    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_LOR_UINT32,
        F_BINARY (LAGraph_lor_uint32),
        GrB_UINT32, GrB_UINT32, GrB_UINT32)) ;

    // create a new built-in binary operator using LAGraph_lor_int64
    LAGRAPH_OK (GrB_BinaryOp_new (&LAGraph_LOR_INT64,
        F_BINARY (LAGraph_lor_int64),
        GrB_INT64, GrB_INT64, GrB_INT64)) ;

    #endif

    //--------------------------------------------------------------------------
    // create the unary operators that check if equal to 1
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_INT8,
        F_UNARY (LAGraph_isone_int8),
        GrB_BOOL, GrB_INT8)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_INT16,
        F_UNARY (LAGraph_isone_int16),
        GrB_BOOL, GrB_INT16)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_INT32,
        F_UNARY (LAGraph_isone_int32),
        GrB_BOOL, GrB_INT32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_INT64,
        F_UNARY (LAGraph_isone_int64),
        GrB_BOOL, GrB_INT64)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_UINT8,
        F_UNARY (LAGraph_isone_uint8),
        GrB_BOOL, GrB_UINT8)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_UINT16,
        F_UNARY (LAGraph_isone_uint16),
        GrB_BOOL, GrB_UINT16)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_UINT32,
        F_UNARY (LAGraph_isone_uint32),
        GrB_BOOL, GrB_UINT32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_UINT64,
        F_UNARY (LAGraph_isone_uint64),
        GrB_BOOL, GrB_UINT64)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_FP32,
        F_UNARY (LAGraph_isone_float),
        GrB_BOOL, GrB_FP32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISONE_FP64,
        F_UNARY (LAGraph_isone_double),
        GrB_BOOL, GrB_FP64)) ;

    //--------------------------------------------------------------------------
    // create the unary operator that checks if equal to 2
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISTWO_UINT32,
        F_UNARY (LAGraph_istwo_uint32),
        GrB_BOOL, GrB_UINT32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ISTWO_INT64,
        F_UNARY (LAGraph_istwo_int64),
        GrB_BOOL, GrB_INT64)) ;

    //--------------------------------------------------------------------------
    // create the unary decrement operators
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_DECR_INT32,
        F_UNARY (LAGraph_decr_int32),
        GrB_INT32, GrB_INT32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_DECR_INT64,
        F_UNARY (LAGraph_decr_int64),
        GrB_INT64, GrB_INT64)) ;

    //--------------------------------------------------------------------------
    // create the unary greater-than-zero operators
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_GT0_FP32,
        F_UNARY (LAGraph_gt0_fp32),
        GrB_BOOL, GrB_FP32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_GT0_FP64,
        F_UNARY (LAGraph_gt0_fp64),
        GrB_BOOL, GrB_FP64)) ;

    //--------------------------------------------------------------------------
    // create the unary YMAX operators
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_YMAX_FP32,
        F_UNARY (LAGraph_ymax_fp32),
        GrB_FP32, GrB_FP32)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_YMAX_FP64,
        F_UNARY (LAGraph_ymax_fp64),
        GrB_FP64, GrB_FP64)) ;

    //--------------------------------------------------------------------------
    // create the unary operators that return true
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_TRUE_BOOL,
        F_UNARY (LAGraph_true_bool),
        GrB_BOOL, GrB_BOOL)) ;

    #ifdef GxB_SUITESPARSE_GRAPHBLAS
    // use the built-in unary operator
    LAGraph_ONE_INT64  = GxB_ONE_INT64 ;
    LAGraph_ONE_UINT32 = GxB_ONE_UINT32 ;
    LAGraph_ONE_FP64   = GxB_ONE_FP64 ;
    #else

    // create a new built-in unary operator using LAGraph_one_uint32
    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ONE_UINT32,
        F_UNARY (LAGraph_one_uint32),
        GrB_UINT32, GrB_UINT32)) ;

    // create a new built-in unary operator using LAGraph_one_int64
    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ONE_INT64,
        F_UNARY (LAGraph_one_int64),
        GrB_INT64, GrB_INT64)) ;

    // create a new built-in unary operator using LAGraph_one_fp64
    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_ONE_FP64,
        F_UNARY (LAGraph_one_fp64),
        GrB_FP64, GrB_FP64)) ;

    #endif

    //--------------------------------------------------------------------------
    // create the operators for LAGraph_lcc
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_COMB_DIR_FP64,
        F_UNARY (LAGraph_comb_dir_fp64),
        GrB_FP64, GrB_FP64)) ;

    LAGRAPH_OK (GrB_UnaryOp_new (&LAGraph_COMB_UNDIR_FP64,
        F_UNARY (LAGraph_comb_undir_fp64),
        GrB_FP64, GrB_FP64)) ;

    //--------------------------------------------------------------------------
    // create the monoids
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_Monoid_new_INT64 (&LAGraph_PLUS_INT64_MONOID,
        GrB_PLUS_INT64, 0)) ;

    LAGRAPH_OK (GrB_Monoid_new_INT32 (&LAGraph_MAX_INT32_MONOID,
        GrB_MAX_INT32, INT32_MIN)) ;

    LAGRAPH_OK (GrB_Monoid_new_INT32 (&LAGraph_MIN_INT32_MONOID,
        GrB_MIN_INT32, INT32_MAX)) ;

    LAGRAPH_OK (GrB_Monoid_new_INT64 (&LAGraph_MIN_INT64_MONOID,
        GrB_MIN_INT64, INT64_MAX)) ;

    LAGRAPH_OK (GrB_Monoid_new_BOOL (&LAGraph_LAND_MONOID, GrB_LAND, true )) ;

    LAGRAPH_OK (GrB_Monoid_new_BOOL (&LAGraph_LOR_MONOID , GrB_LOR , false)) ;

    LAGRAPH_OK (GrB_Monoid_new_UINT32 (&LAGraph_PLUS_UINT32_MONOID,
        GrB_PLUS_UINT32, 0)) ;

    LAGRAPH_OK (GrB_Monoid_new_FP64 (&LAGraph_PLUS_FP64_MONOID,
        GrB_PLUS_FP64, (double) 0)) ;

    LAGRAPH_OK (GrB_Monoid_new_FP32 (&LAGraph_PLUS_FP32_MONOID,
        GrB_PLUS_FP32, (float) 0)) ;

    LAGRAPH_OK (GrB_Monoid_new_FP64 (&LAGraph_DIV_FP64_MONOID,
        GrB_DIV_FP64, (double) 1.0)) ;

    //--------------------------------------------------------------------------
    // create the semirings
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_LOR_LAND_BOOL,
        LAGraph_LOR_MONOID, GrB_LAND)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_LOR_FIRST_BOOL,
        LAGraph_LOR_MONOID, GrB_FIRST_BOOL)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_LOR_SECOND_BOOL,
        LAGraph_LOR_MONOID, GrB_SECOND_BOOL)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_MIN_SECOND_INT32,
        LAGraph_MIN_INT32_MONOID, GrB_SECOND_INT32)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_MIN_FIRST_INT32,
        LAGraph_MIN_INT32_MONOID, GrB_FIRST_INT32)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_MIN_SECOND_INT64,
        LAGraph_MIN_INT64_MONOID, GrB_SECOND_INT64)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_MIN_FIRST_INT64,
        LAGraph_MIN_INT64_MONOID, GrB_FIRST_INT64)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_PLUS_TIMES_UINT32,
        LAGraph_PLUS_UINT32_MONOID, GrB_TIMES_UINT32)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_PLUS_TIMES_INT64,
        LAGraph_PLUS_INT64_MONOID, GrB_TIMES_INT64)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_PLUS_TIMES_FP32,
        LAGraph_PLUS_FP32_MONOID, GrB_TIMES_FP32)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_PLUS_PLUS_FP32,
        LAGraph_PLUS_FP32_MONOID, GrB_PLUS_FP32)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_PLUS_TIMES_FP64,
        LAGraph_PLUS_FP64_MONOID, GrB_TIMES_FP64)) ;

    LAGRAPH_OK (GrB_Semiring_new (&LAGraph_PLUS_PLUS_FP64,
        LAGraph_PLUS_FP64_MONOID, GrB_PLUS_FP64)) ;

    //--------------------------------------------------------------------------
    // create 15 descriptors (one does not need to be allocated)
    //--------------------------------------------------------------------------

    LAGraph_desc_oooo = NULL ;   // default (NULL)
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_ooor)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_ooco)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_oocr)) ;

    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_otoo)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_otor)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_otco)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_otcr)) ;

    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_tooo)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_toor)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_toco)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_tocr)) ;

    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_ttoo)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_ttor)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_ttco)) ;
    LAGRAPH_OK (GrB_Descriptor_new (&LAGraph_desc_ttcr)) ;

    //--------------------------------------------------------------------------
    // set the descriptors
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ooor, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_oocr, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otor, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otcr, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_toor, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_tocr, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttor, GrB_OUTP, GrB_REPLACE)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttcr, GrB_OUTP, GrB_REPLACE)) ;

    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ooco, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_oocr, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otco, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otcr, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_toco, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_tocr, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttco, GrB_MASK, GrB_SCMP)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttcr, GrB_MASK, GrB_SCMP)) ;

    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otoo, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otor, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otco, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_otcr, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttoo, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttor, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttco, GrB_INP1, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttcr, GrB_INP1, GrB_TRAN)) ;

    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_tooo, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_toor, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_toco, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_tocr, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttoo, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttor, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttco, GrB_INP0, GrB_TRAN)) ;
    LAGRAPH_OK (GrB_Descriptor_set (LAGraph_desc_ttcr, GrB_INP0, GrB_TRAN)) ;

    //--------------------------------------------------------------------------
    // allocate the select function for ktruss and allktruss
    //--------------------------------------------------------------------------

    #if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
        && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,0,1) )
    // Note the added parameter (SuiteSparse:GraphBLAS, July 19, V3.0.1 draft)
    LAGRAPH_OK (GxB_SelectOp_new (&LAGraph_support,
        F_SELECT (LAGraph_support_function), GrB_UINT32, GrB_UINT32)) ;
    #endif

    return (GrB_SUCCESS) ;
}

