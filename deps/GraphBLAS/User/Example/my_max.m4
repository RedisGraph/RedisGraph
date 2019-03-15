//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_max.m4: example user built-in objects
//------------------------------------------------------------------------------

// user-defined MAX functions for GxB_Monoid_terminal_new, to choose a
// non-default terminal value

#ifdef GxB_USER_INCLUDE

    #define MY_MAX

    static inline void my_maxdouble
    (
        double *z,
        const double *x,
        const double *y
    )
    {
        // this is not safe with NaNs
        (*z) = ((*x) > (*y)) ? (*x) : (*y) ;
    }

#endif

// max operator
GxB_BinaryOp_define(My_Max, my_maxdouble, GrB_FP64, GrB_FP64, GrB_FP64) ;

// The max monoid, with terminal value of 1
GxB_Monoid_terminal_define(My_Max_Terminal1, My_Max, (-INFINITY), 1) ;

