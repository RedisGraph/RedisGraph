//------------------------------------------------------------------------------
// GB_mx_mxArray_to_Matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Convert a MATLAB sparse matrix or struct to a GraphBLAS sparse matrix.  The
// mxArray is either a struct containing two terms: a sparse matrix or vector,
// and class (a string, "logical", "double", etc), or it is just a plain sparse
// matrix.  The matrix must be sparse and real, either logical or double.  If
// A.class is present, it is used to typecast the MATLAB matrix into the
// corresponding type in GraphBLAS.

// That is:
// A = sparse (...) ;   % a sparse double or logical GraphBLAS matrix

// A.matrix = A ; A.class = 'int8' ; Represents  A MATLAB sparse matrix that
// represents a GraphBLAS int8 matrix.  On input, the MATLAB sparse matrix is
// typecasted.

// The MATLAB matrix or struct is not modified.  If deep_copy is true, the
// GraphBLAS matrix is always a deep copy and can be modified by GraphBLAS.
// Otherwise, its pattern (A->p, A->h, and A->i) may be a shallow copy, and
// A->x is a shallow copy if the MATLAB matrix is 'logical' or 'double'.  A->x
// is always a deep copy for other types, since it must be typecasted from
// MATLAB to GraphBLAS.

// Like GB_mx_Matrix_to_mxArray, this could be done using only user-callable
// GraphBLAS functions, but the method used here is faster.

#include "GB_mex.h"

#define FREE_ALL            \
{                           \
    GB_MATRIX_FREE (&A) ;   \
}

GrB_Matrix GB_mx_mxArray_to_Matrix     // returns GraphBLAS version of A
(
    const mxArray *A_matlab,            // MATLAB version of A
    const char *name,                   // name of the argument
    bool deep_copy,                     // if true, return a deep copy
    const bool empty    // if false, 0-by-0 matrices are returned as NULL.
                        // if true, a 0-by-0 matrix is returned.
)
{

    //--------------------------------------------------------------------------
    // check for empty matrix
    //--------------------------------------------------------------------------

    GB_WHERE ("mxArray_to_Matrix") ;

    GrB_Matrix A = NULL ;

    if (A_matlab == NULL)
    {
        // input is not present; this is not an error if A is an
        // optional input
        return (NULL) ;
    }

    if ((mxGetM (A_matlab) == 0) && (mxGetN (A_matlab) == 0))
    {
        // input is "[ ]", zero-by-zero.
        if (empty)
        {
            // treat as a sparse 0-by-0 matrix, not NULL
            GrB_Matrix_new (&A, GrB_FP64, 0, 0) ;
            ASSERT_OK (GB_check (A, "got A = [ ] from MATLAB", GB0)) ;
            return (A) ;
        }
        else
        {
            // Treat as NULL in GraphBLAS.  Useful for mask matrices
            return (NULL) ;
        }
    }

    //--------------------------------------------------------------------------
    // get the matrix
    //--------------------------------------------------------------------------

    const mxArray *Amatrix = NULL ;
    mxClassID aclass_in, aclass_out ;

    if (mxIsStruct (A_matlab))
    {
        // look for A.matrix
        int fieldnumber = mxGetFieldNumber (A_matlab, "matrix") ;
        if (fieldnumber >= 0)
        {
            Amatrix = mxGetFieldByNumber (A_matlab, 0, fieldnumber) ;
        }
        else
        {
            // A.matrix not present, try A.vector
            fieldnumber = mxGetFieldNumber (A_matlab, "vector") ;
            if (fieldnumber < 0)
            {
                FREE_ALL ;
                mexWarnMsgIdAndTxt ("GB:warn", "invalid matrix/vector struct") ;
                return (NULL) ;
            }
            Amatrix = mxGetFieldByNumber (A_matlab, 0, fieldnumber) ;
            if (mxGetN (Amatrix) != 1)
            {
                FREE_ALL ;
                mexWarnMsgIdAndTxt ("GB:warn", "vector must be n-by-1") ;
                return (NULL) ;
            }
        }

        // get the class: defaults to class (Amatrix)
        ASSERT (Amatrix != NULL) ;

        aclass_out = aclass_in = mxGetClassID (Amatrix) ;
        fieldnumber = mxGetFieldNumber (A_matlab, "class") ;
        if (fieldnumber >= 0)
        {
            aclass_out = GB_mx_string_to_classID (aclass_in,
                mxGetFieldByNumber (A_matlab, 0, fieldnumber)) ;
        }
    }
    else
    {
        // just a matrix; use the class as-is
        Amatrix = A_matlab ;
        aclass_out = aclass_in = mxGetClassID (Amatrix) ;
    }

    if (!mxIsSparse (Amatrix))
    {
        FREE_ALL ;
        mexWarnMsgIdAndTxt ("GB:warn", "input matrix must be sparse") ;
        return (NULL) ;
    }

    //--------------------------------------------------------------------------
    // get the matrix type
    //--------------------------------------------------------------------------

    GrB_Type atype_in, atype_out ;

    if (mxIsComplex (Amatrix))
    {
        // use the user-defined Complex type
        atype_in  = Complex ;
        atype_out = Complex ;
        deep_copy = true ;
    }
    else
    {
        // get the GraphBLAS types
        atype_in  = GB_mx_classID_to_Type (aclass_in) ;
        atype_out = GB_mx_classID_to_Type (aclass_out) ;
    }

    //--------------------------------------------------------------------------
    // get the size and content of the MATLAB matrix
    //--------------------------------------------------------------------------

    int64_t nrows = mxGetM (Amatrix) ;
    int64_t ncols = mxGetN (Amatrix) ;
    int64_t *Mp = (int64_t *) mxGetJc (Amatrix) ;
    int64_t *Mi = (int64_t *) mxGetIr (Amatrix) ;
    int64_t anz = Mp [ncols] ;
    void *Mx = mxGetData (Amatrix) ;

    //--------------------------------------------------------------------------
    // look for A.values
    //--------------------------------------------------------------------------

    if (mxIsStruct (A_matlab))
    {
        int fieldnumber = mxGetFieldNumber (A_matlab, "values") ;
        if (fieldnumber >= 0)
        {
            mxArray *values = mxGetFieldByNumber (A_matlab, 0, fieldnumber) ;
            if (mxGetNumberOfElements (values) >= anz)
            {
                Mx = mxGetData (values) ;
                aclass_in = mxGetClassID (values) ;
                atype_in = GB_mx_classID_to_Type (aclass_in) ;
            }
        }
    }

    ASSERT_OK (GB_check (atype_in,  "A type in", GB0)) ;
    ASSERT_OK (GB_check (atype_out, "A type out", GB0)) ;

    if (atype_in == NULL || atype_out == NULL)
    {
        FREE_ALL ;
        mexWarnMsgIdAndTxt ("GB:warn", "types must be numeric") ;
        return (NULL) ;
    }

    GrB_Info info ;

    // MATLAB matrices are non-hypersparse CSC
    bool is_csc = true ;
    bool is_hyper = false ;

    //--------------------------------------------------------------------------
    // get the pattern of A
    //--------------------------------------------------------------------------

    if (deep_copy)
    {

        // create the GraphBLAS matrix
        GB_NEW (&A, atype_out, (GrB_Index) nrows, (GrB_Index) ncols,
            GB_Ap_calloc, is_csc, is_hyper, GB_HYPER_DEFAULT, 0) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            mexWarnMsgIdAndTxt ("GB:warn", "new deep matrix failed") ;
            return (NULL) ;
        }

        // A is a deep copy and can be modified by GraphBLAS
        info = GB_ix_alloc (A, anz, true, Context) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            mexWarnMsgIdAndTxt ("GB:warn", "out of memory") ;
            return (NULL) ;
        }

        memcpy (A->p, Mp, (ncols+1) * sizeof (int64_t)) ;
        memcpy (A->i, Mi, anz * sizeof (int64_t)) ;
        A->magic = GB_MAGIC ;

    }
    else
    {

        // the GraphBLAS pattern (A->p and A->i) are pointers into the
        // MATLAB matrix and must not be modified.

        // [ create the GraphBLAS matrix, do not allocate A->p
        GB_NEW (&A, atype_out, (GrB_Index) nrows, (GrB_Index) ncols,
            GB_Ap_null, is_csc, is_hyper, GB_HYPER_DEFAULT, 0) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            mexWarnMsgIdAndTxt ("GB:warn", "new shallow matrix failed") ;
            return (NULL) ;
        }

        A->p = Mp ;
        A->magic = GB_MAGIC ;       // A->p now initialized ]
        A->i = Mi ;
        A->p_shallow = true ;
        A->h_shallow = false ;      // A->h is NULL
        A->i_shallow = true ;
    }

    //--------------------------------------------------------------------------
    // copy the numerical values from MATLAB to the GraphBLAS matrix
    //--------------------------------------------------------------------------

    A->x_shallow = (!deep_copy &&
           ((atype_out->code == GB_BOOL_code ||
             atype_out->code == GB_FP64_code)
         && (atype_out->code == atype_in->code))) ;

    if (A->x_shallow)
    {
        // the MATLAB matrix and GraphBLAS matrix have the same type;
        // (logical or double), and a deep copy is not requested.
        // Just make a shallow copy.
        A->nzmax = mxGetNzmax (Amatrix) ;
        A->x = Mx ;
    }
    else
    {
        if (!deep_copy)
        {
            // allocate new space for the GraphBLAS values
            A->nzmax = GB_IMAX (anz, 1) ;
            GB_MALLOC_MEMORY (A->x, A->nzmax, atype_out->size) ;
            if (A->x == NULL)
            {
                FREE_ALL ;
                mexWarnMsgIdAndTxt ("GB:warn", "out of memory") ;
                return (NULL) ;
            }
        }
        if (atype_in == Complex)
        {
            // copy the real part (Mx) and imaginary part (Mz) into A->x
            GB_mx_complex_merge (anz, (double *) (A->x), Amatrix) ;
        }
        else
        {
            GB_cast_array (A->x, atype_out->code, Mx, atype_in->code, anz) ;
        }
    }

    //--------------------------------------------------------------------------
    // look for CSR/CSC and hyper/non-hyper format
    //--------------------------------------------------------------------------

    bool has_hyper_ratio = false ;
    double hyper_ratio = GB_HYPER_DEFAULT ;

    if (mxIsStruct (A_matlab))
    {
        // look for A.is_csc
        int fieldnumber = mxGetFieldNumber (A_matlab, "is_csc") ;
        if (fieldnumber >= 0)
        {
            is_csc = mxGetScalar (mxGetFieldByNumber (A_matlab,
                0, fieldnumber)) ;
        }

        // look for A.is_hyper (ignored if hyper_ratio present)
        fieldnumber = mxGetFieldNumber (A_matlab, "is_hyper") ;
        if (fieldnumber >= 0)
        {
            is_hyper = mxGetScalar (mxGetFieldByNumber (A_matlab,
                0, fieldnumber)) ;
        }

        // look for A.hyper_ratio
        fieldnumber = mxGetFieldNumber (A_matlab, "hyper_ratio") ;
        if (fieldnumber >= 0)
        {
            has_hyper_ratio = true ;
            hyper_ratio = mxGetScalar (mxGetFieldByNumber (A_matlab,
                0, fieldnumber)) ;
        }
    }

    //--------------------------------------------------------------------------
    // count the # of non-empty vectors in A
    //--------------------------------------------------------------------------

    A->nvec_nonempty = GB_nvec_nonempty (A) ;

    ASSERT_OK (GB_check (A, "got natural A from MATLAB", GB0)) ;
    ASSERT (!A->is_hyper) ;

    //--------------------------------------------------------------------------
    // convert to CSR if requested
    //--------------------------------------------------------------------------

    int64_t nrows_old = GB_NROWS (A) ;
    int64_t ncols_old = GB_NCOLS (A) ;

    if (!is_csc)
    {
        // this might convert A to hypersparse
        GxB_set (A, GxB_FORMAT, GxB_BY_ROW) ;
        // so convert it back; hypersparsity is defined below
        GB_to_nonhyper (A, Context) ;
        ASSERT (!A->is_csc) ;
    }

    ASSERT_OK (GB_check (A, "conformed from MATLAB", GB0)) ;
    ASSERT (!A->is_hyper) ;
    ASSERT (A->is_csc == is_csc) ;

    //--------------------------------------------------------------------------
    // convert to hypersparse or set hypersparse ratio, if requested
    //--------------------------------------------------------------------------

    if (has_hyper_ratio)
    {
        // this sets the hyper_ratio and then conforms the matrix to its
        // desired hypersparsity.  It may stay non-hypersparse.
        GxB_set (A, GxB_HYPER, hyper_ratio) ;
    }
    else if (is_hyper)
    {
        // this forces the matrix to be always hypersparse
        GxB_set (A, GxB_HYPER, GxB_ALWAYS_HYPER) ;
        if (A->vdim > 1)
        {
            ASSERT (A->is_hyper == is_hyper) ;
        }
        else
        {
            // column vectors are never hypersparse
            ASSERT (!A->is_hyper) ;
        }
    }

    ASSERT_OK (GB_check (A, "final hyper/nonhyper", GB0)) ;
    ASSERT (A->is_csc == is_csc) ;
    ASSERT (nrows_old == GB_NROWS (A)) ;
    ASSERT (ncols_old == GB_NCOLS (A)) ;

    //--------------------------------------------------------------------------
    // return the GraphBLAS matrix
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A, "got A from MATLAB", GB0)) ;
    return (A) ;
}

