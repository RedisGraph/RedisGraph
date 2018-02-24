//------------------------------------------------------------------------------
// GB_mx_mxArray_to_Matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
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
// GraphBLAS matrix is a deep copy and can be modified by GraphBLAS.  Otherwise,
// its pattern (A->p and A->i) are a shallow copy, and A->x is a shallow copy
// if the MATLAB matrix is 'logical' or 'double'.  A->x is always a deep copy
// for other types, since it must be typecasted from MATLAB to GraphBLAS.

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
    bool deep_copy                      // if true, return a deep copy
)
{

    GrB_Matrix A = NULL ;

    if (A_matlab == NULL)
    {
        // input is not present; this is not an error if A is an
        // optional input
        return (NULL) ;
    }

    if ((mxGetM (A_matlab) == 0) && (mxGetN (A_matlab) == 0))
    {
        // input is "[ ]", zero-by-zero.  Treat as NULL in GraphBLAS
        return (NULL) ;
    }

    const mxArray *Amatrix ;
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

    // get the size and content of the MATLAB matrix
    int64_t nrows = mxGetM (Amatrix) ;
    int64_t ncols = mxGetN (Amatrix) ;
    int64_t *Mp = (int64_t *) mxGetJc (Amatrix) ;
    int64_t *Mi = (int64_t *) mxGetIr (Amatrix) ;
    int64_t anz = Mp [ncols] ;
    void *Mx = mxGetData (Amatrix) ;

    // look for A.values
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

    ASSERT_OK (GB_check (atype_in,  "A type in",  0)) ;
    ASSERT_OK (GB_check (atype_out, "A type out", 0)) ;
    if (atype_in == NULL || atype_out == NULL)
    {
        FREE_ALL ;
        mexWarnMsgIdAndTxt ("GB:warn", "types must be numeric") ;
        return (NULL) ;
    }

    GrB_Info info ;

    // get the pattern of A
    if (deep_copy)
    {

        // create the GraphBLAS matrix
        GB_NEW (&A, atype_out, (GrB_Index) nrows, (GrB_Index) ncols,
            true, false) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            mexWarnMsgIdAndTxt ("GB:warn", "new deep matrix failed") ;
            return (NULL) ;
        }

        // A is a deep copy and can be modified by GraphBLAS
        if (!GB_Matrix_alloc (A, anz, true, NULL))
        {
            FREE_ALL ;
            mexWarnMsgIdAndTxt ("GB:warn", "out of memory") ;
            return (NULL) ;
        }

        memcpy (A->p, Mp, (ncols+1) * sizeof (int64_t)) ;
        memcpy (A->i, Mi, anz * sizeof (int64_t)) ;
    }
    else
    {
        // the GraphBLAS pattern (A->p and A->i) are pointers into the
        // MATLAB matrix and must not be modified.

        // [ create the GraphBLAS matrix, do not allocate A->p
        GB_NEW (&A, atype_out, (GrB_Index) nrows, (GrB_Index) ncols,
            false, false) ;
        if (info != GrB_SUCCESS)
        {
            FREE_ALL ;
            mexWarnMsgIdAndTxt ("GB:warn", "new shallow matrix failed") ;
            return (NULL) ;
        }

        A->p = Mp ;
        A->magic = MAGIC ;          // A->p now initialized ]
        A->i = Mi ;
        A->p_shallow = true ;
        A->i_shallow = true ;
    }

    // copy the numerical values from MATLAB to the GraphBLAS matrix
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
            A->nzmax = IMAX (anz, 1) ;
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

    // return the GraphBLAS matrix
    return (A) ;
}

