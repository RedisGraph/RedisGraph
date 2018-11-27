//------------------------------------------------------------------------------
// GB_accum_mask: accumulate results via the mask and accum operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,T)

// The primary computation of a GraphBLAS operations is done, and the results
// are in the T matrix.  The T matrix is then used to modify C, via the accum
// operator and the mask matrix M.

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

// The next step is C<M> = Z.

// This denotes how the matrix Z is written into C, under the control of the
// mask (or ~M if Mask_complement is true), and the C_replace flag (which
// indicates that C should be set to zero first.  This is C<M>=Z in
// GraphBLAS notation.  See GB_mask.c, or GB_spec_mask.m for a MATLAB script
// that describes this step.

// If M is not present, C = Z is returned. Otherwise, M defines what
// values of C are modified. If M(i,j) is present and nonzero, then
// C(i,j)=Z(i,j) is done.  Otherwise, C(i,j) is left unchanged.

// The descriptor affects how C and M are handled.  If the descriptor is
// NULL, defaults are used.

// desc [GB_MASK] = GxB_DEFAULT means to use M as-is
// desc [GB_MASK] = GrB_SCMP means to use the logical negation of M

// desc [GB_OUTP] = GxB_DEFAULT means to use C as-is.
// desc [GB_OUTP] = GrB_REPLACE means to clear C before writing Z into C.

#include "GB.h"

GrB_Info GB_accum_mask          // C<M> = accum (C,T)
(
    GrB_Matrix C,               // input/output matrix for results
    const GrB_Matrix M_in,      // optional mask for C, unused if NULL
    const GrB_Matrix MT_in,     // MT=M' if computed already in the caller
    const GrB_BinaryOp accum,   // optional accum for Z=accum(C,results)
    GrB_Matrix *Thandle,        // results of computation, freed when done
    const bool C_replace,       // if true, clear C first
    const bool Mask_complement, // if true, complement the mask
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C and M may be aliased but nothing else
    ASSERT (Thandle != NULL) ;
    ASSERT (GB_ALIAS_OK (C, M_in)) ;       // C can be aliased with M_in

    GrB_Matrix T = *Thandle ;
    GrB_Matrix MT = NULL ;
    GrB_Matrix M = M_in ;

    ASSERT_OK (GB_check (C, "C input for C<M>=accum(C,T)", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for GB_accum_mask", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (MT_in, "MT_in for GB_accum_mask", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_accum_mask", GB0)) ;

    // GB_extract can pass in a matrix T that is jumbled, but it does so
    // only if T->is_csc and C->is_csc are different.  In that case, T is
    // transposed, so the sort can be skipped.
    ASSERT_OK_OR_JUMBLED (GB_check (T, "[T = results of computation]", GB0)) ;

    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (T)) ; ASSERT (!GB_ZOMBIES (T)) ;

    ASSERT (GB_NOT_ALIASED_2 (T, C, M)) ;    // T is not aliased with anything

    //--------------------------------------------------------------------------
    // ensure M and T have the same CSR/CSC format as C
    //--------------------------------------------------------------------------

    GrB_Info info ;

    if (C->is_csc != T->is_csc)
    { 
        // transpose: no typecast, no op, in place of T, jumbled
        info = GB_transpose (Thandle, NULL, C->is_csc, NULL, NULL, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory ; Thandle already freed
            ASSERT (*Thandle == NULL) ;
            return (info) ;
        }
        T = (*Thandle) ;
    }

    ASSERT_OK (GB_check (T, "[T = transposed]", GB0)) ;

    if (M != NULL && C->is_csc != M->is_csc)
    {
        // MT = M' to conform M to the same CSR/CSC format as C.
        // transpose: typecast, no op, not in place
        if (MT_in == NULL)
        { 
            info = GB_transpose (&MT, GrB_BOOL, C->is_csc, M, NULL, Context) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                GB_MATRIX_FREE (Thandle) ;
                return (info) ;
            }
            // use the transpose mask
            M = MT ;
        }
        else
        { 
            // Use the transpose mask passed in by the caller.
            // It is the right vlen-by-vdim dimension, but its
            // CSR/CSC format is ignored.
            M = MT_in ;
        }
    }

    // T and M now conform to the dimensions and CSR/CSC format of C
    ASSERT (C->vlen == T->vlen && C->vdim == T->vdim) ;
    ASSERT (C->is_csc == T->is_csc) ;
    ASSERT (M == NULL || (C->vlen == M->vlen && C->vdim == M->vdim)) ;
    ASSERT (M == NULL || (C->is_csc == M->is_csc)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;

    //--------------------------------------------------------------------------
    // Z = accum (C,T) or Z = T if accum not present
    //--------------------------------------------------------------------------

    // see GB_spec_accum.m for a description of this step

    GrB_Matrix Z = NULL ;

    if (accum == NULL)
    { 

        //----------------------------------------------------------------------
        // Z = (ctype) T ;
        //----------------------------------------------------------------------

        // [ Z is just the header; the rest can be allocated by the transplant
        // if needed.  Z has the same hypersparsity as T.
        Z = NULL ;                  // allocate a new header for Z
        GB_NEW (&Z, C->type, C->vlen, C->vdim, GB_Ap_null, C->is_csc,
            GB_SAME_HYPER_AS (T->is_hyper), T->hyper_ratio, T->plen) ;
        if (info != GrB_SUCCESS)
        { 
            GB_MATRIX_FREE (Thandle) ;
            GB_MATRIX_FREE (&MT) ;
            return (info) ;
        }

        // Transplant T into Z, typecasting if needed, and free T.  This may
        // need to do a deep copy if T is shallow.  T is always freed by
        // GB_transplant.

        // FUTURE: use GB_shallow_cast and free Thandle later

        // Z and T have same vlen, vdim, is_csc, is_hyper
        info = GB_transplant (Z, C->type, Thandle, Context) ;
        // Z is now initialized, and Z->p, Z->h, Z->i, and Z->x are allocated ]

    }
    else
    { 

        //----------------------------------------------------------------------
        // Z = (ctype) accum (C,T) ;
        //----------------------------------------------------------------------

        info = GB_add (&Z, C->type, C->is_csc, C, T, accum, Context) ;
        GB_MATRIX_FREE (Thandle) ;
    }

    ASSERT (*Thandle == NULL) ;

    if (info != GrB_SUCCESS)
    { 
        GB_MATRIX_FREE (&Z) ;
        GB_MATRIX_FREE (&MT) ;
        return (info) ;
    }

    // C and Z have the same type
    ASSERT_OK (GB_check (Z, "Z in accum_mask", GB0)) ;
    ASSERT (Z->type == C->type) ;

    //--------------------------------------------------------------------------
    // C<M> = Z
    //--------------------------------------------------------------------------

    // see GB_spec_mask.m for a description of this step.

    // C->hyper_ratio is not modified by GB_mask, which conforms
    // the hypersparsity of C to that parameter.

    // apply the mask, storing the results back into C, and free Z.
    ASSERT_OK (GB_check (C, "C<M>=Z input", GB0)) ;
    info = GB_mask (C, M, &Z, C_replace, Mask_complement, Context) ;
    ASSERT (Z == NULL) ;
    ASSERT (!C->p_shallow && !C->h_shallow && !C->i_shallow && !C->x_shallow) ;

    // free the transposed mask, if it was created here
    GB_MATRIX_FREE (&MT) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    ASSERT_OK (GB_check (C, "C<M>=accum(C,T) output", GB0)) ;
    return (GrB_SUCCESS)  ;
}

