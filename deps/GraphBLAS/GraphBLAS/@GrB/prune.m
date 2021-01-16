function C = prune (G, id)
%GRB.PRUNE remove explicit values from a matrix.
% C = GrB.prune (G) removes any explicit zeros from G.
% C = GrB.prune (G, id) removes entries equal to the given scalar id.
%
% See also GrB/full, GrB.select, GrB.prune.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (nargin == 1)
    id = 0 ;
else
    id = gb_get_scalar (id) ;
end

if (builtin ('issparse', G) && id == 0)
    % a MATLAB sparse matrix 'never' contains explicit zeros,
    % so no need to prune.  C should be returned as a GraphBLAS
    % matrix, however.
    C = GrB (G) ;
else
    if (isobject (G))
        % extract the contents of a GraphBLAS matrix
        G = G.opaque ;
    end
    if (id == 0)
        C = GrB (gbselect (G, 'nonzero')) ;
    else
        C = GrB (gbselect (G, '~=', id)) ;
    end
end

