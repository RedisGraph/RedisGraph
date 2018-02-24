//------------------------------------------------------------------------------
// GB_build: build a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Construct a matrix C from a list of indices and values.  Any duplicate
// entries with identical indices are assembled using the binary dup operator
// provided on input.  All three types (x,y,z for z=dup(x,y)) must be
// identical.  The types of dup, X, and C must all be compatible.

// Duplicates are assembled using T(i,j) = dup (T (i,j), X (k)) into a
// temporary matrix T that has the same type as the dup operator.  The
// GraphBLAS spec requires dup to be associative so that entries can be
// assembled in any order.  There is no way to check this condition if dup is a
// user-defined operator.  It could be checked for built-in operators, but the
// GraphBLAS spec does not require this condition to cause an error so that is
// not done here.  If dup is not associative, the GraphBLAS spec states that
// the results are not defined.

// SuiteSparse:GraphBLAS provides a well-defined order of assembly, however.
// Entries in [I,J,X] are first sorted in increasing order of row and column
// index via a stable sort, with ties broken by the position of the tuple in
// the [I,J,X] list.  If duplicates appear, they are assembled in the order
// they appear in the [I,J,X] input.  That is, if the same indices i and j
// appear in positions k1, k2, k3, and k4 in [I,J,X], where k1 < k2 < k3 < k4,
// then the following operations will occur in order:

//      T (i,j) = X (k1) ;
//      T (i,j) = dup (T (i,j), X (k2)) ;
//      T (i,j) = dup (T (i,j), X (k3)) ;
//      T (i,j) = dup (T (i,j), X (k4)) ;

// This is a well-defined order but the user should not depend upon it since
// the GraphBLAS spec does not require this ordering.  Results may differ in
// different implementations of GraphBLAS.

// However, with this well-defined order, the "SECOND" operator will result in
// the last tuple overwriting the earlier ones.  This is relied upon internally
// by GB_wait.

// After the matrix T is assembled, it is typecasted into the type of C.  That
// is, no typecasting is done during assembly of duplicates, since mixing the
// two can break associativity and lead to unpredictable results.

// On input, C must not be NULL.  C->type, C->nrows, and C->ncols must be valid
// on input and are not changed.  C->p must exist and be of the right size; its
// content is ignored.  C->x and C->i are freed and new ones are allocated.  C
// must not have any existing entries on input (GrB_*_nvals (C) must return
// zero).

// The list of numerical values is given by the void * X array and a type code,
// X_code.  The latter is defined by the actual C type of the X parameter in
// the user-callable functions.  However, for user-defined types, there is no
// way of knowing that the X array has the same type as dup or C, since in that
// case X is just a void * pointer.  Behavior is undefined if the user breaks
// this condition.

#include "GB.h"

GrB_Info GB_build               // check inputs then build matrix
(
    GrB_Matrix C,               // matrix to build
    const GrB_Index *I,         // row indices of tuples
    const GrB_Index *J,         // col indices of tuples, ignored if ncols <= 1
    const void *X,              // array of values of tuples
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code X_code   // GB_Type_code of X array
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C matrix to build", 0)) ;
    RETURN_IF_NULL (I) ;
    if (I == GrB_ALL)
    {
        return (ERROR (GrB_INVALID_VALUE, (LOG,
            "List of row indices I cannot be 'GrB_ALL'"))) ;
    }

    int64_t nrows = C->nrows ;
    int64_t ncols = C->ncols ;
    if (ncols > 1)
    {
        RETURN_IF_NULL (J) ;
        if (J == GrB_ALL)
        {
            return (ERROR (GrB_INVALID_VALUE, (LOG,
                "List of column indices J cannot be 'GrB_ALL'"))) ;
        }
    }

    RETURN_IF_NULL (X) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (dup) ;

    ASSERT_OK (GB_check (dup, "dup operator for assembling duplicates", 0)) ;
    ASSERT (X_code <= GB_UDT_code) ;

    if (nvals > GB_INDEX_MAX)
    {
        // problem too large
        return (ERROR (GrB_INVALID_VALUE, (LOG,
            "problem too large: nvals "GBu" exceeds "GBu,
            nvals, GB_INDEX_MAX))) ;
    }

    // check types of dup
    if (dup->xtype != dup->ztype || dup->ytype != dup->ztype)
    {
        // all 3 types of z = dup (x,y) must be the same.  dup must also be
        // associative but there is no way to check this in general.
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG, "All domains of dup "
        "operator for assembling duplicates must be identical.\n"
        "operator is: [%s] = %s ([%s],[%s])",
        dup->ztype->name, dup->name, dup->xtype->name, dup->ytype->name))) ;
    }

    if (!GB_Type_compatible (C->type, dup->ztype))
    {
        // the type of C and dup must be compatible
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
        "operator dup [%s] has type [%s]\n"
        "cannot be typecast to entries in output of type [%s]",
        dup->name, dup->ztype->name, C->type->name))) ;
    }

    // C and X must be compatible
    if (!GB_Type_code_compatible (X_code, dup->ztype->code))
    {
        // All types must be compatible with each other: C, dup, and X.
        // User-defined types are only compatible with themselves; they are not
        // compatible with any built-in type nor any other user-defined type.
        // Thus, if C, dup, or X have any user-defined type, this
        // condition requires all three types to be identical: the same
        // user-defined type.  No casting will be done in this case.
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
        "input values X of type [%s]\n"
        "cannot be typecast as input to the dup operator\n"
        "z=%s(x,y), whose input types are [%s]",
        GB_code_string (X_code), dup->name, dup->ztype->name))) ;
    }

    if (NNZ (C) > 0 || PENDING (C) || ZOMBIES (C))
    {
        // The matrix has existing entries.
        return (ERROR (GrB_OUTPUT_NOT_EMPTY, (LOG,
            "output already has existing entries"))) ;
    }

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    // allocate workspace to load and sort the index tuples:

    // ncols <= 1: iwork and kwork for (i,k) tuples, where i = I(k)
    // ncols > 1: also jwork for (j,i,k) tuples where i = I(k) and j = J (k).

    // The k value in the tuple gives the position in the original set of
    // tuples: I[k] and X[k] when ncols <= 1, and also J[k] for matrices with
    // ncols > 1.

    // The workspace iwork and jwork are allocated here but freed (or
    // transplanted) inside GB_builder.  kwork is allocated, used, and freed
    // in GB_builder.

    int64_t len = (int64_t) nvals ;

    GB_MALLOC_MEMORY (int64_t *iwork, len, sizeof (int64_t)) ;
    double memory = GBYTES (len, sizeof (int64_t)) ;
    bool ok = (iwork != NULL) ;
    int64_t *jwork = NULL ;
    if (ncols > 0)
    {
        GB_MALLOC_MEMORY (jwork, len, sizeof (int64_t)) ;
        memory += GBYTES (len, sizeof (int64_t)) ;
        ok = ok && (jwork != NULL) ;
    }

    if (!ok)
    {
        // out of memory
        GB_FREE_MEMORY (iwork, len, sizeof (int64_t)) ;
        GB_FREE_MEMORY (jwork, len, sizeof (int64_t)) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    //--------------------------------------------------------------------------
    // create the tuples to sort, and check for any invalid indices
    //--------------------------------------------------------------------------

    bool sorted = true ;

    if (ncols > 1)
    {

        //----------------------------------------------------------------------
        // C has more than one column
        //----------------------------------------------------------------------

        int64_t ilast = -1 ;
        int64_t jlast = -1 ;

        for (int64_t k = 0 ; k < len ; k++)
        {
            // get kth index from user input: (i,j)
            int64_t i = (int64_t) I [k] ;
            int64_t j = (int64_t) J [k] ;
            bool out_of_bounds = (i < 0 || i >= nrows || j < 0 || j >= ncols) ;

            if (out_of_bounds)
            {
                // invalid index
                GB_FREE_MEMORY (iwork, len, sizeof (int64_t)) ;
                GB_FREE_MEMORY (jwork, len, sizeof (int64_t)) ;
                return (ERROR (GrB_INDEX_OUT_OF_BOUNDS, (LOG,
                    "index ("GBu","GBu") out of bounds,"
                    " must be < ("GBd", "GBd")",
                    I [k], J [k], nrows, ncols))) ;
            }

            // check if the tuples are already sorted
            sorted = sorted && ((jlast < j) || (jlast == j && ilast <= i)) ;

            // copy the tuple into the work arrays to be sorted
            iwork [k] = i ;
            jwork [k] = j ;

            // log the last index seen
            ilast = i ;
            jlast = j ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C has one (or zero) columns; ignore J
        //----------------------------------------------------------------------

        int64_t ilast = -1 ;

        for (int64_t k = 0 ; k < len ; k++)
        {
            // get kth index from user input, just (i)
            int64_t i = (int64_t) I [k] ;
            bool out_of_bounds = (i < 0 || i >= nrows) ;

            if (out_of_bounds)
            {
                // invalid index
                GB_FREE_MEMORY (iwork, len, sizeof (int64_t)) ;
                GB_FREE_MEMORY (jwork, len, sizeof (int64_t)) ;
                return (ERROR (GrB_INDEX_OUT_OF_BOUNDS, (LOG,
                    "index ("GBu") out of bounds, must be < ("GBd")",
                    I [k], nrows))) ;
            }

            // check if the tuples are already sorted
            sorted = sorted && (ilast <= i) ;

            // copy the tuple into the work arrays to be sorted
            iwork [k] = i ;

            // log the last index seen
            ilast = i ;
        }
    }

    //--------------------------------------------------------------------------
    // build the matrix T, or directly build C
    //--------------------------------------------------------------------------

    // If successful, GB_builder will transplant iwork into its output matrix T
    // (or C) as the row indices T->i (or C->i) and set iwork to NULL, or if it
    // fails it has freed iwork.  In either case iwork is NULL.  It always
    // frees jwork and sets it to NULL.

    GrB_Info info ;
    if (C->type == dup->ztype)
    {
        // construct C directly; this is the fastest option
        info = GB_builder (C, &iwork, &jwork, sorted, X, len, len, dup, X_code) ;
        ASSERT (iwork == NULL) ;
        ASSERT (jwork == NULL) ;
        ASSERT (info == GrB_SUCCESS || info == GrB_OUT_OF_MEMORY) ;
        return (info) ;
    }
    else
    {
        // create T with the same type as dup->ztype
        GrB_Matrix T ;
        GB_NEW (&T, dup->ztype, C->nrows, C->ncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_FREE_MEMORY (iwork, len, sizeof (int64_t)) ;
            GB_FREE_MEMORY (jwork, len, sizeof (int64_t)) ;
            ASSERT (info == GrB_OUT_OF_MEMORY) ;
            return (info) ;
        }

        // build T from the tuples
        info = GB_builder (T, &iwork, &jwork, sorted, X, len, len, dup, X_code) ;
        ASSERT (iwork == NULL) ;
        ASSERT (jwork == NULL) ;
        if (info != GrB_SUCCESS)
        {
            // out of memory is the only error possible here
            GB_MATRIX_FREE (&T) ;
            ASSERT (info == GrB_OUT_OF_MEMORY) ;
            return (info) ;
        }

        // transplant and typecast T into C, and free T
        info = GB_Matrix_transplant (C, C->type, &T) ;
        ASSERT (T == NULL) ;
        ASSERT (info == GrB_SUCCESS || info == GrB_OUT_OF_MEMORY) ;
        return (info) ;
    }
}

