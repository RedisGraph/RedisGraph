//------------------------------------------------------------------------------
// GB_subassigner: C(I,J)<#M> = accum (C(I,J), A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Submatrix assignment: C(I,J)<M> = A, or accum (C (I,J), A), no transpose

// All assignment operations rely on this function, including the GrB_*_assign
// operations in the spec, and the GxB_*_subassign operations that are a
// SuiteSparse:GraphBLAS extension to the spec:

// GrB_Matrix_assign,
// GrB_Matrix_assign_TYPE,
// GrB_Vector_assign,
// GrB_Vector_assign_TYPE,
// GrB_Row_assign,
// GrB_Col_assign

// GxB_Matrix_subassign,
// GxB_Matrix_subassign_TYPE,
// GxB_Vector_subassign,
// GxB_Vector_subassign_TYPE,
// GxB_Row_subassign,
// GxB_Col_subassign

// This function handles the accumulator, and the mask M, and the C_replace
// option itself, without relying on GB_accum_mask or GB_mask.  The mask M has
// the same size as C(I,J) and A.  M(0,0) governs how A(0,0) is assigned
// into C(I[0],J[0]).  This is how GxB_subassign operates.  For GrB_assign, the
// mask M in this function is the SubMask, constructed via SubMask=M(I,J).

// No transposed case is handled.  This function is also agnostic about the
// CSR/CSC format of C, A, and M.  The A matrix must have A->vlen == nI and
// A->vdim == nJ (except for scalar expansion, in which case A is NULL).  The
// mask M must be the same size as A, if present.

// Any or all of the C, M, and/or A matrices may be hypersparse or standard
// non-hypersparse.

// C is operated on in-place and thus cannot be aliased with the inputs A or M.

// Since the pattern of C does not change here, C->p, C->h, C->nvec, and
// C->nvec_nonempty are constant.  C->x and C->i can be modified, but only one
// entry at a time.  No entries are shifted.  C->x can be modified, and C->i
// can be changed by turning an entry into a zombie, or by bringing a zombie
// back to life, but no entry in C->i moves in position.

#define GB_FREE_WORK                                    \
{                                                       \
    GB_MATRIX_FREE (&S) ;                               \
    GB_MATRIX_FREE (&A2) ;                              \
    GB_MATRIX_FREE (&M2) ;                              \
    GB_FREE_MEMORY (I2,  ni, sizeof (GrB_Index)) ;      \
    GB_FREE_MEMORY (I2k, ni, sizeof (GrB_Index)) ;      \
    GB_FREE_MEMORY (J2,  nj, sizeof (GrB_Index)) ;      \
    GB_FREE_MEMORY (J2k, nj, sizeof (GrB_Index)) ;      \
}

#include "GB_subassign.h"
#include "GB_subassign_methods.h"
#include "GB_subref.h"
#include "GB_dense.h"
#ifdef GB_DEBUG
#include "GB_iterator.h"
#endif

#undef  GB_FREE_ALL
#define GB_FREE_ALL                                     \
{                                                       \
    GB_PHIX_FREE (C) ;                                  \
    GB_FREE_WORK ;                                      \
}

GrB_Info GB_subassigner             // C(I,J)<#M> = A or accum (C (I,J), A)
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // C matrix descriptor
    const GrB_Matrix M_input,       // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // mask descriptor
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A_input,       // input matrix (NULL for scalar expansion)
    const GrB_Index *I_input,       // list of indices
    const int64_t   ni_input,       // number of indices
    const GrB_Index *J_input,       // list of vector indices
    const int64_t   nj_input,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix S = NULL ;
    GrB_Matrix A2 = NULL ;
    GrB_Matrix M2 = NULL ;

    GrB_Index *GB_RESTRICT I2  = NULL ;
    GrB_Index *GB_RESTRICT I2k = NULL ;
    GrB_Index *GB_RESTRICT J2  = NULL ;
    GrB_Index *GB_RESTRICT J2k = NULL ;

    GrB_Matrix A = A_input ;
    GrB_Matrix M = M_input ;
    int64_t ni = ni_input ;
    int64_t nj = nj_input ;

    // I and J are either the user inputs, or sorted copies
    #define I ((I_jumbled) ? I2 : I_input)
    #define J ((J_jumbled) ? J2 : J_input)

    // GB_subassigner cannot tolerate C==A and C==M aliasing.  A==M is OK.
    ASSERT (C != NULL) ;
    ASSERT (!GB_aliased (C, M)) ;
    ASSERT (!GB_aliased (C, A)) ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C input for subassigner", GB0) ;

    // subassign tolerates both zombies and pending tuples in C, but not M or A
    GB_WAIT (M) ;
    GB_WAIT (A) ;

    //--------------------------------------------------------------------------
    // check mask conditions
    //--------------------------------------------------------------------------

    bool empty_mask = false ;   // true if mask not present and complemented
    bool no_mask = false ;      // true if mask not present and not complemented

    if (M == NULL)
    {
        // the mask is not present
        if (Mask_comp)
        {
            // empty_mask:  mask is not present, and complemented
            empty_mask = true ;
            if (!C_replace)
            { 
                // No work to do.  This the same as the GB_RETURN_IF_QUICK_MASK
                // case in other GraphBLAS functions, except here only the
                // sub-case of C_replace == false is handled.  The C_replace ==
                // true sub-case needs to delete all entries in C(I,J), which
                // is handled below in GB_subassign_00.  This "quick" case is
                // checked again if C_replace becomes effectively false, below.
                GBBURBLE ("quick ") ;
                return (GrB_SUCCESS) ;
            }
        }
        else
        {
            // no_mask:  mask is not present, and not complemented
            no_mask = true ;
            if (C_replace)
            { 
                // The mask is not present and not complemented.  In this case,
                // C_replace is effectively false.  Disable it, since it can
                // force pending tuples to be assembled.  In the comments below
                // "C_replace effectively false" means that either C_replace is
                // false on input, or the mask is not present and not
                // complemented and thus C_replace is set to false here.
                GBBURBLE ("(no mask: C_replace effectively false) ") ;
                C_replace = false ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // check if C is empty
    //--------------------------------------------------------------------------

    bool C_is_empty = (GB_NNZ (C) == 0 && !GB_PENDING (C) && !GB_ZOMBIES (C)) ;
    if (C_is_empty)
    { 
        // C is completely empty.  C_replace is irrelevant, so set it to false.
        // The burble for this case occurs below, after GB_wait (C), since C
        // may become empty if it contains nothing but zombies, or after the
        // GB_clear (C) below.
        C_replace = false ;
    }

    //--------------------------------------------------------------------------
    // get the C matrix
    //--------------------------------------------------------------------------

    int64_t cvlen = C->vlen ;
    int64_t cvdim = C->vdim ;

    // the matrix C may have pending tuples and/or zombies
    ASSERT (GB_PENDING_OK (C)) ; ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    //--------------------------------------------------------------------------
    // determine the length and kind of I and J, and check their properties
    //--------------------------------------------------------------------------

    int64_t nI, nJ, Icolon [3], Jcolon [3] ;
    int Ikind, Jkind ;
    GB_ijlength (I_input, ni, cvlen, &nI, &Ikind, Icolon) ;
    GB_ijlength (J_input, nj, cvdim, &nJ, &Jkind, Jcolon) ;

    // If the descriptor says that A must be transposed, it has already been
    // transposed in the caller.  Thus C(I,J), A, and M (if present) all
    // have the same size: length(I)-by-length(J)

    bool I_unsorted, I_has_dupl, I_contig, J_unsorted, J_has_dupl, J_contig ;
    int64_t imin, imax, jmin, jmax ;
    GB_OK (GB_ijproperties (I_input, ni, nI, cvlen, &Ikind, Icolon,
                &I_unsorted, &I_has_dupl, &I_contig, &imin, &imax, Context)) ;
    GB_OK (GB_ijproperties (J_input, nj, nJ, cvdim, &Jkind, Jcolon,
                &J_unsorted, &J_has_dupl, &J_contig, &jmin, &jmax, Context)) ;

    //--------------------------------------------------------------------------
    // sort I and J and remove duplicates, if needed
    //--------------------------------------------------------------------------

    // If I or J are explicit lists, and either of are unsorted or are sorted
    // but have duplicate entries, then both I and J are sorted and their
    // duplicates are removed.  A and M are adjusted accordingly.  Removing
    // duplicates decreases the length of I and J.

    bool I_jumbled = (I_unsorted || I_has_dupl) ;
    bool J_jumbled = (J_unsorted || J_has_dupl) ;
    bool presort = I_jumbled || J_jumbled ;

    // This pre-sort of I and J is required for the parallel subassign.
    // Otherwise, multiple threads may attempt to modify the same part of C.
    // This could cause a race condition, if one thread flags a zombie at the
    // same time another thread is using that index in a binary search.  If the
    // 2nd thread finds either zombie/not-zombie, this is fine, but the
    // modification would have to be atomic.  Atomic read/write is slow, so to
    // avoid the use of atomics, the index lists I and J are sorted and all
    // duplicates are removed.

    // A side benefit of this pre-sort is that it ensures that the results of
    // GrB_assign and GxB_subassign are fully defined if I and J have
    // duplicates.  The definition of this pre-sort is given in the M-file
    // below.

    /*
        function C = subassign (C, I, J, A)
        % submatrix assignment with pre-sort of I and J; and remove duplicates

        % delete duplicates from I, keeping the last one seen
        [I2 I2k] = sort (I) ;
        Idupl = [(I2 (1:end-1) == I2 (2:end)), false] ;
        I2  = I2  (~Idupl) ;
        I2k = I2k (~Idupl) ;
        assert (isequal (I2, unique (I)))

        % delete duplicates from J, keeping the last one seen
        [J2 J2k] = sort (J) ;
        Jdupl = [(J2 (1:end-1) == J2 (2:end)), false] ;
        J2  = J2  (~Jdupl) ;
        J2k = J2k (~Jdupl) ;
        assert (isequal (J2, unique (J)))

        % do the submatrix assignment, with no duplicates in I2 or J2
        C (I2,J2) = A (I2k,J2k) ;
    */

    // With this subassign script, the result returned by GB_subassigner
    // matches the behavior in MATLAB, so the following holds:

    /*
        C2 = C ;
        C2 (I,J) = A ;
        C3 = subassign (C, I, J, A) ;
        assert (isequal (C2, C3)) ;
    */

    // That is, the pre-sort of I, J, and A has no effect on the final C, in
    // MATLAB.

    // The pre-sort itself takes additional work and memory space, but it may
    // actually improve the performance of GB_subassigner, since it makes
    // the data access of C more regular, even in the sequential case.

    if (presort)
    {

        ASSERT (Ikind == GB_LIST || Jkind == GB_LIST) ;

        if (I_jumbled)
        { 
            // I2 = sort I_input and remove duplicates
            ASSERT (Ikind == GB_LIST) ;
            GB_OK (GB_ijsort (I_input, &ni, &I2, &I2k, Context)) ;
            // Recheck the length and properties of the new I2.  This may
            // convert I2 to GB_ALL or GB_RANGE, after I2 has been sorted.
            GB_ijlength (I2, ni, cvlen, &nI, &Ikind, Icolon) ;
            GB_OK (GB_ijproperties (I2, ni, nI, cvlen, &Ikind, Icolon,
                &I_unsorted, &I_has_dupl, &I_contig, &imin, &imax, Context)) ;
            ASSERT (! (I_unsorted || I_has_dupl)) ;
        }

        if (J_jumbled)
        { 
            // J2 = sort J_input and remove duplicates
            ASSERT (Jkind == GB_LIST) ;
            GB_OK (GB_ijsort (J_input, &nj, &J2, &J2k, Context)) ;
            // Recheck the length and properties of the new J2.  This may
            // convert J2 to GB_ALL or GB_RANGE, after J2 has been sorted.
            GB_ijlength (J2, nj, cvdim, &nJ, &Jkind, Jcolon) ;
            GB_OK (GB_ijproperties (J2, nj, nJ, cvdim, &Jkind, Jcolon,
                &J_unsorted, &J_has_dupl, &J_contig, &jmin, &jmax, Context)) ;
            ASSERT (! (J_unsorted || J_has_dupl)) ;
        }

        if (!scalar_expansion)
        { 
            // A2 = A (I2k, J2k)
            GB_OK (GB_subref (&A2, A->is_csc, A,
                I_jumbled ? I2k : GrB_ALL, ni,
                J_jumbled ? J2k : GrB_ALL, nj, false, true, Context)) ;
            A = A2 ;
        }

        if (M != NULL)
        { 
            // M2 = M (I2k, J2k)
            GB_OK (GB_subref (&M2, M->is_csc, M,
                I_jumbled ? I2k : GrB_ALL, ni,
                J_jumbled ? J2k : GrB_ALL, nj, false, true, Context)) ;
            M = M2 ;
        }

        GB_FREE_MEMORY (I2k, ni, sizeof (GrB_Index)) ;
        GB_FREE_MEMORY (J2k, nj, sizeof (GrB_Index)) ;
    }

    // I and J are now sorted, with no duplicate entries.  They are either
    // GB_ALL, GB_RANGE, or GB_STRIDE, which are intrinsically sorted with no
    // duplicates, or they are explicit GB_LISTs with sorted entries and no
    // duplicates.

    ASSERT (! (I_unsorted || I_has_dupl)) ;
    ASSERT (! (J_unsorted || J_has_dupl)) ;

    //--------------------------------------------------------------------------
    // determine the type and nnz of A (from a scalar or matrix)
    //--------------------------------------------------------------------------

    // also determines if A is dense.  The scalar is always dense.

    // mn = nI * nJ; valid only if mn_ok is true.
    GrB_Index mn ;
    bool mn_ok = GB_Index_multiply (&mn, nI, nJ) ;
    bool A_is_dense ;   // true if A is dense (or scalar expansion)
    int64_t anz ;       // nnz(A), or mn for scalar expansion
    GrB_Type atype ;    // the type of A or the scalar

    if (scalar_expansion)
    { 
        // The input is a scalar; the matrix A is not present.  Scalar
        // expansion results in an implicit dense matrix A whose type is
        // defined by the scalar_code.
        ASSERT (A == NULL) ;
        ASSERT (scalar != NULL) ;
        anz = mn ;
        A_is_dense = true ;
        // a user-defined scalar is assumed to have the same type as C->type
        // which is also user-defined (or else it would not be compatible).
        // Compatibility has already been checked in the caller.  The type of
        // scalar for built-in types is determined by scalar_code, instead,
        // since it can differ from C (in which case it is typecasted into
        // C->type).  User-defined scalars cannot be typecasted.
        atype = GB_code_type (scalar_code, C->type) ;
        ASSERT_TYPE_OK (atype, "atype for scalar expansion", GB0) ;
    }
    else
    { 
        // A is an nI-by-nJ matrix, with no pending computations
        ASSERT_MATRIX_OK (A, "A for subassign kernel", GB0) ;
        ASSERT (nI == A->vlen && nJ == A->vdim) ;
        ASSERT (!GB_PENDING (A)) ;   ASSERT (!GB_ZOMBIES (A)) ;
        ASSERT (scalar == NULL) ;
        anz = GB_NNZ (A) ;
        A_is_dense = (mn_ok && anz == (int64_t) mn) ;
        atype = A->type ;
    }

    //--------------------------------------------------------------------------
    // check the size of the mask
    //--------------------------------------------------------------------------

    // For subassignment, the mask must be |I|-by-|J|

    if (M != NULL)
    { 
        // M can have no pending tuples nor zombies
        ASSERT_MATRIX_OK (M, "M for subassign kernel", GB0) ;
        ASSERT (!GB_PENDING (M)) ;  ASSERT (!GB_ZOMBIES (M)) ;
        ASSERT (nI == M->vlen && nJ == M->vdim) ;
    }

    //--------------------------------------------------------------------------
    // C(:,:) assignment
    //--------------------------------------------------------------------------

    // whole_C_matrix is true if all of C(:,:) is being assigned to
    bool whole_C_matrix = (Ikind == GB_ALL) && (Jkind == GB_ALL) ;

    bool C_splat_scalar = false ;   // C(:,:) = x
    bool C_splat_matrix = false ;   // C(:,:) = A

    if (whole_C_matrix && no_mask && (accum == NULL))
    {

        //----------------------------------------------------------------------
        // C(:,:) = x or A:  whole matrix assignment with no mask
        //----------------------------------------------------------------------

        if (scalar_expansion)
        { 
            // Method 21: C(:,:) = x
            C_splat_scalar = true ;
        }
        else
        { 
            // Method 24: C(:,:) = A
            C_splat_matrix = true ;
        }
        // C_replace is already effectively false (see no_mask case above)
        ASSERT (C_replace == false) ;

        // free pending tuples early but do not clear all of C.  If it is
        // already dense then its pattern can be reused.
        GB_Pending_free (&(C->Pending)) ;

    }
    else if (whole_C_matrix && C_replace && (accum == NULL))
    {

        //----------------------------------------------------------------------
        // C(:,:)<any mask, replace> = A or x, no accum operator present
        //----------------------------------------------------------------------

        // If the entire C(:,:) is being assigned to, and if no accum operator
        // is present, then the matrix can be cleared of all entries now, and
        // then C_replace can be set false.  This can only be done because C is
        // not aliased to M or A on input. which the caller ensures is true.
        // See the assertion above.  Clearing C now speeds up the assignment
        // since the wait on C can be skipped, below.  It also simplifies the
        // kernels.  If S is constructed, it is just an empty matrix.

        GB_OK (GB_clear (C, Context)) ;
        if (C_replace)
        { 
            GBBURBLE ("(C cleared early) ") ;
            C_replace = false ;
        }

        // By clearing C now and setting C_replace to false, the following
        // methods are used: 09 becomes 05, 10 becomes 06n or 06s, 17
        // becomes 13, and 18 becomes 14.  The S matrix for methods 06s,
        // 13, and 14 is still created, but it is very fast to construct
        // and traverse since C is empty.  Method 00 can be skipped since
        // C is already empty (see "quick" case below).

        // prior time             new  time           action
        // ----- ----             ---  ----           ------

        // 00:  O(S)              nothing, O(1)       C already cleared

        // 09:  O(M+S)            05:  O(M)           C<M> = x, no S

        // 10:  O((A+S)*log(m))   06n: O(M*(log(a))   C<M> = A, no S
        //                        06s: O(A*(log(m))   C<M> = A, with S

        // 17:  O(m*n)            13:  O(m*n)         C<!M> = x, with S

        // 18:  O(A*log(m))       14:  O(A*log(m))    C<!M> = A, with S

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============

        //  M   -   -   -   -   -       05:  C(I,J)<M> = x, no S
        //  M   -   -   -   A   -       06n: C(I,J)<M> = A, no S
        //  M   -   -   -   A   S       06s: C(I,J)<M> = A, with S

        //  M   -   r   -   -   S       09:  C(I,J)<M,repl> = x, with S
        //  M   -   r   -   A   S       10:  C(I,J)<M,repl> = A, with S

        //  M   c   -   -   -   S       13:  C(I,J)<!M> = x, with S
        //  M   c   -   -   A   S       14:  C(I,J)<!M> = A, with S

        //  M   c   r   -   -   S       17:  C(I,J)<!M,repl> = x, with S
        //  M   c   r   -   A   S       18:  C(I,J)<!M,repl> = A, with S

        // Methods 09, 10, 17, and 18 are now used only if C(I,J) is a
        // submatrix of C, and not for the whole_C_matrix case.
    }

    //--------------------------------------------------------------------------
    // check compatibilty of prior pending tuples
    //--------------------------------------------------------------------------

    // The action: ( delete ), described below, can only delete a live
    // entry in the pattern.  It cannot delete a pending tuple; pending tuples
    // cannot become zombies.  Thus, if this call to GB_subassigner has the
    // potential for creating zombies, all prior pending tuples must be
    // assembled now.  They thus become live entries in the pattern of C, so
    // that this GB_subassigner can (potentially) turn them into zombies via
    // action: ( delete ).

    // If accum is NULL, the operation is C(I,J) = A, or C(I,J)<M> = A.
    // If A has any implicit zeros at all, or if M is present, then
    // the action: ( delete ) is possible.  This action is taken when an entry
    // is found in C but not A.  It is thus not possible to check A in advance
    // if an entry in C must be deleted.  If an entry does not appear in C but
    // appears as a pending tuple, deleting it would require a scan of all the
    // pending tuples in C.  This is costly, and simply assembling all pending
    // tuples first is faster.

    // The action: ( insert ), described below, adds additional pending tuples.
    // All pending tuples will be assembled sometime later on, using a single
    // pending operator, and thus the current accum operator must match the
    // prior pending operator.  If the operators do not match, then all prior
    // pending tuples must be assembled now, so that this GB_subassigner can
    // (potentially) insert new pending tuples whose pending operator is accum.

    // These tests are conservative because it is possible that this
    // GxB_subassign will not need to use action: ( insert ).

    // In the discussion below, let SECOND_Ctype denote the SECOND operator
    // z=f(x,y) whose ztype, xtype, and ytype matches the type of C.

    bool wait = false ;

    if (C->Pending == NULL)
    { 

        //----------------------------------------------------------------------
        // no pending tuples currently exist
        //----------------------------------------------------------------------

        // If any new pending tuples are added, their pending operator is
        // accum, or the implicit SECOND_Ctype operator if accum is NULL.
        // The type of any pending tuples will become C->type.
        // Prior zombies have no effect on this decision.

        wait = false ;

    }
    else
    {

        //----------------------------------------------------------------------
        // prior pending tuples exist: check if action: ( delete ) can occur
        //----------------------------------------------------------------------

        // action: ( delete ) can only operate on entries in the pattern by
        // turning them into zombies.  It cannot delete prior pending tuples.
        // Thus all prior pending tuples must be assembled first if
        // action: ( delete ) can occur.

        if (C_replace)
        { 
            // C_replace must use the action: ( delete )
            wait = true ;
        }
        else if (accum == NULL)
        {
            // This GxB_subassign can potentially use action: ( delete ), and
            // thus prior pending tuples must be assembled first.  However, if
            // A is completely dense and if there is no mask M, then C(I,J)=A
            // cannot delete any entries from C.

            if (M == NULL && A_is_dense)
            { 
                // A is a dense matrix, so entries cannot be deleted
                wait = false ;
            }
            else
            { 
                // A is sparse or M is present.
                // In this case, action: ( delete ) might occur
                wait = true ;
            }
        }

        //----------------------------------------------------------------------
        // check if pending operator is compatible
        //----------------------------------------------------------------------

        if (!wait)
        {

            // ( delete ) will not occur, but new pending tuples may be added
            // via the action: ( insert ).  Check if the accum operator is the
            // same as the prior pending operator and ensure the types are
            // the same.

            ASSERT (C->Pending != NULL) ;
            ASSERT (C->Pending->type != NULL) ;

            if (atype != C->Pending->type)
            { 
                // entries in A are copied directly into the list of pending
                // tuples for C, with no typecasting.  The type of the prior
                // pending tuples must match the type of A.  Since the types
                // do not match, prior updates must be assembled first.
                wait = true ;
            }
            else if
            (
                // the types match, now check the pending operator
                ! (
                    // the operators are the same
                    (accum == C->Pending->op)
                    // or both operators are SECOND_Ctype, implicit or explicit
                    || (GB_op_is_second (accum, C->type) &&
                        GB_op_is_second (C->Pending->op, C->type))
                  )
            )
            { 
                wait = true ;
            }
        }
    }

    if (wait)
    { 
        // Prior computations are not compatible with this assignment, so all
        // prior work must be finished.  This potentially costly.
        // delete any lingering zombies and assemble any pending tuples
        ASSERT_MATRIX_OK (C, "C before wait", GB0) ;
        GB_OK (GB_wait (C, Context)) ;
    }

    ASSERT_MATRIX_OK (C, "C before subassign", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for assign", GB0) ;

    //--------------------------------------------------------------------------
    // check again if C is empty
    //--------------------------------------------------------------------------

    // GB_clear or GB_wait, above, may have deleted all the zombies in C, so
    // check again if C is empty.
    C_is_empty = (GB_NNZ (C) == 0 && !GB_PENDING (C) && !GB_ZOMBIES (C)) ;
    if (C_is_empty)
    { 
        // C is completely empty.  C_replace is irrelevant, so set it to false.
        GBBURBLE ("(C empty) ") ;
        C_replace = false ;
    }

    //--------------------------------------------------------------------------
    // check "quick" case again
    //--------------------------------------------------------------------------

    if (empty_mask && !C_replace)
    { 
        // The mask is empty (not present, but complemented), and C_replace is
        // now effectively false.  If C_replace was false on input, then the
        // "quick" case above has already been triggered.  However, if C is now
        // empty (either cleared with GB_clear, empty on input, or empty after
        // GB_wait), then C_replace is now effectively false.  In this case,
        // the "quick" case can be checked again.  No more work to do.
        GBBURBLE ("quick ") ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // keep track of the current accum operator
    //--------------------------------------------------------------------------

    // If accum is NULL and pending tuples are added, they will be assembled
    // sometime later (not here) using the implied SECOND_Ctype operator.  This
    // GB_subassigner operation corresponds to C(I,J)=A or C(I,J)<M>=A.
    // Subsequent calls to GrB_setElement, and subsequent calls to GrB_assign
    // or GxB_subassign with an explict SECOND_Ctype operator, may create
    // additional pending tuples and add them to the list without requiring
    // that they be assembled first.

    // If accum is non-NULL, then all prior pending tuples have the same
    // pending operator as this accum.  If that prior operator was the implicit
    // SECOND_Ctype and those pending tuples still exist, then this accum
    // operator is the explicit SECOND_ctype operator.  The implicit
    // SECOND_Ctype operator is replaced with the current accum, which is the
    // explicit SECOND_Ctype operator.

    if (C->Pending != NULL)
    {
        C->Pending->op = accum ;
    }

    //--------------------------------------------------------------------------
    // select the method to use
    //--------------------------------------------------------------------------

    // check if C is competely dense:  all entries present and no pending work.
    bool C_is_dense = !GB_PENDING_OR_ZOMBIES (C) && GB_is_dense (C) ;
    bool C_dense_update = false ;
    if (C_is_dense)
    { 
        GBBURBLE ("(C dense) ") ;
        if (whole_C_matrix && no_mask && (accum != NULL)
            && (C->type == accum->ztype) && (C->type == accum->xtype))
        { 
            // C(:,:) += x or A, where C is dense, no typecasting of C
            C_dense_update = true ;
        }
    }

    // simple_mask: C(I,J)<M> = ... ; or C(I,J)<M> += ...
    bool simple_mask = (!C_replace && M != NULL && !Mask_comp) ;

    // C_Mask_scalar: C(I,J)<M> = scalar or += scalar
    bool C_Mask_scalar = (scalar_expansion && simple_mask) ;

    // C_Mask_matrix:  C(I,J)<M> = A or += A
    bool C_Mask_matrix = (!scalar_expansion && simple_mask) ;

    bool S_Extraction ;
    if (empty_mask)
    { 
        // The mask is not present, but complemented.
        // Method 00: C(I,J)<!,repl> = empty
        S_Extraction = true ;
    }
    else if (C_splat_scalar)
    { 
        // Method 21: C(:,:) = x where x is a scalar; C becomes dense
        S_Extraction = false ;
    }
    else if (C_splat_matrix)
    { 
        // Method 24: C(:,:) = A
        S_Extraction = false ;
    }
    else if (C_dense_update)
    { 
        // Methods 22 and 23: C(:,:) += x or A where C is dense
        S_Extraction = false ;
    }
    else if (C_Mask_scalar)
    { 
        // Method 05*, or 07: C(I,J)<M> = or += scalar; C_replace false
        S_Extraction = false ;
    }
    else if (C_Mask_matrix)
    {
        // C(I,J)<M> = A or += A
        if (accum != NULL)
        { 
            // Method 08: C(I,J)<M> += A
            S_Extraction = false ;
        }
        else
        { 
            // C(I,J)<M> = A ;  use 06s (with S) or 06n (without S)
            // method 06s (with S) is faster when nnz (A) < nnz (M).
            // If M and A are aliased, then nnz (A) == nnz (M), so method
            // 06n is used.
            if (C_is_dense && whole_C_matrix && M == A)
            {
                // Method 06d: C<A> = A
                S_Extraction = false ;
            }
            else if (C_is_empty && whole_C_matrix && A_is_dense && Mask_struct)
            {
                // Method 25: C<M,s> = A, where M is structural, A is
                // dense, and C starts out empty.  The pattern of C will be the
                // same as M, and the subassign method is extremely simple.
                S_Extraction = false ;
            }
            else
            {
                // Method 06n: or Method 06s:
                S_Extraction = (anz < GB_NNZ (M)) ;
            }
        }
    }
    else
    { 
        // all other methods require S
        S_Extraction = true ;
    }

    //--------------------------------------------------------------------------
    // extract the pattern: S = C(I,J) for S_Extraction method, and quick mask
    //--------------------------------------------------------------------------

    // S is a sparse int64_t matrix.  Its "values" are not numerical, but
    // indices into C.  For example, suppose 100 = I [5] and 200 = J [7].  Then
    // S(5,7) is the entry C(I(5),J(7)), and the value of S(5,7) is the
    // position in C that holds that particular entry C(100,200):
    // pC = S->x [...] gives the location of the value C->x [pC] and row index
    // 100 = C->i [pC], and pC will be between C->p [200] ... C->p [200+1]-1
    // if C is non-hypersparse.  If C is hyperparse then pC will be still
    // reside inside the vector jC, in the range C->p [k] ... C->p [k+1]-1,
    // if jC is the kth non-empty vector in the hyperlist of C.

    if (S_Extraction)
    { 

        //----------------------------------------------------------------------
        // extract symbolic structure S=C(I,J)
        //----------------------------------------------------------------------

        // FUTURE::: if whole_C_matrix is true, then C(:,:) = ... and S == C,
        // except that S is zombie-free, read-only; and C collects zombies.

        // FUTURE:: the properties of I and J are already known, and thus do
        // not need to be recomputed by GB_subref.

        // S and C have the same CSR/CSC format.  S is always returned sorted,
        // in the same hypersparse form as C (unless S is empty, in which case
        // it is always returned as hypersparse). This also checks I and J.

        GB_OK (GB_subref (&S, C->is_csc, C, I, ni, J, nj, true, true, Context));

        ASSERT_MATRIX_OK (C, "C for subref extraction", GB0) ;
        ASSERT_MATRIX_OK (S, "S for subref extraction", GB0) ;

        #ifdef GB_DEBUG
        const int64_t *GB_RESTRICT Si = S->i ;
        const int64_t *GB_RESTRICT Sx = S->x ;
        // this body of code explains what S contains.
        // S is nI-by-nJ where nI = length (I) and nJ = length (J)
        GBI_for_each_vector (S)
        {
            // prepare to iterate over the entries of vector S(:,jnew)
            GBI_jth_iteration (jnew, pS_start, pS_end) ;
            // S (inew,jnew) corresponds to C (iC, jC) ;
            // jC = J [j] ; or J is a colon expression
            int64_t jC = GB_ijlist (J, jnew, Jkind, Jcolon) ;
            for (int64_t pS = pS_start ; pS < pS_end ; pS++)
            {
                // S (inew,jnew) is a pointer back into C (I(inew), J(jnew))
                int64_t inew = Si [pS] ;
                ASSERT (inew >= 0 && inew < nI) ;
                // iC = I [iA] ; or I is a colon expression
                int64_t iC = GB_ijlist (I, inew, Ikind, Icolon) ;
                int64_t p = Sx [pS] ;
                ASSERT (p >= 0 && p < GB_NNZ (C)) ;
                int64_t pC_start, pC_end, pleft = 0, pright = C->nvec-1 ;
                bool found = GB_lookup (C->is_hyper, C->h, C->p,
                    &pleft, pright, jC, &pC_start, &pC_end) ;
                ASSERT (found) ;
                // If iC == I [inew] and jC == J [jnew], (or the equivaleent
                // for GB_ALL, GB_RANGE, GB_STRIDE) then A(inew,jnew) will be
                // assigned to C(iC,jC), and p = S(inew,jnew) gives the pointer
                // into C to where the entry (C(iC,jC) appears in C:
                ASSERT (pC_start <= p && p < pC_end) ;
                ASSERT (iC == GB_UNFLIP (C->i [p])) ;
            }
        }
        #endif
    }

    //==========================================================================
    // submatrix assignment C(I,J)<M> = accum (C(I,J),A): meta-algorithm
    //==========================================================================

    // There are up to 64 combinations of options, but not required to be
    // implemented, because they are either identical to another method
    // (C_replace is effectively false if M=NULL and Mask_comp=false), or they
    // are not used (the last option, whether or not S is constructed, is
    // determined here; it is not a user input).  The first 5 options are
    // determined by the input.  The table below has been pruned to remove
    // combinations that are not used, or equivalent to other entries in the
    // table.  Only 22 unique combinations of the 64 combinations are needed,
    // with additional special cases when C(:,:) is dense.

    //      M           present or NULL
    //      Mask_comp   true or false
    //      Mask_struct structural or valued mask
    //      C_replace   true or false
    //      accum       present or NULL
    //      A           scalar (x) or matrix (A)
    //      S           constructed or not 

    // C(I,J)<(M,comp,repl)> ( = , += ) (A, scalar), (with or without S);
    // I and J can be anything for any of these methods (":", colon, or list).

    // See the "No work to do..." comment above:
    // If M is not present, Mask_comp true, C_replace false: no work to do.
    // If M is not present, Mask_comp true, C_replace true: use Method 00
    // If M is not present, Mask_comp false:  C_replace is now false.

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============

        //  -   -   x   -   -   -       21:  C = x, no S, C anything
        //  -   -   x   -   A   -       24:  C = A, no S, C and A anything
        //  -   -   -   +   -   -       22:  C += x, no S, C dense
        //  -   -   -   +   A   -       23:  C += A, no S, C dense

        //  -   -   -   -   -   S       01:  C(I,J) = x, with S
        //  -   -   -   -   A   S       02:  C(I,J) = A, with S
        //  -   -   -   +   -   S       03:  C(I,J) += x, with S
        //  -   -   -   +   A   S       04:  C(I,J) += A, with S
        //  -   -   r                        uses methods 01, 02, 03, 04
        //  -   c   -                        no work to do
        //  -   c   r           S       00:  C(I,J)<!,repl> = empty, with S

        //  M   -   -   -   -   -       05d: C<M> = x, no S, C dense
        //  M   -   -   -   -   -       05e: C<M,s> = x, no S, C empty
        //  M   -   -   -   -   -       05:  C(I,J)<M> = x, no S
        //  A   -   -   -   A   -       06d: C<A> = A, no S, C dense
        //  M   -   -   -   A   -       20:  C<M,s> = A, A dense, C empty
        //  M   -   -   -   A   -       06n: C(I,J)<M> = A, no S
        //  M   -   -   -   A   S       06s: C(I,J)<M> = A, with S
        //  M   -   -   +   -   -       07:  C(I,J)<M> += x, no S
        //  M   -   -   +   A   -       08:  C(I,J)<M> += A, no S
        //  M   -   r   -   -   S       09:  C(I,J)<M,repl> = x, with S
        //  M   -   r   -   A   S       10:  C(I,J)<M,repl> = A, with S
        //  M   -   r   +   -   S       11:  C(I,J)<M,repl> += x, with S
        //  M   -   r   +   A   S       12:  C(I,J)<M,repl> += A, with S

        //  M   c   -   -   -   S       13:  C(I,J)<!M> = x, with S
        //  M   c   -   -   A   S       14:  C(I,J)<!M> = A, with S
        //  M   c   -   +   -   S       15:  C(I,J)<!M> += x, with S
        //  M   c   -   +   A   S       16:  C(I,J)<!M> += A, with S
        //  M   c   r   -   -   S       17:  C(I,J)<!M,repl> = x, with S
        //  M   c   r   -   A   S       18:  C(I,J)<!M,repl> = A, with S
        //  M   c   r   +   -   S       19:  C(I,J)<!M,repl> += x, with S
        //  M   c   r   +   A   S       20:  C(I,J)<!M,repl> += A, with S

        //----------------------------------------------------------------------
        // FUTURE::: C<C,s> = x    C == M, replace all values, C_replace ignored
        // FUTURE::: C<C,s> += x   C == M, update all values, C_replace ignored
        // FUTURE::: C<C,s> = A    C == M, A dense, C_replace ignored
        //----------------------------------------------------------------------

    // For the single case C(I,J)<M>=A, two methods can be used: 06n and 06s.

    #define Istring ((Ikind == GB_ALL) ? ":" : "I")
    #define Jstring ((Jkind == GB_ALL) ? ":" : "I")

    if (empty_mask)
    { 

        //----------------------------------------------------------------------
        // C(I,J)<!,repl> = empty
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  -   c   r           S       00:  C(I,J)<!,repl> = empty, with S

        ASSERT (C_replace) ;
        ASSERT (S != NULL) ;

        // Method 00: C(I,J) = empty ; using S
        GBBURBLE ("Method 00: C(%s,%s) = empty ; using S ",
            Istring, Jstring) ;
        GB_OK (GB_subassign_00 (C,
            I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
            S, Context)) ;

    }
    else if (C_splat_scalar)
    { 

        //----------------------------------------------------------------------
        // C = x where x is a scalar; C becomes dense
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============

        //  -   -   x   -   -   -       21:  C = x, no S, C anything

        ASSERT (whole_C_matrix) ;           // C(:,:) is modified
        ASSERT (M == NULL) ;                // no mask present
        ASSERT (accum == NULL) ;            // accum is not present
        ASSERT (!C_replace) ;               // C_replace is effectively false
        ASSERT (S == NULL) ;                // S is not used
        ASSERT (scalar_expansion) ;         // x is a scalar

        // Method 21: C = x where x is a scalar; C becomes dense
        GBBURBLE ("Method 21: (C dense) = scalar ") ;
        GB_OK (GB_dense_subassign_21 (C, scalar, atype, Context)) ;

    }
    else if (C_splat_matrix)
    { 

        //----------------------------------------------------------------------
        // C = A
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============

        //  -   -   x   -   A   -       24:  C = A, no S, C and A anything

        ASSERT (whole_C_matrix) ;           // C(:,:) is modified
        ASSERT (M == NULL) ;                // no mask present
        ASSERT (accum == NULL) ;            // accum is not present
        ASSERT (!C_replace) ;               // C_replace is effectively false
        ASSERT (S == NULL) ;                // S is not used
        ASSERT (!scalar_expansion) ;        // A is a matrix

        // Method 24: C = A
        GBBURBLE ("Method 24: C = Z ") ;
        GB_OK (GB_dense_subassign_24 (C, A, Context)) ;

    }
    else if (C_dense_update)
    { 

        //----------------------------------------------------------------------
        // C += A or x where C is dense
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  -   -   -   +   -   -       22:  C += x, no S, C dense
        //  -   -   -   +   A   -       23:  C += A, no S, C dense

        ASSERT (C_is_dense) ;               // C is dense
        ASSERT (whole_C_matrix) ;           // C(:,:) is modified
        ASSERT (M == NULL) ;                // no mask present
        ASSERT (accum != NULL) ;            // accum is present
        ASSERT (!C_replace) ;               // C_replace is false
        ASSERT (S == NULL) ;                // S is not used

        if (scalar_expansion)
        {
            // Method 22: C(:,:) += x where C is dense
            GBBURBLE ("Method 22: (C dense) += scalar ") ;
            GB_OK (GB_dense_subassign_22 (C, scalar, atype, accum, Context)) ;
        }
        else
        {
            // Method 23: C(:,:) += A where C is dense
            GBBURBLE ("Method 23: (C dense) += Z ") ;
            GB_OK (GB_dense_subassign_23 (C, A, accum, Context)) ;
        }

    }
    else if (C_Mask_scalar)
    {

        //----------------------------------------------------------------------
        // C(I,J)<M> = scalar or +=scalar
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  M   -   -   -   -   -       05d: C(:,:)<M> = x, no S, C dense
        //  M   -   -   -   -   -       05e: C(:,:)<M,s> = x, no S, C empty
        //  M   -   -   -   -   -       05:  C(I,J)<M> = x, no S
        //  M   -   -   +   -   -       07:  C(I,J)<M> += x, no S

        ASSERT (scalar_expansion) ;         // A is a scalar
        ASSERT (M != NULL && !Mask_comp) ;  // mask M present, not compl.
        ASSERT (!C_replace) ;               // C_replace is false
        ASSERT (S == NULL) ;                // S is not used

        if (accum == NULL)
        {
            if (C_is_empty && whole_C_matrix && Mask_struct)
            { 
                // Method 05e: C(:,:)<M> = scalar ; no S; C empty, M structural
                GBBURBLE ("Method 05e: (C empty)<M> = scalar ") ;
                GB_OK (GB_subassign_05e (C, M, scalar, atype, Context)) ;
            }
            else if (C_is_dense && whole_C_matrix)
            { 
                // Method 05d: C(:,:)<M> = scalar ; no S; C is dense
                GBBURBLE ("Method 05d: (C dense)<M> = scalar ") ;
                GB_OK (GB_dense_subassign_05d (C,
                    M, Mask_struct, scalar, atype, Context)) ;
            }
            else
            { 
                // Method 05: C(I,J)<M> = scalar ; no S
                GBBURBLE ("Method 05: C(%s,%s)<M> = scalar ; no S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_05 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, scalar, atype, Context)) ;
            }
        }
        else
        { 
            // Method 07: C(I,J)<M> += scalar ; no S
            GBBURBLE ("Method 07: C(%s,%s)<M> += scalar ; no S",
                Istring, Jstring) ;
            GB_OK (GB_subassign_07 (C,
                I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                M, Mask_struct, accum, scalar, atype, Context)) ;
        }

    }
    else if (C_Mask_matrix)
    {

        //----------------------------------------------------------------------
        // C(I,J)<M> = A or += A
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  M   -   -   +   A   -       08:  C(I,J)<M> += A, no S
        //  A   -   -   -   A   -       06d: C<A> = A, no S, C dense
        //  M   -   x   -   A   -       25:  C<M,s> = A, A dense, C empty
        //  M   -   -   -   A   -       06n: C(I,J)<M> = A, no S
        //  M   -   -   -   A   S       06s: C(I,J)<M> = A, with S

        ASSERT (!scalar_expansion) ;        // A is a matrix
        ASSERT (M != NULL && !Mask_comp) ;  // mask M present, not compl.
        ASSERT (!C_replace) ;

        if (accum != NULL)
        { 
            // Method 08: C(I,J)<M> += A ; no S
            GBBURBLE ("Method 08: C(%s,%s)<M> += Z ; no S ",
                Istring, Jstring) ;
            ASSERT (S == NULL) ;
            GB_OK (GB_subassign_08 (C,
                I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                M, Mask_struct, accum, A, Context)) ;
        }
        else if (C_is_dense && whole_C_matrix && M == A)
        { 
            // Method 06d: C(:,:)<A> = A ; no S, C dense
            GBBURBLE ("Method 06d: (C dense)<Z> = Z ") ;
            GB_OK (GB_dense_subassign_06d (C, A, Mask_struct, Context)) ;
        }
        else if (C_is_empty && whole_C_matrix && A_is_dense && Mask_struct)
        { 
            GBBURBLE ("Method 25: (C empty)<M> = (Z dense) ") ;
            GB_OK (GB_dense_subassign_25 (C, M, A, Context)) ;
        }
        else if (S == NULL)
        { 
            // Method 06n: C(I,J)<M> = A ; no S
            GBBURBLE ("Method 06n: C(%s,%s)<M> = Z ; no S ",
                Istring, Jstring) ;
            GB_OK (GB_subassign_06n (C,
                I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                M, Mask_struct, A, Context)) ;
        }
        else
        { 
            // Method 06s: C(I,J)<M> = A ; using S
            GBBURBLE ("Method 06s: C(%s,%s)<M> = Z ; using S ",
                Istring, Jstring) ;
            GB_OK (GB_subassign_06s (C,
                I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                M, Mask_struct, A, S, Context)) ;
        }

    }
    else if (M == NULL)
    {

        //----------------------------------------------------------------------
        // assignment using S_Extraction method, no mask M
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  -   -   -   -   -   S       01:  C(I,J) = x, with S
        //  -   -   -   -   A   S       02:  C(I,J) = A, with S
        //  -   -   -   +   -   S       03:  C(I,J) += x, with S
        //  -   -   -   +   A   S       04:  C(I,J) += A, with S

        ASSERT (!Mask_comp) ;
        ASSERT (!C_replace) ;
        ASSERT (S != NULL) ;

        if (scalar_expansion)
        {
            if (accum == NULL)
            { 
                // Method 01: C(I,J) = scalar ; using S
                GBBURBLE ("Method 01: C(%s,%s) = scalar ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_01 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    scalar, atype, S, Context)) ;
            }
            else
            { 
                // Method 03: C(I,J) += scalar ; using S
                GBBURBLE ("Method 03: C(%s,%s) += scalar ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_03 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    accum, scalar, atype, S, Context)) ;
            }
        }
        else
        {
            if (accum == NULL)
            { 
                // Method 02: C(I,J) = A ; using S
                GBBURBLE ("Method 02: C(%s,%s) = Z ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_02 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    A, S, Context)) ;
            }
            else
            { 
                // Method 04: C(I,J) += A ; using S
                GBBURBLE ("Method 04: C(%s,%s) += Z ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_04 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    accum, A, S, Context)) ;
            }
        }

    }
    else if (scalar_expansion)
    {

        //----------------------------------------------------------------------
        // C(I,J)<#M> = scalar or += scalar ; using S
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  M   -   r   -   -   S       09:  C(I,J)<M,repl> = x, with S
        //  M   -   r   +   -   S       11:  C(I,J)<M,repl> += x, with S
        //  M   c   -   -   -   S       13:  C(I,J)<!M> = x, with S
        //  M   c   -   +   -   S       15:  C(I,J)<!M> += x, with S
        //  M   c   r   -   -   S       17:  C(I,J)<!M,repl> = x, with S
        //  M   c   r   +   -   S       19:  C(I,J)<!M,repl> += x, with S

        ASSERT (!C_Mask_scalar) ;
        ASSERT (C_replace || Mask_comp) ;
        ASSERT (S != NULL) ;

        if (accum == NULL)
        {
            if (Mask_comp && C_replace)
            { 
                // Method 17: C(I,J)<!M,repl> = scalar ; using S
                GBBURBLE ("Method 17: C(%s,%s)<!M,repl> = scalar ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_17 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, scalar, atype, S, Context)) ;
            }
            else if (Mask_comp)
            { 
                // Method 13: C(I,J)<!M> = scalar ; using S
                GBBURBLE ("Method 13: C(%s,%s)<!M> = scalar ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_13 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, scalar, atype, S, Context)) ;
            }
            else // if (C_replace)
            { 
                // Method 09: C(I,J)<M,repl> = scalar ; using S
                GBBURBLE ("Method 09: C(%s,%s)<M,repl> = scalar ; using S ",
                    Istring, Jstring) ;
                ASSERT (C_replace) ;
                GB_OK (GB_subassign_09 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, scalar, atype, S, Context)) ;
            }
        }
        else
        {
            if (Mask_comp && C_replace)
            { 
                // Method 19: C(I,J)<!M,repl> += scalar ; using S
                GBBURBLE ("Method 19: C(%s,%s)<!M,repl> += scalar ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_19 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, accum, scalar, atype, S, Context)) ;
            }
            else if (Mask_comp)
            { 
                // Method 15: C(I,J)<!M> += scalar ; using S
                GBBURBLE ("Method 15: C(%s,%s)<!M> += scalar ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_15 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, accum, scalar, atype, S, Context)) ;
            }
            else // if (C_replace)
            { 
                // Method 11: C(I,J)<M,repl> += scalar ; using S
                GBBURBLE ("Method 11: C(%s,%s)<M,repl> += scalar ; using S ",
                    Istring, Jstring) ;
                ASSERT (C_replace) ;
                GB_OK (GB_subassign_11 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, accum, scalar, atype, S, Context)) ;
            }
        }

    }
    else
    {

        //------------------------------------------------------------------
        // C(I,J)<#M> = A or += A ; using S
        //------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  M   -   r   -   A   S       10:  C(I,J)<M,repl> = A, with S
        //  M   -   r   +   A   S       12:  C(I,J)<M,repl> += A, with S
        //  M   c   -   -   A   S       14:  C(I,J)<!M> = A, with S
        //  M   c   -   +   A   S       16:  C(I,J)<!M> += A, with S
        //  M   c   r   -   A   S       18:  C(I,J)<!M,repl> = A, with S
        //  M   c   r   +   A   S       20:  C(I,J)<!M,repl> += A, with S

        ASSERT (Mask_comp || C_replace) ;
        ASSERT (S != NULL) ;

        if (accum == NULL)
        {
            if (Mask_comp && C_replace)
            { 
                // Method 18: C(I,J)<!M,repl> = A ; using S
                GBBURBLE ("Method 18: C(%s,%s)<!M,repl> = Z ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_18 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, A, S, Context)) ;
            }
            else if (Mask_comp)
            { 
                // Method 14: C(I,J)<!M> = A ; using S
                GBBURBLE ("Method 14: C(%s,%s)<!M> = Z ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_14 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, A, S, Context)) ;
            }
            else // if (C_replace)
            { 
                // Method 10: C(I,J)<M,repl> = A ; using S
                GBBURBLE ("Method 10: C(%s,%s)<M,repl> = Z ; using S ",
                    Istring, Jstring) ;
                ASSERT (C_replace) ;
                GB_OK (GB_subassign_10 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, A, S, Context)) ;
            }
        }
        else
        {
            if (Mask_comp && C_replace)
            { 
                // Method 20: C(I,J)<!M,repl> += A ; using S
                GBBURBLE ("Method 20: C(%s,%s)<!M,repl> += Z ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_20 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, accum, A, S, Context)) ;
            }
            else if (Mask_comp)
            { 
                // Method 16: C(I,J)<!M> += A ; using S
                GBBURBLE ("Method 16: C(%s,%s)<!M> += Z ; using S ",
                    Istring, Jstring) ;
                GB_OK (GB_subassign_16 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, accum, A, S, Context)) ;
            }
            else // if (C_replace)
            { 
                // Method 12: C(I,J)<M,repl> += A ; using S
                GBBURBLE ("Method 12: C(%s,%s)<M,repl> += Z ; using S ",
                    Istring, Jstring) ;
                ASSERT (C_replace) ;
                GB_OK (GB_subassign_12 (C,
                    I, nI, Ikind, Icolon, J, nJ, Jkind, Jcolon,
                    M, Mask_struct, accum, A, S, Context)) ;
            }
            // note that C(I,J)<M> += A always uses method 6b, without S.
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;

    //--------------------------------------------------------------------------
    // insert C in the queue if it has work to do and isn't already queued
    //--------------------------------------------------------------------------

    if (C->nzombies == 0 && C->Pending == NULL)
    { 
        // C may be in the queue from a prior assignment, but this assignemt
        // can bring zombies back to life, and the zombie count can go to zero.
        // In that case, C must be removed from the queue.  The removal does
        // nothing if C is already not in the queue.

        // FUTURE:: this might cause thrashing if lots of assigns or
        // setElements are done in parallel.  Instead, leave the matrix in the
        // queue, and allow matrices to be in the queue even if they have no
        // unfinished computations.  See also GB_setElement.

        GB_CRITICAL (GB_queue_remove (C)) ;
    }
    else
    { 
        // If C has any zombies or pending tuples, it must be in the queue.
        // The queue insert does nothing if C is already in the queue.
        GB_CRITICAL (GB_queue_insert (C)) ;
    }

    //--------------------------------------------------------------------------
    // finalize C and return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C(I,J) result", GB0) ;
    return (GB_block (C, Context)) ;
}

