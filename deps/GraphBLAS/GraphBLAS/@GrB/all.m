function C = all (G, option)
%ALL True if all elements of a GraphBLAS matrix are nonzero or true.
%
% C = all (G) is true if all entries G are nonzero or true.  If G is a
% matrix, C is a row vector with C(j) = all (G (:,j)).
%
% C = all (G, 'all') is a scalar, true if all entries G are nonzero or true.
% C = all (G, 1) is a row vector with C(j) = all (G (:,j))
% C = all (G, 2) is a column vector with C(i) = all (G (i,:))
%
% See also any, nnz, GrB.entries.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
desc = struct ('in0', 'transpose') ;

if (nargin == 1)

    % C = all (G)
    if (isvector (G))
        % C = all (G) for a vector G results in a scalar C
        if (~GrB.isfull (G))
            C = GrB (false, 'logical') ;
        else
            C = GrB.reduce ('&.logical', G) ;
        end
    else
        % C = all (G) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce ('&.logical', G, desc) ;
        % if C(j) is true, but the column is sparse, then assign C(j) = 0.
        coldegree = GrB.entries (G, 'col', 'degree') ;
        C = GrB.subassign (C, C & (coldegree < m), 0)' ;
    end

else

    % C = all (G, option)
    if (isequal (option, 'all'))
        % C = all (G, 'all'), reducing all entries to a scalar
        if (~GrB.isfull (G))
            C = GrB (false, 'logical') ;
        else
            C = GrB.reduce ('&.logical', G) ;
        end
    elseif (isequal (option, 1))
        % C = all (G, 1) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce ('&.logical', G, desc) ;
        % if C(j) is true, but the column is sparse, then assign C(j) = 0.
        coldegree = GrB.entries (G, 'col', 'degree') ;
        C = GrB.subassign (C, C & (coldegree < m), 0)' ;
    elseif (isequal (option, 2))
        % C = all (G, 2) reduces each row to a scalar,
        % giving an m-by-1 column vector.
        C = GrB.vreduce ('&.logical', G) ;
        % if C(i) is true, but the row is sparse, then assign C(i) = 0.
        rowdegree = GrB.entries (G, 'row', 'degree') ;
        C = GrB.subassign (C, C & (rowdegree < n), 0) ;
    else
        gb_error ('unknown option') ;
    end
end

