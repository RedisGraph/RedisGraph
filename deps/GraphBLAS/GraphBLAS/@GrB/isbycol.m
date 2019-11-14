function s = isbycol (X)
%GRB.ISBYCOL True if X is stored by column, false if by column.
% s = GrB.isbycol (X) is true if X is stored by column, false if by row.
% X may be a GraphBLAS matrix or MATLAB matrix (sparse or full).  MATLAB
% matrices are always stored by column.
%
% See also GrB.isbyrow, GrB.format.

s = isequal (GrB.format (X), 'by col')  ;

