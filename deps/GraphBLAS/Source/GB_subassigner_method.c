//------------------------------------------------------------------------------
// GB_subassigner_method: determine method for GB_subassign
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_subassign.h"

int GB_subassigner_method           // return method to use in GB_subassigner
(
    const GrB_Matrix C,             // input/output matrix for results
    const bool C_replace,           // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // mask descriptor
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A,             // input matrix (NULL for scalar expansion)
    const int Ikind,
    const int Jkind,
    const bool scalar_expansion     // if true, expand scalar to A
)
{

    //--------------------------------------------------------------------------
    // check mask conditions
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    // empty_mask: mask not present and complemented.  This condition has
    // already handled by GB_assign_prep.
    bool empty_mask = (M == NULL) && Mask_comp ;
    ASSERT (!empty_mask) ;
    #endif

    // no_mask: mask not present and not complemented
    bool no_mask = (M == NULL) && !Mask_comp ;

    // GB_assign_prep has already disabled C_replace if no mask present
    ASSERT (GB_IMPLIES (no_mask, !C_replace)) ;

    bool M_is_bitmap = GB_IS_BITMAP (M) ;

    //--------------------------------------------------------------------------
    // check if C is empty
    //--------------------------------------------------------------------------

    bool C_is_empty = (GB_NNZ (C) == 0 && !GB_PENDING (C) && !GB_ZOMBIES (C)) ;

    // if C is empty, C_replace is effectively false and already disabled
    ASSERT (GB_IMPLIES (C_is_empty, !C_replace)) ;

    //--------------------------------------------------------------------------
    // check if A is dense; a scalar is an implicit dense matrix, or bitmap
    //--------------------------------------------------------------------------

    bool A_is_bitmap = GB_IS_BITMAP (A) ;

    //--------------------------------------------------------------------------
    // determine the method to use
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
    }

    // check if C is competely dense:  all entries present and no pending work.
    bool C_is_bitmap = GB_IS_BITMAP (C) ;
    bool C_as_if_full = GB_as_if_full (C) ;
    bool C_dense_update = false ;
    if (C_as_if_full)
    { 
        // C is dense with no pending work
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
    bool method_06d = false ;
    bool method_25 = false ;

    if (C_splat_scalar)
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
            // Method 08n: C(I,J)<M> += A, no S.  Cannot use M or A as bitmap.
            // Method 08s: C(I,J)<M> += A, with S.  Can use M or A as bitmap.
            // if S_Extraction is true, Method 08s is used (with S).
            // Method 08n is not used if any matrix is bitmap.
            // If C is bitmap, GB_bitmap_assign_M_accum is used instead.
            S_Extraction = M_is_bitmap || A_is_bitmap ;
        }
        else
        { 
            // C(I,J)<M> = A ;  use 06s (with S) or 06n (without S)
            // method 06s (with S) is faster when nnz (A) < nnz (M).
            if ((C_as_if_full || C_is_bitmap) && whole_C_matrix && M == A)
            {
                // Method 06d: C<A> = A
                method_06d = true ;
                S_Extraction = false ;
            }
            else if (C_is_empty && whole_C_matrix && Mask_struct &&
                (scalar_expansion || GB_as_if_full (A) || A_is_bitmap))
            {
                // Method 25: C<M,s> = A, where M is structural, A is
                // dense, and C starts out empty.  The pattern of C will be the
                // same as M, and the subassign method is extremely simple.
                method_25 = true ;
                S_Extraction = false ;
            }
            else
            {
                // Method 06n (no S) or Method 06s (with S):
                // Method 06n is not used if M or A are bitmap.  If M and A are
                // aliased and Method 06d is not used, then 06s is used instead
                // of 06n since M==A implies nnz(A) == nnz(M).
                S_Extraction = (GB_NNZ (A) < GB_NNZ (M))
                    || M_is_bitmap || A_is_bitmap ;
            }
        }
    }
    else
    { 
        // all other methods require S
        S_Extraction = true ;
    }

    //==========================================================================
    // submatrix assignment C(I,J)<M> = accum (C(I,J),A): meta-algorithm
    //==========================================================================

    // There are up to 128 combinations of options, but not all must be
    // implemented, because they are either identical to another method
    // (C_replace is effectively false if M=NULL and Mask_comp=false), or they
    // are not used (the last option, whether or not S is constructed, is
    // determined here; it is not a user input).  The first 5 options are
    // determined by the input.  The table below has been pruned to remove
    // combinations that are not used, or equivalent to other entries in the
    // table.  Only 22 unique combinations of the 128 combinations are needed,
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
        //  M   -   -   -   A   -       25:  C<M,s> = A, A dense, C empty
        //  M   -   -   -   A   -       06n: C(I,J)<M> = A, no S
        //  M   -   -   -   A   S       06s: C(I,J)<M> = A, with S
        //  M   -   -   +   -   -       07:  C(I,J)<M> += x, no S
        //  M   -   -   +   A   -       08n: C(I,J)<M> += A, no S
        //  M   -   -   +   A   -       08s: C(I,J)<M> += A, with S
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
        // FUTURE::: 8 simpler cases when I and J are ":" (S not needed):
        //----------------------------------------------------------------------

        //  M   -   -   -   A   -       06x: C(:,:)<M> = A
        //  M   -   -   +   A   -       08x: C(:,:)<M> += A
        //  M   -   r   -   A   -       10x: C(:,:)<M,repl> = A
        //  M   -   r   +   A   -       12x: C(:,:)<M,repl> += A
        //  M   c   -   -   A   -       14x: C(:,:)<!M> = A
        //  M   c   -   +   A   -       16x: C(:,:)<!M> += A
        //  M   c   r   -   A   -       18x: C(:,:)<!M,repl> = A
        //  M   c   r   +   A   -       20x: C(:,:)<!M,repl> += A

        //----------------------------------------------------------------------
        // FUTURE::: C<C,s> = x    C == M, replace all values, C_replace ignored
        // FUTURE::: C<C,s> += x   C == M, update all values, C_replace ignored
        // FUTURE::: C<C,s> = A    C == M, A dense, C_replace ignored
        //----------------------------------------------------------------------

    // For the single case C(I,J)<M>=A, two methods can be used: 06n and 06s.

    int subassign_method = -1 ;

    if (C_splat_scalar)
    { 

        //----------------------------------------------------------------------
        // C = x where x is a scalar; C becomes full
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============

        //  -   -   x   -   -   -       21:  C = x, no S, C anything

        ASSERT (whole_C_matrix) ;           // C(:,:) is modified
        ASSERT (M == NULL) ;                // no mask present
        ASSERT (accum == NULL) ;            // accum is not present
        ASSERT (!C_replace) ;               // C_replace is effectively false
        ASSERT (!S_Extraction) ;            // S is not used
        ASSERT (scalar_expansion) ;         // x is a scalar

        // Method 21: C = x where x is a scalar; C becomes full
        subassign_method = GB_SUBASSIGN_METHOD_21 ;

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
        ASSERT (!S_Extraction) ;            // S is not used
        ASSERT (!scalar_expansion) ;        // A is a matrix

        // Method 24: C = A
        subassign_method = GB_SUBASSIGN_METHOD_24 ;

    }
    else if (C_dense_update)
    { 

        //----------------------------------------------------------------------
        // C += A or x where C is dense or full (and becomes full)
        //----------------------------------------------------------------------

        //  =====================       ==============
        //  M   cmp rpl acc A   S       method: action
        //  =====================       ==============
        //  -   -   -   +   -   -       22:  C += x, no S, C dense
        //  -   -   -   +   A   -       23:  C += A, no S, C dense

        ASSERT (C_as_if_full) ;             // C is dense
        ASSERT (whole_C_matrix) ;           // C(:,:) is modified
        ASSERT (M == NULL) ;                // no mask present
        ASSERT (accum != NULL) ;            // accum is present
        ASSERT (!C_replace) ;               // C_replace is false
        ASSERT (!S_Extraction) ;            // S is not used

        if (scalar_expansion)
        {
            // Method 22: C(:,:) += x where C is dense or full
            subassign_method = GB_SUBASSIGN_METHOD_22 ;
        }
        else
        {
            // Method 23: C(:,:) += A where C is dense or full
            subassign_method = GB_SUBASSIGN_METHOD_23 ;
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
        ASSERT (!S_Extraction) ;            // S is not used

        if (accum == NULL)
        {
            if (C_is_empty && whole_C_matrix && Mask_struct)
            { 
                // Method 05e: C(:,:)<M> = scalar ; no S; C empty, M structural
                subassign_method = GB_SUBASSIGN_METHOD_05e ;
            }
            else if (C_as_if_full && whole_C_matrix)
            { 
                // Method 05d: C(:,:)<M> = scalar ; no S; C is dense or full;
                // C becomes full.
                subassign_method = GB_SUBASSIGN_METHOD_05d ;
            }
            else
            { 
                // Method 05: C(I,J)<M> = scalar ; no S
                subassign_method = GB_SUBASSIGN_METHOD_05 ;
            }
        }
        else
        { 
            // Method 07: C(I,J)<M> += scalar ; no S
            subassign_method = GB_SUBASSIGN_METHOD_07 ;
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
        //  M   -   -   +   A   -       08n:  C(I,J)<M> += A, no S
        //  M   -   -   +   A   -       08s:  C(I,J)<M> += A, with S
        //  A   -   -   -   A   -       06d: C<A> = A, no S, C dense
        //  M   -   x   -   A   -       25:  C<M,s> = A, A dense, C empty
        //  M   -   -   -   A   -       06n: C(I,J)<M> = A, no S
        //  M   -   -   -   A   S       06s: C(I,J)<M> = A, with S

        ASSERT (!scalar_expansion) ;        // A is a matrix
        ASSERT (M != NULL && !Mask_comp) ;  // mask M present, not compl.
        ASSERT (!C_replace) ;

        if (accum != NULL)
        { 
            if (S_Extraction)
            {
                // Method 08s: C(I,J)<M> += A ; with S
                subassign_method = GB_SUBASSIGN_METHOD_08s ;
            }
            else
            {
                // Method 08n: C(I,J)<M> += A ; no S
                // No matrix can be bitmap.
                subassign_method = GB_SUBASSIGN_METHOD_08n ;
            }
        }
        else if (method_06d)
        { 
            // Method 06d: C(:,:)<A> = A ; no S, C dense or full;
            subassign_method = GB_SUBASSIGN_METHOD_06d ;
            ASSERT ((C_as_if_full || C_is_bitmap) && whole_C_matrix && M == A) ;
            ASSERT ((C_as_if_full || C_is_bitmap) && whole_C_matrix && M == A) ;
        }
        else if (method_25)
        { 
            // Method 25: C<M,struct> = A, C empty; A is dense, full, or bitmap
            subassign_method = GB_SUBASSIGN_METHOD_25 ;
        }
        else if (!S_Extraction)
        { 
            // Method 06n: C(I,J)<M> = A ; no S
            // If M or A are bitmap, this method is not used;
            // 06s is used instead.
            subassign_method = GB_SUBASSIGN_METHOD_06n ;
        }
        else
        { 
            // Method 06s: C(I,J)<M> = A ; using S
            subassign_method = GB_SUBASSIGN_METHOD_06s ;
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
        ASSERT (S_Extraction) ;            // S is used

        if (scalar_expansion)
        {
            if (accum == NULL)
            { 
                // Method 01: C(I,J) = scalar ; using S
                subassign_method = GB_SUBASSIGN_METHOD_01 ;
            }
            else
            { 
                // Method 03: C(I,J) += scalar ; using S
                subassign_method = GB_SUBASSIGN_METHOD_03 ;
            }
        }
        else
        {
            if (accum == NULL)
            { 
                // Method 02: C(I,J) = A ; using S
                subassign_method = GB_SUBASSIGN_METHOD_02 ;
            }
            else
            { 
                // Method 04: C(I,J) += A ; using S
                subassign_method = GB_SUBASSIGN_METHOD_04 ;
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
        ASSERT (S_Extraction) ;            // S is used

        if (accum == NULL)
        {
            if (Mask_comp && C_replace)
            { 
                // Method 17: C(I,J)<!M,repl> = scalar ; using S
                subassign_method = GB_SUBASSIGN_METHOD_17 ;
            }
            else if (Mask_comp)
            { 
                // Method 13: C(I,J)<!M> = scalar ; using S
                subassign_method = GB_SUBASSIGN_METHOD_13 ;
            }
            else // if (C_replace)
            { 
                // Method 09: C(I,J)<M,repl> = scalar ; using S
                ASSERT (C_replace) ;
                subassign_method = GB_SUBASSIGN_METHOD_09 ;
            }
        }
        else
        {
            if (Mask_comp && C_replace)
            { 
                // Method 19: C(I,J)<!M,repl> += scalar ; using S
                subassign_method = GB_SUBASSIGN_METHOD_19 ;
            }
            else if (Mask_comp)
            { 
                // Method 15: C(I,J)<!M> += scalar ; using S
                subassign_method = GB_SUBASSIGN_METHOD_15 ;
            }
            else // if (C_replace)
            { 
                // Method 11: C(I,J)<M,repl> += scalar ; using S
                ASSERT (C_replace) ;
                subassign_method = GB_SUBASSIGN_METHOD_11 ;
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
        ASSERT (S_Extraction) ;            // S is used

        if (accum == NULL)
        {
            if (Mask_comp && C_replace)
            { 
                // Method 18: C(I,J)<!M,repl> = A ; using S
                subassign_method = GB_SUBASSIGN_METHOD_18 ;
            }
            else if (Mask_comp)
            { 
                // Method 14: C(I,J)<!M> = A ; using S
                subassign_method = GB_SUBASSIGN_METHOD_14 ;
            }
            else // if (C_replace)
            { 
                // Method 10: C(I,J)<M,repl> = A ; using S
                ASSERT (C_replace) ;
                subassign_method = GB_SUBASSIGN_METHOD_10 ;
            }
        }
        else
        {
            if (Mask_comp && C_replace)
            { 
                // Method 20: C(I,J)<!M,repl> += A ; using S
                subassign_method = GB_SUBASSIGN_METHOD_20 ;
            }
            else if (Mask_comp)
            { 
                subassign_method = GB_SUBASSIGN_METHOD_16 ;
            }
            else // if (C_replace)
            { 
                // Method 12: C(I,J)<M,repl> += A ; using S
                ASSERT (C_replace) ;
                subassign_method = GB_SUBASSIGN_METHOD_12 ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // determine if the subassign method can handle this case for bitmaps
    //--------------------------------------------------------------------------

    bool C_is_full = GB_IS_FULL (C) ;

    #define GB_USE_BITMAP_IF(condition) \
        if (condition) subassign_method = GB_SUBASSIGN_METHOD_BITMAP ;

    switch (subassign_method)
    {

        //----------------------------------------------------------------------
        // scalar assignent methods
        //----------------------------------------------------------------------

        case GB_SUBASSIGN_METHOD_01 :   // C(I,J) = scalar
        case GB_SUBASSIGN_METHOD_03 :   // C(I,J) += scalar
        case GB_SUBASSIGN_METHOD_05 :   // C(I,J)<M> = scalar
        case GB_SUBASSIGN_METHOD_07 :   // C(I,J)<M> += scalar
        case GB_SUBASSIGN_METHOD_13 :   // C(I,J)<!M> = scalar
        case GB_SUBASSIGN_METHOD_15 :   // C(I,J)<!M> += scalar
        case GB_SUBASSIGN_METHOD_21 :   // C(:,:) = scalar
            // M can have any sparsity structure, including bitmap
            GB_USE_BITMAP_IF (C_is_bitmap) ;
            break ;

        case GB_SUBASSIGN_METHOD_05d :  // C(:,:)<M> = scalar ; C is dense
        case GB_SUBASSIGN_METHOD_05e :  // C(:,:)<M,struct> = scalar
        case GB_SUBASSIGN_METHOD_22 :   // C += scalar ; C is dense
            // C and M can have any sparsity pattern, including bitmap
            break ;

        case GB_SUBASSIGN_METHOD_09 :   // C(I,J)<M,replace> = scalar
        case GB_SUBASSIGN_METHOD_11 :   // C(I,J)<M,replace> += scalar
        case GB_SUBASSIGN_METHOD_17 :   // C(I,J)<!M,replace> = scalar
        case GB_SUBASSIGN_METHOD_19 :   // C(I,J)<!M,replace> = scalar
            // M can have any sparsity structure, including bitmap
            GB_USE_BITMAP_IF (C_is_bitmap || C_is_full) ;
            break ;

        //----------------------------------------------------------------------
        // matrix assignent methods
        //----------------------------------------------------------------------

        // GB_accum_mask may use any of these methods, with I and J as GB_ALL.

        case GB_SUBASSIGN_METHOD_02 :   // C(I,J) = A
        case GB_SUBASSIGN_METHOD_06s :  // C(I,J)< M> = A ; with S
        case GB_SUBASSIGN_METHOD_14 :   // C(I,J)<!M> = A
        case GB_SUBASSIGN_METHOD_10 :   // C(I,J)< M,replace> = A
        case GB_SUBASSIGN_METHOD_18 :   // C(I,J)<!M,replace> = A
        case GB_SUBASSIGN_METHOD_12 :   // C(I,J)< M,replace> += A
        case GB_SUBASSIGN_METHOD_20 :   // C(I,J)<!M,replace> += A
            // M can have any sparsity structure, including bitmap
            GB_USE_BITMAP_IF (C_is_bitmap || C_is_full) ;
            break ;

        case GB_SUBASSIGN_METHOD_04 :   // C(I,J) += A
        case GB_SUBASSIGN_METHOD_08s :  // C(I,J)< M> += A, with S
        case GB_SUBASSIGN_METHOD_16 :   // C(I,J)<!M> += A 
        case GB_SUBASSIGN_METHOD_24 :   // C = A
            // M can have any sparsity structure, including bitmap
            GB_USE_BITMAP_IF (C_is_bitmap) ;
            break ;

        case GB_SUBASSIGN_METHOD_06d :  // C(:,:)<A> = A ; C is dense
        case GB_SUBASSIGN_METHOD_23 :   // C += A ; C is dense
            // C, M, and A can have any sparsity structure, including bitmap
            break ;

        case GB_SUBASSIGN_METHOD_25 :   // C(:,:)<M,struct> = A ; C empty
            // C, M, and A can have any sparsity structure, including bitmap,
            // but if M is bitmap or full, use bitmap assignment instead.
            GB_USE_BITMAP_IF (M_is_bitmap || GB_IS_FULL (M)) ;
            break ;

        case GB_SUBASSIGN_METHOD_06n :  // C(I,J)<M> = A ; no S
            // If M or A are bitmap, Method 06s is used instead of 06n.
            GB_USE_BITMAP_IF (C_is_bitmap || C_is_full) ;
            ASSERT (!M_is_bitmap) ;
            ASSERT (!A_is_bitmap) ;
            break ;

        case GB_SUBASSIGN_METHOD_08n :  // C(I,J)<M> += A, no S
            // Method 08s is used instead of 08n if M or A are bitmap.
            GB_USE_BITMAP_IF (C_is_bitmap) ;
            ASSERT (!M_is_bitmap) ;
            ASSERT (!A_is_bitmap) ;
            break ;

        // case GB_SUBASSIGN_METHOD_BITMAP:
        default :;
            subassign_method = GB_SUBASSIGN_METHOD_BITMAP ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (subassign_method) ;
}

