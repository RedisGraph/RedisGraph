function C = sprandsym (G, varargin)
%SPRANDSYM random symmetric GraphBLAS matrix
% C = sprandsym (G) is a symmetric random GraphBLAS matrix.  Its
%   lower triangle and diagonal have the same pattern as tril (G).
%   The values of C have a normal distribution.  G must be square.
%
% All optional parameters of GrB.random may be used:
%
%   C = sprandsym (G, 'uniform') uses a uniform distribution instead.
%
%   C = sprandsym (G, 'range', [lo hi]) modifies the range of the
%       distribution.  See GrB.random for more details.  The class of
%       [lo hi] determines the class of C ('double', 'single', ...).
%
% Example:
%
%   A = sprand (1000, 1000, 0.5) ;
%   G = GrB (A) ;
%   C0 = sprandsym (A) ;                % the built-in sprandsym
%   C1 = sprandsym (G) ;                % GrB/sprandsym
%   C2 = sprandsym (G, 'normal') ;      % same as sprandsym(G)
%   C3 = sprandsym (G, 'uniform') ;     % uniform distribution
%
%   C = sprandsym (G, 'range', int16 ([-3 6])) ;
%   [i,j,x] = find (C) ;
%   histogram (x, 'BinMethod', 'integers') ;
%
%   C = sprandsym (G, 'uniform', 'range', int16 ([-3 6])) ;
%   [i,j,x] = find (C) ;
%   histogram (x, 'BinMethod', 'integers') ;
%
% See also sprand, sprandn, GrB/sprand, GrB.random.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% make the default 'normal' instead of 'uniform'
have_dist = false ;
for k = 1:nargin-1
    arg = varargin {k} ;
    if (ischar (arg))
        if (isequal (arg, 'uniform') || isequal (arg, 'normal'))
            have_dist = true ;
        end
    end
end
if (~have_dist)
    varargin {end+1} = 'normal' ;
end

C = GrB.random (G, 'symmetric', varargin {:}) ;

