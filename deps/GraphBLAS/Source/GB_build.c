//------------------------------------------------------------------------------
// GB_build: build a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Construct a matrix C from a list of indices and values.  Any duplicate
// entries with identical indices are assembled using the binary dup operator
// provided on input.  All three types (x,y,z for z=dup(x,y)) must be
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
// Entries in [I,J,S] are first sorted in increasing order of row and column
// index via a stable sort, with ties broken by the position of the tuple in
// the [I,J,S] list.  If duplicates appear, they are assembled in the order
// they appear in the [I,J,S] input.  That is, if the same indices i and j
// appear in positions k1, k2, k3, and k4 in [I,J,S], where k1 < k2 < k3 < k4,
// then the following operations will occur in order:

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

// If nvals == 0, I_in, J_in, and S may be NULL.

#include "GB.h"

GrB_Info GB_build               // build matrix
(
    GrB_Matrix C,               // matrix to build
    const GrB_Index *I_in,      // row indices of tuples
    const GrB_Index *J_in,      // col indices of tuples
    const void *S,              // array of values of tuples
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of S array
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
    // get C
    //--------------------------------------------------------------------------

    GrB_Type ctype = C->type ;
    int64_t vlen = C->vlen ;
    int64_t vdim = C->vdim ;
    bool C_is_csc = C->is_csc ;
    int64_t nrows = GB_NROWS (C) ;
    int64_t ncols = GB_NCOLS (C) ;

    //--------------------------------------------------------------------------
    // free all content of C, but keep the C->Sauna
    //--------------------------------------------------------------------------

    // the type, dimensions, and hyper ratio are still preserved in C.
    GB_PHIX_FREE (C) ;
    ASSERT (GB_EMPTY (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (C->magic == GB_MAGIC2) ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format
    //--------------------------------------------------------------------------

    int64_t *I, *J ;
    if (C_is_csc)
    { 
        // C can be a CSC GrB_Matrix, or a GrB_Vector.
        // If C is a typecasted GrB_Vector, then J_in and J must both be NULL.
        I = (int64_t *) I_in ;  // indices in the range 0 to vlen-1
        J = (int64_t *) J_in ;  // indices in the range 0 to vdim-1
    }
    else
    { 
        // C can only be a CSR GrB_Matrix
        I = (int64_t *) J_in ;  // indices in the range 0 to vlen-1
        J = (int64_t *) I_in ;  // indices in the range 0 to vdim-1
    }

    // J contains vector names and I contains indices into those vectors.
    // The rest of this function is agnostic to the CSR/CSC format.

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    // allocate workspace to load and sort the index tuples:

    // vdim <= 1: iwork and kwork for (i,k) tuples, where i = I(k)
    // vdim > 1: also jwork for (j,i,k) tuples where i = I(k) and j = J (k).

    // The k value in the tuple gives the position in the original set of
    // tuples: I[k] and S[k] when vdim <= 1, and also J[k] for matrices with
    // vdim > 1.

    // The workspace iwork and jwork are allocated here but freed (or
    // transplanted) inside GB_builder.  kwork is allocated, used, and freed
    // in GB_builder.

    int64_t len = (int64_t) nvals ;

    GB_MALLOC_MEMORY (int64_t *iwork, len, sizeof (int64_t)) ;
    double memory = GBYTES (len, sizeof (int64_t)) ;
    bool ok = (iwork != NULL) ;
    int64_t *jwork = NULL ;
    if (vdim > 1)
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
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // create the tuples to sort, and check for any invalid indices
    //--------------------------------------------------------------------------

    bool sorted = true ;

    if (len == 0)
    { 

        // nothing to do
        ;

    }
    else if (is_matrix)
    {

        //----------------------------------------------------------------------
        // C is a matrix; check both I and J
        //----------------------------------------------------------------------

        // but if vdim <= 1, do not create jwork
        ASSERT (I != NULL) ;
        ASSERT (J != NULL) ;
        ASSERT (iwork != NULL) ;
        ASSERT ((vdim > 1) == (jwork != NULL)) ;

        int64_t ilast = -1 ;
        int64_t jlast = -1 ;

        for (int64_t k = 0 ; k < len ; k++)
        {
            // get kth index from user input: (i,j)
            int64_t i = I [k] ;
            int64_t j = J [k] ;
            bool out_of_bounds = (i < 0 || i >= vlen || j < 0 || j >= vdim) ;

            if (out_of_bounds)
            { 
                // invalid index
                GB_FREE_MEMORY (iwork, len, sizeof (int64_t)) ;
                GB_FREE_MEMORY (jwork, len, sizeof (int64_t)) ;
                int64_t row = C_is_csc ? i : j ;
                int64_t col = C_is_csc ? j : i ;
                return (GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS, (GB_LOG,
                    "index ("GBd","GBd") out of bounds,"
                    " must be < ("GBd", "GBd")", row, col, nrows, ncols))) ;
            }

            // check if the tuples are already sorted
            sorted = sorted && ((jlast < j) || (jlast == j && ilast <= i)) ;

            // copy the tuple into the work arrays to be sorted
            iwork [k] = i ;
            if (jwork != NULL)
            { 
                jwork [k] = j ;
            }

            // log the last index seen
            ilast = i ;
            jlast = j ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C is a typecasted GrB_Vector; check only I
        //----------------------------------------------------------------------

        ASSERT (I != NULL) ;

        if (ijcheck)
        {

            //------------------------------------------------------------------
            // GrB_*_build: check the user's input array I
            //------------------------------------------------------------------

            int64_t ilast = -1 ;

            for (int64_t k = 0 ; k < len ; k++)
            {
                // get kth index from user input, just (i)
                int64_t i = I [k] ;
                bool out_of_bounds = (i < 0 || i >= vlen) ;

                if (out_of_bounds)
                { 
                    // invalid index
                    GB_FREE_MEMORY (iwork, len, sizeof (int64_t)) ;
                    GB_FREE_MEMORY (jwork, len, sizeof (int64_t)) ;
                    return (GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS, (GB_LOG,
                        "index ("GBd") out of bounds, must be < ("GBd")",
                        i, vlen))) ;
                }

                // check if the tuples are already sorted
                sorted = sorted && (ilast <= i) ;

                // copy the tuple into the work arrays to be sorted
                iwork [k] = i ;

                // log the last index seen
                ilast = i ;
            }

        }
        else
        { 

            //------------------------------------------------------------------
            // GB_reduce_to_column: do not check I, assume not sorted
            //------------------------------------------------------------------

            memcpy (iwork, I, len * sizeof (int64_t)) ;
            sorted = false ;
        }
    }

    //--------------------------------------------------------------------------
    // build the matrix T and transplant it into C
    //--------------------------------------------------------------------------

    // If successful, GB_builder will transplant iwork into its output matrix T
    // as the row indices T->i and set iwork to NULL, or if it fails it has
    // freed iwork.  In either case iwork is NULL when GB_builder returns.  It
    // always frees jwork and sets it to NULL.  T can be non-hypersparse or
    // hypersparse, as determined by GB_builder; it will typically be
    // hypersparse.  Its type is the same as the z output of the z=dup(x,y)
    // operator.

    GrB_Matrix T ;
    GrB_Info info = GB_builder (&T, dup->ztype, vlen, vdim,
        C_is_csc, &iwork, &jwork, sorted, S, len, len, dup, scode, Context) ;
    ASSERT (iwork == NULL) ;
    ASSERT (jwork == NULL) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    // transplant and typecast T into C, conform C, and free T
    return (GB_transplant_conform (C, ctype, &T, Context)) ;
}

