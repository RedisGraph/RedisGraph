//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_plus_rdiv.m4: example user built-in objects
//------------------------------------------------------------------------------

#ifdef GxB_USER_INCLUDE

#define MY_RDIV

    static inline void my_rdiv
    (
        double *z,
        const double *x,
        const double *y
    )
    {
        (*z) = (*y) / (*x) ;
    }

#endif

// rdiv operator
GxB_BinaryOp_define(My_rdiv,  my_rdiv,  GrB_FP64, GrB_FP64, GrB_FP64) ;

// plus-rdiv semiring
GxB_Semiring_define(My_plus_rdiv, GxB_PLUS_FP64_MONOID, My_rdiv) ;

