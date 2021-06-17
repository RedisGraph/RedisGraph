//------------------------------------------------------------------------------
// GB_mex_about2: more basic tests
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Test lots of random stuff.  The function otherwise serves no purpose.

#include "GB_mex.h"
#include "GB_mex_errors.h"
#include "GB_ij.h"

#define USAGE "GB_mex_about2"

typedef struct
{
    int gunk [16] ;
}
wild ;

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;
    GrB_Matrix A = NULL, B = NULL, C = NULL ;
    GxB_Scalar scalar = NULL ;
    GrB_Vector victor = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Type Wild = NULL ;

    //--------------------------------------------------------------------------
    // startup GraphBLAS
    //--------------------------------------------------------------------------

    bool malloc_debug = GB_mx_get_global (true) ;
    FILE *f = fopen ("errlog3.txt", "w") ;
    int expected = GrB_SUCCESS ;

    //--------------------------------------------------------------------------
    // test removeElement/setElement when jumbled
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_INT32, 10, 10)) ;
    OK (GrB_Vector_new (&victor, GrB_INT32, 10)) ;
    OK (GxB_Vector_Option_set_(victor, GxB_BITMAP_SWITCH, 2.0)) ;
    OK (GxB_Scalar_new (&scalar, GrB_INT32)) ;

    OK (GxB_Matrix_fprint (A, "A before set", 3, NULL)) ;
    OK (GrB_Matrix_setElement_INT32 (A, 314159, 0, 0)) ;
    OK (GxB_Matrix_fprint (A, "A after set", 3, NULL)) ;
    A->jumbled = true ;
    OK (GrB_Matrix_removeElement (A, 0, 0)) ;
    OK (GxB_Matrix_fprint (A, "A after remove", 3, NULL)) ;
    A->jumbled = true ;
    OK (GrB_Matrix_setElement_INT32 (A, 99099, 0, 0)) ;
    OK (GxB_Matrix_fprint (A, "A after set again", 3, NULL)) ;

    OK (GxB_Vector_fprint (victor, "victor before set", 3, NULL)) ;
    OK (GrB_Vector_setElement_INT32 (victor, 44, 0)) ;
    OK (GxB_Vector_fprint (victor, "victor after set", 3, NULL)) ;
    victor->jumbled = true ;
    OK (GrB_Vector_removeElement (victor, 0)) ;
    OK (GxB_Vector_fprint (victor, "victor remove set", 3, NULL)) ;
    victor->jumbled = true ;
    OK (GrB_Vector_setElement_INT32 (victor, 88, 0)) ;
    OK (GxB_Vector_fprint (victor, "victor after set again", 3, NULL)) ;

    OK (GxB_Scalar_fprint (scalar, "scalar before set", 3, NULL)) ;
    OK (GxB_Scalar_setElement_INT32 (scalar, 404)) ;
    OK (GxB_Scalar_fprint (scalar, "scalar after set", 3, NULL)) ;
    int i = 0 ;
    OK (GxB_Scalar_extractElement_INT32 (&i, scalar)) ;
    CHECK (i == 404) ;
    OK (GxB_Scalar_fprint (scalar, "scalar after extract", 3, NULL)) ;
    OK (GrB_Matrix_removeElement ((GrB_Matrix) scalar, 0, 0)) ;
    OK (GxB_Scalar_fprint (scalar, "scalar after remove", 3, NULL)) ;
    i = 777 ;
    expected = GrB_NO_VALUE ;
    ERR (GxB_Scalar_extractElement_INT32 (&i, scalar)) ;
    CHECK (i == 777) ;

    // force a zombie into the scalar
    OK (GxB_Scalar_setElement_INT32 (scalar, 707)) ;
    OK (GxB_Scalar_wait (&scalar)) ;
    OK (GxB_Scalar_fprint (scalar, "scalar after wait", 3, NULL)) ;
    OK (GxB_Matrix_Option_set (scalar, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    CHECK (scalar->i != NULL) ;
    scalar->i [0] = GB_FLIP (0) ;
    scalar->nzombies = 1 ;
    OK (GxB_Scalar_fprint (scalar, "scalar with zombie", 3, NULL)) ;
    expected = GrB_NO_VALUE ;
    ERR (GxB_Scalar_extractElement_INT32 (&i, scalar)) ;
    OK (GxB_Scalar_fprint (scalar, "scalar after extract", 3, NULL)) ;
    CHECK (i == 777) ;

    GrB_Vector_free_(&victor) ;
    GrB_Matrix_free_(&A) ;
    GxB_Scalar_free_(&scalar) ;

    //--------------------------------------------------------------------------
    // builtin comparators not defined for complex types
    //--------------------------------------------------------------------------

    int n = 10 ;
    OK (GrB_Matrix_new (&A, GxB_FC32, n, n)) ;
    OK (GxB_Matrix_Option_set_(A, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;

    OK (GrB_Matrix_new (&C, GxB_FC32, n, n)) ;
    OK (GxB_Scalar_new (&scalar, GxB_FC32)) ;
    expected = GrB_DOMAIN_MISMATCH ;
    ERR (GxB_Matrix_select (C, NULL, NULL, GxB_LT_THUNK, A, scalar, NULL)) ;
    char *message = NULL ;
    OK (GrB_Matrix_error (&message, C)) ;
    printf ("error expected: %s\n", message) ;
    GrB_Matrix_free_(&C) ;
    GxB_Scalar_free_(&scalar) ;

    //--------------------------------------------------------------------------
    // GB_pslice
    //--------------------------------------------------------------------------

    int64_t *Slice = NULL ;
    GB_pslice (&Slice, A->p, n, 2, true) ;
    CHECK (Slice [0] == 0) ;
    GB_FREE (Slice) ;

    int64_t Ap [11] = { 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1 } ;
    bool ok = GB_pslice (&Slice, Ap, 10, 10, false) ;
    printf ("Slice: ") ;
    for (int k = 0 ; k <= 10 ; k++) printf (" %ld", Slice [k]) ;
    printf ("\n") ;
    GB_FREE (Slice) ;

    GrB_Matrix_free_(&A) ;

    //--------------------------------------------------------------------------
    // GrB_Matrix_check
    //--------------------------------------------------------------------------

    double bswitch = 1 ;
    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GxB_Matrix_Option_set_(A, GxB_BITMAP_SWITCH, 0.125)) ;
    OK (GxB_Matrix_Option_get_(A, GxB_BITMAP_SWITCH, &bswitch)) ;
    CHECK (fabsf (bswitch - 0.125) < 1e-5) ;

    OK (GxB_Matrix_Option_set_(A, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    OK (GrB_Matrix_assign_INT32 (A, NULL, NULL, 3, GrB_ALL, n, GrB_ALL, n,
        NULL)) ;
    OK (GrB_Matrix_wait (&A)) ;
    OK (GxB_Matrix_fprint (A, "valid matrix", GxB_SHORT, NULL)) ;
    // mangle the matrix
    GB_FREE (A->p) ;
    GB_FREE (A->x) ;
    expected = GrB_INVALID_OBJECT ;
    ERR (GxB_Matrix_fprint (A, "invalid sparse matrix", GxB_SHORT, NULL)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    A->sparsity = 999 ;
    ERR (GxB_Matrix_fprint (A, "invalid sparsity control", GxB_SHORT, NULL)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GrB_Matrix_assign_INT32 (A, NULL, NULL, 3, GrB_ALL, n, GrB_ALL, n,
        NULL)) ;
    OK (GrB_Matrix_wait (&A)) ;

    A->jumbled = true ;
    ERR (GxB_Matrix_fprint (A, "full matrix cannot be jumbled", GxB_SHORT,
        NULL)) ;

    A->jumbled = false ;
    A->plen = 999 ;
    ERR (GxB_Matrix_fprint (A, "invalid full matrix", GxB_SHORT, NULL)) ;

    A->plen = -1 ;
    A->nzombies = 1 ;
    ERR (GxB_Matrix_fprint (A, "full matrix cannot have zombies",
        GxB_SHORT, NULL)) ;
    A->nzombies = 0 ;
    CHECK (GB_Pending_alloc (&(A->Pending), GrB_INT32, NULL, true, 4)) ;
    ERR (GxB_Matrix_fprint (A, "full matrix cannot have pending tuples",
        GxB_SHORT, NULL)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GxB_Matrix_Option_set_(A, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
    A->plen = 999 ;
    ERR (GxB_Matrix_fprint (A, "invalid bitmap", GxB_SHORT, NULL)) ;

    A->plen = -1 ;
    A->b [0] = 1 ;
    ERR (GxB_Matrix_fprint (A, "invalid bitmap", GxB_SUMMARY, NULL)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GxB_Matrix_Option_set_(A, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
    OK (GrB_Matrix_setElement_INT32 (A, 12345, 0, 0)) ;
    OK (GxB_Matrix_fprint (A, "valid matrix", GxB_SHORT, NULL)) ;
    A->b [0] = 3 ;
    ERR (GxB_Matrix_fprint (A, "invalid bitmap", GxB_SHORT, NULL)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    A->nvec_nonempty = 2 ;
    ERR (GxB_Matrix_fprint (A, "invalid nvec_nonempty", GxB_SHORT, NULL)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GrB_Matrix_setElement_INT32 (A, 12345, 0, 0)) ;
    OK (GxB_Matrix_fprint (A, "valid matrix with 1 pending", GxB_SHORT, NULL)) ;
    A->Pending->size = 900 ;
    ERR (GxB_Matrix_fprint (A, "invalid pending type", GxB_SHORT, NULL)) ;
    GrB_Matrix_free_(&A) ;

    //--------------------------------------------------------------------------
    // lo:stride:hi with stride of zero
    //--------------------------------------------------------------------------

    OK (GxB_Global_Option_set_(GxB_BURBLE, true)) ;
    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    GrB_Index I [3] = { 1, 1, 0 } ;
    OK (GrB_Matrix_new (&C, GrB_INT32, n, 0)) ;
    OK (GrB_Matrix_extract_(C, NULL, NULL, A, GrB_ALL, n, I, GxB_STRIDE,
        NULL)) ;
    OK (GxB_Matrix_fprint (C, "C = A (:,1:0:1)", GxB_COMPLETE, NULL)) ;
    GrB_Matrix_free_(&C) ;
    OK (GrB_Matrix_new (&C, GrB_INT32, 0, n)) ;
    OK (GrB_Matrix_extract_(C, NULL, NULL, A, I, GxB_STRIDE, GrB_ALL, n,
        NULL)) ;
    OK (GxB_Matrix_fprint (C, "C = A (1:0:1,:)", GxB_COMPLETE, NULL)) ;
    GrB_Matrix_free_(&C) ;
    GrB_Matrix_free_(&A) ;
    OK (GxB_Global_Option_set_(GxB_BURBLE, false)) ;

    int64_t Icolon [3] = { 1, 1, 0 } ;
    CHECK (!GB_ij_is_in_list (NULL, 0, 0, GB_STRIDE, Icolon)) ;

    //--------------------------------------------------------------------------
    // GB_aliased
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GrB_Matrix_setElement_INT32 (A, 12345, 0, 0)) ;
    OK (GrB_Matrix_dup (&C, A)) ;
    CHECK (!GB_aliased (A, C)) ;
    GB_FREE (C->p) ;
    C->p = A->p ;
    C->p_shallow = true ;
    CHECK (GB_aliased (A, C)) ;
    C->p = NULL ;
    C->p_shallow = false ;
    CHECK (!GB_aliased (A, C)) ;
    GB_FREE (C->i) ;
    C->i = A->i ;
    C->i_shallow = true ;
    CHECK (GB_aliased (A, C)) ;
    C->i = NULL ;
    C->i_shallow = false ;
    GrB_Matrix_free_(&A) ;
    GrB_Matrix_free_(&C) ;

    //--------------------------------------------------------------------------
    // GrB_apply with empty scalar
    //--------------------------------------------------------------------------

    OK (GxB_Scalar_new (&scalar, GrB_INT32)) ;
    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GrB_Matrix_new (&C, GrB_INT32, n, n)) ;
    expected = GrB_INVALID_VALUE ;
    ERR (GxB_Matrix_apply_BinaryOp2nd (C, NULL, NULL, GrB_PLUS_INT32, A,
        scalar, NULL)) ;
    OK (GrB_Matrix_error (&message, C)) ;
    printf ("error expected: %s\n", message) ;
    GrB_Matrix_free_(&A) ;
    GrB_Matrix_free_(&C) ;
    GxB_Scalar_free_(&scalar) ;

    //--------------------------------------------------------------------------
    // invalid descriptor
    //--------------------------------------------------------------------------

    int method ;
    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_Descriptor_fprint (desc, "descriptor", GxB_COMPLETE, NULL)) ;

    OK (GxB_Desc_get (NULL, GxB_AxB_METHOD, &method)) ;
    CHECK (method == GxB_DEFAULT) ;
    OK (GxB_Desc_set (desc, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
    OK (GxB_Descriptor_fprint (desc, "descriptor", GxB_COMPLETE, NULL)) ;
    OK (GxB_Desc_get (desc, GxB_AxB_METHOD, &method)) ;
    CHECK (method == GxB_AxB_GUSTAVSON) ;

    desc->mask = GrB_REPLACE ;
    expected = GrB_INVALID_OBJECT ;
    ERR (GxB_Descriptor_fprint (desc, "invalid", GxB_COMPLETE, NULL)) ;
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // GrB_build an empty matrix
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GrB_Matrix_build_INT32 (A, I, I, I, 0, GrB_PLUS_INT32)) ;
    OK (GxB_Matrix_fprint (A, "empty", GxB_COMPLETE, NULL)) ;
    CHECK (!GB_is_shallow (A)) ;
    GrB_Matrix_free_(&A) ;

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    expected = GrB_DOMAIN_MISMATCH ;
    ERR (GrB_Matrix_build_INT32 (A, I, I, I, 0, GxB_FIRSTI_INT32)) ;
    OK (GrB_Matrix_error (&message, A)) ;
    printf ("error expected: %s\n", message) ;
    GrB_Matrix_free_(&A) ;

    //--------------------------------------------------------------------------
    // reduce with positional op
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_INT32, n, n)) ;
    OK (GrB_Vector_new (&victor, GrB_INT32, n)) ;
    OK (GxB_Vector_Option_get_(victor, GxB_BITMAP_SWITCH, &bswitch)) ;
    printf ("vector bitmap switch: %g\n\n", bswitch) ;

    expected = GrB_DOMAIN_MISMATCH ;
    ERR (GrB_Matrix_reduce_BinaryOp (victor, NULL, NULL, GxB_FIRSTI_INT32,
        A, NULL)) ;
    OK (GrB_Matrix_error (&message, victor)) ;
    printf ("error expected: %s\n", message) ;
    GrB_Matrix_free_(&A) ;
    GrB_Vector_free_(&victor) ;

    //--------------------------------------------------------------------------
    // GrB_init
    //--------------------------------------------------------------------------

    expected = GrB_INVALID_VALUE ;
    ERR (GrB_init (GrB_BLOCKING)) ;

    //--------------------------------------------------------------------------
    // jumbled user-defined matrix
    //--------------------------------------------------------------------------

    wild w ;
    n = 3 ;
    memset (w.gunk, 13, 16 * sizeof (int)) ;
    OK (GrB_Type_new (&Wild, sizeof (wild))) ;
    OK (GrB_Matrix_new (&C, Wild, n, n)) ;
    OK (GxB_Matrix_Option_set (C, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    OK (GrB_Matrix_assign_UDT (C, NULL, NULL, &w, GrB_ALL, n, GrB_ALL, n,
        NULL)) ;
    OK (GxB_Matrix_fprint (C, "wild matrix", GxB_SHORT, NULL)) ;

    // jumble the matrix
    C->jumbled = true ;
    C->i [0] = 1 ;
    C->i [1] = 0 ;
    OK (GxB_Matrix_fprint (C, "wild matrix jumbled", GxB_SHORT, NULL)) ;

    // unjumble the matrix
    OK (GrB_Matrix_wait (&C)) ;
    OK (GxB_Matrix_fprint (C, "wild matrix unjumbled", GxB_SHORT, NULL)) ;

    GrB_Matrix_free_(&C) ;
    GrB_Type_free_(&Wild) ;

    //--------------------------------------------------------------------------
    // malloc/realloc wrappers
    //--------------------------------------------------------------------------

    ok = false ;
    int *p = GB_malloc_memory (4, sizeof (int)) ;
    CHECK (p != NULL) ;
    p = GB_realloc_memory (4, 4, sizeof (int), p, &ok) ;
    CHECK (p != NULL) ;
    CHECK (ok) ;
    GB_free_memory (p) ;
    p = NULL ;

    //--------------------------------------------------------------------------
    // try to import a huge full matrix (this will fail):
    //--------------------------------------------------------------------------

    GrB_Matrix X = NULL ;
    info = GxB_Matrix_import_FullC (&X, GrB_FP32, GxB_INDEX_MAX, GxB_INDEX_MAX,
        NULL, UINT64_MAX, NULL) ;
    if (info != GrB_INVALID_VALUE || X != NULL) mexErrMsgTxt ("huge fail1") ;

    GrB_Index nhuge = (((GrB_Index) 2) << 50) ;
    info = GxB_Matrix_import_BitmapC (&X, GrB_FP32, nhuge, nhuge,
        NULL, NULL, 0, 0, 0, NULL) ;
    if (info != GrB_INVALID_VALUE || X != NULL) mexErrMsgTxt ("huge fail5") ;

    // try to convert a huge sparse matrix to bitmap (this will fail too):
    info = GrB_Matrix_new (&X, GrB_FP32, nhuge, nhuge) ;
    if (info != GrB_SUCCESS) mexErrMsgTxt ("huge fail2") ;
    info = GxB_Matrix_Option_set_(X, GxB_SPARSITY_CONTROL, GxB_BITMAP) ;
    if (info != GrB_OUT_OF_MEMORY) mexErrMsgTxt ("huge fail3") ;
    info = GB_convert_to_full (X) ;
    if (info != GrB_OUT_OF_MEMORY) mexErrMsgTxt ("huge fail4") ;
    GrB_Matrix_free (&X) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    GB_mx_put_global (true) ;   
    fclose (f) ;
    printf ("\nAll errors printed above were expected.\n") ;
    printf ("GB_mex_about2: all tests passed\n\n") ;
}

