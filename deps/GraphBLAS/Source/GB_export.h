//------------------------------------------------------------------------------
// GB_export.h: definitions for import/export
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_EXPORT_H
#define GB_EXPORT_H
#include "GB_transpose.h"

//------------------------------------------------------------------------------
// macros for import/export
//------------------------------------------------------------------------------

#define GB_IMPORT_CHECK                                         \
    GB_RETURN_IF_NULL (A) ;                                     \
    (*A) = NULL ;                                               \
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;                        \
    if (nrows > GB_INDEX_MAX)                                   \
    {                                                           \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,           \
            "problem too large: nrows "GBu" exceeds "GBu,       \
            nrows, GB_INDEX_MAX))) ;                            \
    }                                                           \
    if (ncols > GB_INDEX_MAX)                                   \
    {                                                           \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,           \
            "problem too large: ncols "GBu" exceeds "GBu,       \
            ncols, GB_INDEX_MAX))) ;                            \
    }                                                           \
    if (nvals > GB_INDEX_MAX)                                   \
    {                                                           \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,           \
            "problem too large: nvals "GBu" exceeds "GBu,       \
            nvals, GB_INDEX_MAX))) ;                            \
    }                                                           \
    /* get the descriptor */                                    \
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6) ;

#define GB_EXPORT_CHECK                                         \
    GB_RETURN_IF_NULL (A) ;                                     \
    GB_RETURN_IF_NULL_OR_FAULTY (*A) ;                          \
    ASSERT_MATRIX_OK (*A, "A to export", GB0) ;                 \
    /* finish any pending work */                               \
    GB_WAIT (*A) ;                                              \
    /* check these after forcing completion */                  \
    GB_RETURN_IF_NULL (type) ;                                  \
    GB_RETURN_IF_NULL (nrows) ;                                 \
    GB_RETURN_IF_NULL (ncols) ;                                 \
    GB_RETURN_IF_NULL (nvals) ;                                 \
    GB_RETURN_IF_NULL (nonempty) ;                              \
    /* get the descriptor */                                    \
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6) ; \
    /* export basic attributes */                               \
    (*type) = (*A)->type ;                                      \
    (*nrows) = GB_NROWS (*A) ;                                  \
    (*ncols) = GB_NCOLS (*A) ;                                  \
    (*nvals) = GB_NNZ (*A) ;

#endif

