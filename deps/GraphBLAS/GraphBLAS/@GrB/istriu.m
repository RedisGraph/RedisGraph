function s = istriu (G)
%ISTRIU  Determine if a matrix is upper triangular.
% istriu (G) is true if all entries in the GraphBLAS matrix G are on or
% above the diagonal.  A GraphBLAS matrix G may have explicit zeros.  If
% these appear in the lower triangular part of G, then istriu (G) is
% false, but istriu (double (G)) can be true since double (G) drops those
% entries.
%
% See also GrB/istriu, GrB/isbanded.

% FUTURE: this will be much faster when written as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = (GrB.entries (tril (G, -1)) == 0) ;

