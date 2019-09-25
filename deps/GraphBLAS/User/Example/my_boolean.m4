//------------------------------------------------------------------------------
// GraphBLAS/User/Example/my_terminal.m4: example user built-in objects
//------------------------------------------------------------------------------

// user-defined Boolean semiring.  This is just for testing.  The semiring
// is identical to GxB_LOR_LAND_BOOL, and the monoid is identical to
// GxB_LOR_BOOL_MONOID.  The only difference is that these objects are
// user-defined.

#ifdef GxB_USER_INCLUDE

    #define MY_BOOL

#endif

// The LOR monoid, with identity = false and terminal = true
GxB_Monoid_terminal_define(My_LOR, GrB_LOR, false, true) ;

// The LOR_LAND semiring
GxB_Semiring_define(My_LOR_LAND, My_LOR, GrB_LAND) ;


