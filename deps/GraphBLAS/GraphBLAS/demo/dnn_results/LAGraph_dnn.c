//------------------------------------------------------------------------------
// LAGraph_dnn: sparse deep neural network
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2020 LAGraph Contributors.

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

// LAGraph_dnn: sparse deep neural network.  Contributed by Tim Davis,
// Texas A&M University.  Based on inferenceReLUvec.m by Jeremy Kepner, MIT.

// Performs ReLU inference using input feature vectors Y0.

// See http://graphchallenge.org/ for a description of the algorithm.

// On input, Y0 is the initial feature vectors, of size nfeatures-by-nneurons.
// This format uses the graph convention that A(i,j) is the edge (i,j),
// Each row of Y0 is a single feature.

// W is an array of size nlayers of sparse matrices.  Each W[layer] matrix has
// the same size: nneurons-by-nneurons.  W[layer] represents the DNN weights
// for that layer.

// The Bias[layer] matrices are diagaonal, and the same size as W[layer].

// All matrices must have the same type:  either GrB_FP32 or GrB_FP64.

// On output, Y is the computed result, of the same size and type as Y0.

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (&M) ;         \
    GrB_free (Yhandle) ;    \
}

GrB_Info LAGraph_dnn    // returns GrB_SUCCESS if successful
(
    // output
    GrB_Matrix *Yhandle,    // Y, created on output
    // input: not modified
    GrB_Matrix *W,      // W [0..nlayers-1], each nneurons-by-nneurons
    GrB_Matrix *Bias,   // Bias [0..nlayers-1], diagonal nneurons-by-nneurons
    int nlayers,        // # of layers
    GrB_Matrix Y0       // input features: nfeatures-by-nneurons
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    if (Yhandle == NULL || W == NULL || Bias == NULL || Y0 == NULL)
    {
        return (GrB_NULL_POINTER) ;
    }

    GrB_Matrix Y = NULL ;
    GrB_Matrix M = NULL ;
    (*Yhandle) = NULL ;

    GrB_Type type ;
    LAGRAPH_OK (GxB_Matrix_type (&type, Y0)) ;

    GrB_Semiring plus_times, plus_plus ;
    GrB_UnaryOp gt0, id, ymax ;

    if (type == GrB_FP32)
    {
        plus_times = LAGraph_PLUS_TIMES_FP32 ;
        plus_plus  = LAGraph_PLUS_PLUS_FP32 ;
        gt0        = LAGraph_GT0_FP32 ;
        ymax       = LAGraph_YMAX_FP32 ;
        id         = GrB_IDENTITY_FP32 ;
    }
    else if (type == GrB_FP64)
    {
        plus_times = LAGraph_PLUS_TIMES_FP64 ;
        plus_plus  = LAGraph_PLUS_PLUS_FP64 ;
        gt0        = LAGraph_GT0_FP64 ;
        ymax       = LAGraph_YMAX_FP64 ;
        id         = GrB_IDENTITY_FP64 ;
    }
    else
    {
        return (GrB_DOMAIN_MISMATCH) ;
    }

    for (int layer = 0 ; layer < nlayers ; layer++)
    {
        GrB_Type type2 ;
        LAGRAPH_OK (GxB_Matrix_type (&type2, W [layer])) ;
        if (type != type2) return (GrB_DOMAIN_MISMATCH) ;
        LAGRAPH_OK (GxB_Matrix_type (&type2, Bias [layer])) ;
        if (type != type2) return (GrB_DOMAIN_MISMATCH) ;
    }

    //--------------------------------------------------------------------------
    // create the output matrix Y
    //--------------------------------------------------------------------------

    GrB_Index nfeatures, nneurons ;
    LAGRAPH_OK (GrB_Matrix_nrows (&nfeatures, Y0)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&nneurons,  Y0)) ;
    LAGRAPH_OK (GrB_Matrix_new (&Y, type, nfeatures, nneurons)) ;
    LAGRAPH_OK (GrB_Matrix_new (&M, GrB_BOOL, nfeatures, nneurons)) ;

    //--------------------------------------------------------------------------
    // propagate the features through the neuron layers
    //--------------------------------------------------------------------------

    // double t1 = 0, t2 = 0, t3 = 0, t4 = 0, t ;

    for (int layer = 0 ; layer < nlayers ; layer++)
    {
        // Y = Y * W [layer], using the conventional PLUS_TIMES semiring
        // t = omp_get_wtime ( ) ;
        LAGRAPH_OK (GrB_mxm (Y, NULL, NULL, plus_times,
            ((layer == 0) ? Y0 : Y), W [layer], NULL)) ;
        // t1 += (omp_get_wtime ( ) - t) ;

        // Y = Y * Bias [layer], using the PLUS_PLUS semiring.  This computes
        // Y(i,j) += Bias [layer] (j,j) for each entry Y(i,j).  It does not
        // introduce any new entries in Y.
        // t = omp_get_wtime ( ) ;
        LAGRAPH_OK (GrB_mxm (Y, NULL, NULL, plus_plus, Y, Bias [layer], NULL)) ;
        // t2 += (omp_get_wtime ( ) - t) ;

        // delete entries from Y: keep only those entries greater than zero
        #if defined ( GxB_SUITESPARSE_GRAPHBLAS ) \
            && ( GxB_IMPLEMENTATION >= GxB_VERSION (3,0,0) )
        // using SuiteSparse:GraphBLAS 3.0.0 or later.
        // t = omp_get_wtime ( ) ;
        LAGRAPH_OK (GxB_select (Y, NULL, NULL, GxB_GT_ZERO, Y, NULL, NULL)) ;
        // t3 += (omp_get_wtime ( ) - t) ;

        #else
        // using SuiteSparse v2.x or earlier, or any other GraphBLAS library.
        LAGRAPH_OK (GrB_apply (M, NULL, NULL, gt0, Y, NULL)) ;
        LAGRAPH_OK (GrB_apply (Y, M, NULL, id, Y, LAGraph_desc_ooor)) ;
        #endif

        // threshold maximum values: Y (Y > 32) = 32
        // t = omp_get_wtime ( ) ;
        LAGRAPH_OK (GrB_apply (Y, NULL, NULL, ymax, Y, NULL)) ;
        // t4 += (omp_get_wtime ( ) - t) ;
    }

    // printf ("\nY*W: %g sec\n", t1) ;
    // printf ("Y+B: %g sec\n", t2) ;
    // printf ("RelU %g sec\n", t3) ;
    // printf ("Ymax %g sec\n", t4) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GrB_free (&M) ;
    (*Yhandle) = Y ;
    return (GrB_SUCCESS) ;
}

