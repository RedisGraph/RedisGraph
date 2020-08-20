function gbtest52
%GBTEST52 test GrB.format

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

GrB.format
GrB.format ('by col') ;
f = GrB.format %#ok<*NOPRT>
A = magic (4)
G = GrB (A)
assert (isequal (f, GrB.format (G))) ;
GrB.format ('by row')
f = GrB.format %#ok<*NASGU>

H = GrB (5,5)
assert (isequal ('by row', GrB.format (H))) ;

H = GrB (5,5, 'by row')
assert (isequal ('by row', GrB.format (H))) ;

H = GrB (5,5, 'by col')
assert (isequal ('by col', GrB.format (H))) ;

GrB.format ('by col')
f = GrB.format

H = GrB (5,5)
assert (isequal ('by col', GrB.format (H))) ;

H = GrB (5,5, 'by row')
assert (isequal ('by row', GrB.format (H))) ;

H = GrB (5,5, 'by col')
assert (isequal ('by col', GrB.format (H))) ;

fprintf ('gbtest52: all tests passed\n') ;

