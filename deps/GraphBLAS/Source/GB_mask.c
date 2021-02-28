//------------------------------------------------------------------------------
// GB_mask: apply a mask: C<M> = Z
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = Z

// Nearly all GraphBLAS operations take a mask, which controls how the result
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

#include "GB_mask.h"

/*

    function R = GB_spec_mask (C, M, Z, C_replace, Mask_comp,identity)
    %GB_SPEC_MASK: a pure MATLAB implementation of GB_mask
    %
    % Computes C<M> = Z, in GraphBLAS notation.
    %
    % Usage:
    % C = GB_spec_mask (C, M, Z, C_replace, Mask_comp, identity)
    %
    % C and Z: matrices of the same size.
    %
    % optional inputs:
    % M: if empty or not present, M = ones (size (C))
    % C_replace: set C to zero first. Default is false.
    % Mask_comp: use ~M instead of M. Default is false.
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
        Mask_comp = false ;
    end
    if (nargin < 4)
        C_replace = false ;
    end

    if (isstruct (C))
        % apply the mask to both the matrix and the pattern
        R.matrix  = GB_spec_mask (C.matrix,  M, Z.matrix,  C_replace, ...
            Mask_comp, identity) ;
        R.pattern = GB_spec_mask (C.pattern, M, Z.pattern, C_replace, ...
            Mask_comp, false) ;
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
        if (~Mask_comp)
            R = Z ;
        else
            % note that Z need never have been computed
            R = C ;
        end
    else
        % form the Boolean mask. For GraphBLAS, this does the
        % right thing and ignores explicit zeros in M.
        M = (M ~= 0) ;
        if (~Mask_comp)
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

#define GB_FREE_ALL                     \
{                                       \
    GB_MATRIX_FREE (Zhandle) ;          \
    GB_MATRIX_FREE (&C_cleared) ;       \
    GB_MATRIX_FREE (&R) ;               \
}

//------------------------------------------------------------------------------

GrB_Info GB_mask                // C<M> = Z
(
    GrB_Matrix C_result,        // both input C and result matrix
    const GrB_Matrix M,         // optional mask matrix, can be NULL
    GrB_Matrix *Zhandle,        // Z = results of computation, perhaps shallow.
                                // Z is freed when done.
    const bool C_replace,       // true if clear(C) to be done first
    const bool Mask_comp,       // true if M is to be complemented
    const bool Mask_struct,     // if true, use the only structure of M
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C_result may be aliased with M
    ASSERT_MATRIX_OK (C_result, "C_result for GB_mask", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_mask", GB0) ;

    // C may be cleared anyway, without the need for finishing it
    ASSERT (GB_PENDING_OK (C_result)) ; ASSERT (GB_ZOMBIES_OK (C_result)) ;

    // M may have zombies and pending tuples
    ASSERT (GB_PENDING_OK (M)) ; ASSERT (GB_ZOMBIES_OK (M)) ;

    ASSERT (Zhandle != NULL) ;
    GrB_Matrix Z = *Zhandle ;

    // Z has the same type as C_result, with no zombies or pending tuples
    ASSERT_MATRIX_OK (Z, "Z for GB_mask", GB0) ;
    ASSERT (!GB_PENDING (Z)) ;
    ASSERT (!GB_ZOMBIES (Z)) ;
    ASSERT (Z->type == C_result->type) ;
    // Z and C_result are never aliased. C_result and M might be.
    ASSERT (Z != C_result) ;
    // Z and C_result must have the same format and dimensions
    ASSERT (C_result->vlen == Z->vlen) ;
    ASSERT (C_result->vdim == Z->vdim) ;

    // M must be compatible with C_result
    ASSERT_OK (GB_Mask_compatible (M, C_result, 0, 0, Context)) ;

    GrB_Info info = GrB_SUCCESS ;

    GrB_Matrix R = NULL ;
    GrB_Matrix C = NULL ;
    GrB_Matrix C_cleared = NULL ;

    //--------------------------------------------------------------------------
    // apply the mask
    //--------------------------------------------------------------------------

    if (M == NULL)
    {

        //----------------------------------------------------------------------
        // there is no mask (implicitly M(i,j)=1 for all i and j)
        //----------------------------------------------------------------------

        // Any pending work on C is abandoned (zombies and/or pending tuples)

        if (!Mask_comp)
        { 

            //------------------------------------------------------------------
            // mask is not complemented: this is the default
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
            // an empty mask is complemented: Z is ignored
            //------------------------------------------------------------------

            // Z is ignored, and can even be NULL.  The method that calls
            // GB_mask can short circuit its computation, ignore accum, and
            // apply the mask immediately, and then return to its caller.
            // This done by the GB_RETURN_IF_QUICK_MASK macro.

            // In the current version, this work is always done by the
            // GB_RETURN_IF_QUICK_MASK macro, and GB_mask is no longer called
            // with an empty complemented mask.  The following is thus dead
            // code.  It is kept here in case this function is called to handle
            // this case in a future version.

            ASSERT (GB_DEAD_CODE) ;    // the following is no longer used

            // free Z if it exists (this is OK if Zhandle is NULL)
            GB_MATRIX_FREE (Zhandle) ;

            if (C_replace)
            {
                // C_result = 0
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
        // the mask is present
        //----------------------------------------------------------------------

        // delete any lingering zombies and assemble any pending tuples
        if (GB_PENDING (M) || GB_ZOMBIES (M)) GB_OK (GB_wait (M, Context)) ;

        // R has the same CSR/CSC format as C_result.  It is hypersparse if
        // both C and Z are hypersparse.

        bool R_is_csc = C_result->is_csc ;
        int64_t vdim = C_result->vdim ;
        int64_t vlen = C_result->vlen ;

        if (C_replace)
        {
            if (GB_aliased (C_result, M))
            { 
                // C_result and M are aliased.  This is OK, unless C_replace is
                // true.  In this case, M must be left unchanged but C_result
                // must be cleared.  To resolve this, a new matrix C_cleared is
                // created, which is what C_result would look like if cleared.
                // C_result is left unchanged since changing it would change M.
                // The C_cleared matrix has the same hypersparsity and CSC/CSR
                // format as the orginal C matrix.
                C_cleared = NULL;   // allocate a new header for C_cleared
                GB_CREATE (&C_cleared, C_result->type, vlen, vdim,
                    GB_Ap_calloc, R_is_csc, GB_AUTO_HYPER,
                    C_result->hyper_ratio, 0, 0, true, Context) ;
                GB_OK (info) ;
                C = C_cleared ;
            }
            else
            { 
                // Clear all entries from C_result
                GB_OK (GB_clear (C_result, Context)) ;
                C = C_result ;
            }
            // C has been cleared, so it has no zombies or pending tuples
        }
        else
        { 
            C = C_result ;

            // delete any lingering zombies and assemble any pending tuples
            if (GB_PENDING (C) || GB_ZOMBIES (C)) GB_OK (GB_wait (C, Context)) ;
        }

        // no more zombies or pending tuples
        ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
        ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;

        // continue with C, do not use C_result until the end since it may be
        // aliased with M.

        //----------------------------------------------------------------------
        // R = masker (M, C, Z):  compute C<M>=Z, placing results in R
        //----------------------------------------------------------------------

        GB_OK (GB_masker (&R, R_is_csc, M, Mask_comp, Mask_struct, C, Z,
            Context)) ;

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

