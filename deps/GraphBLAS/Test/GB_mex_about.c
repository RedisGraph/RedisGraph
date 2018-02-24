//------------------------------------------------------------------------------
// GB_mex_about: print the 'about' information
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Test lots of random stuff.  The function otherwise serves no purpose.

#include "GB_mex.h"
#include "../Source/GB.h"

GrB_Info ack (int64_t *stuff, GrB_Matrix GunkIt)
{
    RETURN_IF_NULL (stuff) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (GunkIt) ;
    return (REPORT_SUCCESS) ;
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global ( ) ;

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

    printf ("sizeof (GB_UnaryOp_opaque) %d\n",  sizeof (GB_UnaryOp_opaque)) ;
    printf ("sizeof (GB_BinaryOp_opaque) %d\n", sizeof (GB_BinaryOp_opaque)) ;
    printf ("sizeof (GB_Type_opaque) %d\n",     sizeof (GB_Type_opaque)) ;
    printf ("sizeof (GB_Monoid_opaque) %d\n",   sizeof (GB_Monoid_opaque)) ;
    printf ("sizeof (GB_Semiring_opaque) %d\n", sizeof (GB_Semiring_opaque)) ;
    printf ("sizeof (GB_Vector_opaque) %d\n",   sizeof (GB_Vector_opaque)) ;
    printf ("sizeof (GB_Matrix_opaque) %d\n",   sizeof (GB_Matrix_opaque)) ;
    printf ("sizeof (GB_Descriptor_opaque) %d\n",
        sizeof (GB_Descriptor_opaque)) ;

    WHERE ("GB_mex_about") ;
    GrB_Info info = ERROR (GrB_PANIC, (LOG,
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

    GB_check (GrB_LNOT, "LNOT", 3) ;
    GxB_UnaryOp_ztype (&t, GrB_LNOT) ;
    GB_check (t, "ztype", 3) ;
    GxB_UnaryOp_xtype (&t, GrB_LNOT) ;
    GB_check (t, "xtype", 3) ;

    GB_check (GxB_LNOT_FP32, "LNOT_FP32", 3) ;
    GxB_UnaryOp_ztype (&t, GxB_LNOT_FP32) ;
    GB_check (t, "ztype", 3) ;
    GxB_UnaryOp_xtype (&t, GxB_LNOT_FP32) ;
    GB_check (t, "xtype", 3) ;

    GB_check (GxB_ISEQ_INT32, "ISEQ_INT32", 3) ;
    GxB_BinaryOp_ztype (&t, GxB_ISEQ_INT32) ;
    GB_check (t, "ztype", 3) ;
    GxB_BinaryOp_xtype (&t, GxB_ISEQ_INT32) ;
    GB_check (t, "xtype", 3) ;
    GxB_BinaryOp_ytype (&t, GxB_ISEQ_INT32) ;
    GB_check (t, "ytype", 3) ;

    GB_check (GrB_EQ_INT32, "EQ_INT32", 3) ;
    GxB_BinaryOp_ztype (&t, GrB_EQ_INT32) ;
    GB_check (t, "ztype", 3) ;
    GxB_BinaryOp_xtype (&t, GrB_EQ_INT32) ;
    GB_check (t, "xtype", 3) ;
    GxB_BinaryOp_ytype (&t, GrB_EQ_INT32) ;
    GB_check (t, "ytype", 3) ;

    GrB_Monoid m ;
    GrB_BinaryOp op ;

    GrB_Monoid_new (&m, GrB_PLUS_UINT16, (uint16_t) 0) ;
    GB_check (m, "plus uint16 monoid", 3) ;
    uint16_t id ;
    GxB_Monoid_identity (&id, m) ;
    printf ("id is %d\n", id) ;
    GxB_Monoid_operator (&op, m) ;
    GB_check (op, "plus op from monoid", 3) ;

    GrB_free (&m) ;

    int16_t id0 = MINUS_INF (id0) ;
    
    GrB_Monoid_new (&m, GrB_MAX_INT16, id0) ;
    GB_check (m, "max int16 monoid", 3) ;
    int16_t id1 ;
    GxB_Monoid_identity (&id1, m) ;
    printf ("id1 is %d\n", id1) ;
    GxB_Monoid_operator (&op, m) ;
    GB_check (op, "plus op from monoid", 3) ;

    GrB_Semiring sem ;
    GrB_Semiring_new (&sem, m, GrB_TIMES_INT16) ;
    GB_check (sem, "\nnew sem", 3) ;

    GrB_Monoid mm ;
    GxB_Semiring_add (&mm, sem) ;
    GB_check (mm, "sem mm", 3) ;
    GxB_Semiring_multiply (&op, sem) ;
    GB_check (op, "sem mult", 3) ;

    GrB_free (&m) ;
    GrB_free (&sem) ;

    int64_t *stuff = NULL ;
    int64_t ok = 44 ;
    GrB_Matrix Gunk ;
    GrB_Matrix_new (&Gunk, GrB_FP64, 5, 5) ;
    info = ack (&ok, Gunk) ;

    GxB_Matrix_type (&t, Gunk) ;
    GB_check (t, "matrix Gunk type is:", 3) ;

    GrB_Vector victor ;
    GrB_Vector_new (&victor, GrB_UINT32, 43) ;
    GxB_Vector_type (&t, victor) ;
    GB_check (t, "victor type is:", 3) ;
    GxB_Type_size (&s, t) ;
    printf ("and its size of type is %d\n", s) ;
    GrB_free (&victor) ;

    GrB_Descriptor Duh ;
    GrB_Desc_Value val ;

    GrB_Descriptor_new (&Duh) ;
    GB_check (Duh, "\n------------------------------------- Duh:", 3) ;
    GxB_Descriptor_get (&val, Duh, GrB_OUTP) ; printf ("got outp %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_MASK) ; printf ("got mask %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP0) ; printf ("got inp0 %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP1) ; printf ("got inp1 %d\n", val) ;

    GrB_Descriptor_set (Duh, GrB_INP0, GrB_TRAN) ;
    GB_check (Duh, "\n------------------------------------- Duh set:", 3) ;
    GxB_Descriptor_get (&val, Duh, GrB_OUTP) ; printf ("got outp %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_MASK) ; printf ("got mask %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP0) ; printf ("got inp0 %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP1) ; printf ("got inp1 %d\n", val) ;

    GrB_Descriptor_set (Duh, GrB_MASK, GrB_SCMP) ;
    GB_check (Duh, "\n-----Duh set mask", 3) ;
    GxB_Descriptor_get (&val, Duh, GrB_OUTP) ; printf ("got outp %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_MASK) ; printf ("got mask %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP0) ; printf ("got inp0 %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP1) ; printf ("got inp1 %d\n", val) ;

    GrB_Descriptor_set (Duh, GrB_OUTP, GrB_REPLACE) ;
    GB_check (Duh, "\n-----Duh set out", 3) ;
    GxB_Descriptor_get (&val, Duh, GrB_OUTP) ; printf ("got outp %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_MASK) ; printf ("got mask %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP0) ; printf ("got inp0 %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP1) ; printf ("got inp1 %d\n", val) ;

    GrB_Descriptor_set (Duh, GrB_MASK, GxB_DEFAULT) ;
    GB_check (Duh, "\n-----Duh set mask back", 3) ;
    GxB_Descriptor_get (&val, Duh, GrB_OUTP) ; printf ("got outp %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_MASK) ; printf ("got mask %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP0) ; printf ("got inp0 %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP1) ; printf ("got inp1 %d\n", val) ;

    info = GrB_Descriptor_set (Duh, GrB_INP1, GrB_REPLACE) ;
    printf ("%s\n", GrB_error () ) ;
    GB_check (Duh, "\n-----Duh set in1", 3) ;
    GxB_Descriptor_get (&val, Duh, GrB_OUTP) ; printf ("got outp %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_MASK) ; printf ("got mask %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP0) ; printf ("got inp0 %d\n", val) ;
    GxB_Descriptor_get (&val, Duh, GrB_INP1) ; printf ("got inp1 %d\n", val) ;

    GrB_free (&Duh) ;


    printf ("ok:\n%s", GrB_error ( )) ;

    info = ack (NULL, Gunk) ;

    printf ("%s", GrB_error ( )) ;
    
    Gunk->magic = 999 ;
    info = ack (&ok, Gunk) ;
    printf ("%s", GrB_error ( )) ;

    printf ("\nall tests passed (errors expected; testing error handling)\n") ;

    printf ("\n=================== MALLOC DEBUG: %d \n", malloc_debug) ;

    Gunk->magic = MAGIC ;
    GrB_free (&Gunk) ;

    GB_check (Complex, "user Complex type", 3) ;
    GxB_Type_size (&s, Complex) ;
    printf ("size is %d\n", (int) s) ;

    // test the #ifdefs
    #ifdef GXB_SUITESPARSE_GRAPHBLAS

    printf ("\nAbout:\n%s\n", GXB_ABOUT) ;
    printf ("Date: %s\n", GXB_DATE) ;
    printf ("Implementation: %d.%d.%d ("GBu")\n",
        GXB_IMPLEMENTATION_MAJOR,
        GXB_IMPLEMENTATION_MINOR,
        GXB_IMPLEMENTATION_SUB,
        GXB_IMPLEMENTATION) ;
    printf ("License:%s\n", GXB_LICENSE) ;
    printf ("Spec: %d.%d.%d ("GBu")\n",
        GXB_MAJOR, GXB_MINOR, GXB_SUB, GXB) ;
    printf ("Spec Date: %s\n", GXB_SPEC_DATE) ;
    printf ("About the spec:\n%s\n", GXB_SPEC) ;

    #if GXB >= GXB_VERSION(1,0,0)
    printf ("The spec is >= version 1.0.0\n") ;
    #else
    printf ("The spec is < version 1.0.0\n") ;
    #endif

    #if GXB < GXB_VERSION(2,3,0)
    printf ("The spec is < version 2.3.0\n") ;
    #else
    printf ("The spec is >= version 2.3.0\n") ;
    #endif

    #if GXB_IMPLEMENTATION < GXB_VERSION(1,0,0)
    printf ("This implementation is <  version 1.0.0\n") ;
    #else
    printf ("This implementation is >= version 1.0.0\n") ;
    #endif

    #endif

    GB_mx_put_global (malloc_debug) ;
}

