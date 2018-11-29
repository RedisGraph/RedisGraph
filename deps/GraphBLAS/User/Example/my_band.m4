//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_band.m4: example user built-in objects
//------------------------------------------------------------------------------

// user-defined functions for GxB_select, to choose entries within a band

#ifdef GxB_USER_INCLUDE

    #define MY_BAND

    static inline bool myband (GrB_Index i, GrB_Index j, GrB_Index nrows,
        GrB_Index ncols, const void *x, const void *thunk)
    {
        int64_t *lohi = (int64_t *) thunk ;
        int64_t i2 = (int64_t) i ;
        int64_t j2 = (int64_t) j ;
        return ((lohi [0] <= (j2-i2)) && ((j2-i2) <= lohi [1])) ;
    }

#endif

// Select operator to compute C = tril (triu (A, k1), k2)
GxB_SelectOp_define(My_band, myband, NULL) ;

