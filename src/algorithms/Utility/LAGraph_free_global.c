//------------------------------------------------------------------------------
// LAGraph_free_global:  free all global operators and types
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

// LAGraph_free_global:  free all global operators and types for LAGraph.
// Contributed by Tim Davis, Texas A&M.

// This function is not meant to be user-callable.
// TODO move to LAGraph_finalize?  Or rename?

#include "LAGraph_internal.h"

GrB_Info LAGraph_free_global ( )
{

    // free the unary ops
    GrB_free (&LAGraph_ISONE_INT8) ;
    GrB_free (&LAGraph_ISONE_INT16) ;
    GrB_free (&LAGraph_ISONE_INT32) ;
    GrB_free (&LAGraph_ISONE_INT64) ;
    GrB_free (&LAGraph_ISONE_UINT8) ;
    GrB_free (&LAGraph_ISONE_UINT16) ;
    GrB_free (&LAGraph_ISONE_UINT32) ;
    GrB_free (&LAGraph_ISONE_UINT64) ;
    GrB_free (&LAGraph_ISONE_FP32) ;
    GrB_free (&LAGraph_ISONE_FP64) ;
    GrB_free (&LAGraph_ISONE_ComplexFP64) ;
    GrB_free (&LAGraph_ISTWO_UINT32) ;
    GrB_free (&LAGraph_ISTWO_INT64) ;
    GrB_free (&LAGraph_DECR_INT32) ;
    GrB_free (&LAGraph_DECR_INT64) ;
    GrB_free (&LAGraph_TRUE_BOOL) ;
    GrB_free (&LAGraph_TRUE_BOOL_ComplexFP64) ;
    GrB_free (&LAGraph_ONE_FP64) ;
    GrB_free (&LAGraph_ONE_UINT32) ;
    GrB_free (&LAGraph_ONE_INT64) ;
    GrB_free (&LAGraph_COMB_DIR_FP64) ;
    GrB_free (&LAGraph_COMB_UNDIR_FP64) ;
    GrB_free (&LAGraph_GT0_FP32) ;
    GrB_free (&LAGraph_GT0_FP64) ;
    GrB_free (&LAGraph_YMAX_FP32) ;
    GrB_free (&LAGraph_YMAX_FP64) ;

    // free the binary ops
    GrB_free (&LAGraph_EQ_ComplexFP64) ;
    GrB_free (&LAGraph_SKEW_INT8) ;
    GrB_free (&LAGraph_SKEW_INT16) ;
    GrB_free (&LAGraph_SKEW_INT32) ;
    GrB_free (&LAGraph_SKEW_INT64) ;
    GrB_free (&LAGraph_SKEW_FP32) ;
    GrB_free (&LAGraph_SKEW_FP64) ;
    GrB_free (&LAGraph_LOR_UINT32) ;
    GrB_free (&LAGraph_LOR_INT64) ;

    // free the monoids
    GrB_free (&LAGraph_PLUS_INT64_MONOID) ;
    GrB_free (&LAGraph_MAX_INT32_MONOID) ;
    GrB_free (&LAGraph_LAND_MONOID) ;
    GrB_free (&LAGraph_LOR_MONOID) ;
    GrB_free (&LAGraph_MIN_INT32_MONOID) ;
    GrB_free (&LAGraph_MIN_INT64_MONOID) ;
    GrB_free (&LAGraph_PLUS_UINT32_MONOID) ;
    GrB_free (&LAGraph_PLUS_FP64_MONOID) ;
    GrB_free (&LAGraph_PLUS_FP32_MONOID) ;
    GrB_free (&LAGraph_DIV_FP64_MONOID) ;

    // free the semirings
    GrB_free (&LAGraph_LOR_LAND_BOOL) ;
    GrB_free (&LAGraph_LOR_SECOND_BOOL) ;
    GrB_free (&LAGraph_LOR_FIRST_BOOL) ;
    GrB_free (&LAGraph_MIN_SECOND_INT32) ;
    GrB_free (&LAGraph_MIN_FIRST_INT32) ;
    GrB_free (&LAGraph_MIN_SECOND_INT64) ;
    GrB_free (&LAGraph_MIN_FIRST_INT64) ;
    GrB_free (&LAGraph_PLUS_TIMES_UINT32) ;
    GrB_free (&LAGraph_PLUS_TIMES_INT64) ;
    GrB_free (&LAGraph_PLUS_TIMES_FP64) ;
    GrB_free (&LAGraph_PLUS_PLUS_FP64) ;
    GrB_free (&LAGraph_PLUS_TIMES_FP32) ;
    GrB_free (&LAGraph_PLUS_PLUS_FP32) ;

    // free the descriptors
//  GrB_free (&LAGraph_desc_oooo) ;     // NULL, no need to free it
    GrB_free (&LAGraph_desc_ooor) ;
    GrB_free (&LAGraph_desc_ooco) ;
    GrB_free (&LAGraph_desc_oocr) ;
    GrB_free (&LAGraph_desc_otoo) ;
    GrB_free (&LAGraph_desc_otor) ;
    GrB_free (&LAGraph_desc_otco) ;
    GrB_free (&LAGraph_desc_otcr) ;
    GrB_free (&LAGraph_desc_tooo) ;
    GrB_free (&LAGraph_desc_toor) ;
    GrB_free (&LAGraph_desc_toco) ;
    GrB_free (&LAGraph_desc_tocr) ;
    GrB_free (&LAGraph_desc_ttoo) ;
    GrB_free (&LAGraph_desc_ttor) ;
    GrB_free (&LAGraph_desc_ttco) ;
    GrB_free (&LAGraph_desc_ttcr) ;

    // free the select operator for ktruss and allktruss
    GrB_free (&LAGraph_support) ;

    return (GrB_SUCCESS) ;
}

