function C = flip (G, dim)
%FLIP flip the order of elements
% C = flip (G) flips the order of elements in each column of G.  That is,
% C = G (end:-1:1,:).  C = flip (G, dim) specifies the dimension to flip,
% so that flip (G,1) and flip (G) are the same thing, and flip (G,2) flips
% the columns so that C = G (:,end:-1,1).
%
% See also transpose.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 2)
    if (isrow (G))
        dim = 2 ;
    else
        dim = 1 ;
    end
end

[m, n] = size (G) ;

if (~isscalar (dim))
    gb_error ('dim must be a scalar') ;
end
dim = floor (double (dim)) ;
if (dim <= 0)
    gb_error ('dim must be positive') ;
end

if (dim == 1 && m ~= 1)
    % C = G (m:-1:1, :)
    I = { m, -1, 1 } ;
    J = { } ;
    C = GrB.extract (G, I, J) ;
elseif (dim == 2 && n ~= 1)
    % C = G (:, n:-1:1)
    I = { } ;
    J = { n, -1, 1 } ;
    C = GrB.extract (G, I, J) ;
else
    % nothing to do
    C = G ;
end

