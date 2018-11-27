//------------------------------------------------------------------------------
// GB_mex_subassign: C(I,J)<Mask> = accum (C (I,J), A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// This function is a wrapper for all GxB_*_subassign functions.
// For these uses, the Mask must always be the same size as C(I,J) and A.

// GxB_Matrix_subassign: Mask has the same size as C(I,J) and A
// GxB_Matrix_subassign_TYPE: Mask has the same size as C(I,J).  A is scalar.

// GxB_Vector_subassign: Mask has the same size as C(I,J) and A
// GxB_Vector_subassign_TYPE: Mask has the same size as C(I,J).  A is scalar.

// GxB_Col_subassign: on input to GB_mex_subassign, Mask and A are a single
// columns, the same size as the single subcolumn C(I,j).  They are not column
// vectors.

// GxB_Row_subassign: on input to GB_mex_subassign, Mask and A are single ROWS,
// the same size as the single subrow C(i,J).  They are not column vectors.
// Before GxB_Row_subassign is called, the A and the Mask (if present) are
// transposed.

// Thus, in all cases, A and the Mask (if present) have the same size as C(I,J),
// except in the case where A is a scalar.  In that case, A is implicitly
// expanded into a matrix the same size as C(I,J), but this occurs inside
// GxB_*subassign, not here.

// This function does the same thing as the MATLAB mimic GB_spec_subassign.m.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[C,s,t] = GB_mex_subassign " \
              "(C, Mask, accum, A, I, J, desc, reduce) or (C, Work)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&Mask) ;            \
    GB_MATRIX_FREE (&C) ;               \
    GrB_free (&desc) ;                  \
    if (!reduce_is_complex) GrB_free (&reduce) ;                \
    GB_mx_put_global (true, 0) ;        \
}

#define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;

#define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;

GrB_Matrix C = NULL ;
GrB_Matrix Mask = NULL ;
GrB_Matrix A = NULL ;
GrB_Matrix mask = NULL, u = NULL ;
GrB_Descriptor desc = NULL ;
GrB_BinaryOp accum = NULL ;
GrB_Index *I = NULL, ni = 0, I_range [3] ;
GrB_Index *J = NULL, nj = 0, J_range [3] ;
bool ignore ;
bool malloc_debug = false ;
GrB_Info info = GrB_SUCCESS ;
GrB_Monoid reduce = NULL ;
GrB_BinaryOp op = NULL ;
bool reduce_is_complex = false ;

//------------------------------------------------------------------------------
// assign: perform a single assignment
//------------------------------------------------------------------------------

#define OK(method)                      \
{                                       \
    info = method ;                     \
    if (info != GrB_SUCCESS)            \
    {                                   \
        GB_MATRIX_FREE (&mask) ;        \
        GB_MATRIX_FREE (&u) ;           \
        return (info) ;                 \
    }                                   \
}

GrB_Info assign (GB_Context Context)
{
    bool at = (desc != NULL && desc->in0 == GrB_TRAN) ;
    GrB_Info info ;

    int pr = 0 ;
    bool ph = (pr > 0) ;

    ASSERT_OK (GB_check (C, "C before mex assign", pr)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for mex assign", pr)) ;
    ASSERT_OK (GB_check (A, "A for mex assign", pr)) ;

    if (GB_NROWS (A) == 1 && GB_NCOLS (A) == 1 && GB_NNZ (A) == 1)
    {
        // scalar expansion to matrix or vector
        void *Ax = A->x ;

        if (ni == 1 && nj == 1 && Mask == NULL && I != GrB_ALL && J != GrB_ALL
            && GB_op_is_second (accum, C->type) && A->type->code <= GB_FP64_code
            && desc == NULL)
        {
            if (ph) printf ("setElement\n") ;
            // test GrB_Matrix_setElement
            #define ASSIGN(type)                                        \
            {                                                           \
                type x = ((type *) Ax) [0] ;                            \
                OK (GrB_Matrix_setElement (C, x, I [0], J [0])) ;       \
            } break ;

            switch (A->type->code)
            {
                case GB_BOOL_code   : ASSIGN (bool) ;
                case GB_INT8_code   : ASSIGN (int8_t) ;
                case GB_UINT8_code  : ASSIGN (uint8_t) ;
                case GB_INT16_code  : ASSIGN (int16_t) ;
                case GB_UINT16_code : ASSIGN (uint16_t) ;
                case GB_INT32_code  : ASSIGN (int32_t) ;
                case GB_UINT32_code : ASSIGN (uint32_t) ;
                case GB_INT64_code  : ASSIGN (int64_t) ;
                case GB_UINT64_code : ASSIGN (uint64_t) ;
                case GB_FP32_code   : ASSIGN (float) ;
                case GB_FP64_code   : ASSIGN (double) ;
                case GB_UCT_code    :
                case GB_UDT_code    :
                default:
                    FREE_ALL ;
                    mexErrMsgTxt ("unsupported class") ;
            }
            #undef ASSIGN

        }
        if (GB_VECTOR_OK (C) && GB_VECTOR_OK (Mask))
        {

            // test GxB_Vector_subassign_scalar functions
            if (ph) printf ("scalar assign to vector\n") ;
            #define ASSIGN(type)                                        \
            {                                                           \
                type x = ((type *) Ax) [0] ;                            \
                OK (GxB_subassign ((GrB_Vector) C, (GrB_Vector) Mask,   \
                    accum, x, I, ni, desc)) ;      \
            } break ;

            switch (A->type->code)
            {
                case GB_BOOL_code   : ASSIGN (bool) ;
                case GB_INT8_code   : ASSIGN (int8_t) ;
                case GB_UINT8_code  : ASSIGN (uint8_t) ;
                case GB_INT16_code  : ASSIGN (int16_t) ;
                case GB_UINT16_code : ASSIGN (uint16_t) ;
                case GB_INT32_code  : ASSIGN (int32_t) ;
                case GB_UINT32_code : ASSIGN (uint32_t) ;
                case GB_INT64_code  : ASSIGN (int64_t) ;
                case GB_UINT64_code : ASSIGN (uint64_t) ;
                case GB_FP32_code   : ASSIGN (float) ;
                case GB_FP64_code   : ASSIGN (double) ;
                case GB_UCT_code    :
                case GB_UDT_code    :
                {
                    OK (GxB_subassign ((GrB_Vector) C, (GrB_Vector) Mask,
                        accum, Ax, I, ni, desc)) ;
                }
                break ;
                default:
                    FREE_ALL ;
                    mexErrMsgTxt ("unsupported class") ;
            }
            #undef ASSIGN

        }
        else
        {

            // test Matrix_subassign_scalar functions
            if (ph) printf ("scalar assign to matrix\n") ;
            #define ASSIGN(type)                                            \
            {                                                               \
                type x = ((type *) Ax) [0] ;                                \
                OK (GxB_subassign (C, Mask, accum, x, I, ni, J, nj,desc)) ; \
            } break ;

            switch (A->type->code)
            {
                case GB_BOOL_code   : ASSIGN (bool) ;
                case GB_INT8_code   : ASSIGN (int8_t) ;
                case GB_UINT8_code  : ASSIGN (uint8_t) ;
                case GB_INT16_code  : ASSIGN (int16_t) ;
                case GB_UINT16_code : ASSIGN (uint16_t) ;
                case GB_INT32_code  : ASSIGN (int32_t) ;
                case GB_UINT32_code : ASSIGN (uint32_t) ;
                case GB_INT64_code  : ASSIGN (int64_t) ;
                case GB_UINT64_code : ASSIGN (uint64_t) ;
                case GB_FP32_code   : ASSIGN (float) ;
                case GB_FP64_code   : ASSIGN (double) ;
                case GB_UCT_code    :
                case GB_UDT_code    :
                {
                    OK (GxB_subassign (C, Mask, accum, Ax, I, ni, J, nj, desc));
                }
                break ;

                default:
                    FREE_ALL ;
                    mexErrMsgTxt ("unsupported class") ;
            }
            #undef ASSIGN

        }
    }
    else if (GB_VECTOR_OK (C) && GB_VECTOR_OK (A) &&
        (Mask == NULL || GB_VECTOR_OK (Mask)) && !at)
    {
        // test GxB_Vector_subassign
        if (ph) printf ("vector assign\n") ;
        OK (GxB_subassign ((GrB_Vector) C, (GrB_Vector) Mask, accum,
            (GrB_Vector) A, I, ni, desc)) ;
    }
    else if (GB_VECTOR_OK (A) && nj == 1 &&
        (Mask == NULL || GB_VECTOR_OK (Mask)) && !at)
    {
        // test GxB_Col_subassign
        if (ph) printf ("col assign\n") ;
        OK (GxB_subassign (C, (GrB_Vector) Mask, accum, (GrB_Vector) A,
            I, ni, J [0], desc)) ;
    }
    else if (A->vlen == 1 && ni == 1 &&
        (Mask == NULL || Mask->vlen == 1) && !at)
    {
        // test GxB_Row_subassign; this is not meant to be efficient,
        // just for testing
        if (ph) printf ("row assign\n") ;
        if (Mask != NULL)
        {
            OK (GB_transpose_bucket (&mask, GrB_BOOL, true, Mask, NULL,
                Context)) ;
            ASSERT (GB_VECTOR_OK (mask)) ;
        }
        OK (GB_transpose_bucket (&u, A->type, true, A, NULL, Context)) ;
        ASSERT (GB_VECTOR_OK (u)) ;
        OK (GxB_subassign (C, (GrB_Vector) mask, accum, (GrB_Vector) u,
            I [0], J, nj, desc)) ;
        GB_MATRIX_FREE (&mask) ;
        GB_MATRIX_FREE (&u) ;
    }
    else
    {
        // standard submatrix assignment
        if (ph) printf ("submatrix assign\n") ;
        OK (GxB_subassign (C, Mask, accum, A, I, ni, J, nj, desc)) ;
    }

    ASSERT_OK (GB_check (C, "C after assign", pr)) ;
    return (info) ;
}

//------------------------------------------------------------------------------
// many_subassign: do a sequence of assignments
//------------------------------------------------------------------------------

// The list of assignments is in a struct array

GrB_Info many_subassign
(
    int nwork,
    int fA,
    int fI,
    int fJ,
    int faccum,
    int fMask,
    int fdesc,
    mxClassID cclass,
    const mxArray *pargin [ ],
    GB_Context Context
)
{
    GrB_Info info = GrB_SUCCESS ;

    for (int64_t k = 0 ; k < nwork ; k++)
    {
        // printf ("work %g of %g\n", (double) k, (double) nwork-1) ;

        //----------------------------------------------------------------------
        // get the kth work to do
        //----------------------------------------------------------------------

        // each struct has fields A, I, J, and optionally Mask, accum, and desc

        mxArray *p ;

        // [ turn off malloc debugging
        bool save = GB_Global.malloc_debug ;
        GB_Global.malloc_debug = false ;

        // get Mask (shallow copy)
        Mask = NULL ;
        if (fMask >= 0)
        {
            p = mxGetFieldByNumber (pargin [1], k, fMask) ;
            Mask = GB_mx_mxArray_to_Matrix (p, "Mask", false, false) ;
            if (Mask == NULL && !mxIsEmpty (p))
            {
                FREE_ALL ;
                mexErrMsgTxt ("Mask failed") ;
            }
        }

        // get A (shallow copy)
        p = mxGetFieldByNumber (pargin [1], k, fA) ;
        A = GB_mx_mxArray_to_Matrix (p, "A", false, true) ;
        if (A == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("A failed") ;
        }

        // get accum; default: NOP, default class is class(C)
        accum = NULL ;
        if (faccum >= 0)
        {
            p = mxGetFieldByNumber (pargin [1], k, faccum) ;
            if (!GB_mx_mxArray_to_BinaryOp (&accum, p, "accum",
                GB_NOP_opcode, cclass,
                C->type == Complex, A->type == Complex))
            {
                FREE_ALL ;
                mexErrMsgTxt ("accum failed") ;
            }
        }

        // get I
        p = mxGetFieldByNumber (pargin [1], k, fI) ;
        if (!GB_mx_mxArray_to_indices (&I, p, &ni, I_range, &ignore))
        {
            FREE_ALL ;
            mexErrMsgTxt ("I failed") ;
        }

        // get J
        p = mxGetFieldByNumber (pargin [1], k, fJ) ;
        if (!GB_mx_mxArray_to_indices (&J, p, &nj, J_range, &ignore))
        {
            FREE_ALL ;
            mexErrMsgTxt ("J failed") ;
        }

        // get desc
        desc = NULL ;
        if (fdesc > 0)
        {
            p = mxGetFieldByNumber (pargin [1], k, fdesc) ;
            if (!GB_mx_mxArray_to_Descriptor (&desc, p, "desc"))
            {
                FREE_ALL ;
                mexErrMsgTxt ("desc failed") ;
            }
        }
        // restore malloc debugging to test the method
        GB_Global.malloc_debug = save ;   // ]

        //----------------------------------------------------------------------
        // C(I,J)<Mask> = A
        //----------------------------------------------------------------------

        info = assign (Context) ;

        GB_MATRIX_FREE (&A) ;
        GB_MATRIX_FREE (&Mask) ;
        GrB_free (&desc) ;

        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }
    }

    OK (GrB_wait ( )) ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GB_mex_subassign mexFunction
//------------------------------------------------------------------------------

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

    malloc_debug = GB_mx_get_global (true) ;
    A = NULL ;
    C = NULL ;
    Mask = NULL ;
    desc = NULL ;
    reduce_is_complex = false ;
    op = NULL ;
    reduce = NULL ;

    GB_WHERE (USAGE) ;
    if (!((nargout == 1 && (nargin == 2 || nargin == 6 || nargin == 7)) ||
          ((nargout == 2 || nargout == 3) && nargin == 8)))
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    //--------------------------------------------------------------------------
    // get C (make a deep copy)
    //--------------------------------------------------------------------------

    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    if (nargin == 2)
    {

        //----------------------------------------------------------------------
        // get a list of work to do: a struct array of length nwork
        //----------------------------------------------------------------------

        // each entry is a struct with fields:
        // Mask, accum, A, I, J, desc

        if (!mxIsStruct (pargin [1]))
        {
            FREE_ALL ;
            mexErrMsgTxt ("2nd argument must be a struct") ;
        }

        int nwork = mxGetNumberOfElements (pargin [1]) ;
        int nf = mxGetNumberOfFields (pargin [1]) ;
        for (int f = 0 ; f < nf ; f++)
        {
            mxArray *p ;
            for (int k = 0 ; k < nwork ; k++)
            {
                p = mxGetFieldByNumber (pargin [1], k, f) ;
            }
        }

        int fA = mxGetFieldNumber (pargin [1], "A") ;
        int fI = mxGetFieldNumber (pargin [1], "I") ;
        int fJ = mxGetFieldNumber (pargin [1], "J") ;
        int faccum = mxGetFieldNumber (pargin [1], "accum") ;
        int fMask = mxGetFieldNumber (pargin [1], "Mask") ;
        int fdesc = mxGetFieldNumber (pargin [1], "desc") ;

        if (fA < 0 || fI < 0 || fJ < 0) mexErrMsgTxt ("A,I,J required") ;

        METHOD (many_subassign (nwork, fA, fI, fJ, faccum, fMask, fdesc, cclass,
            pargin, Context)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C(I,J)<Mask> = A, with a single assignment
        //----------------------------------------------------------------------

        // get Mask (shallow copy)
        Mask = GB_mx_mxArray_to_Matrix (pargin [1], "Mask", false, false) ;
        if (Mask == NULL && !mxIsEmpty (pargin [1]))
        {
            FREE_ALL ;
            mexErrMsgTxt ("Mask failed") ;
        }

        // get A (shallow copy)
        A = GB_mx_mxArray_to_Matrix (pargin [3], "A", false, true) ;
        if (A == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("A failed") ;
        }

        // get accum; default: NOP, default class is class(C)
        accum = NULL ;
        if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
            GB_NOP_opcode, cclass, C->type == Complex, A->type == Complex))
        {
            FREE_ALL ;
            mexErrMsgTxt ("accum failed") ;
        }

        // get I
        if (!GB_mx_mxArray_to_indices (&I, pargin [4], &ni, I_range, &ignore))
        {
            FREE_ALL ;
            mexErrMsgTxt ("I failed") ;
        }

        // get J
        if (!GB_mx_mxArray_to_indices (&J, pargin [5], &nj, J_range, &ignore))
        {
            FREE_ALL ;
            mexErrMsgTxt ("J failed") ;
        }

        // get desc
        if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (6), "desc"))
        {
            FREE_ALL ;
            mexErrMsgTxt ("desc failed") ;
        }

        if (nargin == 8 && (nargout == 2 || nargout == 3))
        {
            // get reduce operator
            if (!GB_mx_mxArray_to_BinaryOp (&op, PARGIN (7), "op",
                GB_NOP_opcode, cclass, C->type == Complex, C->type == Complex))
            {
                FREE_ALL ;
                mexErrMsgTxt ("op failed") ;
            }

            // get the reduce monoid
            if (op == Complex_plus)
            {
                reduce_is_complex = true ;
                reduce = Complex_plus_monoid ;
            }
            else if (op == Complex_times)
            {
                reduce_is_complex = true ;
                reduce = Complex_times_monoid ;
            }
            else
            {
                // create the reduce monoid
                if (!GB_mx_Monoid (&reduce, op, malloc_debug))
                {
                    FREE_ALL ;
                    mexErrMsgTxt ("reduce failed") ;
                }
            }
        }

        // C(I,J)<Mask> = A

        METHOD (assign (Context)) ;

        // apply the reduce monoid
        if (nargin == 8 && (nargout == 2 || nargout == 3))
        {
            // if (C->nzombies > 0)
            //  printf ("do the reduce thing, zombies %lld\n", C->nzombies) ;

            #define REDUCE(type)                                             \
            {                                                                \
                type c = 0 ;                                                 \
                GrB_reduce (&c, NULL, reduce, C, NULL) ;                     \
                pargout [1] = mxCreateNumericMatrix (1, 1, cclass, mxREAL) ; \
                void *p = mxGetData (pargout [1]) ;                          \
                memcpy (p, &c, sizeof (type)) ;                              \
                double d = 0 ;                                               \
                GrB_reduce (&d, NULL, GxB_PLUS_FP64_MONOID, C, NULL) ;       \
                if (nargout > 2) pargout [2] = mxCreateDoubleScalar (d) ;    \
            }                                                                \
            break ;

            if (reduce_is_complex)
            {
                double c [2] = {0, 0} ;
                GrB_reduce ((void *) c, NULL, reduce, C, NULL) ;
                pargout [1] = mxCreateNumericMatrix (1, 1,
                    mxDOUBLE_CLASS, mxCOMPLEX) ;
                GB_mx_complex_split (1, c, pargout [1]) ;
            }
            else
            {
                switch (cclass)
                {

                    case mxLOGICAL_CLASS : REDUCE (bool) ;
                    case mxINT8_CLASS    : REDUCE (int8_t) ;
                    case mxUINT8_CLASS   : REDUCE (uint8_t) ;
                    case mxINT16_CLASS   : REDUCE (int16_t) ;
                    case mxUINT16_CLASS  : REDUCE (uint16_t) ;
                    case mxINT32_CLASS   : REDUCE (int32_t) ;
                    case mxUINT32_CLASS  : REDUCE (uint32_t) ;
                    case mxINT64_CLASS   : REDUCE (int64_t) ;
                    case mxUINT64_CLASS  : REDUCE (uint64_t) ;
                    case mxSINGLE_CLASS  : REDUCE (float) ;
                    case mxDOUBLE_CLASS  : REDUCE (double) ;

                    case mxCELL_CLASS    :
                    case mxCHAR_CLASS    :
                    case mxUNKNOWN_CLASS :
                    case mxFUNCTION_CLASS:
                    case mxSTRUCT_CLASS  :
                    default              :
                        FREE_ALL ;
                        mexErrMsgTxt ("unsupported class") ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // return C to MATLAB as a struct
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "Final C before wait", GB0)) ;
    GrB_wait ( ) ;
    TOC ;
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C assign result", true) ;
    FREE_ALL ;
}

