//------------------------------------------------------------------------------
// GB_accum_mask: accumulate results via the Mask and accum operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C,T)

// The primary computation of a GraphBLAS operations is done, and the results
// are in the T matrix.  The T matrix is then used to modify C, via the accum
// operator and the Mask matrix.

// The results are first accumulated into Z via the accum operator.

// Let Z = accum (C,T) if accum is present, or Z = T otherwise.
// In either case, the type of Z is the same as the C->type defined on input.
// If accum is present, T is typecast into the type of the y input to accum.
// If accum is not present, T is typecast into the same type as C.

// If the function z = accum(x,y) is present, then it defines how the existing
// values of C are used to accumulate T into Z.  If both T(i,j) and C(i,j) are
// present in the pattern, then Z(i,j) = accum (C(i,j), T(i,j)).  Otherwise,
// accum is not used: If C(i,j) is present but not T(i,j), then
// Z(i,j)=C(i,j).  If C(i,j) is not present but T(i,j) is present, then
// Z(i,j)=T(i,j).  The pattern of Z = accum(C,T) is the union of C and T.

// The Z = accum (C,T) phase is mimiced by the GB_spec_accum.m MATLAB script:

/* -----------------------------------------------------------------------------

    function Z = GB_spec_accum (accum, C, T, identity)
    %GB_SPEC_ACCUM: a MATLAB mimic of the Z=accum(C,T) operation in GraphBLAS
    %
    % Z = GB_spec_accum (accum, C, T, identity)
    %
    % Apply accum binary operator to the input C and the intermediate result T.
    %

    % get the operator; default is class(C) if class is not present
    [opname opclass] = GB_spec_operator (accum, C.class) ;

    if (nargin < 4)
        identity = 0 ;
    end

    % initialize the matrix Z, same size and class as C
    [nrows ncols] = size (C.matrix) ;
    Z.matrix  = zeros (nrows, ncols, C.class) ;
    Z.matrix (:,:) = identity ;
    Z.pattern = false (nrows, ncols) ;
    Z.class = C.class ;

    if (isempty (opname))

        % Z = T, casting into the class of C
        Z.matrix  = GB_mex_cast (T.matrix, C.class) ;
        Z.pattern = T.pattern ;

    else

        % Z = accum (C,T)

        % apply the operator to entries in the intersection of C and T
        p = T.pattern & C.pattern ;
        % first cast the entries into the class of the operator
        % note that in the spec, all three domains z=op(x,y) can be different
        % here they are assumed to all be the same
        c = GB_mex_cast (C.matrix (p), opclass) ;
        t = GB_mex_cast (T.matrix (p), opclass) ;
        z = GB_spec_op (accum, c, t) ;
        % cast the result z frop opclass into the class of C
        Z.matrix (p) = GB_mex_cast (z, C.class) ;

        % copy entries in C but not in T, into the result Z, no typecasting
        p = C.pattern & ~T.pattern ;
        Z.matrix (p) = C.matrix (p) ;

        % cast entries in T but not in C, into the result Z
        p = T.pattern & ~C.pattern ;
        Z.matrix (p) = GB_mex_cast (T.matrix (p), C.class) ;

        % the pattern of Z is the union of both T and C
        Z.pattern = C.pattern | T.pattern ;

    end

----------------------------------------------------------------------------- */

// The next step is C<Mask> = Z.

// This denotes how the matrix Z is written into C, under the control of the
// Mask (or ~Mask if Mask_complement is true), and the C_replace flag (which
// indicates that C should be set to zero first.  This is C<Mask>=Z in
// GraphBLAS notation.  See GB_mask.c, or GB_spec_mask.m for a MATLAB script
// that describes this step.

// If Mask is not present, C = Z is returned. Otherwise, Mask defines what
// values of C are modified. If Mask(i,j) is present and nonzero, then
// C(i,j)=Z(i,j) is done.  Otherwise, C(i,j) is left unchanged.

// The descriptor affects how C and Mask are handled.  If the descriptor is
// NULL, defaults are used.

// desc [GB_MASK] = GxB_DEFAULT means to use Mask as-is
// desc [GB_MASK] = GrB_SCMP means to use the logical negation of Mask

// desc [GB_OUTP] = GxB_DEFAULT means to use C as-is.
// desc [GB_OUTP] = GrB_REPLACE means to clear C before writing Z into C.

#include "GB.h"

GrB_Info GB_accum_mask          // C<Mask> = accum (C,T)
(
    GrB_Matrix C,               // input/output matrix for results
    const GrB_Matrix Mask,      // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,   // optional accum for Z=accum(C,results)
    GrB_Matrix *Thandle,        // results of computation, freed when done
    const bool C_replace,       // if true, clear C first
    const bool Mask_complement  // if true, complement the Mask
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C input for C<Mask>=accum(C,T)", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum", 0)) ;
    ASSERT (Thandle != NULL) ;
    GrB_Matrix T = *Thandle ;
    ASSERT_OK (GB_check (T, "[T = results of computation]", 0)) ;

    ASSERT (!PENDING (C)) ;     ASSERT (!ZOMBIES (C)) ;
    ASSERT (!PENDING (Mask)) ;  ASSERT (!ZOMBIES (Mask)) ;
    ASSERT (!PENDING (T)) ;     ASSERT (!ZOMBIES (T)) ;

    //--------------------------------------------------------------------------
    // Z = accum (C,T) or Z = T if accum not present
    //--------------------------------------------------------------------------

    // see GB_spec_accum.m for a description of this step

    GrB_Matrix Z ;
    GrB_Info info ;

    if (accum == NULL)
    {

        //----------------------------------------------------------------------
        // Z = (ctype) T ;
        //----------------------------------------------------------------------

        // [ Z is just the header; the rest can be allocated by the
        // transplant if needed.  Z->p must be allocated if T->p is shallow
        GB_NEW (&Z, C->type, C->nrows, C->ncols, false, T->p_shallow) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (Thandle) ;
            return (info) ;
        }

        // Transplant T into Z, typecasting if needed, and free T.  This may
        // need to do a deep copy if T is shallow.  T is normally not shallow,
        // but there are a few cases in which it can be a shallow copy of the
        // user's input matrix.  T is freed by GB_Matrix_transplant.
        // T may have zombies, which are transplanted into Z.
        info = GB_Matrix_transplant (Z, C->type, Thandle) ;
        // Z is now initialized, and Z->p, Z->i, and Z->x are allocated ]

    }
    else
    {

        //----------------------------------------------------------------------
        // Z = (ctype) accum (C,T) ;
        //----------------------------------------------------------------------

        // [ allocate Z->p but do not initialize it
        GB_NEW (&Z, C->type, C->nrows, C->ncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (Thandle) ;
            return (info) ;
        }

        // Z has the same type as C but the type of C and T can differ
        ASSERT (Z->type == C->type) ;
        info = GB_Matrix_add (Z, C, T, accum) ;
        // Z is now initialized ]
        GB_MATRIX_FREE (Thandle) ;
    }

    if (info != GrB_SUCCESS)
    {
        GB_MATRIX_FREE (&Z) ;
        return (info) ;
    }

    // C and Z have the same type
    ASSERT_OK (GB_check (Z, "Z in accum_mask", 0)) ;
    ASSERT (Z->type == C->type) ;

    //--------------------------------------------------------------------------
    // C<Mask> = Z
    //--------------------------------------------------------------------------

    // see GB_spec_mask.m for a description of this step
    // apply the mask, storing the results back into C, and free Z
    ASSERT_OK (GB_check (C, "C<Mask>=Z input", 0)) ;
    info = GB_mask (C, Mask, &Z, C_replace, Mask_complement) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    ASSERT_OK (GB_check (C, "C<Mask>=accum(C,T) output", 0)) ;
    ASSERT (Z == NULL) ;
    return (REPORT_SUCCESS)  ;
}

