function s = isbyrow (X)
%GRB.ISBYROW True if X is stored by row, false if by column.
% s = GrB.isbyrow (X) is true if X is stored by row, false if by column.
% X may be a GraphBLAS matrix or MATLAB matrix (sparse or full).  MATLAB
% matrices are always stored by column.
%
% See also GrB.isbycol, GrB.format.

s = isequal (GrB.format (X), 'by row')  ;

