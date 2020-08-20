function e = nzmax (G)
%NZMAX the number of entries in a GraphBLAS matrix.
% Since the GraphBLAS data structure is opaque, nzmax (G) has no
% particular meaning.  Thus, nzmax (G) is simply max (GrB.entries (G), 1).  
% It appears as an overloaded operator for a GrB matrix simply for
% compatibility with MATLAB sparse matrices.
%
% See also nnz, GrB.entries, GrB.nonz.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

e = max (GrB.entries (G), 1) ;

