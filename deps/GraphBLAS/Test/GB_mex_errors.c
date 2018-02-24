//------------------------------------------------------------------------------
// GB_mex_errors: test error handling
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This mexFunction intentionally creates many errors for GraphBLAS, to test
// error-handling.  Many error messages are printed.  If the test ends with
// "All tests passed" then all errors were expected.  The test fails if
// GraphBLAS does not catch the error with the right error code, or if it
// generates an unexpected error.

#include "GB_mex.h"

#define FREE_ALL                                                \
{                                                               \
    GrB_free (&Empty1) ;       CHECK (Empty1       == NULL) ;      \
    GrB_free (&A) ;            CHECK (A            == NULL) ;      \
    GrB_free (&B) ;            CHECK (B            == NULL) ;      \
    GrB_free (&C) ;            CHECK (C            == NULL) ;      \
    GrB_free (&Z) ;            CHECK (Z            == NULL) ;      \
    GrB_free (&E) ;            CHECK (E            == NULL) ;      \
    GrB_free (&F) ;            CHECK (F            == NULL) ;      \
    GrB_free (&H) ;            CHECK (H            == NULL) ;      \
    GrB_free (&Agunk) ;        CHECK (Agunk        == NULL) ;      \
    GrB_free (&Aempty) ;       CHECK (Aempty       == NULL) ;      \
    GrB_free (&T) ;            CHECK (T            == NULL) ;      \
    GrB_free (&Tgunk) ;        CHECK (Tgunk        == NULL) ;      \
    GrB_free (&op1) ;          CHECK (op1          == NULL) ;      \
    GrB_free (&op1gunk) ;      CHECK (op1gunk      == NULL) ;      \
    GrB_free (&op2) ;          CHECK (op2          == NULL) ;      \
    GrB_free (&op3) ;          CHECK (op3          == NULL) ;      \
    GrB_free (&op1b) ;         CHECK (op1b         == NULL) ;      \
    GrB_free (&op2b) ;         CHECK (op2b         == NULL) ;      \
    GrB_free (&monoidb) ;      CHECK (monoidb      == NULL) ;      \
    GrB_free (&semiringb) ;    CHECK (semiringb    == NULL) ;      \
    GrB_free (&descb) ;        CHECK (descb        == NULL) ;      \
    GrB_free (&vb) ;           CHECK (vb           == NULL) ;      \
    GrB_free (&op2gunk) ;      CHECK (op2gunk      == NULL) ;      \
    GrB_free (&monoid) ;       CHECK (monoid       == NULL) ;      \
    GrB_free (&monoid_gunk) ;  CHECK (monoid_gunk  == NULL) ;      \
    GrB_free (&semiring) ;     CHECK (semiring     == NULL) ;      \
    GrB_free (&semigunk) ;     CHECK (semigunk     == NULL) ;      \
    GrB_free (&v) ;            CHECK (v            == NULL) ;      \
    GrB_free (&w) ;            CHECK (w            == NULL) ;      \
    GrB_free (&u) ;            CHECK (u            == NULL) ;      \
    GrB_free (&z) ;            CHECK (z            == NULL) ;      \
    GrB_free (&h) ;            CHECK (h            == NULL) ;      \
    GrB_free (&vgunk) ;        CHECK (vgunk        == NULL) ;      \
    GrB_free (&vempty) ;       CHECK (vempty       == NULL) ;      \
    GrB_free (&desc) ;         CHECK (desc         == NULL) ;      \
    GrB_free (&dtn) ;          CHECK (dtn          == NULL) ;      \
    GrB_free (&dnt) ;          CHECK (dnt          == NULL) ;      \
    GrB_free (&dtt) ;          CHECK (dtt          == NULL) ;      \
    GrB_free (&dgunk) ;        CHECK (dgunk        == NULL) ;      \
    GrB_free (&selectop) ;     CHECK (selectop     == NULL) ;      \
    GrB_free (&selectopgunk) ; CHECK (selectopgunk == NULL) ;      \
    GB_mx_put_global (malloc_debug) ;                           \
}

#define FAIL(s)                                             \
{                                                           \
    fprintf (f,"\ntest failure: line %d\n", __LINE__) ;     \
    fprintf (f,"%s\n", GB_STR(s)) ;                         \
    fclose (f) ;                                            \
    mexErrMsgTxt (GB_STR(s) " line: " GB_XSTR(__LINE__)) ;  \
}

#define CHECK(x)    if (!(x)) FAIL(x) ;
#define CHECK2(x,s) if (!(x)) FAIL(s) ;

// assert that a method should return a particular error code
#define ERR(method)                                         \
{                                                           \
    info = method ;                                         \
    fprintf (f,"GB_mex_errors, line %d:", __LINE__) ;       \
    fprintf (f,"%s\n", GrB_error ( )) ;                     \
    CHECK2 (info == expected, method) ;                     \
}

// assert that a method should succeed
#define OK(method)                                          \
{                                                           \
    info = method ;                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))    \
    {                                                       \
        fprintf (f,"%s\n", GrB_error ( )) ;                 \
        printf ("%s\n", GrB_error ( )) ;                    \
        FAIL (method) ;                                     \
    }                                                       \
}

void f1 (double *z, uint32_t *x)
{ 
    (*z) = (*x) + 1 ;
}

void f2 (int32_t *z, uint8_t *x, int16_t *y)
{
    (*z) = (*x) + (*y) + 1 ;
}

void f3 (double complex *z, double complex *x, double *y)
{
    (*z) = (*x) + CMPLX (0,(*y))  ;
}

bool fselect (const GrB_Index i, const GrB_Index j, const GrB_Index nrows,
    const GrB_Index ncols, const double *x, const double *k)
{
    // select entries in triu(A) that are greater than k
    int64_t ii = (int64_t) i ;
    int64_t jj = (int64_t) j ;
    return (x > k && (jj-ii) > 0) ;
}


void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    FILE *f = fopen ("errlog.txt", "w") ;

    GrB_Info info, expected  ;
    OK (GrB_init (GrB_NONBLOCKING)) ;

    fprintf (f,"\n========================================================\n") ;
    fprintf (f,"=== GB_mex_errors : testing error handling =============\n") ;
    fprintf (f,"========================================================\n") ;
    fprintf (f,"many errors are expected\n") ;

    GxB_Statistics stats ;
    int64_t nmalloc ;
    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;

    printf ("nmalloc %d at start\n", nmalloc) ;
    bool malloc_debug = GB_mx_get_global ( ) ;
    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d after complex init\n", nmalloc) ;

    GrB_Matrix A = NULL, B = NULL, C = NULL, Z = NULL, Agunk = NULL,
               Aempty = NULL, E = NULL, F = NULL, A0 = NULL, H = NULL,
               Empty1 = NULL ;
    GrB_Vector v = NULL, vgunk = NULL, vempty = NULL, w = NULL, u = NULL,
               v0 = NULL, v2 = NULL, z = NULL, h = NULL, vb = NULL ;
    GrB_Type T = NULL, Tgunk ;
    GrB_UnaryOp op1 = NULL, op1gunk = NULL, o1 = NULL, op1b = NULL ;
    GrB_BinaryOp op2 = NULL, op2gunk = NULL, o2 = NULL, op0 = NULL, op3 = NULL,
                op2b = NULL ;
    GrB_Monoid monoid = NULL, monoid_gunk = NULL, m2 = NULL, m0 = NULL,
        monoidb = NULL ;
    GrB_Semiring semiring = NULL, semigunk = NULL, s2 = NULL, s0 = NULL,
        semiringb = NULL ;
    GrB_Descriptor desc = NULL, dgunk = NULL, d0 = NULL,
        dnt = NULL, dtn = NULL, dtt = NULL, descb = NULL ;
    GrB_Desc_Value dval ;
    GrB_Index n = 0, nvals = 0, n2 = 0, i = 0, j = 0, a, b, uvals = 0 ;
    GrB_Index *I0 = NULL, *J0 = NULL ;
    #define LEN 100
    GrB_Index I [5] = { 0,   7,   8,   3,    2 },       I2 [LEN] ;
    GrB_Index J [5] = { 4,   1,   2,   2,    1 },       J2 [LEN] ;
    double    X [5] = { 4.5, 8.2, 9.1, -1.2, 3.14159 }, X2 [LEN]  ;

    size_t s ;
    bool        x_bool, ok ;
    int8_t      x_int8 ;
    int16_t     x_int16 ;
    uint16_t    x_uint16 ;
    int32_t     x_int32 ;
    uint32_t    x_uint32 ;
    int64_t     x_int64 ;
    uint64_t    x_uint64 ;
    float       x_float ;
    double      x_double, x = 0 ;
    double complex c ;

    void *pp = NULL ;

    GxB_SelectOp selectop = NULL, selectopgunk = NULL, sel0 ;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (nargout > 0 || nargin > 0)
    {
        mexErrMsgTxt ("Usage: GB_mex_errors") ;
    }

    //--------------------------------------------------------------------------
    // initialize simple_rand
    //--------------------------------------------------------------------------

    fprintf (f, "random seed is %llu\n", simple_rand_getseed ( )) ;
    simple_rand_seed (1) ;
    fprintf (f, "random seed is now %llu\n", simple_rand_getseed ( )) ;

    //--------------------------------------------------------------------------
    // init
    //--------------------------------------------------------------------------

    expected = GrB_INVALID_VALUE ;

    ERR (GrB_init (42)) ;
    OK (GrB_finalize ( )) ;

    //--------------------------------------------------------------------------
    // GB_Mark_* and GB_Work_*
    //--------------------------------------------------------------------------

    CHECK (GB_Mark_alloc (8)) ;
    GB_Mark_reset (INT64_MAX/2, 0) ;
    GB_Mark_reset (INT64_MAX/2, 0) ;
    GB_Mark_reset (INT64_MAX/2, 0) ;
    GB_Mark_free ( ) ;

    CHECK (GB_Work_alloc (1,1)) ;
    CHECK (!GB_Work_alloc (INT64_MAX,8)) ;
    GB_Work_free ( ) ;

    //--------------------------------------------------------------------------
    // Type
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Type_new (NULL, int)) ;
    ERR (GxB_Type_size (NULL, NULL)) ;
    ERR (GxB_Type_size (&s, NULL)) ;

    OK (GrB_Type_new (&T, int)) ;
    CHECK (T != NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    Tgunk = T ;
    T = NULL ;
    Tgunk->magic = 42 ;
    ERR (GxB_Type_size (&s, Tgunk)) ;

    T = GrB_INT32 ;
    OK (GrB_Type_free (&GrB_INT32)) ;
    CHECK (GrB_INT32 == T) ;
    T = NULL ;

    OK (GrB_Type_new (&T, int)) ;
    CHECK (T != NULL) ;

    OK (GxB_Type_size (&s, T)) ;
    CHECK (s == sizeof (int)) ;
    s = 0 ;

    OK (GrB_Type_free (&T)) ;
    CHECK (T == NULL) ;

    OK (GrB_Type_free (&T)) ;
    CHECK (T == NULL) ;

    s = GB_Type_size (-1,1) ;
    CHECK (s == 0) ;

    // Tgunk is allocated but uninitialized

    //--------------------------------------------------------------------------
    // UnaryOp
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_UnaryOp_new (NULL, NULL, NULL, NULL)) ;

    ERR (GrB_UnaryOp_new (&op1, NULL, NULL, NULL)) ;
    CHECK (op1 == NULL) ;

    ERR (GrB_UnaryOp_new (&op1, f1, NULL, NULL)) ;
    CHECK (op1 == NULL) ;

    ERR (GrB_UnaryOp_new (&op1, f1, GrB_FP64, NULL)) ;
    CHECK (op1 == NULL) ;

    ERR (GrB_UnaryOp_new (&op1, f1, NULL, GrB_UINT32)) ;
    CHECK (op1 == NULL) ;

    ERR (GrB_UnaryOp_new (&op1, NULL, GrB_FP64, GrB_UINT32)) ;
    CHECK (op1 == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_UnaryOp_new (&op1, f1, Tgunk, Tgunk)) ;
    CHECK (op1 == NULL) ;

    ERR (GrB_UnaryOp_new (&op1, f1, GrB_FP64, Tgunk)) ;
    CHECK (op1 == NULL) ;

    ERR (GrB_UnaryOp_new (&op1, f1, Tgunk, GrB_FP64)) ;
    CHECK (op1 == NULL) ;

    OK (GrB_UnaryOp_new (&op1, f1, GrB_FP64, GrB_UINT32)) ;
    CHECK (op1 != NULL) ;

    expected = GrB_NULL_POINTER ;
    T = NULL ;

    ERR (GxB_UnaryOp_ztype (NULL, op1)) ;
    ERR (GxB_UnaryOp_xtype (NULL, op1)) ;

    ERR (GxB_UnaryOp_ztype (&T, NULL)) ;
    CHECK (T == NULL) ;

    ERR (GxB_UnaryOp_xtype (&T, NULL)) ;
    CHECK (T == NULL) ;

    OK (GxB_UnaryOp_ztype (&T, op1)) ;
    CHECK (T == GrB_FP64) ;

    OK (GxB_UnaryOp_xtype (&T, op1)) ;
    CHECK (T == GrB_UINT32) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    op1gunk = op1 ;
    op1 = NULL ;

    op1gunk->magic = 99 ;
    T = NULL ;

    ERR (GxB_UnaryOp_ztype (&T, op1gunk)) ;
    CHECK (T == NULL) ;

    ERR (GxB_UnaryOp_xtype (&T, op1gunk)) ;
    CHECK (T == NULL) ;

    o1 = GrB_IDENTITY_BOOL ;
    OK (GrB_UnaryOp_free (&o1)) ;
    CHECK (o1 == GrB_IDENTITY_BOOL) ;
    o1 = NULL ;

    OK (GrB_UnaryOp_new (&o1, f1, GrB_FP64, GrB_UINT32)) ;
    CHECK (o1 != NULL) ;

    OK (GrB_UnaryOp_free (&o1)) ;
    o1 = NULL ;

    OK (GrB_UnaryOp_free (&o1)) ;
    o1 = NULL ;

    // op1gunk is allocated but uninitialized

    //--------------------------------------------------------------------------
    // BinaryOp
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;
    ERR (GrB_BinaryOp_new (NULL, NULL, NULL, NULL, NULL)) ;

    ERR (GrB_BinaryOp_new (&op2, NULL, NULL, NULL, NULL)) ;
    CHECK (op2 == NULL) ;

    // void f2 (int32_t *z, uint8_t *x, int16_t *y)
    ERR (GrB_BinaryOp_new (&op2, f2, NULL, NULL, NULL)) ;
    CHECK (op2 == NULL) ;

    ERR (GrB_BinaryOp_new (&op2, f2, GrB_INT32, NULL, NULL)) ;
    CHECK (op2 == NULL) ;

    ERR (GrB_BinaryOp_new (&op2, f2, GrB_INT32, GrB_UINT8, NULL)) ;
    CHECK (op2 == NULL) ;

    ERR (GrB_BinaryOp_new (&op2, f2, GrB_INT32, NULL, GrB_INT16)) ;
    CHECK (op2 == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_BinaryOp_new (&op2, f2, Tgunk, GrB_UINT8, GrB_INT16)) ;
    CHECK (op2 == NULL) ;

    ERR (GrB_BinaryOp_new (&op2, f2, GrB_INT32, Tgunk, GrB_INT16)) ;
    CHECK (op2 == NULL) ;

    ERR (GrB_BinaryOp_new (&op2, f2, GrB_INT32, GrB_UINT8, Tgunk)) ;
    CHECK (op2 == NULL) ;

    OK (GrB_BinaryOp_new (&op2, f2, GrB_INT32, GrB_UINT8, GrB_INT16)) ;
    CHECK (op2 != NULL) ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_BinaryOp_ztype (NULL, op2)) ;
    ERR (GxB_BinaryOp_xtype (NULL, op2)) ;
    ERR (GxB_BinaryOp_ytype (NULL, op2)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    op2gunk = op2 ;
    op2 = NULL ;
    op2gunk->magic = 77 ;
    T = NULL ;

    ERR (GxB_BinaryOp_ztype (&T, op2gunk)) ;
    CHECK (T == NULL) ;

    ERR (GxB_BinaryOp_xtype (&T, op2gunk)) ;
    CHECK (T == NULL) ;

    ERR (GxB_BinaryOp_ytype (&T, op2gunk)) ;
    CHECK (T == NULL) ;

    o2 = GrB_PLUS_FP64 ;
    OK (GrB_BinaryOp_free (&o2)) ;
    CHECK (o2 == GrB_PLUS_FP64) ;
    o2 = NULL ;

    OK (GrB_BinaryOp_new (&o2, f2, GrB_INT32, GrB_UINT8, GrB_INT16)) ;
    CHECK (o2 != NULL) ;

    OK (GrB_BinaryOp_free (&o2)) ;
    CHECK (o2 == NULL) ;

    OK (GrB_BinaryOp_free (&o2)) ;
    CHECK (o2 == NULL) ;

    //--------------------------------------------------------------------------
    // SelectOp
    //--------------------------------------------------------------------------

    CHECK (selectop == NULL) ;
    OK (GxB_SelectOp_new (&selectop, fselect, GrB_FP64)) ;
    OK (GxB_SelectOp_free (&selectop)) ;
    CHECK (selectop == NULL) ;

    expected = GrB_NULL_POINTER ;

    CHECK (T == NULL) ;
    ERR (GxB_SelectOp_xtype (&T, selectop)) ;
    CHECK (T == NULL) ;

    CHECK (selectop == NULL) ;
    OK (GxB_SelectOp_new (&selectop, fselect, GrB_FP64)) ;

    CHECK (T == NULL) ;
    OK (GxB_SelectOp_xtype (&T, selectop)) ;
    CHECK (T == GrB_FP64) ;
    T = NULL ;

    OK (GxB_SelectOp_free (&selectop)) ;
    CHECK (selectop == NULL) ;

    expected = GrB_NULL_POINTER ;
    ERR (GxB_SelectOp_new (&selectop, NULL, GrB_FP64)) ;
    CHECK (selectop == NULL) ;

    OK (GxB_SelectOp_free (&selectop)) ;
    CHECK (selectop == NULL) ;

    //--------------------------------------------------------------------------
    // Monoid
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Monoid_BOOL_new    (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_INT8_new    (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_UINT8_new   (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_INT16_new   (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_UINT16_new  (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_INT32_new   (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_UINT32_new  (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_INT64_new   (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_UINT64_new  (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_FP32_new    (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_FP64_new    (NULL, NULL, 0)) ;
    ERR (GrB_Monoid_UDT_new     (NULL, NULL, NULL)) ;
    ERR (GrB_Monoid_new         (NULL, NULL, NULL)) ;

    ERR (GrB_Monoid_BOOL_new    (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_INT8_new    (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_UINT8_new   (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_INT16_new   (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_UINT16_new  (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_INT32_new   (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_UINT32_new  (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_INT64_new   (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_UINT64_new  (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_FP32_new    (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_FP64_new    (&monoid, NULL, 0)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_UDT_new     (&monoid, NULL, NULL)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_new         (&monoid, NULL, NULL)) ; CHECK (monoid == NULL);

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Monoid_BOOL_new    (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_INT8_new    (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_UINT8_new   (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_INT16_new   (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_UINT16_new  (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_INT32_new   (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_UINT32_new  (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_INT64_new   (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_UINT64_new  (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_FP32_new    (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);
    ERR (GrB_Monoid_FP64_new    (&monoid, op2gunk, 0)) ; CHECK (monoid == NULL);

    ERR (GrB_Monoid_UDT_new (&monoid, op2gunk, NULL)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_new     (&monoid, op2gunk, NULL)) ; CHECK (monoid == NULL) ;
    ERR (GrB_Monoid_new     (&monoid, op2gunk, 0)) ;    CHECK (monoid == NULL) ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Monoid_UDT_new (&monoid, GrB_PLUS_FP64, NULL)) ;
    CHECK (monoid == NULL) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Monoid_new (&monoid, GrB_EQ_FP64, (double) 0)) ;
    CHECK (monoid == NULL) ;

    // These feel like they should work, but '0' becomes int, and it does not
    // match the type of the operator.  So it is expected to fail with a
    // domain mismatch.
    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, 0)) ;
    CHECK (monoid == NULL) ;

    // likewise, all these fail:
    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (bool) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (int8_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (uint8_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (int16_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (uint16_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (int32_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (uint32_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (int64_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (uint64_t) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (float) 0)) ;
    CHECK (monoid == NULL) ;

    ERR (GrB_Monoid_new (&monoid, GrB_PLUS_FP32, (double) 0)) ;
    CHECK (monoid == NULL) ;

    // this works
    OK (GrB_Monoid_new (&monoid, GrB_PLUS_FP64, (double) 0)) ;
    CHECK (monoid != NULL) ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_Monoid_identity (NULL, NULL)) ;
    ERR (GxB_Monoid_identity (NULL, monoid)) ;
    x_double = 97.0 ;
    ERR (GxB_Monoid_identity (&x_double, NULL)) ;
    CHECK (x_double == 97.0) ;

    OK (GxB_Monoid_identity (&x_double, GxB_TIMES_FP64_MONOID)) ;
    CHECK (x_double == 1.0) ;

    monoid_gunk = monoid ;
    monoid_gunk->magic = 8080 ;
    monoid = NULL ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    x_double = 33. ;
    ERR (GxB_Monoid_identity (&x_double, monoid_gunk)) ;
    CHECK (x_double == 33.) ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_Monoid_operator (NULL, NULL)) ;
    ERR (GxB_Monoid_operator (NULL, GxB_TIMES_FP64_MONOID)) ;
    ERR (GxB_Monoid_operator (&o2, NULL)) ;
    CHECK (o2 == NULL) ;

    OK (GxB_Monoid_operator (&o2, GxB_TIMES_FP64_MONOID)) ;
    CHECK (o2 == GrB_TIMES_FP64) ;

    m2 = GxB_TIMES_FP64_MONOID ;
    OK (GrB_Monoid_free (&m2)) ;
    CHECK (m2 == GxB_TIMES_FP64_MONOID) ;
    m2 = NULL ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    o2 = NULL ;
    ERR (GxB_Monoid_operator (&o2, monoid_gunk)) ;
    CHECK (o2 == NULL) ;

    OK (GrB_Monoid_new (&m2, GrB_PLUS_FP64, (double) 0)) ;
    CHECK (m2 != NULL) ;

    OK (GrB_Monoid_free (&m2)) ;
    CHECK (m2 == NULL) ;

    OK (GrB_Monoid_free (&m2)) ;
    CHECK (m2 == NULL) ;

    // monoid_gunk is allocated but not initialized

    //--------------------------------------------------------------------------
    // Semiring
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Semiring_new (NULL, NULL, NULL)) ;
    ERR (GrB_Semiring_new (NULL, GxB_MAX_FP64_MONOID, GrB_PLUS_FP64)) ;

    ERR (GrB_Semiring_new (&semiring, NULL, GrB_PLUS_FP64)) ;
    CHECK (semiring == NULL) ;

    ERR (GrB_Semiring_new (&semiring, GxB_MAX_FP64_MONOID, NULL)) ;
    CHECK (semiring == NULL) ;

    ERR (GrB_Semiring_new (&semiring, NULL, NULL)) ;
    CHECK (semiring == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Semiring_new (&semiring, monoid_gunk, GrB_PLUS_FP64)) ;
    CHECK (semiring == NULL) ;

    ERR (GrB_Semiring_new (&semiring, GxB_MAX_FP32_MONOID, op2gunk)) ;
    CHECK (semiring == NULL) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Semiring_new (&semiring, GxB_MAX_FP32_MONOID, GrB_PLUS_FP64)) ;
    CHECK (semiring == NULL) ;

    OK (GrB_Semiring_new (&semiring, GxB_MAX_FP64_MONOID, GrB_PLUS_FP64)) ;
    CHECK (semiring != NULL) ;

    o2 = NULL ;
    OK (GxB_Semiring_multiply (&o2, GxB_MAX_PLUS_FP64)) ;
    CHECK (o2 == GrB_PLUS_FP64) ;
    o2 = NULL ;

    m2 = NULL ;
    OK (GxB_Semiring_add (&m2, GxB_MAX_PLUS_FP64)) ;
    CHECK (m2 == GxB_MAX_FP64_MONOID) ;
    m2 = NULL ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_Semiring_multiply (NULL, NULL)) ;
    ERR (GxB_Semiring_multiply (NULL, GxB_MAX_PLUS_FP64)) ;
    ERR (GxB_Semiring_multiply (&o2, NULL)) ;
    CHECK (o2 == NULL) ;

    ERR (GxB_Semiring_add (NULL, NULL)) ;
    ERR (GxB_Semiring_add (NULL, GxB_MAX_PLUS_FP64)) ;
    ERR (GxB_Semiring_add (&m2, NULL)) ;
    CHECK (m2 == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    semigunk = semiring ;
    semigunk->magic = 747 ;
    semiring = NULL ;

    m2 = GxB_MAX_INT32_MONOID ;
    ERR (GxB_Semiring_add (&m2, semigunk)) ;
    CHECK (m2 == GxB_MAX_INT32_MONOID) ;
    m2 = NULL ;

    o2 = GxB_LXOR_BOOL ;
    ERR (GxB_Semiring_multiply (&o2, semigunk)) ;
    CHECK (o2 == GxB_LXOR_BOOL) ;
    o2 = NULL ;

    s2 = GxB_PLUS_TIMES_FP64 ;
    OK (GrB_Semiring_free (&s2)) ;
    CHECK (s2 == GxB_PLUS_TIMES_FP64) ;
    s2 = NULL ;

    OK (GrB_Semiring_new (&s2, GxB_MAX_FP64_MONOID, GrB_PLUS_FP64)) ;
    CHECK (s2 != NULL) ;

    OK (GrB_Semiring_free (&s2)) ;
    CHECK (s2 == NULL) ;

    // semigunk is allocated but not initialized

    //--------------------------------------------------------------------------
    // basic Vector methods
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Vector_new (NULL, NULL, 0)) ;
    ERR (GrB_Vector_new (NULL, GrB_BOOL, 0)) ;
    ERR (GrB_Vector_new (&v, NULL, 0)) ;
    CHECK (v == NULL) ;

    expected = GrB_INVALID_VALUE ;

    ERR (GrB_Vector_new (&v, GrB_BOOL, UINT64_MAX)) ;
    CHECK (v == NULL) ;

    OK (GrB_Vector_new (&vempty, GrB_BOOL, 0)) ;
    CHECK (vempty != NULL) ;

    OK (GrB_Vector_free (&vempty)) ;
    CHECK (vempty == NULL) ;

    OK (GrB_Vector_free (&vempty)) ;
    CHECK (vempty == NULL) ;

    OK (GrB_Vector_new (&vempty, GrB_BOOL, 0)) ;
    CHECK (vempty != NULL) ;

    OK (GrB_Vector_new (&vgunk, GrB_BOOL, 1)) ;
    CHECK (vgunk != NULL) ;
    vgunk->magic = 1024 ;

    OK (GrB_Vector_new (&v, GrB_UINT16, 32)) ;
    CHECK (v != NULL) ;

    OK (GrB_Vector_dup (&u, v)) ;
    CHECK (u != NULL) ;

    OK (GrB_Vector_free (&u)) ;
    CHECK (u == NULL) ;

    OK (GrB_Vector_free (&u)) ;
    CHECK (u == NULL) ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Vector_dup (NULL, NULL)) ;
    ERR (GrB_Vector_dup (NULL, v)) ;
    ERR (GrB_Vector_dup (NULL, vgunk)) ;
    ERR (GrB_Vector_dup (NULL, vempty)) ;
    ERR (GrB_Vector_clear (NULL)) ;

    ERR (GrB_Vector_dup (&u, NULL)) ;
    CHECK (u == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_dup (&u, vgunk)) ;
    CHECK (u == NULL) ;

    OK (GrB_Vector_clear (v)) ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Vector_size (NULL, NULL)) ;
    ERR (GrB_Vector_size (NULL, v)) ;
    ERR (GrB_Vector_size (NULL, vgunk)) ;
    ERR (GrB_Vector_size (NULL, vempty)) ;
    ERR (GrB_Vector_size (&n, NULL)) ;
    CHECK (n == 0) ;

    OK (GrB_Vector_size (&n, v)) ;
    CHECK (n == 32) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    n = 7 ;
    ERR (GrB_Vector_dup (&u, vgunk)) ;
    CHECK (n == 7) ;

    OK (GrB_Vector_setElement (v, 12, 0)) ;
    OK (GrB_Vector_setElement (v, 17, 3)) ;

    expected = GrB_NULL_POINTER ;

    n = 0 ;
    ERR (GrB_Vector_nvals (NULL, NULL)) ;
    ERR (GrB_Vector_nvals (NULL, v)) ;
    ERR (GrB_Vector_nvals (NULL, vempty)) ;
    ERR (GrB_Vector_nvals (&nvals, NULL)) ;
    CHECK (n == 0) ;

    OK (GrB_Vector_nvals (&nvals, v)) ;
    CHECK (nvals == 2) ;

    OK (GrB_Vector_clear (v)) ;
    OK (GrB_Vector_nvals (&nvals, v)) ;
    CHECK (n == 0) ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_Vector_type (NULL, NULL)) ;

    ERR (GxB_Vector_type (NULL, v)) ;
    ERR (GxB_Vector_type (NULL, vempty)) ;
    ERR (GxB_Vector_type (&T, NULL)) ;
    CHECK (T == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_nvals (NULL, vgunk)) ;
    ERR (GxB_Vector_type (NULL, vgunk)) ;
    ERR (GxB_Vector_type (&T, vgunk)) ;
    CHECK (T == NULL) ;

    nvals = 42 ;
    ERR (GrB_Vector_nvals (&nvals, vgunk)) ;
    CHECK (nvals == 42) ;

    OK (GrB_Vector_free (&v)) ;
    CHECK (v == NULL) ;

    // vgunk is allocated but not initialized

    //--------------------------------------------------------------------------
    // Vector build
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Vector_build_BOOL   (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT8   (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT8  (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT16  (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT16 (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT32  (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT32 (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT64  (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT64 (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_FP32   (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_FP64   (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UDT    (NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build        (NULL, NULL, NULL, 0, NULL)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_build_BOOL   (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT8   (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT8  (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT16  (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT16 (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT32  (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT32 (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_INT64  (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UINT64 (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_FP32   (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_FP64   (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build_UDT    (vgunk, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build        (vgunk, NULL, NULL, 0, NULL)) ;

    expected = GrB_NULL_POINTER ;

    OK  (GrB_Vector_new (&v, GrB_FP64, 10)) ;
    ERR (GrB_Vector_build (v, I, NULL, 0, NULL)) ;
    ERR (GrB_Vector_build (v, I, X,    0, NULL)) ;

    expected = GrB_INVALID_VALUE ;

    o2 = GrB_SECOND_FP64 ;
    ERR (GrB_Vector_build (v, GrB_ALL, X, 0, o2)) ;
    ERR (GrB_Vector_build (v, I, X, GB_INDEX_MAX+1, o2)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_build (v, I, X, 5, op2gunk)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Vector_build (v, I, X, 5, GrB_LE_FP64)) ;
    ERR (GrB_Vector_build (v, I, X, 5, Complex_plus)) ;
    ERR (GrB_Vector_build (v, I, (void *) X, 5, GrB_PLUS_FP64)) ;

    expected = GrB_OUTPUT_NOT_EMPTY ;

    OK  (GrB_Vector_setElement (v, 12, 0)) ;
    ERR (GrB_Vector_build (v, I, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Vector_clear (v)) ;
    OK  (GrB_Vector_build (v, I, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Vector_clear (v)) ;

    expected = GrB_INDEX_OUT_OF_BOUNDS ;

    I [0] = 10 ;
    ERR (GrB_Vector_build (v, I, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Vector_nvals (&nvals, v)) ;
    CHECK (nvals == 0) ;

    I [0] = -1 ;
    ERR (GrB_Vector_build (v, I, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Vector_nvals (&nvals, v)) ;
    CHECK (nvals == 0) ;
    I [0] = 0 ;

    // v is a valid 10-by-1 FP64 vector with no entries

    //--------------------------------------------------------------------------
    // Vector setElement
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Vector_setElement_BOOL   (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT8   (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT8  (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT16  (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT16 (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT32  (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT32 (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT64  (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT64 (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_FP32   (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_FP64   (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_UDT    (NULL, 0, 0)) ;
    ERR (GrB_Vector_setElement_UDT    (v, NULL, 0)) ;
    ERR (GrB_Vector_setElement        (NULL, 0, 0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_setElement_BOOL   (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT8   (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT8  (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT16  (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT16 (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT32  (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT32 (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_INT64  (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_UINT64 (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_FP32   (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_FP64   (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement_UDT    (vgunk, 0, 0)) ;
    ERR (GrB_Vector_setElement        (vgunk, 0, 0)) ;

    expected = GrB_INVALID_INDEX ;

    ERR (GrB_Vector_setElement (v, 0, -1)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Vector_setElement (v, (void *) X, 0)) ;

    //--------------------------------------------------------------------------
    // Vector extractElement
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Vector_extractElement_BOOL   (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_INT8   (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_UINT8  (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_INT16  (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_UINT16 (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_INT32  (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_UINT32 (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_INT64  (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_UINT64 (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_FP32   (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_FP64   (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement_UDT    (NULL, NULL, 0)) ;
    ERR (GrB_Vector_extractElement        (NULL, NULL, 0)) ;

    ERR (GrB_Vector_extractElement_BOOL   (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_INT8   (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_UINT8  (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_INT16  (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_UINT16 (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_INT32  (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_UINT32 (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_INT64  (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_UINT64 (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_FP32   (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_FP64   (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement_UDT    (NULL, v, 0)) ;
    ERR (GrB_Vector_extractElement        (NULL, v, 0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_extractElement_BOOL   (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_INT8   (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_UINT8  (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_INT16  (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_UINT16 (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_INT32  (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_UINT32 (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_INT64  (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_UINT64 (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_FP32   (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_FP64   (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement_UDT    (NULL, vgunk, 0)) ;
    ERR (GrB_Vector_extractElement        (NULL, vgunk, 0)) ;

    expected = GrB_INVALID_INDEX ;

    ERR (GrB_Vector_extractElement (&x_double, v, -1)) ;
    ERR (GrB_Vector_extractElement (&x_double, v, 10)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Vector_extractElement ((void *) X, v, 0)) ;

    OK (GrB_Vector_setElement (v, 22.8, 2)) ;
    OK (GrB_Vector_setElement (v, 44.9, 4)) ;

    x_double = 404 ;
    OK (GrB_Vector_extractElement (&x_double, v, 3)) ;
    CHECK (x_double == 404) ;
    CHECK (info == GrB_NO_VALUE) ;
    fprintf (f, "%s\n", GrB_error ()) ;

    OK (GrB_Vector_setElement (v, 77.3, 0)) ;

    OK (GrB_Vector_extractElement (&x_double, v, 0)) ;

    CHECK (info == GrB_SUCCESS) ;
    CHECK (x_double == 77.3) ;

    // v is now a valid FP64 vector with 3 entries

    //--------------------------------------------------------------------------
    // Vector extractTuples
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;
    OK (GrB_Vector_nvals (&n2, v)) ;
    nvals = n2 ;

    ERR (GrB_Vector_extractTuples_BOOL   (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_INT8   (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_UINT8  (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_INT16  (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_UINT16 (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_INT32  (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_UINT32 (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_INT64  (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_UINT64 (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_FP32   (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_FP64   (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples_UDT    (NULL, NULL, NULL, v)) ;
    ERR (GrB_Vector_extractTuples        (NULL, NULL, NULL, v)) ;

    ERR (GrB_Vector_extractTuples_BOOL   (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_INT8   (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_UINT8  (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_INT16  (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_UINT16 (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_INT32  (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_UINT32 (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_INT64  (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_UINT64 (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_FP32   (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_FP64   (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples_UDT    (NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Vector_extractTuples        (NULL, NULL, &nvals, NULL)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Vector_extractTuples_BOOL   (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_INT8   (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_UINT8  (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_INT16  (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_UINT16 (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_INT32  (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_UINT32 (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_INT64  (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_UINT64 (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_FP32   (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_FP64   (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples_UDT    (NULL, NULL, &nvals, vgunk)) ;
    ERR (GrB_Vector_extractTuples        (NULL, NULL, &nvals, vgunk)) ;

    expected = GrB_INSUFFICIENT_SPACE ;

    nvals = n2-1 ;
    ERR (GrB_Vector_extractTuples (I2, X2, &nvals, v)) ;
    nvals = n2 ;
    OK  (GrB_Vector_extractTuples (I2, X2, &nvals, v)) ;

    for (int k = 0 ; k < n2 ; k++)
    {
        fprintf (f, "%d: v("GBu") = %g\n", k, I2 [k], X2 [k]) ;
    }
    fprintf (f, "\n") ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Vector_extractTuples (I2, (void *) X2, &nvals, v)) ;

    GrB_free (&v) ;
    CHECK (v == NULL) ;

    //--------------------------------------------------------------------------
    // basic Matrix methods
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Matrix_new (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_new (NULL, GrB_BOOL, 0, 0)) ;
    ERR (GrB_Matrix_new (&A, NULL, 0, 0)) ;
    CHECK (A == NULL) ;

    expected = GrB_INVALID_VALUE ;

    ERR (GrB_Matrix_new (&A, GrB_BOOL, UINT64_MAX, 0)) ; CHECK (A == NULL) ;
    ERR (GrB_Matrix_new (&A, GrB_BOOL, 0, UINT64_MAX)) ; CHECK (A == NULL) ;

    OK (GrB_Matrix_new (&Aempty, GrB_BOOL, 0, 0)) ;
    CHECK (Aempty != NULL) ;

    OK (GrB_Matrix_free (&Aempty)) ;
    CHECK (Aempty == NULL) ;

    OK (GrB_Matrix_free (&Aempty)) ;
    CHECK (Aempty == NULL) ;

    OK (GrB_Matrix_new (&Aempty, GrB_BOOL, 0, 0)) ;
    CHECK (Aempty != NULL) ;

    OK (GrB_Matrix_new (&Agunk, GrB_BOOL, 1, 1)) ;
    CHECK (Agunk != NULL) ;
    Agunk->magic = 1024 ;

    OK (GrB_Matrix_new (&A, GrB_UINT16, 32, 8)) ;
    CHECK (A != NULL) ;

    OK (GrB_Matrix_dup (&C, A)) ;
    CHECK (C != NULL) ;

    OK (GrB_Matrix_free (&C)) ;
    CHECK (C == NULL) ;

    OK (GrB_Matrix_free (&C)) ;
    CHECK (C == NULL) ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Matrix_dup (NULL, NULL)) ;
    ERR (GrB_Matrix_dup (NULL, A)) ;
    ERR (GrB_Matrix_dup (NULL, Agunk)) ;
    ERR (GrB_Matrix_dup (NULL, Aempty)) ;
    ERR (GrB_Matrix_clear (NULL)) ;

    ERR (GrB_Matrix_dup (&C, NULL)) ;
    CHECK (C == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_nrows (&n, Agunk)) ;
    ERR (GrB_Matrix_ncols (&n, Agunk)) ;
    ERR (GrB_Matrix_nvals (&n, Agunk)) ;
    ERR (GrB_Matrix_dup (&C, Agunk)) ;
    CHECK (C == NULL) ;

    OK (GrB_Matrix_clear (A)) ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Matrix_nrows (NULL, NULL)) ;
    ERR (GrB_Matrix_nrows (NULL, A)) ;
    ERR (GrB_Matrix_nrows (NULL, Agunk)) ;
    ERR (GrB_Matrix_nrows (NULL, Aempty)) ;
    ERR (GrB_Matrix_nrows (&n, NULL)) ;
    CHECK (n == 0) ;

    ERR (GrB_Matrix_ncols (NULL, NULL)) ;
    ERR (GrB_Matrix_ncols (NULL, A)) ;
    ERR (GrB_Matrix_ncols (NULL, Agunk)) ;
    ERR (GrB_Matrix_ncols (NULL, Aempty)) ;
    ERR (GrB_Matrix_ncols (&n, NULL)) ;
    CHECK (n == 0) ;

    OK (GrB_Matrix_nrows (&n, A)) ;
    CHECK (n == 32) ;

    OK (GrB_Matrix_ncols (&n, A)) ;
    CHECK (n == 8) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    n = 7 ;
    ERR (GrB_Matrix_dup (&C, Agunk)) ;
    CHECK (n == 7) ;

    OK (GrB_Matrix_setElement (A, 21, 0, 2)) ;
    OK (GrB_Matrix_setElement (A, 19, 3, 1)) ;

    expected = GrB_INVALID_INDEX ;
    ERR (GrB_Matrix_setElement (A, 19, 3, 1000)) ;

    expected = GrB_NULL_POINTER ;

    n = 0 ;
    ERR (GrB_Matrix_nvals (NULL, NULL)) ;
    ERR (GrB_Matrix_nvals (NULL, A)) ;
    ERR (GrB_Matrix_nvals (NULL, Aempty)) ;
    ERR (GrB_Matrix_nvals (&nvals, NULL)) ;
    CHECK (n == 0) ;

    OK (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 2) ;

    OK (GrB_Matrix_clear (A)) ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (n == 0) ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_Matrix_type (NULL, NULL)) ;

    ERR (GxB_Matrix_type (NULL, A)) ;
    ERR (GxB_Matrix_type (NULL, Aempty)) ;
    ERR (GxB_Matrix_type (&T, NULL)) ;
    CHECK (T == NULL) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_nvals (NULL, Agunk)) ;
    ERR (GxB_Matrix_type (NULL, Agunk)) ;
    ERR (GxB_Matrix_type (&T, Agunk)) ;
    CHECK (T == NULL) ;

    nvals = 42 ;
    ERR (GrB_Matrix_nvals (&nvals, Agunk)) ;
    CHECK (nvals == 42) ;

    OK (GrB_Matrix_free (&A)) ;
    CHECK (A == NULL) ;

    // Agunk is allocated but not initialized

    //--------------------------------------------------------------------------
    // Matrix build
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Matrix_build_BOOL   (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT8   (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT8  (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT16  (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT16 (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT32  (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT32 (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT64  (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT64 (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_FP32   (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_FP64   (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UDT    (NULL, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build        (NULL, NULL, NULL, NULL, 0, NULL)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_build_BOOL   (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT8   (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT8  (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT16  (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT16 (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT32  (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT32 (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_INT64  (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UINT64 (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_FP32   (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_FP64   (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build_UDT    (Agunk, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build        (Agunk, NULL, NULL, NULL, 0, NULL)) ;

    expected = GrB_NULL_POINTER ;

    OK (GrB_Matrix_new (&A, GrB_FP64, 10, 5)) ;
    ERR (GrB_Matrix_build (A, I,    NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build (A, NULL, NULL, NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build (A, I,    J,    NULL, 0, NULL)) ;
    ERR (GrB_Matrix_build (A, I,    J,    X,    0, NULL)) ;

    expected = GrB_INVALID_VALUE ;

    o2 = GrB_SECOND_FP64 ;
    ERR (GrB_Matrix_build (A, GrB_ALL, J, X, 0, o2)) ;
    ERR (GrB_Matrix_build (A, I, GrB_ALL, X, 0, o2)) ;
    ERR (GrB_Matrix_build (A, I,       J, X, GB_INDEX_MAX+1, o2)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_build (A, I, J, X, 5, op2gunk)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Matrix_build (A, I, J,          X, 5, GrB_LE_FP64)) ;
    ERR (GrB_Matrix_build (A, I, J,          X, 5, Complex_plus)) ;
    ERR (GrB_Matrix_build (A, I, J, (void *) X, 5, GrB_PLUS_FP64)) ;

    expected = GrB_OUTPUT_NOT_EMPTY ;

    OK  (GrB_Matrix_setElement (A, 12, 0, 0)) ;
    ERR (GrB_Matrix_build (A, I, J, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Matrix_clear (A)) ;
    OK  (GrB_Matrix_build (A, I, J, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Matrix_clear (A)) ;

    expected = GrB_INDEX_OUT_OF_BOUNDS ;

    I [0] = 10 ;
    ERR (GrB_Matrix_build (A, I, J, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 0) ;

    I [0] = -1 ;
    ERR (GrB_Matrix_build (A, I, J, X, 5, GrB_PLUS_FP64)) ;
    OK  (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 0) ;
    I [0] = 0 ;

    // A is a valid 10-by-5 FP64 matrix with no entries

    //--------------------------------------------------------------------------
    // Matrix setElement
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Matrix_setElement_BOOL   (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT8   (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT8  (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT16  (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT16 (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT32  (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT32 (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT64  (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT64 (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_FP32   (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_FP64   (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UDT    (NULL, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UDT    (A, NULL, 0, 0)) ;
    ERR (GrB_Matrix_setElement        (NULL, 0, 0, 0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_setElement_BOOL   (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT8   (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT8  (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT16  (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT16 (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT32  (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT32 (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_INT64  (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UINT64 (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_FP32   (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_FP64   (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement_UDT    (Agunk, 0, 0, 0)) ;
    ERR (GrB_Matrix_setElement        (Agunk, 0, 0, 0)) ;

    expected = GrB_INVALID_INDEX ;

    ERR (GrB_Matrix_setElement (A, 0, -1, 0)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Matrix_setElement (A, (void *) X, 0, 0)) ;

    //--------------------------------------------------------------------------
    // Matrix extractElement
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Matrix_extractElement_BOOL   (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT8   (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT8  (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT16  (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT16 (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT32  (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT32 (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT64  (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT64 (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_FP32   (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_FP64   (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UDT    (NULL, NULL, 0, 0)) ;
    ERR (GrB_Matrix_extractElement        (NULL, NULL, 0, 0)) ;

    ERR (GrB_Matrix_extractElement_BOOL   (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT8   (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT8  (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT16  (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT16 (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT32  (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT32 (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT64  (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT64 (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_FP32   (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_FP64   (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UDT    (NULL, A, 0, 0)) ;
    ERR (GrB_Matrix_extractElement        (NULL, A, 0, 0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_extractElement_BOOL   (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT8   (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT8  (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT16  (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT16 (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT32  (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT32 (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_INT64  (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UINT64 (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_FP32   (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_FP64   (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement_UDT    (NULL, Agunk, 0, 0)) ;
    ERR (GrB_Matrix_extractElement        (NULL, Agunk, 0, 0)) ;

    expected = GrB_INVALID_INDEX ;

    ERR (GrB_Matrix_extractElement (&x_double, A, -1, 0)) ;
    ERR (GrB_Matrix_extractElement (&x_double, A, 10, 0)) ;
    ERR (GrB_Matrix_extractElement (&x_double, A, 0, 911)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_Matrix_extractElement ((void *) X, A, 0, 0)) ;

    OK (GrB_Matrix_setElement (A, 22.8, 2, 0)) ;
    OK (GrB_Matrix_setElement (A, 44.9, 4, 0)) ;

    x_double = 404 ;
    OK (GrB_Matrix_extractElement (&x_double, A, 3, 0)) ;
    CHECK (x_double == 404) ;
    CHECK (info == GrB_NO_VALUE) ;
    fprintf (f, "%s\n", GrB_error ()) ;

    OK (GrB_Matrix_setElement (A, 77.3, 0, 0)) ;

    OK (GrB_Matrix_extractElement (&x_double, A, 0, 0)) ;

    CHECK (info == GrB_SUCCESS) ;
    CHECK (x_double == 77.3) ;

    OK (GrB_Matrix_nvals (&n2, A)) ;
    fprintf (f, "nvals: %d\n", (int) n2) ;

    // A is now a valid FP64 matrix with 3 entries

    //--------------------------------------------------------------------------
    // Matrix extractTuples
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;
    OK (GrB_Matrix_nvals (&n2, A)) ;
    nvals = n2 ;

    ERR (GrB_Matrix_extractTuples_BOOL   (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_INT8   (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_UINT8  (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_INT16  (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_UINT16 (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_INT32  (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_UINT32 (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_INT64  (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_UINT64 (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_FP32   (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_FP64   (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples_UDT    (NULL, NULL, NULL, NULL, A)) ;
    ERR (GrB_Matrix_extractTuples        (NULL, NULL, NULL, NULL, A)) ;

    ERR (GrB_Matrix_extractTuples_BOOL   (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_INT8   (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_UINT8  (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_INT16  (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_UINT16 (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_INT32  (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_UINT32 (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_INT64  (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_UINT64 (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_FP32   (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_FP64   (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples_UDT    (NULL, NULL, NULL, &nvals, NULL)) ;
    ERR (GrB_Matrix_extractTuples        (NULL, NULL, NULL, &nvals, NULL)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Matrix_extractTuples_BOOL   (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_INT8   (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_UINT8  (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_INT16  (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_UINT16 (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_INT32  (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_UINT32 (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_INT64  (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_UINT64 (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_FP32   (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_FP64   (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples_UDT    (NULL, NULL, NULL, &nvals, Agunk)) ;
    ERR (GrB_Matrix_extractTuples        (NULL, NULL, NULL, &nvals, Agunk)) ;

    expected = GrB_INSUFFICIENT_SPACE ;

    nvals = n2-1 ;
    ERR (GrB_Matrix_extractTuples (I2, J2, X2, &nvals, A)) ;
    nvals = n2 ;
    OK  (GrB_Matrix_extractTuples (I2, J2, X2, &nvals, A)) ;

    for (int k = 0 ; k < n2 ; k++)
    {
        fprintf (f, "%d: A("GBu","GBu") = %g\n", k, I2 [k], J2 [k], X2 [k]) ;
    }
    fprintf (f, "\n") ;

    expected = GrB_DOMAIN_MISMATCH ;

    nvals = n2 ;
    ERR (GrB_Matrix_extractTuples (I2, J2, (void *) X2, &nvals, A)) ;

    GrB_free (&A) ;
    CHECK (A == NULL) ;

    // The following are now allocated; keep them for the rest the tests:
    // Agunk, Tgunk, op1gunk, op2gunk, monoid_gunk, semigunk, Aempty, vempty

    //--------------------------------------------------------------------------
    // Descriptor
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_Descriptor_new (NULL)) ;

    OK (GrB_Descriptor_new (&dgunk)) ;
    CHECK (dgunk != NULL) ;
    dgunk->magic = 22309483 ;

    ERR (GrB_Descriptor_set (NULL, 0, 0)) ;
    ERR (GxB_Descriptor_get (NULL, NULL, 0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_Descriptor_set (dgunk, 0, 0)) ;
    ERR (GxB_Descriptor_get (&dval, dgunk, 0)) ;

    OK (GxB_Descriptor_get (&dval, NULL, 0)) ;
    CHECK (dval == GxB_DEFAULT) ;

    OK (GrB_Descriptor_new (&desc)) ;

    expected = GrB_INVALID_VALUE ;

    ERR (GxB_Descriptor_get (&dval, desc, -1)) ;
    ERR (GrB_Descriptor_set (desc, -1, 0)) ;

    ERR (GrB_Descriptor_set (desc, GrB_OUTP, -1)) ;
    ERR (GrB_Descriptor_set (desc, GrB_MASK, -1)) ;
    ERR (GrB_Descriptor_set (desc, GrB_INP0, -1)) ;
    ERR (GrB_Descriptor_set (desc, GrB_INP1, -1)) ;

    OK (GxB_Descriptor_get (&dval, desc, GrB_OUTP)) ;
    CHECK (dval == GxB_DEFAULT) ;
    OK (GxB_Descriptor_get (&dval, desc, GrB_MASK)) ;
    CHECK (dval == GxB_DEFAULT) ;
    OK (GxB_Descriptor_get (&dval, desc, GrB_INP0)) ;
    CHECK (dval == GxB_DEFAULT) ;
    OK (GxB_Descriptor_get (&dval, desc, GrB_INP1)) ;
    CHECK (dval == GxB_DEFAULT) ;

    // The following are now allocated; keep them for the rest the tests:
    // Agunk, Tgunk, op1gunk, op2gunk, monoid_gunk, semigunk, Aempty, vempty,
    // desc, dgunk

    //--------------------------------------------------------------------------
    // create some valid matrices and vectors
    //--------------------------------------------------------------------------

    OK (random_matrix (&A, false, false, 3, 4, 12, 0, false)) ;
    OK (random_matrix (&B, false, false, 4, 2,  6, 0, false)) ;
    OK (random_matrix (&C, false, false, 3, 2,  4, 0, false)) ;
    OK (random_matrix (&E, false, false, 3, 2,  4, 0, false)) ;
    OK (random_matrix (&F, false, false, 3, 2,  4, 0, false)) ;
    OK (random_matrix (&Z, false, false, 3, 2,  8, 0, true)) ;   // Z complex
    OK (GrB_Vector_new (&v, GrB_FP64, 5)) ;
    OK (GrB_Vector_new (&u, GrB_FP64, 5)) ;
    OK (GrB_Vector_new (&z, Complex, 5)) ;

    OK (GrB_Descriptor_new (&dnt)) ;
    OK (GrB_Descriptor_set (dnt, GrB_INP1, GrB_TRAN)) ;

    OK (GrB_Descriptor_new (&dtn)) ;
    OK (GrB_Descriptor_set (dtn, GrB_INP0, GrB_TRAN)) ;

    OK (GrB_Descriptor_new (&dtt)) ;
    OK (GrB_Descriptor_set (dtt, GrB_INP0, GrB_TRAN)) ;
    OK (GrB_Descriptor_set (dtt, GrB_INP1, GrB_TRAN)) ;

    //--------------------------------------------------------------------------
    // GrB_mxm, mxv, and vxm
    //--------------------------------------------------------------------------

    s2 = GxB_MAX_PLUS_FP32 ;
    o2 = GrB_MAX_FP32 ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_mxm (Agunk,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxm (C    ,  Agunk, NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxm (C    ,  C    , NULL   , NULL    , Agunk, NULL , NULL )) ;
    ERR (GrB_mxm (C    ,  C    , NULL   , NULL    , A    , Agunk, NULL )) ;
    ERR (GrB_mxm (C    ,  C    , NULL   , NULL    , A    , A    , dgunk)) ;
    ERR (GrB_mxm (C    ,  C    , op2gunk, NULL    , A    , A    , NULL )) ;
    ERR (GrB_mxm (C    ,  C    , o2     , semigunk, A    , A    , NULL )) ;

    ERR (GrB_vxm (vgunk,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_vxm (v    ,  vgunk, NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_vxm (v    ,  v    , NULL   , NULL    , vgunk, NULL , NULL )) ;
    ERR (GrB_vxm (v    ,  v    , NULL   , NULL    , v    , Agunk, NULL )) ;
    ERR (GrB_vxm (v    ,  v    , NULL   , NULL    , v    , A    , dgunk)) ;
    ERR (GrB_vxm (v    ,  v    , op2gunk, NULL    , v    , A    , NULL )) ;
    ERR (GrB_vxm (v    ,  v    , o2     , semigunk, v    , A    , NULL )) ;

    ERR (GrB_mxv (vgunk,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxv (v    ,  vgunk, NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxv (v    ,  v    , NULL   , NULL    , Agunk, NULL , NULL )) ;
    ERR (GrB_mxv (v    ,  v    , NULL   , NULL    , A    , vgunk, NULL )) ;
    ERR (GrB_mxv (v    ,  v    , NULL   , NULL    , A    , v    , dgunk)) ;
    ERR (GrB_mxv (v    ,  v    , op2gunk, NULL    , A    , v    , NULL )) ;
    ERR (GrB_mxv (v    ,  v    , o2     , semigunk, A    , v    , NULL )) ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_mxm (NULL ,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxm (C    ,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxm (C    ,  NULL , NULL   , NULL    , A    , NULL , NULL )) ;
    ERR (GrB_mxm (C    ,  NULL , o2     , NULL    , A    , A    , NULL )) ;

    ERR (GrB_vxm (NULL ,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_vxm (v    ,  v    , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_vxm (v    ,  v    , NULL   , NULL    , v    , NULL , NULL )) ;
    ERR (GrB_vxm (v    ,  v    , o2     , NULL    , v    , A    , NULL )) ;

    ERR (GrB_mxv (NULL ,  NULL , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxv (v    ,  v    , NULL   , NULL    , NULL , NULL , NULL )) ;
    ERR (GrB_mxv (v    ,  v    , NULL   , NULL    , A    , NULL , NULL )) ;
    ERR (GrB_mxv (v    ,  v    , o2     , NULL    , A    , v    , NULL )) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_mxm (C   , NULL, NULL, s2  , B   , A   , NULL)) ;
    ERR (GrB_mxm (C   , A   , NULL, s2  , A   , B   , NULL)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_mxm (C, NULL, NULL, Complex_plus_times, A, B, NULL)) ;
    ERR (GrB_mxm (Z, NULL, NULL, s2, A, B, NULL)) ;
    ERR (GrB_mxm (C, NULL, NULL, s2, Z, B, NULL)) ;
    ERR (GrB_mxm (C, NULL, NULL, s2, B, Z, NULL)) ;
    ERR (GrB_mxm (C, Z   , NULL, s2, A, B, NULL)) ;

    OK (GrB_mxm (C, NULL, o2 , s2, A, B, NULL)) ;

    // The following are now allocated; keep them for the rest the tests:
    // Agunk, Tgunk, op1gunk, op2gunk, monoid_gunk, semigunk, Aempty, vempty,
    // desc, dgunk, A, B, C, E, F, Z, v, u

    //--------------------------------------------------------------------------
    // eWiseMult and eWiseAdd
    //--------------------------------------------------------------------------

    m2 = GxB_MIN_FP64_MONOID ;
    s2 = GxB_PLUS_ISEQ_FP32 ;

    expected = GrB_NULL_POINTER ;

    ERR (GrB_eWiseMult (v0, NULL, NULL, s0 , v0, v0, d0)) ;  // vector semiring
    ERR (GrB_eWiseMult (v0, NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, s2 , v , v0, d0)) ;

    ERR (GrB_eWiseMult (v0, NULL, NULL, m0 , v0, v0, d0)) ;  // vector monoid
    ERR (GrB_eWiseMult (v0, NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, m2 , v , v0, d0)) ;

    ERR (GrB_eWiseMult (v0, NULL, NULL, op0, v0, v0, d0)) ;  // vector op
    ERR (GrB_eWiseMult (v0, NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, o2 , v , v0, d0)) ;

    ERR (GrB_eWiseMult (A0, NULL, NULL, s0 , A0, A0, d0)) ;  // matrix semiring
    ERR (GrB_eWiseMult (A0, NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, s2 , A , A0, d0)) ;

    ERR (GrB_eWiseMult (A0, NULL, NULL, m0 , A0, A0, d0)) ;  // matrix monoid
    ERR (GrB_eWiseMult (A0, NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, m2 , A , A0, d0)) ;

    ERR (GrB_eWiseMult (A0, NULL, NULL, op0, A0, A0, d0)) ;  // matrix op
    ERR (GrB_eWiseMult (A0, NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, o2 , A , A0, d0)) ;

    ERR (GrB_eWiseAdd  (v0, NULL, NULL, s0 , v0, v0, d0)) ;  // vector semiring
    ERR (GrB_eWiseAdd  (v0, NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, s2 , v , v0, d0)) ;

    ERR (GrB_eWiseAdd  (v0, NULL, NULL, m0 , v0, v0, d0)) ;  // vector monoid
    ERR (GrB_eWiseAdd  (v0, NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, m2 , v , v0, d0)) ;

    ERR (GrB_eWiseAdd  (v0, NULL, NULL, op0, v0, v0, d0)) ;  // vector op
    ERR (GrB_eWiseAdd  (v0, NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, o2 , v , v0, d0)) ;

    ERR (GrB_eWiseAdd  (A0, NULL, NULL, s0 , A0, A0, d0)) ;  // matrix semiring
    ERR (GrB_eWiseAdd  (A0, NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, s2 , A , A0, d0)) ;

    ERR (GrB_eWiseAdd  (A0, NULL, NULL, m0 , A0, A0, d0)) ;  // matrix monoid
    ERR (GrB_eWiseAdd  (A0, NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, m2 , A , A0, d0)) ;

    ERR (GrB_eWiseAdd  (A0, NULL, NULL, op0, A0, A0, d0)) ;  // matrix op
    ERR (GrB_eWiseAdd  (A0, NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, o2 , A , A0, d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    m0 = monoid_gunk ;
    s0 = semigunk ;
    v0 = vgunk ;
    A0 = Agunk ;
    op0 = op2gunk ;

    ERR (GrB_eWiseMult (v0, NULL, NULL, s0 , v0, v0, d0)) ;  // vector semiring
    ERR (GrB_eWiseMult (v0, NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, s2 , v , v0, d0)) ;
    ERR (GrB_eWiseMult (v , v0  , NULL, s2 , v , v , d0)) ;
    ERR (GrB_eWiseMult (v , NULL, op0 , s2 , v , v , NULL)) ;

    ERR (GrB_eWiseMult (v0, NULL, NULL, m0 , v0, v0, d0)) ;  // vector monoid
    ERR (GrB_eWiseMult (v0, NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, m2 , v , v0, d0)) ;
    ERR (GrB_eWiseMult (v , v0  , NULL, m2 , v , v , d0)) ;
    ERR (GrB_eWiseMult (v , NULL, op0 , m2 , v , v , NULL)) ;

    ERR (GrB_eWiseMult (v0, NULL, NULL, op0, v0, v0, d0)) ;  // vector op
    ERR (GrB_eWiseMult (v0, NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseMult (v , NULL, NULL, o2 , v , v0, d0)) ;
    ERR (GrB_eWiseMult (v , v0  , NULL, o2 , v , v , d0)) ;
    ERR (GrB_eWiseMult (v , NULL, op0 , o2 , v , v , NULL)) ;

    ERR (GrB_eWiseMult (A0, NULL, NULL, s0 , A0, A0, d0)) ;  // matrix semiring
    ERR (GrB_eWiseMult (A0, NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, s2 , A , A0, d0)) ;
    ERR (GrB_eWiseMult (A , A0  , NULL, s2 , A , A , d0)) ;
    ERR (GrB_eWiseMult (A , NULL, op0 , s2 , A , A , NULL)) ;

    ERR (GrB_eWiseMult (A0, NULL, NULL, m0 , A0, A0, d0)) ;  // matrix monoid
    ERR (GrB_eWiseMult (A0, NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, m2 , A , A0, d0)) ;
    ERR (GrB_eWiseMult (A , A0  , NULL, m2 , A , A , d0)) ;
    ERR (GrB_eWiseMult (A , NULL, op0 , m2 , A , A , NULL)) ;

    ERR (GrB_eWiseMult (A0, NULL, NULL, op0, A0, A0, d0)) ;  // matrix op
    ERR (GrB_eWiseMult (A0, NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseMult (A , NULL, NULL, o2 , A , A0, d0)) ;
    ERR (GrB_eWiseMult (A , A0  , NULL, o2 , A , A , d0)) ;
    ERR (GrB_eWiseMult (A , NULL, op0 , o2 , A , A , NULL)) ;

    ERR (GrB_eWiseAdd  (v0, NULL, NULL, s0 , v0, v0, d0)) ;  // vector semiring
    ERR (GrB_eWiseAdd  (v0, NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, s2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, s2 , v , v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , v0  , NULL, s2 , v , v , d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, op0 , s2 , v , v , NULL)) ;

    ERR (GrB_eWiseAdd  (v0, NULL, NULL, m0 , v0, v0, d0)) ;  // vector monoid
    ERR (GrB_eWiseAdd  (v0, NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, m2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, m2 , v , v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , v0  , NULL, m2 , v , v , d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, op0 , m2 , v , v , NULL)) ;

    ERR (GrB_eWiseAdd  (v0, NULL, NULL, op0, v0, v0, d0)) ;  // vector op
    ERR (GrB_eWiseAdd  (v0, NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, o2 , v0, v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, NULL, o2 , v , v0, d0)) ;
    ERR (GrB_eWiseAdd  (v , v0  , NULL, o2 , v , v , d0)) ;
    ERR (GrB_eWiseAdd  (v , NULL, op0 , o2 , v , v , NULL)) ;

    ERR (GrB_eWiseAdd  (A0, NULL, NULL, s0 , A0, A0, d0)) ;  // matrix semiring
    ERR (GrB_eWiseAdd  (A0, NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, s2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, s2 , A , A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , A0  , NULL, s2 , A , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, op0 , s2 , A , A , NULL)) ;

    ERR (GrB_eWiseAdd  (A0, NULL, NULL, m0 , A0, A0, d0)) ;  // matrix monoid
    ERR (GrB_eWiseAdd  (A0, NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, m2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, m2 , A , A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , A0  , NULL, m2 , A , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, op0 , m2 , A , A , NULL)) ;

    ERR (GrB_eWiseAdd  (A0, NULL, NULL, op0, A0, A0, d0)) ;  // matrix op
    ERR (GrB_eWiseAdd  (A0, NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, o2 , A , A0, d0)) ;
    ERR (GrB_eWiseAdd  (A , A0  , NULL, o2 , A , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, op0 , o2 , A , A , NULL)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_eWiseAdd  (A , NULL, NULL, o2 , Z , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, o2 , A , Z , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, Complex_plus, Z , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, Complex_plus, A , Z , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, NULL, Complex_plus, Z , Z , d0)) ;
    ERR (GrB_eWiseAdd  (Z , Z   , NULL, Complex_plus, Z , Z , d0)) ;
    ERR (GrB_eWiseAdd  (Z , NULL, NULL, Complex_complex, A , A , d0)) ;

    OK (GrB_BinaryOp_new (&op3, f3, Complex, Complex, GrB_FP64)) ;
    ERR (GrB_eWiseAdd  (Z , NULL, NULL, op3, Z , A , d0)) ;
    ERR (GrB_eWiseAdd  (Z , NULL, op3 , o2 , A , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, op3 , o2 , A , A , d0)) ;
    ERR (GrB_eWiseAdd  (A , NULL, Complex_complex, o2 , A , A , d0)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_eWiseAdd  (C , NULL, NULL, o2 , A , B , d0)) ;
    ERR (GrB_eWiseAdd  (C , NULL, NULL, o2 , A , B , dtn)) ;
    ERR (GrB_eWiseAdd  (C , NULL, NULL, o2 , A , B , dnt)) ;
    ERR (GrB_eWiseAdd  (C , NULL, NULL, o2 , A , B , dtt)) ;

    // The following are now allocated; keep them for the rest the tests:
    // Agunk, Tgunk, op1gunk, op2gunk, monoid_gunk, semigunk, Aempty, vempty,
    // desc, dgunk, A, B, C, E, F, Z, v, u, dnt, dtn, dtt

    //--------------------------------------------------------------------------
    // GxB_kron
    //--------------------------------------------------------------------------

    m2 = GxB_MIN_FP64_MONOID ;
    s2 = GxB_PLUS_ISEQ_FP32 ;

    m0 = NULL ;
    s0 = NULL ;
    v0 = NULL ;
    A0 = NULL ;
    op0 = NULL ;

    expected = GrB_NULL_POINTER ;

    info = (GxB_kron (A0, NULL, NULL, op0, A0, A0, d0)) ;  // matrix op
    printf ("dod %d %s\n", info, GrB_error ( )) ;
    ERR (GxB_kron (A0, NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GxB_kron (A , NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GxB_kron (A , NULL, NULL, o2 , A , A0, d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    m0 = monoid_gunk ;
    s0 = semigunk ;
    v0 = vgunk ;
    A0 = Agunk ;
    op0 = op2gunk ;

    ERR (GxB_kron (A0, NULL, NULL, op0, A0, A0, d0)) ;  // matrix op
    ERR (GxB_kron (A0, NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GxB_kron (A , NULL, NULL, o2 , A0, A0, d0)) ;
    ERR (GxB_kron (A , NULL, NULL, o2 , A , A0, d0)) ;
    ERR (GxB_kron (A , A0  , NULL, o2 , A , A , d0)) ;
    ERR (GxB_kron (A , NULL, op0 , o2 , A , A , NULL)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GxB_kron  (A , NULL, NULL, o2 , Z , A , d0)) ;
    ERR (GxB_kron  (A , NULL, NULL, o2 , A , Z , d0)) ;
    ERR (GxB_kron  (A , NULL, NULL, Complex_plus, Z , A , d0)) ;
    ERR (GxB_kron  (A , NULL, NULL, Complex_plus, A , Z , d0)) ;
    ERR (GxB_kron  (A , NULL, NULL, Complex_plus, Z , Z , d0)) ;
    ERR (GxB_kron  (Z , Z   , NULL, Complex_plus, Z , Z , d0)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GxB_kron  (C , NULL, NULL, o2 , A , B , d0)) ;
    ERR (GxB_kron  (C , NULL, NULL, o2 , A , B , dtn)) ;
    ERR (GxB_kron  (C , NULL, NULL, o2 , A , B , dnt)) ;
    ERR (GxB_kron  (C , NULL, NULL, o2 , A , B , dtt)) ;

    //--------------------------------------------------------------------------
    // extract
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    m0 = NULL ;
    s0 = NULL ;
    v0 = NULL ;
    A0 = NULL ;
    op0 = NULL ;

    ERR (GrB_extract (v0, NULL, NULL, v0, I0, 0,    d0)) ;     // vector extract
    ERR (GrB_extract (v , NULL, NULL, v0, I0, 0,    d0)) ;
    ERR (GrB_extract (v , NULL, NULL, u , I0, 0,    d0)) ;

    ERR (GrB_extract (v0, NULL, NULL, A0, I0, 0, 0, d0)) ;     // column extract
    ERR (GrB_extract (v , NULL, NULL, A0, I0, 0, 0, d0)) ;
    ERR (GrB_extract (v , NULL, NULL, A , I0, 0, 0, d0)) ;

    ERR (GrB_extract (A0, NULL, NULL, A0, I0, 0, J0, 0, d0)) ; // matrix extract
    ERR (GrB_extract (A , NULL, NULL, A0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_extract (A , NULL, NULL, A , I0, 0, J0, 0, d0)) ;
    ERR (GrB_extract (A , NULL, NULL, A , I , 0, J0, 0, d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    v0 = vgunk ;
    A0 = Agunk ;
    d0 = dgunk ;
    op0 = op2gunk ;

    ERR (GrB_extract (v0, NULL, NULL, v0, I0, 0,    d0)) ;     // vector extract
    ERR (GrB_extract (v , v0  , NULL, v0, I0, 0,    d0)) ;
    ERR (GrB_extract (v , v   , NULL, v0, I0, 0,    d0)) ;
    ERR (GrB_extract (v , v   , NULL, v , I , 1,    d0)) ;
    ERR (GrB_extract (v , v   , op0 , v , I , 1,    NULL)) ;

    ERR (GrB_extract (v0, NULL, NULL, A0, I0, 0, 0, d0)) ;     // column extract
    ERR (GrB_extract (v , v0  , NULL, A0, I0, 0, 0, d0)) ;
    ERR (GrB_extract (v , v   , NULL, A0, I0, 0, 0, d0)) ;
    ERR (GrB_extract (v , v   , NULL, A , I , 1, 0, d0)) ;
    ERR (GrB_extract (v , v   , op0 , A , I , 1, 0, NULL)) ;

    ERR (GrB_extract (A0, NULL, NULL, A0, I0, 0, J0, 0, d0)) ; // matrix extract
    ERR (GrB_extract (A , A0  , NULL, A0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_extract (A , A   , NULL, A0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_extract (A , A   , NULL, A0, I , 1, J , 1, d0)) ;
    ERR (GrB_extract (A , A   , op0 , A , I , 1, J , 1, NULL)) ;

    v0 = NULL ;
    A0 = NULL ;
    d0 = NULL ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR (GrB_extract (v, z, NULL, u, I, 0, d0)) ;              // vector extract
    ERR (GrB_extract (v, NULL, Complex_plus, u, I, 0, d0)) ;
    ERR (GrB_extract (v, NULL, Complex_plus, z, I, 0, d0)) ;
    ERR (GrB_extract (z, NULL, o2 , u, I, 0, d0)) ;
    ERR (GrB_extract (v, NULL, o2 , z, I, 0, d0)) ;

    ERR (GrB_extract (v, z, NULL, A, I, 0, 0, d0)) ;           // column extract
    ERR (GrB_extract (v, NULL, Complex_plus, A, I, 0, 0, d0)) ;
    ERR (GrB_extract (v, NULL, Complex_plus, Z, I, 0, 0, d0)) ;
    ERR (GrB_extract (z, NULL, o2 , A, I, 0, 0, d0)) ;
    ERR (GrB_extract (v, NULL, o2 , Z, I, 0, 0, d0)) ;

    ERR (GrB_extract (A, Z, NULL, A, I, 0, J, 0, d0)) ;        // matrix extract
    ERR (GrB_extract (A, NULL, Complex_plus, A, I, 0, J, 0, d0)) ;
    ERR (GrB_extract (A, NULL, Complex_plus, Z, I, 0, J, 0, d0)) ;
    ERR (GrB_extract (Z, NULL, o2 , A, I, 0, J, 0, d0)) ;
    ERR (GrB_extract (A, NULL, o2 , Z, I, 0, J, 0, d0)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_extract (A, NULL, NULL, A, I, 1, J, 2, d0)) ;
    ERR (GrB_extract (A, NULL, NULL, A, I, 1, J, 2, dtn)) ;

    expected = GrB_INVALID_INDEX ;

    OK (GrB_Vector_new (&h, GrB_FP64, 1)) ;

    OK  (GrB_extract (h, NULL, NULL, A, I, 1,   0, d0)) ;  // column extract
    ERR (GrB_extract (h, NULL, NULL, A, I, 1, 911, d0)) ;  // column extract

    expected = GrB_INDEX_OUT_OF_BOUNDS ;

    OK (GrB_Matrix_new (&H, GrB_FP64, 1, 1)) ;

    I [0] = 911 ;
    ERR (GrB_extract (H, NULL, NULL, A, I, 1, J, 1, d0)) ;
    I [0] = 0 ;

    J [0] = 911 ;
    ERR (GrB_extract (H, NULL, NULL, A, I, 1, J, 1, d0)) ;
    J [0] = 4 ;

    //--------------------------------------------------------------------------
    // subassign
    //--------------------------------------------------------------------------
    
    expected = GrB_NULL_POINTER ;

    // GxB_Vector_subassign   (w,mask,acc,u,I,ni,d)
    // GxB_Matrix_subassign   (C,Mask,acc,A,I,ni,J,nj,d)
    // GxB_Col_subassign      (C,mask,acc,u,I,ni,j,d)
    // GxB_Row_subassign      (C,mask,acc,u,i,J,nj,d)
    // GxB_Vector_subassign_T (w,mask,acc,x,I,ni,d)
    // GxB_Matrix_subassign_T (C,Mask,acc,x,I,ni,J,nj,d)

    ERR (GxB_subassign (v0, NULL, NULL, v0, I0, 0, d0)) ;       // vector assign
    ERR (GxB_subassign (v , NULL, NULL, v0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL, v , I0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL, A0, I0, 0, J0, 0, d0)) ;// matrix assign
    ERR (GxB_subassign (A , NULL, NULL, A0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL, A , I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL, A , I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL, v0, I0, 0,  0, d0)) ;   // column assign
    ERR (GxB_subassign (A , NULL, NULL, v0, I0, 0,  0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL, v , I0, 0,  0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL, v0,  0, J0, 0, d0)) ;   // row assign
    ERR (GxB_subassign (A , NULL, NULL, v0,  0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL, v ,  0, J0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  x, I0, 0, d0)) ;       // vector scalar
    ERR (GxB_subassign (v , NULL, NULL,  x, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (bool) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (bool) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int8_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (int8_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint8_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (uint8_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int16_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (int16_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint16_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (uint16_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int32_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (int32_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint32_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (uint32_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int64_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (int64_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint64_t) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (uint64_t) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (float) 0, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (float) 0, I0, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (void *) X, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (void *) X, I0, 0, d0)) ;
    ERR (GxB_subassign (v , NULL, NULL,  (void *) NULL, I, 0, d0)) ;


    ERR (GxB_subassign (A0, NULL, NULL,  x, I0, 0, J0, 0, d0)) ;// matrix scalar
    ERR (GxB_subassign (A , NULL, NULL,  x, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  x, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (bool) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (bool) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (bool) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int8_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint8_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int16_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint16_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int32_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint32_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (int64_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (uint64_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (float) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (float) 0, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (float) 0, I , 0, J0, 0, d0)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (void *) X, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (void *) X, I0, 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (void *) X, I , 0, J0, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, NULL,  (void *) NULL, I , 0, J, 0, d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    v0 = vgunk ;
    A0 = Agunk ;
    d0 = dgunk ;
    op0 = op2gunk ;

    ERR (GxB_subassign (v0, NULL, NULL, v0, I, 0, d0)) ;        // vector assign
    ERR (GxB_subassign (v , v0  , NULL, v0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL, v0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL, v , I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 , v , I, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL, A0, I, 0, J, 0, d0)) ;  // matrix assign
    ERR (GxB_subassign (A , A0  , NULL, A0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL, A0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL, A , I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 , A , I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL, v0, I, 0,  0, d0)) ;    // column assign
    ERR (GxB_subassign (A , v0  , NULL, v0, I, 0,  0, d0)) ;
    ERR (GxB_subassign (A , v   , NULL, v0, I, 0,  0, d0)) ;
    ERR (GxB_subassign (A , v   , NULL, v , I, 0,  0, d0)) ;
    ERR (GxB_subassign (A , v   , op0 , v , I, 0,  0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL, v0,  0, J, 0, d0)) ;    // row assign
    ERR (GxB_subassign (A , v0  , NULL, v0,  0, J, 0, d0)) ;
    ERR (GxB_subassign (A , v   , NULL, v0,  0, J, 0, d0)) ;
    ERR (GxB_subassign (A , v   , NULL, v ,  0, J, 0, d0)) ;
    ERR (GxB_subassign (A , NULL, op0 , v ,  0, J, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  x, I, 0, d0)) ;       // vector scalar
    ERR (GxB_subassign (v , v0  , NULL,  x, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  x, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  x, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (bool) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (bool) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (bool) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (bool) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int8_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (int8_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (int8_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (int8_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint8_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (uint8_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (uint8_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (uint8_t) 0, I, 0, d0)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int16_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (int16_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (int16_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (int16_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint16_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (uint16_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (uint16_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (uint16_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int32_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (int32_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (int32_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (int32_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint32_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (uint32_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (uint32_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (uint32_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (int64_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (int64_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (int64_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (int64_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (uint64_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (uint64_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (uint64_t) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (uint64_t) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (float) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (float) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (float) 0, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (float) 0, I, 0, NULL)) ;

    ERR (GxB_subassign (v0, NULL, NULL,  (void *) X, I, 0, d0)) ;
    ERR (GxB_subassign (v , v0  , NULL,  (void *) X, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , NULL,  (void *) X, I, 0, d0)) ;
    ERR (GxB_subassign (v , v   , op0 ,  (void *) X, I, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  x, I, 0, J, 0, d0)) ;  // matrix scalar
    ERR (GxB_subassign (A , A0  , NULL,  x, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  x, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  x, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (bool) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (bool) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (bool) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (bool) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (int8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (int8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (int8_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (uint8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (uint8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (uint8_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (int16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (int16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (int16_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (uint16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (uint16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (uint16_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (int32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (int32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (int32_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (uint32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (uint32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (uint32_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (int64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (int64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (int64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (int64_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (uint64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (uint64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (uint64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (uint64_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (float) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (float) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (float) 0, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (float) 0, I, 0, J, 0, NULL)) ;

    ERR (GxB_subassign (A0, NULL, NULL,  (void *) X, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A0  , NULL,  (void *) X, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , NULL,  (void *) X, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A , A   , op0 ,  (void *) X, I, 0, J, 0, NULL)) ;

    v0 = NULL ;
    A0 = NULL ;
    d0 = NULL ;
    op0 = NULL ;

    expected = GrB_DOMAIN_MISMATCH ;

    op0 = Complex_plus ;

    ERR (GxB_subassign (v, z , NULL, v, I, 0, d0)) ;            // vector assign
    ERR (GxB_subassign (v, v0, op0 , v, I, 0, d0)) ;
    ERR (GxB_subassign (v, v0, op0 , z, I, 0, d0)) ;
    ERR (GxB_subassign (z, v0, o2  , v, I, 0, d0)) ;
    ERR (GxB_subassign (v, v0, o2  , z, I, 0, d0)) ;
    ERR (GxB_subassign (v, v0, NULL, z, I, 0, d0)) ;

    ERR (GxB_subassign (A, Z , NULL, A, I, 0, J, 0, d0)) ;      // matrix assign
    ERR (GxB_subassign (A, A0, op0 , A, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, A0, op0 , Z, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (Z, A0, o2  , A, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, A0, o2  , Z, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, A0, NULL, Z, I, 0, J, 0, d0)) ;

    ERR (GxB_subassign (A, z , NULL, v, I, 0, 0, d0)) ;         // column assign
    ERR (GxB_subassign (A, v0, op0 , v, I, 0, 0, d0)) ;
    ERR (GxB_subassign (A, v0, op0 , z, I, 0, 0, d0)) ;
    ERR (GxB_subassign (Z, v0, o2  , v, I, 0, 0, d0)) ;
    ERR (GxB_subassign (A, v0, o2  , z, I, 0, 0, d0)) ;
    ERR (GxB_subassign (A, v0, NULL, z, I, 0, 0, d0)) ;

    ERR (GxB_subassign (A, z , NULL, v, 0, J, 0, d0)) ;         // row assign
    ERR (GxB_subassign (A, v0, op0 , v, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, v0, op0 , z, 0, J, 0, d0)) ;
    ERR (GxB_subassign (Z, v0, o2  , v, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, v0, o2  , z, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, v0, NULL, z, 0, J, 0, d0)) ;

    ERR (GxB_subassign (v, z , NULL, x, I, 0, d0)) ;            // vector scalar
    ERR (GxB_subassign (v, v0, op0 , x, I, 0, d0)) ;
    ERR (GxB_subassign (v, v0, op0 ,(void *) &c, I, 0, d0)) ;
    ERR (GxB_subassign (z, v0, o2  , x, I, 0, d0)) ;
    ERR (GxB_subassign (v, v0, o2  ,(void *) &c, I, 0, d0)) ;
    ERR (GxB_subassign (v, v0, NULL,(void *) &c, I, 0, d0)) ;

    ERR (GxB_subassign (A, Z , NULL, x, I, 0, J, 0, d0)) ;      // matrix scalar
    ERR (GxB_subassign (A, A0, op0 , x, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, A0, op0 ,(void *) &c , I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (Z, A0, o2  , x, I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, A0, o2  ,(void *) &c , I, 0, J, 0, d0)) ;
    ERR (GxB_subassign (A, A0, NULL,(void *) &c , I, 0, J, 0, d0)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GxB_subassign (A, NULL, NULL, A, I, 2, J, 3, d0)) ;
    ERR (GxB_subassign (A, NULL, NULL, A, I, 2, J, 3, dtn)) ;
    ERR (GxB_subassign (A , v   , NULL, v ,  0, J, 0, NULL)) ;

    // for (int k = 0 ; k < 3 ; k++) printf ("I [%d] = %lld\n", k, I [k]) ;
    // for (int k = 0 ; k < 2 ; k++) printf ("J [%d] = %lld\n", k, J [k]) ;
    // GB_check (A, "Aok", 3) ;
    expected = GrB_INDEX_OUT_OF_BOUNDS ;
    ERR (GxB_subassign (A, NULL, GrB_PLUS_FP64, C, I, 3, J, 2, NULL)) ;

    // GB_check (A, "Aok1", 3) ;
    // GB_check (C, "Cok", 3) ;

    GrB_Index I3 [5] = { 0,   1,   2,   3,    4 } ;
    GrB_Index J3 [5] = { 0,   1,   2,   3,    4 } ;

    OK (GxB_subassign (A, NULL, GrB_PLUS_FP64, C, I3, 3, J3, 2, NULL)) ;
    // GB_check (A, "Aok2", 3) ;

    OK (GxB_subassign (C, C, GrB_PLUS_FP64, C, I3, 3, J3, 2, NULL)) ;

    J3 [0] = 999 ;
    ERR (GxB_subassign (C, C, GrB_PLUS_FP64, C, I3, 3, J3, 2, NULL)) ;
    ERR (GxB_subassign (A, NULL, GrB_PLUS_FP64, x_double, I3, 1, J3, 1, NULL)) ;

    //--------------------------------------------------------------------------
    // assign
    //--------------------------------------------------------------------------
    
    expected = GrB_NULL_POINTER ;

    // GrB_Vector_assign   (w,mask,acc,u,I,ni,d)
    // GrB_Matrix_assign   (C,Mask,acc,A,I,ni,J,nj,d)
    // GrB_Col_assign      (C,mask,acc,u,I,ni,j,d)
    // GrB_Row_assign      (C,mask,acc,u,i,J,nj,d)
    // GrB_Vector_assign_T (w,mask,acc,x,I,ni,d)
    // GrB_Matrix_assign_T (C,Mask,acc,x,I,ni,J,nj,d)

    ERR (GrB_assign (v0, NULL, NULL, v0, I0, 0, d0)) ;          // vector assign
    ERR (GrB_assign (v , NULL, NULL, v0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL, v , I0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL, A0, I0, 0, J0, 0, d0)) ;   // matrix assign
    ERR (GrB_assign (A , NULL, NULL, A0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL, A , I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL, A , I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL, v0, I0, 0,  0, d0)) ;      // column assign
    ERR (GrB_assign (A , NULL, NULL, v0, I0, 0,  0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL, v , I0, 0,  0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL, v0,  0, J0, 0, d0)) ;      // row assign
    ERR (GrB_assign (A , NULL, NULL, v0,  0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL, v ,  0, J0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  x, I0, 0, d0)) ;          // vector scalar
    ERR (GrB_assign (v , NULL, NULL,  x, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (bool) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (bool) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int8_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (int8_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint8_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (uint8_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int16_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (int16_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint16_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (uint16_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int32_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (int32_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint32_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (uint32_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int64_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (int64_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint64_t) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (uint64_t) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (float) 0, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (float) 0, I0, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (void *) X, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (void *) X, I0, 0, d0)) ;
    ERR (GrB_assign (v , NULL, NULL,  (void *) NULL, I, 0, d0)) ;


    ERR (GrB_assign (A0, NULL, NULL,  x, I0, 0, J0, 0, d0)) ;   // matrix scalar
    ERR (GrB_assign (A , NULL, NULL,  x, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  x, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (bool) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (bool) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (bool) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int8_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint8_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint8_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int16_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint16_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint16_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int32_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint32_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint32_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (int64_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint64_t) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (uint64_t) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (float) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (float) 0, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (float) 0, I , 0, J0, 0, d0)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (void *) X, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (void *) X, I0, 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (void *) X, I , 0, J0, 0, d0)) ;
    ERR (GrB_assign (A , NULL, NULL,  (void *) NULL, I , 0, J, 0, d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    v0 = vgunk ;
    A0 = Agunk ;
    d0 = dgunk ;
    op0 = op2gunk ;

    ERR (GrB_assign (v0, NULL, NULL, v0, I, 0, d0)) ;          // vector assign
    ERR (GrB_assign (v , v0  , NULL, v0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL, v0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL, v , I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 , v , I, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL, A0, I, 0, J, 0, d0)) ;   // matrix assign
    ERR (GrB_assign (A , A0  , NULL, A0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL, A0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL, A , I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 , A , I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL, v0, I, 0,  0, d0)) ;      // column assign
    ERR (GrB_assign (A , v0  , NULL, v0, I, 0,  0, d0)) ;
    ERR (GrB_assign (A , v   , NULL, v0, I, 0,  0, d0)) ;
    ERR (GrB_assign (A , v   , NULL, v , I, 0,  0, d0)) ;
    ERR (GrB_assign (A , v   , op0 , v , I, 0,  0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL, v0,  0, J, 0, d0)) ;      // row assign
    ERR (GrB_assign (A , v0  , NULL, v0,  0, J, 0, d0)) ;
    ERR (GrB_assign (A , v   , NULL, v0,  0, J, 0, d0)) ;
    ERR (GrB_assign (A , v   , NULL, v ,  0, J, 0, d0)) ;
    ERR (GrB_assign (A , NULL, op0 , v ,  0, J, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  x, I, 0, d0)) ;          // vector scalar
    ERR (GrB_assign (v , v0  , NULL,  x, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  x, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  x, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (bool) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (bool) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (bool) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (bool) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int8_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (int8_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (int8_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (int8_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint8_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (uint8_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (uint8_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (uint8_t) 0, I, 0, d0)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int16_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (int16_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (int16_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (int16_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint16_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (uint16_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (uint16_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (uint16_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int32_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (int32_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (int32_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (int32_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint32_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (uint32_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (uint32_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (uint32_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (int64_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (int64_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (int64_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (int64_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (uint64_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (uint64_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (uint64_t) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (uint64_t) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (float) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (float) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (float) 0, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (float) 0, I, 0, NULL)) ;

    ERR (GrB_assign (v0, NULL, NULL,  (void *) X, I, 0, d0)) ;
    ERR (GrB_assign (v , v0  , NULL,  (void *) X, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , NULL,  (void *) X, I, 0, d0)) ;
    ERR (GrB_assign (v , v   , op0 ,  (void *) X, I, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  x, I, 0, J, 0, d0)) ;   // matrix scalar
    ERR (GrB_assign (A , A0  , NULL,  x, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  x, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  x, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (bool) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (bool) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (bool) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (bool) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (int8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (int8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (int8_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (uint8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (uint8_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (uint8_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (int16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (int16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (int16_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (uint16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (uint16_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (uint16_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (int32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (int32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (int32_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (uint32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (uint32_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (uint32_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (int64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (int64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (int64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (int64_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (uint64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (uint64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (uint64_t) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (uint64_t) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (float) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (float) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (float) 0, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (float) 0, I, 0, J, 0, NULL)) ;

    ERR (GrB_assign (A0, NULL, NULL,  (void *) X, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A0  , NULL,  (void *) X, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , NULL,  (void *) X, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A , A   , op0 ,  (void *) X, I, 0, J, 0, NULL)) ;

    v0 = NULL ;
    A0 = NULL ;
    d0 = NULL ;
    op0 = NULL ;

    expected = GrB_DOMAIN_MISMATCH ;

    op0 = Complex_plus ;

    ERR (GrB_assign (v, z , NULL, v, I, 0, d0)) ;               // vector assign
    ERR (GrB_assign (v, v0, op0 , v, I, 0, d0)) ;
    ERR (GrB_assign (v, v0, op0 , z, I, 0, d0)) ;
    ERR (GrB_assign (z, v0, o2  , v, I, 0, d0)) ;
    ERR (GrB_assign (v, v0, o2  , z, I, 0, d0)) ;
    ERR (GrB_assign (v, v0, NULL, z, I, 0, d0)) ;

    ERR (GrB_assign (A, Z , NULL, A, I, 0, J, 0, d0)) ;         // matrix assign
    ERR (GrB_assign (A, A0, op0 , A, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, A0, op0 , Z, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (Z, A0, o2  , A, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, A0, o2  , Z, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, A0, NULL, Z, I, 0, J, 0, d0)) ;

    ERR (GrB_assign (A, z , NULL, v, I, 0, 0, d0)) ;            // column assign
    ERR (GrB_assign (A, v0, op0 , v, I, 0, 0, d0)) ;
    ERR (GrB_assign (A, v0, op0 , z, I, 0, 0, d0)) ;
    ERR (GrB_assign (Z, v0, o2  , v, I, 0, 0, d0)) ;
    ERR (GrB_assign (A, v0, o2  , z, I, 0, 0, d0)) ;
    ERR (GrB_assign (A, v0, NULL, z, I, 0, 0, d0)) ;

    ERR (GrB_assign (A, z , NULL, v, 0, J, 0, d0)) ;            // row assign
    ERR (GrB_assign (A, v0, op0 , v, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, v0, op0 , z, 0, J, 0, d0)) ;
    ERR (GrB_assign (Z, v0, o2  , v, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, v0, o2  , z, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, v0, NULL, z, 0, J, 0, d0)) ;

    ERR (GrB_assign (v, z , NULL, x, I, 0, d0)) ;               // vector scalar
    ERR (GrB_assign (v, v0, op0 , x, I, 0, d0)) ;
    ERR (GrB_assign (v, v0, op0 ,(void *) &c, I, 0, d0)) ;
    ERR (GrB_assign (z, v0, o2  , x, I, 0, d0)) ;
    ERR (GrB_assign (v, v0, o2  ,(void *) &c, I, 0, d0)) ;
    ERR (GrB_assign (v, v0, NULL,(void *) &c, I, 0, d0)) ;

    ERR (GrB_assign (A, Z , NULL, x, I, 0, J, 0, d0)) ;         // matrix scalar
    ERR (GrB_assign (A, A0, op0 , x, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, A0, op0 ,(void *) &c , I, 0, J, 0, d0)) ;
    ERR (GrB_assign (Z, A0, o2  , x, I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, A0, o2  ,(void *) &c , I, 0, J, 0, d0)) ;
    ERR (GrB_assign (A, A0, NULL,(void *) &c , I, 0, J, 0, d0)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_assign (A, NULL, NULL, A, I, 2, J, 3, d0)) ;
    ERR (GrB_assign (A, NULL, NULL, A, I, 2, J, 3, dtn)) ;
    ERR (GrB_assign (A , v   , NULL, v ,  0, J, 0, NULL)) ;

    //--------------------------------------------------------------------------
    // apply
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_apply (v0, NULL, NULL, NULL, v0, d0)) ;     // vector apply
    ERR (GrB_apply (v , NULL, NULL, NULL, v0, d0)) ;
    ERR (GrB_apply (v , NULL, NULL, NULL, v , d0)) ;

    ERR (GrB_apply (A0, NULL, NULL, NULL, A0, d0)) ;     // matrix apply
    ERR (GrB_apply (A , NULL, NULL, NULL, A0, d0)) ;
    ERR (GrB_apply (A , NULL, NULL, NULL, A , d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    v0 = vgunk ;
    A0 = Agunk ;
    d0 = dgunk ;
    op0 = op2gunk ;

    ERR (GrB_apply (v0, NULL, NULL, op1gunk, v0, d0)) ;     // vector apply
    ERR (GrB_apply (v , v0  , NULL, op1gunk, v0, d0)) ;
    ERR (GrB_apply (v , v   , NULL, op1gunk, v0, d0)) ;
    ERR (GrB_apply (v , v   , NULL, op1gunk, v , d0)) ;
    ERR (GrB_apply (v , v   , op0 , op1gunk, v , NULL)) ;
    ERR (GrB_apply (v , v   , NULL, op1gunk, v , NULL)) ;

    ERR (GrB_apply (A0, NULL, NULL, op1gunk, A0, d0)) ;     // matrix apply
    ERR (GrB_apply (A , A0  , NULL, op1gunk, A0, d0)) ;
    ERR (GrB_apply (A , A   , NULL, op1gunk, A0, d0)) ;
    ERR (GrB_apply (A , A   , NULL, op1gunk, A , d0)) ;
    ERR (GrB_apply (A , A   , op0 , op1gunk, A , NULL)) ;
    ERR (GrB_apply (A , A   , NULL, op1gunk, A , NULL)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    o2  = Complex_plus ;

    ERR (GrB_apply (A, Z   , NULL, GrB_AINV_FP64, A, NULL)) ;
    ERR (GrB_apply (A, NULL, o2  , GrB_AINV_FP64, A, NULL)) ;
    ERR (GrB_apply (A, NULL, o2  , GrB_AINV_FP64, Z, NULL)) ;
    ERR (GrB_apply (A, NULL, NULL, GrB_AINV_FP64, Z, NULL)) ;
    ERR (GrB_apply (Z, NULL, NULL, GrB_AINV_FP64, A, NULL)) ;
    ERR (GrB_apply (Z, NULL, NULL, GrB_AINV_FP64, Z, NULL)) ;

    v0 = NULL ;
    A0 = NULL ;
    d0 = NULL ;
    op0 = NULL ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_apply (A , NULL, NULL, GrB_AINV_FP64, C , d0)) ;

    //--------------------------------------------------------------------------
    // select
    //--------------------------------------------------------------------------

    CHECK (selectop == NULL) ;
    OK (GxB_SelectOp_new (&selectop, fselect, GrB_FP64)) ;
    CHECK (selectop != NULL) ;

    expected = GrB_NULL_POINTER ;

    ERR (GxB_select (v0, NULL, NULL, NULL, v0, NULL, d0)) ;     // vector select
    ERR (GxB_select (v , NULL, NULL, NULL, v0, NULL, d0)) ;
    ERR (GxB_select (v , NULL, NULL, NULL, v , NULL, d0)) ;

    ERR (GxB_select (A0, NULL, NULL, NULL, A0, NULL, d0)) ;     // matrix select
    ERR (GxB_select (A , NULL, NULL, NULL, A0, NULL, d0)) ;
    ERR (GxB_select (A , NULL, NULL, NULL, A , NULL, d0)) ;

    CHECK (selectopgunk == NULL) ;
    OK (GxB_SelectOp_new (&selectopgunk, fselect, GrB_FP64)) ;
    CHECK (selectopgunk != NULL) ;
    selectopgunk->magic = 22309483 ;
    expected = GrB_UNINITIALIZED_OBJECT ;
    ERR (GB_SelectOp_check (selectopgunk, "select gunk", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    v0 = vgunk ;
    A0 = Agunk ;
    d0 = dgunk ;
    op0 = op2gunk ;
    sel0 = selectopgunk ;

    ERR (GxB_select (v0, NULL, NULL, sel0, v0, NULL, d0)) ;     // vector select
    ERR (GxB_select (v , v0  , NULL, sel0, v0, NULL, d0)) ;
    ERR (GxB_select (v , v   , NULL, sel0, v0, NULL, d0)) ;
    ERR (GxB_select (v , v   , NULL, sel0, v , NULL, d0)) ;
    ERR (GxB_select (v , v   , op0 , sel0, v , NULL, NULL)) ;
    ERR (GxB_select (v , v   , NULL, sel0, v , NULL, NULL)) ;

    ERR (GxB_select (A0, NULL, NULL, sel0, A0, NULL, d0)) ;     // matrix select
    ERR (GxB_select (A , A0  , NULL, sel0, A0, NULL, d0)) ;
    ERR (GxB_select (A , A   , NULL, sel0, A0, NULL, d0)) ;
    ERR (GxB_select (A , A   , NULL, sel0, A , NULL, d0)) ;
    ERR (GxB_select (A , A   , op0 , sel0, A , NULL, NULL)) ;
    ERR (GxB_select (A , A   , NULL, sel0, A , NULL, NULL)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    o2  = Complex_plus ;

    double thresh = 42 ;

    ERR (GxB_select (A, Z   , NULL, selectop, A, &thresh, NULL)) ;
    ERR (GxB_select (A, NULL, o2  , selectop, A, &thresh, NULL)) ;
    ERR (GxB_select (A, NULL, o2  , selectop, Z, &thresh, NULL)) ;
    ERR (GxB_select (A, NULL, NULL, selectop, Z, &thresh, NULL)) ;
    ERR (GxB_select (Z, NULL, NULL, selectop, A, &thresh, NULL)) ;
    ERR (GxB_select (Z, NULL, NULL, selectop, Z, &thresh, NULL)) ;

    v0 = NULL ;
    A0 = NULL ;
    d0 = NULL ;
    op0 = NULL ;
    sel0 = NULL ;

    OK (GxB_SelectOp_free (&selectop)) ;
    CHECK (selectop == NULL) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GxB_select (A , NULL, NULL, GxB_TRIL, C , &thresh, d0)) ;

    //--------------------------------------------------------------------------
    // reduce to scalar
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    o2 = GrB_PLUS_FP32 ;
    m2 = GxB_TIMES_FP64_MONOID ;

    // matrix to scalar

    ERR (GrB_reduce ((bool     *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((float    *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((double   *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((void     *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m2, A , d0)) ;

    // vector to scalar

    ERR (GrB_reduce ((bool     *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((float    *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((double   *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((void     *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m2, v , d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    m0 = monoid_gunk ;
    v0 = vgunk ;
    A0 = Agunk ;
    op0 = op2gunk ;

    // matrix to scalar

    ERR (GrB_reduce ((bool     *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((float    *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((double   *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m2, A , d0)) ;

    ERR (GrB_reduce ((void     *) NULL, op0, m0, A0, d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m0, A , d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m2, A , d0)) ;

    // vector to scalar

    ERR (GrB_reduce ((bool     *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((bool     *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int8_t   *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint8_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int16_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint16_t *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int32_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint32_t *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((int64_t  *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((uint64_t *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((float    *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((float    *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((double   *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((double   *) NULL, op0, m2, v , d0)) ;

    ERR (GrB_reduce ((void     *) NULL, op0, m0, v0, d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m0, v , d0)) ;
    ERR (GrB_reduce ((void     *) NULL, op0, m2, v , d0)) ;

    m0 = NULL ;
    v0 = NULL ;
    A0 = NULL ;
    op0 = NULL ;

    expected = GrB_DOMAIN_MISMATCH ;

    o2  = Complex_plus ;

    ERR (GrB_reduce (&x, op0 , GxB_PLUS_FP64_MONOID , Z , d0)) ;
    ERR (GrB_reduce (&x, op0 , Complex_plus_monoid  , Z , d0)) ;
    ERR (GrB_reduce (&x, op0 , Complex_plus_monoid  , A , d0)) ;
    ERR (GrB_reduce (&x, o2  , Complex_plus_monoid  , Z , d0)) ;
    ERR (GrB_reduce (&c, o2  , Complex_plus_monoid  , A , d0)) ;

    //--------------------------------------------------------------------------
    // reduce to vector
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    o2 = GrB_PLUS_FP64 ;
    m2 = GxB_TIMES_FP64_MONOID ;

    ERR (GrB_reduce (v0, NULL, NULL, op0, A0, d0)) ;    // reduce via op
    ERR (GrB_reduce (v0, NULL, NULL, o2 , A0, d0)) ;
    ERR (GrB_reduce (v , NULL, NULL, o2 , A0, d0)) ;

    ERR (GrB_reduce (v0, NULL, NULL, m0 , A0, d0)) ;    // reduce via monoid
    ERR (GrB_reduce (v0, NULL, NULL, m2 , A0, d0)) ;
    ERR (GrB_reduce (v , NULL, NULL, m2 , A0, d0)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    m0 = monoid_gunk ;
    v0 = vgunk ;
    A0 = Agunk ;
    op0 = op2gunk ;
    d0 = dgunk ;

    ERR (GrB_reduce (v0, v0  , op0 , op0, A0, d0)) ;    // reduce via op
    ERR (GrB_reduce (v0, v0  , op0 , o2 , A0, d0)) ;
    ERR (GrB_reduce (v , v0  , op0 , o2 , A0, d0)) ;
    ERR (GrB_reduce (v , v   , op0 , o2 , A0, d0)) ;
    ERR (GrB_reduce (v , v   , o2  , o2 , A0, d0)) ;
    ERR (GrB_reduce (v , v   , o2  , o2 , A , d0)) ;

    ERR (GrB_reduce (v0, v0  , op0 , m0 , A0, d0)) ;    // reduce via op
    ERR (GrB_reduce (v0, v0  , op0 , m2 , A0, d0)) ;
    ERR (GrB_reduce (v , v0  , op0 , m2 , A0, d0)) ;
    ERR (GrB_reduce (v , v   , op0 , m2 , A0, d0)) ;
    ERR (GrB_reduce (v , v   , o2  , m2 , A0, d0)) ;
    ERR (GrB_reduce (v , v   , o2  , m2 , A , d0)) ;

    m0 = NULL ;
    v0 = NULL ;
    A0 = NULL ;
    op0 = NULL ;
    d0 = NULL ;

    expected = GrB_DOMAIN_MISMATCH ;

    o2 = Complex_plus ;

    ERR (GrB_reduce (v, z   , NULL, GrB_PLUS_FP64, A, d0)) ;
    ERR (GrB_reduce (z, NULL, NULL, GrB_PLUS_FP64, A, d0)) ;
    ERR (GrB_reduce (v, NULL, o2  , GrB_PLUS_FP64, A, d0)) ;
    ERR (GrB_reduce (v, NULL, NULL, GrB_EQ_FP64  , A, d0)) ;
    ERR (GrB_reduce (v, NULL, NULL, GrB_PLUS_FP64, Z, d0)) ;

    ERR (GrB_reduce (v, z   , NULL, GxB_PLUS_FP64_MONOID, A, d0)) ;
    ERR (GrB_reduce (z, NULL, NULL, GxB_PLUS_FP64_MONOID, A, d0)) ;
    ERR (GrB_reduce (v, NULL, o2  , GxB_PLUS_FP64_MONOID, A, d0)) ;
    ERR (GrB_reduce (v, NULL, NULL, GxB_PLUS_FP64_MONOID, Z, d0)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_reduce (v, NULL, NULL, GrB_PLUS_FP64, A, dtn)) ;
    ERR (GrB_reduce (v, NULL, NULL, GrB_PLUS_FP64, A, d0)) ;

    ERR (GrB_reduce (v, NULL, NULL, GxB_PLUS_FP64_MONOID, A, dtn)) ;
    ERR (GrB_reduce (v, NULL, NULL, GxB_PLUS_FP64_MONOID, A, d0)) ;

    //--------------------------------------------------------------------------
    // transpose
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;

    ERR (GrB_transpose (NULL, NULL, NULL, NULL, NULL)) ;
    ERR (GrB_transpose (A   , NULL, NULL, NULL, NULL)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GrB_transpose (Agunk, NULL , NULL   , NULL , NULL )) ;
    ERR (GrB_transpose (A    , Agunk, NULL   , NULL , NULL )) ;
    ERR (GrB_transpose (A    , A    , op2gunk, NULL , NULL )) ;
    ERR (GrB_transpose (A    , NULL , NULL   , Agunk, NULL )) ;
    ERR (GrB_transpose (A    , NULL , NULL   , A    , dgunk)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    o2 = Complex_plus ;

    ERR (GrB_transpose (A   , Z   , NULL, A, NULL)) ;
    ERR (GrB_transpose (A   , NULL, NULL, Z, NULL)) ;
    ERR (GrB_transpose (A   , NULL, NULL, Z, NULL)) ;
    ERR (GrB_transpose (Z   , NULL, NULL, A, NULL)) ;
    ERR (GrB_transpose (A   , NULL, o2  , A, NULL)) ;
    ERR (GrB_transpose (A   , NULL, o2  , Z, NULL)) ;
    ERR (GrB_transpose (Z   , NULL, o2  , A, NULL)) ;

    expected = GrB_DIMENSION_MISMATCH ;

    ERR (GrB_transpose (A   , NULL, NULL, A, NULL)) ;
    ERR (GrB_transpose (C   , NULL, NULL, A, dtn )) ;

    //==========================================================================
    //=== internal functions ===================================================
    //==========================================================================

    //--------------------------------------------------------------------------
    // Entry print
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Entry_print:\n") ;

    WHERE ("GB_Entry_print (type, x)") ;

    expected = GrB_NULL_POINTER ;

    ERR (GB_Entry_print (NULL, NULL)) ;
    ERR (GB_Entry_print (NULL, X)) ;
    OK (GB_Entry_print (GrB_FP64, X)) ;
    printf ("\n") ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GB_Entry_print (Tgunk, X)) ;
    printf ("\nAll GB_Entry_print tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // Type check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Type_check:\n") ;

    WHERE ("GB_Type_check") ;

    // GrB_error is not updated since checking a null object may not be an
    // error; it may indicate an optional input
    info = GB_Type_check (NULL, "null type", 1) ;
    CHECK (info == GrB_NULL_POINTER) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    ERR (GB_Type_check (Tgunk, "Tgunk", 1)) ;

    CHECK (T == NULL) ;
    OK (GrB_Type_new (&T, int)) ;

    WHERE ("GB_Type_check") ;
    OK (GB_Type_check (T, "T ok", 3)) ;

    T->magic = FREED ;
    ERR (GB_Type_check (T, "T freed", 1)) ;
    T->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    T->code = 99 ;
    ERR (GB_Type_check (T, "T bad code", 1)) ;
    T->code = GB_UDT_code ;
    T->magic = MAGIC ;
    T->size = 0 ;
    ERR (GB_Type_check (T, "T bad size", 1)) ;
    T->size = sizeof (int) ;

    char *e = GB_code_string (9999) ;
    printf ("unknown code: [%s]\n", e) ;
    CHECK (strcmp (e, "unknown!") == 0) ;

    OK (GB_Type_check (T, "type ok", 1)) ;
    printf ("\nAll GB_Type_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // UnaryOp check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_UnaryOp_check:\n") ;

    WHERE ("GB_UnaryOp_check") ;

    info = GB_UnaryOp_check (NULL, "null unary op", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (op1b == NULL) ;
    OK (GrB_UnaryOp_new (&op1b, f1, GrB_FP64, GrB_UINT32)) ;
    CHECK (op1b != NULL) ;

    WHERE ("GB_UnaryOp_check") ;
    OK (GB_UnaryOp_check (op1b, "op1b ok", 1)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    op1b->magic = FREED ;
    ERR (GB_UnaryOp_check (op1b, "op1b freed", 1)) ;
    op1b->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    op1b->function = NULL ;
    ERR (GB_UnaryOp_check (op1b, "op1b null func", 1)) ;
    op1b->function = f1 ;

    op1b->opcode = 1024 ;
    ERR (GB_UnaryOp_check (op1b, "op1b invalid opcode", 1)) ;
    op1b->opcode = GB_USER_opcode ;

    op1b->ztype = NULL ;
    ERR (GB_UnaryOp_check (op1b, "op1b invalid ztype", 1)) ;
    op1b->ztype = GrB_FP64 ;

    op1b->xtype = NULL ;
    ERR (GB_UnaryOp_check (op1b, "op1b invalid xtype", 1)) ;
    op1b->xtype = GrB_UINT32 ;

    printf ("\nAll GB_UnaryOp_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // BinaryOp check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_BinaryOp_check:\n") ;

    WHERE ("GB_BinaryOp_check") ;

    info = GB_BinaryOp_check (NULL, "null unary op", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (op2b == NULL) ;
    OK (GrB_BinaryOp_new (&op2b, f2, GrB_INT32, GrB_UINT8, GrB_INT16)) ;
    CHECK (op2b != NULL) ;

    WHERE ("GB_BinaryOp_check") ;
    OK (GB_BinaryOp_check (op2b, "op2b ok", 1)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    op2b->magic = FREED ;
    ERR (GB_BinaryOp_check (op2b, "op2b freed", 1)) ;
    op2b->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    op2b->function = NULL ;
    ERR (GB_BinaryOp_check (op2b, "op2b null func", 1)) ;
    op2b->function = f2 ;

    op2b->opcode = 1024 ;
    ERR (GB_BinaryOp_check (op2b, "op2b invalid opcode", 1)) ;
    op2b->opcode = GB_USER_opcode ;

    op2b->ztype = NULL ;
    ERR (GB_BinaryOp_check (op2b, "op2b invalid ztype", 1)) ;
    op2b->ztype = GrB_INT32 ;

    op2b->xtype = NULL ;
    ERR (GB_BinaryOp_check (op2b, "op2b invalid xtype", 1)) ;
    op2b->xtype = GrB_UINT8 ;

    op2b->ytype = NULL ;
    ERR (GB_BinaryOp_check (op2b, "op2b invalid ytype", 1)) ;
    op2b->ytype = GrB_UINT16 ;

    printf ("\nAll GB_BinaryOp_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // SelectOp check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_SelectOp_check:\n") ;

    WHERE ("GB_SelectOp_check") ;

    info = GB_SelectOp_check (NULL, "null selectop", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (selectop == NULL) ;
    OK (GxB_SelectOp_new (&selectop, fselect, GrB_FP64)) ;
    CHECK (selectop != NULL) ;

    WHERE ("GB_SelectOp_check") ;
    OK (GB_SelectOp_check (selectop, "user selectop ok", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    selectop->magic = FREED ;
    ERR (GB_SelectOp_check (selectop, "selectop freed", 1)) ;
    selectop->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    selectop->function = NULL ;
    ERR (GB_SelectOp_check (selectop, "selectop invalid function", 1)) ;
    selectop->function = fselect ;

    selectop->opcode = 9999 ;
    ERR (GB_SelectOp_check (selectop, "selectop invalid opcode", 1)) ;
    selectop->opcode = GB_USER_SELECT_opcode ;

    selectop->xtype = Tgunk ;
    ERR (GB_SelectOp_check (selectop, "selectop invalid xtype", 1)) ;
    selectop->xtype = GrB_FP64 ;

    OK (GB_SelectOp_check (selectop, "user selectop ok", 3)) ;

    printf ("\nAll GB_SelectOp_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // Monoid check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Monoid_check:\n") ;

    WHERE ("GB_Monoid_check") ;

    info = GB_Monoid_check (NULL, "null monoid", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (monoidb == NULL) ;
    OK (GrB_Monoid_new (&monoidb, GrB_TIMES_INT32, (int) 1)) ;
    CHECK (monoidb != NULL) ;

    WHERE ("GB_Monoid_check") ;
    OK (GB_Monoid_check (monoidb, "monoidb ok", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    monoidb->magic = FREED ;
    ERR (GB_Monoid_check (monoidb, "monoidb freed", 1)) ;
    monoidb->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    monoidb->op = NULL ;
    ERR (GB_Monoid_check (monoidb, "monoidb invalid op", 1)) ;
    monoidb->op = GrB_TIMES_INT32 ;

    monoidb->op = GrB_EQ_INT32 ;
    ERR (GB_Monoid_check (monoidb, "monoidb invalid op domains", 1)) ;
    monoidb->op = GrB_TIMES_INT32 ;

    OK (GB_Monoid_check (Complex_plus_monoid, "complex plus monoid", 3)) ;
    OK (GB_Monoid_check (Complex_times_monoid, "complex times monoid", 3)) ;

    printf ("\nAll GB_Monoid_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // Semiring check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Semiring_check:\n") ;

    WHERE ("GB_Semiring_check") ;

    info = GB_Semiring_check (NULL, "null semiring", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (semiringb == NULL) ;
    OK (GrB_Semiring_new (&semiringb, GxB_MAX_FP32_MONOID, GrB_TIMES_FP32)) ;
    CHECK (semiringb != NULL) ;

    WHERE ("GB_Semiring_check") ;
    OK (GB_Semiring_check (semiringb, "semiringb ok", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    semiringb->magic = FREED ;
    ERR (GB_Semiring_check (semiringb, "semiringb freed", 1)) ;
    semiringb->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    semiringb->add = NULL ;
    ERR (GB_Semiring_check (semiringb, "semiringb invalid add monoid", 1)) ;
    semiringb->add = GxB_MAX_FP32_MONOID ;

    semiringb->multiply = NULL ;
    ERR (GB_Semiring_check (semiringb, "semiringb invalid mult", 1)) ;
    semiringb->multiply = GrB_TIMES_FP32 ;

    semiringb->multiply = GrB_TIMES_INT32 ;
    ERR (GB_Semiring_check (semiringb, "semiringb invalid mix", 1)) ;
    semiringb->multiply = GrB_TIMES_FP32 ;

    printf ("\nAll GB_Semiring_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // Descriptor check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Descriptor_check:\n") ;

    WHERE ("GB_Descriptor_check") ;

    info = GB_Descriptor_check (NULL, "null descriptor", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (descb == NULL) ;
    OK (GrB_Descriptor_new (&descb)) ;
    CHECK (descb != NULL) ;

    WHERE ("GB_Descriptor_check") ;
    OK (GB_Descriptor_check (descb, "descb ok", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    descb->magic = FREED ;
    ERR (GB_Descriptor_check (descb, "descb freed", 1)) ;
    descb->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    descb->out = 42 ;
    ERR (GB_Descriptor_check (descb, "descb invalid", 1)) ;
    descb->out = GxB_DEFAULT ;

    printf ("\nAll GB_Descriptor_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // Vector check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Vector_check:\n") ;

    OK (GrB_free (&v)) ;
    CHECK (v == NULL) ;

    WHERE ("GB_Vector_check") ;

    info = GB_Vector_check (NULL, "null vector", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (v == NULL) ;
    OK (GrB_Vector_new (&v, GrB_FP64, 10)) ;
    CHECK (v != NULL) ;

    WHERE ("GB_Vector_check") ;
    OK (GB_Vector_check (v, "v ok", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    v->magic = FREED ;
    ERR (GB_Vector_check (v, "v freed", 1)) ;
    v->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    v->ncols = 2 ;
    int64_t *psave = v->p ;
    v->p = mxCalloc (3, sizeof (int64_t)) ;
    ERR (GB_Vector_check (v, "v invalid", 1)) ;
    v->ncols = 1 ;

    v->p [0] = 1 ;
    ERR (GB_Vector_check (v, "v p[0] invalid", 1)) ;

    mxFree (v->p) ;
    v->p = psave ;
    psave = NULL ;

    printf ("\nAll GB_Vector_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // Matrix check
    //--------------------------------------------------------------------------

    printf ("\n-------------- GB_Matrix_check:\n") ;

    OK (GrB_free (&A)) ;
    CHECK (A == NULL) ;

    GrB_wait ( ) ;
    CHECK (GB_Global.queue_head == NULL) ;

    WHERE ("GB_Matrix_check") ;

    info = GB_Matrix_check (NULL, "null matrix", 3) ;
    CHECK (info == GrB_NULL_POINTER) ;

    CHECK (A == NULL) ;
    OK (GrB_Matrix_new (&A, GrB_FP64, 10, 4)) ;
    CHECK (A != NULL) ;

    WHERE ("GB_Matrix_check") ;
    OK (GB_Matrix_check (A, "A ok", 3)) ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    A->magic = FREED ;
    ERR (GB_Matrix_check (A, "A freed", 1)) ;
    A->magic = MAGIC ;

    expected = GrB_INVALID_OBJECT ;

    A->p [0] = 1 ;
    ERR (GB_Matrix_check (A, "p[0] invalid", 1)) ;
    A->p [0] = 0 ;

    A->nrows = -1 ;
    ERR (GB_Matrix_check (A, "invalid dimensions", 1)) ;
    A->nrows = 10 ;

    A->type = NULL ;
    ERR (GB_Matrix_check (A, "invalid type", 1)) ;
    A->type = GrB_FP64 ;

    psave = A->p ;
    A->p = NULL ;
    ERR (GB_Matrix_check (A, "NULL Ap", 1)) ;
    A->p = psave ;

    CHECK (A->i == NULL) ;
    A->i = mxMalloc (1) ;
    ERR (GB_Matrix_check (A, "invalid empty", 1)) ;
    mxFree (A->i) ;
    A->i = NULL ;

    OK (GrB_Matrix_setElement (A, 3.14159, 0, 0)) ;
    OK (GB_Matrix_check (A, "valid pending pi", 3)) ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 1) ;
    OK (GB_Matrix_check (A, "valid pi", 3)) ;

    WHERE ("GB_Matrix_check") ;

    psave = A->i ;
    A->i = NULL ;
    ERR (GB_Matrix_check (A, "NULL Ai", 1)) ;
    A->i = psave ;
    OK (GB_Matrix_check (A, "valid pi", 0)) ;

    A->p [0] = 1 ;
    ERR (GB_Matrix_check (A, "Ap[0] invalid", 1)) ;
    A->p [0] = 0 ;

    int64_t isave = A->p [1] ;
    A->p [1] = -1 ;
    ERR (GB_Matrix_check (A, "Ap[1] invalid", 1)) ;
    A->p [1] = isave ;

    isave = A->p [4] ;
    A->p [4]++ ;
    ERR (GB_Matrix_check (A, "Ap[ncols] invalid", 1)) ;
    A->p [4] = isave ;

    isave = A->nzombies ;
    A->nzombies = -1 ;
    ERR (GB_Matrix_check (A, "negative zombies", 1)) ;
    A->nzombies = isave ;

    isave = A->nzombies ;
    A->nzombies = 1000 ;
    ERR (GB_Matrix_check (A, "too many zombies", 1)) ;
    A->nzombies = isave ;

    isave = A->i [0] ;
    A->i [0] = -1 ;
    ERR (GB_Matrix_check (A, "row index invalid", 3)) ;
    A->i [0] = isave ;

    isave = A->nzombies ;
    A->nzombies = 1 ;
    ERR (GB_Matrix_check (A, "bad zombies", 3)) ;
    A->nzombies = isave ;

    isave = A->npending ;
    A->npending = -1 ;
    ERR (GB_Matrix_check (A, "negative pending", 1)) ;
    A->npending = isave ;

    CHECK (A->ipending == NULL) ;
    A->ipending = mxMalloc (1) ;
    ERR (GB_Matrix_check (A, "bad pending", 1)) ;
    mxFree (A->ipending) ;
    A->ipending = NULL ;

    OK (GrB_Matrix_setElement (A, 7.1, 1, 0)) ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1]", 3)) ;

    WHERE ("GB_Matrix_check") ;

    psave = A->ipending ;
    A->ipending = NULL ;
    ERR (GB_Matrix_check (A, "missing pending", 3)) ;
    A->ipending = psave ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1]", 0)) ;

    CHECK (A->jpending != NULL) ;
    isave = A->jpending [0] ;
    A->jpending [0] = 1070 ;
    ERR (GB_Matrix_check (A, "bad pending tuple", 3)) ;
    A->jpending [0] = isave ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1]", 0)) ;

    OK (GrB_Matrix_setElement (A, 11.4, 0, 1)) ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1 11.4]", 3)) ;

    WHERE ("GB_Matrix_check") ;
    isave = A->jpending [0] ;
    A->jpending [0] = 2 ;
    ERR (GB_Matrix_check (A, "jumbled pending tuples", 3)) ;
    A->jpending [0] = isave ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1 11.4]", 0)) ;

    CHECK (A->operator_pending == NULL) ;
    A->operator_pending = op2gunk ;
    ERR (GB_Matrix_check (A, "invalid operator", 3)) ;
    A->operator_pending = NULL ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1 11.4]", 0)) ;

    CHECK (GB_Global.queue_head == A) ;
    GB_Global.queue_head = NULL ;
    ERR (GB_Matrix_check (A, "inconsistent queue", 3)) ;
    A->enqueued = false ;
    ERR (GB_Matrix_check (A, "missing from queue", 3)) ;
    GB_Global.queue_head = A ;
    A->enqueued = true ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1 11.4]", 0)) ;

    CHECK (A->queue_prev == NULL) ;
    A->queue_prev = A ;
    ERR (GB_Matrix_check (A, "invalid queue", 3)) ;
    A->queue_prev = NULL ;
    OK (GB_Matrix_check (A, "valid pending [pi 7.1 11.4]", 0)) ;

    OK (GrB_Matrix_nvals (&nvals, A)) ;

    WHERE ("GB_Matrix_check") ;
    OK (GB_Matrix_check (A, "valid [pi 7 11.4]", 3)) ;
    CHECK (nvals == 3) ;

    expected = GrB_INDEX_OUT_OF_BOUNDS ;

    A->i [0] = 1 ;
    A->i [1] = 0 ;
    info = GB_Matrix_check (A, "jumbled", 3) ;
    printf ("jumbled info %d\n", info) ;
    CHECK (info == GrB_INDEX_OUT_OF_BOUNDS) ;
    A->i [0] = 0 ;
    A->i [1] = 1 ;
    OK (GB_Matrix_check (A, "OK", 3)) ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 3) ;

    OK (A->npending == 0 && A->nzombies == 0) ;
    OK (GrB_Matrix_new (&Empty1, GrB_FP64, 1, 1)) ;
    I [0] = 0 ;
    J [0] = 0 ;
    OK (GxB_subassign (A, NULL, NULL, Empty1, I, 1, J, 1, NULL)) ;
    OK (GB_Matrix_check (A, "valid zombie", 3)) ;
    OK (A->npending == 0 && A->nzombies == 1) ;
    OK (GrB_Matrix_setElement (A, 99099, 0, 0)) ;
    OK (A->npending == 0 && A->nzombies == 0) ;
    OK (GB_Matrix_check (A, "no more zombie", 3)) ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 3) ;

    OK (GxB_subassign (A, NULL, NULL, Empty1, I, 1, J, 1, NULL)) ;
    OK (GB_Matrix_check (A, "valid zombie", 3)) ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;
    CHECK (nvals == 2) ;
    OK (GB_Matrix_check (A, "again no more zombie", 3)) ;
    OK (A->npending == 0 && A->nzombies == 0) ;

    expected = GrB_INVALID_OBJECT ;

    CHECK (GB_Global.queue_head == NULL) ;
    GB_Global.queue_head = A ;
    A->enqueued = true ;
    ERR (GB_Matrix_check (A, "should not be in queue", 3)) ;
    GB_Global.queue_head = NULL ;
    A->enqueued = false ;
    OK (GB_Matrix_check (A, "valid, no pending", 3)) ;

    printf ("\nAll GB_Matrix_check tests passed (errors expected)\n") ;

    //--------------------------------------------------------------------------
    // blocking vs non-blocking mode
    //--------------------------------------------------------------------------

    GrB_wait ( ) ;
    CHECK (GB_Global.queue_head == NULL) ;
    OK (GrB_Matrix_setElement (A, 32.4, 3, 2)) ;
    OK (GB_Matrix_check (A, "A with one pending", 3)) ;
    CHECK (A->npending == 1 && A->nzombies == 0) ;
    GB_Global.mode = GrB_BLOCKING ;
    OK (GB_block (A)) ;
    OK (GB_Matrix_check (A, "A with no pending", 3)) ;
    CHECK (A->npending == 0 && A->nzombies == 0) ;
    OK (GrB_Matrix_setElement (A, 99.4, 3, 3)) ;
    OK (GB_Matrix_check (A, "A blocking mode", 3)) ;
    GB_Global.mode = GrB_NONBLOCKING ;
    CHECK (A->npending == 0 && A->nzombies == 0) ;

    printf ("\nAll blocking/nonblocking mode tests passed\n") ;

    //--------------------------------------------------------------------------
    // restore all 'gunk' objects so they can be freed
    //--------------------------------------------------------------------------

    // printf ("\n-------------- Restore gunk objects:\n") ;

    expected = GrB_UNINITIALIZED_OBJECT ;

    WHERE ("GB_check [generic]") ;

    ERR (GB_check (Tgunk, "", 0)) ;
    ERR (GB_check (op1gunk, "", 0)) ;
    ERR (GB_check (op2gunk, "", 0)) ;
    ERR (GB_check (monoid_gunk, "", 0)) ;
    ERR (GB_check (semigunk, "", 0)) ;
    ERR (GB_check (vgunk, "", 0)) ;
    ERR (GB_check (Agunk, "", 0)) ;
    ERR (GB_check (dgunk, "", 0)) ;
    GB_check (selectopgunk, "", 3) ;
    ERR (GB_check (selectopgunk, "", 0)) ;

    #define REMAGIC(p) if (p != NULL) p->magic = MAGIC ;
    REMAGIC (Tgunk)
    REMAGIC (op1gunk)
    REMAGIC (op2gunk)
    REMAGIC (monoid_gunk)
    REMAGIC (semigunk)
    REMAGIC (vgunk)
    REMAGIC (Agunk)
    REMAGIC (dgunk)
    REMAGIC (selectopgunk)
    #undef REMAGIC

    OK (GB_check (Tgunk, "", 0)) ;
    OK (GB_check (op1gunk, "", 0)) ;
    OK (GB_check (op2gunk, "", 0)) ;
    OK (GB_check (monoid_gunk, "", 0)) ;
    OK (GB_check (semigunk, "", 0)) ;
    OK (GB_check (vgunk, "", 0)) ;
    OK (GB_check (Agunk, "", 0)) ;
    OK (GB_check (dgunk, "", 0)) ;
    OK (GB_check (selectopgunk, "", 0)) ;

    //--------------------------------------------------------------------------
    // GB_Descriptor_get
    //--------------------------------------------------------------------------

    expected = GrB_INVALID_OBJECT ;
    dgunk->out = 999 ;
    x_bool = false ;
    WHERE ("GB_Descriptor_get") ;
    ERR (GB_Descriptor_get (dgunk, &x_bool, NULL, NULL, NULL)) ;
    CHECK (x_bool == false) ;
    dgunk->out = GxB_DEFAULT ;

    //--------------------------------------------------------------------------
    // Wathen
    //--------------------------------------------------------------------------

    expected = GrB_INVALID_VALUE ;
    ERR (wathen (NULL, -1, -1, 0, 0, &x)) ;

    //--------------------------------------------------------------------------
    // malloc wrappers
    //--------------------------------------------------------------------------

    pp = &x ;
    pp = GB_malloc_memory (UINT64_MAX, 1) ;
    CHECK (pp == NULL) ;

    pp = &x ;
    pp = GB_calloc_memory (UINT64_MAX, 1) ;
    CHECK (pp == NULL) ;

    ok = true ;
    pp = GB_realloc_memory (UINT64_MAX, 0, 1, NULL, &ok) ;
    CHECK (!ok) ;

    s = 1 ;
    ok = GB_size_t_multiply (&s, UINT64_MAX, 0) ;
    CHECK (ok) ;
    CHECK (s == 0) ;

    s = 911 ;
    ok = GB_size_t_multiply (&s, UINT64_MAX/2, UINT64_MAX/2) ;
    CHECK (!ok) ;
    CHECK (s == 0) ;

    ok = GB_size_t_multiply (&s,
        ((size_t) UINT32_MAX)+2,
        ((size_t) UINT32_MAX)+1) ;
    CHECK (!ok) ;

    ok = GB_size_t_multiply (&s,
        ((size_t) UINT32_MAX)+1,
        ((size_t) UINT32_MAX)+1) ;
    CHECK (!ok) ;

    ok = GB_size_t_multiply (&s,
        ((size_t) UINT32_MAX)/2,
        ((size_t) UINT32_MAX)/2) ;
    CHECK (ok) ;
    CHECK (s == (((size_t) UINT32_MAX)/2) * (((size_t) UINT32_MAX)/2)) ; 
    WHERE ("GrB_error") ;

    GB_thread_local.info = 99 ;
    printf ("testing invalid error code:\n%s\n", GrB_error ( )) ;
    GB_thread_local.info = GrB_SUCCESS ;

    n = 1 ;
    ok = GB_Index_multiply (&n, INT64_MAX, 0) ;
    CHECK (ok) ;
    CHECK (n == 0) ;

    n = 911 ;
    ok = GB_Index_multiply (&n, -1, -1) ;
    CHECK (!ok) ;
    CHECK (n == 0) ;

    n = 911 ;
    ok = GB_Index_multiply (&n, 1, GB_INDEX_MAX+1) ;
    CHECK (!ok) ;
    CHECK (n == 0) ;

    ok = GB_Index_multiply (&n,
        ((GrB_Index) GB_INDEX_MAX)+1,
        ((GrB_Index) GB_INDEX_MAX)+1) ;
    CHECK (!ok) ;

    ok = GB_Index_multiply (&n,
        ((GrB_Index) GB_INDEX_MAX),
        ((GrB_Index) GB_INDEX_MAX)) ;
    CHECK (!ok) ;

    a = (GrB_Index) 16777216/2 ;     // (2^24)/2
    b = (GrB_Index) 16777216 ;
    ok = GB_Index_multiply (&n, a, b) ;
    // printf ("%lld %lld n\n", n, a*b) ;
    CHECK (ok) ;
    CHECK (n == a*b)

    //--------------------------------------------------------------------------
    // internal GB_* routines
    //--------------------------------------------------------------------------

    CHECK (A != NULL) ;
    ok = GB_Matrix_alloc (A, GB_INDEX_MAX+1, true, NULL) ;
    CHECK (!ok) ;

    CHECK (A != NULL) ;
    ok = GB_Matrix_realloc (A, GB_INDEX_MAX+1, true, NULL) ;
    CHECK (!ok) ;

    ok = GB_Matrix_realloc (A, 20, false, NULL) ;
    CHECK (ok) ;
    OK (GB_Matrix_check (A, "A pattern", 3)) ;

    GB_Matrix_ixfree (NULL) ;

    GrB_free (&C) ;
    GrB_free (&B) ;
    CHECK (C == NULL) ;
    CHECK (B == NULL) ;
    OK (GrB_Matrix_new (&C, GrB_FP32, 1, 1)) ;
    OK (GB_shallow_op (&B, GrB_AINV_FP32, C)) ;
    OK (GB_Matrix_check (B, "B empty, float", 3)) ;
    GrB_free (&B) ;
    OK (GB_shallow_cast (&B, GrB_FP64, C)) ;
    OK (GB_Matrix_check (B, "B empty, double", 3)) ;

    bool b1, b2 ;
    int64_t imin, imax ;
    OK (GB_ijproperties (I, 0, J, 0, 2, 2, &b1, &b2, &imin, &imax)) ;

    OK (GB_op_is_second (GrB_SECOND_FP64, NULL)) ;

    //--------------------------------------------------------------------------
    // check for inputs aliased with outputs
    //--------------------------------------------------------------------------

    Complex_finalize ( ) ;

    GrB_free (&A) ;
    GrB_free (&B) ;
    GrB_free (&C) ;
    GrB_free (&E) ;
    GrB_free (&F) ;
    GrB_free (&v) ;
    GrB_free (&u) ;
    GrB_free (&z) ;

    #define NWHAT 12
    n = NWHAT ;
    nvals = 40 ;
    uvals = 4 ;
    GrB_Index ilist [NWHAT] = { 8, 9, 0, 1, 5, 6, 11, 3, 2, 10, 7, 4 } ;
    GrB_Index jlist [NWHAT] = { 0, 11, 1, 7, 8, 4, 2, 3, 5, 6, 10, 9 } ;

    OK (random_matrix (&A, false, false, n, n, nvals, 0, false)) ;
    OK (GrB_Vector_new (&u, GrB_FP64, n)) ;
    OK (GrB_Vector_setElement (u, (double) 3.4, 0)) ;

    E = A ;
    GrB_Matrix_dup (&A, A) ;
    CHECK (GB_mx_isequal (A,E)) ;
    GrB_free (&E) ;

    z = u ;
    GrB_Vector_dup (&u, u) ;
    CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) z)) ;
    GrB_free (&z) ;

    for (int what = 0 ; what <= 2 ; what++)
    {

        GrB_Matrix Amask ;
        GrB_Vector umask ;
        switch (what)
        {
            case 0: Amask = NULL ; umask = NULL ; break ;
            case 1: Amask = A    ; umask = u    ; break ;
            case 2:
            OK (random_matrix (&Amask, false, false, n, n, nvals, 0, false)) ;
            OK (random_matrix (&F,     false, false, n, 1, uvals, 0, false)) ;
            umask = (GrB_Vector) F ;
            F = NULL ;
            break ;
        }

        //----------------------------------------------------------------------
        // GrB_mxm, GrB_vxm, and GrB_mxv
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_mxm (B, Amask, NULL, GxB_PLUS_TIMES_FP64, A, A, NULL)) ;
        OK (GrB_mxm (A, Amask, NULL, GxB_PLUS_TIMES_FP64, A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_vxm (v, umask, NULL, GxB_PLUS_TIMES_FP64, u, A, NULL)) ;
        OK (GrB_vxm (u, umask, NULL, GxB_PLUS_TIMES_FP64, u, A, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_mxv (v, umask, NULL, GxB_PLUS_TIMES_FP64, A, u, NULL)) ;
        OK (GrB_mxv (u, umask, NULL, GxB_PLUS_TIMES_FP64, A, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        //----------------------------------------------------------------------
        // GrB_eWiseMult
        //----------------------------------------------------------------------

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_eWiseMult (v, umask, NULL, GxB_PLUS_TIMES_FP64,  u, u, NULL)) ;
        OK (GrB_eWiseMult (u, umask, NULL, GxB_PLUS_TIMES_FP64,  u, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_eWiseMult (v, umask, NULL, GxB_PLUS_FP64_MONOID, u, u, NULL)) ;
        OK (GrB_eWiseMult (u, umask, NULL, GxB_PLUS_FP64_MONOID, u, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_eWiseMult (v, umask, NULL, GrB_PLUS_FP64,        u, u, NULL)) ;
        OK (GrB_eWiseMult (u, umask, NULL, GrB_PLUS_FP64,        u, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_eWiseMult (B, Amask, NULL, GxB_PLUS_TIMES_FP64,  A, A, NULL)) ;
        OK (GrB_eWiseMult (A, Amask, NULL, GxB_PLUS_TIMES_FP64,  A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_eWiseMult (B, Amask, NULL, GxB_PLUS_FP64_MONOID, A, A, NULL)) ;
        OK (GrB_eWiseMult (A, Amask, NULL, GxB_PLUS_FP64_MONOID, A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_eWiseMult (B, Amask, NULL, GrB_PLUS_FP64,        A, A, NULL)) ;
        OK (GrB_eWiseMult (A, Amask, NULL, GrB_PLUS_FP64,        A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        //----------------------------------------------------------------------
        // GrB_eWiseAdd
        //----------------------------------------------------------------------

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_eWiseAdd  (v, umask, NULL, GxB_PLUS_TIMES_FP64,  u, u, NULL)) ;
        OK (GrB_eWiseAdd  (u, umask, NULL, GxB_PLUS_TIMES_FP64,  u, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_eWiseAdd  (v, umask, NULL, GxB_PLUS_FP64_MONOID, u, u, NULL)) ;
        OK (GrB_eWiseAdd  (u, umask, NULL, GxB_PLUS_FP64_MONOID, u, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_eWiseAdd  (v, umask, NULL, GrB_PLUS_FP64,        u, u, NULL)) ;
        OK (GrB_eWiseAdd  (u, umask, NULL, GrB_PLUS_FP64,        u, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_eWiseAdd  (B, Amask, NULL, GxB_PLUS_TIMES_FP64,  A, A, NULL)) ;
        OK (GrB_eWiseAdd  (A, Amask, NULL, GxB_PLUS_TIMES_FP64,  A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_eWiseAdd  (B, Amask, NULL, GxB_PLUS_FP64_MONOID, A, A, NULL)) ;
        OK (GrB_eWiseAdd  (A, Amask, NULL, GxB_PLUS_FP64_MONOID, A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_eWiseAdd  (B, Amask, NULL, GrB_PLUS_FP64,        A, A, NULL)) ;
        OK (GrB_eWiseAdd  (A, Amask, NULL, GrB_PLUS_FP64,        A, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        //----------------------------------------------------------------------
        // GrB_extract
        //----------------------------------------------------------------------

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_extract   (u, umask, NULL, u, GrB_ALL, n, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_extract   (B, Amask, NULL, A, GrB_ALL, n, GrB_ALL, n, NULL)) ;
        OK (GrB_extract   (A, Amask, NULL, A, GrB_ALL, n, GrB_ALL, n, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_extract   (v, umask, NULL, A, GrB_ALL, n, 0, NULL)) ;
        OK (GrB_extract   (u, umask, NULL, A, GrB_ALL, n, 0, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        //----------------------------------------------------------------------
        // GxB_subassign
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GxB_subassign (B, Amask, NULL, A, GrB_ALL, n, GrB_ALL, n, NULL)) ;
        OK (GxB_subassign (A, Amask, NULL, A, GrB_ALL, n, GrB_ALL, n, NULL)) ;
        GB_wait (B) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GxB_subassign (B, Amask, NULL, A, ilist, n, jlist, n, NULL)) ;
        OK (GxB_subassign (A, Amask, NULL, A, ilist, n, jlist, n, NULL)) ;
        GB_wait (B) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GxB_subassign (v, umask, NULL, u, GrB_ALL, n, NULL)) ;
        OK (GxB_subassign (u, umask, NULL, u, GrB_ALL, n, NULL)) ;
        GB_wait ((GrB_Matrix) v) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GxB_subassign (v, umask, NULL, u, ilist, n, NULL)) ;
        OK (GxB_subassign (u, umask, NULL, u, ilist, n, NULL)) ;
        GB_wait ((GrB_Matrix) v) ;
        GB_wait ((GrB_Matrix) u) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        //----------------------------------------------------------------------
        // GrB_assign
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_assign (B, Amask, NULL, A, GrB_ALL, n, GrB_ALL, n, NULL)) ;
        OK (GrB_assign (A, Amask, NULL, A, GrB_ALL, n, GrB_ALL, n, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_assign (B, Amask, NULL, A, ilist, n, jlist, n, NULL)) ;
        OK (GrB_assign (A, Amask, NULL, A, ilist, n, jlist, n, NULL)) ;
        GB_wait (B) ;
        GB_wait (A) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_assign (v, umask, NULL, u, GrB_ALL, n, NULL)) ;
        OK (GrB_assign (u, umask, NULL, u, GrB_ALL, n, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_assign (v, umask, NULL, u, ilist, n, NULL)) ;
        OK (GrB_assign (u, umask, NULL, u, ilist, n, NULL)) ;
        GB_wait ((GrB_Matrix) v) ;
        GB_wait ((GrB_Matrix) u) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        //----------------------------------------------------------------------
        // GrB_apply
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_apply (B, Amask, NULL, GrB_AINV_FP64, A, NULL)) ;
        OK (GrB_apply (A, Amask, NULL, GrB_AINV_FP64, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GrB_apply (v, umask, NULL, GrB_AINV_FP64, u, NULL)) ;
        OK (GrB_apply (u, umask, NULL, GrB_AINV_FP64, u, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        //----------------------------------------------------------------------
        // GxB_select
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GxB_select (B, Amask, NULL, GxB_NONZERO, A, NULL, NULL)) ;
        OK (GxB_select (A, Amask, NULL, GxB_NONZERO, A, NULL, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        OK (GrB_Vector_dup (&v, u)) ;
        OK (GxB_select (v, umask, NULL, GxB_NONZERO, u, NULL, NULL)) ;
        OK (GxB_select (u, umask, NULL, GxB_NONZERO, u, NULL, NULL)) ;
        CHECK (GB_mx_isequal ((GrB_Matrix) u, (GrB_Matrix) v)) ;
        GrB_free (&v) ;

        //----------------------------------------------------------------------
        // GrB_transepose
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&B, A)) ;
        OK (GrB_transpose (B, Amask, NULL, A, NULL)) ;
        OK (GrB_transpose (A, Amask, NULL, A, NULL)) ;
        CHECK (GB_mx_isequal (A,B)) ;
        GrB_free (&B) ;

        if (what == 2)
        {
            GrB_free (&Amask) ;
            GrB_free (&umask) ;
        }
    }

    //--------------------------------------------------------------------------
    // free all
    //--------------------------------------------------------------------------

    // this is also done by FREE_ALL, but the list here is meant to be
    // accurate, so nmalloc should be zero at the check below

    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d\n", nmalloc) ;

    expected = GrB_NULL_POINTER ;
    ERR (GxB_stats (NULL)) ;

    GrB_free (&Empty1) ;       CHECK (Empty1       == NULL) ;
    GrB_free (&v) ;            CHECK (v            == NULL) ;
    GrB_free (&u) ;            CHECK (u            == NULL) ;
    GrB_free (&A) ;            CHECK (A            == NULL) ;
    GrB_free (&u) ;            CHECK (u            == NULL) ;
    GrB_free (&z) ;            CHECK (z            == NULL) ;
    GrB_free (&h) ;            CHECK (h            == NULL) ;
    GrB_free (&B) ;            CHECK (B            == NULL) ;
    GrB_free (&C) ;            CHECK (C            == NULL) ;
    GrB_free (&E) ;            CHECK (E            == NULL) ;
    GrB_free (&F) ;            CHECK (F            == NULL) ;
    GrB_free (&Z) ;            CHECK (Z            == NULL) ;
    GrB_free (&H) ;            CHECK (H            == NULL) ;
    GrB_free (&T) ;            CHECK (T            == NULL) ;
    GrB_free (&Agunk) ;        CHECK (Agunk        == NULL) ;
    GrB_free (&Tgunk) ;        CHECK (Tgunk        == NULL) ;
    GrB_free (&op1gunk) ;      CHECK (op1gunk      == NULL) ;
    GrB_free (&op2gunk) ;      CHECK (op2gunk      == NULL) ;
    GrB_free (&op3) ;          CHECK (op3          == NULL) ;
    GrB_free (&op3) ;          CHECK (op3          == NULL) ;
    GrB_free (&op1b) ;         CHECK (op1b         == NULL) ;
    GrB_free (&op2b) ;         CHECK (op2b         == NULL) ;
    GrB_free (&semiringb) ;    CHECK (semiringb    == NULL) ;
    GrB_free (&descb) ;        CHECK (descb        == NULL) ;
    GrB_free (&vb) ;           CHECK (vb           == NULL) ;
    GrB_free (&monoidb) ;      CHECK (monoidb      == NULL) ;
    GrB_free (&monoid_gunk) ;  CHECK (monoid_gunk  == NULL) ;
    GrB_free (&semigunk) ;     CHECK (semigunk     == NULL) ;
    GrB_free (&vgunk) ;        CHECK (vgunk        == NULL) ;
    GrB_free (&Aempty) ;       CHECK (Aempty       == NULL) ;
    GrB_free (&vempty) ;       CHECK (vempty       == NULL) ;
    GrB_free (&desc) ;         CHECK (desc         == NULL) ;
    GrB_free (&dtn) ;          CHECK (dtn          == NULL) ;
    GrB_free (&dnt) ;          CHECK (dnt          == NULL) ;
    GrB_free (&dtt) ;          CHECK (dtt          == NULL) ;
    GrB_free (&dgunk) ;        CHECK (dgunk        == NULL) ;
    GrB_free (&selectop) ;     CHECK (selectop     == NULL) ;
    GrB_free (&selectopgunk) ; CHECK (selectopgunk == NULL) ;

    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d before complex_finalize\n", nmalloc) ;
    Complex_finalize ( ) ;
    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d done\n", nmalloc) ;
    GrB_finalize ( ) ;
    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d all freed\n", nmalloc) ;

    FREE_ALL ;
    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d all freed\n", nmalloc) ;
    GrB_finalize ( ) ;
    GxB_stats (&stats) ; nmalloc = stats.nmalloc ;
    printf ("nmalloc %d after finalize\n", nmalloc) ;
    CHECK (nmalloc == 0) ;

    printf ("\ncheck errlog.txt for errors tested\n") ;
    printf ("All error-handling tests passed"
            " (all errors above were expected)\n") ;
    fprintf (f, "\nAll error-handling tests passed"
            " (all errors above were expected)\n") ;
    fclose (f) ;
}

