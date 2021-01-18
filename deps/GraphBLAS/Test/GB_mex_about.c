//------------------------------------------------------------------------------
// GB_mex_about: print the 'about' information
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Test lots of random stuff.  The function otherwise serves no purpose.

#include "GB_mex.h"
#include "GB_mex_errors.h"
#include "GB_bitmap_AxB_saxpy.h"

#define USAGE "GB_mex_about"

GrB_Info ack (int64_t *stuff, GrB_Matrix GunkIt) ;

GrB_Info ack (int64_t *stuff, GrB_Matrix GunkIt)
{
    GB_RETURN_IF_NULL (stuff) ;
    GB_RETURN_IF_NULL_OR_FAULTY (GunkIt) ;
    return (GrB_SUCCESS) ;
}

bool select_plus_one (GrB_Index i, GrB_Index j, 
    const double *x, const double *thunk) ;

bool select_nothing (GrB_Index i, GrB_Index j,
    const void *x, const void *thunk) ;

bool select_plus_one (GrB_Index i, GrB_Index j,
    const double *x, const double *thunk)
{
    // return true if x >= thunk+1
    return ((*x) >= ((*thunk)+1)) ;
}

bool select_nothing (GrB_Index i, GrB_Index j,
    const void *x, const void *thunk)
{
    return (false) ;
}

typedef int16_t user_int ;

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // start error log
    //--------------------------------------------------------------------------

    FILE *f = fopen ("errlog2.txt", "w") ;
    printf ("in %s\n", __FILE__) ;
    char *err ;

    //--------------------------------------------------------------------------
    // test GrB_init with invalid mode
    //--------------------------------------------------------------------------

    GB_Global_GrB_init_called_set (false) ;
    GrB_Info info = GrB_init (911) ;
    printf ("expected error: [%d]\n", info) ;
    mxAssert (info == GrB_INVALID_VALUE, "error must be 'invalid value'") ;

    bool malloc_debug = GB_mx_get_global (true) ;

    // GB_CONTEXT (USAGE) ;

    printf ("in %s:\n", __FILE__) ;

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

    GB_UnaryOp_check (GrB_LNOT, "LNOT", GxB_COMPLETE, stdout) ;
    GxB_UnaryOp_ztype (&t, GrB_LNOT) ;
    GB_UnaryOp_check (t, "ztype", GxB_COMPLETE, stdout) ;
    GxB_UnaryOp_xtype (&t, GrB_LNOT) ;
    GB_UnaryOp_check (t, "xtype", GxB_COMPLETE, stdout) ;

    GB_UnaryOp_check (GxB_LNOT_FP32, "LNOT_FP32", GxB_COMPLETE, stdout) ;
    GxB_UnaryOp_ztype (&t, GxB_LNOT_FP32) ;
    GB_UnaryOp_check (t, "ztype", GxB_COMPLETE, stdout) ;
    GxB_UnaryOp_xtype (&t, GxB_LNOT_FP32) ;
    GB_UnaryOp_check (t, "xtype", GxB_COMPLETE, stdout) ;

    GB_BinaryOp_check (GxB_ISEQ_INT32, "ISEQ_INT32", GxB_COMPLETE, stdout) ;
    GxB_BinaryOp_ztype (&t, GxB_ISEQ_INT32) ;
    GB_BinaryOp_check (t, "ztype", GxB_COMPLETE, stdout) ;
    GxB_BinaryOp_xtype (&t, GxB_ISEQ_INT32) ;
    GB_BinaryOp_check (t, "xtype", GxB_COMPLETE, stdout) ;
    GxB_BinaryOp_ytype (&t, GxB_ISEQ_INT32) ;
    GB_BinaryOp_check (t, "ytype", GxB_COMPLETE, stdout) ;

    GB_BinaryOp_check (GrB_EQ_INT32, "EQ_INT32", GxB_COMPLETE, stdout) ;
    GxB_BinaryOp_ztype (&t, GrB_EQ_INT32) ;
    GB_BinaryOp_check (t, "ztype", GxB_COMPLETE, stdout) ;
    GxB_BinaryOp_xtype (&t, GrB_EQ_INT32) ;
    GB_BinaryOp_check (t, "xtype", GxB_COMPLETE, stdout) ;
    GxB_BinaryOp_ytype (&t, GrB_EQ_INT32) ;
    GB_BinaryOp_check (t, "ytype", GxB_COMPLETE, stdout) ;

    GrB_Monoid m ;
    GrB_BinaryOp op ;

    GrB_Monoid_new_UINT16_(&m, GrB_PLUS_UINT16, (uint16_t) 0) ;
    OK (GrB_Monoid_wait_(&m)) ;
    GB_Monoid_check (m, "plus uint16 monoid", GxB_COMPLETE, stdout) ;
    uint16_t id ;
    GxB_Monoid_identity (&id, m) ;
    printf ("id is %d\n", id) ;
    GxB_Monoid_operator (&op, m) ;
    GB_Monoid_check (op, "plus op from monoid", GxB_COMPLETE, stdout) ;

    GrB_Monoid_free_(&m) ;

    int16_t id0 = INT16_MIN ;

    GrB_Monoid_new_INT16_(&m, GrB_MAX_INT16, id0) ;
    GB_Monoid_check (m, "max int16 monoid", GxB_COMPLETE, stdout) ;
    int16_t id1 ;
    GxB_Monoid_identity (&id1, m) ;
    printf ("id1 is %d\n", id1) ;
    GxB_Monoid_operator (&op, m) ;
    GB_BinaryOp_check (op, "plus op from monoid", GxB_COMPLETE, stdout) ;

    GrB_Semiring sem ;
    GrB_Semiring_new (&sem, m, GrB_TIMES_INT16) ;
    OK (GrB_Semiring_wait_(&sem)) ;
    GB_Semiring_check (sem, "\nnew sem", GxB_COMPLETE, stdout) ;

    GrB_Monoid mm ;
    GxB_Semiring_add (&mm, sem) ;
    GB_Monoid_check (mm, "sem mm", GxB_COMPLETE, stdout) ;
    GxB_Semiring_multiply (&op, sem) ;
    GB_BinaryOp_check (op, "sem mult", GxB_COMPLETE, stdout) ;

    GrB_Monoid_free_(&m) ;
    GrB_Semiring_free_(&sem) ;

    int64_t *stuff = NULL ;
    int64_t morestuff = 44 ;
    GrB_Matrix Gunk ;
    GrB_Matrix_new (&Gunk, GrB_FP64, 5, 5) ;
    info = ack (&morestuff, Gunk) ;

    GxB_Matrix_type (&t, Gunk) ;
    GB_Type_check (t, "matrix Gunk type is:", GxB_COMPLETE, stdout) ;

    GrB_Vector victor ;
    GrB_Vector_new (&victor, GrB_UINT32, 43) ;
    GxB_Vector_type (&t, victor) ;
    OK (GrB_Vector_wait_(&victor)) ;
    GB_Type_check (t, "victor type is:", GxB_COMPLETE, stdout) ;
    GxB_Type_size (&s, t) ;
    printf ("and its size of type is %d\n", s) ;
    GrB_Vector_free_(&victor) ;

    //--------------------------------------------------------------------------
    // descriptor
    //--------------------------------------------------------------------------

    GrB_Descriptor Duh ;
    GrB_Desc_Value val ;

    GrB_Descriptor_new (&Duh) ;
    GB_Descriptor_check (Duh, "\n---------------------------------- Duh:",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    GxB_Desc_set (Duh, GrB_INP0, GrB_TRAN) ;
    GB_Descriptor_check (Duh, "\n------------------------------- Duh set:",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GrB_TRAN) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    GxB_Desc_set (Duh, GrB_MASK, GrB_COMP) ;
    GB_Descriptor_check (Duh, "\n-----Duh set mask",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GrB_COMP) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GrB_TRAN) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    GxB_Desc_set (Duh, GrB_OUTP, GrB_REPLACE) ;
    GB_Descriptor_check (Duh, "\n-----Duh set out",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GrB_REPLACE) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GrB_COMP) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GrB_TRAN) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    GrB_Descriptor_set (Duh, GrB_MASK, GrB_STRUCTURE) ;
    GB_Descriptor_check (Duh, "\n-----Duh set mask structural",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GrB_REPLACE) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GrB_COMP + GrB_STRUCTURE) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GrB_TRAN) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    GrB_Descriptor_set (Duh, GrB_MASK, GxB_DEFAULT) ;
    GB_Descriptor_check (Duh, "\n-----Duh set mask back",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GrB_REPLACE) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GrB_TRAN) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    info = GxB_Desc_set (Duh, GrB_INP1, GrB_REPLACE) ;
    OK (GrB_Descriptor_wait_(&Duh)) ;
    GrB_Descriptor_error_(&err, Duh) ;
    printf ("%s\n", err) ;
    GB_Descriptor_check (Duh, "\n-----Duh set in1",
        GxB_COMPLETE, stdout) ;
    GxB_Desc_get (Duh, GrB_OUTP, &val) ; printf ("got outp %d\n", val) ; CHECK (val == GrB_REPLACE) ;
    GxB_Desc_get (Duh, GrB_MASK, &val) ; printf ("got mask %d\n", val) ; CHECK (val == GxB_DEFAULT) ;
    GxB_Desc_get (Duh, GrB_INP0, &val) ; printf ("got inp0 %d\n", val) ; CHECK (val == GrB_TRAN) ;
    GxB_Desc_get (Duh, GrB_INP1, &val) ; printf ("got inp1 %d\n", val) ; CHECK (val == GxB_DEFAULT) ;

    GrB_Descriptor_free_(&Duh) ;

    //--------------------------------------------------------------------------
    // error handling
    //--------------------------------------------------------------------------

    info = ack (NULL, Gunk) ;

    Gunk->magic = 999 ;
    info = ack (&morestuff, Gunk) ;

    Gunk->magic = GB_MAGIC ;
    GrB_Matrix_free_(&Gunk) ;

    GB_Type_check (Complex, "user Complex type", GxB_COMPLETE, stdout);
    GxB_Type_size (&s, Complex) ;
    printf ("size is %d\n", (int) s) ;

    //--------------------------------------------------------------------------
    // about the spec
    //--------------------------------------------------------------------------

    int all_version [3] = { -1, -1, -1 } ;
    unsigned int version = 0 , subversion = 9999 ;
    char *name, *date, *about, *license, *compile_date, *compile_time, *url ;

    OK (GrB_getVersion (&version, &subversion)) ;
    printf ("Spec: %d.%d.%d ("GBu"): %d.%d\n",
        GxB_SPEC_MAJOR, GxB_SPEC_MINOR, GxB_SPEC_SUB, GxB_SPEC_VERSION,
        version, subversion) ;
    CHECK (version == GxB_SPEC_MAJOR) ;
    CHECK (subversion == GxB_SPEC_MINOR) ;
    CHECK (version == GRB_VERSION) ;
    CHECK (subversion == GRB_SUBVERSION) ;
    printf ("Spec Date: %s\n", GxB_SPEC_DATE) ;

    OK (GxB_Global_Option_get_(GxB_API_ABOUT, &about)) ;
    CHECK (strcmp (about, GxB_SPEC_ABOUT) == 0) ;
    printf ("About the spec:\n%s\n", about) ;

    OK (GxB_Global_Option_get_(GxB_API_DATE, &date)) ;
    CHECK (strcmp (date, GxB_SPEC_DATE) == 0) ;
    printf ("date: %s\n", date) ;

    OK (GxB_Global_Option_get_(GxB_API_URL, &url)) ;
    printf ("URL: %s\n", url) ;

    OK (GxB_Global_Option_get_(GxB_API_VERSION, all_version)) ;
    CHECK (all_version [0] == GxB_SPEC_MAJOR) ;
    CHECK (all_version [1] == GxB_SPEC_MINOR) ;
    CHECK (all_version [2] == GxB_SPEC_SUB) ;
    printf ("Spec Version (%d.%d.%d)\n",
        all_version [0], all_version [1], all_version [2]) ;

    //--------------------------------------------------------------------------
    // about the library
    //--------------------------------------------------------------------------

    #ifdef GxB_SUITESPARSE_GRAPHBLAS
    printf ("library info:\n") ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_NAME, &name)) ;
    CHECK (strcmp (name, GxB_IMPLEMENTATION_NAME) == 0) ;
    printf ("name: %s\n", name) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_DATE, &date)) ;
    if (date != NULL) printf ("date: %s\n", date) ;
    CHECK (strcmp (date, GxB_IMPLEMENTATION_DATE) == 0) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_ABOUT, &about)) ;
    CHECK (strcmp (about, GxB_IMPLEMENTATION_ABOUT) == 0) ;
    printf ("about:\n%s\n", about) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_LICENSE, &license)) ;
    CHECK (strcmp (license, GxB_IMPLEMENTATION_LICENSE) == 0) ;
    printf ("license:\n%s\n", license) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_VERSION, all_version)) ;
    CHECK (all_version [0] == GxB_IMPLEMENTATION_MAJOR) ;
    CHECK (all_version [1] == GxB_IMPLEMENTATION_MINOR) ;
    CHECK (all_version [2] == GxB_IMPLEMENTATION_SUB) ;
    printf ("Version (%d.%d.%d)\n",
        all_version [0], all_version [1], all_version [2]) ;
    printf ("Implementation: ("GBu")\n", GxB_IMPLEMENTATION) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_COMPILE_DATE, &compile_date)) ;
    printf ("compile date: %s\n", compile_date) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_COMPILE_TIME, &compile_time)) ;
    printf ("compile time: %s\n", compile_time) ;

    OK (GxB_Global_Option_get_(GxB_LIBRARY_URL, &url)) ;
    printf ("URL: %s\n", url) ;

    #if GxB_SPEC_VERSION >= GxB_VERSION(1,0,0)
    printf ("The spec is >= version 1.0.0\n") ;
    #else
    printf ("The spec is < version 1.0.0\n") ;
    #endif

    #if GxB_SPEC_VERSION < GxB_VERSION(2,3,0)
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

    //--------------------------------------------------------------------------
    // CUDA
    //--------------------------------------------------------------------------

    int gpu_count = GB_Global_gpu_count_get ( ) ;
    printf ("gpu count: %d\n", gpu_count) ;

    GrB_Desc_Value gpu_control = -99 ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_GPU_CONTROL, &gpu_control)) ;
    printf ("gpu control: %d\n", gpu_control) ;

    OK (GxB_Global_Option_set_(GxB_GLOBAL_GPU_CONTROL, GxB_GPU_NEVER)) ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_GPU_CONTROL, &gpu_control)) ;
    CHECK (gpu_control == GxB_GPU_NEVER) ;

    OK (GxB_Global_Option_set_(GxB_GLOBAL_GPU_CONTROL, GxB_GPU_ALWAYS)) ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_GPU_CONTROL, &gpu_control)) ;
    CHECK (gpu_control == (gpu_count > 0) ? GxB_GPU_ALWAYS : GxB_GPU_NEVER) ;

    OK (GxB_Global_Option_set_(GxB_GLOBAL_GPU_CONTROL, GxB_DEFAULT)) ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_GPU_CONTROL, &gpu_control)) ;
    CHECK (gpu_control == (gpu_count > 0) ? GxB_DEFAULT : GxB_GPU_NEVER) ;

    double gpu_chunk = -1 ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_GPU_CHUNK, &gpu_chunk)) ;
    printf ("gpu chunk: %g\n", gpu_chunk) ;

    double gpu_chunk_42 = 42e6 ;
    OK (GxB_Global_Option_set_(GxB_GLOBAL_GPU_CHUNK, gpu_chunk_42)) ;
    OK (GxB_Global_Option_get_(GxB_GLOBAL_GPU_CHUNK, &gpu_chunk)) ;
    CHECK (gpu_chunk == 42e6) ;

    //--------------------------------------------------------------------------
    // types
    //--------------------------------------------------------------------------

    printf ("built-in types:\n") ;
    GB_Type_check (GrB_BOOL, "bool", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_INT8, "int8", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_UINT8, "uint8", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_INT16, "int16", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_UINT16, "uint16", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_INT32, "int32", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_UINT32, "uint32", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_INT64, "int64", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_UINT64, "uint64", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_FP32, "fp32", GxB_COMPLETE, stdout) ;
    GB_Type_check (GrB_FP64, "fp64", GxB_COMPLETE, stdout) ;

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

    GB_code_check (GB_BOOL_code,   &b  , 5, stdout) ; printf ("\n");
    GB_code_check (GB_INT8_code,   &i8 , 5, stdout) ; printf ("\n");
    GB_code_check (GB_UINT8_code,  &u8 , 5, stdout) ; printf ("\n");
    GB_code_check (GB_INT16_code,  &i16, 5, stdout) ; printf ("\n");
    GB_code_check (GB_UINT16_code, &u16, 5, stdout) ; printf ("\n");
    GB_code_check (GB_INT32_code,  &i32, 5, stdout) ; printf ("\n");
    GB_code_check (GB_UINT32_code, &u32, 5, stdout) ; printf ("\n");
    GB_code_check (GB_INT64_code,  &i64, 5, stdout) ; printf ("\n");
    GB_code_check (GB_UINT64_code, &u64, 5, stdout) ; printf ("\n");
    GB_code_check (GB_FP32_code,   &f32, 5, stdout) ; printf ("\n");
    GB_code_check (GB_FP64_code,   &f64, 5, stdout) ; printf ("\n");
    GB_code_check (GB_UDT_code,    &f64, 5, stdout) ; printf ("\n");

    for (int i = 0 ; i <= GrB_PANIC + 1 ; i++)
    {
        printf ("info: %2d %s\n", i, GB_status_code (i)) ;
    }

    //--------------------------------------------------------------------------
    // global get/set
    //--------------------------------------------------------------------------

    double h, bswitch [GxB_NBITMAP_SWITCH] ;
    GxB_Format_Value ff ;
    GxB_Global_Option_get_(GxB_HYPER_SWITCH, &h) ;
    GxB_Global_Option_get_(GxB_BITMAP_SWITCH, bswitch) ;
    GxB_Global_Option_get_(GxB_FORMAT, &ff) ;
    printf ("hyper_switch %g csc %d\n", h, (ff == GxB_BY_COL)) ;
    for (int k = 0 ; k < GxB_NBITMAP_SWITCH ; k++)
    {
        printf ("bitmap_switch [%d]: %g\n", k, bswitch [k]) ;
    }

    GrB_Mode mode ;
    GxB_Global_Option_get_(GxB_MODE, &mode) ;
    printf ("mode: %d\n", mode) ;

    int nthreads ;
    GxB_Global_Option_get_(GxB_NTHREADS, &nthreads) ;
    printf ("# threads: %d\n", nthreads) ;

    double chunk ;
    GxB_Global_Option_get_(GxB_CHUNK, &chunk) ;
    printf ("chunk: %g\n", chunk) ;

    //--------------------------------------------------------------------------
    // check A and B aliased
    //--------------------------------------------------------------------------

    GrB_Matrix A = NULL, B = NULL, C = NULL ;
    OK (GrB_Matrix_new (&A, GrB_BOOL, 10000, 10000)) ;
    OK (GrB_Matrix_new (&B, GrB_BOOL, 10000, 10000)) ;
    OK (GrB_Matrix_setElement_BOOL (A, true, 0, 0)) ;
    OK (GrB_Matrix_setElement_BOOL (B, true, 0, 0)) ;
    OK (GrB_Matrix_wait_(&A)) ;
    OK (GrB_Matrix_wait_(&B)) ;
    CHECK (!GB_aliased (A, B)) ;
    int64_t *Bh_save = B->h ;
    B->h = A->h ; B->h_shallow = true ;
    CHECK (GB_aliased (A, B)) ;
    B->h = Bh_save ; B->h_shallow = false ;
    CHECK (!GB_aliased (A, B)) ;
    OK (GxB_Matrix_fprint_(A, 3, NULL)) ;
    OK (GxB_Matrix_fprint_(B, 3, NULL)) ;
    GrB_Matrix_free_(&A) ;
    GrB_Matrix_free_(&B) ;

    //--------------------------------------------------------------------------
    // check descripter set/get for nthreads and chunk
    //--------------------------------------------------------------------------

    GrB_Descriptor desc ;
    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_Desc_set (desc, GxB_NTHREADS, 42)) ;
    OK (GxB_Desc_set (desc, GxB_CHUNK, (double) 12345)) ;
    OK (GxB_Desc_get (desc, GxB_CHUNK, &chunk)) ;
    OK (GxB_Desc_get (desc, GxB_NTHREADS, &nthreads)) ;
    OK (GrB_Descriptor_wait_(&desc)) ;
    OK (GxB_Descriptor_fprint_(desc, GxB_COMPLETE, NULL)) ;
    CHECK (chunk == 12345) ;
    CHECK (nthreads == 42) ;
    GrB_Descriptor_free_(&desc) ;

    //--------------------------------------------------------------------------
    // make a shallow copy of an empty matrix
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_BOOL, 10000, 10000)) ;
    OK (GB_shallow_copy (&C, A->is_csc, A, NULL)) ;
    OK (GxB_Matrix_fprint_(C, GxB_COMPLETE, NULL)) ;
    GrB_Matrix_free_(&A) ;
    GrB_Matrix_free_(&C) ;

    //--------------------------------------------------------------------------
    // tests with memory tracking off
    //--------------------------------------------------------------------------

    GB_Global_malloc_tracking_set (false) ;
    GB_void *p = GB_malloc_memory (4, sizeof (int64_t)) ;
    CHECK (p != NULL) ;
    GB_FREE (p) ;
    p = GB_calloc_memory (4, sizeof (int64_t)) ;
    CHECK (p != NULL) ;
    bool ok = true ;
    p = GB_realloc_memory (6, 4, sizeof (int64_t), p, &ok) ;
    CHECK (p != NULL) ;
    CHECK (ok) ;
    GB_FREE (p) ;

    CHECK (!GB_Global_malloc_is_thread_safe_get ( )) ;
    GB_Global_malloc_is_thread_safe_set (true) ;
    CHECK (GB_Global_malloc_is_thread_safe_get ( )) ;
    GB_Global_malloc_is_thread_safe_set (false) ;
    CHECK (!GB_Global_malloc_is_thread_safe_get ( )) ;

    GB_Global_malloc_tracking_set (true) ;

    //--------------------------------------------------------------------------
    // other global settings
    //--------------------------------------------------------------------------

    GB_Global_hack_set (90123) ;
    CHECK (GB_Global_hack_get ( ) == 90123) ;

    GrB_Info expected = GrB_INVALID_VALUE ;

    //--------------------------------------------------------------------------
    // GB_pslice
    //--------------------------------------------------------------------------

    int64_t *Slice = NULL ;
    GB_pslice (&Slice, NULL, 0, 4, true) ;
    for (int t = 0 ; t < 4 ; t++) CHECK (Slice [t] == 0) ;
    GB_FREE (Slice) ;

    //--------------------------------------------------------------------------
    // renamed boolean monoids
    //--------------------------------------------------------------------------

    GrB_Monoid mono = NULL ;

    // DIV renamed to FIRST
    OK (GrB_Monoid_new_BOOL_(&mono, GrB_DIV_BOOL, (bool) false)) ;
    printf ("\ndiv_bool monoid:\n") ;
    OK (GxB_Monoid_fprint_(mono, GxB_COMPLETE, NULL)) ;
    GrB_Monoid_free_(&mono) ;

    // RDIV renamed to SECOND
    OK (GrB_Monoid_new_BOOL_(&mono, GxB_RDIV_BOOL, (bool) false)) ;
    printf ("\nrdiv_bool monoid:\n") ;
    OK (GxB_Monoid_fprint_(mono, GxB_COMPLETE, NULL)) ;
    GrB_Monoid_free_(&mono) ;

    // ISGT renamed to GT
    OK (GrB_Monoid_new_BOOL_(&mono, GxB_ISGT_BOOL, (bool) false)) ;
    printf ("\nisgt_bool monoid:\n") ;
    OK (GxB_Monoid_fprint_(mono, GxB_COMPLETE, NULL)) ;
    GrB_Monoid_free_(&mono) ;

    // ISLT renamed to LT
    OK (GrB_Monoid_new_BOOL_(&mono, GxB_ISLT_BOOL, (bool) false)) ;
    printf ("\nislt_bool monoid:\n") ;
    OK (GxB_Monoid_fprint_(mono, GxB_COMPLETE, NULL)) ;
    GrB_Monoid_free_(&mono) ;

    // ISGE renamed to GE
    OK (GrB_Monoid_new_BOOL_(&mono, GxB_ISGE_BOOL, (bool) false)) ;
    printf ("\nisge_bool monoid:\n") ;
    OK (GxB_Monoid_fprint_(mono, GxB_COMPLETE, NULL)) ;
    GrB_Monoid_free_(&mono) ;

    // ISLE renamed to LE
    OK (GrB_Monoid_new_BOOL_(&mono, GxB_ISLE_BOOL, (bool) false)) ;
    printf ("\nisle_bool monoid:\n") ;
    OK (GxB_Monoid_fprint_(mono, GxB_COMPLETE, NULL)) ;
    GrB_Monoid_free_(&mono) ;

    //--------------------------------------------------------------------------
    // select
    //--------------------------------------------------------------------------

    GrB_Type user_type = NULL ;
    OK (GrB_Type_new (&user_type, sizeof (user_int))) ;
    OK (GrB_Type_wait_(&user_type)) ;
    OK (GrB_Matrix_new (&A, user_type, 10, 10)) ;
    OK (GrB_Matrix_new (&B, GrB_INT16, 10, 10)) ;
    user_int value ;
    for (int i = 0 ; i < 10 ; i++)
    {
        value = (int64_t) i ;
        OK (GrB_Matrix_setElement_UDT (A, &value, i, i)) ;
        OK (GrB_Matrix_setElement_INT16 (B, i, i, i)) ;
    }
    OK (GrB_Matrix_wait_(&A)) ;
    OK (GrB_Matrix_wait_(&B)) ;
    OK (GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;

    GxB_Scalar thunk = NULL ;
    OK (GxB_Scalar_new (&thunk, user_type)) ;
    GrB_Type type2 = NULL ;
    OK (GxB_Scalar_type (&type2, thunk)) ;
    CHECK (type2 == user_type) ;
    OK (GxB_Scalar_fprint_(thunk, GxB_COMPLETE, NULL)) ;
    OK (GxB_Matrix_select_(A, NULL, NULL, GxB_NE_THUNK, A, thunk, NULL)) ;

    value = (int64_t) 4 ;
    OK (GxB_Scalar_setElement_UDT (thunk, &value)) ;

    expected = GrB_DOMAIN_MISMATCH ;
    ERR1 (A, GxB_Matrix_select_(A, NULL, NULL, GxB_GE_THUNK, A, thunk, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("Expected error: info: %d\n%s\n", info, err) ;

    GxB_Scalar thunk2 = NULL ;
    OK (GxB_Scalar_new (&thunk2, GrB_INT16)) ;
    OK (GxB_Scalar_setElement_INT16 (thunk2, 4)) ;
    OK (GxB_Scalar_wait_(&thunk2)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR1 (A, GxB_Matrix_select_(A, NULL, NULL, GxB_GE_ZERO, A, NULL, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("Expected error: info: %d\n%s\n", info, err) ;

    ERR1 (A, GxB_Matrix_select_(A, NULL, NULL, GxB_GT_ZERO, A, NULL, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("Expected error: info: %d\n%s\n", info, err) ;

    ERR1 (A, GxB_Matrix_select_(A, NULL, NULL, GxB_LT_ZERO, A, NULL, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("Expected error: info: %d\n%s\n", info, err) ;

    ERR1 (A, GxB_Matrix_select_(A, NULL, NULL, GxB_LE_ZERO, A, NULL, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("Expected error: info: %d\n%s\n", info, err) ;

    ERR1 (B, GxB_Matrix_select_(B, NULL, NULL, GxB_LE_THUNK, B, thunk, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("Expected error: info: %d\n%s\n", info, err) ;
    GrB_Matrix_free_(&B) ;

    OK (GrB_Matrix_new (&B, user_type, 10, 10)) ;
    printf ("\n============== B = select (A != 0)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_NONZERO, A, NULL, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;
    printf ("\n============== B = select (A == 0)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_EQ_ZERO, A, NULL, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;
    printf ("\n============== B = select (A != 4)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_NE_THUNK, A, thunk, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;
    printf ("\n============== B = select (A == 4)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_EQ_THUNK, A, thunk, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;

    GrB_Matrix_free_(&B) ;
    GrB_Matrix_free_(&A) ;
    GxB_Scalar_free_(&thunk) ;
    GxB_Scalar_free_(&thunk2) ;
    GrB_Type_free_(&user_type) ;

    OK (GrB_Matrix_new (&A, GrB_BOOL, 10, 10)) ;
    OK (GrB_Matrix_new (&B, GrB_BOOL, 10, 10)) ;
    OK (GxB_Scalar_new (&thunk, GrB_BOOL)) ;
    OK (GxB_Scalar_setElement_BOOL (thunk, 0)) ;
    for (int i = 0 ; i < 10 ; i++)
    {
        OK (GrB_Matrix_setElement_BOOL_(A, (bool) (i % 2), i, i)) ;
    }
    OK (GrB_Matrix_wait_(&A)) ;
    OK (GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL)) ;

    printf ("\n============== B = select (A > 0)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_GT_THUNK, A, thunk, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;
    printf ("\n============== B = select (A >= 0)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_GE_THUNK, A, thunk, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;
    printf ("\n============== B = select (A < 0)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_LT_THUNK, A, thunk, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;
    printf ("\n============== B = select (A <= 0)\n") ;
    OK (GxB_Matrix_select_(B, NULL, NULL, GxB_LE_THUNK, A, thunk, NULL)) ;
    OK (GxB_Matrix_fprint_(B, GxB_COMPLETE, NULL)) ;

    GrB_Matrix_free_(&B) ;
    GrB_Matrix_free_(&A) ;
    GxB_Scalar_free_(&thunk) ;

    //--------------------------------------------------------------------------
    // create a test matrix
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_FP64, 8, 8)) ;
    for (int i = 0 ; i < 8 ; i++)
    {
        for (int j = 0 ; j < 8 ; j++)
        {
            OK (GrB_Matrix_setElement_FP64 (A, i*100+j, i, j)) ;
        }
    }
    OK (GrB_Matrix_wait_(&A)) ;

    GrB_Vector_new (&victor, GrB_FP64, 43) ;
    OK (GrB_Vector_setElement_FP64 (victor, 99, 0)) ;
    OK (GrB_Vector_wait_(&victor)) ;

    //--------------------------------------------------------------------------
    // GxB_get
    //--------------------------------------------------------------------------

    int sparsity ;
    OK (GxB_Matrix_Option_get_(A, GxB_SPARSITY_CONTROL, &sparsity)) ;
    CHECK (sparsity == GxB_AUTO_SPARSITY) ;
    OK (GxB_Vector_Option_get_(victor, GxB_SPARSITY_CONTROL, &sparsity)) ;
    CHECK (sparsity == GxB_AUTO_SPARSITY) ;
    OK (GxB_Vector_Option_get_(victor, GxB_SPARSITY_STATUS, &sparsity)) ;
    CHECK (sparsity == GxB_SPARSE) ;
    GxB_Format_Value fmt ;
    OK (GxB_Vector_Option_get_(victor, GxB_FORMAT, &fmt)) ;
    CHECK (fmt == GxB_BY_COL) ;
    bool is_hyper ;
    OK (GxB_Vector_Option_get_(victor, GxB_IS_HYPER, &is_hyper)) ;
    CHECK (!is_hyper) ;
    expected = GrB_INVALID_VALUE ;
    ERR (GxB_Vector_Option_get_(victor, -999, &is_hyper)) ;

    //--------------------------------------------------------------------------
    // GxB_set
    //--------------------------------------------------------------------------

    ERR (GxB_Vector_Option_set_(victor, -999, &is_hyper)) ;
    OK (GxB_Vector_Option_set_(victor, GxB_SPARSITY_CONTROL, 9999)) ;
    OK (GxB_Vector_Option_get_(victor, GxB_SPARSITY_CONTROL, &sparsity)) ;
    CHECK (sparsity == GxB_AUTO_SPARSITY) ;

    //--------------------------------------------------------------------------
    // removeElement errors
    //--------------------------------------------------------------------------

    expected = GrB_INVALID_INDEX ;
    ERR1 (victor, GrB_Vector_removeElement (victor, 9999)) ;
    GrB_Vector_error_(&err, victor) ;
    printf ("expected error: %s\n", err) ;
    ERR1 (A, GrB_Matrix_removeElement (A, 0, 9999)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("expected error: %s\n", err) ;
    ERR1 (A, GrB_Matrix_removeElement (A, 9999, 0)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("expected error: %s\n", err) ;

    //--------------------------------------------------------------------------
    // pending tuples
    //--------------------------------------------------------------------------

    GrB_Matrix_free_(&A) ;
    OK (GrB_Matrix_new (&A, GrB_FP64, 8, 8)) ;

    GrB_Index I [1] = { 0 }, J [1] = { 0 } ;
    OK (GrB_Matrix_assign_FP64_(A, NULL, GrB_PLUS_FP64,
        (double) 2, I, 1, J, 1, NULL)) ;
    GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL) ;
    OK (GrB_Matrix_setElement_FP64_(A, (double) 3, 0, 0)) ;
    GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL) ;
    OK (GrB_Matrix_wait_(&A)) ;
    GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL) ;

    GrB_Matrix_free_(&A) ;
    GrB_Vector_free_(&victor) ;

    //--------------------------------------------------------------------------
    // remvoe element with empty matrix and vector
    //--------------------------------------------------------------------------

    printf ("testing removeElement\n") ;
    OK (GrB_Vector_new (&victor, GrB_FP64, 4)) ;
    OK (GrB_Matrix_new (&A, GrB_FP64, 4, 4)) ;
    OK (GrB_Vector_removeElement (victor, 0)) ;
    OK (GrB_Matrix_removeElement (A, 0, 0)) ;
    GrB_Matrix_free_(&A) ;
    GrB_Vector_free_(&victor) ;
    printf ("removeElement: OK\n") ;

    //--------------------------------------------------------------------------
    // select error handling
    //--------------------------------------------------------------------------

    GxB_SelectOp selectop = NULL ;
    OK (GxB_SelectOp_new (&selectop, select_plus_one, GrB_FP64, GrB_FP64)) ;
    OK (GxB_SelectOp_wait_(&selectop)) ;
    OK (GrB_Matrix_new (&A, GrB_FP64, 8, 8)) ;
    OK (GrB_Matrix_new (&C, GrB_FP64, 8, 8)) ;
    for (int i = 0 ; i < 8 ; i++)
    {
        OK (GrB_Matrix_setElement_FP64 (A, i, i, i)) ;
    }
    OK (GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL)) ;
    OK (GxB_Scalar_new (&thunk, GrB_FP64)) ;
    OK (GxB_Scalar_setElement_FP64 (thunk, 4)) ;
    OK (GxB_Matrix_select_(C, NULL, NULL, selectop, A, thunk, NULL)) ;

    printf ("\nprint in one-based, long format:\n") ;
    GB_Global_print_one_based_set (true) ;
    OK (GxB_Matrix_fprint_(C, GxB_COMPLETE_VERBOSE, NULL)) ;
    GB_Global_print_one_based_set (false) ;

    expected = GrB_NULL_POINTER ;
    ERR1 (C, GxB_Matrix_select_(C, NULL, NULL, selectop, A, NULL, NULL)) ;
    GrB_Matrix_error_(&err, C) ;
    printf ("Error expected: %d\n%s\n", info, err) ;

    expected = GrB_INVALID_VALUE ;
    OK (GxB_Scalar_clear (thunk)) ;
    ERR1 (C, GxB_Matrix_select_(C, NULL, NULL, selectop, A, thunk, NULL)) ;
    GrB_Matrix_error_(&err, C) ;
    printf ("Error expected: %d\n%s\n", info, err) ;

    expected = GrB_DOMAIN_MISMATCH ;
    GxB_Scalar_free_(&thunk) ;
    OK (GxB_Scalar_new (&thunk, GrB_FP32)) ;
    ERR1 (C, GxB_Matrix_select_(C, NULL, NULL, selectop, A, thunk, NULL)) ;
    GrB_Matrix_error_(&err, C) ;
    printf ("Error expected: %d\n%s\n", info, err) ;

    GxB_SelectOp_free_(&selectop) ;
    OK (GxB_SelectOp_new (&selectop, select_nothing, GrB_FP64, NULL)) ;
    ERR1 (C, GxB_Matrix_select_(C, NULL, NULL, selectop, A, thunk, NULL)) ;
    GrB_Matrix_error_(&err, C) ;
    printf ("Error expected: %d\n%s\n", info, err) ;

    expected = GrB_UNINITIALIZED_OBJECT ;
    OK (GrB_Type_new (&user_type, sizeof (user_int))) ;
    user_type->magic = 0xDEAD ;
    ERR (GxB_Type_fprint_(user_type, GxB_COMPLETE, NULL)) ;
    expected = GrB_INVALID_OBJECT ;
    selectop->ttype = user_type ;
    ERR (GxB_SelectOp_fprint_(selectop, GxB_COMPLETE, NULL)) ;
    user_type->magic = GB_MAGIC ;

    expected = GrB_UNINITIALIZED_OBJECT ;
    thunk->magic = 0xDEAD ;
    ERR (GxB_Scalar_fprint_(thunk, GxB_COMPLETE, NULL)) ;
    thunk->magic = GB_MAGIC ;
    printf ("Error expected: %d\n", info) ;

    GrB_Matrix_free_(&A) ;
    GrB_Matrix_free_(&C) ;
    GxB_Scalar_free_(&thunk) ;
    GxB_SelectOp_free_(&selectop) ;

    //--------------------------------------------------------------------------
    // GxB_Scalar
    //--------------------------------------------------------------------------

    GrB_Index nvals = 42 ;
    GxB_Scalar scalar = NULL, scalar2 = NULL ;
    OK (GxB_Scalar_new (&scalar, GrB_FP64)) ;
    OK (GxB_Scalar_nvals (&nvals, scalar)) ;
    OK (GxB_Scalar_wait_(&scalar)) ;
    CHECK (nvals == 0) ;

    bool     b_8 = 0 ;
    int8_t   i_8 = 0 ;
    int16_t  i_16 = 0 ;
    int32_t  i_32 = 0 ;
    int64_t  i_64 = 0 ;
    uint8_t  u_8 = 0 ;
    uint16_t u_16 = 0 ;
    uint32_t u_32 = 0 ;
    uint64_t u_64 = 0 ;
    float    x_32 = 0 ;
    double   x_64 = 0 ;

    OK (GxB_Scalar_setElement_FP64_(scalar, (double) 1.25)) ;
    OK (GxB_Scalar_nvals (&nvals, scalar)) ;
    OK (GxB_Scalar_wait_(&scalar)) ;
    CHECK (nvals == 1) ;

    OK (GxB_Scalar_dup (&scalar2, scalar)) ;
    OK (GxB_Scalar_fprint_(scalar2, GxB_COMPLETE, NULL)) ;

    OK (GxB_Scalar_extractElement_BOOL_(&b_8,  scalar)) ; CHECK (b_8 == 1) ;

    OK (GxB_Scalar_extractElement_INT8_ (&i_8,  scalar)) ; CHECK (i_8  == 1) ;
    OK (GxB_Scalar_extractElement_INT16_(&i_16, scalar)) ; CHECK (i_16 == 1) ;
    OK (GxB_Scalar_extractElement_INT32_(&i_32, scalar)) ; CHECK (i_32 == 1) ;
    OK (GxB_Scalar_extractElement_INT64_(&i_64, scalar)) ; CHECK (i_64 == 1) ;

    OK (GxB_Scalar_extractElement_UINT8_ (&u_8,  scalar)) ; CHECK (u_8  == 1) ;
    OK (GxB_Scalar_extractElement_UINT16_(&u_16, scalar)) ; CHECK (u_16 == 1) ;
    OK (GxB_Scalar_extractElement_UINT32_(&u_32, scalar)) ; CHECK (u_32 == 1) ;
    OK (GxB_Scalar_extractElement_UINT64_(&u_64, scalar)) ; CHECK (u_64 == 1) ;

    OK (GxB_Scalar_extractElement_FP32_(&x_32, scalar)) ; CHECK (x_32 == 1.25) ;
    OK (GxB_Scalar_extractElement_FP64_(&x_64, scalar)) ; CHECK (x_64 == 1.25) ;

    OK (GxB_Scalar_clear (scalar)) ;
    info = GxB_Scalar_extractElement_FP64_(&x_64, scalar) ;
    CHECK (info == GrB_NO_VALUE) ;
    CHECK (x_64 == 1.25) ;

    info = GrB_Matrix_extractElement_FP64_(&x_64, (GrB_Matrix) scalar, 0, 0) ;
    CHECK (info == GrB_NO_VALUE) ;
    CHECK (x_64 == 1.25) ;

    info = GrB_Vector_extractElement_FP64_(&x_64, (GrB_Vector) scalar, 0) ;
    CHECK (info == GrB_NO_VALUE) ;
    CHECK (x_64 == 1.25) ;

    u_64 = 0 ;
    OK (GxB_Scalar_extractElement_UINT64_(&u_64, scalar2)) ; CHECK (u_64 == 1) ;
    OK (GxB_Scalar_nvals (&nvals, scalar2)) ;
    OK (GxB_Scalar_wait_(&scalar2)) ;
    CHECK (nvals == 1) ;

    expected = GrB_INVALID_OBJECT ;
    scalar2->vlen = 2 ;
    ERR (GxB_Scalar_fprint_(scalar2, GxB_COMPLETE, NULL)) ;
    scalar2->vlen = 1 ;
    OK (GxB_Scalar_fprint_(scalar2, GxB_COMPLETE, NULL)) ;

    GxB_Scalar_free_(&scalar) ;
    GxB_Scalar_free_(&scalar2) ;

    //--------------------------------------------------------------------------
    // predefined descriptors
    //--------------------------------------------------------------------------

    OK (GxB_Descriptor_fprint (GrB_DESC_T1     , "T1    ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_T0     , "T0    ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_T0T1   , "T0T1  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_C      , "C     ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_CT1    , "CT1   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_CT0    , "CT0   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_CT0T1  , "CT0T1 ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_S      , "S     ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_ST1    , "ST1   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_ST0    , "ST0   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_ST0T1  , "ST0T1 ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_SC     , "SC    ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_SCT1   , "SCT1  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_SCT0   , "SCT0  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_SCT0T1 , "SCT0T1", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_R      , "R     ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RT1    , "RT1   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RT0    , "RT0   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RT0T1  , "RT0T1 ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RC     , "RC    ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RCT1   , "RCT1  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RCT0   , "RCT0  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RCT0T1 , "RCT0T1", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RS     , "RS    ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RST1   , "RST1  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RST0   , "RST0  ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RST0T1 , "RST0T1", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RSC    , "RSC   ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RSCT1  , "RSCT1 ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RSCT0  , "RSCT0 ", GxB_COMPLETE, NULL));
    OK (GxB_Descriptor_fprint (GrB_DESC_RSCT0T1, "RSCT0T1",GxB_COMPLETE, NULL));

    GrB_Descriptor_new (&Duh) ;
    OK (GxB_Desc_set (Duh, GxB_AxB_METHOD, GxB_AxB_SAXPY)) ;
    OK (GxB_Descriptor_fprint_(Duh, GxB_COMPLETE, NULL)) ;
    OK (GxB_Desc_set (Duh, GxB_AxB_METHOD, GxB_AxB_HASH)) ;
    OK (GxB_Descriptor_fprint_(Duh, GxB_COMPLETE, NULL)) ;
    OK (GxB_Desc_set (Duh, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
    OK (GxB_Descriptor_fprint_(Duh, GxB_COMPLETE, NULL)) ;
    OK (GxB_Desc_set (Duh, GxB_AxB_METHOD, GxB_AxB_DOT)) ;
    OK (GxB_Descriptor_fprint_(Duh, GxB_COMPLETE, NULL)) ;
    GrB_Descriptor_free_(&Duh) ;

    expected = GrB_INVALID_VALUE ;
    ERR (GxB_Desc_set (GrB_DESC_S, GrB_INP0, GrB_TRAN)) ;

    ERR (GrB_Descriptor_set (GrB_DESC_S, GrB_INP0, GrB_TRAN)) ;

    //--------------------------------------------------------------------------
    // burble
    //--------------------------------------------------------------------------

    bool burble ;
    OK (GxB_Global_Option_get_(GxB_BURBLE, &burble)) ;
    printf ("burble: %d\n", burble) ;

    //--------------------------------------------------------------------------
    // select ops
    //--------------------------------------------------------------------------

    OK (GxB_SelectOp_fprint (GxB_TRIL,     "tril"    , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_TRIU,     "triu"    , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_DIAG,     "diag"    , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_OFFDIAG,  "offidiag", GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_NONZERO,  "nonzero" , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_EQ_ZERO,  "eq_zero" , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_GT_ZERO,  "gt_zero" , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_GE_ZERO,  "ge_zero" , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_LT_ZERO,  "lt_zero" , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_LE_ZERO,  "le_zero" , GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_NE_THUNK, "ne_thunk", GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_EQ_THUNK, "eq_thunk", GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_GT_THUNK, "gt_thunk", GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_GE_THUNK, "ge_thunk", GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_LT_THUNK, "lt_thunk", GxB_COMPLETE, NULL)) ;
    OK (GxB_SelectOp_fprint (GxB_LE_THUNK, "le_thunk", GxB_COMPLETE, NULL)) ;

    //--------------------------------------------------------------------------
    // assign scalar into hypersparse
    //--------------------------------------------------------------------------

    GrB_Index n = INT32_MAX ;
    n = n * 1024 ;
    OK (GrB_Matrix_new (&A, GrB_FP64, n, n)) ;
    expected = GrB_OUT_OF_MEMORY ;
    ERR1 (A, GrB_Matrix_assign_FP64_(A, NULL, NULL, (double) 1,
        GrB_ALL, n, GrB_ALL, n, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    printf ("\nproblem too large, expected error: %s\n", err) ;
    OK (GrB_Matrix_free_(&A)) ;

    //--------------------------------------------------------------------------
    // setElement typecast
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, user_type, 10, 10)) ;

    expected = GrB_DOMAIN_MISMATCH ;

    ERR1 (A, GrB_Matrix_setElement_BOOL   (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_INT8   (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_INT16  (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_INT32  (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_INT64  (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_UINT8  (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_UINT16 (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_UINT32 (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_UINT64 (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_FP32   (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GrB_Matrix_setElement_FP64   (A, 0, 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GxB_Matrix_setElement_FC32   (A, GxB_CMPLXF(0,0), 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;
    ERR1 (A, GxB_Matrix_setElement_FC64   (A, GxB_CMPLX (0,0), 0, 0)) ;
    GrB_Matrix_error_(&err, A) ; printf ("expected: %s\n", err) ;

    //--------------------------------------------------------------------------
    // GrB_error
    //--------------------------------------------------------------------------

    GrB_Type_error_(&err, user_type) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GrB_UnaryOp_error_(&err, GrB_AINV_FP32) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GrB_BinaryOp_error_(&err, GrB_PLUS_FP32) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GxB_SelectOp_error_(&err, GxB_TRIL) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GrB_Monoid_error_(&err, GrB_LOR_MONOID_BOOL) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GrB_Semiring_error_(&err, GrB_PLUS_TIMES_SEMIRING_FP32) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GrB_Descriptor_error_(&err, GrB_DESC_T0) ;
    CHECK (err != NULL && err [0] == '\0') ;

    OK (GxB_Scalar_new (&scalar, GrB_FP32)) ;
    GxB_Scalar_error_(&err, scalar) ;
    CHECK (err != NULL && err [0] == '\0') ;

    OK (GrB_Vector_new (&victor, GrB_FP32, 10)) ;
    GrB_Vector_error_(&err, victor) ;
    CHECK (err != NULL && err [0] == '\0') ;

    int16_t fortytwo = 42 ;
    OK (GrB_Matrix_setElement_UDT (A, (void *) &fortytwo, 3, 7)) ;
    OK (GxB_Matrix_fprint_(A, GxB_COMPLETE, NULL)) ;
    GrB_Matrix_error_(&err, A) ;
    CHECK (err != NULL && err [0] == '\0') ;

    GrB_Vector_free_(&victor) ;
    GxB_Scalar_free_(&scalar) ;
    GrB_Type_free_(&user_type) ;
    GrB_Matrix_free_(&A) ;

    //--------------------------------------------------------------------------
    // test for problem too large in GB_bitmap_AxB_saxpy
    //--------------------------------------------------------------------------

    n = INT32_MAX ;
    OK (GrB_Matrix_new (&A, GrB_FP32, n, 0)) ;
    OK (GrB_Matrix_new (&B, GrB_FP32, 0, n)) ;
    expected = GrB_OUT_OF_MEMORY ;
    bool ignore ;
    ERR (GB_bitmap_AxB_saxpy (&C, GxB_BITMAP, NULL, false, false, A, B,
        GrB_PLUS_TIMES_SEMIRING_FP32, false, &ignore, NULL)) ;
    GrB_Matrix_free_(&A) ;
    GrB_Matrix_free_(&B) ;
    CHECK (C == NULL) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    // #include "GB_Test_about_mkl_template.c"
    GB_mx_put_global (true) ;   
    fclose (f) ;
    printf ("\nAll errors printed above were expected.\n") ;
    printf ("GB_mex_about: all tests passed\n\n") ;
}

