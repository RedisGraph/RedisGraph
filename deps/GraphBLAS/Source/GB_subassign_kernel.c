//------------------------------------------------------------------------------
// GB_subassign_kernel: C (I,J)<Mask> = accum (C (I,J), A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Submatrix assignment: C(I,J)<Mask> = A, or accum (C (I,J), A)

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

// This function handles the accumulator, and the Mask, and the C_replace
// option itself, without relying on GB_accum_mask or GB_mask.  The Mask has
// the same size as C(I,J) and A.  Mask(0,0) governs how A(0,0) is assigned
// into C(I[0],J[0]).  This is how GxB_subassign operates.  For GrB_assign, the
// Mask in this function is the SubMask, constructed via SubMask=Mask (I,J).

#include "GB.h"

GrB_Info GB_subassign_kernel        // C(I,J)<Mask> = A or accum (C (I,J), A)
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // C matrix descriptor
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const bool Mask_comp,           // Mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),A)
    const GrB_Matrix A,             // input matrix (NULL for scalar expansion)
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Index *J,             // column indices
    const GrB_Index nj,             // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code  // type code of scalar to expand
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // this function operates on C in place and this cannot be aliased with
    // A or Mask
    ASSERT (C != Mask && C != A) ;

    //--------------------------------------------------------------------------
    // check empty Mask conditions
    //--------------------------------------------------------------------------

    int64_t nzMask = 0 ;
    if (Mask == NULL)
    {
        // the Mask is empty
        if (Mask_comp)
        {
            // an empty Mask is complemented
            if (!C_replace)
            {
                // No work to do.  This the same as the RETURN_IF_QUICK_MASK
                // case in other GraphBLAS functions, except here only the
                // sub-case of C_replace=false is handled.  The C_replace=true
                // sub-case needs to delete all entries in C(I,J), which is
                // handled below, not here.
                return (REPORT_SUCCESS) ;
            }
        }
        else
        {
            // The Mask is empty and not complemented.  In this case, C_replace
            // is effectively false.  Disable it, since it can force pending
            // tuples to be assembled.  In the comments below "C_replace
            // effectively false" means that either C_replace is false on
            // input, or the Mask is empty and not complemented and thus
            // C_replace is set to false here.
            C_replace = false ;
        }
    }
    else
    {
        nzMask = NNZ (Mask) ;
    }

    // C_replace now has its effective value: it is true only if true on input
    // and if the Mask is present, or empty and complemented.  C_replace is
    // false if it is false on input, or if the Mask is empty and not
    // complemented.

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    // the matrix C may have pending tuples and/or zombies
    ASSERT_OK (GB_check (C, "C for assign", 0)) ;
    ASSERT (PENDING_OK (C)) ; ASSERT (ZOMBIES_OK (C)) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    // If the descriptor says that A must be transposed, it has already been
    // transposed in the caller.  Thus C(I,J), A, and Mask (if present) all
    // have the same size: length(I)-by-length(J)

    GrB_Index mn ;
    bool mn_ok = GB_Index_multiply (&mn, ni, nj) ;
    bool is_dense ;
    int64_t anz ;

    if (scalar_expansion)
    {
        // A is not present
        ASSERT (A == NULL) ;
        ASSERT (scalar != NULL) ;
        anz = mn ;
        is_dense = true ;
    }
    else
    {
        // A is an ni-by-nj matrix, with no pending computations
        ASSERT_OK (GB_check (A, "A for assign", 0)) ;
        ASSERT (A->nrows == ni && A->ncols == nj) ;
        ASSERT (!PENDING (A)) ;   ASSERT (!ZOMBIES (A)) ;
        ASSERT (scalar == NULL) ;
        anz = NNZ (A) ;
        is_dense = (mn_ok && anz == (int64_t) mn) ;
    }

    if (Mask != NULL)
    {
        // Mask can have no pending tuples nor zombies
        ASSERT (!PENDING (Mask)) ;  ASSERT (!ZOMBIES (Mask)) ;
        ASSERT (ni == Mask->nrows && nj == Mask->ncols) ;
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

    // If accum is NULL, the operation is C(I,J) = A, or C(I,J)<Mask> = A.
    // If A has any implicit zeros at all, or if the Mask is present, then
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
    // z=f(x,y) whose ztype and ytype matches the type of C.

    bool apply_pending_updates = false ;

    if (C->npending == 0)
    {

        //----------------------------------------------------------------------
        // no pending tuples currently exist.
        //----------------------------------------------------------------------

        // If any new pending tuples are added, their pending operator is
        // accum, or the implicit SECOND_Ctype operator if accum is NULL.
        // Prior zombies have no effect.

        apply_pending_updates = false ;

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
            apply_pending_updates = true ;
        }
        else if (accum == NULL)
        {
            // This GxB_subassign can potentially use action: ( delete ), and
            // thus prior pending tuples must be assembled first.  However, if
            // A is completely dense and if there is no Mask, then C(I,J)=A
            // cannot delete any entries from C.

            if (Mask == NULL && is_dense)
            {
                // A is a dense matrix, so entries cannot be deleted
                apply_pending_updates = false ;
            }
            else
            {
                // A has at least one implicit entry, or the Mask is present.
                // In this case, action: ( delete ) might occur
                apply_pending_updates = true ;
            }
        }

        //----------------------------------------------------------------------
        // check if pending operator is compatible
        //----------------------------------------------------------------------

        if (!apply_pending_updates)
        {

            // Either accum is NULL and A is completely dense, or accum is
            // non-NULL.  In either case, this call to GxB_subassign will not
            // use the action: ( delete ), but it may add new pending tuples
            // via the action: ( insert ).  Check if the accum operator is the
            // same as the prior pending operator.

            if (accum == C->operator_pending)
            {
                // the operators are the same
                apply_pending_updates = false ;
            }
            else if (GB_op_is_second (accum, C->type) &&
                     GB_op_is_second (C->operator_pending, C->type))
            {
                // both operators are SECOND_Ctype, implicit or explicit
                apply_pending_updates = false ;
            }
            else
            {
                // the operators do not match
                apply_pending_updates = true ;
            }
        }
    }

    if (apply_pending_updates)
    {
        // Prior computations are not compatible with this assignment, so all
        // prior work must be finished.  This potentially costly.

        // delete any lingering zombies and assemble any pending tuples
        APPLY_PENDING_UPDATES (C) ;
    }

    ASSERT_OK_OR_NULL (GB_check (accum, "accum for assign", 0)) ;

    //--------------------------------------------------------------------------
    // keep track of the current accum operator
    //--------------------------------------------------------------------------

    // If accum is NULL and pending tuples are added, they will be assembled
    // sometime later (not here) using the implied SECOND_Ctype operator.  This
    // GxB_subassign operation corresponds to C(I,J)=A or C(I,J)<Mask>=A.
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

    #define C_LOOKUP                                                        \
        int64_t pC = Sx [pS] ;                                              \
        int64_t iC = Ci [pC] ;                                              \
        bool is_zombie = IS_ZOMBIE (iC) ;                                   \
        if (is_zombie) iC = FLIP (iC) ;

    //--------------------------------------------------------------------------
    // C(I,J)<Mask> = accum (C(I,J),A): consider all cases
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
        //      with: accum is SECOND, C not replaced, no Mask, Mask not
        //      complemented.  If GrB_setElement needs to insert its update as
        //      a pending tuple, then it will always be compatible with all
        //      pending tuples inserted here, by GxB_subassign.

        // (2) zombie entries.  These are entries that are still present in the
        // pattern but marked for deletion (via FLIP(i) for the row index).

        // For the current GxB_subassign, there are 16 cases to handle,
        // all combinations of the following options:

        //      accum is NULL, accum is not NULL
        //      C is not replaced, C is replaced
        //      no Mask, Mask is present
        //      Mask is not complemented, Mask is complemented

        // Complementing an empty Mask:  This does not require the matrix A
        // at all so it is handled as a special case.  It corresponds to
        // the RETURN_IF_QUICK_MASK option in other GraphBLAS operations.
        // Thus only 12 cases are considered in the tables below:

        //      These 4 cases are listed in Four Tables below:
        //      2 cases: accum is NULL, accum is not NULL
        //      2 cases: C is not replaced, C is replaced

        //      3 cases: no Mask, Mask is present and not complemented,
        //               and Mask is present and complemented.  If there is no
        //               Mask, then Mask(i,j)=1 for all (i,j).  These 3 cases
        //               are the columns of each of the Four Tables.

        // Each of these 12 cases can encounter up to 12 combinations of
        // entries in C, A, and Mask (6 if no Mask is present).  The left
        // column of the Four Tables below consider all 12 combinations for all
        // (i,j) in the cross product IxJ:

        //      C(I(i),J(j)) present, zombie, or not there: C, X, or '.'
        //      A(i,j) present or not, labeled 'A' or '.' below
        //      M(i,j) = 1 or 0 (but only if Mask is present)

        //      These 12 cases become the left columns as listed below.
        //      The zombie cases are handled a sub-case for "C present:
        //      regular entry or zombie".  The acronyms below use "D" for
        //      "dot", meaning the entry (C or A) is not present.

        //      [ C A 1 ]   C_A_1: both C and A present, Mask=1
        //      [ X A 1 ]   C_A_1: both C and A present, Mask=1, C is a zombie
        //      [ . A 1 ]   D_A_1: C not present, A present, Mask=1

        //      [ C . 1 ]   C_D_1: C present, A not present, Mask=1
        //      [ X . 1 ]   C_D_1: C present, A not present, Mask=1, C a zombie
        //      [ . . 1 ]          only Mask=1 present, but nothing to do

        //      [ C A 0 ]   C_A_0: both C and A present, Mask=0
        //      [ X A 0 ]   C_A_0: both C and A present, Mask=0, C is a zombie
        //      [ . A 0 ]          C not present, A present, Mask=0,
        //                              nothing to do

        //      [ C . 0 ]   C_D_0: C present, A not present, Mask=1
        //      [ X . 0 ]   C_D_0: C present, A not present, Mask=1, C a zombie
        //      [ . . 0 ]          only Mask=0 present, but nothing to do

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
        //               occur:  GxB_subassign with no Mask it cannot have
        //               Mask(i,j)=0, and GrB_setElement does not have the Mask
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
            //          NNZ(A)==1.  No mask, and the descriptor is the default:
            //          C_replace effectively false, Mask not complemented, A
            //          not transposed.  As a result, GrB_setElement can be
            //          freely mixed with calls to GxB_subassign with C_replace
            //          effectively false and with the identical
            //          GrB_SECOND_Ctype operator.  These calls to
            //          GxB_subassign can use the Mask, either complemented or
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
            //          of S=C(I,J). Then examine all of A, Mask, S, and update
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

            //          With no accum: If there is no Mask and the Mask is not
            //          complemented, then C_replace is irrelavant,  Whether
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
            //          entries in C(I,J), nor all entries in Mask.  Entries in
            //          C but in not A remain unchanged.  This is like an
            //          extended GrB_setElement.  No entries in C can be
            //          deleted.  All other methods must examine all of C(I,J).

            //          Without S_Extraction: C(:,J) or Mask have many entries
            //          compared with A, do not extract S=C(I,J); use
            //          binary search instead.  Otherwise, use the same
            //          S_Extraction method as the other 3 cases.

            //          S_Extraction Method: if nnz(C(:,j)) + nnz(Mask) is
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

            //          With accum: If there is no Mask and the Mask is not
            //          complemented, then C_replace is irrelavant,  Whether
            //          true or false, the results in the two tables
            //          above are the same.

            //          This condition on C_replace holds with our without
            //          accum.  Thus, if there is no Mask, and the Mask is
            //          not complemented, the C_replace can be set to false.

            //------------------------------------------------------------

            // ^^^^^ legend for left columns above:
            // C        prior entry C(I(i),J(j)) exists
            // X        prior entry C(I(i),J(j)) exists but is a zombie
            // .        no prior entry C(I(i),J(j))
            //   A      A(i,j) exists
            //   .      A(i,j) does not exist
            //     1    Mask(i,j)=1, assuming the Mask exists (or if implicit)
            //     0    Mask(i,j)=0, only if Mask exists

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
            //          index via the FLIP function.

            //      ( undelete ):

            //          C(I(i),J(j)) = A(i,j) was a zombie and is no longer
            //          a zombie.  Its row index is restored with another FLIP.

            //      ( X ):

            //          C(I(i),J(j)) was a zombie, and still is a zombie.
            //          row index is < 0, and actual index is FLIP(I(i))

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

        // Once the Mask(i,j) entry is extracted, the codes below explicitly
        // complement the scalar value if Mask_complement is true, before using
        // these action functions.  For the [no mask] case, Mask(i,j)=1.  Thus,
        // only the middle column needs to be considered by each action; the
        // action will handle all three columns at the same time.  All three
        // columns remain in the re-sorted tables below for reference.

        //----------------------------------------------------------------------
        // ----[C A 1] or [X A 1]: C and A present, Mask=1
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

            // Both C(I(i),J(j)) == S(i,j) and A(i,j) are present, and Mij = 1.
            // C(I(i),J(i)) is updated with the entry A(i,j).
            // C_replace has no impact on this action.

            #define COPY_A_TO_C                                             \
            {                                                               \
                /* copies the value of A into C, typecasting if needed */   \
                if (scalar_expansion)                                       \
                {                                                           \
                    memcpy (Cx +(pC*csize), cwork, csize) ;                 \
                }                                                           \
                else                                                        \
                {                                                           \
                    cast_A_to_C (Cx +(pC*csize), Ax +(pA*asize), csize) ;   \
                }                                                           \
            }

            #define X_A_1                                                   \
            {                                                               \
                /* ----[X A 1]                                           */ \
                /* action: ( undelete ): bring a zombie back to life     */ \
                Ci [pC] = iC ;                                              \
                C->nzombies-- ;                                             \
                /* C may need to be removed from the queue; this is done */ \
                /* later, when this function returns */                     \
                COPY_A_TO_C ;                                               \
            }

            #define C_A_1_no_accum                                          \
            {                                                               \
                /* ----[C A 1] no accum                                  */ \
                /* action: ( =A ): copy A into C                         */ \
                COPY_A_TO_C ;                                               \
            }                                                               \

            #define C_A_1_with_accum                                        \
            {                                                               \
                /* ----[C A 1] with accum                                */ \
                /* action: ( =C+A ): apply the accumulator               */ \
                if (!scalar_expansion)                                      \
                {                                                           \
                    cast_A_to_Y (ywork, Ax +(pA*asize), asize) ;            \
                }                                                           \
                cast_C_to_X (xwork, Cx +(pC*csize), csize) ;                \
                faccum (zwork, xwork, ywork) ;                              \
                cast_Z_to_C (Cx +(pC*csize), zwork, csize) ;                \
            }                                                               \

            // C_A_1, general case
            #define C_A_1                                                   \
            {                                                               \
                if (is_zombie)                                              \
                {                                                           \
                    X_A_1 ;                                                 \
                }                                                           \
                else if (faccum == NULL)                                    \
                {                                                           \
                    C_A_1_no_accum ;                                        \
                }                                                           \
                else                                                        \
                {                                                           \
                    C_A_1_with_accum ;                                      \
                }                                                           \
            }

            // C_A_1 with scalar_expansion always true
            #define C_A_1_scalar_expansion                                  \
            {                                                               \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X A 1]                                       */ \
                    /* action: ( undelete ): bring a zombie back to life */ \
                    Ci [pC] = iC ;                                          \
                    C->nzombies-- ;                                         \
                    /* C may need to be removed from the queue; this is */  \
                    /* done later, when this function returns */            \
                    /* Copy A to C */                                       \
                    memcpy (Cx +(pC*csize), cwork, csize) ;                 \
                }                                                           \
                else if (faccum == NULL)                                    \
                {                                                           \
                    /* ----[C A 1] no accum, scalar expansion            */ \
                    /* action: ( =A ): copy A into C                     */ \
                    memcpy (Cx +(pC*csize), cwork, csize) ;                 \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C A 1] with accum, scalar expansion          */ \
                    /* action: ( =C+A ): apply the accumulator           */ \
                    cast_C_to_X (xwork, Cx +(pC*csize), csize) ;            \
                    faccum (zwork, xwork, ywork) ;                          \
                    cast_Z_to_C (Cx +(pC*csize), zwork, csize) ;            \
                }                                                           \
            }

        //----------------------------------------------------------------------
        // ----[. A 1]: C not present, A present, Mask=1
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
            // Mij = 1. The Mask allows C to be written, but no entry present
            // in C (neither a live entry nor a zombie).  This entry must be
            // added to C but it doesn't fit in the pattern.  It is added as a
            // pending tuple.  Zombies and pending tuples do not intersect.

            // If GB_add_pending runs out of memory, it clears C entirely.
            // Otherwise the matrix C would be left in an incoherent partial
            // state of computation.  It's cleaner to just free it all.

            #define D_A_1                                                   \
            {                                                               \
                /* ----[. A 1]                                           */ \
                /* action: ( insert )                                    */ \
                if (scalar_expansion)                                       \
                {                                                           \
                    info = GB_add_pending (C, cwork, ccode, iC, jC) ;       \
                }                                                           \
                else                                                        \
                {                                                           \
                    info = GB_add_pending (C, Ax +(pA*asize), acode, iC, jC) ; \
                }                                                           \
                if (info != GrB_SUCCESS)                                    \
                {                                                           \
                    /* failed to add pending tuple */                       \
                    GB_MATRIX_FREE (&S) ;                                   \
                    return (info) ;                                         \
                }                                                           \
            }

            // D_A_1 with scalar_expansion always true
            #define D_A_1_scalar_expansion                                  \
            {                                                               \
                /* ----[. A 1]                                           */ \
                /* action: ( insert )                                    */ \
                info = GB_add_pending (C, cwork, ccode, iC, jC) ;           \
                if (info != GrB_SUCCESS)                                    \
                {                                                           \
                    /* failed to add pending tuple */                       \
                    GB_MATRIX_FREE (&S) ;                                   \
                    return (info) ;                                         \
                }                                                           \
            }

        //----------------------------------------------------------------------
        // ----[C . 1] or [X . 1]: C present, A not present, Mask=1
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
            // Mij = 1. The Mask allows C to be written, but no entry present
            // in A.  If no accum operator is present, C becomes a zombie.

            // This condition cannot occur if A is a dense matrix,
            // nor for scalar expansion

            #define C_D_1                                                   \
            {                                                               \
                ASSERT (!scalar_expansion && !is_dense) ;                   \
                if (is_zombie)                                              \
                {                                                           \
                    /* ----[X . 1]                                       */ \
                    /* action: ( X ): still a zombie                     */ \
                }                                                           \
                else if (faccum == NULL)                                    \
                {                                                           \
                    /* ----[C . 1] no accum                              */ \
                    /* action: ( delete ): becomes a zombie              */ \
                    C->nzombies++ ;                                         \
                    Ci [pC] = FLIP (iC) ;                                   \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ----[C . 1] with accum                            */ \
                    /* action: ( C ): no change                          */ \
                }                                                           \
            }

        //----------------------------------------------------------------------
        // ----[C A 0] or [X A 0]: both C and A present but Mask=0
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

            // Both C(I(i),J(j)) == S(i,j) and A(i,j) are present, and Mij = 0.
            // The mask prevents A being written to C, so A has no effect on
            // the result.  If C_replace is true, however, the entry is
            // deleted, becoming a zombie.  This case does not occur if
            // the Mask is not present.  This action also handles the
            // [C . 0] and [X . 0] cases; see the next section below.

            // This condition can still occur if A is dense, so if a Mask is
            // present, entries can still be deleted from C.  As a result, the
            // fact that A is dense cannot be exploited when the Mask is
            // present.

            #define C_A_0                                                   \
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
                    C->nzombies++ ;                                         \
                    Ci [pC] = FLIP (iC) ;                                   \
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
            // the entry is cleared.  The Mask does not protect C from the
            // C_replace action.

        //----------------------------------------------------------------------
        // ----[C . 0] or [X . 0]: C present, A not present, Mask=0
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
            // and Mij = 0.  Since A(i,j) has no effect on the result,
            // this is the same as the C_A_0 action above.

            // This condition cannot occur if A is a dense matrix, nor for
            // scalar expansion, but the existance of the entry A is not
            // relevant.

            #define C_D_0 C_A_0

        //----------------------------------------------------------------------
        // ----[. A 0]: C not present, A present, Mask=0
        //----------------------------------------------------------------------

            // . A 0                    .           insert   | no accum,no Crepl
            // . A 0                    .           insert   | no accum,no Crepl
            // . A 0                    .           insert   | accum, no Crepl
            // . A 0                    .           insert   | accum, Crepl

            // C(I(i),J(j)) == S(i,j) is not present, A(i,j) is present,
            // but Mij = 0.  The Mask prevents A from modifying C, so the
            // A(i,j) entry is ignored.  C_replace has no effect since the
            // entry is already cleared.  There is nothing to do.

        //----------------------------------------------------------------------
        // ----[. . 1] and [. . 0]: no entries in C and A, Mask = 0 or 1
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
            // Nothing happens.  The Mask(i,j) entry is present, otherwise
            // this (i,j) position would not be considered at all.
            // The Mask(i,j) entry has no effect.  There is nothing to do.

    //--------------------------------------------------------------------------
    // check for quicker method if accum is present and C_replace is false
    //--------------------------------------------------------------------------

    // Before allocating S, see if there is a faster method
    // that does not require S to be created.

    const int64_t *Cp = C->p ;
    int64_t *Ci = C->i ;
    void    *Cx = C->x ;
    const size_t csize = C->type->size ;
    const GB_Type_code ccode = C->type->code ;

    bool S_Extraction = true ;

    bool C_Mask_scalar = (scalar_expansion && !C_replace &&
        Mask != NULL && !Mask_comp && I == GrB_ALL) ;

    if (C_Mask_scalar)
    {
        // C(:,J)<Mask>=scalar, mask present and not complemented,
        // C_replace false, special case
        S_Extraction = false ;
    }
    else if (accum != NULL && !C_replace)
    {
        // If accum is present and C_replace is false, then only entries in A
        // need to be examined.  Not all entries in C(I,J) and Mask need to be
        // examined.  As a result, computing S=C(I,J) can dominate the time and
        // memory required for the S_Extraction method.
        int64_t cnz = NNZ (C) ;
        if (ni == 1 || nj == 1 || cnz == 0)
        {
            // No need to form S if it has just a single row or column.  If C
            // is empty so is S, so don't bother computing it.
            S_Extraction = false ;
            // printf ("No S, one\n") ;
        }
        else if (mn_ok && cnz + nzMask > anz)
        {
            // If C and Mask are very dense, then do not extract S
            int64_t cnz = nzMask ;
            for (int64_t j = 0 ; S_Extraction && j < nj ; j++)
            {
                int64_t jC = (J == GrB_ALL) ? j : J [j] ;
                if (jC < 0 || jC > C->ncols)
                {
                    // invalid column index; check them all in
                    // GB_subref_symbolic
                    break ;
                }
                cnz += Cp [jC+1] - Cp [jC] ;
                if (cnz/8 > anz)
                {
                    // NNZ(C) + NNZ(Mask) is much larger than NNZ(A).  Do not
                    // construct S=C(I,J).  Instead, scan through all of A and
                    // use binary search to find the corresponding positions in
                    // C and Mask.
                    // printf ("No S, two\n") ;
                    S_Extraction = false ;
                }
            }
            if (cnz == 0)
            {
                // C (:,J) and Mask (:,J) are empty; no need to compute S
                // printf ("No S, three\n") ;
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
    // 100 = C->i [pC], and pC will be between C->pC [200] ... C->pC [200+1]-1.

    GrB_Matrix S = NULL ;
    const int64_t *Sp = NULL ;
    const int64_t *Si = NULL ;
    const int64_t *Sx = NULL ;

    if (S_Extraction)
    {

        //----------------------------------------------------------------------
        // extract symbolic structure S=C(I,J)
        //----------------------------------------------------------------------

        // also checks I and J
        info = GB_subref_symbolic (&S, ni, nj, C, I, ni, J, nj) ;
        if (info != GrB_SUCCESS)
        {
            // out of memory or invalid indices I and J
            // the C matrix is unchanged, no need to clear it
            return (info) ;
        }

        ASSERT_OK (GB_check (C, "C for subref extraction", 0)) ;
        ASSERT_OK (GB_check (S, "S extraction", 0)) ;

        Sp = S->p ;
        Si = S->i ;
        Sx = S->x ;

        #ifndef NDEBUG
        // this body of code explains what S contains
        // S is ni-by-nj where ni = length (I) and nj = length (J)
        for (int64_t jnew = 0 ; jnew < nj ; jnew++)
        {
            // S (inew,jnew) corresponds to C (iold, jold) ;
            int64_t jold = (J == GrB_ALL) ? jnew : J [jnew] ;
            for (int64_t pS = Sp [jnew] ; pS < Sp [jnew+1] ; pS++)
            {
                // S (inew,jnew) is a pointer back into C (I(inew), J(jnew))
                int64_t inew = Si [pS] ;
                ASSERT (inew >= 0 && inew < ni) ;
                int64_t iold = (I == GrB_ALL) ? inew : I [inew] ;
                int64_t p = Sx [pS] ;
                ASSERT (p >= 0 && p < NNZ (C)) ;
                ASSERT (C->p [jold] <= p && p < C->p [jold+1]) ;
                ASSERT (iold == UNFLIP (C->i [p])) ;
            }
        }
        #endif

    }
    else
    {

        //----------------------------------------------------------------------
        // do not create S=C(I,J), but do check I and J
        //----------------------------------------------------------------------

        // GB_subref_symbolic does this task, so do it here for method (2)

        bool need_qsort, contig ;
        int64_t imin, imax ;
        info = GB_ijproperties (I, ni, J, nj, C->nrows, C->ncols,
                            &need_qsort, &contig, &imin, &imax) ;
        if (info != GrB_SUCCESS)
        {
            // I or J invalid
            // the C matrix is unchanged, no need to clear it
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // check if an empty Mask is complemented (case: C_replace true)
    //--------------------------------------------------------------------------

    if (Mask_comp && Mask == NULL)
    {
        // C_replace==false has already been handled; see "No work..." above
        // S = C(I,J) is required, and has just been computed above
        ASSERT (C_replace) ;
        ASSERT (S != NULL) ;
        ASSERT_OK (GB_check (C, "C: empty mask compl, C_replace true", 0)) ;
        ASSERT_OK (GB_check (S, "S pattern", 0)) ;

        // C(I,J) = "zero"; turn all entries in C(I,J) into zombies
        for (int64_t jnew = 0 ; jnew < nj ; jnew++)
        {
            for (int64_t pS = Sp [jnew] ; pS < Sp [jnew+1] ; pS++)
            {
                // S (inew,jnew) is a pointer back into C (I(inew), J(jnew))
                C_LOOKUP ;
                if (!is_zombie)
                {
                    // ----[C - 0] replace
                    // action: ( delete ): becomes a zombie
                    C->nzombies++ ;
                    Ci [pC] = FLIP (iC) ;
                }
            }
        }

        //----------------------------------------------------------------------
        // insert C in the queue if it has work to do and isn't already queued
        //----------------------------------------------------------------------

        GB_queue_insert (C) ;
        ASSERT_OK (GB_check(C, "C: empty mask compl, C_replace, done", 0)) ;

        //----------------------------------------------------------------------
        // free workspace, check blocking mode, and return
        //----------------------------------------------------------------------

        GB_MATRIX_FREE (&S) ;
        return (GB_block (C)) ;
    }

    // C_replace can now only be true if the Mask is present.  If the Mask
    // is not present, then C_replace is now effectively false.

    //--------------------------------------------------------------------------
    // scalar workspace
    //--------------------------------------------------------------------------

    const int64_t *Ap = NULL ;
    const int64_t *Ai = NULL ;
    const void    *Ax = NULL ;
    size_t asize ;
    GB_Type_code acode ;

    size_t xsize = 1, ysize = 1, zsize = 1 ;
    if (accum != NULL)
    {
        xsize = accum->xtype->size ;
        ysize = accum->ytype->size ;
        zsize = accum->ztype->size ;
    }
    char cwork [csize] ;
    char xwork [xsize] ;
    char ywork [ysize] ;
    char zwork [zsize] ;

    if (scalar_expansion)
    {
        // For user-defined types, the scalar is required to have the same
        // type as C.  This cannot be checked; results are undefined if the
        // user passes in a void * pointer to a different user-defined type.
        acode = scalar_code ;
        asize = GB_Type_size (scalar_code, csize) ;
    }
    else
    {
        Ap = A->p ;
        Ai = A->i ;
        Ax = A->x ;
        acode = A->type->code ;
        asize = A->type->size ;
    }

    //--------------------------------------------------------------------------
    // get casting functions
    //--------------------------------------------------------------------------

    // if any of these are user-defined types, the "cast" is just a
    // user-to-user copy, using GB_copy_user_user.  See GB_cast_factory.
    GB_cast_function cast_A_to_C, cast_A_to_Y, cast_C_to_X, cast_Z_to_C ;
    cast_A_to_C = GB_cast_factory (ccode, acode) ;

    GB_binary_function faccum = NULL ;

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
    // get the Mask
    //--------------------------------------------------------------------------

    const int64_t *Maskp = NULL ;
    const int64_t *Maski = NULL ;
    const void    *Maskx = NULL ;
    size_t msize = 0 ;
    GB_cast_function cast_Mask_to_bool = NULL ;

    if (Mask != NULL)
    {
        Maskp = Mask->p ;
        Maski = Mask->i ;
        Maskx = Mask->x ;
        msize = Mask->type->size ;
        cast_Mask_to_bool = GB_cast_factory (GB_BOOL_code, Mask->type->code) ;
        ASSERT_OK (GB_check (Mask, "Mask for assign", 0)) ;
    }

    //==========================================================================
    // submatrix assignment C(I,J)<Mask> = accum (C(I,J),A):  four methods
    //==========================================================================

    if (C_Mask_scalar)
    {

        //----------------------------------------------------------------------
        // C(:,J)<Mask> = scalar or += scalar, special case
        //----------------------------------------------------------------------

        // This method iterates across all entries in the Mask, and for each
        // place where the Mask(i,j) is true, it updates C(i,j).  Not all of
        // C needs to be examined.  Also, since I=GrB_ALL, the row indices of
        // the Mask are the same as the row indices of C.  The accum operator
        // may be present, or absent.

        ASSERT (scalar_expansion) ;                 // A is a scalar
        ASSERT (Mask != NULL && !Mask_comp) ;       // Mask present, not compl.
        ASSERT (I == GrB_ALL) ;                     // I is GrB_ALL
        ASSERT (!C_replace) ;                       // C_replace is false

        int64_t pA = 0 ;    // unused

        for (int64_t j = 0 ; j < nj ; j++)
        {
            int64_t jC = (J == GrB_ALL) ? j : J [j] ;

            if (Cp [jC+1] - Cp [jC] == C->nrows)
            {

                // C(:,jC) is dense so the binary search of C is not needed
                for (int64_t pM = Maskp [j] ; pM < Maskp [j+1] ; pM++)
                {

                    //----------------------------------------------------------
                    // consider the entry Mask(i,j)
                    //----------------------------------------------------------

                    bool Mij ;
                    cast_Mask_to_bool (&Mij, Maskx +(pM*msize), 0) ;

                    //----------------------------------------------------------
                    // update C(iC,jC), but only if Mask(iC,j) allows it
                    //----------------------------------------------------------

                    if (Mij)
                    {
                        int64_t iC = Maski [pM] ;
                        int64_t pC = Cp [jC] + iC ;
                        bool is_zombie = IS_ZOMBIE (Ci [pC]) ;
                        ASSERT (UNFLIP (Ci [pC]) == iC) ;

                        //------------------------------------------------------
                        // set the element
                        //------------------------------------------------------

                        // ----[C A 1] or [X A 1]-------------------------------
                        // [C A 1]: action: ( =C+A ): apply accum
                        // [C A 1]: action: ( =A ): copy A into C, no accum
                        // [X A 1]: action: ( undelete ): bring zombie back
                        C_A_1_scalar_expansion ;
                    }
                }
            }
            else
            {

                // C(:,jC) is sparse; use binary search for C
                for (int64_t pM = Maskp [j] ; pM < Maskp [j+1] ; pM++)
                {

                    //----------------------------------------------------------
                    // consider the entry Mask(i,j)
                    //----------------------------------------------------------

                    bool Mij ;
                    cast_Mask_to_bool (&Mij, Maskx +(pM*msize), 0) ;

                    //----------------------------------------------------------
                    // find C(iC,jC), but only if Mask(i,j) allows it
                    //----------------------------------------------------------

                    if (Mij)
                    {
                        int64_t iC = Maski [pM] ;

                        int64_t pC = Cp [jC] ;
                        bool found, is_zombie ;
                        int64_t pright = Cp [jC+1] - 1 ;
                        GB_BINARY_ZOMBIE (iC, Ci, pC, pright, found,
                            C->nzombies, is_zombie) ;

                        //------------------------------------------------------
                        // set the element
                        //------------------------------------------------------

                        if (found)
                        {
                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [C A 1]: action: ( =A ): copy A into C, no accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            C_A_1_scalar_expansion ;
                        }
                        else
                        {
                            // ----[. A 1]--------------------------------------
                            // action: ( insert )
                            D_A_1_scalar_expansion ;
                        }
                    }
                }
            }
        }

    }
    else if (S == NULL)
    {

        //----------------------------------------------------------------------
        // assignment without S
        //----------------------------------------------------------------------

        // this method can only be used if accum is present and C_replace
        // is false
        ASSERT (accum != NULL) ;
        ASSERT (C_replace == false) ;

        for (int64_t j = 0 ; j < nj ; j++)
        {
            int64_t jC = (J == GrB_ALL) ? j : J [j] ;

            int64_t pA_start = (scalar_expansion) ?  0 : Ap [j] ;
            int64_t pA_end   = (scalar_expansion) ? ni : Ap [j+1] ;

            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
            {

                //--------------------------------------------------------------
                // consider the entry A(iA,j)
                //--------------------------------------------------------------

                int64_t iA = (scalar_expansion) ? pA : Ai [pA] ;
                int64_t iC = (I == GrB_ALL) ? iA : I [iA] ;

                //--------------------------------------------------------------
                // find Mask(iA,j)
                //--------------------------------------------------------------

                bool Mij = true ;

                if (Mask != NULL)
                {
                    int64_t pleft  = Maskp [j] ;
                    int64_t pright = Maskp [j+1] - 1 ;
                    bool found ;
                    GB_BINARY_SEARCH (iA, Maski, pleft, pright, found) ;
                    if (found)
                    {
                        // found it
                        cast_Mask_to_bool (&Mij, Maskx +(pleft*msize), 0) ;
                    }
                    else
                    {
                        // Mask(iA,j) not present, implicitly false
                        Mij = false ;
                    }
                    if (Mask_comp)
                    {
                        // negate the Mask if Mask_comp is true
                        Mij = !Mij ;
                    }
                }

                //--------------------------------------------------------------
                // find C(iC,jC), but only if Mask(iA,j) allows it
                //--------------------------------------------------------------

                if (Mij)
                {

                    int64_t pC = Cp [jC] ;
                    int64_t pright = Cp [jC+1] - 1 ;
                    bool found, is_zombie ;

                    GB_BINARY_ZOMBIE (iC, Ci, pC, pright, found,
                        C->nzombies, is_zombie) ;

                    //----------------------------------------------------------
                    // set the element
                    //----------------------------------------------------------

                    if (found)
                    {
                        // ----[C A 1] or [X A 1]-------------------------------
                        // [C A 1]: action: ( =C+A ): apply accum
                        // [X A 1]: action: ( undelete ): bring zombie back
                        // the ( =A ) case with no accum does not appear since
                        // this method is not used when accum is NULL
                        if (is_zombie)
                        {
                            X_A_1 ;
                        }
                        else
                        {
                            C_A_1_with_accum ;
                        }
                    }
                    else
                    {
                        // ----[. A 1]------------------------------------------
                        // action: ( insert )
                        D_A_1 ;
                    }
                }
            }
        }


    }
    else if (Mask == NULL)
    {

        //----------------------------------------------------------------------
        // assignment using S_Extraction method, no Mask
        //----------------------------------------------------------------------

        // 6 cases to consider

        // [ C A 1 ]    C_A_1: C present, A present
        // [ X A 1 ]    C_A_1: C zombie, A present
        // [ . A 1 ]    D_A_1: C not present, A present
        // [ C . 1 ]    C_D_1: C present, A not present
        // [ X . 1 ]    C_D_1: C zombie, A not present
        // [ . . 1 ]           not encountered, nothing to do

        for (int64_t j = 0 ; j < nj ; j++)
        {

            //------------------------------------------------------------------
            // do a 2-way merge of S(:,j) and A(:,j)
            //------------------------------------------------------------------

            int64_t jC = (J == GrB_ALL) ? j : J [j] ;

            int64_t pS     = Sp [j] ;
            int64_t pS_end = Sp [j+1] ;

            int64_t pA     = (scalar_expansion) ?  0 : Ap [j] ;
            int64_t pA_end = (scalar_expansion) ? ni : Ap [j+1] ;

            // while both list S (:,j) and A (:,j) have entries
            while (pS < pS_end && pA < pA_end)
            {
                int64_t iS = Si [pS] ;
                int64_t iA = (scalar_expansion) ? pA : Ai [pA] ;

                if (iS < iA)
                {
                    // ----[C . 1] or [X . 1]-----------------------------------
                    // S (i,j) is present but A (i,j) is not
                    // [C . 1]: action: ( delete ): becomes a zombie
                    // [X . 1]: action: ( X ): still a zombie
                    C_LOOKUP ;
                    C_D_1 ;
                    pS++ ;

                }
                else if (iA < iS)
                {
                    // ----[. A 1]----------------------------------------------
                    // S (i,j) is not present, A (i,j) is present
                    // [. A 1]: action: ( insert )
                    int64_t iC = (I == GrB_ALL) ? iA : I [iA] ;
                    D_A_1 ;
                    pA++ ;
                }
                else
                {
                    // ----[C A 1] or [X A 1]-----------------------------------
                    // both S (i,j) and A (i,j) present
                    // [C A 1]: action: ( =A ): copy A into C, no accum
                    // [C A 1]: action: ( =C+A ): apply accum
                    // [X A 1]: action: ( undelete ): bring zombie back
                    C_LOOKUP ;
                    C_A_1 ;
                    pS++ ;
                    pA++ ;
                }
            }

            // while list S (:,j) has entries.  List A (:,j) exhausted
            while (pS < pS_end)
            {
                // ----[C . 1] or [X . 1]---------------------------------------
                // S (i,j) is present but A (i,j) is not
                // [C . 1]: action: ( delete ): becomes a zombie
                // [X . 1]: action: ( X ): still a zombie
                C_LOOKUP ;
                C_D_1 ;
                pS++ ;
            }

            // while list A (:,j) has entries.  List S (:,j) exhausted
            while (pA < pA_end)
            {
                // ----[. A 1]--------------------------------------------------
                // S (i,j) is not present, A (i,j) is present
                // [. A 1]: action: ( insert )
                int64_t iA = (scalar_expansion) ? pA : Ai [pA] ;
                int64_t iC = (I == GrB_ALL) ? iA : I [iA] ;
                D_A_1 ;
                pA++ ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // assignment using S_Extraction method, with Mask
        //----------------------------------------------------------------------

        // 12 cases to consider

        // [ C A 1 ]    C_A_1: C present, A present, Mask present and = 1
        // [ X A 1 ]    C_A_1: C zombie, A present, Mask present and = 1
        // [ . A 1 ]    D_A_1: C not present, A present, Mask present and = 1
        // [ C . 1 ]    C_D_1: C present, A not present, Mask present and = 1
        // [ X . 1 ]    C_D_1: C zombie, A not present, Mask present and = 1
        // [ . . 1 ]           only Mask=1 present, nothing to do

        // [ C A 0 ]    C_A_0: C present, A present, Mask not present or zero
        // [ X A 0 ]    C_A_0: C zombie, A present, Mask not present or zero
        // [ . A 0 ]           C not present, A present, Mask 0; nothing to do
        // [ C . 0 ]    C_D_0: C present, A not present, Mask 0
        // [ X . 0 ]    C_D_0: C zombie, A not present, Mask 0
        // [ . . 0 ]           Mask not present or zero, nothing to do

        for (int64_t j = 0 ; j < nj ; j++)
        {

            //------------------------------------------------------------------
            // do a 3-way merge of S(:,j), A(:,j), and Mask(:,j)
            //------------------------------------------------------------------

            int64_t jC = (J == GrB_ALL) ? j : J [j] ;

            // The merge is similar to GB_mask, except that it does not produce
            // another output matrix.  Instead, the results are written
            // directly into C, either modifying the entries there or adding
            // pending tuples.

            int64_t pS     = Sp [j] ;
            int64_t pS_end = Sp [j+1] ;

            int64_t pM     = Maskp [j] ;
            int64_t pM_end = Maskp [j+1] ;

            int64_t pA     = (scalar_expansion) ?  0 : Ap [j] ;
            int64_t pA_end = (scalar_expansion) ? ni : Ap [j+1] ;

            // There are three sorted lists to merge:
            // S(:,j)    in [pS .. pS_end-1]
            // A(:,j)    in [pA .. pA_end-1]
            // Mask(:,j) in [pM .. pM_end-1]
            // The head of each list is at index pS, pA, and pM, and
            // an entry is 'discarded' by incremented its respective index.
            // Once a list is consumed, a query for its next row index will
            // result in a dummy value ni larger than all valid row indices.

            //------------------------------------------------------------------
            // while either list S(:,j) or A(:,j) have entries
            //------------------------------------------------------------------

            while (pS < pS_end || pA < pA_end)
            {

                //--------------------------------------------------------------
                // Get the indices at the top of each list.
                //--------------------------------------------------------------

                // If a list has been consumed, use a dummy index ni
                // that is larger than all valid indices.
                int64_t iS = (pS < pS_end) ? Si    [pS] : ni ;
                int64_t iA = (pA < pA_end) ?
                             ((scalar_expansion) ? pA : Ai [pA]) : ni ;
                int64_t iM = (pM < pM_end) ? Maski [pM] : ni ;

                //--------------------------------------------------------------
                // find the smallest index of [iS iA iM]
                //--------------------------------------------------------------

                // i = min ([iS iA iM])
                int64_t i = IMIN (iS, IMIN (iA, iM)) ;
                ASSERT (i < ni) ;

                // When an entry S(i,j), A(i,j), or Mask(i,j) is found and
                // processed it must be 'discarded' from its list.  This is
                // done by incrementing its corresponding pointer.
                #define DISCARD(X) (p ## X)++ ;

                //--------------------------------------------------------------
                // get Mask (i,j)
                //--------------------------------------------------------------

                // If an explicit value of Mask(i,j) must be tested, it must
                // first be typecasted to bool.  If (i == iM), then Mask(i,j)
                // is present and is typecasted into Mij and then discarded.
                // Otherwise, if Mask(i,j) is not present, Mij is set to false.
                bool Mij ;
                if (i == iM)
                {
                    // Mij = (bool) Mask [pM]
                    cast_Mask_to_bool (&Mij, Maskx +(pM*msize), 0) ;
                    DISCARD (M) ;
                }
                else
                {
                    // Mij not present, implicitly false
                    ASSERT (i < iM) ;
                    Mij = false ;
                }

                // explicitly complement the mask entry Mij
                if (Mask_comp)
                {
                    Mij = !Mij ;
                }

                //--------------------------------------------------------------
                // handle all 12 cases
                //--------------------------------------------------------------

                if (i == iS)
                {
                    if (i == iA)
                    {
                        // both S (i,j) and A (i,j) present
                        C_LOOKUP ;
                        if (Mij)
                        {
                            // ----[C A 1] or [X A 1]---------------------------
                            // [C A 1]: action: ( =A ): copy A into C, no accum
                            // [C A 1]: action: ( =C+A ): apply accum
                            // [X A 1]: action: ( undelete ): bring zombie back
                            C_A_1 ;
                        }
                        else
                        {
                            // ----[C A 0] or [X A 0]---------------------------
                            // [X A 0]: action: ( X ): still a zombie
                            // [C A 0]: C_repl: action: ( delete ): zombie
                            // [C A 0]: no C_replace: action: ( C ): no change
                            C_A_0 ;
                        }
                        DISCARD (S) ;
                        DISCARD (A) ;
                    }
                    else
                    {
                        // S (i,j) is present but A (i,j) is not
                        C_LOOKUP ;
                        if (Mij)
                        {
                            // ----[C . 1] or [X . 1]---------------------------
                            // [C . 1]: action: ( delete ): becomes a zombie
                            // [X . 1]: action: ( X ): still a zombie
                            C_D_1 ;
                        }
                        else
                        {
                            // ----[C . 0] or [X . 0]---------------------------
                            // [X . 0]: action: ( X ): still a zombie
                            // [C . 0]: if C_repl: action: ( delete ): zombie
                            // [C . 0]: no C_replace: action: ( C ): no change
                            C_D_0 ;
                        }
                        DISCARD (S) ;
                    }
                }
                else
                {
                    if (i == iA)
                    {
                        // S (i,j) is not present, A (i,j) is present
                        if (Mij)
                        {
                            // ----[. A 1]--------------------------------------
                            // [. A 1]: action: ( insert )
                            int64_t iC = (I == GrB_ALL) ? iA : I [iA] ;
                            D_A_1 ;
                        }
                        else
                        {
                            // ----[. A 0]--------------------------------------
                            // action: ( . ): no action
                        }
                        DISCARD (A) ;
                    }
                    else
                    {
                        // neither S (i,j) nor A (i,j) present
                        // ----[. . 1]------------------------------------------
                        // ----[. . 0]------------------------------------------
                        // action: ( . ): no action
                        ASSERT (i == iM) ; // i must be equal to one of them
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // insert C in the queue if it has work to do and isn't already queued
    //--------------------------------------------------------------------------

    if (C->nzombies == 0 && C->npending == 0)
    {
        // C may be in the queue from a prior assignment, but this assignemt
        // can bring zombies back to life, and the zombie count can go to zero.
        // In that case, C must be removed from the queue.  The removal does
        // nothing if C is already not in the queue.
        GB_queue_remove (C) ;
    }
    else
    {
        // If C has any zombies or pending tuples, it must be in the queue.
        // The queue insert does nothing if C is already in the queue.
        GB_queue_insert (C) ;
    }
    ASSERT_OK (GB_check (C, "C(I,J) result", 0)) ;

    //--------------------------------------------------------------------------
    // free workspace, check blocking mode, and return
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&S) ;
    return (GB_block (C)) ;
}

#undef C_LOOKUP
#undef COPY_A_TO_C
#undef X_A_1
#undef C_A_1_no_accum
#undef C_A_1_with_accum
#undef C_A_1
#undef D_A_1
#undef C_D_1
#undef C_A_0
#undef C_D_0
#undef DISCARD
#undef C_A_1_scalar_expansion
#undef D_A_1_scalar_expansion

