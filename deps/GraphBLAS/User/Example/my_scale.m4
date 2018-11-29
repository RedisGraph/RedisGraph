//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_scale.m4: example user built-in objects
//------------------------------------------------------------------------------

// user-defined unary operator: z = f(x) = my_scalar*x and its global scalar

#ifdef GxB_USER_INCLUDE

    //--------------------------------------------------------------------------
    // declarations: for GraphBLAS.h
    //--------------------------------------------------------------------------

    // The following are declarations that are enabled in GraphBLAS.h and
    // appear in all user codes that #include "GraphBLAS.h", and also in all
    // internal GraphBLAS codes.  All user declarations (not definitions)
    // should appear here.

    #define MY_SCALE

    extern double my_scalar ;
    // for thread safety if the user application uses OpenMP
    #pragma omp threadprivate(my_scalar)

    static inline void my_scale
    (
        double *z,
        const double *x
    )
    {
        (*z) = my_scalar * (*x) ;
    }

#else

    //--------------------------------------------------------------------------
    // definitions: code appears just once, in Source/all_user_objects.c
    //--------------------------------------------------------------------------

    // The following defintions are enabled in only a single place:
    // SuiteSparse/GraphBLAS/Source/all_user_objects.c.  This is the place
    // where all user-defined global variables should be defined.

    double my_scalar = 0 ;

#endif


//------------------------------------------------------------------------------
// define/declare the GrB_UnaryOp My_scale
//------------------------------------------------------------------------------

// Unary operator to compute z = my_scalar*x

GxB_UnaryOp_define(My_scale, my_scale, GrB_FP64, GrB_FP64) ;

