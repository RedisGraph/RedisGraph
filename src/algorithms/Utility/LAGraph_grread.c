//------------------------------------------------------------------------------
// LAGraph_grread:  read a matrix from a binary file
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

// LAGraph_grread:  read a matrix from a binary file.
// Contributed by Tim Davis, Texas A&M, based on the Galois graph reader
// file format.

// The file format consists of a header, with the following content:

//      uint64_t version : either 1 or 2.  1: nodes are 2^32, 2: nodes are
//          64 bit.  This value is returned to the caller, but is otherwise
//          unused.

//      uint64_t esize : the size of the edge weight, as sizeof (edgetype).
//          For example, if the file contains edge weights of type int32_t,
//          esize is sizeof (int32_t) == 4.  The caller must specify the
//          corresponding GrB_Type, and its size must match esize.

//      uint64_t n : the number of node in the graph.  The GrB_Matrix is
//          n-by-n.  Rectangular matrices are not supported by this format.

//      uint64_t e : the number of edges in the graph

// This header is followed by a matrix in CSR format:

//      Gp : an array of size ((n+1) * sizeof (uint64_t)) bytes, but Gp [0] = 0
//          does not appear in the file.  This section of the file is thus
//          (n * sizeof (uint64_t)) bytes in length.

//      Gj : an array of size (e * sizeof (int32_t)), containing the adjaceny
//          lists.  Note that the indices are 32 bit, not 64 bit, and thus
//          this format is limited to graphs with n < 2^32.

//      Gx : an array of size (e * esize), containing the edge weights.

// LAgraph_grread returns its status: GrB_SUCCESS if succesful,
// GrB_OUT_OF_MEMORY if out of memory, GrB_INVALID_VALUE if a file I/O error
// occurs or the edge size is not what was expected.

#include "LAGraph_internal.h"

//------------------------------------------------------------------------------
// gr_header
//------------------------------------------------------------------------------

// The gr_header specifies the first 4 * sizeof(uint64_t) bytes of the file.

typedef struct
{
    uint64_t version ;      // either 1 or 2.
                            // 1: node id's are in the range 0 to 2^32
                            // 2: node id's are in the range 0 to 2^64
    uint64_t esize ;        // sizeof (edgetype)
    uint64_t n ;            // # of nodes in the graph
    uint64_t e ;            // # of edges in the graph
}
gr_header ;

//------------------------------------------------------------------------------
// LAGraph_binary_read
//------------------------------------------------------------------------------

// Read a block of binary data from a file.  Returns GrB_SUCCESS if successful,
// GrB_INVALID_VALUE otherwise.

static GrB_Info LAGraph_binary_read
(
    char *name,             // name of array being read in
    FILE *fp,               // file to read from
    void *buffer,           // buffer of size nbytes to read into
    size_t n,               // # of elements to read
    size_t size             // size of each element
)
{
    if (fp == NULL)
    {
        fprintf (stderr, "LAGraph_grread: file I/O error\n") ;
        return (GrB_INVALID_VALUE) ;
    }
    size_t n_read = fread (buffer, size, n, fp) ;
    if (n_read != n)
    {
        fprintf (stderr, "LAGraph_grread: file I/O error; expected %g items"
            ", got %g, object %s, size %g\n", (double) n_read, (double) n,
            name, (double) size) ;
        return (GrB_INVALID_VALUE) ;
    }
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// LAGRAPH_FREE_ALL
//------------------------------------------------------------------------------

// Free all allocated space; used only for error return.

#define LAGRAPH_FREE_ALL        \
{                               \
    GrB_free (G) ;              \
    LAGRAPH_FREE (Gp) ;         \
    LAGRAPH_FREE (Gj) ;         \
    LAGRAPH_FREE (Gj_32) ;      \
    LAGRAPH_FREE (Gx) ;         \
    if (fp != NULL) fclose (fp) ; \
    fp = NULL ;                 \
}

//------------------------------------------------------------------------------
// LAGraph_grread
//------------------------------------------------------------------------------

GrB_Info LAGraph_grread     // read a matrix from a binary file
(
    GrB_Matrix *G,          // handle of matrix to create
    uint64_t *G_version,    // the version in the file
    const char *filename,   // name of file to open
    GrB_Type gtype          // type of matrix to read, NULL if no edge weights
                            // (in that case, G has type GrB_BOOL with all
                            // edge weights equal to 1).
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Index *Gp = NULL ;
    int32_t *Gj_32 = NULL ;
    GrB_Index *Gj = NULL ;
    void *Gx = NULL ;
    FILE *fp = NULL ;

    if (G == NULL || G_version == NULL  || filename == NULL)
    {
        LAGRAPH_ERROR ("invalid input arguments", GrB_NULL_POINTER) ;
    }

    (*G) = NULL ;
    (*G_version) = 0 ;

    //--------------------------------------------------------------------------
    // open the file
    //--------------------------------------------------------------------------

    fp = fopen (filename, "r") ;
    if (fp == NULL)
    {
        fprintf (stderr, "LAGraph_grread: file not found: %s\n", filename) ;
        LAGRAPH_ERROR ("input file not found", GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // open the file and read the gr_header
    //--------------------------------------------------------------------------

    gr_header header ;
    LAGRAPH_OK (LAGraph_binary_read ("header",
        fp, &header, 1, sizeof (gr_header))) ;

    uint64_t version = header.version ; // version, 1 or 2
    uint64_t esize = header.esize ;     // sizeof (edge type)
    uint64_t n = header.n ;             // # of nodes
    uint64_t e = header.e ;             // # of edges
    (*G_version) = version ;

    size_t esize_expected = 0 ;
    if (gtype != NULL)
    {
        LAGRAPH_OK (GxB_Type_size (&esize_expected, gtype)) ;
    }
    if (esize != esize_expected)
    {
        fprintf (stderr, "LAGraph_grread: esize in file (%g) does not match"
            " gtype size (%g)\n", (double) esize, (double) esize_expected) ;
        LAGRAPH_ERROR ("unexpected edge size", GrB_INVALID_VALUE) ;
    }

    if (! (version == 1 || version == 2))
    {
        LAGRAPH_ERROR ("invalid version, must be 1 or 2", GrB_INVALID_VALUE) ;
    }

    if (version == 1 && n > UINT32_MAX)
    {
        LAGRAPH_ERROR ("problem too large", GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // allocate and read in the pointers
    //--------------------------------------------------------------------------

    Gp = LAGraph_malloc (n+1, sizeof (GrB_Index)) ;
    if (Gp == NULL)
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }

    Gp [0] = 0 ;
    LAGRAPH_OK (LAGraph_binary_read ("pointers",
        fp, Gp+1, n, sizeof (GrB_Index))) ;

    //--------------------------------------------------------------------------
    // allocate and read in the indices
    //--------------------------------------------------------------------------

    Gj = LAGraph_malloc (e, sizeof (GrB_Index)) ;
    if (Gj == NULL)
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }

    if (version == 1)
    {

        //----------------------------------------------------------------------
        // indices are in 32-bit format in the file
        //----------------------------------------------------------------------

        // allocate workspace for a single chunk
        #define CHUNK (10 * 1024 * 1024)
        int64_t chunk = LAGRAPH_MIN (CHUNK, e) ;
        Gj_32 = LAGraph_malloc (chunk, sizeof (int32_t)) ;
        if (Gj_32 == NULL)
        {
            LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
        }

        // read in the indices one chunk at a time
        for (int64_t k = 0 ; k < e ; k += CHUNK)
        {
            // read in the next chunk
            int64_t chunk = LAGRAPH_MIN (CHUNK, e-k) ;
            LAGRAPH_OK (LAGraph_binary_read ("indices",
                fp, Gj_32, chunk, sizeof (int32_t))) ;
            // convert the chunk to 64-bit
            #pragma omp parallel for schedule(static)
            for (GrB_Index p = 0 ; p < chunk ; p++)
            {
                Gj [k + p] = (GrB_Index) Gj_32 [p] ;
            }
        }
        LAGRAPH_FREE (Gj_32) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // indices are in 64-bit format in the file
        //----------------------------------------------------------------------

        LAGRAPH_OK (LAGraph_binary_read ("indices",
            fp, Gj, e, sizeof (GrB_Index))) ;
    }

    //--------------------------------------------------------------------------
    // read in the values
    //--------------------------------------------------------------------------

    bool no_edge_weights = (gtype == NULL) ;
    if (no_edge_weights)
    {
        // the input file has no edge weights
        gtype = GrB_BOOL ;
        esize = sizeof (bool) ;
    }

    Gx = LAGraph_malloc (e, esize) ;
    if (Gx == NULL) LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;

    if (no_edge_weights)
    {
        // set all edge weights to boolean true
        bool *Gbool = (bool *) Gx ;
        #pragma omp parallel for schedule(static)
        for (GrB_Index p = 0 ; p < e ; p++)
        {
            Gbool [p] = true ;
        }
    }
    else
    {
        // read in the edge weights
        LAGRAPH_OK (LAGraph_binary_read ("edgeweights", fp, Gx, e, esize)) ;
    }

    //--------------------------------------------------------------------------
    // import the data into the GrB_Matrix
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GxB_Matrix_import_CSR (G, gtype, n, n, e, -1, &Gp, &Gj, &Gx,
        NULL)) ;

    //--------------------------------------------------------------------------
    // close the file and return result
    //--------------------------------------------------------------------------

    fclose (fp) ;
    return (GrB_SUCCESS) ;
}

