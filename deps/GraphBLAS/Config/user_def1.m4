//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Config/user_def1.m4: define user-defined objects
//------------------------------------------------------------------------------

m4_define(`GxB_Type_define', `
    #define GB_DEF_$1_type $2
    struct GB_Type_opaque GB_opaque_$1 =
    {
        GB_MAGIC,           // object is defined
        sizeof ($2),        // size of the type
        GB_UCT_code,        // user-defined at compile-time
        "$2"
    } ;
    GrB_Type $1 = & GB_opaque_$1')

m4_define(`GxB_UnaryOp_define', `
    #define GB_DEF_$1_function $2
    #define GB_DEF_$1_ztype GB_DEF_$3_type
    #define GB_DEF_$1_xtype GB_DEF_$4_type
    extern void $2
    (
        GB_DEF_$1_ztype *z,
        const GB_DEF_$1_xtype *x
    ) ;
    struct GB_UnaryOp_opaque GB_opaque_$1 =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_$4,     // type of x
        & GB_opaque_$3,     // type of z
        $2,                 // pointer to the C function
        "$2",
        GB_USER_C_opcode    // user-defined at compile-time
    } ;
    GrB_UnaryOp $1 = & GB_opaque_$1')

m4_define(`GxB_BinaryOp_define', `
    #define GB_DEF_$1_function $2
    #define GB_DEF_$1_ztype GB_DEF_$3_type
    #define GB_DEF_$1_xtype GB_DEF_$4_type
    #define GB_DEF_$1_ytype GB_DEF_$5_type
    extern void $2
    (
        GB_DEF_$1_ztype *z,
        const GB_DEF_$1_xtype *x,
        const GB_DEF_$1_ytype *y
    ) ;
    struct GB_BinaryOp_opaque GB_opaque_$1 =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_$4,     // type of x
        & GB_opaque_$5,     // type of y
        & GB_opaque_$3,     // type of z
        $2,                 // pointer to the C function
        "$2",
        GB_USER_C_opcode    // user-defined at compile-time
    } ;
    GrB_BinaryOp $1 = & GB_opaque_$1')

m4_define(`GxB_Monoid_define', `
    #define GB_DEF_$1_add GB_DEF_$2_function
    GB_DEF_$2_ztype GB_DEF_$1_identity = $3 ;
    struct GB_Monoid_opaque GB_opaque_$1 =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_$2,     // binary operator
        & GB_DEF_$1_identity,   // identity value
        sizeof (GB_DEF_$2_ztype),   // identity size
        GB_USER_COMPILED    // user-defined at compile-time
    } ;
    GrB_Monoid $1 = & GB_opaque_$1')

m4_define(`GB_semirings', `if (0)
    {
        ;
    }')

m4_define(`GB_semiring', `m4_define(`GB_semirings', GB_semirings()
    else if (GB_s == $1)
    {
        if (GB_AxB_method == GxB_AxB_GUSTAVSON)
        { 
            GB_info = GB_AxB_user_gus_$1
                (*GB_Chandle, GB_M, GB_A, GB_B, GB_flipxy, GB_C_Sauna,
                Context) ;
        }
        else if (GB_AxB_method == GxB_AxB_DOT)
        { 
            GB_info = GB_AxB_user_dot_$1
                (GB_Chandle, GB_M, GB_A, GB_B, GB_flipxy, Context) ;
        }
        else // (GB_AxB_method == GxB_AxB_HEAP)
        { 
            GB_info = GB_AxB_user_heap_$1
                (GB_Chandle, GB_M, GB_A, GB_B, GB_flipxy,
                GB_List, GB_pA_pair, GB_Heap, GB_bjnz_max, Context) ;
        }
    } ) $2')

m4_define(`GxB_Semiring_define', `GB_semiring($1,`
    #define GB_AgusB    GB_AxB_user_gus_$1
    #define GB_AdotB    GB_AxB_user_dot_$1
    #define GB_AheapB   GB_AxB_user_heap_$1
    #define GB_identity    GB_DEF_$2_identity
    #define GB_ADD(z,y)    GB_DEF_$2_add (&(z), &(z), &(y))
    #define GB_MULT(z,x,y) GB_DEF_$3_function (&(z), &(x), &(y))
    #define GB_ztype       GB_DEF_$3_ztype
    #define GB_xtype       GB_DEF_$3_xtype
    #define GB_ytype       GB_DEF_$3_ytype
    #define GB_handle_flipxy 1
    #undef GBCOMPACT
    #include "GB_AxB.c"
    #undef GB_identity
    #undef GB_ADD
    #undef GB_xtype
    #undef GB_ytype
    #undef GB_ztype
    #undef GB_MULT
    #undef GB_AgusB
    #undef GB_AdotB
    #undef GB_AheapB
    struct GB_Semiring_opaque GB_opaque_$1 =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_$2,     // add monoid
        & GB_opaque_$3,     // multiply operator
        GB_USER_COMPILED    // user-defined at compile-time
    } ;
    GrB_Semiring $1 = & GB_opaque_$1')')

m4_define(`GxB_SelectOp_define', `
    #define GB_DEF_$1_function $2
    extern bool $2
    (
        GrB_Index i,
        GrB_Index j,
        GrB_Index nrows,
        GrB_Index ncols,
        const m4_ifelse(`$3', `NULL', `void', `GB_DEF_$3_type') *x,
        const void *thunk
    ) ;
    struct GB_SelectOp_opaque GB_opaque_$1 =
    {
        GB_MAGIC,           // object is defined
        m4_ifelse(`$3', `NULL',
            `NULL,  // x not used',
            `& GB_opaque_$3 // type of x')
        $2,                 // pointer to the C function
        "$2",
        GB_USER_SELECT_C_opcode // user-defined at compile-time
    } ;
    GxB_SelectOp $1 = & GB_opaque_$1')
