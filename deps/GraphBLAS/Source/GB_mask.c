//------------------------------------------------------------------------------
// GB_mask: apply a mask: C<Mask> = Z
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = Z
//
// Nearly all GraphBLAS operations take a Mask, which controls how the result
// of the computations, Z, are copied into the result matrix C.  The following
// working MATLAB script, GB_spec_mask, defines how this is done.

/*

    function R = GB_spec_mask (C, Mask, Z, C_replace, Mask_complement,identity)
    %GB_SPEC_MASK: a pure MATLAB implementation of GB_mask
    %
    % Computes C<Mask> = Z, in GraphBLAS notation.
    %
    % Usage:
    % C = GB_spec_mask (C, Mask, Z, C_replace, Mask_complement, identity)
    %
    % C and Z: matrices of the same size.
    %
    % optional inputs:
    % Mask: if empty or not present, Mask = ones (size (C))
    % C_replace: set C to zero first. Default is false.
    % Mask_complement: use ~Mask instead of Mask. Default is false.
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
        R.matrix  = GB_spec_mask (C.matrix,  Mask, Z.matrix,  C_replace, ...
            Mask_complement, identity) ;
        R.pattern = GB_spec_mask (C.pattern, Mask, Z.pattern, C_replace, ...
            Mask_complement, false) ;
        R.class = C.class ;
        return
    end

    if (~isequal (size (C), size (Z)))
        error ('C and Z must have the same size') ;
    end
    if (~isempty (Mask))
        if (~isequal (size (C), size (Mask)))
            error ('C and Mask must have the same size') ;
        end
    end

    % replace C if requested
    if (C_replace)
        C (:,:) = identity ;
    end

    if (isempty (Mask))
        % in GraphBLAS, this means Mask is NULL;
        % implicitly, Mask = ones (size (C))
        if (~Mask_complement)
            R = Z ;
        else
            % note that Z need never have been computed
            R = C ;
        end
    else
        % form the Boolean mask. For GraphBLAS, this does the
        % right thing and ignores explicit zeros in Mask.
        Mask = (Mask ~= 0) ;
        if (~Mask_complement)
            % R will equal C where Mask is false
            R = C ;
            % overwrite R with Z where Mask is true
            R (Mask) = Z (Mask) ;
        else
            % Mask is complemented
            % R will equal Z where Mask is false
            R = Z ;
            % overwrite R with C where Mask is true
            R (Mask) = C (Mask) ;
        end
    end

*/

//------------------------------------------------------------------------------

// If an entry C(i,j) or Z(i,j) is found that must be copied into R(i,j), the
// following macro is used.   It moves the the value (Rx[rnz]=Xx[pX]), and the
// index (Ri[rnz++]=i) where X is C or Z.
#define COPY(X)                                         \
{                                                       \
    memcpy (Rx +(rnz*s), X ## x +((p ## X)*s), s) ;     \
    Ri [rnz++] = i ;                                    \
}

// When an entry C(i,j), Z(i,j), or Mask(i,j) is found and
// processed it must be 'discarded' from its list.  This is
// done by incrementing its corresponding pointer.
#define DISCARD(X) (p ## X)++ ;

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_mask                // C<Mask> = Z
(
    GrB_Matrix C,               // both input C and result matrix Cresult
    const GrB_Matrix Mask,      // optional Mask matrix, can be NULL
    GrB_Matrix *Zhandle,        // Z = results of computation, might be shallow
                                // or can even be NULL if Mask is empty and
                                // complemented.  Z is freed when done.
    const bool C_replace,       // true if clear(C) to be done first
    const bool Mask_complement  // true if Mask is to be complemented
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C has no pending tuples and no zombies
    // Mask is optional but if present it has no pending tuples and no zombies
    ASSERT_OK (GB_check (C, "C for GB_mask", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_mask", 0)) ;
    ASSERT (!PENDING (C)) ;
    ASSERT (!ZOMBIES (C)) ;
    ASSERT (!PENDING (Mask)) ;
    ASSERT (!ZOMBIES (Mask)) ;
    if (Zhandle != NULL)
    {
        // If Z is not NULL, it has the same type as C.
        // It has no pending tuples and no zombies
        ASSERT_OK_OR_NULL (GB_check (*Zhandle, "Z", 0)) ;
        ASSERT (!PENDING (*Zhandle)) ; ASSERT (!ZOMBIES (*Zhandle)) ;
        ASSERT ((*Zhandle)->type == C->type) ;
        // *Zhandle and C are never aliased. C and Mask might be, however.
        ASSERT ((*Zhandle) != C) ;
    }

    // Mask must be compatible with C
    ASSERT_OK (GB_Mask_compatible (Mask, C, 0, 0)) ;

    //--------------------------------------------------------------------------
    // apply the mask
    //--------------------------------------------------------------------------

    if (Mask == NULL)
    {

        //----------------------------------------------------------------------
        // there is no Mask (implicitly Mask(i,j)=1 for all i and j)
        //----------------------------------------------------------------------

        if (!Mask_complement)
        {

            //------------------------------------------------------------------
            // Mask is not complemented: this is the default
            //------------------------------------------------------------------

            // Cresult = Z, but make sure a deep copy is made as needed.  It is
            // possible that Z is a shallow copy of another matrix.
            // Z is freed by GB_Matrix_transplant
            ASSERT (C->p != NULL && !C->p_shallow) ;
            return (GB_Matrix_transplant (C, C->type, Zhandle)) ;

        }
        else
        {

            //------------------------------------------------------------------
            // an empty Mask is complemented: Z is ignored
            //------------------------------------------------------------------

            // Z is ignored, and can even be NULL.  The method that calls
            // GB_mask can short circuit its computation, ignore accum, and
            // apply the mask immediately, and then return to its caller.

            // free Z if it exists (this is OK if Zhandle is NULL)
            GB_MATRIX_FREE (Zhandle) ;

            if (C_replace)
            {
                // Cresult = 0
                GB_Matrix_clear (C) ;
            }
            else
            {
                // Cresult = C ; so nothing happens
                ;
            }
            return (REPORT_SUCCESS) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // the Mask is present
        //----------------------------------------------------------------------

        GrB_Info info ;
        GrB_Matrix C2 = C ;
        GrB_Matrix C_cleared = NULL ;

        if (C_replace)
        {
            if (C == Mask)
            {
                // C and Mask are aliased.  This is OK, unless C_replace is
                // true.  In this case, Mask mst be left unchanged but C must
                // be cleared.  To resolve this, a new matrix C_cleared is
                // created, which is what C would look like if cleared.  C is
                // left unchanged since changing it would change the Mask.
                GB_NEW (&C_cleared, C->type, C->nrows, C->ncols, true, false) ;
                if (info != GrB_SUCCESS)
                {
                    GB_MATRIX_FREE (Zhandle) ;
                    return (info) ;
                }
                C2 = C_cleared ;
            }
            else
            {
                // Clear all entries from C
                GB_Matrix_clear (C) ;
            }
        }

        // these conditions must be enforced in the caller
        ASSERT (Zhandle != NULL) ;
        GrB_Matrix Z = *Zhandle ;
        ASSERT_OK (GB_check (C, "C input to 3-way merge", 0)) ;
        ASSERT_OK (GB_check (Z, "Z input to 3-way merge", 0)) ;
        ASSERT_OK (GB_check (Mask, "Mask for 3-way merge", 0)) ;
        ASSERT (Mask->type->code != GB_UDT_code) ;

        // [ allocate the result R; R->p is malloc'd
        double memory = GBYTES (C->ncols + 1, sizeof (int64_t)) ;
        GrB_Matrix R ;
        GB_NEW (&R, C->type, C->nrows, C->ncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&C_cleared) ;
            GB_MATRIX_FREE (Zhandle) ;
            return (info) ;
        }

        // worst case size of R is nnz (Z) + nnz (C)
        if (!GB_Matrix_alloc (R, NNZ (Z) + NNZ (C), true, &memory))
        {
            GB_MATRIX_FREE (&R) ;
            GB_MATRIX_FREE (&C_cleared) ;
            GB_MATRIX_FREE (Zhandle) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }

        // get the function pointer for casting Mask(i,j) from its current
        // type into boolean
        GB_cast_function cast_Mask_to_bool =
            GB_cast_factory (GB_BOOL_code, Mask->type->code) ;

        size_t s = C->type->size, msize = Mask->type->size ;
        int64_t dummy = R->nrows ;

        int64_t rnz = 0 ;
        int64_t ncols = R->ncols ;
        int64_t nrows = R->nrows ;

        int64_t *Rp = R->p ;
        int64_t *Ri = R->i ;
        void    *Rx = R->x ;

        const int64_t *Cp = C2->p ;
        const int64_t *Ci = C2->i ;
        const void    *Cx = C2->x ;

        const int64_t *Zp = Z->p ;
        const int64_t *Zi = Z->i ;
        const void    *Zx = Z->x ;

        const int64_t *Maskp = Mask->p ;
        const int64_t *Maski = Mask->i ;
        const void    *Maskx = Mask->x ;

        for (int64_t j = 0 ; j < ncols ; j++)
        {

            //------------------------------------------------------------------
            // construct R(:,j) = jth column of result
            //------------------------------------------------------------------

            Rp [j] = rnz ;          // mark the start of R(:,j)

            int64_t pC = Cp [j] ;
            int64_t pZ = Zp [j] ;
            int64_t pM = Maskp [j] ;

            int64_t pC_end = Cp [j+1] ;
            int64_t pZ_end = Zp [j+1] ;
            int64_t pM_end = Maskp [j+1] ;

            int64_t jC_nnz = pC_end - pC ;  // number of entries in C (:,j)
            int64_t jZ_nnz = pZ_end - pZ ;  // number of entries in C (:,j)
            int64_t jM_nnz = pM_end - pM ;  // number of entries in Mask (:,j)
            
            if (jM_nnz == nrows)
            {

                //--------------------------------------------------------------
                // Mask (:,j) is completely dense; no need for binary search
                //--------------------------------------------------------------

                // The jth column of the Mask is dense (# of entries = nrows);
                // do a 2-way merge of C and Z, and use a lookup to get Mij.

                // There are two sorted lists to merge:
                // C(:,j)    in Ci    and Cx    [pC .. pC_end-1]
                // Z(:,j)    in Zi    and Zx    [pZ .. pZ_end-1]
                // Mask(:,j) is completely dense.
                // The head of each list is at index pC and pZ, and
                // an entry is 'discarded' by incremented its respective index.
                // Once a list is consumed, a query for its next row index will
                // result in a dummy value larger than all valid row indices.

                //--------------------------------------------------------------
                // while either list C(:,j) or Z(:,j) have entries
                //--------------------------------------------------------------

                while (pC < pC_end || pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // Get the indices at the top of each list.
                    //----------------------------------------------------------

                    // If a list has been consumed, assign a dummy index
                    // that is larger than all valid indices.
                    int64_t iC = (pC < pC_end) ? Ci    [pC] : dummy ;
                    int64_t iZ = (pZ < pZ_end) ? Zi    [pZ] : dummy ;

                    //----------------------------------------------------------
                    // find the smallest index of [iC iZ]
                    //----------------------------------------------------------

                    // i = min ([iC iZ])
                    int64_t i = IMIN (iC, iZ) ;
                    ASSERT (i < dummy) ;

                    //----------------------------------------------------------
                    // get Mask (i,j) using a simple lookup
                    //----------------------------------------------------------

                    // Mij = (bool) Mask [pM + i]
                    bool Mij ;
                    ASSERT (Maski [pM+i] == i) ;
                    cast_Mask_to_bool (&Mij, Maskx +((pM+i)*msize), 0) ;

                    //----------------------------------------------------------
                    // create the entry R(i,j)
                    //----------------------------------------------------------

                    // Given one or both C(i,j), Z(i,j):
                    // R = Z .* Mask + C .* (~Mask) ;   Mask not complemented
                    // R = Z .* (~Mask) + C .* Mask ;   Mask complemented

                    if (i == iC)
                    {
                        if (i == iZ)
                        {
                            // C(i,j) and Z(i,j) both present
                            if (Mask_complement)
                            {
                                if (Mij)
                                {
                                    COPY (C) ;
                                }
                                else
                                {
                                    COPY (Z) ;
                                }
                            }
                            else
                            {
                                if (!Mij)
                                {
                                    COPY (C)
                                }
                                else
                                {
                                    COPY (Z) ;
                                }
                            }
                            DISCARD (C) ;
                            DISCARD (Z) ;
                        }
                        else
                        {
                            // C(i,j) present, Z(i,j) not present
                            if (Mask_complement)
                            {
                                if (Mij)
                                {
                                    COPY (C) ;
                                }
                            }
                            else
                            {
                                if (!Mij)
                                {
                                    COPY (C) ;
                                }
                            }
                            DISCARD (C) ;
                        }
                    }
                    else
                    {
                        ASSERT (i == iZ) ;
                        {
                            // C(i,j) not present, Z(i,j) present
                            if (Mask_complement)
                            {
                                if (!Mij)
                                {
                                    COPY (Z) ;
                                }
                            }
                            else
                            {
                                if (Mij)
                                {
                                    COPY (Z) ;
                                }
                            }
                            DISCARD (Z) ;
                        }
                    }
                }

            }
            else if (jM_nnz > 8 * (jC_nnz + jZ_nnz))
            {

                //--------------------------------------------------------------
                // 2-way merge of C(:,j) and Z(:,j); binary search of M(:,j)
                //--------------------------------------------------------------

                // The number of entries in Mask (:,j) is large compared with
                // nnz(C(:,j)) + nnz(Z(:,j)).  Use a 2-way merge of C and Z,
                // and use a binary search to find the value of Mij.

                // There are two sorted lists to merge:
                // C(:,j)    in Ci    and Cx    [pC .. pC_end-1]
                // Z(:,j)    in Zi    and Zx    [pZ .. pZ_end-1]
                // Mask(:,j) is not merged; a binary search is used instead.
                // The head of each list is at index pC and pZ, and
                // an entry is 'discarded' by incremented its respective index.
                // Once a list is consumed, a query for its next row index will
                // result in a dummy value larger than all valid row indices.

                //--------------------------------------------------------------
                // while either list C(:,j) or Z(:,j) have entries
                //--------------------------------------------------------------

                while (pC < pC_end || pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // Get the indices at the top of each list.
                    //----------------------------------------------------------

                    // If a list has been consumed, assign a dummy index
                    // that is larger than all valid indices.
                    int64_t iC = (pC < pC_end) ? Ci    [pC] : dummy ;
                    int64_t iZ = (pZ < pZ_end) ? Zi    [pZ] : dummy ;

                    //----------------------------------------------------------
                    // find the smallest index of [iC iZ]
                    //----------------------------------------------------------

                    // i = min ([iC iZ])
                    int64_t i = IMIN (iC, iZ) ;
                    ASSERT (i < dummy) ;

                    //----------------------------------------------------------
                    // get Mask (i,j)
                    //----------------------------------------------------------

                    bool Mij = false ;  // Mask (i,j) false if not present
                    int64_t pleft  = pM ;
                    int64_t pright = pM_end - 1 ;
                    bool found ;
                    GB_BINARY_SEARCH (i, Maski, pleft, pright, found) ;
                    if (found)
                    {
                        // found it
                        cast_Mask_to_bool (&Mij, Maskx +(pleft*msize), 0) ;
                    }

                    //----------------------------------------------------------
                    // create the entry R(i,j)
                    //----------------------------------------------------------

                    // Given one or both C(i,j), Z(i,j):
                    // R = Z .* Mask + C .* (~Mask) ;   Mask not complemented
                    // R = Z .* (~Mask) + C .* Mask ;   Mask complemented

                    if (i == iC)
                    {
                        if (i == iZ)
                        {
                            // C(i,j) and Z(i,j) both present
                            if (Mask_complement)
                            {
                                if (Mij)
                                {
                                    COPY (C) ;
                                }
                                else
                                {
                                    COPY (Z) ;
                                }
                            }
                            else
                            {
                                if (!Mij)
                                {
                                    COPY (C)
                                }
                                else
                                {
                                    COPY (Z) ;
                                }
                            }
                            DISCARD (C) ;
                            DISCARD (Z) ;
                        }
                        else
                        {
                            // C(i,j) present, Z(i,j) not present
                            if (Mask_complement)
                            {
                                if (Mij)
                                {
                                    COPY (C) ;
                                }
                            }
                            else
                            {
                                if (!Mij)
                                {
                                    COPY (C) ;
                                }
                            }
                            DISCARD (C) ;
                        }
                    }
                    else
                    {
                        ASSERT (i == iZ) ;
                        {
                            // C(i,j) not present, Z(i,j) present
                            if (Mask_complement)
                            {
                                if (!Mij)
                                {
                                    COPY (Z) ;
                                }
                            }
                            else
                            {
                                if (Mij)
                                {
                                    COPY (Z) ;
                                }
                            }
                            DISCARD (Z) ;
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // do a 3-way merge of C(:,j), Z(:,j), and Mask(:,j)
                //--------------------------------------------------------------

                // The number of entries in Mask (:,j) is small; use a 3-way
                // merge.

                // There are three sorted lists to merge:
                // C(:,j)    in Ci    and Cx    [pC .. pC_end-1]
                // Z(:,j)    in Zi    and Zx    [pZ .. pZ_end-1]
                // Mask(:,j) in Maski and Maskx [pM .. pM_end-1]
                // The head of each list is at index pC, pZ, and pM, and
                // an entry is 'discarded' by incremented its respective index.
                // Once a list is consumed, a query for its next row index will
                // result in a dummy value larger than all valid row indices.

                //--------------------------------------------------------------
                // while either list C(:,j) or Z(:,j) have entries
                //--------------------------------------------------------------

                while (pC < pC_end || pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // Get the indices at the top of each list.
                    //----------------------------------------------------------

                    // If a list has been consumed, assign a dummy index
                    // that is larger than all valid indices.
                    int64_t iC = (pC < pC_end) ? Ci    [pC] : dummy ;
                    int64_t iZ = (pZ < pZ_end) ? Zi    [pZ] : dummy ;
                    int64_t iM = (pM < pM_end) ? Maski [pM] : dummy ;

                    //----------------------------------------------------------
                    // find the smallest index of [iC iZ iM]
                    //----------------------------------------------------------

                    // i = min ([iC iZ iM])
                    int64_t i = IMIN (iC, IMIN (iZ, iM)) ;
                    ASSERT (i < dummy) ;

                    //----------------------------------------------------------
                    // get Mask (i,j)
                    //----------------------------------------------------------

                    // If an explicit value of Mask(i,j) must be tested, it
                    // must first be typecasted to bool.  If (i == iM), then
                    // Mask(i,j) is present and is typecasted into Mij and then
                    // discarded.  Otherwise, if Mask(i,j) is not present, Mij
                    // is set to false.

                    bool Mij ;
                    if (i == iM)
                    {
                        // Mij = (bool) Mask [pM]
                        cast_Mask_to_bool (&Mij, Maskx +(pM*msize), 0) ;
                        DISCARD (M) ;
                    }
                    else
                    {
                        ASSERT (i < iM) ;
                        Mij = false ;
                    }

                    //----------------------------------------------------------
                    // create the entry R(i,j)
                    //----------------------------------------------------------

                    // Given one, two, or all three of C(i,j), Z(i,j), Mask(i,j)
                    // R = Z .* Mask + C .* (~Mask) ;   Mask not complemented
                    // R = Z .* (~Mask) + C .* Mask ;   Mask complemented

                    if (i == iC)
                    {
                        if (i == iZ)
                        {
                            // C(i,j) and Z(i,j) both present
                            if (Mask_complement)
                            {
                                if (Mij)
                                {
                                    COPY (C) ;
                                }
                                else
                                {
                                    COPY (Z) ;
                                }
                            }
                            else
                            {
                                if (!Mij)
                                {
                                    COPY (C)
                                }
                                else
                                {
                                    COPY (Z) ;
                                }
                            }
                            DISCARD (C) ;
                            DISCARD (Z) ;
                        }
                        else
                        {
                            // C(i,j) present, Z(i,j) not present
                            if (Mask_complement)
                            {
                                if (Mij)
                                {
                                    COPY (C) ;
                                }
                            }
                            else
                            {
                                if (!Mij)
                                {
                                    COPY (C) ;
                                }
                            }
                            DISCARD (C) ;
                        }
                    }
                    else
                    {
                        if (i == iZ)
                        {
                            // C(i,j) not present, Z(i,j) present
                            if (Mask_complement)
                            {
                                if (!Mij)
                                {
                                    COPY (Z) ;
                                }
                            }
                            else
                            {
                                if (Mij)
                                {
                                    COPY (Z) ;
                                }
                            }
                            DISCARD (Z) ;
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
        }

        // terminate the last column of R
        Rp [ncols] = rnz ;
        R->magic = MAGIC ;      // R is now initialized ]
        ASSERT_OK (GB_check (R, "Result R", 0)) ;

        //----------------------------------------------------------------------
        // Cresult = R
        //----------------------------------------------------------------------

        // free Z
        GB_MATRIX_FREE (Zhandle) ;

        // Since R has been computed, and already in the type of C, this
        // transplant takes no time.  It just does pointer transplants.
        // The header of R is freed by GB_Matrix_transplant.

        ASSERT (R->type == C2->type) ;
        ASSERT (R->p != NULL && !R->p_shallow) ;
        ASSERT (C2->p != NULL && !C2->p_shallow) ;

        // free the cleared version of C, if created 
        GB_MATRIX_FREE (&C_cleared) ;

        return (GB_Matrix_transplant (C, R->type, &R)) ;
    }
}

#undef COPY
#undef DISCARD

