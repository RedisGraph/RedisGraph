function C = random (varargin)
%GRB.RANDOM uniformly distributed random GraphBLAS matrix.
% C = GrB.random (A) has the same pattern as A, but with uniformly
%   distributed random entries.  If the same random seed is used,
%   GrB.random (A) and the MATLAB sprand (A) produce the same result.
%
% C = GrB.random (m, n, d) is a random m-by-n GraphBLAS matrix, with
%   about d*m*n uniformly distributed entries.  With the same inputs
%   and same random seed, GrB.random and the MATLAB built-in sprand
%   produce the same pattern, but the values differ.  The entries are
%   constructed by computing d*m*n entries at random positions, and
%   then any duplicates are discarded, so if d is large or m*n is
%   small, then C will have fewer than d*m*n entries.  The value of
%   d may exceed 1 (this differs from the MATLAB sprand, which limits
%   d to 1).  If d is inf, then C is generated as a full GraphBLAS
%   matrix, with numel (C) = m*n.
%
% Optional parameters may be used, in any order, after the A or m,n,d
% arguments:
%
%   C = GrB.random (..., 'uniform') uses a uniform distribution
%           of values, with entries greater than zero and less than one.
%
%   C = GrB.random (..., 'normal') uses a normal distribution, like
%           the built-in MATLAB sprandn.
%
%   C = GrB.random (..., 'range', [lo hi]) changes the range of
%           the random numbers.  If 'range' is not present, the default
%           is double ([0 1]).  The class of [lo hi] determines the type
%           of the random matrix C.  If [lo hi] is logical, all entries
%           in the pattern are true.  If [lo hi] is 'double' or 'single',
%           then if the random number generator (rand or randn) produces
%           a double random value of x, it is scaled to (hi-lo)*x+lo.  If
%           [lo hi] is integer, then x becomes floor ((hi-lo+1)*x + lo),
%           which is then typecasted to the requested. integer type. This
%           scaling applies to both the 'uniform' and 'normal' distribution.  
%           With the normal distribution, [lo hi] specifies the mean (lo)
%           and the standard deviation (hi) of the final distribution.
%
%   C = GrB.random (A, 'symmetric') creates a symmetric matrix, like
%           the built-in C = sprandsym (A), except that the default
%           distribution is 'uniform'.  The input matrix A must be
%           square.  Only tril(A) is used to construct C.
%
%   C = GrB.random (n, d, 'symmetric') creates an n-by-n symmetric
%           matrix C, with a uniform distribution of values.  To create
%           a matrix like C = srandsym (n,d) with the built-in MATLAB
%           sprandym, use C = GrB.random (n, d, 'symmetric', 'normal').
%
% The rc option of the built-in MATLAB sprand and sprandn is not supported.
%
% Example:
%
%   rng ('default')
%   A = sprand (4, 5, 0.5)          % 4-by-5 with at most 10 entries
%   rng ('default')
%   C = GrB.random (4, 5, 0.5)      % same pattern as A
%   assert (isequal (spones (A), spones (C)))
%
%   C = GrB.random (C)              % same pattern but newly random values
%
%   C = GrB.random (2, 4, inf)      % 2-by-4 with all 8 entries
%   C = GrB.random (2, 4, 0.5, 'normal')    % like sprandn (2,4,0.5)
%   
%   % random 10-by-10 int16 matrix with entries from -3 to 6,
%   % including explicit zeros, with a uniform distribution
%   C = GrB.random (10, 10, 0.5, 'range', int16 ([-3 6]))
%
%   % larger matrix, with normal distribution:
%   C = GrB.random (1000, 1000, 0.5, 'normal', 'range', int16 ([-3 6])) ;
%   [i,j,x] = find (C) ;
%   histogram (x, 'BinMethod', 'integers') ;
%
%   % lots of explicit zeros, since uint8 saturates:
%   C = GrB.random (1000, 1000, 0.5, 'normal', 'range', uint8 ([-3 6])) ;
%   [i,j,x] = find (C) ;
%   histogram (x, 'BinMethod', 'integers') ;
%
%   % uniform distribution across all possible uint8 values
%   C = GrB.random (1000, 1000, 0.5, 'range', uint8 ([0 255])) ;
%   [i,j,x] = find (C) ;
%   histogram (x, 'BinMethod', 'integers') ;
%
%   % large symmetric matrix with normal distribution
%   C1 = GrB.random (1e6, 1e-5, 'symmetric', 'normal')
%   [i,j,x] = find (C) ;
%   histogram (x)
%
% See also sprand, sprandn, sprandsym, GrB/sprand, GrB/sprandn, GrB/sprandsym.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% defaults
dist = 'uniform' ;
type = 'double' ;
range = [ ] ;
sym_option = 'unsymmetric' ;
firstchar = nargin + 1 ;

% parse input options
for k = 1:nargin
    arg = varargin {k} ;
    if (ischar (arg))
        firstchar = min (firstchar, k) ;
        switch arg
            case { 'uniform', 'normal' }
                dist = arg ;
            case 'range'
                range = varargin {k+1} ;
                type = GrB.type (range) ;
            case { 'unsymmetric', 'symmetric' }
                sym_option = arg ;
            otherwise
                gb_error ('unknown option') ;
        end
    end
end

symmetric = isequal (sym_option, 'symmetric') ;
desc.base = 'zero-based' ;

% construct the pattern
if (firstchar == 2)
    % C = GrB.random (A, ...) ;
    A = varargin {1} ;
    [m, n] = size (A) ;
    if (symmetric && (m ~= n))
        gb_error ('input matrix must be square') ;
    end
    [I, J] = GrB.extracttuples (A, desc) ;
    e = length (I) ;
elseif (firstchar == (4 - symmetric))
    % C = GrB.random (m, n, d, ...)
    % C = GrB.random (n, d, ... 'symmetric')
    m = varargin {1} ;
    if (symmetric)
        n = m ;
        d = varargin {2} ;
    else
        n = varargin {2} ;
        d = varargin {3} ;
    end
    if (isinf (d))
        e = m * n ;
        I = repmat ((int64 (0) : int64 (m-1)), 1, n) ;
        J = repmat ((int64 (0) : int64 (n-1)), m, 1) ;
    else
        e = round (m * n * d) ;
        I = int64 (floor (rand (e, 1) * m)) ;
        J = int64 (floor (rand (e, 1) * n)) ;
    end
else
    gb_error ('invalid usage') ;
end

% construct the values
if (islogical (range))
    X = true ;
else
    if (isequal (dist, 'uniform'))
        X = rand (e, 1) ;
    else
        X = randn (e, 1) ;
    end
    if (~isempty (range))
        lo = min (double (range)) ;
        hi = max (double (range)) ;
        if (isinteger (range))
            X = cast (floor ((hi - lo + 1) * X + lo), type) ;
        else
            X = cast ((hi - lo) * X + lo, type) ;
        end
    end
end

% build the matrix
C = GrB.build (I, J, X, m, n, '2nd', desc) ;

% make it symmetric, if requested
if (symmetric)
    C = tril (C) + tril (C, -1)' ;
end

