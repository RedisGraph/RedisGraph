//------------------------------------------------------------------------------
// GB_concat_full: concatenate an array of matrices into a full matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_FREE_WORKSPACE   \
    GB_Matrix_free (&T) ;

#define GB_FREE_ALL         \
    GB_FREE_WORKSPACE ;     \
    GB_phybix_free (C) ;

#include "GB_concat.h"

GrB_Info GB_concat_full             // concatenate into a full matrix
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_iso,               // if true, construct C as iso
    const GB_void *cscalar,         // iso value of C, if C is io 
    const GrB_Matrix *Tiles,        // 2D row-major array of size m-by-n,
    const GrB_Index m,
    const GrB_Index n,
    const int64_t *restrict Tile_rows,  // size m+1
    const int64_t *restrict Tile_cols,  // size n+1
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // allocate C as a full matrix
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL ;
    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = NULL ;

    GrB_Type ctype = C->type ;
    int64_t cvlen = C->vlen ;
    int64_t cvdim = C->vdim ;
    bool csc = C->is_csc ;
    size_t csize = ctype->size ;
    GB_Type_code ccode = ctype->code ;
    if (!GB_IS_FULL (C))
    { 
        // set C->iso = C_iso   OK
        GB_phybix_free (C) ;
        GB_OK (GB_bix_alloc (C, GB_nnz_full (C), GxB_FULL, false, true, C_iso,
            Context)) ;
        C->plen = -1 ;
        C->nvec = cvdim ;
        C->nvec_nonempty = (cvlen > 0) ? cvdim : 0 ;
    }
    ASSERT (GB_IS_FULL (C)) ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    int64_t nouter = csc ? n : m ;
    int64_t ninner = csc ? m : n ;

    if (C_iso)
    { 
        // copy in the scalar as the iso value; no more work to do
        memcpy (C->x, cscalar, csize) ;
        C->magic = GB_MAGIC ;
        ASSERT_MATRIX_OK (C, "C output for concat iso full", GB0) ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // concatenate all matrices into C
    //--------------------------------------------------------------------------

    for (int64_t outer = 0 ; outer < nouter ; outer++)
    {
        for (int64_t inner = 0 ; inner < ninner ; inner++)
        {

            //------------------------------------------------------------------
            // get the tile A; transpose and typecast, if needed
            //------------------------------------------------------------------

            A = csc ? GB_TILE (Tiles, inner, outer)
                    : GB_TILE (Tiles, outer, inner) ;
            if (csc != A->is_csc)
            { 
                // T = (ctype) A', not in-place
                GB_CLEAR_STATIC_HEADER (T, &T_header) ;
                GB_OK (GB_transpose_cast (T, ctype, csc, A, false, Context)) ;
                A = T ;
                GB_MATRIX_WAIT (A) ;
            }
            ASSERT (C->is_csc == A->is_csc) ;
            ASSERT (GB_is_dense (A)) ;
            ASSERT (!GB_ANY_PENDING_WORK (A)) ;
            GB_Type_code acode = A->type->code ;

            //------------------------------------------------------------------
            // determine where to place the tile in C
            //------------------------------------------------------------------

            // The tile A appears in vectors cvstart:cvend-1 of C, and indices
            // cistart:ciend-1.

            int64_t cvstart, cvend, cistart, ciend ;
            if (csc)
            { 
                // C and A are held by column
                // Tiles is row-major and accessed in column order
                cvstart = Tile_cols [outer] ;
                cvend   = Tile_cols [outer+1] ;
                cistart = Tile_rows [inner] ;
                ciend   = Tile_rows [inner+1] ;
            }
            else
            { 
                // C and A are held by row
                // Tiles is row-major and accessed in row order
                cvstart = Tile_rows [outer] ;
                cvend   = Tile_rows [outer+1] ;
                cistart = Tile_cols [inner] ;
                ciend   = Tile_cols [inner+1] ;
            }

            int64_t avdim = cvend - cvstart ;
            int64_t avlen = ciend - cistart ;
            ASSERT (avdim == A->vdim) ;
            ASSERT (avlen == A->vlen) ;
            int64_t anz = avdim * avlen ;
            int A_nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

            //------------------------------------------------------------------
            // copy the tile A into C
            //------------------------------------------------------------------

            bool done = false ;

            #ifndef GBCUDA_DEV
                if (ccode == acode)
                {
                    // no typecasting needed
                    switch (csize)
                    {
                        #define GB_COPY(pC,pA,A_iso)                        \
                            Cx [pC] = GBX (Ax, pA, A_iso) ;

                        case GB_1BYTE : // uint8, int8, bool, or 1-byte user
                            #define GB_CTYPE uint8_t
                            #include "GB_concat_full_template.c"
                            break ;

                        case GB_2BYTE : // uint16, int16, or 2-byte user
                            #define GB_CTYPE uint16_t
                            #include "GB_concat_full_template.c"
                            break ;

                        case GB_4BYTE : // uint32, int32, float, or 4-byte user
                            #define GB_CTYPE uint32_t
                            #include "GB_concat_full_template.c"
                            break ;

                        case GB_8BYTE : // uint64, int64, double, float complex,
                                        // or 8-byte user defined
                            #define GB_CTYPE uint64_t
                            #include "GB_concat_full_template.c"
                            break ;

                        case GB_16BYTE : // double complex or 16-byte user
                            #define GB_CTYPE GB_blob16
                            /*
                            #define GB_CTYPE uint64_t
                            #undef  GB_COPY
                            #define GB_COPY(pC,pA,A_iso)                    \
                                Cx [2*pC  ] = Ax [A_iso ? 0 : (2*pA)] ;     \
                                Cx [2*pC+1] = Ax [A_iso ? 1 : (2*pA+1)] ;
                            */
                            #include "GB_concat_full_template.c"
                            break ;

                        default:;
                    }
                }
            #endif

            if (!done)
            { 
                // with typecasting or user-defined types
                GB_cast_function cast_A_to_C = GB_cast_factory (ccode, acode) ;
                size_t asize = A->type->size ;
                #define GB_CTYPE GB_void
                #undef  GB_COPY
                #define GB_COPY(pC,pA,A_iso)                    \
                    cast_A_to_C (Cx + (pC)*csize,               \
                        Ax + (A_iso ? 0:(pA)*asize), asize) ;
                #include "GB_concat_full_template.c"
            }

            GB_FREE_WORKSPACE ;
        }
    }

    C->magic = GB_MAGIC ;
    ASSERT_MATRIX_OK (C, "C output for concat full", GB0) ;
    return (GrB_SUCCESS) ;
}

