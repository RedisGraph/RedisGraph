//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_complex.m4: example user built-in objects
//------------------------------------------------------------------------------

// user-defined functions for a double complex type

#ifdef GxB_USER_INCLUDE

    // Get the complex.h definitions, but remove "I" since it is used elsewhere
    // in GraphBLAS.
    #include <complex.h>
    #undef I

    // Not all complex.h definitions include the CMPLX macro
    #ifndef CMPLX
    #define CMPLX(real,imag) \
        ( \
        (double complex)((double)(real)) + \
        (double complex)((double)(imag) * _Complex_I) \
        )
    #endif

    // define a token so a user application can check for existence 
    #define MY_COMPLEX

    static inline void my_complex_plus
    (
        double complex *z,
        const double complex *x,
        const double complex *y
    )
    {
        (*z) = (*x) + (*y) ;
    }

    static inline void my_complex_times
    (
        double complex *z,
        const double complex *x,
        const double complex *y
    )
    {
        (*z) = (*x) * (*y) ;
    }

#endif

// GraphBLAS does not have a complex type; this defines one:
GxB_Type_define(My_Complex, double complex) ;

// The two operators, complex add and multiply:
GxB_BinaryOp_define(My_Complex_plus,  my_complex_plus, 
    My_Complex, My_Complex, My_Complex) ;

GxB_BinaryOp_define(My_Complex_times, my_complex_times,
    My_Complex, My_Complex, My_Complex) ;

// The plus monoid:
GxB_Monoid_define(My_Complex_plus_monoid, My_Complex_plus, CMPLX(0,0)) ;

// the conventional plus-times semiring for C=A*B for the complex case
GxB_Semiring_define(My_Complex_plus_times, My_Complex_plus_monoid,
    My_Complex_times) ;

