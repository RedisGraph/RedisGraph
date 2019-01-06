//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Source/all_user_objects.c
//------------------------------------------------------------------------------

// This file is constructed automatically by cmake and m4 when GraphBLAS is
// compiled, from the Config/user_def*.m4 and User/*.m4 files.  Do not edit
// this file directly.  It contains references to internally-defined functions
// and objects inside GraphBLAS, which are not user-callable.

#include "GB.h"

//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Config/user_def1.m4: define user-defined objects
//------------------------------------------------------------------------------
















//------------------------------------------------------------------------------
// GraphBLAS/User/rg_structured_bool.m4: RedisGraph boolean semiring
//------------------------------------------------------------------------------

#ifdef GxB_USER_INCLUDE
// Define a boolean structured semiring which RedisGraph can use
// while compiling GraphBLAS with GBCOMPACT.
#define RG_STRUCTURED_BOOL

    static inline void rg_bplus
    (
        bool *z,
        const bool *x,
        const bool *y
    )
    {
        (void)(x);
        (void)(y);
        (*z) = true ;
    }

    static inline void rg_bmul
    (
        bool *z,
        const bool *x,
        const bool *y
    )
    {
        (void)(x);
        (void)(y);
        (*z) = true ;
    }

#endif

// The two operators, bool add and multiply:
// bplus operator

    #define GB_DEF_Rg_bplus_function rg_bplus
    #define GB_DEF_Rg_bplus_ztype GB_DEF_GrB_BOOL_type
    #define GB_DEF_Rg_bplus_xtype GB_DEF_GrB_BOOL_type
    #define GB_DEF_Rg_bplus_ytype GB_DEF_GrB_BOOL_type
    extern void rg_bplus
    (
        GB_DEF_Rg_bplus_ztype *z,
        const GB_DEF_Rg_bplus_xtype *x,
        const GB_DEF_Rg_bplus_ytype *y
    ) ;
    struct GB_BinaryOp_opaque GB_opaque_Rg_bplus =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_GrB_BOOL,     // type of x
        & GB_opaque_GrB_BOOL,     // type of y
        & GB_opaque_GrB_BOOL,     // type of z
        rg_bplus,                 // pointer to the C function
        "rg_bplus",
        GB_USER_C_opcode    // user-defined at compile-time
    } ;
    GrB_BinaryOp Rg_bplus = & GB_opaque_Rg_bplus ;

// bmul operator

    #define GB_DEF_Rg_bmul_function rg_bmul
    #define GB_DEF_Rg_bmul_ztype GB_DEF_GrB_BOOL_type
    #define GB_DEF_Rg_bmul_xtype GB_DEF_GrB_BOOL_type
    #define GB_DEF_Rg_bmul_ytype GB_DEF_GrB_BOOL_type
    extern void rg_bmul
    (
        GB_DEF_Rg_bmul_ztype *z,
        const GB_DEF_Rg_bmul_xtype *x,
        const GB_DEF_Rg_bmul_ytype *y
    ) ;
    struct GB_BinaryOp_opaque GB_opaque_Rg_bmul =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_GrB_BOOL,     // type of x
        & GB_opaque_GrB_BOOL,     // type of y
        & GB_opaque_GrB_BOOL,     // type of z
        rg_bmul,                 // pointer to the C function
        "rg_bmul",
        GB_USER_C_opcode    // user-defined at compile-time
    } ;
    GrB_BinaryOp Rg_bmul = & GB_opaque_Rg_bmul ;

// The plus monoid:

    #define GB_DEF_Rg_Bool_plus_monoid_add GB_DEF_Rg_bplus_function
    GB_DEF_Rg_bplus_ztype GB_DEF_Rg_Bool_plus_monoid_identity = false ;
    struct GB_Monoid_opaque GB_opaque_Rg_Bool_plus_monoid =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_Rg_bplus,     // binary operator
        & GB_DEF_Rg_Bool_plus_monoid_identity,   // identity value
        sizeof (GB_DEF_Rg_bplus_ztype),   // identity size
        GB_USER_COMPILED    // user-defined at compile-time
    } ;
    GrB_Monoid Rg_Bool_plus_monoid = & GB_opaque_Rg_Bool_plus_monoid ;

// plus-bmul semiring
 
    #define GB_AgusB    GB_AxB_user_gus_Rg_structured_bool
    #define GB_AdotB    GB_AxB_user_dot_Rg_structured_bool
    #define GB_AheapB   GB_AxB_user_heap_Rg_structured_bool
    #define GB_identity    GB_DEF_Rg_Bool_plus_monoid_identity
    #define GB_ADD(z,y)    GB_DEF_Rg_Bool_plus_monoid_add (&(z), &(z), &(y))
    #define GB_MULT(z,x,y) GB_DEF_Rg_bmul_function (&(z), &(x), &(y))
    #define GB_ztype       GB_DEF_Rg_bmul_ztype
    #define GB_xtype       GB_DEF_Rg_bmul_xtype
    #define GB_ytype       GB_DEF_Rg_bmul_ytype
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
    struct GB_Semiring_opaque GB_opaque_Rg_structured_bool =
    {
        GB_MAGIC,           // object is defined
        & GB_opaque_Rg_Bool_plus_monoid,     // add monoid
        & GB_opaque_Rg_bmul,     // multiply operator
        GB_USER_COMPILED    // user-defined at compile-time
    } ;
    GrB_Semiring Rg_structured_bool = & GB_opaque_Rg_structured_bool ;

//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Config/user_def2.m4: code to call user semirings
//------------------------------------------------------------------------------

GrB_Info GB_AxB_user
(
    const GrB_Desc_Value GB_AxB_method,
    const GrB_Semiring GB_s,

    GrB_Matrix *GB_Chandle,
    const GrB_Matrix GB_M,
    const GrB_Matrix GB_A,
    const GrB_Matrix GB_B,
    bool GB_flipxy,                // if true, A and B have been swapped

    // for heap method only:
    int64_t *restrict GB_List,
    GB_pointer_pair *restrict GB_pA_pair,
    GB_Element *restrict GB_Heap,
    const int64_t GB_bjnz_max,

    // for Gustavson method only:
    GB_Sauna GB_C_Sauna,

    GB_Context Context
)
{
    GrB_Info GB_info = GrB_INVALID_OBJECT ;

    if (0)
    {
        ;
    }
    else if (GB_s == Rg_structured_bool)
    {
        if (GB_AxB_method == GxB_AxB_GUSTAVSON)
        { 
            GB_info = GB_AxB_user_gus_Rg_structured_bool
                (*GB_Chandle, GB_M, GB_A, GB_B, GB_flipxy, GB_C_Sauna,
                Context) ;
        }
        else if (GB_AxB_method == GxB_AxB_DOT)
        { 
            GB_info = GB_AxB_user_dot_Rg_structured_bool
                (GB_Chandle, GB_M, GB_A, GB_B, GB_flipxy, Context) ;
        }
        else // (GB_AxB_method == GxB_AxB_HEAP)
        { 
            GB_info = GB_AxB_user_heap_Rg_structured_bool
                (GB_Chandle, GB_M, GB_A, GB_B, GB_flipxy,
                GB_List, GB_pA_pair, GB_Heap, GB_bjnz_max, Context) ;
        }
    } 

    if (GB_info == GrB_INVALID_OBJECT)
    {
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG, "undefined semiring"))) ;
    }
    return (GB_info) ;
}

