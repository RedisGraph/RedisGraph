//------------------------------------------------------------------------------
// GB_subassign_kernel: C(I,J)<M> = accum (C(I,J), A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Submatrix assignment: C(I,J)<M> = A, or accum (C (I,J), A), no transpose

// All assignment operations rely on this function, including the GrB_*_assign
// operations in the spec, and the GxB_*_subassign operations that are a
// SuiteSparse:GraphBLAS extension to the spec:

// GrB_Matrix_assign
// GrB_Matrix_assign_TYPE
// GrB_Vector_assign
// GrB_Vector_assign_TYPE
// GrB_Row_assign
// GrB_Col_assign

// GxB_Matrix_subassign
// GxB_Matrix_subassign_TYPE
// GxB_Vector_subassign
// GxB_Vector_subassign_TYPE
// GxB_Row_subassign
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

#include "GB.h"

GrB_Info GB_subassign_kernel        // C(I,J)<M> = A or accum (C (I,J), A)
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A,             // input matrix (NULL for scalar expansion)
    const GrB_Index *I,             // list of indices
    const int64_t   ni,             // number of indices
    const GrB_Index *J,             // list of vector indices
    const int64_t   nj,             // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_NOT_ALIASED_2 (C, M, A)) ;

    //--------------------------------------------------------------------------
    // check empty mask conditions
    //--------------------------------------------------------------------------

    int64_t nzMask = 0 ;
    if (M == NULL)
    {
        // the mask is empty
        if (Mask_comp)
        {
            // an empty mask is complemented
            if (!C_replace)
            { 
                // No work to do.  This the same as the GB_RETURN_IF_QUICK_MASK
                // case in other GraphBLAS functions, except here only the
                // sub-case of C_replace=false is handled.  The C_replace=true
                // sub-case needs to delete all entries in C(I,J), which is
                // handled below, not here.
                return (GrB_SUCCESS) ;
            }
        }
        else
        { 
            // The mask is empty and not complemented.  In this case, C_replace
            // is effectively false.  Disable it, since it can force pending
            // tuples to be assembled.  In the comments below "C_replace
            // effectively false" means that either C_replace is false on
            // input, or the mask is empty and not complemented and thus
            // C_replace is set to false here.
            C_replace = false ;
        }
    }
    else
    { 
        nzMask = GB_NNZ (M) ;
    }

    // C_replace now has its effective value: it is true only if true on input
    // and if the mask is present, or empty and complemented.  C_replace is
    // false if it is false on input, or if the mask is empty and not
    // complemented.

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    // the matrix C may have pending tuples and/or zombies
    ASSERT_OK (GB_check (C, "C for subassign kernel", GB0)) ;
    ASSERT (GB_PENDING_OK (C)) ; ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    // C(I,J) = A, so I is in 0:C->vlen-1 and J is in 0:C->vdim-1
    int64_t cvlen = C->vlen ;
    int64_t cvdim = C->vdim ;
    int64_t nI, nJ, Icolon [3], Jcolon [3] ;
    int Ikind, Jkind ;
    GB_ijlength (I, ni, cvlen, &nI, &Ikind, Icolon) ;
    GB_ijlength (J, nj, cvdim, &nJ, &Jkind, Jcolon) ;

    // If the descriptor says that A must be transposed, it has already been
    // transposed in the caller.  Thus C(I,J), A, and M (if present) all
    // have the same size: length(I)-by-length(J)

    GrB_Index mn ;
    bool mn_ok = GB_Index_multiply (&mn, nI, nJ) ;
    bool is_dense ;
    int64_t anz ;
    GrB_Type atype ;
    GB_Type_code acode ;

    if (scalar_expansion)
    { 
        // The input is a scalar; the matrix A is not present
        ASSERT (A == NULL) ;
        ASSERT (scalar != NULL) ;
        anz = mn ;
        is_dense = true ;
        // a user-defined scalar is assumed to have the same type as
        // C->type which is also user-defined (or else it would not be
        // compatible)
        atype = GB_code_type (scalar_code, C->type) ;
        acode = scalar_code ;
    }
    else
    { 
        // A is an nI-by-nJ matrix, with no pending computations
        ASSERT_OK (GB_check (A, "A for subassign kernel", GB0)) ;
        ASSERT (nI == A->vlen && nJ == A->vdim) ;
        ASSERT (!GB_PENDING (A)) ;   ASSERT (!GB_ZOMBIES (A)) ;
        ASSERT (scalar == NULL) ;
        anz = GB_NNZ (A) ;
        is_dense = (mn_ok && anz == (int64_t) mn) ;
        atype = A->type ;
        acode = atype->code ;
    }

    // the size of the entries of the matrix A, or the scalar
    size_t asize = atype->size ;
    ASSERT (acode == atype->code) ;

    if (M != NULL)
    { 
        // M can have no pending tuples nor zombies
        ASSERT_OK (GB_check (M, "M for subassign kernel", GB0)) ;
        ASSERT (!GB_PENDING (M)) ;  ASSERT (!GB_ZOMBIES (M)) ;
        ASSERT (nI == M->vlen && nJ == M->vdim) ;
    }

    //--------------------------------------------------------------------------
    // check compatibilty of prior pending tuples
    //--------------------------------------------------------------------------

    // The action: ( delete ), described below, can only delete a live
    // entry in the pattern.  It cannot delete a pending tuple; pending tuples
    // cannot become zombies.  Thus, if this call to GxB_subassign has the
    // potential for creating zombies, all prior pending tuples must be
    // assembled now.  They thus become live entries in the pattern of C, so
    // that this GxB_subassign can (potentially) turn them into zombies via
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
    // pending tuples must be assembled now, so that this GxB_subassign can
    // (potentially) insert new pending tuples whose pending operator is accum.

    // These tests are conservative because it is possible that this
    // GxB_subassign will not need to use action: (insert ).

    // In the discussion below, let SECOND_Ctype denote the SECOND operator
    // z=f(x,y) whose ztype, xtype, and ytype matches the type of C.

    bool wait = false ;

    if (C->n_pending == 0)
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

            if (M == NULL && is_dense)
            { 
                // A is a dense matrix, so entries cannot be deleted
                wait = false ;
            }
            else
            { 
                // A has at least one implicit entry, or M is present.
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

            ASSERT (C->type_pending != NULL) ;

            if (atype != C->type_pending)
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
                    (accum == C->operator_pending)
                    // or both operators are SECOND_Ctype, implicit or explicit
                    || (GB_op_is_second (accum, C->type) &&
                        GB_op_is_second (C->operator_pending, C->type))
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
        GB_WAIT (C) ;
    }

    ASSERT_OK_OR_NULL (GB_check (accum, "accum for assign", GB0)) ;

    //--------------------------------------------------------------------------
    // keep track of the current accum operator
    //--------------------------------------------------------------------------

    // If accum is NULL and pending tuples are added, they will be assembled
    // sometime later (not here) using the implied SECOND_Ctype operator.  This
    // GxB_subassign operation corresponds to C(I,J)=A or C(I,J)<M>=A.
    // Subsequent calls to GrB_setElement, and subsequent calls to
    // GxB_subassign with an explict SECOND_Ctype operator, may create
    // additional pending tuples and add them to the list without requiring
    // that they be assembled first.

    // If accum is non-NULL, then all prior pending tuples have the same
    // pending operator as this accum.  If that prior operator was the implicit
    // SECOND_Ctype and those pending tuples still exist, then this accum
    // operator is the explicit SECOND_ctype operator.  The implicit
    // SECOND_Ctype operator is replace with the current accum, which is the
    // explicit SECOND_Ctype operator.

    C->operator_pending = accum ;

    //--------------------------------------------------------------------------
    // S_Extraction: finding C(iC,jC) via lookup through S=C(I,J)
    //--------------------------------------------------------------------------

    // S is the symbolic pattern of the submatrix S = C(I,J).  The "numerical"
    // value (held in S->x) of an entry S(i,j) is not a value, but a pointer
    // back into C where the corresponding entry C(iC,jC) can be found, where
    // iC = I [i] and jC = J [j].

    // The following macro performs the lookup.  Given a pointer pS into a
    // column S(:,j), it finds the entry C(iC,jC), and also determines if the
    // C(iC,jC) entry is a zombie.  The column indices j and jC are implicit.

    #define GB_C_S_LOOKUP                                                   \
        int64_t pC = Sx [pS] ;                                              \
        int64_t iC = Ci [pC] ;                                              \
        bool is_zombie = GB_IS_ZOMBIE (iC) ;                                \
        if (is_zombie) iC = GB_FLIP (iC) ;

    //--------------------------------------------------------------------------
    // C(:,jC) is dense: iC = I [iA], and then look up C(iC,jC)
    //--------------------------------------------------------------------------

    // C(:,jC) is dense, and thus can be accessed with a constant-time lookup
    // with the index iC, where the index iC comes from I [iA] or via a
    // colon notation for I.

    #define GB_CDENSE_I_LOOKUP                                              \
        int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;                     \
        int64_t pC = pC_start + iC ;                                        \
        bool is_zombie = GB_IS_ZOMBIE (Ci [pC]) ;                           \
        ASSERT (GB_UNFLIP (Ci [pC]) == iC) ;

    //--------------------------------------------------------------------------
    // get the C(:,jC) vector where jC = J [j]
    //--------------------------------------------------------------------------

    // C may be standard sparse, or hypersparse

    #define GB_jC_LOOKUP                                                    \
        /* lookup jC in C */                                                \
        /* jC = J [j] ; or J is ":" or jbegin:jend or jbegin:jinc:jend */   \
        jC = GB_ijlist (J, j, Jkind, Jcolon) ;                              \
        int64_t pC_start, pC_end, pleft = 0, pright = cnvec-1 ;             \
        GB_lookup (C_is_hyper, Ch, Cp, &pleft, pright, jC, &pC_start, &pC_end) ;

    //--------------------------------------------------------------------------
    // get C(iC,jC) via binary search of C(:,jC)
    //--------------------------------------------------------------------------

    #define GB_iC_BINARY_SEARCH                                                \
        int64_t pC = pC_start ;                                                \
        int64_t pright = pC_end - 1 ;                                          \
        bool found, is_zombie ;                                                \
        GB_BINARY_ZOMBIE (iC, Ci, pC, pright, found, C->nzombies, is_zombie) ;

    //--------------------------------------------------------------------------
    // for a 2-way or 3-way merge
    //--------------------------------------------------------------------------

    // An entry S(i,j), A(i,j), or M(i,j) has been processed;
    // move to the next one.
    #define GB_NEXT(X) (p ## X)++ ;

    //--------------------------------------------------------------------------
    // basic operations
    //--------------------------------------------------------------------------

    #define GB_COPY_scalar_to_C                                         \
    {                                                                   \
        /* C(iC,jC) = scalar, already typecasted into cwork      */     \
        ASSERT (scalar_expansion) ;                                     \
        memcpy (Cx +(pC*csize), cwork, csize) ;                         \
    }

    #define GB_COPY_aij_to_C                                            \
    {                                                                   \
        /* C(iC,jC) = A(i,j), with typecasting                   */     \
        ASSERT (!scalar_expansion) ;                                    \
        cast_A_to_C (Cx +(pC*csize), Ax +(pA*asize), csize) ;           \
    }

    #define GB_COPY_aij_to_ywork                                        \
    {                                                                   \
        /* ywork = A(i,j), with typecasting                      */     \
        ASSERT (!scalar_expansion) ;                                    \
        cast_A_to_Y (ywork, Ax +(pA*asize), asize) ;                    \
    }

    #define GB_ACCUMULATE                                               \
    {                                                                   \
        /* C(iC,jC) = accum (C(iC,jC), ywork)                    */     \
        cast_C_to_X (xwork, Cx +(pC*csize), csize) ;                    \
        faccum (zwork, xwork, ywork) ;                                  \
        cast_Z_to_C (Cx +(pC*csize), zwork, csize) ;                    \
    }                                                                   \

    #define GB_DELETE                                                   \
    {                                                                   \
        /* turn C(iC,jC) into a zombie */                               \
        C->nzombies++ ;                                                 \
        Ci [pC] = GB_FLIP (iC) ;                                        \
    }

    #define GB_UNDELETE                                                 \
    {                                                                   \
        /* bring a zombie C(iC,jC) back to life;                 */     \
        /* the value of C(iC,jC) must also be assigned.          */     \
        Ci [pC] = iC ;                                                  \
        C->nzombies-- ;                                                 \
    }

    #define GB_INSERT(aij)                                              \
    {                                                                   \
        /* C(iC,jC) = aij, inserting a pending tuple.  aij is */        \
        /* either A(i,j) or the scalar for scalar expansion */          \
        ASSERT (aij != NULL) ;                                          \
        info = GB_pending_add (C, aij, atype, accum, iC, jC, Context) ; \
        if (info != GrB_SUCCESS)                                        \
        {                                                               \
            /* failed to add pending tuple */                           \
            GB_MATRIX_FREE (&S) ;                                       \
            return (info) ;                                             \
        }                                                               \
    }

    //--------------------------------------------------------------------------
    // C(I,J)<M> = accum (C(I,J),A): consider all cases
    //--------------------------------------------------------------------------

        // The matrix C may have pending tuples and zombies:

        // (1) pending tuples:  this is a list of pending updates held as a set
        // of (i,j,x) tuples.  They had been added to the list via a prior
        // GrB_setElement or GxB_subassign.  No operator needs to be applied to
        // them; the implied operator is SECOND, for both GrB_setElement and
        // GxB_subassign, regardless of whether or not an accum operator is
        // present.  Pending tuples are inserted if and only if the
        // corresponding entry C(i,j) does not exist, and in that case no accum
        // operator is applied.

        //      The GrB_setElement method (C(i,j) = x) is same as GxB_subassign
        //      with: accum is SECOND, C not replaced, no mask M, mask not
        //      complemented.  If GrB_setElement needs to insert its update as
        //      a pending tuple, then it will always be compatible with all
        //      pending tuples inserted here, by GxB_subassign.

        // (2) zombie entries.  These are entries that are still present in the
        // pattern but marked for deletion (via GB_FLIP(i) for the row index).

        // For the current GxB_subassign, there are 16 cases to handle,
        // all combinations of the following options:

        //      accum is NULL, accum is not NULL
        //      C is not replaced, C is replaced
        //      no mask, mask is present
        //      mask is not complemented, mask is complemented

        // Complementing an empty mask:  This does not require the matrix A
        // at all so it is handled as a special case.  It corresponds to
        // the GB_RETURN_IF_QUICK_MASK option in other GraphBLAS operations.
        // Thus only 12 cases are considered in the tables below:

        //      These 4 cases are listed in Four Tables below:
        //      2 cases: accum is NULL, accum is not NULL
        //      2 cases: C is not replaced, C is replaced

        //      3 cases: no mask, M is present and not complemented,
        //               and M is present and complemented.  If there is no
        //               mask, then M(i,j)=1 for all (i,j).  These 3 cases
        //               are the columns of each of the Four Tables.

        // Each of these 12 cases can encounter up to 12 combinations of
        // entries in C, A, and M (6 if no mask M is present).  The left
        // column of the Four Tables below consider all 12 combinations for all
        // (i,j) in the cross product IxJ:

        //      C(I(i),J(j)) present, zombie, or not there: C, X, or '.'
        //      A(i,j) present or not, labeled 'A' or '.' below
        //      M(i,j) = 1 or 0 (but only if M is present)

        //      These 12 cases become the left columns as listed below.
        //      The zombie cases are handled a sub-case for "C present:
        //      regular entry or zombie".  The acronyms below use "D" for
        //      "dot", meaning the entry (C or A) is not present.

        //      [ C A 1 ]   C_A_1: both C and A present, M=1
        //      [ X A 1 ]   C_A_1: both C and A present, M=1, C is a zombie
        //      [ . A 1 ]   D_A_1: C not present, A present, M=1

        //      [ C . 1 ]   C_D_1: C present, A not present, M=1
        //      [ X . 1 ]   C_D_1: C present, A not present, M=1, C a zombie
        //      [ . . 1 ]          only M=1 present, but nothing to do

        //      [ C A 0 ]   C_A_0: both C and A present, M=0
        //      [ X A 0 ]   C_A_0: both C and A present, M=0, C is a zombie
        //      [ . A 0 ]          C not present, A present, M=0,
        //                              nothing to do

        //      [ C . 0 ]   C_D_0: C present, A not present, M=1
        //      [ X . 0 ]   C_D_0: C present, A not present, M=1, C a zombie
        //      [ . . 0 ]          only M=0 present, but nothing to do

        // Legend for action taken in the right half of the table:

        //      delete   live entry C(I(i),J(j)) marked for deletion (zombie)
        //      =A       live entry C(I(i),J(j)) is overwritten with new value
        //      =C+A     live entry C(I(i),J(j)) is modified with accum(c,a)
        //      C        live entry C(I(i),J(j)) is unchanged

        //      undelete entry C(I(i),J(j)) a zombie, bring back with A(i,j)
        //      X        entry C(I(i),J(j)) a zombie, no change, still zombie

        //      insert   entry C(I(i),J(j)) not present, add pending tuple
        //      .        entry C(I(i),J(j)) not present, no change

        //      blank    the table is left blank where the the event cannot
        //               occur:  GxB_subassign with no M cannot have
        //               M(i,j)=0, and GrB_setElement does not have the M
        //               column

        //----------------------------------------------------------------------
        // GrB_setElement and the Four Tables for GxB_subassign:
        //----------------------------------------------------------------------

            //------------------------------------------------------------
            // GrB_setElement:  no mask
            //------------------------------------------------------------

            // C A 1        =A                               |
            // X A 1        undelete                         |
            // . A 1        insert                           |

            //          GrB_setElement acts exactly like GxB_subassign with the
            //          implicit GrB_SECOND_Ctype operator, I=i, J=j, and a
            //          1-by-1 matrix A containing a single entry (not an
            //          implicit entry; there is no "." for A).  That is,
            //          nnz(A)==1.  No mask, and the descriptor is the default;
            //          C_replace effectively false, mask not complemented, A
            //          not transposed.  As a result, GrB_setElement can be
            //          freely mixed with calls to GxB_subassign with C_replace
            //          effectively false and with the identical
            //          GrB_SECOND_Ctype operator.  These calls to
            //          GxB_subassign can use the mask, either complemented or
            //          not, and they can transpose A if desired, and there is
            //          no restriction on I and J.  The matrix A can be any
            //          type and the type of A can change from call to call.

            //------------------------------------------------------------
            // NO accum  |  no mask     mask        mask
            // NO repl   |              not compl   compl
            //------------------------------------------------------------

            // C A 1        =A          =A          C        |
            // X A 1        undelete    undelete    X        |
            // . A 1        insert      insert      .        |

            // C . 1        delete      delete      C        |
            // X . 1        X           X           X        |
            // . . 1        .           .           .        |

            // C A 0                    C           =A       |
            // X A 0                    X           undelete |
            // . A 0                    .           insert   |

            // C . 0                    C           delete   |
            // X . 0                    X           X        |
            // . . 0                    .           .        |

            //          S_Extraction Method works well: first extract pattern
            //          of S=C(I,J). Then examine all of A, M, S, and update
            //          C(I,J).  The method needs to examine all entries in
            //          in C(I,J) to delete them if A is not present, so
            //          S=C(I,J) is not costly.

            //------------------------------------------------------------
            // NO accum  |  no mask     mask        mask
            // WITH repl |              not compl   compl
            //------------------------------------------------------------

            // C A 1        =A          =A          delete   |
            // X A 1        undelete    undelete    X        |
            // . A 1        insert      insert      .        |

            // C . 1        delete      delete      delete   |
            // X . 1        X           X           X        |
            // . . 1        .           .           .        |

            // C A 0                    delete      =A       |
            // X A 0                    X           undelete |
            // . A 0                    .           insert   |

            // C . 0                    delete      delete   |
            // X . 0                    X           X        |
            // . . 0                    .           .        |

            //          S_Extraction Method works well, since all of C(I,J)
            //          needs to be traversed, S=C(I,J) is reasonable to
            //          compute.

            //          With no accum: If there is no M and M is not
            //          complemented, then C_replace is irrelevant,  Whether
            //          true or false, the results in the two tables
            //          above are the same.

            //------------------------------------------------------------
            // ACCUM     |  no mask     mask        mask
            // NO repl   |              not compl   compl
            //------------------------------------------------------------

            // C A 1        =C+A        =C+A        C        |
            // X A 1        undelete    undelete    X        |
            // . A 1        insert      insert      .        |

            // C . 1        C           C           C        |
            // X . 1        X           X           X        |
            // . . 1        .           .           .        |

            // C A 0                    C           =C+A     |
            // X A 0                    X           undelete |
            // . A 0                    .           insert   |

            // C . 0                    C           C        |
            // X . 0                    X           X        |
            // . . 0                    .           .        |

            //          With ACCUM but NO C_replace: This method only needs to
            //          examine entries in A.  It does not need to examine all
            //          entries in C(I,J), nor all entries in M.  Entries in
            //          C but in not A remain unchanged.  This is like an
            //          extended GrB_setElement.  No entries in C can be
            //          deleted.  All other methods must examine all of C(I,J).

            //          Without S_Extraction: C(:,J) or M have many entries
            //          compared with A, do not extract S=C(I,J); use
            //          binary search instead.  Otherwise, use the same
            //          S_Extraction method as the other 3 cases.

            //          S_Extraction Method: if nnz(C(:,j)) + nnz(M) is
            //          similar to nnz(A) then the S_Extraction method would
            //          work well.

            //------------------------------------------------------------
            // ACCUM     |  no mask     mask        mask
            // WITH repl |              not compl   compl
            //------------------------------------------------------------

            // C A 1        =C+A        =C+A        delete   |
            // X A 1        undelete    undelete    X        |
            // . A 1        insert      insert      .        |

            // C . 1        C           C           delete   |
            // X . 1        X           X           X        |
            // . . 1        .           .           .        |

            // C A 0                    delete      =C+A     |
            // X A 0                    X           undelete |
            // . A 0                    .           insert   |

            // C . 0                    delete      C        |
            // X . 0                    X           X        |
            // . . 0                    .           .        |

            //          S_Extraction Method works well since all entries
            //          in C(I,J) must be examined.

            //          With accum: If there is no M and M is not
            //          complemented, then C_replace is irrelavant,  Whether
            //          true or false, the results in the two tables
            //          above are the same.

            //          This condition on C_replace holds with our without
            //          accum.  Thus, if there is no M, and M is
            //          not complemented, the C_replace can be set to false.

            //------------------------------------------------------------

            // ^^^^^ legend for left columns above:
            // C        prior entry C(I(i),J(j)) exists
            // X        prior entry C(I(i),J(j)) exists but is a zombie
            // .        no prior entry C(I(i),J(j))
            //   A      A(i,j) exists
            //   .      A(i,j) does not exist
            //     1    M(i,j)=1, assuming M exists (or if implicit)
            //     0    M(i,j)=0, only if M exists

        //----------------------------------------------------------------------
        // Actions in the Four Tables above
        //----------------------------------------------------------------------

            // Each entry in the Four Tables above are now explained in more
            // detail, describing what must be done in each case.  Zombies and
            // pending tuples are disjoint; they do not mix.  Zombies are IN
            // the pattern but pending tuples are updates that are NOT in the
            // pattern.  That is why a separate list of pending tuples must be
            // kept; there is no place for them in the pattern.  Zombies, on
            // the other hand, are entries IN the pattern that have been
            // marked for deletion.

            //--------------------------------
            // For entries NOT in the pattern:
            //--------------------------------

            // They can have pending tuples, and can acquire more.  No zombies.

            //      ( insert ):

            //          An entry C(I(i),J(j)) is NOT in the pattern, but its
            //          value must be modified.  This is an insertion, like
            //          GrB_setElement, and the insertion is added as a pending
            //          tuple for C(I(i),J(j)).  There can be many insertions
            //          to the same element, each in the list of pending
            //          tuples, in order of their insertion.  Eventually these
            //          pending tuples must be assembled into C(I(i),J(j)) in
            //          the right order using the implied SECOND operator.

            //      ( . ):

            //          no change.  C(I(i),J(j)) not in the pattern, and not
            //          modified.  This C(I(i),J(j)) position could have
            //          pending tuples, in the list of pending tuples, but none
            //          of them are changed.  If C_replace is true then those
            //          pending tuples would have to be discarded, but that
            //          condition will not occur because C_replace=true forces
            //          all prior tuples to the matrix to be assembled.

            //--------------------------------
            // For entries IN the pattern:
            //--------------------------------

            // They have no pending tuples, and acquire none.  It can be
            // zombie, can become a zombie, or a zombie can come back to life.

            //      ( delete ):

            //          C(I(i),J(j)) becomes a zombie, by flipping its row
            //          index via the GB_FLIP function.

            //      ( undelete ):

            //          C(I(i),J(j)) = A(i,j) was a zombie and is no longer a
            //          zombie.  Its row index is restored with GB_FLIP.

            //      ( X ):

            //          C(I(i),J(j)) was a zombie, and still is a zombie.
            //          row index is < 0, and actual index is GB_FLIP(I(i))

            //      ( C ):

            //          no change; C(I(i),J(j)) already an entry, and its value
            //          doesn't change.

            //      ( =A ):

            //          C(I(i),J(j)) = A(i,j), value gets overwritten.

            //      ( =C+A ):

            //          C(I(i),J(j)) = accum (C(I(i),J(j)), A(i,j))
            //          The existing balue is modified via the accumulator.


    //--------------------------------------------------------------------------
    // handling each action
    //--------------------------------------------------------------------------

        // Each of the 12 cases are handled by the following actions,
        // implemented as macros.  The Four Tables are re-sorted below,
        // and folded together according to their left column.

        // Once the M(i,j) entry is extracted, the codes below explicitly
        // complement the scalar value if Mask_complement is true, before using
        // these action functions.  For the [no mask] case, M(i,j)=1.  Thus,
        // only the middle column needs to be considered by each action; the
        // action will handle all three columns at the same time.  All three
        // columns remain in the re-sorted tables below for reference.

        //----------------------------------------------------------------------
        // ----[C A 1] or [X A 1]: C and A present, M=1
        //----------------------------------------------------------------------

            //------------------------------------------------
            //           |  no mask     mask        mask
            //           |              not compl   compl
            //------------------------------------------------
            // C A 1        =A          =A          C        | no accum,no Crepl
            // C A 1        =A          =A          delete   | no accum,Crepl
            // C A 1        =C+A        =C+A        C        | accum, no Crepl
            // C A 1        =C+A        =C+A        delete   | accum, Crepl

            // X A 1        undelete    undelete    X        | no accum,no Crepl
            // X A 1        undelete    undelete    X        | no accum,Crepl
            // X A 1        undelete    undelete    X        | accum, no Crepl
            // X A 1        undelete    undelete    X        | accum, Crepl

            // Both C(I(i),J(j)) == S(i,j) and A(i,j) are present, and mij = 1.
            // C(I(i),J(i)) is updated with the entry A(i,j).
            // C_replace has no impact on this action.

            // [X A 1] matrix case
            #define GB_X_A_1_matrix                                         \
            {                                                               \
                /* ----[X A 1]                                           */ \
                /* action: ( undelete ): bring a zombie back to life     */ \
                GB_UNDELETE ;                                               \
                GB_COPY_aij_to_C ;                                          \
            }

            // [X A 1] scalar case
            #define GB_X_A_1_scalar                                         \
            {                                                               \
                /* ----[X A 1]                                           */ \
                /* action: ( undelete ): bring a zombie back to life     */ \
                GB_UNDELETE ;                                               \
                GB_COPY_scalar_to_C ;                                       \
            }

            // [C A 1] scalar case, with accum
            #define GB_C_A_1_accum_matrix                                   \
            {                                                               \
                /* ----[C A 1] with accum, scalar expansion              */ \
                /* action: ( =C+A ): apply the accumulator               */ \
                GB_COPY_aij_to_ywork ;                                      \
                GB_ACCUMULATE ;                                             \
            }                                                               \

            // [C A 1] scalar case, with accum
            #define GB_C_A_1_accum_scalar                                   \
            {                                                               \
                /* ----[C A 1] with accum, scalar expansion              */ \
                /* action: ( =C+A ): apply the accumulator               */ \
                GB_ACCUMULATE ;                                             \
            }

            // [C A 1] matrix case when accum is present
            #define GB_withaccum_C_A_1_matrix                               \
            {                                                               \
                ASSERT (accum != NULL) ;                                    \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X A 1]                                       */ \
                    /* action: ( undelete ): bring a zombie back to life */ \
                    GB_X_A_1_matrix ;                                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C A 1] with accum, scalar expansion          */ \
                    /* action: ( =C+A ): apply the accumulator           */ \
                    GB_C_A_1_accum_matrix ;                                 \
                }                                                           \
            }

            // [C A 1] matrix case when no accum is present
            #define GB_noaccum_C_A_1_matrix                                 \
            {                                                               \
                ASSERT (accum == NULL) ;                                    \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X A 1]                                       */ \
                    /* action: ( undelete ): bring a zombie back to life */ \
                    GB_X_A_1_matrix ;                                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C A 1] no accum, scalar expansion            */ \
                    /* action: ( =A ): copy A into C                     */ \
                    GB_COPY_aij_to_C ;                                      \
                }                                                           \
            }

            // [C A 1] scalar case when accum is present
            #define GB_withaccum_C_A_1_scalar                               \
            {                                                               \
                ASSERT (accum != NULL) ;                                    \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X A 1]                                       */ \
                    /* action: ( undelete ): bring a zombie back to life */ \
                    GB_X_A_1_scalar ;                                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C A 1] with accum, scalar expansion          */ \
                    /* action: ( =C+A ): apply the accumulator           */ \
                    GB_C_A_1_accum_scalar ;                                 \
                }                                                           \
            }

            // [C A 1] scalar case when no accum is present
            #define GB_noaccum_C_A_1_scalar                                 \
            {                                                               \
                ASSERT (accum == NULL) ;                                    \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X A 1]                                       */ \
                    /* action: ( undelete ): bring a zombie back to life */ \
                    GB_X_A_1_scalar ;                                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C A 1] no accum, scalar expansion            */ \
                    /* action: ( =A ): copy A into C                     */ \
                    GB_COPY_scalar_to_C ;                                   \
                }                                                           \
            }

        //----------------------------------------------------------------------
        // ----[. A 1]: C not present, A present, M=1
        //----------------------------------------------------------------------

            //------------------------------------------------
            //           |  no mask     mask        mask
            //           |              not compl   compl
            //------------------------------------------------
            // . A 1        insert      insert      .        | no accum,no Crepl
            // . A 1        insert      insert      .        | no accum,Crepl
            // . A 1        insert      insert      .        | accum, no Crepl
            // . A 1        insert      insert      .        | accum, Crepl

            // C(I(i),J(j)) == S (i,j) is not present, A (i,j) is present, and
            // mij = 1. The mask M allows C to be written, but no entry present
            // in C (neither a live entry nor a zombie).  This entry must be
            // added to C but it doesn't fit in the pattern.  It is added as a
            // pending tuple.  Zombies and pending tuples do not intersect.

            // If adding the pending tuple fails, C is cleared entirely.
            // Otherwise the matrix C would be left in an incoherent partial
            // state of computation.  It's cleaner to just free it all.

            #define GB_D_A_1_scalar                                         \
            {                                                               \
                /* ----[. A 1]                                           */ \
                /* action: ( insert )                                    */ \
                GB_INSERT (scalar) ;                                        \
            }

            #define GB_D_A_1_matrix                                         \
            {                                                               \
                /* ----[. A 1]                                           */ \
                /* action: ( insert )                                    */ \
                GB_INSERT (Ax +(pA*asize)) ;                                \
            }

        //----------------------------------------------------------------------
        // ----[C . 1] or [X . 1]: C present, A not present, M=1
        //----------------------------------------------------------------------

            //------------------------------------------------
            //           |  no mask     mask        mask
            //           |              not compl   compl
            //------------------------------------------------
            // C . 1        delete      delete      C        | no accum,no Crepl
            // C . 1        delete      delete      delete   | no accum,Crepl
            // C . 1        C           C           C        | accum, no Crepl
            // C . 1        C           C           delete   | accum, Crepl

            // X . 1        X           X           X        | no accum,no Crepl
            // X . 1        X           X           X        | no accum,Crepl
            // X . 1        X           X           X        | accum, no Crepl
            // X . 1        X           X           X        | accum, Crepl

            // C(I(i),J(j)) == S (i,j) is present, A (i,j) not is present, and
            // mij = 1. The mask M allows C to be written, but no entry present
            // in A.  If no accum operator is present, C becomes a zombie.

            // This condition cannot occur if A is a dense matrix,
            // nor for scalar expansion

            // [C . 1] matrix case when no accum is present
            #define GB_noaccum_C_D_1_matrix                                 \
            {                                                               \
                ASSERT (!scalar_expansion && !is_dense) ;                   \
                ASSERT (accum == NULL) ;                                    \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X . 1]                                       */ \
                    /* action: ( X ): still a zombie                     */ \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C . 1] no accum                              */ \
                    /* action: ( delete ): becomes a zombie              */ \
                    GB_DELETE ;                                             \
                }                                                           \
            }

        //----------------------------------------------------------------------
        // ----[C A 0] or [X A 0]: both C and A present but M=0
        //----------------------------------------------------------------------

            //------------------------------------------------
            //           |  no mask     mask        mask
            //           |              not compl   compl
            //------------------------------------------------
            // C A 0                    C           =A       | no accum,no Crepl
            // C A 0                    delete      =A       | no accum,Crepl
            // C A 0                    C           =C+A     | accum, no Crepl
            // C A 0                    delete      =C+A     | accum, Crepl

            // X A 0                    X           undelete | no accum,no Crepl
            // X A 0                    X           undelete | no accum,Crepl
            // X A 0                    X           undelete | accum, no Crepl
            // X A 0                    X           undelete | accum, Crepl

            // Both C(I(i),J(j)) == S(i,j) and A(i,j) are present, and mij = 0.
            // The mask prevents A being written to C, so A has no effect on
            // the result.  If C_replace is true, however, the entry is
            // deleted, becoming a zombie.  This case does not occur if
            // the mask M is not present.  This action also handles the
            // [C . 0] and [X . 0] cases; see the next section below.

            // This condition can still occur if A is dense, so if a mask M is
            // present, entries can still be deleted from C.  As a result, the
            // fact that A is dense cannot be exploited when the mask M is
            // present.

            #define GB_C_A_0                                                \
            {                                                               \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X A 0]                                       */ \
                    /* ----[X . 0]                                       */ \
                    /* action: ( X ): still a zombie                     */ \
                }                                                           \
                else if (C_replace)                                         \
                {                                                           \
                    /* ----[C A 0] replace                               */ \
                    /* ----[C . 0] replace                               */ \
                    /* action: ( delete ): becomes a zombie              */ \
                    GB_DELETE ;                                             \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C A 0] no replace                            */ \
                    /* ----[C . 0] no replace                            */ \
                    /* action: ( C ): no change                          */ \
                }                                                           \
            }

            // The above action is very similar to C_D_1.  The only difference
            // is how the entry C becomes a zombie.  With C_D_1, there is no
            // entry in A, so C becomes a zombie if no accum function is used
            // because the implicit value A(i,j) gets copied into C, causing it
            // to become an implicit value also (deleting the entry in C).
            // With C_A_0, the entry C is protected from any modification from
            // A (regardless of accum or not).  However, if C_replace is true,
            // the entry is cleared.  The mask M does not protect C from the
            // C_replace action.

        //----------------------------------------------------------------------
        // ----[C . 0] or [X . 0]: C present, A not present, M=0
        //----------------------------------------------------------------------

            //------------------------------------------------
            //           |  no mask     mask        mask
            //           |              not compl   compl
            //------------------------------------------------

            // C . 0                    C           delete   | no accum,no Crepl
            // C . 0                    delete      delete   | no accum,Crepl
            // C . 0                    C           C        | accum, no Crepl
            // C . 0                    delete      C        | accum, Crepl

            // X . 0                    X           X        | no accum,no Crepl
            // X . 0                    X           X        | no accum,Crepl
            // X . 0                    X           X        | accum, no Crepl
            // X . 0                    X           X        | accum, Crepl

            // C(I(i),J(j)) == S(i,j) is present, but A(i,j) is not present,
            // and mij = 0.  Since A(i,j) has no effect on the result,
            // this is the same as the C_A_0 action above.

            // This condition cannot occur if A is a dense matrix, nor for
            // scalar expansion, but the existance of the entry A is not
            // relevant.

            #define GB_C_D_0 GB_C_A_0

        //----------------------------------------------------------------------
        // ----[. A 0]: C not present, A present, M=0
        //----------------------------------------------------------------------

            // . A 0                    .           insert   | no accum,no Crepl
            // . A 0                    .           insert   | no accum,no Crepl
            // . A 0                    .           insert   | accum, no Crepl
            // . A 0                    .           insert   | accum, Crepl

            // C(I(i),J(j)) == S(i,j) is not present, A(i,j) is present,
            // but mij = 0.  The mask M prevents A from modifying C, so the
            // A(i,j) entry is ignored.  C_replace has no effect since the
            // entry is already cleared.  There is nothing to do.

        //----------------------------------------------------------------------
        // ----[. . 1] and [. . 0]: no entries in C and A, M = 0 or 1
        //----------------------------------------------------------------------

            //------------------------------------------------
            //           |  no mask     mask        mask
            //           |              not compl   compl
            //------------------------------------------------

            // . . 1        .           .           .        | no accum,no Crepl
            // . . 1        .           .           .        | no accum,Crepl
            // . . 1        .           .           .        | accum, no Crepl
            // . . 1        .           .           .        | accum, Crepl

            // . . 0        .           .           .        | no accum,no Crepl
            // . . 0        .           .           .        | no accum,Crepl
            // . . 0        .           .           .        | accum, no Crepl
            // . . 0        .           .           .        | accum, Crepl

            // Neither C(I(i),J(j)) == S(i,j) nor A(i,j) are not present,
            // Nothing happens.  The M(i,j) entry is present, otherwise
            // this (i,j) position would not be considered at all.
            // The M(i,j) entry has no effect.  There is nothing to do.

    //--------------------------------------------------------------------------
    // check for quicker method if accum is present and C_replace is false
    //--------------------------------------------------------------------------

    // Before allocating S, see if there is a faster method
    // that does not require S to be created.

    const int64_t *Ch = C->h ;
    const int64_t *Cp = C->p ;
    int64_t *Ci = C->i ;
    GB_void *Cx = C->x ;
    const size_t csize = C->type->size ;
    const GB_Type_code ccode = C->type->code ;
    int64_t cnvec = C->nvec ;

    // if C is hypersparse but all vectors are present, then
    // treat C as if it were non-hypersparse
    bool C_is_hyper = C->is_hyper && (cnvec < cvdim) ;

    bool S_Extraction = true ;

    bool C_Mask_scalar = (scalar_expansion && !C_replace &&
        M != NULL && !Mask_comp) ;

    if (C_Mask_scalar)
    { 
        // C(I,J)<M>=scalar or +=scalar, mask present and not complemented,
        // C_replace false, special case
        S_Extraction = false ;
    }
    else if (accum != NULL && !C_replace)
    {
        // If accum is present and C_replace is false, then only entries in A
        // need to be examined.  Not all entries in C(I,J) and M need to be
        // examined.  As a result, computing S=C(I,J) can dominate the time and
        // memory required for the S_Extraction method.
        int64_t cnz = GB_NNZ (C) ;
        if (nI == 1 || nJ == 1 || cnz == 0)
        { 
            // No need to form S if it has just a single row or column.  If C
            // is empty so is S, so don't bother computing it.
            S_Extraction = false ;
        }
        else if (mn_ok && cnz + nzMask > anz)
        {
            // If C and M are very dense, then do not extract S
            int64_t cnz = nzMask ;
            for (int64_t j = 0 ; S_Extraction && j < nJ ; j++)
            {
                // jC = J [j] ; or J is a colon expression
                int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;
                if (jC < 0 || jC >= cvdim)
                { 
                    // invalid vector; check them all in GB_subref_symbolic
                    break ;
                }
                // get the C(:,jC) vector where jC = J [j]
                GB_jC_LOOKUP ;
                cnz += pC_end - pC_start ;
                if (cnz/8 > anz)
                { 
                    // nnz(C) + nnz(M) is much larger than nnz(A).  Do not
                    // construct S=C(I,J).  Instead, scan through all of A and
                    // use binary search to find the corresponding positions in
                    // C and M.
                    S_Extraction = false ;
                }
            }
            if (cnz == 0)
            { 
                // C(:,J) and M(:,J) are empty; no need to compute S
                S_Extraction = false ;
            }
        }
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

    GrB_Matrix S = NULL ;
    const int64_t *Si = NULL ;
    const int64_t *Sx = NULL ;

    if (S_Extraction)
    {

        //----------------------------------------------------------------------
        // extract symbolic structure S=C(I,J)
        //----------------------------------------------------------------------

        // S and C have the same CSR/CSC format.  S is always returned sorted,
        // in the same hypersparse form as C. This step also checks I and J.
        info = GB_subref_symbolic (&S, C->is_csc, C, I, ni, J, nj, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory or invalid indices I and J
            // the C matrix is unchanged, no need to clear it
            return (info) ;
        }

        ASSERT_OK (GB_check (C, "C for subref extraction", GB0)) ;
        ASSERT_OK (GB_check (S, "S extraction", GB0)) ;

        Si = S->i ;
        Sx = S->x ;

        #ifndef NDEBUG
        // this body of code explains what S contains.
        // S is nI-by-nJ where nI = length (I) and nJ = length (J)
        GB_for_each_vector (S)
        {
            // prepare to iterate over the entries of vector S(:,jnew)
            int64_t GBI1_initj (Iter, jnew, pS_start, pS_end) ;
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
                int64_t pC_start, pC_end, pleft = 0, pright = cnvec-1 ;
                bool found = GB_lookup (C_is_hyper, Ch, Cp, &pleft, pright, jC,
                    &pC_start, &pC_end) ;
                ASSERT (found) ;
                // If iC == I [inew] and jC == J [jnew], (or the equivaleent
                // for GB_ALL, GB_RANGE, GB_STRIDE) then A(inew,jnew) will be
                // assigned to C(iC,jC), and p = S(inew,jnew) gives the pointer
                // into C to where the entry (C(iC,jC) appears in C:
                ASSERT (pC_start <= p && p < pC_end) ;
                ASSERT (iC == GB_UNFLIP (Ci [p])) ;
            }
        }
        #endif

    }
    else
    {

        //----------------------------------------------------------------------
        // do not create S=C(I,J), but do check I and J
        //----------------------------------------------------------------------

        // GB_subref_symbolic does this task, so do it here when S is not
        // computed.  Make sure I and J are valid for C(I,J)=A

        bool I_unsorted, I_contig, J_unsorted, J_contig ;
        int64_t imin, imax, jmin, jmax ;
        info = GB_ijproperties (I, ni, nI, cvlen, Ikind, Icolon,
                    &I_unsorted, &I_contig, &imin, &imax, true, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // I invalid
            return (info) ;
        }
        info = GB_ijproperties (J, nj, nJ, cvdim, Jkind, Jcolon,
                    &J_unsorted, &J_contig, &jmin, &jmax, false, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // J invalid
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // check if an empty mask is complemented (case: C_replace true)
    //--------------------------------------------------------------------------

    if (Mask_comp && M == NULL)
    {
        // C_replace==false has already been handled; see "No work..." above
        // S = C(I,J) is required, and has just been computed above
        ASSERT (C_replace) ;
        ASSERT (S != NULL) ;
        ASSERT_OK (GB_check (C, "C: empty mask compl, C_replace true", GB0)) ;
        ASSERT_OK (GB_check (S, "S pattern", GB0)) ;

        // C(I,J) = "zero"; turn all entries in C(I,J) into zombies

        GB_for_each_vector (S)
        {
            GB_for_each_entry (jnew, pS, pS_end)
            {
                // S (inew,jnew) is a pointer back into C (I(inew), J(jnew))
                GB_C_S_LOOKUP ;
                if (!is_zombie)
                { 
                    // ----[C - 0] replace
                    // action: ( delete ): becomes a zombie
                    GB_DELETE ;
                }
            }
        }

        //----------------------------------------------------------------------
        // insert C in the queue if it has work to do and isn't already queued
        //----------------------------------------------------------------------

        GB_CRITICAL (GB_queue_insert (C)) ;
        ASSERT_OK (GB_check(C, "C: empty mask compl, C_replace, done", GB0)) ;

        //----------------------------------------------------------------------
        // free workspace, check blocking mode, and return
        //----------------------------------------------------------------------

        GB_MATRIX_FREE (&S) ;
        return (GB_block (C, Context)) ;
    }

    // C_replace can now only be true if the mask M is present.  If the mask M
    // is not present, then C_replace is now effectively false.

    //--------------------------------------------------------------------------
    // scalar workspace
    //--------------------------------------------------------------------------

    const int64_t *Ai = NULL ;
    const GB_void *Ax = NULL ;

    size_t xsize = 1, ysize = 1, zsize = 1 ;
    if (accum != NULL)
    { 
        xsize = accum->xtype->size ;
        ysize = accum->ytype->size ;
        zsize = accum->ztype->size ;
    }

    char cwork [csize] ;        // cwork = (ctype) scalar
    char xwork [xsize] ;        // xwork = (xtype) C(i,j)
    char ywork [ysize] ;        // ywork = (ytype) scalar or A(i,j)
    char zwork [zsize] ;        // zwork = faccum(xwork,ywork)
                                //         then C(i,J) = (ctype) zwork

    if (!scalar_expansion)
    { 
        Ai = A->i ;
        Ax = A->x ;
    }

    //--------------------------------------------------------------------------
    // get casting functions
    //--------------------------------------------------------------------------

    // if any of these are user-defined types, the "cast" is just a
    // user-to-user copy, using GB_copy_user_user.  See GB_cast_factory.
    GB_cast_function cast_A_to_C, cast_A_to_Y, cast_C_to_X, cast_Z_to_C ;
    cast_A_to_C = GB_cast_factory (ccode, acode) ;

    GxB_binary_function faccum = NULL ;

    cast_A_to_Y = NULL ;
    cast_C_to_X = NULL ;
    cast_Z_to_C = NULL ;

    if (accum != NULL)
    { 
        faccum = accum->function ;
        cast_A_to_Y = GB_cast_factory (accum->ytype->code, acode) ;
        cast_C_to_X = GB_cast_factory (accum->xtype->code, ccode) ;
        cast_Z_to_C = GB_cast_factory (ccode, accum->ztype->code) ;
    }

    if (scalar_expansion)
    {
        // cast the single scalar into the type of C, and the type of Y
        cast_A_to_C (cwork, scalar, asize) ;
        if (accum != NULL)
        { 
            cast_A_to_Y (ywork, scalar, asize) ;
        }
    }

    //--------------------------------------------------------------------------
    // get the mask M
    //--------------------------------------------------------------------------

    const int64_t *Mi = NULL ;
    const GB_void *Mx = NULL ;
    size_t msize = 0 ;
    GB_cast_function cast_M = NULL ;

    if (M != NULL)
    { 
        Mi = M->i ;
        Mx = M->x ;
        msize = M->type->size ;
        cast_M = GB_cast_factory (GB_BOOL_code, M->type->code) ;
        ASSERT_OK (GB_check (M, "M for assign", GB0)) ;
    }

    //==========================================================================
    // submatrix assignment C(I,J)<M> = accum (C(I,J),A): meta-algorithm
    //==========================================================================

    if (C_Mask_scalar)
    {

        //----------------------------------------------------------------------
        // METHOD 1: C(I,J)<M> = scalar or +=scalar, !C_replace, !Mask_comp
        //----------------------------------------------------------------------

        // This method iterates across all entries in the mask M, and for each
        // place where the M(i,j) is true, it updates C(i,J(j)).  Not all of C
        // needs to be examined.  Also, since I=":", the row indices of the
        // mask M are the same as the row indices of C.  The accum operator may
        // be present, or absent.  J can be anything: a list, ":", begin:end,
        // or begin:inc:end.  No entries can be deleted from C.

        ASSERT (scalar_expansion) ;         // A is a scalar
        ASSERT (M != NULL && !Mask_comp) ;  // mask M present, not compl.
        ASSERT (!C_replace) ;               // C_replace is false

        if (accum == NULL)
        {

            //------------------------------------------------------------------
            // METHOD 1a (no accum): C(I,J)<M> = scalar
            //------------------------------------------------------------------

            GB_for_each_vector (M)
            {
                int64_t GBI1_initj (Iter, j, pM, pM_end) ;

                // get the C(:,jC) vector where jC = J [j]
                int64_t GB_jC_LOOKUP ;

                if (pC_end - pC_start == cvlen)
                {

                    // C(:,jC) is dense so the binary search of C is not needed
                    GB_for_each_entry (j, pM, pM_end)
                    {

                        //------------------------------------------------------
                        // consider the entry M(i,j)
                        //------------------------------------------------------

                        bool mij ;
                        cast_M (&mij, Mx +(pM*msize), 0) ;

                        //------------------------------------------------------
                        // update C(iC,jC), but only if M(iC,j) allows it
                        //------------------------------------------------------

                        if (mij)
                        { 

                            //--------------------------------------------------
                            // C(iC,jC) = scalar
                            //--------------------------------------------------

                            int64_t iA = Mi [pM] ;
                            GB_CDENSE_I_LOOKUP ;

                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =A ): copy A into C, no accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_noaccum_C_A_1_scalar ;
                        }
                    }
                }
                else
                {

                    // C(:,jC) is sparse; use binary search for C
                    GB_for_each_entry (j, pM, pM_end)
                    {

                        //------------------------------------------------------
                        // consider the entry M(i,j)
                        //------------------------------------------------------

                        bool mij ;
                        cast_M (&mij, Mx +(pM*msize), 0) ;

                        //------------------------------------------------------
                        // find C(iC,jC), but only if M(i,j) allows it
                        //------------------------------------------------------

                        if (mij)
                        {

                            //--------------------------------------------------
                            // C(iC,jC) = scalar
                            //--------------------------------------------------

                            // binary search for C(iC,jC) in C(:,jC)
                            int64_t iA = Mi [pM] ;
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_iC_BINARY_SEARCH ;

                            if (found)
                            { 
                                // ----[C A 1] or [X A 1]-----------------------
                                // [C A 1]: action: ( =A ): A to C, no accum
                                // [X A 1]: action: ( undelete ): zombie lives
                                GB_noaccum_C_A_1_scalar ;
                            }
                            else
                            { 
                                // ----[. A 1]----------------------------------
                                // action: ( insert )
                                GB_D_A_1_scalar ;
                            }
                        }
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // METHOD 1b (accum): C(I,J)<M> += scalar
            //------------------------------------------------------------------

            GB_for_each_vector (M)
            {
                int64_t GBI1_initj (Iter, j, pM, pM_end) ;

                // get the C(:,jC) vector where jC = J [j]
                int64_t GB_jC_LOOKUP ;

                if (pC_end - pC_start == cvlen)
                {

                    // C(:,jC) is dense so the binary search of C is not needed
                    GB_for_each_entry (j, pM, pM_end)
                    {

                        //------------------------------------------------------
                        // consider the entry M(i,j)
                        //------------------------------------------------------

                        bool mij ;
                        cast_M (&mij, Mx +(pM*msize), 0) ;

                        //------------------------------------------------------
                        // update C(iC,jC), but only if M(iC,j) allows it
                        //------------------------------------------------------

                        if (mij)
                        { 

                            //--------------------------------------------------
                            // C(iC,jC) += scalar
                            //--------------------------------------------------

                            int64_t iA = Mi [pM] ;
                            GB_CDENSE_I_LOOKUP ;

                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_withaccum_C_A_1_scalar ;
                        }
                    }
                }
                else
                {

                    // C(:,jC) is sparse; use binary search for C
                    GB_for_each_entry (j, pM, pM_end)
                    {

                        //------------------------------------------------------
                        // consider the entry M(i,j)
                        //------------------------------------------------------

                        bool mij ;
                        cast_M (&mij, Mx +(pM*msize), 0) ;

                        //------------------------------------------------------
                        // find C(iC,jC), but only if M(i,j) allows it
                        //------------------------------------------------------

                        if (mij)
                        {

                            //--------------------------------------------------
                            // C(iC,jC) += scalar
                            //--------------------------------------------------

                            // binary search for C(iC,jC) in C(:,jC)
                            int64_t iA = Mi [pM] ;
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_iC_BINARY_SEARCH ;

                            if (found)
                            { 
                                // ----[C A 1] or [X A 1]-----------------------
                                // [C A 1]: action: ( =C+A ): apply accum
                                // [X A 1]: action: ( undelete ): zombie lives
                                GB_withaccum_C_A_1_scalar ;
                            }
                            else
                            { 
                                // ----[. A 1]----------------------------------
                                // action: ( insert )
                                GB_D_A_1_scalar ;
                            }
                        }
                    }
                }
            }
        }

    }
    else if (S == NULL)
    {

        //----------------------------------------------------------------------
        // METHOD 2: assignment without S, accum present, C_replace false
        //----------------------------------------------------------------------

        // This method can only be used if accum is present and C_replace is
        // false.  It iterates over the entries in A and ignores any part of C
        // outside of the pattern of A.  It handles any case of the mask:
        // present or NULL, and complemented or not complemented.
        // A can be a matrix or a scalar.  No entries in C are deleted.

        ASSERT (accum != NULL) ;
        ASSERT (C_replace == false) ;

        if (scalar_expansion)
        {

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // METHOD 2a: C(I,J) += scalar, not using S
                //--------------------------------------------------------------

                for (int64_t j = 0 ; j < nJ ; j++)
                {
                    // get the C(:,jC) vector where jC = J [j]
                    int64_t GB_jC_LOOKUP ;

                    if (pC_end - pC_start == cvlen)
                    {

                        //------------------------------------------------------
                        // C(:,jC) is dense so binary search of C is not needed
                        //------------------------------------------------------

                        // for each iA in I [...]:
                        for (int64_t iA = 0 ; iA < nI ; iA++)
                        { 

                            //--------------------------------------------------
                            // C(iC,jC) += scalar
                            //--------------------------------------------------

                            // direct lookup of C(iC,jC)
                            GB_CDENSE_I_LOOKUP ;

                            // ----[C A 1] or [X A 1]-----------------------
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_withaccum_C_A_1_scalar ;
                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // C(:,jC) is sparse; use binary search for C
                        //------------------------------------------------------

                        // for each iA in I [...]:
                        for (int64_t iA = 0 ; iA < nI ; iA++)
                        {

                            //--------------------------------------------------
                            // C(iC,jC) += scalar
                            //--------------------------------------------------

                            // iC = I [iA] ; or I is a colon expression
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            // binary search for C(iC,jC) in C(:,jC)
                            GB_iC_BINARY_SEARCH ;

                            if (found)
                            { 
                                // ----[C A 1] or [X A 1]-----------------------
                                // [C A 1]: action: ( =C+A ): apply accum
                                // [X A 1]: action: ( undelete ): zombie back
                                GB_withaccum_C_A_1_scalar ;
                            }
                            else
                            { 
                                // ----[. A 1]----------------------------------
                                // action: ( insert )
                                GB_D_A_1_scalar ;
                            }
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // METHOD 2b: C(I,J)<!M> += scalar, not using S
                //--------------------------------------------------------------

                // C(I,J)<M> += scalar, not using S, and C_replace false
                // already handled by the C_Mask_scalar case
                ASSERT (Mask_comp) ;
                ASSERT (!C_replace) ;

                GB_for_each_vector2s (M /*, scalar */)
                {

                    int64_t GBI1_initj (Iter, j, pM, pM_end) ;
                    // get the C(:,jC) vector where jC = J [j]
                    int64_t GB_jC_LOOKUP ;

                    if (pC_end - pC_start == cvlen)
                    {

                        //------------------------------------------------------
                        // C(:,jC) is dense so binary search of C is not needed
                        //------------------------------------------------------

                        // for each iA in I [...]:
                        for (int64_t iA = 0 ; iA < nI ; iA++)
                        {

                            //--------------------------------------------------
                            // find M(iA,j)
                            //--------------------------------------------------

                            bool mij ;
                            bool found = (pM < pM_end) && (Mi [pM] == iA) ;
                            if (found)
                            { 
                                // found it
                                cast_M (&mij, Mx +(pM*msize), 0) ;
                                GB_NEXT (M) ;
                            }
                            else
                            { 
                                // M(iA,j) not present, implicitly false
                                mij = false ;
                            }
                            if (Mask_comp)
                            { 
                                // negate the mask M if Mask_comp is true
                                mij = !mij ;
                            }

                            //--------------------------------------------------
                            // find C(iC,jC), but only if M(iA,j) allows it
                            //--------------------------------------------------

                            if (mij)
                            { 

                                //----------------------------------------------
                                // C(iC,jC) += scalar
                                //----------------------------------------------

                                // direct lookup of C(iC,jC)
                                GB_CDENSE_I_LOOKUP ;

                                // ----[C A 1] or [X A 1]-------------------
                                // [C A 1]: action: ( =C+A ): apply accum
                                // [X A 1]: action: ( undelete ) zombie live
                                GB_withaccum_C_A_1_scalar ;
                            }
                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // C(:,jC) is sparse; use binary search for C
                        //------------------------------------------------------

                        // for each iA in I [...]:
                        for (int64_t iA = 0 ; iA < nI ; iA++)
                        {

                            //--------------------------------------------------
                            // find M(iA,j)
                            //--------------------------------------------------

                            bool mij ;
                            bool found = (pM < pM_end) && (Mi [pM] == iA) ;
                            if (found)
                            { 
                                // found it
                                cast_M (&mij, Mx +(pM*msize), 0) ;
                                GB_NEXT (M) ;
                            }
                            else
                            { 
                                // M(iA,j) not present, implicitly false
                                mij = false ;
                            }
                            if (Mask_comp)
                            { 
                                // negate the mask M if Mask_comp is true
                                mij = !mij ;
                            }

                            //--------------------------------------------------
                            // find C(iC,jC), but only if M(iA,j) allows it
                            //--------------------------------------------------

                            if (mij)
                            {

                                //----------------------------------------------
                                // C(iC,jC) += scalar
                                //----------------------------------------------

                                // iC = I [iA] ; or I is a colon expression
                                int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                                // binary search for C(iC,jC) in C(:,jC)
                                GB_iC_BINARY_SEARCH ;

                                if (found)
                                { 
                                    // ----[C A 1] or [X A 1]-------------------
                                    // [C A 1]: action: ( =C+A ): apply accum
                                    // [X A 1]: action: ( undelete ) zombie live
                                    GB_withaccum_C_A_1_scalar ;
                                }
                                else
                                { 
                                    // ----[. A 1]------------------------------
                                    // action: ( insert )
                                    GB_D_A_1_scalar ;
                                }
                            }
                        }
                    }
                }
            }

        }
        else
        {

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // METHOD 2c: C(I,J) += A, not using S
                //--------------------------------------------------------------

                GB_for_each_vector (A)
                {

                    int64_t GBI1_initj (Iter, j, pA, pA_end) ;
                    // get the C(:,jC) vector where jC = J [j]
                    int64_t GB_jC_LOOKUP ;

                    if (pC_end - pC_start == cvlen)
                    {

                        //------------------------------------------------------
                        // C(:,jC) is dense so binary search of C is not needed
                        //------------------------------------------------------

                        for ( ; pA < pA_end ; pA++)
                        {

                            //--------------------------------------------------
                            // consider the entry A(iA,j)
                            //--------------------------------------------------

                            int64_t iA = Ai [pA] ;

                            //--------------------------------------------------
                            // C(iC,jC) += A(iA,j)
                            //--------------------------------------------------

                            // direct lookup of C(iC,jC)
                            GB_CDENSE_I_LOOKUP ;

                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): zombie lives
                            GB_withaccum_C_A_1_matrix ;

                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // C(:,jC) is sparse; use binary search for C
                        //------------------------------------------------------

                        for ( ; pA < pA_end ; pA++)
                        {


                            //--------------------------------------------------
                            // consider the entry A(iA,j)
                            //--------------------------------------------------

                            int64_t iA = Ai [pA] ;

                            //--------------------------------------------------
                            // C(iC,jC) += A(iA,j)
                            //--------------------------------------------------

                            // iC = I [iA] ; or I is a colon expression
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            // binary search for C(iC,jC) in C(:,jC)
                            GB_iC_BINARY_SEARCH ;

                            if (found)
                            { 
                                // ----[C A 1] or [X A 1]-----------------------
                                // [C A 1]: action: ( =C+A ): apply accum
                                // [X A 1]: action: ( undelete ): zombie lives
                                GB_withaccum_C_A_1_matrix ;
                            }
                            else
                            { 
                                // ----[. A 1]----------------------------------
                                // action: ( insert )
                                GB_D_A_1_matrix ;
                            }
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // METHOD 2d: C(I,J)<M> += A, not using S
                //--------------------------------------------------------------

                GB_for_each_vector2 (A, M)
                {

                    int64_t GBI2_initj (Iter, j, pA, pA_end, pM_start, pM_end) ;
                    // get the C(:,jC) vector where jC = J [j]
                    int64_t GB_jC_LOOKUP ;

                    if (pC_end - pC_start == cvlen)
                    {

                        //------------------------------------------------------
                        // C(:,jC) is dense so binary search of C is not needed
                        //------------------------------------------------------

                        for ( ; pA < pA_end ; pA++)
                        {

                            //--------------------------------------------------
                            // consider the entry A(iA,j)
                            //--------------------------------------------------

                            int64_t iA = Ai [pA] ;

                            //--------------------------------------------------
                            // find M(iA,j)
                            //--------------------------------------------------

                            bool mij = true ;
                            int64_t pM     = pM_start ;
                            int64_t pright = pM_end - 1 ;
                            bool found ;
                            GB_BINARY_SEARCH (iA, Mi, pM, pright, found) ;
                            if (found)
                            { 
                                // found it
                                cast_M (&mij, Mx +(pM*msize), 0) ;
                            }
                            else
                            { 
                                // M(iA,j) not present, implicitly false
                                mij = false ;
                            }
                            if (Mask_comp)
                            { 
                                // negate the mask M if Mask_comp is true
                                mij = !mij ;
                            }

                            //--------------------------------------------------
                            // find C(iC,jC), but only if M(iA,j) allows it
                            //--------------------------------------------------

                            if (mij)
                            {

                                //----------------------------------------------
                                // C(iC,jC) += A(iA,j)
                                //----------------------------------------------

                                // direct lookup of C(iC,jC)
                                GB_CDENSE_I_LOOKUP ;

                                // ----[C A 1] or [X A 1]-------------------
                                // [C A 1]: action: ( =C+A ): apply accum
                                // [X A 1]: action: ( undelete ) zombie live
                                GB_withaccum_C_A_1_matrix ;
                            }
                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // C(:,jC) is sparse; use binary search for C
                        //------------------------------------------------------

                        for ( ; pA < pA_end ; pA++)
                        {

                            //--------------------------------------------------
                            // consider the entry A(iA,j)
                            //--------------------------------------------------

                            int64_t iA = Ai [pA] ;

                            //--------------------------------------------------
                            // find M(iA,j)
                            //--------------------------------------------------

                            bool mij = true ;
                            int64_t pM     = pM_start ;
                            int64_t pright = pM_end - 1 ;
                            bool found ;
                            GB_BINARY_SEARCH (iA, Mi, pM, pright, found) ;
                            if (found)
                            { 
                                // found it
                                cast_M (&mij, Mx +(pM*msize), 0) ;
                            }
                            else
                            { 
                                // M(iA,j) not present, implicitly false
                                mij = false ;
                            }
                            if (Mask_comp)
                            { 
                                // negate the mask M if Mask_comp is true
                                mij = !mij ;
                            }

                            //--------------------------------------------------
                            // find C(iC,jC), but only if M(iA,j) allows it
                            //--------------------------------------------------

                            if (mij)
                            {

                                //----------------------------------------------
                                // C(iC,jC) += A(iA,j)
                                //----------------------------------------------

                                // iC = I [iA] ; or I is a colon expression
                                int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                                // binary search for C(iC,jC) in C(:,jC)
                                GB_iC_BINARY_SEARCH ;

                                if (found)
                                { 
                                    // ----[C A 1] or [X A 1]-------------------
                                    // [C A 1]: action: ( =C+A ): apply accum
                                    // [X A 1]: action: ( undelete ) zombie live
                                    GB_withaccum_C_A_1_matrix ;
                                }
                                else
                                { 
                                    // ----[. A 1]------------------------------
                                    // action: ( insert )
                                    GB_D_A_1_matrix ;
                                }
                            }
                        }
                    }
                }
            }
        }

    }
    else if (M == NULL)
    {

        //----------------------------------------------------------------------
        // METHOD 3: assignment using S_Extraction method, no mask M
        //----------------------------------------------------------------------

        // 6 cases to consider

        // [ C A 1 ]    C_A_1: C present, A present
        // [ X A 1 ]    C_A_1: C zombie, A present
        // [ . A 1 ]    D_A_1: C not present, A present
        // [ C . 1 ]    C_D_1: C present, A not present
        // [ X . 1 ]    C_D_1: C zombie, A not present
        // [ . . 1 ]           not encountered, nothing to do

        if (scalar_expansion)
        {

            //------------------------------------------------------------------
            // METHOD 3a: C(I,J) = scalar or C(I,J) += scalar, using S
            //------------------------------------------------------------------

            if (accum == NULL)
            {

                //--------------------------------------------------------------
                // METHOD 3a (no accum): C(I,J) = scalar, using S
                //--------------------------------------------------------------

                GB_for_each_vector2s (S /*, scalar */)
                {

                    //----------------------------------------------------------
                    // do a 2-way merge of S(:,j) and the scalar
                    //----------------------------------------------------------

                    int64_t GBI1_initj (Iter, j, pS, pS_end) ;

                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // for each iA in I [...]:
                    for (int64_t iA = 0 ; iA < nI ; iA++)
                    {
                        bool found = (pS < pS_end) && (Si [pS] == iA) ;

                        if (!found)
                        { 
                            // ----[. A 1]--------------------------------------
                            // S (i,j) is not present, the scalar is present
                            // [. A 1]: action: ( insert )
                            // iC = I [iA] ; or I is a colon expression
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_D_A_1_scalar ;
                        }
                        else
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // both S (i,j) and A (i,j) present
                            // [C A 1]: action: ( =A ): scalar to C, no accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_C_S_LOOKUP ;
                            GB_noaccum_C_A_1_scalar ;
                            GB_NEXT (S) ;
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // METHOD 3a (with accum): C(I,J) += scalar, using S
                //--------------------------------------------------------------

                GB_for_each_vector2s (S /*, scalar */)
                {

                    //----------------------------------------------------------
                    // do a 2-way merge of S(:,j) and the scalar
                    //----------------------------------------------------------

                    int64_t GBI1_initj (Iter, j, pS, pS_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // for each iA in I [...]:
                    for (int64_t iA = 0 ; iA < nI ; iA++)
                    {
                        bool found = (pS < pS_end) && (Si [pS] == iA) ;

                        if (!found)
                        { 
                            // ----[. A 1]--------------------------------------
                            // S (i,j) is not present, the scalar is present
                            // [. A 1]: action: ( insert )
                            // iC = I [iA] ; or I is a colon expression
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_D_A_1_scalar ;
                        }
                        else
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // both S (i,j) and A (i,j) present
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_C_S_LOOKUP ;
                            GB_withaccum_C_A_1_scalar ;
                            GB_NEXT (S) ;
                        }
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // METHOD 3b: C(I,J) = A or C(I,J) += A, using S
            //------------------------------------------------------------------

            if (accum == NULL)
            {

                //--------------------------------------------------------------
                // METHOD 3b (no accum): C(I,J) = A, using S
                //--------------------------------------------------------------

                GB_for_each_vector2 (S, A)
                {

                    //----------------------------------------------------------
                    // do a 2-way merge of S(:,j) and A(:,j)
                    //----------------------------------------------------------

                    int64_t GBI2_initj (Iter, j, pS, pS_end, pA, pA_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // while both list S (:,j) and A (:,j) have entries
                    while (pS < pS_end && pA < pA_end)
                    {
                        int64_t iS = Si [pS] ;
                        int64_t iA = Ai [pA] ;

                        if (iS < iA)
                        { 
                            // ----[C . 1] or [X . 1]---------------------------
                            // S (i,j) is present but A (i,j) is not
                            // [C . 1]: action: ( delete ): becomes a zombie
                            // [X . 1]: action: ( X ): still a zombie
                            GB_C_S_LOOKUP ;
                            GB_noaccum_C_D_1_matrix ;
                            GB_NEXT (S) ;

                        }
                        else if (iA < iS)
                        { 
                            // ----[. A 1]--------------------------------------
                            // S (i,j) is not present, A (i,j) is present
                            // [. A 1]: action: ( insert )
                            // iC = I [iA] ; or I is a colon expression
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_D_A_1_matrix ;
                            GB_NEXT (A) ;
                        }
                        else
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // both S (i,j) and A (i,j) present
                            // [C A 1]: action: ( =A ): copy A into C, no accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_C_S_LOOKUP ;
                            GB_noaccum_C_A_1_matrix ;
                            GB_NEXT (S) ;
                            GB_NEXT (A) ;
                        }
                    }

                    // while list S (:,j) has entries.  List A (:,j) exhausted
                    while (pS < pS_end)
                    { 
                        // ----[C . 1] or [X . 1]-------------------------------
                        // S (i,j) is present but A (i,j) is not
                        // [C . 1]: action: ( delete ): becomes a zombie
                        // [X . 1]: action: ( X ): still a zombie
                        GB_C_S_LOOKUP ;
                        GB_noaccum_C_D_1_matrix ;
                        GB_NEXT (S) ;
                    }

                    // while list A (:,j) has entries.  List S (:,j) exhausted
                    while (pA < pA_end)
                    { 
                        // ----[. A 1]------------------------------------------
                        // S (i,j) is not present, A (i,j) is present
                        // [. A 1]: action: ( insert )
                        int64_t iA = Ai [pA] ;
                        // iC = I [iA] ; or I is a colon expression
                        int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                        GB_D_A_1_matrix ;
                        GB_NEXT (A) ;
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // METHOD 3b (with accum): C(I,J) += A, using S
                //--------------------------------------------------------------

                GB_for_each_vector2 (S, A)
                {

                    //----------------------------------------------------------
                    // do a 2-way merge of S(:,j) and A(:,j)
                    //----------------------------------------------------------

                    int64_t GBI2_initj (Iter, j, pS, pS_end, pA, pA_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // while both list S (:,j) and A (:,j) have entries
                    while (pS < pS_end && pA < pA_end)
                    {
                        int64_t iS = Si [pS] ;
                        int64_t iA = Ai [pA] ;

                        if (iS < iA)
                        { 
                            // ----[C . 1] or [X . 1]---------------------------
                            // S (i,j) is present but A (i,j) is not
                            // [C . 1]: action: ( C ): no change, with accum
                            // [X . 1]: action: ( X ): still a zombie
                            GB_NEXT (S) ;

                        }
                        else if (iA < iS)
                        { 
                            // ----[. A 1]--------------------------------------
                            // S (i,j) is not present, A (i,j) is present
                            // [. A 1]: action: ( insert )
                            // iC = I [iA] ; or I is a colon expression
                            int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                            GB_D_A_1_matrix ;
                            GB_NEXT (A) ;
                        }
                        else
                        { 
                            // ----[C A 1] or [X A 1]---------------------------
                            // both S (i,j) and A (i,j) present
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            GB_C_S_LOOKUP ;
                            GB_withaccum_C_A_1_matrix ;
                            GB_NEXT (S) ;
                            GB_NEXT (A) ;
                        }
                    }

                    // while list S (:,j) has entries.  List A (:,j) exhausted.
                    // The rest of S(:,j) is skipped since it does not change C.
                    // while (pS < pS_end)
                    // {
                        // ----[C . 1] or [X . 1]-------------------------------
                        // S (i,j) is present but A (i,j) is not
                        // [C . 1]: action: ( C ): no change, with accum
                        // [X . 1]: action: ( X ): still a zombie
                        // GB_NEXT (S) ;
                    // }

                    // while list A (:,j) has entries.  List S (:,j) exhausted
                    while (pA < pA_end)
                    { 
                        // ----[. A 1]------------------------------------------
                        // S (i,j) is not present, A (i,j) is present
                        // [. A 1]: action: ( insert )
                        int64_t iA = Ai [pA] ;
                        // iC = I [iA] ; or I is a colon expression
                        int64_t iC = GB_ijlist (I, iA, Ikind, Icolon) ;
                        GB_D_A_1_matrix ;
                        GB_NEXT (A) ;
                    }
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // METHOD 4: assignment using S_Extraction method, with M
        //----------------------------------------------------------------------

        if (scalar_expansion)
        {

            //------------------------------------------------------------------
            // METHOD 4a: C(I,J)<M> = scalar or += scalar, using S
            //------------------------------------------------------------------

            // 6 cases to consider
            // [ C A 1 ]    C_A_1: C present, A present, M present and = 1
            // [ X A 1 ]    C_A_1: C zombie, A present, M present and = 1
            // [ . A 1 ]    D_A_1: C not present, A present, M present and = 1
            // [ C A 0 ]    C_A_0: C present, A present, M not present or zero
            // [ X A 0 ]    C_A_0: C zombie, A present, M not present or zero
            // [ . A 0 ]           C not present, A present, M 0; nothing to do

            if (accum == NULL)
            {

                //--------------------------------------------------------------
                // METHOD 4a (no accum): C(I,J)<M> = scalar, using S
                //--------------------------------------------------------------

                GB_for_each_vector3s (S, M /* scalar */)
                {

                    //----------------------------------------------------------
                    // do a 3-way merge of S(:,j), M(:,j), and the scalar
                    //----------------------------------------------------------

                    int64_t GBI2_initj (Iter, j, pS, pS_end, pM, pM_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // The merge is similar to GB_mask, except that it does not
                    // produce another output matrix.  Instead, the results are
                    // written directly into C, either modifying the entries
                    // there or adding pending tuples.

                    // There are three sorted lists to merge:
                    // S(:,j) in [pS .. pS_end-1]
                    // M(:,j) in [pM .. pM_end-1]
                    // A(:,j) an expanded scalar, an implicit dense vector.
                    // The head of each list is at index pS, pA, and pM, and an
                    // entry is 'discarded' by incrementing its respective
                    // index via GB_NEXT(.).  Once a list is consumed, a query
                    // for its next row index will result in a dummy value nI
                    // larger than all valid row indices.

                    //----------------------------------------------------------
                    // while either list S(:,j) or A(:,j) have entries
                    //----------------------------------------------------------

                    // for each iA in I [...]:
                    for (int64_t iA = 0 ; iA < nI ; iA++)
                    {

                        //------------------------------------------------------
                        // Get the indices at the top of each list.
                        //------------------------------------------------------

                        // If a list has been consumed, use a dummy index nI
                        // that is larger than all valid indices.
                        int64_t iS = (pS < pS_end) ? Si [pS] : nI ;
                        int64_t iM = (pM < pM_end) ? Mi [pM] : nI ;

                        //------------------------------------------------------
                        // find the smallest index of [iS iA iM]
                        //------------------------------------------------------

                        int64_t i = iA ;

                        //------------------------------------------------------
                        // get M(i,j)
                        //------------------------------------------------------

                        // If an explicit value of M(i,j) must be tested, it
                        // must first be typecasted to bool.  If (i == iM),
                        // then M(i,j) is present and is typecasted into mij
                        // and then discarded.  Otherwise, if M(i,j) is not
                        // present, mij is set to false.

                        bool mij ;
                        if (i == iM)
                        { 
                            // mij = (bool) M [pM]
                            cast_M (&mij, Mx +(pM*msize), 0) ;
                            GB_NEXT (M) ;
                        }
                        else
                        { 
                            // mij not present, implicitly false
                            ASSERT (i < iM) ;
                            mij = false ;
                        }

                        // explicitly complement the mask entry mij
                        if (Mask_comp)
                        { 
                            mij = !mij ;
                        }

                        //------------------------------------------------------
                        // handle all 6 cases
                        //------------------------------------------------------

                        if (i == iS)
                        {
                            ASSERT (i == iA) ;
                            {
                                // both S (i,j) and A (i,j) present
                                GB_C_S_LOOKUP ;
                                if (mij)
                                { 
                                    // ----[C A 1] or [X A 1]-------------------
                                    // [C A 1]: action: ( =A ): copy A, no accum
                                    // [X A 1]: action: ( undelete ):zombie live
                                    GB_noaccum_C_A_1_scalar ;
                                }
                                else
                                { 
                                    // ----[C A 0] or [X A 0]-------------------
                                    // [X A 0]: action: ( X ): still a zombie
                                    // [C A 0]: C_repl: action: ( delete ):zombi
                                    // [C A 0]: no C_repl: action: ( C ): none
                                    GB_C_A_0 ;
                                }
                                GB_NEXT (S) ;
                            }
                        }
                        else
                        {
                            ASSERT (i == iA) ;
                            {
                                // S (i,j) is not present, A (i,j) is present
                                if (mij)
                                { 
                                    // ----[. A 1]------------------------------
                                    // [. A 1]: action: ( insert )
                                    // iC = I [iA] ; or I is a colon expression
                                    int64_t iC = GB_ijlist (I, iA, Ikind,
                                                                   Icolon) ;
                                    GB_D_A_1_scalar ;
                                }
                                else
                                { 
                                    // ----[. A 0]------------------------------
                                    // action: ( . ): no action
                                }
                            }
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // METHOD 4a (with accum): C(I,J)<M> += scalar, using S
                //--------------------------------------------------------------

                GB_for_each_vector3s (S, M /* scalar */)
                {

                    //----------------------------------------------------------
                    // do a 3-way merge of S(:,j), M(:,j), and the scalar
                    //----------------------------------------------------------

                    int64_t GBI2_initj (Iter, j, pS, pS_end, pM, pM_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // The merge is similar to GB_mask, except that it does not
                    // produce another output matrix.  Instead, the results are
                    // written directly into C, either modifying the entries
                    // there or adding pending tuples.

                    // There are three sorted lists to merge:
                    // S(:,j) in [pS .. pS_end-1]
                    // M(:,j) in [pM .. pM_end-1]
                    // A(:,j) an expanded scalar, an implicit dense vector.
                    // The head of each list is at index pS, pA, and pM, and an
                    // entry is 'discarded' by incrementing its respective
                    // index via GB_NEXT(.).  Once a list is consumed, a query
                    // for its next row index will result in a dummy value nI
                    // larger than all valid row indices.

                    //----------------------------------------------------------
                    // while either list S(:,j) or A(:,j) have entries
                    //----------------------------------------------------------

                    // for each iA in I [...]:
                    for (int64_t iA = 0 ; iA < nI ; iA++)
                    {

                        //------------------------------------------------------
                        // Get the indices at the top of each list.
                        //------------------------------------------------------

                        // If a list has been consumed, use a dummy index nI
                        // that is larger than all valid indices.
                        int64_t iS = (pS < pS_end) ? Si [pS] : nI ;
                        int64_t iM = (pM < pM_end) ? Mi [pM] : nI ;

                        //------------------------------------------------------
                        // find the smallest index of [iS iA iM]
                        //------------------------------------------------------

                        int64_t i = iA ;

                        //------------------------------------------------------
                        // get M(i,j)
                        //------------------------------------------------------

                        // If an explicit value of M(i,j) must be tested, it
                        // must first be typecasted to bool.  If (i == iM),
                        // then M(i,j) is present and is typecasted into mij
                        // and then discarded.  Otherwise, if M(i,j) is not
                        // present, mij is set to false.

                        bool mij ;
                        if (i == iM)
                        { 
                            // mij = (bool) M [pM]
                            cast_M (&mij, Mx +(pM*msize), 0) ;
                            GB_NEXT (M) ;
                        }
                        else
                        { 
                            // mij not present, implicitly false
                            ASSERT (i < iM) ;
                            mij = false ;
                        }

                        // explicitly complement the mask entry mij
                        if (Mask_comp)
                        { 
                            mij = !mij ;
                        }

                        //------------------------------------------------------
                        // handle all 6 cases
                        //------------------------------------------------------

                        if (i == iS)
                        {
                            ASSERT (i == iA) ;
                            {
                                // both S (i,j) and A (i,j) present
                                GB_C_S_LOOKUP ;
                                if (mij)
                                { 
                                    // ----[C A 1] or [X A 1]-------------------
                                    // [C A 1]: action: ( =C+A ): apply accum
                                    // [X A 1]: action: ( undelete ): zombie liv
                                    GB_withaccum_C_A_1_scalar ;
                                }
                                else
                                { 
                                    // ----[C A 0] or [X A 0]-------------------
                                    // [X A 0]: action: ( X ): still a zombie
                                    // [C A 0]: C_repl: action: ( delete ):zombi
                                    // [C A 0]: no C_repl: action: ( C ): none
                                    GB_C_A_0 ;
                                }
                                GB_NEXT (S) ;
                            }
                        }
                        else
                        {
                            ASSERT (i == iA) ;
                            {
                                // S (i,j) is not present, A (i,j) is present
                                if (mij)
                                { 
                                    // ----[. A 1]------------------------------
                                    // [. A 1]: action: ( insert )
                                    // iC = I [iA] ; or I is a colon expression
                                    int64_t iC = GB_ijlist (I, iA, Ikind,
                                                                   Icolon) ;
                                    GB_D_A_1_scalar ;
                                }
                                else
                                { 
                                    // ----[. A 0]------------------------------
                                    // action: ( . ): no action
                                }
                            }
                        }
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // METHOD 4b: C(I,J)<M> = A or += A, using S
            //------------------------------------------------------------------

            // 12 cases to consider
            // [ C A 1 ]    C_A_1: C present, A present, M present and = 1
            // [ X A 1 ]    C_A_1: C zombie, A present, M present and = 1
            // [ . A 1 ]    D_A_1: C not present, A present, M present and = 1
            // [ C . 1 ]    C_D_1: C present, A not present, M present and = 1
            // [ X . 1 ]    C_D_1: C zombie, A not present, M present and = 1
            // [ . . 1 ]           only M=1 present, nothing to do
            // [ C A 0 ]    C_A_0: C present, A present, M not present or zero
            // [ X A 0 ]    C_A_0: C zombie, A present, M not present or zero
            // [ . A 0 ]           C not present, A present, M 0; nothing to do
            // [ C . 0 ]    C_D_0: C present, A not present, M 0
            // [ X . 0 ]    C_D_0: C zombie, A not present, M 0
            // [ . . 0 ]           M not present or zero, nothing to do

            if (accum == NULL)
            {

                //--------------------------------------------------------------
                // METHOD 4b (no accum): C(I,J)<M> = A, using S
                //--------------------------------------------------------------

                GB_for_each_vector3 (S, M, A)
                {

                    //----------------------------------------------------------
                    // do a 3-way merge of S(:,j), M(:,j), and A(:,j)
                    //----------------------------------------------------------

                    int64_t GBI3_initj (Iter,j,pS,pS_end,pM,pM_end,pA,pA_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // The merge is similar to GB_mask, except that it does not
                    // produce another output matrix.  Instead, the results are
                    // written directly into C, either modifying the entries
                    // there or adding pending tuples.

                    // There are three sorted lists to merge:
                    // S(:,j) in [pS .. pS_end-1]
                    // M(:,j) in [pM .. pM_end-1]
                    // A(:,j) in [pA .. pA_end-1]
                    // The head of each list is at index pS, pA, and pM, and an
                    // entry is 'discarded' by incrementing its respective
                    // index via GB_NEXT(.).  Once a list is consumed, a query
                    // for its next row index will result in a dummy value nI
                    // larger than all valid row indices.

                    //----------------------------------------------------------
                    // while either list S(:,j) or A(:,j) have entries
                    //----------------------------------------------------------

                    while (pS < pS_end || pA < pA_end)
                    {

                        //------------------------------------------------------
                        // Get the indices at the top of each list.
                        //------------------------------------------------------

                        // If a list has been consumed, use a dummy index nI
                        // that is larger than all valid indices.
                        int64_t iS = (pS < pS_end) ? Si [pS] : nI ;
                        int64_t iA = (pA < pA_end) ? Ai [pA] : nI ;
                        int64_t iM = (pM < pM_end) ? Mi [pM] : nI ;

                        //------------------------------------------------------
                        // find the smallest index of [iS iA iM]
                        //------------------------------------------------------

                        // i = min ([iS iA iM])
                        int64_t i = GB_IMIN (iS, GB_IMIN (iA, iM)) ;
                        ASSERT (i < nI) ;

                        //------------------------------------------------------
                        // get M(i,j)
                        //------------------------------------------------------

                        // If an explicit value of M(i,j) must be tested, it
                        // must first be typecasted to bool.  If (i == iM),
                        // then M(i,j) is present and is typecasted into mij
                        // and then discarded.  Otherwise, if M(i,j) is not
                        // present, mij is set to false.

                        bool mij ;
                        if (i == iM)
                        { 
                            // mij = (bool) M [pM]
                            cast_M (&mij, Mx +(pM*msize), 0) ;
                            GB_NEXT (M) ;
                        }
                        else
                        { 
                            // mij not present, implicitly false
                            ASSERT (i < iM) ;
                            mij = false ;
                        }

                        // explicitly complement the mask entry mij
                        if (Mask_comp)
                        { 
                            mij = !mij ;
                        }

                        //------------------------------------------------------
                        // handle all 12 cases
                        //------------------------------------------------------

                        if (i == iS)
                        {
                            if (i == iA)
                            {
                                // both S (i,j) and A (i,j) present
                                GB_C_S_LOOKUP ;
                                if (mij)
                                { 
                                    // ----[C A 1] or [X A 1]-------------------
                                    // [C A 1]: action: ( =A ): A to C no accum
                                    // [X A 1]: action: ( undelete ): zombie liv
                                    GB_noaccum_C_A_1_matrix ;
                                }
                                else
                                { 
                                    // ----[C A 0] or [X A 0]-------------------
                                    // [X A 0]: action: ( X ): still a zombie
                                    // [C A 0]: C_repl: action: ( delete ): zomb
                                    // [C A 0]: no C_repl: action: ( C ): none
                                    GB_C_A_0 ;
                                }
                                GB_NEXT (S) ;
                                GB_NEXT (A) ;
                            }
                            else
                            {
                                // S (i,j) is present but A (i,j) is not
                                GB_C_S_LOOKUP ;
                                if (mij)
                                { 
                                    // ----[C . 1] or [X . 1]-------------------
                                    // [C . 1]: action: ( delete ): zombie
                                    // [X . 1]: action: ( X ): still zombie
                                    GB_noaccum_C_D_1_matrix ;
                                }
                                else
                                { 
                                    // ----[C . 0] or [X . 0]-------------------
                                    // [X . 0]: action: ( X ): still a zombie
                                    // [C . 0]: if C_repl: action: ( delete ):
                                    // [C . 0]: no C_repl: action: ( C ): none
                                    GB_C_D_0 ;
                                }
                                GB_NEXT (S) ;
                            }
                        }
                        else
                        {
                            if (i == iA)
                            {
                                // S (i,j) is not present, A (i,j) is present
                                if (mij)
                                { 
                                    // ----[. A 1]------------------------------
                                    // [. A 1]: action: ( insert )
                                    // iC = I [iA] ; or I is a colon expression
                                    int64_t iC = GB_ijlist (I, iA, Ikind,
                                                                   Icolon) ;
                                    GB_D_A_1_matrix ;
                                }
                                else
                                { 
                                    // ----[. A 0]------------------------------
                                    // action: ( . ): no action
                                }
                                GB_NEXT (A) ;
                            }
                            else
                            { 
                                // neither S (i,j) nor A (i,j) present
                                // ----[. . 1]----------------------------------
                                // ----[. . 0]----------------------------------
                                // action: ( . ): no action
                                ASSERT (i == iM) ;
                            }
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // METHOD 4b (with accum): C(I,J)<M> += A, using S
                //--------------------------------------------------------------

                GB_for_each_vector3 (S, M, A)
                {

                    //----------------------------------------------------------
                    // do a 3-way merge of S(:,j), M(:,j), and A(:,j)
                    //----------------------------------------------------------

                    int64_t GBI3_initj (Iter,j, pS,pS_end,pM,pM_end,pA,pA_end) ;
                    // jC = J [j] ; or J is a colon expression
                    int64_t jC = GB_ijlist (J, j, Jkind, Jcolon) ;

                    // The merge is similar to GB_mask, except that it does not
                    // produce another output matrix.  Instead, the results are
                    // written directly into C, either modifying the entries
                    // there or adding pending tuples.

                    // There are three sorted lists to merge:
                    // S(:,j) in [pS .. pS_end-1]
                    // M(:,j) in [pM .. pM_end-1]
                    // A(:,j) in [pA .. pA_end-1]
                    // The head of each list is at index pS, pA, and pM, and an
                    // entry is 'discarded' by incrementing its respective
                    // index via GB_NEXT(.).  Once a list is consumed, a query
                    // for its next row index will result in a dummy value nI
                    // larger than all valid row indices.

                    //----------------------------------------------------------
                    // while either list S(:,j) or A(:,j) have entries
                    //----------------------------------------------------------

                    while (pS < pS_end || pA < pA_end)
                    {

                        //------------------------------------------------------
                        // Get the indices at the top of each list.
                        //------------------------------------------------------

                        // If a list has been consumed, use a dummy index nI
                        // that is larger than all valid indices.
                        int64_t iS = (pS < pS_end) ? Si [pS] : nI ;
                        int64_t iA = (pA < pA_end) ? Ai [pA] : nI ;
                        int64_t iM = (pM < pM_end) ? Mi [pM] : nI ;

                        //------------------------------------------------------
                        // find the smallest index of [iS iA iM]
                        //------------------------------------------------------

                        // i = min ([iS iA iM])
                        int64_t i = GB_IMIN (iS, GB_IMIN (iA, iM)) ;
                        ASSERT (i < nI) ;

                        //------------------------------------------------------
                        // get M(i,j)
                        //------------------------------------------------------

                        // If an explicit value of M(i,j) must be tested, it
                        // must first be typecasted to bool.  If (i == iM),
                        // then M(i,j) is present and is typecasted into mij
                        // and then discarded.  Otherwise, if M(i,j) is not
                        // present, mij is set to false.

                        bool mij ;
                        if (i == iM)
                        { 
                            // mij = (bool) M [pM]
                            cast_M (&mij, Mx +(pM*msize), 0) ;
                            GB_NEXT (M) ;
                        }
                        else
                        { 
                            // mij not present, implicitly false
                            ASSERT (i < iM) ;
                            mij = false ;
                        }

                        // explicitly complement the mask entry mij
                        if (Mask_comp)
                        { 
                            mij = !mij ;
                        }

                        //------------------------------------------------------
                        // handle all 12 cases
                        //------------------------------------------------------

                        if (i == iS)
                        {
                            if (i == iA)
                            {
                                // both S (i,j) and A (i,j) present
                                GB_C_S_LOOKUP ;
                                if (mij)
                                { 
                                    // ----[C A 1] or [X A 1]-------------------
                                    // [C A 1]: action: ( =A ): A to C no accum
                                    // [C A 1]: action: ( =C+A ): apply accum
                                    // [X A 1]: action: ( undelete ): zombie liv
                                    GB_withaccum_C_A_1_matrix ;
                                }
                                else
                                { 
                                    // ----[C A 0] or [X A 0]-------------------
                                    // [X A 0]: action: ( X ): still a zombie
                                    // [C A 0]: C_repl: action: ( delete ): zomb
                                    // [C A 0]: no C_repl: action: ( C ): none
                                    GB_C_A_0 ;
                                }
                                GB_NEXT (S) ;
                                GB_NEXT (A) ;
                            }
                            else
                            {
                                // S (i,j) is present but A (i,j) is not
                                GB_C_S_LOOKUP ;
                                if (mij)
                                { 
                                    // ----[C . 1] or [X . 1]-------------------
                                    // [C . 1]: action: ( C ): no change w accum
                                    // [X . 1]: action: ( X ): still a zombie
                                    // withaccum_C_D_1_matrix ;
                                }
                                else
                                { 
                                    // ----[C . 0] or [X . 0]-------------------
                                    // [X . 0]: action: ( X ): still a zombie
                                    // [C . 0]: if C_repl: action: ( delete ):
                                    // [C . 0]: no C_repl: action: ( C ): none
                                    GB_C_D_0 ;
                                }
                                GB_NEXT (S) ;
                            }
                        }
                        else
                        {
                            if (i == iA)
                            {
                                // S (i,j) is not present, A (i,j) is present
                                if (mij)
                                { 
                                    // ----[. A 1]------------------------------
                                    // [. A 1]: action: ( insert )
                                    // iC = I [iA] ; or I is a colon expression
                                    int64_t iC = GB_ijlist (I, iA, Ikind,
                                                                   Icolon) ;
                                    GB_D_A_1_matrix ;
                                }
                                else
                                { 
                                    // ----[. A 0]------------------------------
                                    // action: ( . ): no action
                                }
                                GB_NEXT (A) ;
                            }
                            else
                            { 
                                // neither S (i,j) nor A (i,j) present
                                // ----[. . 1]----------------------------------
                                // ----[. . 0]----------------------------------
                                // action: ( . ): no action
                                ASSERT (i == iM) ;
                            }
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // insert C in the queue if it has work to do and isn't already queued
    //--------------------------------------------------------------------------

    if (C->nzombies == 0 && C->n_pending == 0)
    { 
        // C may be in the queue from a prior assignment, but this assignemt
        // can bring zombies back to life, and the zombie count can go to zero.
        // In that case, C must be removed from the queue.  The removal does
        // nothing if C is already not in the queue.
        GB_CRITICAL (GB_queue_remove (C)) ;
    }
    else
    { 
        // If C has any zombies or pending tuples, it must be in the queue.
        // The queue insert does nothing if C is already in the queue.
        GB_CRITICAL (GB_queue_insert (C)) ;
    }
    ASSERT_OK (GB_check (C, "C(I,J) result", GB0)) ;

    //--------------------------------------------------------------------------
    // free workspace, check blocking mode, and return
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&S) ;
    return (GB_block (C, Context)) ;
}

