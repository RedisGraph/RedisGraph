//------------------------------------------------------------------------------
// GB_Vector_diag: extract a diagonal from a matrix, as a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_FREE_WORK        \
    GB_phbix_free (T) ;

#define GB_FREE_ALL         \
    GB_FREE_WORK ;          \
    GB_phbix_free (V) ;

#include "GB_diag.h"
#include "GB_select.h"

GrB_Info GB_Vector_diag     // extract a diagonal from a matrix, as a vector
(
    GrB_Matrix V,                   // output vector (as an n-by-1 matrix)
    const GrB_Matrix A,             // input matrix
    int64_t k,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (A, "A input for GB_Vector_diag", GB0) ;
    ASSERT_MATRIX_OK (V, "V input for GB_Vector_diag", GB0) ;
    ASSERT (GB_VECTOR_OK (V)) ;             // V is a vector on input
    ASSERT (!GB_aliased (A, V)) ;           // A and V cannot be aliased
    ASSERT (!GB_IS_HYPERSPARSE (V)) ;       // vectors cannot be hypersparse

    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = GB_clear_static_header (&T_header) ;

    GrB_Type atype = A->type ;
    GrB_Type vtype = V->type ;
    int64_t nrows = GB_NROWS (A) ;
    int64_t ncols = GB_NCOLS (A) ;
    int64_t n ;
    if (k >= ncols || k <= -nrows)
    { 
        // output vector V must have zero length
        n = 0 ;
    }
    else if (k >= 0)
    { 
        // if k is in range 0 to n-1, V must have length min (m,n-k)
        n = GB_IMIN (nrows, ncols - k) ;
    }
    else
    { 
        // if k is in range -1 to -m+1, V must have length min (m+k,n)
        n = GB_IMIN (nrows + k, ncols) ;
    }

    if (n != V->vlen)
    { 
        GB_ERROR (GrB_DIMENSION_MISMATCH,
            "Input vector must have size " GBd "\n", n) ;
    }

    if (!GB_Type_compatible (atype, vtype))
    { 
        GB_ERROR (GrB_DOMAIN_MISMATCH, "Input matrix of type [%s] "
            "cannot be typecast to output of type [%s]\n",
            atype->name, vtype->name) ;
    }

    //--------------------------------------------------------------------------
    // finish any pending work in A and clear the output vector V
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT (A) ;
    GB_phbix_free (V) ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format of A
    //--------------------------------------------------------------------------

    bool csc = A->is_csc ;
    if (!csc)
    { 
        // The kth diagonal of a CSC matrix is the same as the (-k)th diagonal
        // of the CSR format, so if A is CSR, negate the value of k.  Then
        // treat A as if it were CSC in the rest of this method.
        k = -k ;
    }

    //--------------------------------------------------------------------------
    // extract the kth diagonal of A into the temporary hypersparse matrix T
    //--------------------------------------------------------------------------

    // FUTURE: if A is bitmap or full, do not use GB_selector
    GB_OK (GB_selector (T, GB_DIAG_opcode, NULL, false, A, k, NULL, Context)) ;
    GB_OK (GB_convert_any_to_hyper (T, Context)) ;
    GB_MATRIX_WAIT (T) ;
    ASSERT_MATRIX_OK (T, "T = diag (A,k)", GB0) ;

    //--------------------------------------------------------------------------
    // transplant the pattern of T into the sparse vector V
    //--------------------------------------------------------------------------

    int64_t vnz = GB_nnz (T) ;
    float bitmap_switch = V->bitmap_switch ;
    int sparsity_control = V->sparsity_control ;
    bool static_header = V->static_header ;

    GB_OK (GB_new (&V, static_header,   // prior static or dynamic header
        vtype, n, 1, GB_Ap_malloc, true, GxB_SPARSE,
        GxB_NEVER_HYPER, 1, Context)) ;

    V->sparsity_control = sparsity_control ;
    V->bitmap_switch = bitmap_switch ;
    V->iso = T->iso ;       // OK
    if (V->iso)
    { 
        GBURBLE ("(iso diag) ") ;
    }

    V->p [0] = 0 ;
    V->p [1] = vnz ;
    if (k >= 0)
    { 
        // transplant T->i into V->i
        V->i = T->i ;
        V->i_size = T->i_size ;
        T->i = NULL ;
    }
    else
    { 
        // transplant T->h into V->i
        V->i = T->h ;
        V->i_size = T->h_size ;
        T->h = NULL ;
    }

    //--------------------------------------------------------------------------
    // transplant or typecast the values from T to V
    //--------------------------------------------------------------------------

    GB_Type_code vcode = vtype->code ;
    GB_Type_code acode = atype->code ;
    if (vcode == acode)
    { 
        // transplant T->x into V->x
        V->x = T->x ;
        V->x_size = T->x_size ;
        T->x = NULL ;
    }
    else
    {
        // V->x = (vtype) T->x
        V->x = GB_XALLOC (V->iso, vnz, vtype->size, &(V->x_size)) ;
        if (V->x == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }
        GB_cast_matrix (V, T, Context) ;
    }

    //--------------------------------------------------------------------------
    // finalize the vector V
    //--------------------------------------------------------------------------

    V->jumbled = T->jumbled ;
    V->nvec_nonempty = (vnz == 0) ? 0 : 1 ;
    V->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // free workspace, conform V to its desired format, and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (V, "V before conform for GB_Vector_diag", GB0) ;
    GB_OK (GB_conform (V, Context)) ;
    ASSERT_MATRIX_OK (V, "V output for GB_Vector_diag", GB0) ;
    return (GrB_SUCCESS) ;
}

