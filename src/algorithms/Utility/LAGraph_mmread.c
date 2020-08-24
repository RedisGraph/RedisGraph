//------------------------------------------------------------------------------
// LAGraph_mmread:  read a matrix from a Matrix Market file
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

// TODO: need to add a GrB_Descriptor to all LAGraph functions.

//------------------------------------------------------------------------------

// LAGraph_mmread:  read a matrix from a Matrix Market file.
// Contributed by Tim Davis, Texas A&M

// The file format used here is compatible with all variations of the Matrix
// Market "coordinate" and "array" format (http://www.nist.gov/MatrixMarket).
// The format is fully described in LAGraph/Doc/MatrixMarket.pdf, and
// summarized here (with extensions for LAGraph).

// The first line of the file starts with %%MatrixMarket, with the following
// format:

//      %%MatrixMarket matrix <fmt> <type> <storage>

//      <fmt> is one of: coordinate or array.  The former is a sparse matrix in
//      triplet form.  The latter is a dense matrix in column-major form.
//      Both formats are returned as a GrB_Matrix.

//      <type> is one of: real, complex, pattern, or integer.  The real,
//      integer, and pattern formats are returned as GrB_FP64, GrB_INT64, and
//      GrB_BOOL, respectively, but these types are modified the %GraphBLAS
//      structured comment described below.  Complex matrices are returned
//      using the LAGraph_ComplexFP64 type (which is a GraphBLAS type corresponding
//      to the ANSI C11 double complex type).

//      <storage> is one of: general, Hermitian, symmetric, or skew-symmetric.
//      The Matrix Market format is case-insensitive, so "hermitian" and
//      "Hermitian" are treated the same).

//      Not all combinations are permitted.  Only the following are meaningful:

//      (1) (coordinate or array) x (real, integer, or complex)
//          x (general, symmetric, or skew-symmetric)

//      (2) (coordinate or array) x (complex) x (Hermitian)

//      (3) (coodinate) x (pattern) x (general or symmetric)

// The second line is an optional extension to the Matrix Market format:

//      %%GraphBLAS <entrytype>

//      <entrytype> is one of the 11 built-in types (GrB_BOOL, GrB_INT8,
//      GrB_INT16, GrB_INT32, GrB_INT64, GrB_UINT8, GrB_UINT16, GrB_UINT32,
//      GrB_UINT64, GrB_FP32, GrB_FP64) or LAGraph_ComplexFP64.

//      If this second line is included, it overrides the default GraphBLAS
//      types for the Matrix Market <type> on line one of the file: real,
//      pattern, and integer.  The Matrix Market complex <type> is not
//      modified, and is always returned as LAGraph_ComplexFP64.

// Any other lines starting with "%" are treated as comments, and are ignored.
// Comments may be interspersed throughout the file.  Blank lines are ignored.
// The Matrix Market header is optional in this routine (it is not optional in
// the Matrix Market format).  If not present, the <fmt> defaults to
// coordinate, <type> defaults to real, and <storage> defaults to general.  The
// remaining lines are space delimited, and free format (one or more spaces can
// appear, and each field has arbitrary width).

// The Matrix Market file <fmt> can be coordinate or array:

//      coordinate:  for this format, the first non-comment line must appear,
//          and it must contain three integers:

//              nrows ncols nvals

//          For example, a 5-by-12 matrix with 42 entries would have:

//              5 12 42

//          Each of the remaining lines defines one entry.  The order is
//          arbitrary.  If the Matrix Market <type> is real or integer, each
//          line contains three numbers: row index, column index, and value.
//          For example, if A(3,4) is equal to 5.77, a line:

//              3 4 5.77

//          would appear in the file.  The indices in the Matrix Market are
//          1-based, so this entry becomes A(2,3) in the GrB_Matrix returned to
//          the caller.  If the <type> is pattern, then only the row and column
//          index appears.  If <type> is complex, four values appear.  If
//          A(8,4) has a real part of 6.2 and an imaginary part of -9.3, then
//          the line is:

//              8 4 6.2 -9.3

//          and since the file is 1-based but a GraphBLAS matrix is always
//          0-based, one is subtracted from the row and column indices in the
//          file, so this entry becomes A(7,3).

//      array: for this format, the first non-comment line must appear, and
//          it must contain just two integers:

//              nrows ncols

//          A 5-by-12 matrix would thus have the line

//              5 12

//          Each of the remaining lines defines one entry, in column major
//          order.  If the <type> is real or integer, this is the value of the
//          entry.  An entry if <type> of complex consists of two values, the
//          real and imaginary part.  The <type> cannot be pattern in this
//          case.

//      For both coordinate and array formats, real and complex values may use
//      the terms INF, +INF, -INF, and NAN to represent floating-point infinity
//      and NaN values.

// The <storage> token is general, symmetric, skew-symmetric, or Hermitian:

//      general: the matrix has no symmetry properties (or at least none
//          that were exploited when the file was created).

//      symmetric:  A(i,j) == A(j,i).  Only entries on or below the diagonal
//          appear in the file.  Each off-diagonal entry in the file creates
//          two entries in the GrB_Matrix that is returned.

//      skew-symmetric:  A(i,j) == -A(i,j).  There are no entries on the
//          diagonal.  Only entries below the diagonal appear in the file.
//          Each off-diagonal entry in the file creates two entries in the
//          GrB_Matrix that is returned.

//      Hermitian: square complex matrix with A(i,j) = conj (A(j,i)).
//          All entries on the diagonal are real.  Each off-diagonal entry in
//          the file creates two entries in the GrB_Matrix that is returned.

// According to the Matrix Market format, entries are always listed in
// column-major order.  This rule is follwed by LAGraph_mmwrite.  However,
// LAGraph_mmread can read the entries in any order.

// Parts of this code are from SuiteSparse/CHOLMOD/Check/cholmod_read.c, and
// are used here by permission of the author of CHOLMOD/Check (T. A. Davis).

#include "LAGraph_internal.h"

//------------------------------------------------------------------------------
// get_line
//------------------------------------------------------------------------------

// Read one line of the file, return true if successful, false if EOF.
// The string is returned in buf, converted to lower case.

static inline bool get_line
(
    FILE *f,        // file open for reading
    char *buf       // size MAXLINE+1
)
{

    // check inputs
    ASSERT (f != NULL) ;
    ASSERT (buf != NULL) ;

    // read the line from the file
    buf [0] = '\0' ;
    buf [1] = '\0' ;
    if (fgets (buf, MAXLINE, f) == NULL)
    {
        // EOF or other I/O error
        return (false) ;
    }
    buf [MAXLINE] = '\0' ;

    // convert the string to lower case
    for (int k = 0 ; k < MAXLINE && buf [k] != '\0' ; k++)
    {
        buf [k] = tolower (buf [k]) ;
    }
    return (true) ;
}

//------------------------------------------------------------------------------
// is_blank_line
//------------------------------------------------------------------------------

// returns true if buf is a blank line or comment, false otherwise.

static inline bool is_blank_line
(
    char *buf       // size MAXLINE+1, never NULL
)
{

    // check inputs
    ASSERT (buf != NULL) ;

    // check if comment line
    if (buf [0] == '%')
    {
        // line is a comment
        return (true) ;
    }

    // check if blank line
    for (int k = 0 ; k <= MAXLINE ; k++)
    {
        int c = buf [k] ;
        if (c == '\0')
        {
            // end of line
            break ;
        }
        if (!isspace (c))
        {
            // non-space character; this is not an error
            return (false) ;
        }
    }

    // line is blank
    return (true) ;
}

//------------------------------------------------------------------------------
// read_double
//------------------------------------------------------------------------------

// Read a single double value from a string.  The string may be any string
// recognized by sscanf, or inf, -inf, +inf, or nan.  The token infinity is
// also OK instead of inf (only the first 3 letters of inf* or nan* are
// significant, and the rest are ignored).

static inline bool read_double      // true if successful, false if failure
(
    char *p,        // string containing the value
    double *rval    // value to read in
)
{
    while (*p && isspace (*p)) p++ ;   // skip any spaces

    if ((strncmp (p, "inf", 3) == 0) || (strncmp (p, "+inf", 4) == 0))
    {
        (*rval) = INFINITY ;
    }
    else if (strncmp (p, "-inf", 4) == 0)
    {
        (*rval) = -INFINITY ;
    }
    else if (strncmp (p, "nan", 3) == 0)
    {
        (*rval) = NAN ;
    }
    else
    {
        if (sscanf (p, "%lg", rval) != 1)
        {
            // bad file format, EOF, or other I/O error
            return (false) ;
        }
    }
    return (true) ;
}

//------------------------------------------------------------------------------
// read_entry
//------------------------------------------------------------------------------

static inline bool read_entry   // true if successful, false if failure
(
    char *p,        // string containing the value
    GrB_Type type,  // type of value to read
    bool pattern,   // if true, then the value is 1
    char *x         // value read in, a pointer to space of size of the type
)
{

    int64_t ival = 1 ;
    double rval = 1, zval = 0 ;

    while (*p && isspace (*p)) p++ ;   // skip any spaces

    // printf ("read entry [%s]: ", p) ;

    if (type == GrB_BOOL)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < 0 || ival > 1) return (false) ;
        bool *result = (bool *) x ;
        result [0] = (bool) ival ;
    }
    else if (type == GrB_INT8)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < INT8_MIN || ival > INT8_MAX) return (false) ;
        int8_t *result = (int8_t *) x ;
        result [0] = (int8_t) ival ;
    }
    else if (type == GrB_INT16)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < INT16_MIN || ival > INT16_MAX) return (false) ;
        int16_t *result = (int16_t *) x ;
        result [0] = (int16_t) ival ;
    }
    else if (type == GrB_INT32)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < INT32_MIN || ival > INT32_MAX) return (false) ;
        int32_t *result = (int32_t *) x ;
        result [0] = (int32_t) ival ;
    }
    else if (type == GrB_INT64)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        int64_t *result = (int64_t *) x ;
        result [0] = (int64_t) ival ;
    }
    else if (type == GrB_UINT8)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < 0 || ival > UINT8_MAX) return (false) ;
        uint8_t *result = (uint8_t *) x ;
        result [0] = (uint8_t) ival ;
    }
    else if (type == GrB_UINT16)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < 0 || ival > UINT16_MAX) return (false) ;
        uint16_t *result = (uint16_t *) x ;
        result [0] = (uint16_t) ival ;
    }
    else if (type == GrB_UINT32)
    {
        if (!pattern && sscanf (p, "%" SCNd64, &ival) != 1) return (false) ;
        // printf ("%" PRId64 "\n", ival) ;
        if (ival < 0 || ival > UINT32_MAX) return (false) ;
        uint32_t *result = (uint32_t *) x ;
        result [0] = (uint32_t) ival ;
    }
    else if (type == GrB_UINT64)
    {
        uint64_t uval = 1 ;
        if (!pattern && sscanf (p, "%" SCNu64, &uval) != 1) return (false) ;
        // printf ("%" PRIu64 "\n", uval) ;
        uint64_t *result = (uint64_t *) x ;
        result [0] = (uint64_t) uval ;
    }
    else if (type == GrB_FP32)
    {
        if (!pattern && !read_double (p, &rval)) return (false) ;
        float *result = (float *) x ;
        result [0] = (float) rval ;
    }
    else if (type == GrB_FP64)
    {
        if (!pattern && !read_double (p, &rval)) return (false) ;
        // printf ("%g\n", rval) ;
        double *result = (double *) x ;
        result [0] = rval ;
    }
    else if (type == LAGraph_ComplexFP64)
    {
        if (!pattern && !read_double (p, &rval)) return (false) ;
        // printf ("(%g, ", rval) ;
        while (*p && !isspace (*p)) p++ ;   // skip real part
        if (!pattern && !read_double (p, &zval)) return (false) ;
        // printf ("%g)\n", zval) ;
        double *result = (double *) x ;
        result [0] = rval ;     // real part
        result [1] = zval ;     // imaginary part
    }
    else
    {
        // type not supported
        printf ("LAGraph_mmread: read_entry: type not supported\n") ;
        return (false) ;
    }

    return (true) ;
}

//------------------------------------------------------------------------------
// negate_scalar: negate a scalar value
//------------------------------------------------------------------------------

// negate the scalar x.  Do nothing for bool or uint*.

static inline void negate_scalar
(
    GrB_Type type,
    void *x
)
{

    if (type == GrB_INT8)
    {
        int8_t *value = (int8_t *) x ;
        (*value) = - (*value) ;
    }
    else if (type == GrB_INT16)
    {
        int16_t *value = (int16_t *) x ;
        (*value) = - (*value) ;
    }
    else if (type == GrB_INT32)
    {
        int32_t *value = (int32_t *) x ;
        (*value) = - (*value) ;
    }
    else if (type == GrB_INT64)
    {
        int64_t *value = (int64_t *) x ;
        (*value) = - (*value) ;
    }
    else if (type == GrB_FP32)
    {
        float *value = (float *) x ;
        (*value) = - (*value) ;
    }
    else if (type == GrB_FP64)
    {
        double *value = (double *) x ;
        (*value) = - (*value) ;
    }
    else if (type == LAGraph_ComplexFP64)
    {
        double complex *value = (double complex *) x ;
        (*value) = - (*value) ;
    }
}

//------------------------------------------------------------------------------
// set_value
//------------------------------------------------------------------------------

// A(i,j) = x using GrB_Matrix_setElement_<type>.  No typecasting is done.

static inline GrB_Info set_value
(
    GrB_Matrix A,
    GrB_Type type,
    GrB_Index i,
    GrB_Index j,
    char *x
)
{

    if (type == GrB_BOOL)
    {
        bool *value = (bool *) x ;
        return (GrB_Matrix_setElement_BOOL (A, *value, i, j)) ;
    }
    else if (type == GrB_INT8)
    {
        int8_t *value = (int8_t *) x ;
        return (GrB_Matrix_setElement_INT8 (A, *value, i, j)) ;
    }
    else if (type == GrB_INT16)
    {
        int16_t *value = (int16_t *) x ;
        return (GrB_Matrix_setElement_INT16 (A, *value, i, j)) ;
    }
    else if (type == GrB_INT32)
    {
        int32_t *value = (int32_t *) x ;
        return (GrB_Matrix_setElement_INT32 (A, *value, i, j)) ;
    }
    else if (type == GrB_INT64)
    {
        int64_t *value = (int64_t *) x ;
        return (GrB_Matrix_setElement_INT64 (A, *value, i, j)) ;
    }
    else if (type == GrB_UINT8)
    {
        uint8_t *value = (uint8_t *) x ;
        return (GrB_Matrix_setElement_UINT8 (A, *value, i, j)) ;
    }
    else if (type == GrB_UINT16)
    {
        uint16_t *value = (uint16_t *) x ;
        return (GrB_Matrix_setElement_UINT16 (A, *value, i, j)) ;
    }
    else if (type == GrB_UINT32)
    {
        uint32_t *value = (uint32_t *) x ;
        return (GrB_Matrix_setElement_UINT32 (A, *value, i, j)) ;
    }
    else if (type == GrB_UINT64)
    {
        uint64_t *value = (uint64_t *) x ;
        return (GrB_Matrix_setElement_UINT64 (A, *value, i, j)) ;
    }
    else if (type == GrB_FP32)
    {
        float *value = (float *) x ;
        return (GrB_Matrix_setElement_FP32 (A, *value, i, j)) ;
    }
    else if (type == GrB_FP64)
    {
        double *value = (double *) x ;
        return (GrB_Matrix_setElement_FP64 (A, *value, i, j)) ;
    }
    else if (type == LAGraph_ComplexFP64)
    {
        return (GrB_Matrix_setElement_UDT (A, x, i, j)) ;
    }
    else
    {
        // type not supported
        return (GrB_PANIC) ;
    }
}

//------------------------------------------------------------------------------
// LAGraph_mmread
//------------------------------------------------------------------------------

GrB_Info LAGraph_mmread
(
    GrB_Matrix *A,      // handle of matrix to create
    FILE *f             // file to read from, already open
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (A == NULL || f == NULL)
    {
        // input arguments invalid
        printf ("LAGraph_mmread: bad args\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // set the default properties
    //--------------------------------------------------------------------------

    MM_fmt_enum     MM_fmt     = MM_coordinate ;
    MM_type_enum    MM_type    = MM_real ;
    MM_storage_enum MM_storage = MM_general ;
    GrB_Type type = GrB_FP64 ;
    GrB_Index nrows = 0 ;
    GrB_Index ncols = 0 ;
    GrB_Index nvals = 0 ;

    //--------------------------------------------------------------------------
    // read the Matrix Market header
    //--------------------------------------------------------------------------

    // Read the header.  This consists of zero or more comment lines (blank, or
    // starting with a "%" in the first column), followed by a single data line
    // containing two or three numerical values.  The first line is normally:
    //
    //          %%MatrixMarket matrix <fmt> <type> <storage>
    //
    // but this is optional.  The 2nd line is also optional (the %%MatrixMarket
    // line is required for this 2nd line to be recognized):
    //
    //          %%GraphBLAS <graphblastype>
    //
    // If the %%MatrixMarket line is not present, then the <fmt> <type> and
    // <storage> are implicit.  If the first data line contains 3 items,
    // then the implicit header is:
    //
    //          %%MatrixMarket matrix coordinate real general
    //          %%GraphBLAS GrB_FP64
    //
    // If the first data line contains 2 items (nrows ncols), then the implicit
    // header is:
    //
    //          %%MatrixMarket matrix array real general
    //          %%GraphBLAS GrB_FP64
    //
    // The implicit header is an extension of the Matrix Market format.

    char buf [MAXLINE+1] ;

    bool got_mm_header = false ;

    for (int64_t line = 1 ; get_line (f, buf) ; line++)
    {

        //----------------------------------------------------------------------
        // parse the line
        //----------------------------------------------------------------------

        if ((line == 1) && (strncmp (buf, "%%matrixmarket", 14) == 0))
        {

            //------------------------------------------------------------------
            // read a Matrix Market header
            //------------------------------------------------------------------

            //  %%MatrixMarket matrix <fmt> <type> <storage>
            //  if present, it must be the first line in the file.

            got_mm_header = true ;
            char *p = buf + 14 ;

            //------------------------------------------------------------------
            // get "matrix" token and discard it
            //------------------------------------------------------------------

            while (*p && isspace (*p)) p++ ;        // skip any leading spaces

            // printf ("header now [%s]\n", p) ;
            // printf ("compare %d\n", (strncmp (p, "matrix", 6))) ;

            if (strncmp (p, "matrix", 6) != 0)
            {
                // invalid Matrix Market object
                printf ("LAGraph_mmread: bad object\n") ;
                return (GrB_INVALID_VALUE) ;
            }
            p += 6 ;                                // skip past token "matrix"

            //------------------------------------------------------------------
            // get the fmt token
            //------------------------------------------------------------------

            while (*p && isspace (*p)) p++ ;        // skip any leading spaces

            if (strncmp (p, "coordinate", 10) == 0)
            {
                MM_fmt = MM_coordinate ;
                p += 10 ;
            }
            else if (strncmp (p, "array", 5) == 0)
            {
                MM_fmt = MM_array ;
                p += 5 ;
            }
            else
            {
                // invalid Matrix Market format
                printf ("LAGraph_mmread: bad format\n") ;
                return (GrB_INVALID_VALUE) ;
            }

            //------------------------------------------------------------------
            // get the Matrix Market type token
            //------------------------------------------------------------------

            while (*p && isspace (*p)) p++ ;        // skip any leading spaces

            if (strncmp (p, "real", 4) == 0)
            {
                MM_type = MM_real ;
                type = GrB_FP64 ;
                p += 4 ;
            }
            else if (strncmp (p, "integer", 7) == 0)
            {
                MM_type = MM_integer ;
                type = GrB_INT64 ;
                p += 7 ;
            }
            else if (strncmp (p, "complex", 7) == 0)
            {
                MM_type = MM_complex ;
                type = LAGraph_ComplexFP64 ;
                p += 7 ;
            }
            else if (strncmp (p, "pattern", 7) == 0)
            {
                MM_type = MM_pattern ;
                type = GrB_BOOL ;
                p += 7 ;
            }
            else
            {
                // invalid Matrix Market type
                printf ("LAGraph_mmread: bad type\n") ;
                return (GrB_INVALID_VALUE) ;
            }

            //------------------------------------------------------------------
            // get the storage token
            //------------------------------------------------------------------

            while (*p && isspace (*p)) p++ ;        // skip any leading spaces

            if (strncmp (p, "general", 7) == 0)
            {
                MM_storage = MM_general ;
            }
            else if (strncmp (p, "symmetric", 9) == 0)
            {
                MM_storage = MM_symmetric ;
            }
            else if (strncmp (p, "skew-symmetric", 14) == 0)
            {
                MM_storage = MM_skew_symmetric ;
            }
            else if (strncmp (p, "hermitian", 9) == 0)
            {
                MM_storage = MM_hermitian ;
            }
            else
            {
                // invalid Matrix Market storage
                printf ("LAGraph_mmread: bad type\n") ;
                return (GrB_INVALID_VALUE) ;
            }

            //------------------------------------------------------------------
            // ensure the combinations are valid
            //------------------------------------------------------------------

            if (MM_type == MM_pattern)
            {
                // (coodinate) x (pattern) x (general or symmetric)
                if (! (MM_fmt == MM_coordinate &&
                        (MM_storage == MM_general ||
                         MM_storage == MM_symmetric)))
                {
                    // invalid combination
                    printf ("LAGraph_mmread: bad pattern combo\n") ;
                    return (GrB_INVALID_VALUE) ;
                }
            }

            if (MM_storage == MM_hermitian)
            {
                // (coordinate or array) x (complex) x (Hermitian)
                if (! (MM_type == MM_complex))
                {
                    // invalid combination
                    printf ("LAGraph_mmread: bad complex combo\n") ;
                    return (GrB_INVALID_VALUE) ;
                }
            }

        }
        else if (got_mm_header && (line == 2)
                 && (strncmp (buf, "%%graphblas", 11) == 0))
        {

            // -----------------------------------------------------------------
            // %%GraphBLAS <entrytype>
            // -----------------------------------------------------------------

            // This must appear as the 2nd line in the file, after the
            // %%MatrixMarket header (which is required in this case; otherwise
            // the %%GraphBLAS line is treated as a pure comment and the
            // <entrytype> is ignored).

            char *p = buf + 11 ;

            while (*p && isspace (*p)) p++ ;        // skip any leading spaces

            // <entrytype> is one of the 11 built-in types (GrB_BOOL, GrB_INT8,
            // GrB_INT16, GrB_INT32, GrB_INT64, GrB_UINT8, GrB_UINT16,
            // GrB_UINT32, GrB_UINT64, GrB_FP32, GrB_FP64) or LAGraph_ComplexFP64.

            // printf ("for type: compare [%s]\n", p) ;

            if (strncmp (p, "grb_bool", 8) == 0)
            {
                type = GrB_BOOL ;
            }
            else if (strncmp (p, "grb_int8", 8) == 0)
            {
                type = GrB_INT8 ;
            }
            else if (strncmp (p, "grb_int16", 9) == 0)
            {
                type = GrB_INT16 ;
            }
            else if (strncmp (p, "grb_int32", 9) == 0)
            {
                type = GrB_INT32 ;
            }
            else if (strncmp (p, "grb_int64", 9) == 0)
            {
                type = GrB_INT64 ;
            }
            else if (strncmp (p, "grb_uint8", 9) == 0)
            {
                type = GrB_UINT8 ;
            }
            else if (strncmp (p, "grb_uint16", 10) == 0)
            {
                type = GrB_UINT16 ;
            }
            else if (strncmp (p, "grb_uint32", 10) == 0)
            {
                type = GrB_UINT32 ;
            }
            else if (strncmp (p, "grb_uint64", 10) == 0)
            {
                type = GrB_UINT64 ;
            }
            else if (strncmp (p, "grb_fp32", 8) == 0)
            {
                type = GrB_FP32 ;
            }
            else if (strncmp (p, "grb_fp64", 8) == 0)
            {
                type = GrB_FP64 ;
            }
            else if (strncmp (p, "lagraph_complex", 15) == 0)
            {
                type = LAGraph_ComplexFP64 ;
            }
            else
            {
                // type not supported
                printf ("LAGraph_mmread: type not supported\n") ;
                return (GrB_INVALID_VALUE) ;
            }

        }
        else if (is_blank_line (buf))
        {

            // -----------------------------------------------------------------
            // blank line or comment line
            // -----------------------------------------------------------------

            continue ;

        }
        else
        {

            // -----------------------------------------------------------------
            // read the first data line and return
            // -----------------------------------------------------------------

            // format: [nrows ncols nvals] or just [nrows ncols]

            int nitems = sscanf (buf, "%" SCNu64 " %" SCNu64 " %" SCNu64,
                &nrows, &ncols, &nvals) ;

            if (nitems == 2)
            {
                // a dense matrix
                if (!got_mm_header)
                {
                    // if no header, treat it as if it were
                    // %%MatrixMarket matrix array real general
                    MM_fmt = MM_array ;
                    MM_type = MM_real ;
                    MM_storage = MM_general ;
                    type = GrB_FP64 ;
                }
                nvals = nrows * ncols ;
            }
            else if (nitems == 3)
            {
                // a sparse matrix
                if (!got_mm_header)
                {
                    // if no header, treat it as if it were
                    // %%MatrixMarket matrix coordinate real general
                    MM_fmt = MM_coordinate ;
                    MM_type = MM_real ;
                    MM_storage = MM_general ;
                    type = GrB_FP64 ;
                }
            }
            else
            {
                // wrong number of items in first data line
                printf ("LAGraph_mmread: bad 1st line\n") ;
                return (GrB_INVALID_VALUE) ;
            }

            if (nrows != ncols)
            {
                if (! (MM_storage == MM_general))
                {
                    // a rectangular matrix must be in the general storage
                    printf ("LAGraph_mmread: bad rectangular\n") ;
                    return (GrB_INVALID_VALUE) ;
                }
            }

            //------------------------------------------------------------------
            // header has been read in
            //------------------------------------------------------------------

            break ;
        }
    }

    //--------------------------------------------------------------------------
    // create the matrix
    //--------------------------------------------------------------------------

    GrB_Info info = GrB_Matrix_new (A, type, nrows, ncols) ;
    if (info != GrB_SUCCESS)
    {
        // failed to construct matrix
        // printf ("mmread: failed to construct A\n") ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // quick return for empty matrix
    //--------------------------------------------------------------------------

    if (nrows == 0 || nrows == 0 || nvals == 0)
    {
        // success: return an empty matrix.  This is not an error.
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // read the entries
    //--------------------------------------------------------------------------

    for (int64_t k = 0 ; k < nvals ; k++)
    {

        //----------------------------------------------------------------------
        // get the next triplet, skipping blank lines and comment lines
        //----------------------------------------------------------------------

        GrB_Index i, j ;
        char x [MAXLINE] ;

        for ( ; ; )
        {

            //------------------------------------------------------------------
            // read the file until finding the next triplet
            //------------------------------------------------------------------

            if (!get_line (f, buf))
            {
                // premature end of file - not enough triplets read in
                GrB_free (A) ;
                printf ("LAGraph:mmread: premature EOF\n") ;
                return (GrB_INVALID_VALUE) ;
            }
            if (is_blank_line (buf))
            {
                // blank line or comment
                continue ;
            }

            //------------------------------------------------------------------
            // get the row and column index
            //------------------------------------------------------------------

            char *p ;
            if (MM_fmt == MM_array)
            {
                // array format, column major order
                i = k % nrows ;
                j = k / nrows ;
                p = buf ;
                // printf ("array now [%s]\n", p) ;
            }
            else
            {
                // coordinate format; read the row and column index
                p = buf ;
                if (sscanf (p, "%" SCNu64 " %" SCNu64, &i, &j) != 2)
                {
                    // EOF or other I/O error
                    GrB_free (A) ;
                    printf ("LAGraph_mmread: I/O error on indices\n") ;
                    return (GrB_INVALID_VALUE) ;
                }
                // convert from 1-based to 0-based.
                i-- ;
                j-- ;
                // printf ("got (%g,%g)\n", (double) i, (double) j) ;
                // advance p to the 3rd token to get the value of the entry
                while (*p &&  isspace (*p)) p++ ;   // skip any leading spaces
                while (*p && !isspace (*p)) p++ ;   // skip nrows
                while (*p &&  isspace (*p)) p++ ;   // skip any spaces
                while (*p && !isspace (*p)) p++ ;   // skip nrows
                // printf ("now [%s]\n", p) ;
            }

            //------------------------------------------------------------------
            // read the value of the entry
            //------------------------------------------------------------------

            while (*p && isspace (*p)) p++ ;        // skip any spaces

            if (!read_entry (p, type, MM_type == MM_pattern, x))
            {
                // EOF or other I/O error, or value of entry out of range
                GrB_free (A) ;
                printf ("LAGraph_mmread: I/O error on value\n") ;
                return (GrB_INVALID_VALUE) ;
            }

            //------------------------------------------------------------------
            // set the value in the matrix
            //------------------------------------------------------------------

            info = set_value (*A, type, i, j, x) ;
            if (info != GrB_SUCCESS)
            {
                // unable to set element: invalid indices, or out of memory
                printf ("mmread: unable to set element\n") ;
                GrB_free (A) ;
                return (info) ;
            }

            // GxB_fprint (*A, GxB_COMPLETE, stdout) ;

            //------------------------------------------------------------------
            // also set the A(j,i) entry, if symmetric
            //------------------------------------------------------------------

            if (i != j && MM_storage != MM_general)
            {
                if (MM_storage == MM_symmetric)
                {
                    info = set_value (*A, type, j, i, x) ;
                }
                else if (MM_storage == MM_skew_symmetric)
                {
                    negate_scalar (type, x) ;
                    info = set_value (*A, type, j, i, x) ;
                }
                else if (MM_storage == MM_hermitian)
                {
                    double complex *value = (double complex *) x ;
                    (*value) = conj (*value) ;
                    info = set_value (*A, type, j, i, x) ;
                }
                if (info != GrB_SUCCESS)
                {
                    // unable to set element: invalid indices, or out of memory
                    GrB_free (A) ;
                    // printf ("mmread: unable to set symmetric element\n") ;
                    return (info) ;
                }
            }

            // one more entry has been read in
            break ;
        }
    }

    return (GrB_SUCCESS) ;
}

