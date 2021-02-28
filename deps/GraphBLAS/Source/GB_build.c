//------------------------------------------------------------------------------
// GB_build: build a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// CALLED BY: GB_matvec_build and GB_reduce_to_vector
// CALLS:     GB_builder

// GB_matvec_build constructs a GrB_Matrix or GrB_Vector from the tuples
// provided by the user.  In that case, the tuples must be checked for
// duplicates.  They might be sorted on input, so this condition is checked and
// exploited if found.  GB_reduce_to_vector constructs a GrB_Vector froma
// GrB_Matrix, by discarding the vector index.  As a result, duplicates are
// likely to appear, and the input is likely to be unsorted.  But for
// GB_reduce_to_vector, the validity of the tuples need not be checked.  All of
// these conditions are checked in GB_builder.

// GB_build constructs a matrix C from a list of indices and values.  Any
// duplicate entries with identical indices are assembled using the binary dup
// operator provided on input.  All three types (x,y,z for z=dup(x,y)) must be
// identical.  The types of dup, S, and C must all be compatible.

// Duplicates are assembled using T(i,j) = dup (T (i,j), S (k)) into a
// temporary matrix T that has the same type as the dup operator.  The
// GraphBLAS spec requires dup to be associative so that entries can be
// assembled in any order.  There is no way to check this condition if dup is a
// user-defined operator.  It could be checked for built-in operators, but the
// GraphBLAS spec does not require this condition to cause an error so that is
// not done here.  If dup is not associative, the GraphBLAS spec states that
// the results are not defined.

// SuiteSparse:GraphBLAS provides a well-defined order of assembly, however.
// For a CSC format, entries in [I,J,S] are first sorted in increasing order of
// row and column index via a stable sort, with ties broken by the position of
// the tuple in the [I,J,S] list.  If duplicates appear, they are assembled in
// the order they appear in the [I,J,S] input.  That is, if the same indices i
// and j appear in positions k1, k2, k3, and k4 in [I,J,S], where k1 < k2 < k3
// < k4, then the following operations will occur in order:

//      T (i,j) = S (k1) ;

//      T (i,j) = dup (T (i,j), S (k2)) ;

//      T (i,j) = dup (T (i,j), S (k3)) ;

//      T (i,j) = dup (T (i,j), S (k4)) ;

// This is a well-defined order but the user should not depend upon it since
// the GraphBLAS spec does not require this ordering.  Results may differ in
// different implementations of GraphBLAS.

// However, with this well-defined order, the SECOND operator will result in
// the last tuple overwriting the earlier ones.  This is relied upon internally
// by GB_wait.

// After the matrix T is assembled, it is typecasted into the type of C, the
// final output matrix.  No typecasting is done during assembly of duplicates,
// since mixing the two can break associativity and lead to unpredictable
// results.  Note that this is not the case for GB_wait, which must typecast
// each tuple into its output matrix in the same order they are seen in
// the [I,J,S] pending tuples.

// On input, C must not be NULL.  C->type, C->vlen, C->vdim and C->is_csc must
// be valid on input and are unchanged on output.  C must not have any existing
// entries on input (GrB_*_nvals (C) must return zero, per the specification).
// However, all existing content in C is freed.

// The list of numerical values is given by the void * S array and a type code,
// scode.  The latter is defined by the actual C type of the S parameter in
// the user-callable functions.  However, for user-defined types, there is no
// way of knowing that the S array has the same type as dup or C, since in that
// case S is just a void * pointer.  Behavior is undefined if the user breaks
// this condition.

// C is returned as hypersparse or non-hypersparse, depending on the number of
// non-empty vectors of C.  If C has very few non-empty vectors, then it is
// returned as hypersparse.  Only if the number of non-empty vectors is
// Omega(n) is C returned as non-hypersparse, which implies nvals is Omega(n),
// where n = # of columns of C if CSC, or # of rows if CSR.  As a result, the
// time taken by this function is just O(nvals*log(nvals)), regardless of what
// format C is returned in.

// The input arrays I_input, J_input, and S_input are not modified.
// If nvals == 0, I_input, J_input, and S_input may be NULL.

#include "GB_build.h"

GrB_Info GB_build               // build matrix
(
    GrB_Matrix C,               // matrix to build
    const GrB_Index *I_input,   // "row" indices of tuples (as if CSC)
    const GrB_Index *J_input,   // "col" indices of tuples (as if CSC) NULL for
                                // GrB_Vector_build or GB_reduce_to_vector
    const void *S_input,        // values
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of S_input array
    const bool is_matrix,       // true if C is a matrix, false if GrB_Vector
    const bool ijcheck,         // true if I and J are to be checked
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;

    //--------------------------------------------------------------------------
    // free all content of C
    //--------------------------------------------------------------------------

    // the type, dimensions, and hyper ratio are still preserved in C.
    GB_PHIX_FREE (C) ;
    ASSERT (GB_EMPTY (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (C->magic == GB_MAGIC2) ;

    //--------------------------------------------------------------------------
    // build the matrix T
    //--------------------------------------------------------------------------

    // T is always hypersparse.  Its type is the same as the z output of the
    // z=dup(x,y) operator.

    // S_input must be treated as read-only, so GB_builder is not allowed to
    // transplant it into T->x.

    int64_t *no_I_work = NULL ;
    int64_t *no_J_work = NULL ;
    GB_void *no_S_work = NULL ;
    GrB_Matrix T = NULL ;

    GrB_Info info = GB_builder
    (
        &T,             // create T
        dup->ztype,     // T has the type determined by the dup operator
        C->vlen,        // T->vlen = C->vlen
        C->vdim,        // T->vdim = C->vdim
        C->is_csc,      // T has the same CSR/CSC format as C
        &no_I_work,     // I_work_handle, not used here
        &no_J_work,     // J_work_handle, not used here
        &no_S_work,     // S_work_handle, not used here
        false,          // known_sorted: not yet known
        false,          // known_no_duplicatces: not yet known
        0,              // I_work, J_work, and S_work not used here
        is_matrix,      // true if T is a GrB_Matrix
        ijcheck,        // true if I and J are to be checked
        (int64_t *) ((C->is_csc) ? I_input : J_input),
        (int64_t *) ((C->is_csc) ? J_input : I_input),
        S_input,        // original values, each of size nvals, not modified
        nvals,          // number of tuples
        dup,            // operator to assemble duplicates
        scode,          // type of the S array
        Context
    ) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // transplant and typecast T into C, conform C, and free T
    //--------------------------------------------------------------------------

    return (GB_transplant_conform (C, C->type, &T, Context)) ;
}

