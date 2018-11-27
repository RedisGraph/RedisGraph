//------------------------------------------------------------------------------
// GB_mex_about: print the 'about' information
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Test lots of random stuff.  The function otherwise serves no purpose.

#include "GB_mex.h"

#define USAGE "GB_mex_about"

GrB_Info ack (int64_t *stuff, GrB_Matrix GunkIt)
{
    GB_WHERE ("ack") ;
    GB_RETURN_IF_NULL (stuff) ;
    GB_RETURN_IF_NULL_OR_FAULTY (GunkIt) ;
    return (GrB_SUCCESS) ;
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;

    GB_WHERE (USAGE) ;

    printf ("table of codes:\n") ;
    printf ("bool  code: %d class: %d\n", GB_BOOL_code   , mxLOGICAL_CLASS) ;
    printf ("int8  code: %d class: %d\n", GB_INT8_code   , mxINT8_CLASS   ) ;
    printf ("uint8 code: %d class: %d\n", GB_UINT8_code  , mxUINT8_CLASS  ) ;
    printf ("int16 code: %d class: %d\n", GB_INT16_code  , mxINT16_CLASS  ) ;
    printf ("uin16 code: %d class: %d\n", GB_UINT16_code , mxUINT16_CLASS ) ;
    printf ("int32 code: %d class: %d\n", GB_INT32_code  , mxINT32_CLASS  ) ;
    printf ("uin32 code: %d class: %d\n", GB_UINT32_code , mxUINT32_CLASS ) ;
    printf ("in64  code: %d class: %d\n", GB_INT64_code  , mxINT64_CLASS  ) ;
    printf ("uin64 code: %d class: %d\n", GB_UINT64_code , mxUINT64_CLASS ) ;
    printf ("fp32  code: %d class: %d\n", GB_FP32_code   , mxSINGLE_CLASS ) ;
    printf ("fp64  code: %d class: %d\n", GB_FP64_code   , mxDOUBLE_CLASS ) ;
    printf ("struct   class: %d\n", mxSTRUCT_CLASS) ;
    printf ("cell     class: %d\n", mxCELL_CLASS) ;
    printf ("void     class: %d\n", mxVOID_CLASS) ;
    printf ("function class: %d\n", mxFUNCTION_CLASS) ;
    printf ("unknown mxClassID: %d\n", mxUNKNOWN_CLASS ) ;

    printf ("in %s:\n%s", __FILE__, GrB_error ( )) ;

    printf ("sizeof (struct GB_Type_opaque) %d\n",
             sizeof (struct GB_Type_opaque)) ;
    printf ("sizeof (struct GB_UnaryOp_opaque) %d\n",
             sizeof (struct GB_UnaryOp_opaque)) ;
    printf ("sizeof (struct GB_BinaryOp_opaque) %d\n",
             sizeof (struct GB_BinaryOp_opaque)) ;
    printf ("sizeof (struct GB_SelectOp_opaque) %d\n",
             sizeof (struct GB_SelectOp_opaque)) ;
    printf ("sizeof (struct GB_Monoid_opaque) %d\n",
             sizeof (struct GB_Monoid_opaque)) ;
    printf ("sizeof (struct GB_Semiring_opaque) %d\n",
             sizeof (struct GB_Semiring_opaque)) ;
    printf ("sizeof (struct GB_Vector_opaque) %d\n",
             sizeof (struct GB_Vector_opaque)) ;
    printf ("sizeof (struct GB_Matrix_opaque) %d\n",
             sizeof (struct GB_Matrix_opaque)) ;
    printf ("sizeof (struct GB_Descriptor_opaque) %d\n",
             sizeof (struct GB_Descriptor_opaque)) ;

    GrB_Info info = GB_ERROR (GrB_PANIC, (GB_LOG,
        "just testing the error log ... not really a panic\n"
        "hello world, the answer is %d", 42)) ;

    printf ("%s", GrB_error ( )) ;

    size_t s ;
    GxB_Type_size (&s, GrB_BOOL  ) ; printf ("%d %d\n", s, sizeof (bool      ));
    GxB_Type_size (&s, GrB_INT8  ) ; printf ("%d %d\n", s, sizeof (int8_t    ));
    GxB_Type_size (&s, GrB_UINT8 ) ; printf ("%d %d\n", s, sizeof (uint8_t   ));
    GxB_Type_size (&s, GrB_INT16 ) ; printf ("%d %d\n", s, sizeof (int16_t   ));
    GxB_Type_size (&s, GrB_UINT16) ; printf ("%d %d\n", s, sizeof (uint16_t  ));
    GxB_Type_size (&s, GrB_INT32 ) ; printf ("%d %d\n", s, sizeof (int32_t   ));
    GxB_Type_size (&s, GrB_UINT32) ; printf ("%d %d\n", s, sizeof (uint32_t  ));
    GxB_Type_size (&s, GrB_INT64 ) ; printf ("%d %d\n", s, sizeof (int64_t   ));
    GxB_Type_size (&s, GrB_UINT64) ; printf ("%d %d\n", s, sizeof (uint64_t  ));
    GxB_Type_size (&s, GrB_FP32  ) ; printf ("%d %d\n", s, sizeof (float     ));
    GxB_Type_size (&s, GrB_FP64  ) ; printf ("%d %d\n", s, sizeof (double    ));

    printf ("info is %d\n", info) ;

    GrB_Type t ;

    GB_check (GrB_LNOT, "LNOT", GB3) ;
    GxB_UnaryOp_ztype (&t, GrB_LNOT) ;
    GB_check (t, "ztype", GB3) ;
    GxB_UnaryOp_xtype (&t, GrB_LNOT) ;
    GB_check (t, "xtype", GB3) ;

    GB_check (GxB_LNOT_FP32, "LNOT_FP32", GB3) ;
    GxB_UnaryOp_ztype (&t, GxB_LNOT_FP32) ;
    GB_check (t, "ztype", GB3) ;
    GxB_UnaryOp_xtype (&t, GxB_LNOT_FP32) ;
    GB_check (t, "xtype", GB3) ;

    GB_check (GxB_ISEQ_INT32, "ISEQ_INT32", GB3) ;
    GxB_BinaryOp_ztype (&t, GxB_ISEQ_INT32) ;
    GB_check (t, "ztype", GB3) ;
    GxB_BinaryOp_xtype (&t, GxB_ISEQ_INT32) ;
    GB_check (t, "xtype", GB3) ;
    GxB_BinaryOp_ytype (&t, GxB_ISEQ_INT32) ;
    GB_check (t, "ytype", GB3) ;

    GB_check (GrB_EQ_INT32, "EQ_INT32", GB3) ;
    GxB_BinaryOp_ztype (&t, GrB_EQ_INT32) ;
    GB_check (t, "ztype", GB3) ;
    GxB_BinaryOp_xtype (&t, GrB_EQ_INT32) ;
    GB_check (t, "xtype", GB3) ;
    GxB_BinaryOp_ytype (&t, GrB_EQ_INT32) ;
    GB_check (t, "ytype", GB3) ;

    GrB_Monoid m ;
    GrB_BinaryOp op ;

    GrB_Monoid_new (&m, GrB_PLUS_UINT16, (uint16_t) 0) ;
    GB_check (m, "plus uint16 monoid", GB3) ;
    uint16_t id ;
    GxB_Monoid_identity (&id, m) ;
    printf ("id is %d\n", id) ;
    GxB_Monoid_operator (&op, m) ;
    GB_check (op, "plus op from monoid", GB3) ;

    GrB_free (&m) ;

    int16_t id0 = GB_MINUS_INF (id0) ;

    GrB_Monoid_new (&m, GrB_MAX_INT16, id0) ;
    GB_check (m, "max int16 monoid", GB3) ;
    int16_t id1 ;
    GxB_Monoid_identity (&id1, m) ;
    printf ("id1 is %d\n", id1) ;
    GxB_Monoid_operator (&op, m) ;
    GB_check (op, "plus op from monoid", GB3) ;

    GrB_Semiring sem ;
    GrB_Semiring_new (&sem, m, GrB_TIMES_INT16) ;
    GB_check (sem, "\nnew sem", GB3) ;

    GrB_Monoid mm ;
    GxB_Semiring_add (&mm, sem) ;
    GB_check (mm, "sem mm", GB3) ;
    GxB_Semiring_multiply (&op, sem) ;
    GB_check (op, "sem mult", GB3) ;

    GrB_free (&m) ;
    GrB_free (&sem) ;

    int64_t *stuff = NULL ;
    int64_t ok = 44 ;
    GrB_Matrix Gunk ;
    GrB_Matrix_new (&Gunk, GrB_FP64, 5, 5) ;
    info = ack (&ok, Gunk) ;

    GxB_Matrix_type (&t, Gunk) ;
    GB_check (t, "matrix Gunk type is:", GB3) ;

    GrB_Vector victor ;
    GrB_Vector_new (&victor, GrB_UINT32, 43) ;
    GxB_Vector_type (&t, victor) ;
    GB_check (t, "victor type is:", GB3) ;
    GxB_Type_size (&s, t) ;
    printf ("and its size of type is %d\n", s) ;
    GrB_free (&victor) ;

    GrB_Descriptor Duh ;
    GrB_Desc_Value val ;

    GrB_Descriptor_new (&Duh) ;
    GB_check (Duh, "\n------------------------------------- Duh:", GB3) ;
    GxB_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ;
    GxB_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ;
    GxB_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ;
    GxB_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ;

    GxB_set (Duh, GrB_INP0, GrB_TRAN) ;
    GB_check (Duh, "\n------------------------------------- Duh set:", GB3) ;
    GxB_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ;
    GxB_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ;
    GxB_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ;
    GxB_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ;

    GxB_set (Duh, GrB_MASK, GrB_SCMP) ;
    GB_check (Duh, "\n-----Duh set mask", GB3) ;
    GxB_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ;
    GxB_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ;
    GxB_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ;
    GxB_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ;

    GxB_set (Duh, GrB_OUTP, GrB_REPLACE) ;
    GB_check (Duh, "\n-----Duh set out", GB3) ;
    GxB_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ;
    GxB_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ;
    GxB_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ;
    GxB_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ;

    GxB_set (Duh, GrB_MASK, GxB_DEFAULT) ;
    GB_check (Duh, "\n-----Duh set mask back", GB3) ;
    GxB_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ;
    GxB_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ;
    GxB_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ;
    GxB_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ;

    info = GxB_set (Duh, GrB_INP1, GrB_REPLACE) ;
    printf ("%s\n", GrB_error () ) ;
    GB_check (Duh, "\n-----Duh set in1", GB3) ;
    GxB_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ;
    GxB_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ;
    GxB_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ;
    GxB_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ;

    GrB_free (&Duh) ;


    printf ("ok:\n%s", GrB_error ( )) ;

    info = ack (NULL, Gunk) ;

    printf ("%s", GrB_error ( )) ;

    Gunk->magic = 999 ;
    info = ack (&ok, Gunk) ;
    printf ("%s", GrB_error ( )) ;

    printf ("\nall tests passed (errors expected; testing error handling)\n") ;

    printf ("\n=================== malloc debug: %d \n", malloc_debug) ;

    Gunk->magic = GB_MAGIC ;
    GrB_free (&Gunk) ;

    GB_check (Complex, "user Complex type", GB3) ;
    GxB_Type_size (&s, Complex) ;
    printf ("size is %d\n", (int) s) ;

    // test the #ifdefs
    #ifdef GxB_SUITESPARSE_GRAPHBLAS

    printf ("\nAbout:\n%s\n", GxB_ABOUT) ;
    printf ("Date: %s\n", GxB_DATE) ;
    printf ("Implementation: %d.%d.%d ("GBu")\n",
        GxB_IMPLEMENTATION_MAJOR,
        GxB_IMPLEMENTATION_MINOR,
        GxB_IMPLEMENTATION_SUB,
        GxB_IMPLEMENTATION) ;
    printf ("License:%s\n", GxB_LICENSE) ;
    printf ("Spec: %d.%d.%d ("GBu")\n",
        GxB_MAJOR, GxB_MINOR, GxB_SUB, GxB) ;
    printf ("Spec Date: %s\n", GxB_SPEC_DATE) ;
    printf ("About the spec:\n%s\n", GxB_SPEC) ;

    #if GxB >= GxB_VERSION(1,0,0)
    printf ("The spec is >= version 1.0.0\n") ;
    #else
    printf ("The spec is < version 1.0.0\n") ;
    #endif

    #if GxB < GxB_VERSION(2,3,0)
    printf ("The spec is < version 2.3.0\n") ;
    #else
    printf ("The spec is >= version 2.3.0\n") ;
    #endif

    #if GxB_IMPLEMENTATION < GxB_VERSION(1,0,0)
    printf ("This implementation is <  version 1.0.0\n") ;
    #else
    printf ("This implementation is >= version 1.0.0\n") ;
    #endif

    #endif

    printf ("hyper_ratio %g csc %d\n",
        GB_Global.hyper_ratio, GB_Global.is_csc) ;

    printf ("built-in types:\n") ;
    GB_check (GrB_BOOL, "bool", GB3) ;
    GB_check (GrB_INT8, "int8", GB3) ;
    GB_check (GrB_UINT8, "uint8", GB3) ;
    GB_check (GrB_INT16, "int16", GB3) ;
    GB_check (GrB_UINT16, "uint16", GB3) ;
    GB_check (GrB_INT32, "int32", GB3) ;
    GB_check (GrB_UINT32, "uint32", GB3) ;
    GB_check (GrB_INT64, "int64", GB3) ;
    GB_check (GrB_UINT64, "uint64", GB3) ;
    GB_check (GrB_FP32, "fp32", GB3) ;
    GB_check (GrB_FP64, "fp64", GB3) ;

    printf ("\nprinting built-in types:\n") ;
    bool       b = true ;
    int8_t    i8 = 22   ;
    uint8_t   u8 = 44   ;
    int16_t  i16 = 909  ;
    uint16_t u16 = 777  ;
    int32_t  i32 = 3203 ;
    uint32_t u32 = 8080 ;
    int64_t  i64 = -987 ;
    uint64_t u64 = 987  ;
    float    f32 = 3.14 ;
    double   f64 = 99.4 ;

    GB_code_check (GB_BOOL_code,   &b  , stdout, Context) ; printf ("\n");
    GB_code_check (GB_INT8_code,   &i8 , stdout, Context) ; printf ("\n");
    GB_code_check (GB_UINT8_code,  &u8 , stdout, Context) ; printf ("\n");
    GB_code_check (GB_INT16_code,  &i16, stdout, Context) ; printf ("\n");
    GB_code_check (GB_UINT16_code, &u16, stdout, Context) ; printf ("\n");
    GB_code_check (GB_INT32_code,  &i32, stdout, Context) ; printf ("\n");
    GB_code_check (GB_UINT32_code, &u32, stdout, Context) ; printf ("\n");
    GB_code_check (GB_INT64_code,  &i64, stdout, Context) ; printf ("\n");
    GB_code_check (GB_UINT64_code, &u64, stdout, Context) ; printf ("\n");
    GB_code_check (GB_FP32_code,   &f32, stdout, Context) ; printf ("\n");
    GB_code_check (GB_FP64_code,   &f64, stdout, Context) ; printf ("\n");
    GB_code_check (GB_UDT_code,    &f64, stdout, Context) ; printf ("\n");
    GB_code_check (GB_UCT_code,    &f64, stdout, Context) ; printf ("\n");

    for (int i = 0 ; i <= GrB_PANIC + 1 ; i++)
    {
        printf ("info: %2d %s\n", i, GB_status_code (i)) ;
    }

    #if defined (USER_POSIX_THREADS)
    printf ("User threads: POSIX\n") ;
    #elif defined (USER_WINDOWS_THREADS)
    printf ("User threads: Windows\n") ;
    #elif defined (USER_ANSI_THREADS)
    printf ("User threads: ANSI\n") ;
    #elif defined (USER_OPENMP_THREADS)
    printf ("User threads: OpenMP\n") ;
    #elif defined (USER_NO_THREADS)
    printf ("User threads: none\n") ;
    #else
    printf ("User threads: not specific (none)\n") ;
    #endif

    GrB_Mode mode ;
    GxB_get (GxB_MODE, &mode) ;
    printf ("mode: %d\n", mode) ;

    GxB_Thread_Model threading ;
    GxB_get (GxB_THREAD_SAFETY, &threading) ;
    printf ("thread safety: %d\n", threading) ;
    GxB_get (GxB_THREADING, &threading) ;
    printf ("threading: %d\n", threading) ;

    GB_mx_put_global (true, 0) ;
}

