SuiteSparse/GraphBLAS/User: compile-time user-defined objects
--------------------------------------------------------------------------------

To use the pre-compiled user-defined types, operators, monoids, and semirings,
move one or more of the User/Example/*m4 files into this User/ directory, or
create your own *m4 file using the GxB_*_define functions.  Then recompile
SuiteSparse:GraphBLAS (cmake needs to be rerun).  The cmake process will
construct the appropriate compile-time definitions.  The declarations for all
such user-defined objects are appended to GraphBLAS/Include/GraphBLAS.h.

Example *.m4 files in the User/Example folder are listed below.  These are not
activated by default.  Move them form User/Example to User/ to use them.

my_complex.m4           double complex type, and standard plus-times-complex
                        semiring.  See also Demo/Source/usercomplex.c for a
                        complete set of operators defined a run-time instead.

my_plus_rdiv.m4         a binary operator z=y/x for double x,y,z

my_plus_rdiv2.m4        a binary operator z=y/x for single x, double x,y

my_scale.m4             a unary operator, z=my_scalar*x for double x,z

my_band.m4              a select operator for C=tril(triu(A,hi),lo)

my_pagerank.m4          pagerank type, operators, and semiring for
                        Demo/Source/dpagerank2.c
