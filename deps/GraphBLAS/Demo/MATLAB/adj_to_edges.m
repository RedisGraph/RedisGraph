function E = adj_to_edges (A)
%ADJ_TO_EDGES create an edge incidence matrix from an adjacency matrix
%
% E = adj_to_edges (A)
%
% If A(i,j) is nonzero, then the edge E ([i j],k)=1 is created in E,
% for some column k.  Each column of E has exactly two 1's in it.
% E has size n-by-nnz(A), where n=size(A,1).  A must be square, and
% its diagonal is ignored.  A is symmetrized with A=A+A'.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m n] = size (A) ;
if (m ~= n)
    error ('A must be square') ;
end

% make A symmetric and remove the diagonal
A = spones (A) ;
A = spones (A+A') ;
L = tril (A,-1) ;

[i,j,~] = find (L) ;
nz = nnz (L) ;
I = [i ; j] ;
J = [1:nz 1:nz]' ;
X = ones (2*nz, 1)  ;
E = sparse (I, J, X, n, nz) ;

% check the result
A2 = edges_to_adj (E) ;
if (~isequal (A,A2))
    error ('invalid incidence matrix E') ;
end
