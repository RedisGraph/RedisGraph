//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_plus_rdiv2.m4: example user built-in objects
//------------------------------------------------------------------------------

// This version tests the case when the user-defined multiply operator
// has a different type for x and y.

#ifdef GxB_USER_INCLUDE

    #define MY_RDIV2

    static inline void my_rdiv2
    (
        double *z,
        const double *x,
        const float *y
    )
    {
        (*z) = (*y) / (*x) ;
    }

#endif

// rdiv2 operator
GxB_BinaryOp_define(My_rdiv2,  my_rdiv2,  GrB_FP64, GrB_FP64, GrB_FP32) ;

// plus-rdiv2 semiring
GxB_Semiring_define(My_plus_rdiv2, GxB_PLUS_FP64_MONOID, My_rdiv2) ;

