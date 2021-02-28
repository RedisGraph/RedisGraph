function C = zeros (varargin)
%ZEROS an all-zero matrix, the same type as G.
% C = zeros (m, n, 'like', G) or C = zeros ([m n], 'like', G) returns
% an m-by-n matrix with no entries, of the same type as G.
%
% See also GrB/ones, GrB/false, GrB/true.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

G = varargin {end} ;
if (nargin == 4)
    if (~isequal (varargin {3}, 'like'))
        gb_error ('usage: zeros (m, n, ''like'', G)') ;
    end
    m = varargin {1} ;
    n = varargin {2} ;
elseif (nargin == 3)
    if (~isequal (varargin {2}, 'like'))
        gb_error ('usage: zeros ([m n], ''like'', G)') ;
    end
    mn = varargin {1} ;
    m = mn (1) ;
    n = mn (2) ;
else
    gb_error ('usage: zeros (m, n, ''like'', G)') ;
end

C = GrB (m, n, GrB.type (G)) ;

