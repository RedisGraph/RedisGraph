function result = entries (A, varargin)
%GRB.ENTRIES count or query the entries of a matrix.
% An entry A(i,j) in a GraphBLAS matrix is one that is present in the
% data structure.  Unlike a MATLAB sparse matrix, a GraphBLAS matrix can
% contain explicit zero entries.  All entries in a MATLAB sparse matrix
% are nonzero.  A MATLAB full matrix has all of its entries present,
% regardless of their value.  The GrB.entries function looks only at the
% pattern of A, not its values.  To exclude explicit entries with a value
% of zero (or any specified additive identity value) use GrB.nonz
% instead.
%
% let [m n] = size (A)
%
% e = GrB.entries (A)         number of entries
% e = GrB.entries (A, 'all')  number of entries
% e = GrB.entries (A, 'row')  number of rows with at least one entry
% e = GrB.entries (A, 'col')  number of columns with at least one entry
%
% X = GrB.entries (A, 'list')         list of values of unique entries
% X = GrB.entries (A, 'all', 'list')  list of values of unique entries
% I = GrB.entries (A, 'row', 'list')  list of rows with at least one entry
% J = GrB.entries (A, 'col', 'list')  list of cols with at least one entry
%
% d = GrB.entries (A, 'row', 'degree')
%   If A is m-by-n, then d is a sparse column vector of size m, with d(i)
%   equal to the number of entries in A(i,:).  If A(i,:) has no entries,
%   then d(i) is an implicit zero, not present in the pattern of d, so
%   that I = find (d) is the same I = GrB.entries (A, 'row', 'list').
%
% d = GrB.entries (A, 'col', 'degree')
%   If A is m-by-n, d is a sparse column vector of size n, with d(j)
%   equal to the number of entries in A(:,j).  If A(:,j) has no entries,
%   then d(j) is an implicit zero, not present in the pattern of d, so
%   that I = find (d) is the same I = GrB.entries (A, 'col', 'list').
%
% Example:
%
%   A = magic (5) ;
%   A (A < 10) = 0             % MATLAB full matrix with some explicit zeros
%   GrB.entries (A)            % all entries present in a MATLAB full matrix
%   G = GrB (A)                % contains explicit zeros
%   GrB.entries (G)
%   G (A > 18) = sparse (0)    % entries A>18 deleted, has explicit zeros
%   GrB.entries (G)
%   GrB.entries (G, 'list')
%   S = double (G)             % MATLAB sparse matrix; no explicit zeros
%   GrB.entries (S)
%   GrB.entries (S, 'list')
%
% See also GrB.nonz, nnz, GrB/nnz, nonzeros, GrB/nonzeros.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% get the string arguments
dim = 'all' ;           % 'all', 'row', or 'col'
kind = 'count' ;        % 'count', 'list', or 'degree'
for k = 1:nargin-1
    arg = varargin {k} ;
    switch arg
        case { 'all', 'row', 'col' }
            dim = arg ;
        case { 'count', 'list', 'degree' }
            kind = arg ;
        otherwise
            gb_error ('unknown option') ;
    end
end

if (isequal (dim, 'all'))

    switch kind
        case 'count'
            % number of entries in A
            % e = GrB.entries (A)
            if (isa (A, 'GrB'))
                result = gbnvals (A.opaque) ;
            else
                result = gbnvals (A) ;
            end
        case 'list'
            % list of values of unique entries
            % X = GrB.entries (A, 'list')
            if (isa (A, 'GrB'))
                result = unique (gbextractvalues (A.opaque)) ;
            else
                result = unique (gbextractvalues (A)) ;
            end
        otherwise
            gb_error ('''all'' and ''degree'' cannot be combined') ;
    end

else

    % get the row or column degree
    f = GrB.format (A) ;
    native = (isequal (f, 'by row') && isequal (dim, 'row')) || ...
             (isequal (f, 'by col') && isequal (dim, 'col')) ;
    if (isa (A, 'GrB'))
        degree = GrB (gbdegree (A.opaque, native)) ;
    else
        degree = GrB (gbdegree (A, native)) ;
    end

    switch kind
        case 'count'
            % number of non-empty rows/cols
            % e = GrB.entries (A, 'row')
            % e = GrB.entries (A, 'col')
            result = nnz (degree) ;
        case 'list'
            % list of non-empty rows/cols
            % I = GrB.entries (A, 'row', 'list')
            % J = GrB.entries (A, 'col', 'list')
            result = find (degree) ;
        case 'degree'
            % degree of all rows/cols
            % d = GrB.entries (A, 'row', 'degree')
            % d = GrB.entries (A, 'col', 'degree')
            result = degree ;
    end
end

