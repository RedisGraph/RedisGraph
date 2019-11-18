function s = isfull (A)
%GRB.ISFULL determine if all entries are present.
% For a GraphBLAS matrix, or a MATLAB sparse matrix, GrB.isfull (A) is true if
% numel (A) == nnz (A).  A can be a GraphBLAS matrix, or a MATLAB sparse or
% full matrix.  GrB.isfull (A) is always true if A is a MATLAB full matrix.

if (isa (A, 'GrB') || issparse (A))
    s = (numel (A) == nnz (A)) ;
else
    s = true ;
end

