function f = format (arg)
%GRB.FORMAT get/set the default GraphBLAS matrix format.
%
% In its ANSI C interface, SuiteSparse:GraphBLAS stores its matrices by
% row, by default, since that format tends to be fastest for graph
% algorithms, but it can also store its matrices by column.  MATLAB
% sparse and dense sparse matrices are always stored by column.  For
% better compatibility with MATLAB sparse matrices, the default for the
% MATLAB interface for SuiteSparse:GraphBLAS is to store matrices by
% column.  This has performance implications, and algorithms should be
% designed accordingly.  The default format can be can changed via:
%
%   GrB.format ('by row')
%   GrB.format ('by col')
%
% which changes the format of all subsequent GraphBLAS matrices.
% Existing GrB matrices are not affected.
%
% The current default global format can be queried with
%
%   f = GrB.format ;
%
% which returns the string 'by row' or 'by col'.
%
% Converting a matrix to a specific format can be done with the following,
% where A is either a GraphBLAS matrix or MATLAB matrix:
%
%   G = GrB (A, 'by row')
%
% If a subsequent algorithm works better with its matrices held by row,
% then this transformation can save significant time in the long run.
% Graph algorithms tend to be faster with their matrices held by row,
% since the edge (i,j) is typically the entry G(i,j) in the matrix G, and
% most graph algorithms need to know the outgoing edges of node i.  This
% is G(i,:), which is very fast if G is held by row, but very slow if G
% is held by column.
%
% When the GrB.format (f) is changed, it becomes the default format for
% all subsequent newly created matrices.
%
% All prior matrices created before GrB.format (f) are kept in their same
% format; this setting only applies to new matrices.  Operations on matrices
% can be done with any mix of with different formats.  The format only affects
% time and memory usage, not the results.
%
% The format of the output C of a GraphBLAS method is defined using the
% following rules.  The first rule that holds is used:
%
%   (1) GraphBLAS operations of the form Cout = GrB.method (Cin, ...)
%       that take a Cin input matrix, use the format of Cin as the format
%       for Cout, if Cin is provided on input.
%   (2) If the format is determined by the descriptor to the method, then
%       that determines the format of C.
%   (3) If C is a column vector then C is stored by column.
%   (4) If C is a row vector then C is stored by row.
%   (5) If the method has a first matrix input (usually called A), and it
%       is not a row or column vector, then its format is used for C.
%   (6) If the method has a second matrix input (usually called B), and
%       it is not a row or column vector, then its format is used for C.
%   (7) Otherwise, the global default format is used for C.
%
% The GrB.format setting is reset to 'by col', by 'clear all' or by
% GrB.clear.
%
% To query the format for a given GraphBLAS matrix G, use the following
% (which does not affect the global format setting):
%
%   f = GrB.format (G)
%
% Use G = GrB (G, 'by row') or G = GrB (G, 'by col') to change the format
% of G after it is constructed.
%
% Examples:
%
%   A = sparse (rand (4))
%   G = GrB (A)                  % format always 'by col'
%   G = GrB (A, 'by row')        % change to 'by row'
%   GrB.format (G)
%   GrB.format ('by row') ;      % set the default format to 'by row'
%   G = GrB.build (1:3, 1:3, 1:3)
%   GrB.format (G)               % query the format of G (which is 'by row')
%
% See also GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    % f = GrB.format ; get the global format
    f = gbformat ;
else
    if (isa (arg, 'GrB'))
        % f = GrB.format (G) ; get the format of the matrix G
        f = gbformat (arg.opaque) ;
    else
        % f = GrB.format (f) ; set the global format for all future matrices
        f = gbformat (arg) ;
    end
end

