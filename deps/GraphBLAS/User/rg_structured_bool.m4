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
GxB_BinaryOp_define(Rg_bplus, rg_bplus, GrB_BOOL, GrB_BOOL, GrB_BOOL) ;

// bmul operator
GxB_BinaryOp_define(Rg_bmul, rg_bmul, GrB_BOOL, GrB_BOOL, GrB_BOOL) ;

// The plus monoid:
GxB_Monoid_define(Rg_Bool_plus_monoid, Rg_bplus, false) ;

// plus-bmul semiring
GxB_Semiring_define(Rg_structured_bool, Rg_Bool_plus_monoid, Rg_bmul) ;

