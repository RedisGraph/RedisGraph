function s = istril (G)
%ISTRIL  Determine if a matrix is lower triangular.
% istril (G) is true if all entries in the GraphBLAS matrix G are on or
% below the diagonal.  A GraphBLAS matrix G may have explicit zeros.  If
% these appear in the upper triangular part of G, then istril (G) is
% false, but istril (double (G)) can be true since double (G) drops those
% entries.
%
% See also GrB/istriu, GrB/isbanded.

% FUTURE: this will be much faster when written as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = (GrB.entries (triu (G, 1)) == 0) ;

