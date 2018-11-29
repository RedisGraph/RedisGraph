//------------------------------------------------------------------------------
// GB_mex_assign: C<Mask>(I,J) = accum (C (I,J), A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// This function is a wrapper for GrB_Matrix_assign, GrB_Matrix_assign_T
// GrB_Vector_assign, and GrB_Vector_assign_T (when kind=0 or by default).  For
// these uses, the Mask must always be the same size as C.

// This mexFunction does calls GrB_Row_assign (when kind=2) or GrB_Col_assign
// (when kind=1).  In these cases, the Mask is a single row or column,
// respectively.  C is not modified outside that single row (for
// GrB_Row_assign) or column (for GrB_Col_assign).

// This function does the same thing as the MATLAB mimics GB_spec_assign.m
// (when kind=0), GB_spec_Col_assign (when kind=1), and GB_spec_Row_assign
// (when kind=2).

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_assign (C,Mask,accum,A,I,J,desc,kind) or (C, Work)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&Mask) ;            \
    GB_MATRIX_FREE (&C) ;               \
    GrB_free (&desc) ;                  \
    GB_mx_put_global (true, 0) ;        \
}

#define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;

#define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;

GrB_Matrix C = NULL ;
GrB_Matrix Mask = NULL ;
GrB_Matrix A = NULL ;
GrB_Descriptor desc = NULL ;
GrB_BinaryOp accum = NULL ;
GrB_Index *I = NULL, ni = 0, I_range [3] ;
GrB_Index *J = NULL, nj = 0, J_range [3] ;
bool ignore ;
bool malloc_debug = false ;
GrB_Info info = GrB_SUCCESS ;
int kind = 0 ;

//------------------------------------------------------------------------------
// assign: perform a single assignment
//------------------------------------------------------------------------------

#define OK(method)                      \
{                                       \
    info = method ;                     \
    if (info != GrB_SUCCESS)            \
    {                                   \
        return (info) ;                 \
    }                                   \
}

GrB_Info assign ( )
{
    bool at = (desc != NULL && desc->in0 == GrB_TRAN) ;
    GrB_Info info ;

    int pr = 0 ;
    bool ph = (pr > 0) ;

    ASSERT_OK (GB_check (C, "C for GB_mex_assign", pr)) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for GB_mex_assign", pr)) ;
    ASSERT_OK (GB_check (A, "A for GB_mex_assign", pr)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_mex_assign", pr)) ;
    ASSERT_OK_OR_NULL (GB_check (desc, "desc for GB_mex_assign", pr)) ;

    /*
    if (I == NULL)
    {
        printf ("I is NULL\n") ;
    }
    else if (I == GrB_ALL)
    {
        printf ("I is ALL\n") ;
    }
    else
    {
        for (int64_t k = 0 ; k < ni ; k++)
        {
            printf ("I [%lld] = %lld\n", k, I [k]) ;
        }
    }
    if (J == NULL)
    {
        printf ("J is NULL\n") ;
    }
    else if (J == GrB_ALL)
    {
        printf ("J is ALL\n") ;
    }
    else
    {
        for (int64_t k = 0 ; k < nj ; k++)
        {
            printf ("J [%lld] = %lld\n", k, J [k]) ;
        }
    }
    */

    if (kind == 1)
    {
        // test GrB_Col_assign
        ASSERT (GB_VECTOR_OK (A)) ;
        ASSERT (Mask == NULL || GB_VECTOR_OK (Mask)) ;
        OK (GrB_assign (C, (GrB_Vector) Mask, accum, (GrB_Vector) A,
            I, ni, J [0], desc)) ;
    }
    else if (kind == 2)
    {
        // test GrB_Row_assign
        ASSERT (GB_VECTOR_OK (A)) ;
        ASSERT (Mask == NULL || GB_VECTOR_OK (Mask)) ;
        ASSERT_OK_OR_NULL (GB_check ((GrB_Vector) Mask, "row mask", GB0)) ;
        ASSERT_OK (GB_check ((GrB_Vector) A, "row u", GB0)) ;

        OK (GrB_assign (C, (GrB_Vector) Mask, accum, (GrB_Vector) A,
            I [0], J, nj, desc)) ;
    }
    else if (GB_NROWS (A) == 1 && GB_NCOLS (A) == 1 && GB_NNZ (A) == 1)
    {
        // scalar expansion to matrix or vector
        void *Ax = A->x ;

        if (ni == 1 && nj == 1 && Mask == NULL && I != GrB_ALL && J != GrB_ALL
            && GB_op_is_second (accum, C->type) && A->type->code <= GB_FP64_code
            && desc == NULL)
        {
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

            ASSERT_OK (GB_check (C, "C after setElement", GB0)) ;

        }
        if (GB_VECTOR_OK (C) && (Mask == NULL || GB_VECTOR_OK (Mask)))
        {

            // test GrB_Vector_assign_scalar functions
            #define ASSIGN(type)                                        \
            {                                                           \
                type x = ((type *) Ax) [0] ;                            \
                OK (GrB_assign ((GrB_Vector) C, (GrB_Vector) Mask,      \
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
                    OK (GrB_assign ((GrB_Vector) C, (GrB_Vector) Mask,
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

            // test Matrix_assign_scalar functions
            #define ASSIGN(type)                                            \
            {                                                               \
                type x = ((type *) Ax) [0] ;                                \
                OK (GrB_assign (C, Mask, accum, x, I, ni, J, nj,desc)) ;    \
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
                    OK (GrB_assign (C, Mask, accum, Ax, I, ni, J, nj, desc)) ;
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
        // test GrB_Vector_assign
        OK (GrB_assign ((GrB_Vector) C, (GrB_Vector) Mask, accum,
            (GrB_Vector) A, I, ni, desc)) ;
    }
    else
    {
        // standard submatrix assignment
        OK (GrB_assign (C, Mask, accum, A, I, ni, J, nj, desc)) ;
    }

    ASSERT_OK (GB_check (C, "Final C before wait", GB0)) ;
    OK (GrB_wait ( )) ;
    return (info) ;
}

//------------------------------------------------------------------------------
// many_assign: do a sequence of assignments
//------------------------------------------------------------------------------

// The list of assignments is in a struct array

GrB_Info many_assign
(
    int nwork,
    int fA,
    int fI,
    int fJ,
    int faccum,
    int fMask,
    int fdesc,
    mxClassID cclass,
    int fkind,
    const mxArray *pargin [ ]
)
{
    GrB_Info info = GrB_SUCCESS ;

    for (int64_t k = 0 ; k < nwork ; k++)
    {

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

        /*
        printf ("many assign: fI %d fJ %d ni %lld nj %lld\n", fI, fJ, ni, nj) ;
        for (int kk = 0 ; kk < nj ; kk++) printf ("J[%d]=%lld\n", kk, J[kk]) ;
        */

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

        // get kind
        kind = 0 ;
        if (fkind > 0)
        {
            p = mxGetFieldByNumber (pargin [1], k, fkind) ;
            kind = (int) mxGetScalar (p) ;
        }

        // restore malloc debugging to test the method
        GB_Global.malloc_debug = save ;   // ]

        //----------------------------------------------------------------------
        // C<Mask>(I,J) = A
        //----------------------------------------------------------------------

        info = assign ( ) ;

        GB_MATRIX_FREE (&A) ;
        GB_MATRIX_FREE (&Mask) ;
        GrB_free (&desc) ;

        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }
    }

    ASSERT_OK (GB_check (C, "Final C before wait", GB0)) ;
    OK (GrB_wait ( )) ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GB_mex_assign mexFunction
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

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || !
        (nargin == 2 || nargin == 6 || nargin == 7 || nargin == 8))
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
        int fkind = mxGetFieldNumber (pargin [1], "kind") ;

        if (fA < 0 || fI < 0 || fJ < 0) mexErrMsgTxt ("A,I,J required") ;

        METHOD (many_assign (nwork, fA, fI, fJ, faccum, fMask, fdesc, cclass,
            fkind, pargin)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C<Mask>(I,J) = A, with a single assignment
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

        // get kind (0: matrix/vector, 1: col_assign, 2: row_assign)
        kind = 0 ;
        if (nargin > 7)
        {
            kind = (int) mxGetScalar (pargin [7]) ;
        }

        // C<Mask>(I,J) = A

        METHOD (assign ( )) ;
    }

    //--------------------------------------------------------------------------
    // return C to MATLAB as a struct
    //--------------------------------------------------------------------------

    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C assign result", true) ;
    FREE_ALL ;
}

