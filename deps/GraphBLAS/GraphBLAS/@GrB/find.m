function [I, J, X] = find (G, k, search)
%FIND extract entries from a GraphBLAS matrix.
% [I, J, X] = find (G) extracts the nonzeros from a GraphBLAS matrix G.
% X has the same type as G ('double', 'single', 'int8', ...).
%
% Linear 1D indexing (I = find (S) for the MATLAB matrix S) is not yet
% supported.
%
% G may contain explicit zero entries, and by default these are excluded
% from the result.  Use GrB.extracttuples (G) to return these explicit
% zero entries.
%
% For a column vector, I = find (G) returns I as a list of the row
% indices of nonzeros in G.  For a row vector, I = find (G) returns I as a
% list of the column indices of nonzeros in G.
%
% [...] = find (G, k, 'first') returns the first k nonozeros of G.
% [...] = find (G, k, 'last')  returns the last k nonozeros of G.
% For this usage, the first and last k are in terms of nonzeros in the
% column-major order.
%
% See also sparse, GrB.build, GrB.extracttuples.

% FUTURE: add linear indexing

% FUTURE: find (G,k,'first') and find (G,k,'last') are slow, since as
% they are currently implemented, all entries are extracted and then the
% first or last k are selected from the extracted tuples.  It would be
% faster to use a mexFunction that directly accesses the opaque content
% of G, instead of using GrB_Matrix_extractTuples_*, which always extracts
% the entire matrix.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin > 1 && ~GrB.isbycol (G))
    % find (G, k) assumes the matrix is stored by column, so reformat G
    % if it is stored by row.
    G = GrB (G, 'by col') ;
end

if (nnz (G) < GrB.entries (G))
    G = GrB.prune (G) ;
end

if (nargout == 3)
    [I, J, X] = GrB.extracttuples (G) ;
    if (isrow (G))
        I = I' ;
        J = J' ;
        X = X' ;
    end
elseif (nargout == 2)
    [I, J] = GrB.extracttuples (G) ;
    if (isrow (G))
        I = I' ;
        J = J' ;
    end
else
    if (isrow (G))
        [~, I] = GrB.extracttuples (G) ;
        I = I' ;
    else
        % FUTURE: this does not return the same thing
        I = GrB.extracttuples (G) ;
    end
end

if (nargin > 1)
    % find (G, k, ...): get the first or last k entries
    if (nargin == 2)
        search = 'first' ;
    end
    if (~isscalar (k))
        gb_error ('k must be a scalar') ;
    end
    k = ceil (double (k)) ;
    if (k < 1)
        gb_error ('k must be positive') ;
    end
    n = length (I) ;
    k = min (k, n) ;
    if (isequal (search, 'first'))
        % find (G, k, 'first'): get the first k entries
        I = I (1:k) ;
        if (nargout > 1)
            J = J (1:k) ;
        end
        if (nargout > 2)
            X = X (1:k) ;
        end
    elseif (isequal (search, 'last'))
        % find (G, k, 'last'): get the last k entries
        I = I (n-k+1:n) ;
        if (nargout > 1)
            J = J (n-k+1:n) ;
        end
        if (nargout > 2)
            X = X (n-k+1:n) ;
        end
    else
        gb_error ('invalid search option; must be ''first'' or ''last''') ;
    end
end

