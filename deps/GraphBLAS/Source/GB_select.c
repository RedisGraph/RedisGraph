//------------------------------------------------------------------------------
// GB_select: apply a select operator; optionally transpose a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C, select(A,k)) or select(A,k)').  This function is not
// user-callable.  It does the work for GxB_*_select.
// Compare this function with GrB_apply.

#include "GB.h"

static inline bool is_nonzero (const void *value, int64_t size)
{
    const char *x = value ;
    for (int64_t i = 0 ; i < size ; i++)
    {
        if (x [i] != 0) return (true) ;
    }
    return (false) ;
}

//------------------------------------------------------------------------------

GrB_Info GB_select          // C<Mask> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const bool Mask_comp,           // Mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // input matrix
    const void *k,                  // optional input for select operator
    const bool A_transpose          // A matrix descriptor
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C, Mask, and A already checked in caller
    RETURN_IF_UNINITIALIZED (accum) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (op) ;
    int64_t kk = 0 ; 
    if (op->opcode < GB_NONZERO_opcode)
    {
        RETURN_IF_NULL (k) ;
        kk = *((int64_t *) k) ;
    }

    ASSERT_OK (GB_check (C, "C input for GB_select", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_select", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_select", 0)) ;
    ASSERT_OK (GB_check (op, "selectop for GB_select", 0)) ;
    ASSERT_OK (GB_check (A, "A input for GB_select", 0)) ;

    // check domains and dimensions for C<Mask> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, Mask, accum, A->type) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // C = op (A) must be compatible, already checked in GB_compatible
    // A must also be compatible with op->xtype, unless op->xtype is NULL
    if (op->xtype != NULL && !GB_Type_compatible (A->type, op->xtype))
    {
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "incompatible type for C=%s(A,k):\n"
            "input A type [%s]\n"
            "cannot be typecast to operator x of type [%s]",
            op->name, A->type->name, op->xtype->name))) ;
    }

    // check the dimensions
    int64_t ancols = A->ncols ;
    int64_t anrows = A->nrows ;
    int64_t tnrows = (A_transpose) ? ancols : anrows ;
    int64_t tncols = (A_transpose) ? anrows : ancols ;
    if (C->nrows != tnrows || C->ncols != tncols)
    {
        return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
            "Dimensions not compatible:\n"
            "output is "GBd"-by-"GBd"\n"
            "input is "GBd"-by-"GBd"%s",
            C->nrows, C->ncols,
            tnrows, tncols, A_transpose ? " (transposed)" : ""))) ;
    }

    // quick return if an empty Mask is complemented
    RETURN_IF_QUICK_MASK (C, C_replace, Mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (C) ;
    APPLY_PENDING_UPDATES (Mask) ;
    APPLY_PENDING_UPDATES (A) ;

    //--------------------------------------------------------------------------
    // get the opcode and modify it for the transposed case
    //--------------------------------------------------------------------------

    GB_Select_Opcode opcode = op->opcode ;
    if (A_transpose && opcode < GB_NONZERO_opcode)
    {
        // A is conceptually transposed first, but it is faster to
        // transpose S instead:
        // diag(A',k)    is diag(A,-k)'
        // offdiag(A',k) is offdiag(A,-k)'
        // tril(A',k)    is triu(A,-k)'
        // triu(A',k)    is tril(A,-k)'
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

    //--------------------------------------------------------------------------
    // S = select(A,k)
    //--------------------------------------------------------------------------

    // allocate S, the same type and size as A
    GrB_Matrix S = NULL ;
    GB_NEW (&S, A->type, anrows, ancols, false, true) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    int64_t snzmax = (opcode == GB_DIAG_opcode) ? IMIN (anrows,ancols) : NNZ(A);

    double memory = 0 ;
    if (!GB_Matrix_alloc (S, snzmax, true, &memory))
    {
        // out of memory
        GB_Matrix_clear (C) ;           // C is now initialized, just empty
        ASSERT_OK (GB_check (C, "C cleared", 0)) ;
        GB_MATRIX_FREE (&S) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    int64_t *restrict Sp = S->p ;
    int64_t *restrict Si = S->i ;
    void    *restrict Sx = S->x ;

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const void    *restrict Ax = A->x ;

    int64_t asize = A->type->size ;

    int64_t snz = 0 ;

    switch (opcode)
    {

        //----------------------------------------------------------------------
        // S = tril (A,k)
        //----------------------------------------------------------------------

        case GB_TRIL_opcode:
        {
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                Sp [j] = snz ;
                if (j-anrows > kk) continue ;
                for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                {
                    int64_t i = Ai [p] ;
                    if ((j-i) <= kk)
                    {
                        // keep this entry
                        Si [snz] = i ;
                        memcpy (Sx +(snz*asize), Ax +(p*asize), asize) ;
                        snz++ ;
                    }
                }
            }
        }
        break ;

        //----------------------------------------------------------------------
        // S = triu (A,k)
        //----------------------------------------------------------------------

        case GB_TRIU_opcode:
        {
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                Sp [j] = snz ;
                if (j < kk) continue ;
                for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                {
                    int64_t i = Ai [p] ;
                    if ((j-i) >= kk)
                    {
                        // keep this entry
                        Si [snz] = i ;
                        // Sx [snz] = Ax [p]
                        memcpy (Sx +(snz*asize), Ax +(p*asize), asize) ;
                        snz++ ;
                    }
                    else break ;
                }
            }
        }
        break ;

        //----------------------------------------------------------------------
        // S = diag (A,k)
        //----------------------------------------------------------------------


        case GB_DIAG_opcode:
        {
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                // use binary search to find the entry on the kth diagonal
                Sp [j] = snz ;
                // look for row index j-kk
                int64_t i = j-kk ;
                if (i < 0 || i >= anrows) continue ;
                int64_t p = Ap [j] ;
                int64_t pright = Ap [j+1] - 1 ;
                bool found ;
                if (pright - p == anrows - 1)
                {
                    // column j is dense
                    found = true ;
                    p += i ;
                }
                else
                {
                    GB_BINARY_SEARCH (i, Ai, p, pright, found) ;
                }
                if (found)
                {
                    // keep this entry
                    ASSERT (Ai [p] == i) ;
                    Si [snz] = i ;
                    // Sx [snz] = Ax [p]
                    memcpy (Sx +(snz*asize), Ax +(p*asize), asize) ;
                    snz++ ;
                }
            }
        }
        break ;

        //----------------------------------------------------------------------
        // S = offdiag (A,k)
        //----------------------------------------------------------------------

        case GB_OFFDIAG_opcode:
        {
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                Sp [j] = snz ;
                for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                {
                    int64_t i = Ai [p] ;
                    if ((j-i) != kk)
                    {
                        // keep this entry
                        Si [snz] = i ;
                        // Sx [snz] = Ax [p]
                        memcpy (Sx +(snz*asize), Ax +(p*asize), asize) ;
                        snz++ ;
                    }
                }
            }
        }
        break ;

        //----------------------------------------------------------------------
        // S = nonzero (A,k)
        //----------------------------------------------------------------------

        case GB_NONZERO_opcode:
        {
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                Sp [j] = snz ;
                for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                {
                    int64_t i = Ai [p] ;
                    if (is_nonzero (Ax + (p*asize), asize))
                    {
                        // keep this entry
                        Si [snz] = i ;
                        // Sx [snz] = Ax [p]
                        memcpy (Sx +(snz*asize), Ax +(p*asize), asize) ;
                        snz++ ;
                    }
                }
            }
        }
        break ;

        //----------------------------------------------------------------------
        // S = userselect (A,k)
        //----------------------------------------------------------------------

        case GB_USER_SELECT_opcode:
        {
            GB_select_function select = op->function  ;
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                Sp [j] = snz ;
                for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                {
                    int64_t i = Ai [p] ;
                    bool keep ;
                    if (A_transpose)
                    {
                        keep = select (j, i, ancols, anrows, Ax +(p*asize), k) ;
                    }
                    else
                    {
                        keep = select (i, j, anrows, ancols, Ax +(p*asize), k) ;
                    }
                    if (keep)
                    {
                        // keep this entry
                        Si [snz] = i ;
                        // Sx [snz] = Ax [p]
                        memcpy (Sx +(snz*asize), Ax +(p*asize), asize) ;
                        snz++ ;
                    }
                }
            }
        }
        break ;
    }

    Sp [ancols] = snz ;
    S->magic = MAGIC ;
    ASSERT_OK (GB_check (S, "S=select(A,k)", 0)) ;

    //--------------------------------------------------------------------------
    // T = S or S' and typecast as needed
    //--------------------------------------------------------------------------

    GrB_Matrix T = NULL ;

    if (A_transpose)
    {
        // T = S', see GrB_transpose for the precasting of T when accum is NULL
        GrB_Type T_type ;
        T_type = (accum == NULL) ? C->type : A->type ;
        GB_NEW (&T, T_type, tnrows, tncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&S) ;
            return (info) ;
        }
        info = GB_Matrix_transpose (T, S, NULL, true) ;
        GB_MATRIX_FREE (&S) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&T) ;
            return (info) ;
        }
    }
    else
    {
        T = S ;
        S = NULL ;
    }

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,T): accumulate the results into C via the Mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, Mask, accum, &T, C_replace, Mask_comp)) ;
}

