function C = any (G, option)
%ANY True if any element of a GraphBLAS matrix is nonzero or true.
%
% C = any (G) is true if any entry in G is nonzero or true.  If G is a
% matrix, C is a row vector with C(j) = any (G (:,j)).
%
% C = any (G, 'all') is a scalar, true if any entry in G is nonzero or true.
% C = any (G, 1) is a row vector with C(j) = any (G (:,j))
% C = any (G, 2) is a column vector with C(i) = any (G (i,:))
%
% See also all, nnz, GrB/nnz, GrB.entries, GrB.nonz.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

desc = struct ('in0', 'transpose') ;

if (nargin == 1)

    % C = any (G)
    if (isvector (G))
        % C = any (G) for a vector G results in a scalar C
        C = GrB.reduce ('|.logical', G) ;
    else
        % C = any (G) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce ('|.logical', G, desc)' ;
    end

else

    % C = any (G, option)
    if (isequal (option, 'all'))
        % C = any (G, 'all'), reducing all entries to a scalar
        C = GrB.reduce ('|.logical', G) ;
    elseif (isequal (option, 1))
        % C = any (G, 1) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce ('|.logical', G, desc)' ;
    elseif (isequal (option, 2))
        % C = any (G, 2) reduces each row to a scalar,
        % giving an m-by-1 column vector.
        C = GrB.vreduce ('|.logical', G) ;
    else
        gb_error ('unknown option') ;
    end
end

