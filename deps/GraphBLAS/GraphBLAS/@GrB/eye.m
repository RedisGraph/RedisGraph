function C = eye (varargin)
%GRB.EYE Sparse identity matrix, of any type supported by GraphBLAS.
% C = GrB.eye (n) creates a sparse n-by-n identity matrix of type
% 'double'.
%
% C = GrB.eye (m,n) or GrB.eye ([m n]) is an m-by-n identity matrix.
%
% C = GrB.eye (m,n,type) or GrB.eye ([m n],type) creates a sparse m-by-n
% identity matrix C of the given GraphBLAS type, either 'double',
% 'single', 'logical', 'int8', 'int16', 'int32', 'int64', 'uint8',
% 'uint16', 'uint32', 'uint64', or (in the future) 'complex'.
%
% See also spones, spdiags, speye, GrB.speye, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% get the type
type = 'double' ;
nargs = nargin ;
if (nargs > 1 && ischar (varargin {nargs}))
    type = varargin {nargs} ;
    nargs = nargs - 1 ;
end

% get the size
if (nargs == 0)
    m = 1 ;
    n = 1 ;
elseif (nargs == 1)
    % C = GrB.eye (n) or GrB.eye ([m n])
    arg1 = varargin {1} ;
    if (length (arg1) == 1)
        % C = GrB.eye (n)
        m = arg1 ;
        n = m ;
    elseif (length (arg1) == 2)
        % C = GrB.eye ([m n])
        m = arg1 (1) ;
        n = arg1 (2) ;
    else
        error ('GrB:unsupported', 'only 2D arrays supported') ;
    end
elseif (nargs == 2)
    % C = GrB.eye (m,n)
    m = varargin {1} ;
    n = varargin {2} ;
else
    error ('GrB:unsupported', 'only 2D arrays supported') ;
end

% construct the m-by-n identity matrix of the given type
m = max (m, 0) ;
n = max (n, 0) ;
mn = min (m, n) ;
I = int64 (0) : int64 (mn-1) ;
desc.base = 'zero-based' ;
C = GrB.build (I, I, ones (mn, 1, type), m, n, '1st', type, desc) ;

