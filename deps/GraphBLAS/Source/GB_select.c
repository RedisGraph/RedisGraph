//------------------------------------------------------------------------------
// GB_select: apply a select operator; optionally transpose a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C, select(A,k)) or select(A,k)').  This function is not
// user-callable.  It does the work for GxB_*_select.
// Compare this function with GrB_apply.

#include "GB.h"

static inline bool is_nonzero (const GB_void *value, int64_t size)
{
    for (int64_t i = 0 ; i < size ; i++)
    {
        if (value [i] != 0) return (true) ;
    }
    return (false) ;
}

//------------------------------------------------------------------------------

GrB_Info GB_select          // C<M> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // descriptor for M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // input matrix
    const void *k,                  // optional input for select operator
    const bool A_transpose,         // A matrix descriptor
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK2 (C, M, A)) ;

    GB_RETURN_IF_FAULTY (accum) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    int64_t kk = 0 ;
    if (op->opcode < GB_NONZERO_opcode)
    { 
        GB_RETURN_IF_NULL (k) ;
        kk = *((int64_t *) k) ;
    }

    ASSERT_OK (GB_check (C, "C input for GB_select", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for GB_select", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_select", GB0)) ;
    ASSERT_OK (GB_check (op, "selectop for GB_select", GB0)) ;
    ASSERT_OK (GB_check (A, "A input for GB_select", GB0)) ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, M, accum, A->type, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // C = op (A) must be compatible, already checked in GB_compatible
    // A must also be compatible with op->xtype, unless op->xtype is NULL
    if (op->xtype != NULL && !GB_Type_compatible (A->type, op->xtype))
    { 
        return (GB_ERROR (GrB_DOMAIN_MISMATCH, (GB_LOG,
            "incompatible type for C=%s(A,k):\n"
            "input A type [%s]\n"
            "cannot be typecast to operator x of type [%s]",
            op->name, A->type->name, op->xtype->name))) ;
    }

    // check the dimensions
    int64_t tnrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t tncols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    if (GB_NROWS (C) != tnrows || GB_NCOLS (C) != tncols)
    { 
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "input is "GBd"-by-"GBd"%s",
            GB_NROWS (C), GB_NCOLS (C),
            tnrows, tncols, A_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format and the transposed case
    //--------------------------------------------------------------------------

    // A and C can be in CSR or CSC format (in any combination), and A can be
    // transposed first via A_transpose.  However, A is not explicitly
    // transposed first.  Instead, the selection operation is modified by
    // changing the operator, and the resulting matrix T is transposed, if
    // needed.

    // Instead of explicitly transposing the input matrix A and output T:
    // If A in CSC format and not transposed: treat as if A and T were CSC
    // If A in CSC format and transposed:     treat as if A and T were CSR
    // If A in CSR format and not transposed: treat as if A and T were CSR
    // If A in CSR format and transposed:     treat as if A and T were CSC

    bool A_csc = (A->is_csc == !A_transpose) ;

    // The final transpose, if needed, is accomplished in GB_accum_mask, by
    // tagging T as the same CSR/CSC format as A_csc.  If the format of T and C
    // do not match, GB_accum_mask transposes T, computing C<M>=accum(C,T').

    //--------------------------------------------------------------------------
    // get the opcode and change it if needed
    //--------------------------------------------------------------------------

    GB_Select_Opcode opcode = op->opcode ;

    if (!A_csc)
    {
        // The built-in operators are modified so they can always work as if A
        // were in CSC format.  If A is not in CSC, then the operation is
        // flipped.
        // diag(A,k)    becomes diag(A,-k)
        // offdiag(A,k) becomes offdiag(A,-k)
        // tril(A,k)    becomes triu(A,-k)
        // triu(A,k)    becomes tril(A,-k)
        // nonzero(A)   is unchanged
        // userop(A)    row/col indices and dimensions are swapped
        if (opcode < GB_NONZERO_opcode)
        {
            kk = -kk ;
            if (opcode == GB_TRIL_opcode)
            { 
                opcode = GB_TRIU_opcode ;
            }
            else if (opcode == GB_TRIU_opcode)
            { 
                opcode = GB_TRIL_opcode ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // T = select(A,k)
    //--------------------------------------------------------------------------

    int64_t vlen = A->vlen ;
    int64_t vdim = A->vdim ;
    int64_t tnzmax = (opcode == GB_DIAG_opcode) ?
        GB_IMIN (vlen,vdim) : GB_NNZ (A) ;

    // [ create T in same format as A_csc, with just as many non-empty vectors,
    // and same hypersparsity as A
    GrB_Matrix T = NULL ;           // allocate a new header for T
    GB_CREATE (&T, A->type, A->vlen, A->vdim, GB_Ap_malloc, A_csc,
        GB_SAME_HYPER_AS (A->is_hyper), A->hyper_ratio, A->nvec_nonempty,
        tnzmax, true) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    int64_t *restrict Ti = T->i ;
    GB_void *restrict Tx = T->x ;

    const int64_t *restrict Ai = A->i ;
    const GB_void *restrict Ax = A->x ;

    int64_t asize = A->type->size ;

    // start the construction of T
    int64_t jlast, tnz, tnz_last ;
    GB_jstartup (T, &jlast, &tnz, &tnz_last) ;

    // keep an entry that satisfies a condition, copying it from A to T:
    #define GB_KEEP_IF(condition)                                   \
        if (condition)                                              \
        {                                                           \
            Ti [tnz] = i ;                                          \
            memcpy (Tx +(tnz*asize), Ax +(p*asize), asize) ;        \
            tnz++ ;                                                 \
        }

    switch (opcode)
    {

        //----------------------------------------------------------------------
        // T = tril (A,k)
        //----------------------------------------------------------------------

        case GB_TRIL_opcode:
        {
            GB_for_each_vector (A)
            {
                int64_t GBI1_initj (Iter, j, p, pend) ;
                // an entry is kept if (j-i) <= kk, so smallest i is j-kk.
                int64_t ifirst = j - kk ;
                if (ifirst < vlen)
                {
                    int64_t pright = pend - 1 ;
                    GB_BINARY_TRIM_SEARCH (ifirst, Ai, p, pright) ;
                    for ( ; p < pend ; p++)
                    { 
                        int64_t i = Ai [p] ;
                        GB_KEEP_IF ((j-i) <= kk) ;
                    }
                }
                info = GB_jappend (T, j, &jlast, tnz, &tnz_last, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
        }

        break ;

        //----------------------------------------------------------------------
        // T = triu (A,k)
        //----------------------------------------------------------------------

        case GB_TRIU_opcode:
        {
            GB_for_each_vector (A)
            {
                GB_for_each_entry (j, p, pend)
                { 
                    int64_t i = Ai [p] ;
                    GB_KEEP_IF ((j-i) >= kk) else break ;
                }
                info = GB_jappend (T, j, &jlast, tnz, &tnz_last, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
        }
        break ;

        //----------------------------------------------------------------------
        // T = diag (A,k)
        //----------------------------------------------------------------------

        case GB_DIAG_opcode:
        {
            GB_for_each_vector (A)
            {
                // use binary search to find the entry on the kk-th diagonal:
                // look for entry A(i,j) with index i = j-kk
                int64_t GBI1_initj (Iter, j, p, pend) ;
                int64_t i = j-kk ;
                if (i < 0 || i >= vlen) continue ;
                bool found ;
                if (pend - p == vlen)
                { 
                    // A(:,j) is dense
                    found = true ;
                    p += i ;
                }
                else
                { 
                    int64_t pright = pend - 1 ;
                    GB_BINARY_SEARCH (i, Ai, p, pright, found) ;
                }
                GB_KEEP_IF (found) ;
                info = GB_jappend (T, j, &jlast, tnz, &tnz_last, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
        }
        break ;

        //----------------------------------------------------------------------
        // T = offdiag (A,k)
        //----------------------------------------------------------------------

        case GB_OFFDIAG_opcode:
        {
            GB_for_each_vector (A)
            {
                GB_for_each_entry (j, p, pend)
                { 
                    int64_t i = Ai [p] ;
                    GB_KEEP_IF ((j-i) != kk) ;
                }
                info = GB_jappend (T, j, &jlast, tnz, &tnz_last, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
        }
        break ;

        //----------------------------------------------------------------------
        // T = nonzero (A,k)
        //----------------------------------------------------------------------

        case GB_NONZERO_opcode:
        {
            GB_for_each_vector (A)
            {
                GB_for_each_entry (j, p, pend)
                { 
                    int64_t i = Ai [p] ;
                    GB_KEEP_IF (is_nonzero (Ax +(p*asize), asize)) ;
                }
                info = GB_jappend (T, j, &jlast, tnz, &tnz_last, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
        }
        break ;

        //----------------------------------------------------------------------
        // T = userselect (A,k)
        //----------------------------------------------------------------------

        case GB_USER_SELECT_C_opcode:
        case GB_USER_SELECT_R_opcode:
        {
            GxB_select_function select = (GxB_select_function) (op->function) ;
            GB_for_each_vector (A)
            {
                GB_for_each_entry (j, p, pend)
                {
                    int64_t i = Ai [p] ;
                    bool keep ;
                    if (A_csc)
                    { 
                        keep = select (i, j, vlen, vdim, Ax +(p*asize), k) ;
                    }
                    else
                    { 
                        keep = select (j, i, vdim, vlen, Ax +(p*asize), k) ;
                    }
                    GB_KEEP_IF (keep) ;
                }
                info = GB_jappend (T, j, &jlast, tnz, &tnz_last, Context) ;
                ASSERT (info == GrB_SUCCESS) ;
            }
        }
        break ;

        default:
            ASSERT (false) ;
            break ;

    }

    ASSERT (info == GrB_SUCCESS) ;
    GB_jwrapup (T, jlast, tnz) ;        // T->p is now finalized ]
    ASSERT_OK (GB_check (T, "T=select(A,k)", GB0)) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Context)) ;
}

