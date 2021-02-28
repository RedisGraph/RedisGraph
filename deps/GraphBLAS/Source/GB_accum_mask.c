//------------------------------------------------------------------------------
// GB_accum_mask: accumulate results via the mask and accum operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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

// The Z = accum (C,T) phase is mimiced by the GB_spec_accum.m MATLAB script.

// The next step is C<M> = Z.

// This denotes how the matrix Z is written into C, under the control of the
// mask (or !M if Mask_comp is true), and the C_replace flag (which
// indicates that C should be set to zero first.  This is C<M>=Z in
// GraphBLAS notation.  See GB_mask.c, or GB_spec_mask.m for a MATLAB script
// that describes this step.

// If M is not present, C = Z is returned. Otherwise, M defines what
// values of C are modified. If M(i,j) is present and nonzero, then
// C(i,j)=Z(i,j) is done.  Otherwise, C(i,j) is left unchanged.

// The descriptor affects how C and M are handled.  If the descriptor is
// NULL, defaults are used.

#include "GB_subassign.h"
#include "GB_add.h"
#include "GB_mask.h"
#include "GB_transpose.h"
#include "GB_accum_mask.h"

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

#define GB_FREE_ALL                 \
{                                   \
    GB_MATRIX_FREE (Thandle) ;      \
    GB_MATRIX_FREE (&MT) ;          \
    GB_MATRIX_FREE (&Z) ;           \
}

//------------------------------------------------------------------------------
// GB_accum_mask
//------------------------------------------------------------------------------

GrB_Info GB_accum_mask          // C<M> = accum (C,T)
(
    GrB_Matrix C,               // input/output matrix for results
    const GrB_Matrix M_in,      // optional mask for C, unused if NULL
    const GrB_Matrix MT_in,     // MT=M' if computed already in the caller
    const GrB_BinaryOp accum,   // optional accum for Z=accum(C,results)
    GrB_Matrix *Thandle,        // results of computation, freed when done
    const bool C_replace,       // if true, clear C first
    const bool Mask_comp,       // if true, complement the mask
    const bool Mask_struct,     // if true, use the only structure of M
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M_in

    ASSERT (Thandle != NULL) ;
    GrB_Info info ;
    GrB_Matrix T = *Thandle ;
    GrB_Matrix MT = NULL ;
    GrB_Matrix M = M_in ;
    GrB_Matrix Z = NULL ;

    ASSERT_MATRIX_OK (C, "C input for C<M>=accum(C,T)", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_accum_mask", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (MT_in, "MT_in for GB_accum_mask", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_accum_mask", GB0) ;

    // pending work in C may be abandoned, or it might not need to be
    // finished if GB_subassigner is used, so it is not finished here.
    ASSERT (GB_PENDING_OK (C)) ; ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_PENDING_OK (M)) ; ASSERT (GB_ZOMBIES_OK (M)) ;

    // pending work in T will be finished now
    ASSERT (GB_PENDING_OK (T)) ; ASSERT (GB_ZOMBIES_OK (T)) ;

    // GB_extract can pass in a matrix T that is jumbled, but it does so
    // only if T->is_csc and C->is_csc are different.  In that case, T is
    // transposed, so the sort can be skipped.
    ASSERT_MATRIX_OK_OR_JUMBLED (T, "[T = results of computation]", GB0) ;

    //--------------------------------------------------------------------------
    // remove zombies and pending tuples from T
    //--------------------------------------------------------------------------

    if (GB_PENDING_OR_ZOMBIES (T))
    { 
        // if this fails, *Thandle must be freed
        GB_OK (GB_wait (T, Context)) ;
    }

    //--------------------------------------------------------------------------
    // ensure M and T have the same CSR/CSC format as C
    //--------------------------------------------------------------------------

    #if GB_BURBLE
    bool T_transposed = false ;
    bool M_transposed = false ;
    #endif

    if (C->is_csc != T->is_csc)
    { 
        // transpose: no typecast, no op, in place of T, jumbled, but T
        // cannot have any zombies or pending tuples.
        GB_OK (GB_transpose (Thandle, NULL, C->is_csc, NULL, NULL, Context)) ;
        #if GB_BURBLE
        T_transposed = true ;
        #endif
        T = (*Thandle) ;
    }

    ASSERT_MATRIX_OK (T, "[T = transposed]", GB0) ;

    if (M != NULL && C->is_csc != M->is_csc)
    {
        // M and C have different CSR/CSC formats.  This implies 
        // that C and M are not aliased.

        // MT = M' to conform M to the same CSR/CSC format as C.
        // transpose: typecast, no op, not in place
        if (MT_in == NULL)
        { 
            if (GB_PENDING_OR_ZOMBIES (M))
            {
                // remove zombies and pending tuples from M
                GB_OK (GB_wait (M, Context)) ;
            }
            GB_OK (GB_transpose (&MT, GrB_BOOL, C->is_csc, M, NULL, Context)) ;
            // use the transpose mask
            M = MT ;
            #if GB_BURBLE
            M_transposed = true ;
            #endif
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
    ASSERT (!GB_PENDING (T)) ; ASSERT (!GB_ZOMBIES (T)) ;

    //--------------------------------------------------------------------------
    // apply the accumulator and the mask
    //--------------------------------------------------------------------------

    // decide on the method
    int64_t cnz = GB_NNZ (C) ;          // includes live entries and zombies
    int64_t cnpending = GB_Pending_n (C) ;  // # pending tuples in C
    int64_t tnz = GB_NNZ (T) ;

    // Use subassign for the accum/mask step if either M or accum is present
    // (or both), and if the update is small compared to the size of C.
    // tnz+cnpending is an upper bound on the number of pending tuples in C
    // after the accum/mask step with subassign.  If this is small (< nnz(C)),
    // then use subassign.  It will be fast when T is very sparse and C has
    // many nonzeros.  If the # of pending tuples in C is growing, however,
    // then it would be better to finish the work now, and leave C completed.
    // In this case, GB_transplant (if no accum) or GB_add with accum, and
    // GB_mask are used for the accum/mask step.

    // If there is no mask M, and no accum, then C=T is fast (just
    // GB_transplant for Z=T and GB_transplant_conform in GB_mask for C=Z).
    // So in this case, GB_subassigner takes more work.

    bool use_subassigner =
        ((M != NULL || accum != NULL) && (tnz + cnpending <= cnz)
            && !GB_aliased (C, M) && !GB_aliased (C, T)) ;

    bool use_transplant = (!use_subassigner)
        && (accum == NULL || (cnz + cnpending) == 0) ;

    if (!use_transplant)
    { 
        GBBURBLE ("(C%s%s=Z via %s%s%s) ",
            ((M == NULL) ? "" : ((Mask_comp) ? "<!M>" : "<M>")),
            ((accum == NULL) ? "" : "+"),
            ((use_subassigner) ? "assign" :
                ((use_transplant) ? "transplant" : "add")),
            (M_transposed ? "(M transposed)" : ""),
            (T_transposed ? "(result transposed)" : "")) ;
    }

    if (use_subassigner)
    { 

        //----------------------------------------------------------------------
        // C(:,:)<M> = accum (C(:,:),T) via GB_subassigner
        //----------------------------------------------------------------------

        GB_OK (GB_subassigner (C, C_replace, M, Mask_comp, Mask_struct, accum,
            T, GrB_ALL, 0, GrB_ALL, 0, false, NULL, GB_ignore_code, Context)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C<M> = accum (C,T) via GB_transplant or GB_add, and GB_mask
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // apply the accumulator (Z = accum (C,T) or Z=T if accum not present)
        //----------------------------------------------------------------------

        // see GB_spec_accum.m for a description of this step.  If C is empty,
        // then the accumulator can be ignored.

        if (use_transplant)
        { 

            //------------------------------------------------------------------
            // Z = (ctype) T ;
            //------------------------------------------------------------------

            // [ Z is just the header; the rest can be allocated by the
            // transplant if needed.  Z has the same hypersparsity as T.

            GB_NEW (&Z, C->type, C->vlen, C->vdim, GB_Ap_null, C->is_csc,
                GB_SAME_HYPER_AS (T->is_hyper), T->hyper_ratio, T->plen,
                Context) ;
            GB_OK (info) ;

            // Transplant T into Z, typecasting if needed, and free T.  This
            // may need to do a deep copy if T is shallow.  T is always freed
            // by GB_transplant.

            // Z and T have same vlen, vdim, is_csc, is_hyper
            GB_OK (GB_transplant (Z, C->type, Thandle, Context)) ;
            // Z initialized, and Z->p, Z->h, Z->i, and Z->x are allocated ]

        }
        else
        { 

            //------------------------------------------------------------------
            // Z = (ctype) accum (C,T) ;
            //------------------------------------------------------------------

            // use the mask if present, not complemented, and very sparse
            GrB_Matrix M1 = NULL ;
            if (M != NULL && !Mask_comp && GB_MASK_VERY_SPARSE (M, C, T))
            {
                M1 = M ;
            }

            GB_OK (GB_add (&Z, C->type, C->is_csc, M1, Mask_struct, C, T,
                accum, Context)) ;
            GB_MATRIX_FREE (Thandle) ;
        }

        ASSERT (*Thandle == NULL) ;

        // C and Z have the same type
        ASSERT_MATRIX_OK (Z, "Z in accum_mask", GB0) ;
        ASSERT (Z->type == C->type) ;

        //----------------------------------------------------------------------
        // apply the mask (C<M> = Z)
        //----------------------------------------------------------------------

        // see GB_spec_mask.m for a description of this step.

        // C->hyper_ratio is not modified by GB_mask, which conforms
        // the hypersparsity of C to that parameter.

        // apply the mask, storing the results back into C, and free Z.
        ASSERT_MATRIX_OK (C, "C<M>=Z input", GB0) ;
        GB_OK (GB_mask (C, M, &Z, C_replace, Mask_comp, Mask_struct, Context)) ;
        ASSERT (Z == NULL) ;
        ASSERT (!C->p_shallow && !C->h_shallow) ;
        ASSERT (!C->i_shallow && !C->x_shallow) ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
    ASSERT_MATRIX_OK (C, "C<M>=accum(C,T)", GB0) ;
    return (GrB_SUCCESS) ;
}

