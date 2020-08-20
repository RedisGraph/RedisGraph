function C = sum (G, option)
%SUM Sum of elements.
% C = sum (G), where G is an m-by-n GraphBLAS matrix, computes a 1-by-n
% row vector C where C(j) is the sum of all entries in G(:,j).  If G is a
% row or column vector, then sum (G) is a scalar sum of all the entries
% in the vector.
%
% C = sum (G,'all') sums all elements of G to a single scalar.
%
% C = sum (G,1) is the default when G is a matrix, which is to sum each
% column to a scalar, giving a 1-by-n row vector.  If G is already a row
% vector, then C = G.
%
% C = sum (G,2) sums each row to a scalar, resulting in an m-by-1 column
% vector C where C(i) is the sum of all entries in G(i,:).
%
% The MATLAB sum (A, ... type, nanflag) allows for different types of
% sums to be computed, and the NaN behavior can be specified.  The
% GraphBLAS sum (G,...) uses only a type of 'native', and a nanflag of
% 'includenan'.  See 'help sum' for more details.
%
% See also GrB/prod, GrB/max, GrB/min.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isequal (GrB.type (G), 'logical'))
    op = '+.int64' ;
else
    op = '+' ;
end
desc = struct ('in0', 'transpose') ;

if (nargin == 1)
    % C = sum (G); check if G is a row vector
    if (isvector (G))
        % C = sum (G) for a vector G results in a scalar C
        C = GrB.reduce (op, G) ;
    else
        % C = sum (G) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce (op, G, desc)' ;
    end
elseif (isequal (option, 'all'))
    % C = sum (G, 'all'), reducing all entries to a scalar
    C = GrB.reduce (op, G) ;
elseif (isequal (option, 1))
    % C = sum (G,1) reduces each column to a scalar,
    % giving a 1-by-n row vector.
    C = GrB.vreduce (op, G, desc)' ;
elseif (isequal (option, 2))
    % C = sum (G,2) reduces each row to a scalar,
    % giving an m-by-1 column vector.
    C = GrB.vreduce (op, G) ;
else
    gb_error ('unknown option') ;
end

