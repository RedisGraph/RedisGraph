function A = GB_spec_random (m, n, d, scale, class, is_csc,is_hyper,hyper_ratio)
%GB_SPEC_RANDOM generate random matrix
%
% A = GB_spec_random (m, n, d, scale, class, is_csc, is_hyper, hyper_ratio)
%
% m,n,d: parameters to sprandn (m,n,d)
% m,n: defaults to 4
% d: defaults to 0.5
% scale: a double scalar, defaults to 1.0
% class: a string; see "help classid". defaults to 'double'
% is_csc: true for CSC, false for CSR; defaults to true
% is_hyper: false for non-hypersparse, true for hypersparse, default false

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 1)
    m = 4 ;
end

if (nargin < 2)
    n = 4 ;
end

if (nargin < 3)
    d = 0.5 ;
end

if (nargin < 4)
    scale = 1 ;
end

if (nargin < 5)
    class = 'double' ;
end

if (nargin >= 6)
    A.is_csc = is_csc ;
end

if (nargin >= 7 && ~isempty (is_hyper))
    A.is_hyper = is_hyper ;
end

if (nargin >= 8)
    A.hyper_ratio = hyper_ratio ;
end

A.matrix = scale * sprandn (m, n, d) ;
A.class = class ;
A.pattern = logical (spones (A.matrix)) ;

