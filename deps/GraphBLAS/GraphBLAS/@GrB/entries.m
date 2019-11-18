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
% e = GrB.entries (A, 'row')  number of rows with at least one entries
% e = GrB.entries (A, 'col')  number of columns with at least one entries
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

% FUTURE: if A is stored by row, then the row degree can be found quickly,
% in a mexFunction that accesses A->p and A->h.  If stored by col, then
% the col degree is the same thing.  Write a mexFunction that computes
% the vector degree (by row or by column).

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
            if (isa (A, 'GrB'))
                result = gbnvals (A.opaque) ;
            else
                result = gbnvals (A) ;
            end
        case 'list'
            if (isa (A, 'GrB'))
                result = unique (gbextractvalues (A.opaque)) ;
            else
                result = unique (gbextractvalues (A)) ;
            end
        otherwise
            gb_error ('''all'' and ''degree'' cannot be combined') ;
    end
else
    desc = struct ;
    if (isequal (dim, 'col'))
        desc.in0 = 'transpose' ;
    end
    degree = GrB.vreduce ('+', GrB.apply ('1.double', A), desc) ;
    switch kind
        case 'count'
            result = GrB.entries (degree) ;
        case 'list'
            result = find (degree) ;
        case 'degree'
            result = degree ;
    end
end

