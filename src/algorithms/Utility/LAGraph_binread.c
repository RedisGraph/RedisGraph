//------------------------------------------------------------------------------
// LAGraph_binread:  read a matrix from a binary file
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

// LAGraph_binread:  read a matrix from a binary file
// Contributed by Tim Davis, Texas A&M.

// Reads a matrix from a file in a binary format.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (A) ;          \
    LAGRAPH_FREE (Ap) ;     \
    LAGRAPH_FREE (Ah) ;     \
    LAGRAPH_FREE (Ai) ;     \
    LAGRAPH_FREE (Ax) ;     \
}

#define FREAD(p,s,n)                                                \
{                                                                   \
    size_t result = fread (p, s, n, f) ;                            \
    if (result != n)                                                \
    {                                                               \
        fclose (f) ;                                                \
        LAGRAPH_ERROR ("File I/O error", GrB_INVALID_VALUE) ;       \
    }                                                               \
}

//------------------------------------------------------------------------------
// LAGraph_binread
//------------------------------------------------------------------------------

GrB_Info LAGraph_binread
(
    GrB_Matrix *A,          // matrix to read from the file
    char *filename          // file to read it from
)
{

    GrB_Index *Ap = NULL, *Ai = NULL, *Ah = NULL ;
    void *Ax = NULL ;
    if (A != NULL) (*A) = NULL ;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    if (A == NULL || filename == NULL)
    {
        // input arguments invalid
        LAGRAPH_ERROR ("LAGraph_binread: invalid inputs\n", GrB_NULL_POINTER) ;
    }

    FILE *f = fopen (filename, "r") ;
    if (f == NULL)
    {
        LAGRAPH_ERROR ("LAGraph_binread: file cannot be opened",
            GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // basic matrix properties
    //--------------------------------------------------------------------------

    GxB_Format_Value fmt = -999 ;
    bool is_hyper ;
    int32_t kind, typecode ;
    double hyper = -999 ;
    GrB_Type type ;
    GrB_Index nrows, ncols, nvals, nvec ;
    size_t typesize ;
    int64_t nonempty ;

    //--------------------------------------------------------------------------
    // read the header (and ignore it)
    //--------------------------------------------------------------------------

    // The header is informational only, for "head" command, so the file can
    // be visually inspected.

    char header [LAGRAPH_BIN_HEADER] ;
    FREAD (header, sizeof (char), LAGRAPH_BIN_HEADER) ;
    // printf ("%s\n", header) ;

    //--------------------------------------------------------------------------
    // read the scalar content
    //--------------------------------------------------------------------------

    FREAD (&fmt,      sizeof (GxB_Format_Value), 1) ;
    FREAD (&kind,     sizeof (int32_t), 1) ;
    FREAD (&hyper,    sizeof (double), 1) ;
    FREAD (&nrows,    sizeof (GrB_Index), 1) ;
    FREAD (&ncols,    sizeof (GrB_Index), 1) ;
    FREAD (&nonempty, sizeof (int64_t), 1) ;
    FREAD (&nvec,     sizeof (GrB_Index), 1) ;
    FREAD (&nvals,    sizeof (GrB_Index), 1) ;
    FREAD (&typecode, sizeof (int32_t), 1) ;
    FREAD (&typesize, sizeof (size_t), 1) ;

    is_hyper = (kind == 1) ;

    switch (typecode)
    {
        case 0:  type = GrB_BOOL        ; break ;
        case 1:  type = GrB_INT8        ; break ;
        case 2:  type = GrB_INT16       ; break ;
        case 3:  type = GrB_INT32       ; break ;
        case 4:  type = GrB_INT64       ; break ;
        case 5:  type = GrB_UINT8       ; break ;
        case 6:  type = GrB_UINT16      ; break ;
        case 7:  type = GrB_UINT32      ; break ;
        case 8:  type = GrB_UINT64      ; break ;
        case 9:  type = GrB_FP32        ; break ;
        case 10: type = GrB_FP64        ; break ;
        case 11: type = LAGraph_ComplexFP64 ; break ;
        default: LAGRAPH_ERROR ("unknown type", GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // allocate the array content
    //--------------------------------------------------------------------------

    Ap = LAGraph_malloc (nvec+1, sizeof (GrB_Index)) ;
    if (is_hyper)
    {
        Ah = LAGraph_malloc (nvec, sizeof (GrB_Index)) ;
    }
    Ai = LAGraph_malloc (nvals, sizeof (GrB_Index)) ;
    Ax = LAGraph_malloc (nvals, typesize) ;

    if (Ap == NULL || Ai == NULL || Ax == NULL || (is_hyper && Ah == NULL))
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // read the array content
    //--------------------------------------------------------------------------

    FREAD (Ap, sizeof (GrB_Index), nvec+1) ;
    if (is_hyper)
    {
        FREAD (Ah, sizeof (GrB_Index), nvec) ;
    }
    FREAD (Ai, sizeof (GrB_Index), nvals) ;
    FREAD (Ax, typesize, nvals) ;
    fclose (f) ;

    //--------------------------------------------------------------------------
    // import the matrix
    //--------------------------------------------------------------------------

    if (fmt == GxB_BY_COL && !is_hyper)
    {
        // standard CSC
        LAGRAPH_OK (GxB_Matrix_import_CSC (A, type, nrows, ncols, nvals,
            nonempty, &Ap, &Ai, &Ax, NULL)) ;
    }
    else if (fmt == GxB_BY_ROW && !is_hyper)
    {
        // standard CSR
        LAGRAPH_OK (GxB_Matrix_import_CSR (A, type, nrows, ncols, nvals,
            nonempty, &Ap, &Ai, &Ax, NULL)) ;
    }
    else if (fmt == GxB_BY_COL && is_hyper)
    {
        // hypersparse CSC
        LAGRAPH_OK (GxB_Matrix_import_HyperCSC (A, type, nrows, ncols, nvals,
            nonempty, nvec, &Ah, &Ap, &Ai, &Ax, NULL)) ;
    }
    else if (fmt == GxB_BY_ROW && is_hyper)
    {
        // hypersparse CSR
        LAGRAPH_OK (GxB_Matrix_import_HyperCSR (A, type, nrows, ncols, nvals,
            nonempty, nvec, &Ah, &Ap, &Ai, &Ax, NULL)) ;
    }
    else
    {
        LAGRAPH_ERROR ("unknown", GrB_INVALID_VALUE) ;
    }

    LAGRAPH_OK (GxB_set (*A, GxB_HYPER, hyper)) ;

    return (GrB_SUCCESS) ;
}
