function L = tril (G, k)
%TRIL lower triangular part of a GraphBLAS matrix.
% L = tril (G) returns the lower triangular part of G.
%
% L = tril (G,k) returns the entries on and below the kth diagonal of G,
% where k=0 is the main diagonal.
%
% See also GrB/triu.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 2)
    k = 0 ;
end
L = GrB.select ('tril', G, k) ;

