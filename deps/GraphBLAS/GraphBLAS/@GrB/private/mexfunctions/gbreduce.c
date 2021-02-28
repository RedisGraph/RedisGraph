//------------------------------------------------------------------------------
// gbreduce: reduce a sparse matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbreduce is an interface to GrB_Matrix_reduce_[TYPE].

// Usage:

//  cout = GrB.reduce (op, A, desc)
//  cout = GrB.reduce (cin, accum, op, A, desc)

// If cin is not present then it is implicitly a 1-by-1 matrix with no entries.

#include "gb_matlab.h"

#define USAGE "usage: Cout = GrB.reduce (cin, accum, op, A, desc)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage ((nargin == 3 || nargin == 5) && nargout <= 1, USAGE) ;

    //--------------------------------------------------------------------------
    // find the arguments
    //--------------------------------------------------------------------------

    mxArray *Matrix [4], *String [2], *Cell [2] ;
    base_enum_t base ;
    kind_enum_t kind ;
    GxB_Format_Value fmt ;
    int nmatrices, nstrings, ncells ;
    GrB_Descriptor desc ;
    gb_get_mxargs (nargin, pargin, USAGE, Matrix, &nmatrices, String, &nstrings,
        Cell, &ncells, &desc, &base, &kind, &fmt) ;

    CHECK_ERROR (nmatrices < 1 || nmatrices > 2 || nstrings < 1 || ncells > 0,
        USAGE) ;

    //--------------------------------------------------------------------------
    // get the matrices
    //--------------------------------------------------------------------------

    GrB_Type atype, ctype = NULL ;
    GrB_Matrix C = NULL, A ;

    if (nmatrices == 1)
    { 
        A = gb_get_shallow (Matrix [0]) ;
    }
    else // if (nmatrices == 2)
    { 
        C = gb_get_deep    (Matrix [0]) ;
        A = gb_get_shallow (Matrix [1]) ;
    }

    OK (GxB_Matrix_type (&atype, A)) ;
    if (C != NULL)
    { 
        OK (GxB_Matrix_type (&ctype, C)) ;
    }

    //--------------------------------------------------------------------------
    // get the operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp accum = NULL ;
    GrB_Monoid monoid ;

    if (nstrings == 1)
    { 
        monoid = gb_mxstring_to_monoid (String [0], atype) ;
    }
    else 
    { 
        // if accum appears, then Cin must also appear
        CHECK_ERROR (C == NULL, USAGE) ;
        accum  = gb_mxstring_to_binop  (String [0], ctype) ;
        monoid = gb_mxstring_to_monoid (String [1], atype) ;
    }

    //--------------------------------------------------------------------------
    // construct C if not present on input
    //--------------------------------------------------------------------------

    // If C is NULL, then it is not present on input.
    // Construct C of the right size and type.

    if (C == NULL)
    { 
        // use the ztype of the monoid as the type of C
        GrB_BinaryOp binop ;
        OK (GxB_Monoid_operator (&binop, monoid)) ;
        OK (GxB_BinaryOp_ztype (&ctype, binop)) ;

        OK (GrB_Matrix_new (&C, ctype, 1, 1)) ;
        fmt = gb_get_format (1, 1, A, NULL, fmt) ;
        OK (GxB_Matrix_Option_set (C, GxB_FORMAT, fmt)) ;
    }

    //--------------------------------------------------------------------------
    // ensure C is 1-by-1 with a single entry
    //--------------------------------------------------------------------------

    GrB_Index cnrows, cncols ;
    OK (GrB_Matrix_nrows (&cnrows, C)) ;
    OK (GrB_Matrix_ncols (&cncols, C)) ;
    if (cnrows != 1 || cncols != 1)
    { 
        ERROR ("cin must be a scalar") ;
    }

    GrB_Index nvals ;
    OK (GrB_Matrix_nvals (&nvals, C)) ;
    if (nvals == 0)
    { 
        // set C(0,0) to zero
        OK (GrB_Matrix_nvals (&nvals, C)) ;
        OK (GrB_Matrix_setElement_BOOL (C, 0, 0, 0)) ;
    }

    //--------------------------------------------------------------------------
    // compute C<M> += reduce(A)
    //--------------------------------------------------------------------------

    if (ctype == GrB_BOOL)
    { 
        bool c = false ;
        OK (GrB_Matrix_extractElement_BOOL (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_BOOL (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_BOOL (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_INT8)
    { 
        int8_t c = 0 ;
        OK (GrB_Matrix_extractElement_INT8 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_INT8 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_INT8 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_INT16)
    { 
        int16_t c = 0 ;
        OK (GrB_Matrix_extractElement_INT16 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_INT16 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_INT16 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_INT32)
    { 
        int32_t c = 0 ;
        OK (GrB_Matrix_extractElement_INT32 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_INT32 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_INT32 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_INT64)
    { 
        int64_t c = 0 ;
        OK (GrB_Matrix_extractElement_INT64 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_INT64 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_INT64 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_UINT8)
    { 
        uint8_t c = 0 ;
        OK (GrB_Matrix_extractElement_UINT8 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_UINT8 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_UINT8 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_UINT16)
    { 
        uint16_t c = 0 ;
        OK (GrB_Matrix_extractElement_UINT16 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_UINT16 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_UINT16 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_UINT32)
    { 
        uint32_t c = 0 ;
        OK (GrB_Matrix_extractElement_UINT32 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_UINT32 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_UINT32 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_UINT64)
    { 
        uint64_t c = 0 ;
        OK (GrB_Matrix_extractElement_UINT64 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_UINT64 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_UINT64 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_FP32)
    { 
        float c = 0 ;
        OK (GrB_Matrix_extractElement_FP32 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_FP32 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_FP32 (C, c, 0, 0)) ;
    }
    else if (ctype == GrB_FP64)
    { 
        double c = 0 ;
        OK (GrB_Matrix_extractElement_FP64 (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_FP64 (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_FP64 (C, c, 0, 0)) ;
    }
    #ifdef GB_COMPLEX_TYPE
    else if (ctype == gb_complex_type)
    {
        double complex c = 0 ;
        OK (GrB_Matrix_extractElement_UDT (&c, C, 0, 0)) ;
        OK (GrB_Matrix_reduce_UDT (&c, accum, monoid, A, desc)) ;
        OK (GrB_Matrix_setElement_UDT (C, c, 0, 0)) ;
    }
    #endif
    else
    {
        ERROR ("unsupported type") ;
    }

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    GB_WRAPUP ;
}

