function s = isbyrow (A)
%GRB.ISBYROW true if A is stored by row, false if by column.
% s = GrB.isbyrow (A) is true if A is stored by row, false if by column.
% A may be a GraphBLAS matrix or MATLAB matrix (sparse or full).  MATLAB
% matrices are always stored by column.
%
% See also GrB.isbycol, GrB.format.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (isobject (A))
    A = A.opaque ;
    s = isequal (gbformat (A), 'by row')  ;
else
    s = false ;
end
