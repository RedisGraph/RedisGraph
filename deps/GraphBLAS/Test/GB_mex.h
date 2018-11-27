//------------------------------------------------------------------------------
// GB_mex.h: definitions for the MATLAB interface to GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_MEXH
#define GB_MEXH

#define GB_PANIC mexErrMsgTxt ("panic") ;

#include "GB.h"
#include "demos.h"
#undef OK
#include "usercomplex.h"
#include "mex.h"
#include "matrix.h"

#define PARGIN(k) ((nargin > (k)) ? pargin [k] : NULL)

#define GET_SCALAR(arg,type,n,n_default)    \
    n = n_default ;                    \
    if (nargin > arg) n = (type) mxGetScalar (pargin [arg]) ;

// MATCH(s,t) compares two strings and returns true if equal
#define MATCH(s,t) (strcmp(s,t) == 0)

// timer functions, and result statistics
extern double gbtime, tic [2] ;
void GB_mx_put_time
(
    GrB_Desc_Value AxB_method_used
) ;
void GB_mx_clear_time ( ) ;             // clear the time and start the tic
#define TIC { GB_mx_clear_time ( ) ; }
#define TOC { gbtime = simple_toc (tic) ; }

bool GB_mx_mxArray_to_BinaryOp          // true if successful, false otherwise
(
    GrB_BinaryOp *handle,               // the binary op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name,                   // name of the argument
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const bool XisComplex,              // true if X is complex
    const bool YisComplex               // true if X is complex
) ;

bool GB_mx_mxArray_to_UnaryOp           // true if successful
(
    GrB_UnaryOp *handle,                // returns GraphBLAS version of op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name,                   // name of the argument
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const bool XisComplex               // true if X is complex
) ;

bool GB_mx_mxArray_to_SelectOp          // true if successful
(
    GxB_SelectOp *handle,               // returns GraphBLAS version of op
    const mxArray *op_matlab,           // MATLAB version of op
    const char *name                    // name of the argument
) ;

bool GB_mx_string_to_BinaryOp           // true if successful, false otherwise
(
    GrB_BinaryOp *handle,               // the binary op
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const mxArray *opname_mx,           // MATLAB string with operator name
    const mxArray *opclass_mx,          // MATLAB string with operator class
    GB_Opcode *opcode_return,           // opcode
    mxClassID *opclass_return,          // opclass
    const bool XisComplex,              // true if X is complex
    const bool YisComplex               // true if X is complex
) ;

bool GB_mx_string_to_UnaryOp            // true if successful, false otherwise
(
    GrB_UnaryOp *handle,                // the unary op
    const GB_Opcode default_opcode,     // default operator
    const mxClassID default_opclass,    // default operator class
    const mxArray *opname_mx,           // MATLAB string with operator name
    const mxArray *opclass_mx,          // MATLAB string with operator class
    GB_Opcode *opcode_return,           // opcode
    mxClassID *opclass_return,          // opclass
    const bool XisComplex               // true if X is complex
) ;

mxArray *GB_mx_Vector_to_mxArray    // returns the MATLAB mxArray
(
    GrB_Vector *handle,             // handle of GraphBLAS matrix to convert
    const char *name,               // name for error reporting
    const bool create_struct        // if true, then return a struct
) ;

mxArray *GB_mx_Matrix_to_mxArray    // returns the MATLAB mxArray
(
    GrB_Matrix *handle,             // handle of GraphBLAS matrix to convert
    const char *name,
    const bool create_struct        // if true, then return a struct
) ;

mxArray *GB_mx_object_to_mxArray    // returns the MATLAB mxArray
(
    GrB_Matrix *handle,             // handle of GraphBLAS matrix to convert
    const char *name,
    const bool create_struct        // if true, then return a struct
) ;

GrB_Matrix GB_mx_mxArray_to_Matrix     // returns GraphBLAS version of A
(
    const mxArray *A_matlab,            // MATLAB version of A
    const char *name,                   // name of the argument
    bool deep_copy,                     // if true, return a deep copy
    const bool empty    // if false, 0-by-0 matrices are returned as NULL.
                        // if true, a 0-by-0 matrix is returned.
) ;

GrB_Vector GB_mx_mxArray_to_Vector     // returns GraphBLAS version of V
(
    const mxArray *V_matlab,            // MATLAB version of V
    const char *name,                   // name of the argument
    const bool deep_copy,               // if true, return a deep copy
    const bool empty    // if false, 0-by-0 matrices are returned as NULL.
                        // if true, a 0-by-0 matrix is returned.
) ;

mxClassID GB_mx_string_to_classID       // returns the MATLAB class ID
(
    const mxClassID class_default,      // default if string is NULL
    const mxArray *class_mx             // string with class name
) ;

mxArray *GB_mx_classID_to_string        // returns a MATLAB string
(
    const mxClassID classID             // MATLAB class ID to convert to string
) ;

GrB_Type GB_mx_classID_to_Type          // returns a GraphBLAS type
(
    const mxClassID xclass              // MATLAB class ID to convert
) ;

mxClassID GB_mx_Type_to_classID         // returns a MATLAB class ID
(
    const GrB_Type type                 // GraphBLAS type to convert
) ;

void GB_mx_mxArray_to_array     // convert mxArray to array
(
    const mxArray *Xmatlab,     // input MATLAB array
    // output:
    void **X,                   // pointer to numerical values
    int64_t *nrows,             // number of rows of X
    int64_t *ncols,             // number of columns of X
    mxClassID *xclass,          // MATLAB class of X
    GrB_Type *xtype             // GraphBLAS type of X, NULL if error
) ;

int GB_mx_mxArray_to_string // returns length of string, or -1 if S not a string
(
    char *string,           // size maxlen
    const size_t maxlen,    // length of string
    const mxArray *S        // MATLAB mxArray containing a string
) ;

bool GB_mx_mxArray_to_Descriptor    // true if successful, false otherwise
(
    GrB_Descriptor *handle,         // descriptor to return
    const mxArray *D_matlab,        // MATLAB struct
    const char *name                // name of the descriptor
) ;

bool GB_mx_mxArray_to_Semiring          // true if successful
(
    GrB_Semiring *handle,               // the semiring
    const mxArray *semiring_matlab,     // MATLAB version of semiring
    const char *name,                   // name of the argument
    const mxClassID default_class       // default operator class
) ;

GrB_Semiring GB_mx_builtin_semiring // built-in semiring, or NULL if error
(
    const GrB_Monoid add_monoid,    // input monoid
    const GrB_BinaryOp mult         // input multiply operator
) ;

GrB_Monoid GB_mx_builtin_monoid     // built-in monoid, or NULL if error
(
    const GrB_BinaryOp add          // monoid operator
) ;

bool GB_mx_mxArray_to_indices       // true if successful, false otherwise
(
    GrB_Index **handle,             // index array returned
    const mxArray *I_matlab,        // MATLAB mxArray to get
    GrB_Index *ni,                  // length of I, or special
    GrB_Index Icolon [3],           // for all but GB_LIST
    bool *I_is_list                 // true if I is an explicit list
) ;

bool GB_mx_Monoid               // true if successful, false otherwise
(
    GrB_Monoid *handle,         // monoid to construct
    const GrB_BinaryOp add,     // monoid operator
    const bool malloc_debug     // true if malloc debug should be done
) ;

bool GB_mx_get_global (bool cover) ;

void GB_mx_put_global
(   
    bool cover,
    GrB_Desc_Value AxB_method_used
) ;

void GB_mx_complex_merge    // merge real/imag parts of MATLAB array
(
    int64_t n,
    // output:
    double *X,          // size 2*n, real and imaginary parts interleaved
    // input:
    const mxArray *Y    // MATLAB array with n elements
) ;

void GB_mx_complex_split    // split complex array to real/imag part for MATLAB
(
    int64_t n,
    // input:
    const double *X,    // size 2*n, real and imaginary parts interleaved
    // output:
    mxArray *Y          // MATLAB array with n elements
) ;

bool GB_mx_same     // true if arrays X and Y are the same
(
    char *X,
    char *Y,
    int64_t len     // length of X and Y
) ;

bool GB_mx_xsame    // true if arrays X and Y are the same (ignoring zombies)
(
    char *X,
    char *Y,
    int64_t len,    // length of X and Y
    size_t s,       // size of each entry of X and Y
    int64_t *I      // row indices (for zombies), same length as X and Y
) ;

bool GB_mx_isequal  // true if A and B are exactly the same
(
    GrB_Matrix A,
    GrB_Matrix B
) ;

//------------------------------------------------------------------------------

#ifdef GB_PRINT_MALLOC

#define AS_IF_FREE(p)           \
{                               \
    GB_Global.nmalloc-- ;       \
    printf ("\nfree:                         to MATLAB (%s) line %d file %s\n",\
        GB_STR(p), __LINE__,__FILE__); \
    printf ("free:    %14p %3d %1d\n", \
        p, GB_Global.nmalloc, GB_Global.malloc_debug) ; \
    (p) = NULL ;                \
}

#else

#define AS_IF_FREE(p)           \
{                               \
    GB_Global.nmalloc-- ;       \
    (p) = NULL ;                \
}

#endif

#ifdef GB_PRINT_MALLOC

#define METHOD_START(OP) \
    printf ("\n================================================================================\n") ; \
    printf ("method: [%s] start: %d\n", #OP, GB_Global.nmalloc) ; \
    printf ("================================================================================\n") ;

#define METHOD_TRY \
    printf ("\n--------------------------------------------------------------------- try %d\n", tries) ;

#define METHOD_FINAL(OP) \
    printf ("\n================================================================================\n") ; \
    printf ("method: [%s] # tries before success: %d\n", #OP, tries) ;  \
    printf ("================================================================================\n") ;

#else

#define METHOD_START(OP) ;
#define METHOD_TRY ;
#define METHOD_FINAL(OP) ;

#endif


// test a GraphBLAS operation with malloc debuging
#define METHOD(GRAPHBLAS_OPERATION)                                         \
    METHOD_START (GRAPHBLAS_OPERATION) ;                                    \
    if (!malloc_debug)                                                      \
    {                                                                       \
        /* no malloc debugging; just call the method */                     \
        TIC ;                                                               \
        GrB_Info info = GRAPHBLAS_OPERATION ;                               \
        TOC ;                                                               \
        if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                \
        {                                                                   \
            FREE_ALL ;                                                      \
            mexErrMsgTxt (GrB_error ( )) ;                                  \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* brutal malloc debug */                                           \
        int nmalloc_start = (int) GB_Global.nmalloc ;                       \
        for (int tries = 0 ; ; tries++)                                     \
        {                                                                   \
            /* give GraphBLAS the ability to do a # of mallocs, */          \
            /* callocs, and reallocs of larger size, equal to tries */      \
            GB_Global.malloc_debug_count = tries ;                          \
            METHOD_TRY ;                                                    \
            /* call the method with malloc debug enabled */                 \
            GB_Global.malloc_debug = true ;                                 \
            TIC ;                                                           \
            GrB_Info info = GRAPHBLAS_OPERATION ;                           \
            TOC ;                                                           \
            GB_Global.malloc_debug = false ;                                \
            if (tries > 1000000) mexErrMsgTxt ("infinite loop!") ;          \
            if (info == GrB_SUCCESS || info == GrB_NO_VALUE)                \
            {                                                               \
                /* finally gave GraphBLAS enough malloc's to do the work */ \
                METHOD_FINAL (GRAPHBLAS_OPERATION) ;                        \
                break ;                                                     \
            }                                                               \
            else if (info == GrB_OUT_OF_MEMORY)                             \
            {                                                               \
                /* out of memory; check for leaks */                        \
                /* output matrix may have changed; recopy for next test */  \
                /* but turn off malloc debugging to get the copy */         \
                FREE_DEEP_COPY ;                                            \
                GET_DEEP_COPY ;                                             \
                int nmalloc_end = (int) GB_Global.nmalloc ;                 \
                if (nmalloc_end > nmalloc_start)                            \
                {                                                           \
                    /* memory leak */                                       \
                    printf ("Leak! tries %d : %d %d\nmethod: %s\n",         \
                        tries, nmalloc_end, nmalloc_start,                  \
                        GB_STR (GRAPHBLAS_OPERATION)) ;                     \
                    mexWarnMsgIdAndTxt ("GB:leak", GrB_error ( )) ;         \
                    FREE_ALL ;                                              \
                    mexErrMsgTxt ("Leak!") ;                                \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                /* another error has occurred */                            \
                printf ("an error: %s line %d\n%s\n", __FILE__, __LINE__,   \
                    GrB_error ()) ;                                         \
                FREE_ALL ;                                                  \
                mexErrMsgTxt (GrB_error ( )) ;                              \
            }                                                               \
        }                                                                   \
    }

//------------------------------------------------------------------------------
// statement coverage
//------------------------------------------------------------------------------

// GB_cover_get copies GraphBLAS_gbcov from the MATLAB global workspace into
// the internal GB_cov array.  The MATLAB array is created if it doesn't exist.
// Thus, to clear the counts simply clear GraphBLAS_gbcov from the MATLAB
// global workpace.
void GB_cover_get ( ) ;

// GB_cover_put copies the internal GB_cov array back into the MATLAB
// GraphBLAS_gbcov array, for analysis and for subsequent statement counting.
// This way, multiple tests in MATLAB can be accumulated into a single array
// of counters.
void GB_cover_put ( ) ;

#endif

