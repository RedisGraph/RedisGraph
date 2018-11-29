//------------------------------------------------------------------------------
// GB_mask: apply a mask: C<M> = Z
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = Z

// Nearly all GraphBLAS operations take a Mask, which controls how the result
// of the computations, Z, are copied into the result matrix C.  The following
// working MATLAB script, GB_spec_mask, defines how this is done.

// This function can only handle the case when C, M, and Z all have the same
// format (all CSC and CSR transposed, or all CSR or CSC transposed).  The
// caller (GB_accum_mask) must transpose as needed, before calling this
// function.  This function can handle any combination of hypersparsity of C,
// M, and/or Z, as needed.  In the comments, C(i,j) is shorthand for the index
// i in the jth vector, and likewise for M, Z, and R.  If the matrices are all
// CSC, then this is row i and column j.  If the matrices are all CSR, then it
// is row j and column i.

/*

    function R = GB_spec_mask (C, M, Z, C_replace, Mask_complement,identity)
    %GB_SPEC_MASK: a pure MATLAB implementation of GB_mask
    %
    % Computes C<M> = Z, in GraphBLAS notation.
    %
    % Usage:
    % C = GB_spec_mask (C, M, Z, C_replace, Mask_complement, identity)
    %
    % C and Z: matrices of the same size.
    %
    % optional inputs:
    % M: if empty or not present, M = ones (size (C))
    % C_replace: set C to zero first. Default is false.
    % Mask_complement: use ~M instead of M. Default is false.
    % identity: the additive identity of the semiring.  Default is zero.
    %   This is only needed because the GB_spec_* routines operate on dense
    %   matrices, and thus they need to know the value of the implicit 'zero'.
    %
    % This method operates on both plain matrices and on structs with
    % matrix, pattern, and class components.

    if (nargin < 6)
        identity = 0 ;
    end
    if (nargin < 5)
        Mask_complement = false ;
    end
    if (nargin < 4)
        C_replace = false ;
    end

    if (isstruct (C))
        % apply the mask to both the matrix and the pattern
        R.matrix  = GB_spec_mask (C.matrix,  M, Z.matrix,  C_replace, ...
            Mask_complement, identity) ;
        R.pattern = GB_spec_mask (C.pattern, M, Z.pattern, C_replace, ...
            Mask_complement, false) ;
        R.class = C.class ;
        return
    end

    if (~isequal (size (C), size (Z)))
        error ('C and Z must have the same size') ;
    end
    if (~isempty (M))
        if (~isequal (size (C), size (M)))
            error ('C and M must have the same size') ;
        end
    end

    % replace C if requested
    if (C_replace)
        C (:,:) = identity ;
    end

    if (isempty (M))
        % in GraphBLAS, this means M is NULL;
        % implicitly, M = ones (size (C))
        if (~Mask_complement)
            R = Z ;
        else
            % note that Z need never have been computed
            R = C ;
        end
    else
        % form the Boolean mask. For GraphBLAS, this does the
        % right thing and ignores explicit zeros in M.
        M = (M ~= 0) ;
        if (~Mask_complement)
            % R will equal C where M is false
            R = C ;
            % overwrite R with Z where M is true
            R (M) = Z (M) ;
        else
            % M is complemented
            % R will equal Z where M is false
            R = Z ;
            % overwrite R with C where M is true
            R (M) = C (M) ;
        end
    end

*/

//------------------------------------------------------------------------------

// If an entry C(i,j) or Z(i,j) is found that must be copied into R(i,j), the
// following macro is used.   It moves the the value (Rx[rnz]=Xx[pX]), and the
// index (Ri[rnz++]=i) where X is C or Z.
#define GB_COPY(X)                                                  \
{                                                                   \
    memcpy (Rx +(rnz*csize), X ## x +((p ## X)*csize), csize) ;     \
    Ri [rnz++] = i ;                                                \
}

// An entry C(i,j), Z(i,j), or M(i,j) has been processed; move to the next one.
#define GB_NEXT(X) (p ## X)++ ;

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_mask                // C<M> = Z
(
    GrB_Matrix C_result,        // both input C and result matrix
    const GrB_Matrix M,         // optional Mask matrix, can be NULL
    GrB_Matrix *Zhandle,        // Z = results of computation, might be shallow
                                // or can even be NULL if M is empty and
                                // complemented.  Z is freed when done.
    const bool C_replace,       // true if clear(C) to be done first
    const bool Mask_complement, // true if M is to be complemented
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK (C_result, M)) ;

    // C_result has no pending tuples and no zombies
    // M is optional but if present it has no pending tuples and no zombies
    ASSERT_OK (GB_check (C_result, "C_result for GB_mask", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for GB_mask", GB0)) ;
    ASSERT (!GB_PENDING (C_result)) ;
    ASSERT (!GB_ZOMBIES (C_result)) ;
    ASSERT (!GB_PENDING (M)) ;
    ASSERT (!GB_ZOMBIES (M)) ;
    if (Zhandle != NULL)
    { 
        // If Z is not NULL, it has the same type as C_result.
        // It has no pending tuples and no zombies
        ASSERT_OK_OR_NULL (GB_check (*Zhandle, "Z for GB_mask", GB0)) ;
        ASSERT (!GB_PENDING (*Zhandle)) ;
        ASSERT (!GB_ZOMBIES (*Zhandle)) ;
        ASSERT ((*Zhandle)->type == C_result->type) ;
        // *Zhandle and C_result are never aliased. C_result and M might be.
        ASSERT ((*Zhandle) != C_result) ;
        // Z and C_result must have the same format and dimensions
        ASSERT (C_result->vlen == (*Zhandle)->vlen) ;
        ASSERT (C_result->vdim == (*Zhandle)->vdim) ;
    }

    // M must be compatible with C_result
    ASSERT_OK (GB_Mask_compatible (M, C_result, 0, 0, Context)) ;

    GrB_Info info = GrB_SUCCESS ;

    //--------------------------------------------------------------------------
    // apply the mask
    //--------------------------------------------------------------------------

    if (M == NULL)
    {

        //----------------------------------------------------------------------
        // there is no Mask (implicitly M(i,j)=1 for all i and j)
        //----------------------------------------------------------------------

        if (!Mask_complement)
        {

            //------------------------------------------------------------------
            // Mask is not complemented: this is the default
            //------------------------------------------------------------------

            // C_result = Z, but make sure a deep copy is made as needed.  It is
            // possible that Z is a shallow copy of another matrix.
            // Z is freed by GB_transplant_conform.
            ASSERT (C_result->p != NULL) ;
            ASSERT (!C_result->p_shallow) ;
            ASSERT (!C_result->h_shallow) ;

            // transplant Z into C_result and conform to desired hypersparsity
            return (GB_transplant_conform (C_result, C_result->type, Zhandle,
                Context)) ;
        }
        else
        {

            //------------------------------------------------------------------
            // an empty Mask is complemented: Z is ignored
            //------------------------------------------------------------------

            // Z is ignored, and can even be NULL.  The method that calls
            // GB_mask can short circuit its computation, ignore accum, and
            // apply the mask immediately, and then return to its caller.
            // This done by the GB_RETURN_IF_QUICK_MASK macro.

            // free Z if it exists (this is OK if Zhandle is NULL)
            GB_MATRIX_FREE (Zhandle) ;

            if (C_replace)
            { 
                // C_result = 0, but keep C->Sauna
                return (GB_clear (C_result, Context)) ;
            }
            else
            { 
                // nothing happens
                return (GrB_SUCCESS) ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // the Mask is present
        //----------------------------------------------------------------------

        GrB_Matrix C ;
        GrB_Matrix C_cleared = NULL ;

        // R has the same CSR/CSC format as C_result.  It is hypersparse if
        // both C and Z are hypersparse.

        bool C_result_is_csc = C_result->is_csc ;
        int64_t vdim = C_result->vdim ;
        int64_t vlen = C_result->vlen ;

        if (C_replace)
        {
            if (GB_ALIASED (C_result, M))
            { 
                // C_result and M are aliased.  This is OK, unless C_replace is
                // true.  In this case, M must be left unchanged but C_result
                // must be cleared.  To resolve this, a new matrix C_cleared is
                // created, which is what C_result would look like if cleared.
                // C_result is left unchanged since changing it would change M.
                // The C_cleared matrix is has the same hypersparsity as
                // a matrix created by GrB_Matrix_new.  It has the same CSC/CSR
                // format as the orginal C matrix.
                C_cleared = NULL;   // allocate a new header for C_cleared
                GB_CREATE (&C_cleared, C_result->type, vlen, vdim,
                    GB_Ap_calloc, C_result_is_csc, GB_AUTO_HYPER,
                    C_result->hyper_ratio, 0, 0, true) ;
                C = C_cleared ;
            }
            else
            { 
                // Clear all entries from C_result, but keep C->Sauna
                info = GB_clear (C_result, Context) ;
                C = C_result ;
            }
        }
        else
        { 
            C = C_result ;
        }

        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (Zhandle) ;
            return (info) ;
        }

        // continue with C, do not use C_result until the end since it may be
        // aliased with M.

        // these conditions must be enforced in the caller
        ASSERT (Zhandle != NULL) ;
        GrB_Matrix Z = *Zhandle ;
        ASSERT_OK (GB_check (C, "C input to 3-way merge", GB0)) ;
        ASSERT_OK (GB_check (Z, "Z input to 3-way merge", GB0)) ;
        ASSERT_OK (GB_check (M, "M input to 3-way merge", GB0)) ;
        ASSERT (M->type->code <= GB_FP64_code) ;
        ASSERT (M->vlen == C->vlen && M->vdim == C->vdim) ;

        // [ R->p is malloc'd
        // R is hypersparse if C and Z are hypersparse
        // R->plen is the upper bound: sum of # non-empty columns of C and Z,
        // or C->vdim, whichever is smaller.
        GrB_Matrix R = NULL ;       // allocate a new header for R
        GB_CREATE (&R, C->type, vlen, vdim, GB_Ap_malloc, C_result_is_csc,
            GB_SAME_HYPER_AS (C->is_hyper && Z->is_hyper), C->hyper_ratio,
            GB_IMIN (vdim, C->nvec_nonempty + Z->nvec_nonempty),
            GB_NNZ (C) + GB_NNZ (Z), true) ;

        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (&C_cleared) ;
            GB_MATRIX_FREE (Zhandle) ;
            return (info) ;
        }

        // get the function pointer for casting M(i,j) from its current
        // type into boolean
        GB_cast_function cast_Mask_to_bool =
            GB_cast_factory (GB_BOOL_code, M->type->code) ;

        size_t csize = C->type->size ;
        size_t msize = M->type->size ;

        int64_t *Ri = R->i ;
        GB_void *Rx = R->x ;

        int64_t jlast, rnz, rnz_last ;
        GB_jstartup (R, &jlast, &rnz, &rnz_last) ;

        const int64_t *Ci = C->i, *Zi = Z->i, *Mi = M->i ;
        const GB_void *Cx = C->x, *Zx = Z->x, *Mx = M->x ;

        GB_for_each_vector3 (C, Z, M)
        {

            //------------------------------------------------------------------
            // get the next vector j of C, Z, and M, and their lengths
            //------------------------------------------------------------------

            int64_t GBI3_initj (Iter, j, pC, pC_end, pZ, pZ_end, pM, pM_end) ;
            int64_t jC_nnz = pC_end - pC ;
            int64_t jZ_nnz = pZ_end - pZ ;
            int64_t jM_nnz = pM_end - pM ;

            //------------------------------------------------------------------
            // construct R(:,j) = jth vector of result
            //------------------------------------------------------------------

            if (jM_nnz == vlen)
            {

                //--------------------------------------------------------------
                // M(:,j) is completely dense; no need for binary search
                //--------------------------------------------------------------

                // The jth vector of M is dense (# of entries = vlen);
                // do a 2-way merge of C and Z, and use a lookup to get mij.

                // There are two sorted lists to merge:
                //      C(:,j) in Ci and Cx [pC .. pC_end-1]
                //      Z(:,j) in Zi and Zx [pZ .. pZ_end-1]
                //      M(:,j) is completely dense and need not be searched.
                // The head of each list is at index pC and pZ, and
                // an entry is 'discarded' by incrementing its respective pX.
                // Once a list is consumed, a query for its next index will
                // give vlen, a value larger than all valid indices.

                //--------------------------------------------------------------
                // while either list C(:,j) or Z(:,j) have entries
                //--------------------------------------------------------------

                while (pC < pC_end || pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // Get the indices at the top of each list.
                    //----------------------------------------------------------

                    // If a list has been consumed, assign index vlen, which
                    // is larger than all valid indices.
                    int64_t iC = (pC < pC_end) ? Ci [pC] : vlen ;
                    int64_t iZ = (pZ < pZ_end) ? Zi [pZ] : vlen ;

                    //----------------------------------------------------------
                    // find the smallest index of [iC iZ]
                    //----------------------------------------------------------

                    // i = min ([iC iZ])
                    int64_t i = GB_IMIN (iC, iZ) ;
                    ASSERT (i < vlen) ;

                    //----------------------------------------------------------
                    // get M(i,j) using a simple lookup
                    //----------------------------------------------------------

                    // mij = (bool) M [pM + i]
                    bool mij ;
                    ASSERT (Mi [pM+i] == i) ;
                    cast_Mask_to_bool (&mij, Mx +((pM+i)*msize), 0) ;
                    if (Mask_complement)
                    { 
                        mij = !mij ;
                    }

                    //----------------------------------------------------------
                    // create the entry R(i,j)
                    //----------------------------------------------------------

                    // Given one or both C(i,j), Z(i,j):
                    // R = Z .* M + C .* (~M) ;   Mask not complemented
                    // R = Z .* (~M) + C .* M ;   Mask complemented

                    if (i == iC)
                    {
                        if (i == iZ)
                        {
                            // C(i,j) and Z(i,j) both present
                            if (mij)
                            { 
                                GB_COPY (Z) ;
                            }
                            else
                            { 
                                GB_COPY (C) ;
                            }
                            GB_NEXT (C) ;
                            GB_NEXT (Z) ;
                        }
                        else
                        {
                            // C(i,j) present, Z(i,j) not present
                            if (!mij)
                            { 
                                GB_COPY (C) ;
                            }
                            GB_NEXT (C) ;
                        }
                    }
                    else
                    {
                        ASSERT (i == iZ) ;
                        {
                            // C(i,j) not present, Z(i,j) present
                            if (mij)
                            { 
                                GB_COPY (Z) ;
                            }
                            GB_NEXT (Z) ;
                        }
                    }
                }

            }
            else if (jM_nnz > 8 * (jC_nnz + jZ_nnz))
            {

                //--------------------------------------------------------------
                // 2-way merge of C(:,j) and Z(:,j); binary search of M(:,j)
                //--------------------------------------------------------------

                // The number of entries in M(:,j) is large compared with
                // nnz(C(:,j)) + nnz(Z(:,j)).  Use a 2-way merge of C and Z,
                // and use a binary search to find the value of mij.

                // There are two sorted lists to merge:
                //      C(:,j) in Ci and Cx [pC .. pC_end-1]
                //      Z(:,j) in Zi and Zx [pZ .. pZ_end-1]
                //      M(:,j) is not merged; a binary search is used instead.
                // The head of each list is at index pC and pZ, and
                // an entry is 'discarded' by incrementing its respective pX.
                // Once a list is consumed, a query for its next index will
                // give vlen, a value larger than all valid indices.

                //--------------------------------------------------------------
                // while either list C(:,j) or Z(:,j) have entries
                //--------------------------------------------------------------

                while (pC < pC_end || pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // Get the indices at the top of each list.
                    //----------------------------------------------------------

                    // If a list has been consumed, assign index vlen, which
                    // is larger than all valid indices.
                    int64_t iC = (pC < pC_end) ? Ci [pC] : vlen ;
                    int64_t iZ = (pZ < pZ_end) ? Zi [pZ] : vlen ;

                    //----------------------------------------------------------
                    // find the smallest index of [iC iZ]
                    //----------------------------------------------------------

                    // i = min ([iC iZ])
                    int64_t i = GB_IMIN (iC, iZ) ;
                    ASSERT (i < vlen) ;

                    //----------------------------------------------------------
                    // get M(i,j)
                    //----------------------------------------------------------

                    bool mij = false ;  // M(i,j) false if not present
                    int64_t pleft  = pM ;
                    int64_t pright = pM_end - 1 ;
                    bool found ;
                    GB_BINARY_SEARCH (i, Mi, pleft, pright, found) ;
                    if (found)
                    { 
                        // found it
                        cast_Mask_to_bool (&mij, Mx +(pleft*msize), 0) ;
                    }
                    if (Mask_complement)
                    { 
                        mij = !mij ;
                    }

                    //----------------------------------------------------------
                    // create the entry R(i,j)
                    //----------------------------------------------------------

                    // Given one or both C(i,j), Z(i,j):
                    // R = Z .* M + C .* (~M) ;   Mask not complemented
                    // R = Z .* (~M) + C .* M ;   Mask complemented

                    if (i == iC)
                    {
                        if (i == iZ)
                        {
                            // C(i,j) and Z(i,j) both present
                            if (mij)
                            { 
                                GB_COPY (Z) ;
                            }
                            else
                            { 
                                GB_COPY (C) ;
                            }
                            GB_NEXT (C) ;
                            GB_NEXT (Z) ;
                        }
                        else
                        {
                            // C(i,j) present, Z(i,j) not present
                            if (!mij)
                            { 
                                GB_COPY (C) ;
                            }
                            GB_NEXT (C) ;
                        }
                    }
                    else
                    {
                        ASSERT (i == iZ) ;
                        {
                            // C(i,j) not present, Z(i,j) present
                            if (mij)
                            { 
                                GB_COPY (Z) ;
                            }
                            GB_NEXT (Z) ;
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // do a 3-way merge of C(:,j), Z(:,j), and M(:,j)
                //--------------------------------------------------------------

                // The number of entries in M(:,j) is small; use a 3-way
                // merge.

                // There are three sorted lists to merge:
                //      C(:,j) in Ci and Cx [pC .. pC_end-1]
                //      Z(:,j) in Zi and Zx [pZ .. pZ_end-1]
                //      M(:,j) in Mi and Mx [pM .. pM_end-1]
                // The head of each list is at index pC, pZ, and pM, and an
                // an entry is 'discarded' by incrementing its respective pX.
                // Once a list is consumed, a query for its next index will
                // give vlen, a value larger than all valid indices.

                //--------------------------------------------------------------
                // while either list C(:,j) or Z(:,j) have entries
                //--------------------------------------------------------------

                while (pC < pC_end || pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // Get the indices at the top of each list.
                    //----------------------------------------------------------

                    // If a list has been consumed, assign index vlen, which
                    // is larger than all valid indices.
                    int64_t iC = (pC < pC_end) ? Ci [pC] : vlen ;
                    int64_t iZ = (pZ < pZ_end) ? Zi [pZ] : vlen ;
                    int64_t iM = (pM < pM_end) ? Mi [pM] : vlen ;

                    //----------------------------------------------------------
                    // find the smallest index of [iC iZ iM]
                    //----------------------------------------------------------

                    // i = min ([iC iZ iM])
                    int64_t i = GB_IMIN (iC, GB_IMIN (iZ, iM)) ;
                    ASSERT (i < vlen) ;

                    //----------------------------------------------------------
                    // get M(i,j)
                    //----------------------------------------------------------

                    // If an explicit value of M(i,j) must be tested, it
                    // must first be typecasted to bool.  If (i == iM), then
                    // M(i,j) is present and is typecasted into mij and then
                    // discarded.  Otherwise, if M(i,j) is not present, mij
                    // is set to false.

                    bool mij ;
                    if (i == iM)
                    { 
                        // mij = (bool) M [pM]
                        cast_Mask_to_bool (&mij, Mx +(pM*msize), 0) ;
                        GB_NEXT (M) ;
                    }
                    else
                    { 
                        ASSERT (i < iM) ;
                        mij = false ;
                    }
                    if (Mask_complement)
                    { 
                        mij = !mij ;
                    }

                    //----------------------------------------------------------
                    // create the entry R(i,j)
                    //----------------------------------------------------------

                    // Given one, two, or all three of C(i,j), Z(i,j), M(i,j)
                    // R = Z .* M + C .* (~M) ;   Mask not complemented
                    // R = Z .* (~M) + C .* M ;   Mask complemented

                    if (i == iC)
                    {
                        if (i == iZ)
                        {
                            // C(i,j) and Z(i,j) both present
                            if (mij)
                            { 
                                GB_COPY (Z) ;
                            }
                            else
                            { 
                                GB_COPY (C) ;
                            }
                            GB_NEXT (C) ;
                            GB_NEXT (Z) ;
                        }
                        else
                        {
                            // C(i,j) present, Z(i,j) not present
                            if (!mij)
                            { 
                                GB_COPY (C) ;
                            }
                            GB_NEXT (C) ;
                        }
                    }
                    else
                    {
                        if (i == iZ)
                        {
                            // C(i,j) not present, Z(i,j) present
                            if (mij)
                            { 
                                GB_COPY (Z) ;
                            }
                            GB_NEXT (Z) ;
                        }
                        else
                        { 
                            // neither C(i,j) nor Z(i,j) present, just M(i,j),
                            // which is not needed and thus ignored.
                            ASSERT (i == iM) ; // i must be equal to one of them
                        }
                    }
                }
            }

            //------------------------------------------------------------------
            // finalize R (:,j) and advance to the next vector
            //------------------------------------------------------------------

            // cannot fail since C->plen is at the upper bound
            info = GB_jappend (R, j, &jlast, rnz, &rnz_last, Context) ;
            ASSERT (info == GrB_SUCCESS) ;

            #if 0
            // if GB_jappend could fail, do this:
            if (info != GrB_SUCCESS)
            {
                // out of memory for C->p and C->h (not possible here)
                GB_MATRIX_FREE (&R) ;
                GB_MATRIX_FREE (&C_cleared) ;
                GB_MATRIX_FREE (Zhandle) ;
                return (info) ;
            }
            #endif
        }

        //----------------------------------------------------------------------
        // finalize R
        //----------------------------------------------------------------------

        GB_jwrapup (R, jlast, rnz) ;    // R->p is now defined ]

        //----------------------------------------------------------------------
        // free temporary matrices Z and C_cleared
        //----------------------------------------------------------------------

        GB_MATRIX_FREE (Zhandle) ;
        GB_MATRIX_FREE (&C_cleared) ;

        //----------------------------------------------------------------------
        // transplant the result, conform, and free R
        //----------------------------------------------------------------------

        // finished using the mask M, so it is now safe to modify C_result,
        // even if C_result and M are aliased

        return (GB_transplant_conform (C_result, R->type, &R, Context)) ;
    }
}

