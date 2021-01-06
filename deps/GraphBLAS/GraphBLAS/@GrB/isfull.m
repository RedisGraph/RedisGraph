function s = isfull (A)
%GRB.ISFULL determine if all entries are present.
% For either a GraphBLAS or MATLAB matrix, GrB.isfull (A) is true if
% numel(A) == nnz(A).  GrB.isfull (A) is always true if A is a GraphBLAS
% or MATLAB full matrix.
%
% See also GrB/issparse, GrB/full.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (isobject (A))
    % GraphBLAS matrix
    A = A.opaque ;
    s = gb_isfull (A) ;
elseif (issparse (A))
    % MATLAB sparse matrix
    s = (numel (A) == nnz (A)) ;
else
    % MATLAB full matrix, string, struct, etc
    s = true ;
end

