function check_adj (A)
%CHECK_ADJ ensure A is a valid adjacency matrix
% usage: check_adj (A)
%
% A must be square, symmetric, binary, with no entries on the diagonal

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m n] = size (A) ;
if (m ~= n)
    error ('A must be square') ;
end
if (nnz (diag (A) ~= 0))
    error ('diagonal of A must be all zero') ;
end
[i j x] = find (A) ;
if (any (x ~= 1))
    error ('A is not binary') ;
end
if (~isequal (A, A'))
    error ('A is not symmetric') ;
end

