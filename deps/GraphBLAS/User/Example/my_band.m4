//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_band.m4: example user built-in objects
//------------------------------------------------------------------------------

// user-defined functions for GxB_select, to choose entries within a band

#ifdef GxB_USER_INCLUDE

    #define MY_BAND

    typedef struct
    {
        int64_t lo ;
        int64_t hi ;
    }
    my_bandwidth_type ;

    static inline bool myband (GrB_Index i, GrB_Index j, GrB_Index nrows,
        GrB_Index ncols, /* x is unused: */ const void *x,
        const my_bandwidth_type *thunk)
    {
        int64_t i2 = (int64_t) i ;
        int64_t j2 = (int64_t) j ;
        return ((thunk->lo <= (j2-i2)) && ((j2-i2) <= thunk->hi)) ;
    }

#endif

// The type of the thunk parameter
GxB_Type_define(My_bandwidth_type, my_bandwidth_type) ;

// Select operator to compute C = tril (triu (A, k1), k2)
GxB_SelectOp_define(My_band, myband, NULL, My_bandwidth_type) ;

