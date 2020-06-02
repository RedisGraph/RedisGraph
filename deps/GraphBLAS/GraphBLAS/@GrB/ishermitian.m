function s = ishermitian (G, option)
%ISHERMITIAN Determine if a GraphBLAS matrix is Hermitian, or real symmetric.
% ishermitian (G) is true if G equals G' and false otherwise.
% ishermitian (G, 'skew') is true if G equals -G' and false otherwise.
% ishermitian (G, 'nonskew') is the same as ishermitian (G).
%
% See also issymmetric.

% FUTURE: this can be much faster.  See CHOLMOD/MATLAB/spsym.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
if (m ~= n)
    s = false ;
else
    if (nargin < 2)
        option = 'nonskew' ;
    end
    if (islogical (G))
        G = GrB (G, 'double') ;
    end
    if (isequal (option, 'skew'))
        s = (norm (G + G', 1) == 0) ;
    else
        s = (GrB.normdiff (G, G', 1) == 0) ;
    end
    if (s)
        % also check the pattern; G might have explicit zeros
        S = spones (G, 'logical') ;
        s = isequal (S, S') ;
    end
end

