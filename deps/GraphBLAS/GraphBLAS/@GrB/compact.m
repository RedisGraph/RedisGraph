function [C, I, J] = compact (A, id)
%GRB.COMPACT remove empty rows and columns from a matrix.
% C = GrB.compact (A) returns rows and columns from A that have no entries.
% It has no effect on a MATLAB full matrix, except to convert it to a
% GraphBLAS matrix, since all entries are present in a MATLAB full matrix.
%
% Explicit zeros in A are treated as entries, and are not removed by
% default.  To remove rows and columns with no entries or only explicit
% zero entries, use C = GrB.compact (A,0).  For a MATLAB sparse matrix,
% GrB.compact (A,0) and GrB.compact (A) are identical.
%
% To remove rows and colums that either have no entries, or that only have
% entries equal to a particular scalar value, use C = GrB.compact (A, id),
% where id is the scalar value.
%
% With two additional output arguments, [C,I,J] = GrB.compact (A, ...),
% the indices of non-empty rows and columns of A are returned, so that
% C = A (I,J).  The lists I and J are returned in sorted order.
%
% Example:
%
%   n = 2^40 ;
%   H = GrB (n,n) ;                 % create a huge hypersparse matrix
%   I = sort (randperm (n, 4)) ;
%   J = sort (randperm (n, 4)) ;
%   A = magic (4) ;
%   H (I,J) = A
%   [C, I, J] = GrB.compact (H)
%   isequal (C, A)                  % C and A are the same
%   isequal (C, H(I,J))             % C and H(I,J) are the same
%   H (I, J(1)) = 0
%   [C, I, J] = GrB.compact (H, 0)
%   norm (C - A (:,2:end), 1) == 0
%
% See also GrB.entries, GrB.nonz, GrB.prune.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (isobject (A))
    A = A.opaque ;
end

if (nargin > 1)
    % prune identity values from A
    id = gb_get_scalar (id) ;
    if (id ~= 0)
        % prune a nonzero identity value from A
        A = gbselect (A, '~=', id) ;
    elseif (~builtin ('issparse', A))
        % prune zeros from A, but skip this if A is a MATLAB sparse matrix
        A = gbselect (A, 'nonzero') ;
    end
end

% get the list of non-empty rows and columns
I = gb_entries (A, 'row', 'list') ;
J = gb_entries (A, 'col', 'list') ;

% C = A (I,J)
C = GrB (gbextract (A, { I }, { J })) ;

