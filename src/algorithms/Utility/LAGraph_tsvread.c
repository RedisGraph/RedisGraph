//------------------------------------------------------------------------------
// LAGraph_tsvread: read a tsv file
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

// LAGraph_tsvread: read a tsv file.  Contributed by Tim Davis, Texas A&M
// University.

// Reads a tsv file.  Each line in the file specifies a single entry: i, j, x.
// The indices i and j are assumed to be one-based.  The dimensions of the
// matrix must be provided by the caller.  This format is used for matrices at
// http://graphchallenge.org.  The Matrix Market format is recommended instead;
// it is more flexible and easier to use, since that format includes the matrix
// type and size in the file itself.  See LAGraph_mmread and LAGraph_mmwrite.

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL GrB_free (Chandle) ;

GrB_Info LAGraph_tsvread        // returns GrB_SUCCESS if successful
(
    GrB_Matrix *Chandle,        // C, created on output
    FILE *f,                    // file to read from (already open)
    GrB_Type type,              // the type of C to create
    GrB_Index nrows,            // C is nrows-by-ncols
    GrB_Index ncols
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (Chandle == NULL || f == NULL)
    {
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // create the output matrix
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix C = NULL ;
    (*Chandle) = NULL ;
    LAGRAPH_OK (GrB_Matrix_new (&C, type, nrows, ncols)) ;

    //--------------------------------------------------------------------------
    // read the entries
    //--------------------------------------------------------------------------

    GrB_Index i, j ;

    if (type == GrB_INT64)
    {

        //----------------------------------------------------------------------
        // read the entries as int64
        //----------------------------------------------------------------------

        int64_t x ;
        while (fscanf (f, "%"PRIu64"%"PRIu64"%"PRId64"\n", &i, &j, &x) != EOF)
        {
            LAGRAPH_OK (GrB_Matrix_setElement (C, x, i-1, j-1)) ;
        }

    }
    else if (type == GrB_UINT64)
    {

        //----------------------------------------------------------------------
        // read the entries as uint64
        //----------------------------------------------------------------------

        uint64_t x ;
        while (fscanf (f, "%"PRIu64"%"PRIu64"%"PRIu64"\n", &i, &j, &x) != EOF)
        {
            LAGRAPH_OK (GrB_Matrix_setElement (C, x, i-1, j-1)) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // read the entries as double, and typecast to the matrix type
        //----------------------------------------------------------------------

        double x ;
        while (fscanf (f, "%"PRIu64"%"PRIu64"%lg\n", &i, &j, &x) != EOF)
        {
            LAGRAPH_OK (GrB_Matrix_setElement (C, x, i-1, j-1)) ;
        }
    }

    //--------------------------------------------------------------------------
    // finalize the matrix and return the result
    //--------------------------------------------------------------------------

    GrB_Index ignore ;
    LAGRAPH_OK (GrB_Matrix_nvals (&ignore, C)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

