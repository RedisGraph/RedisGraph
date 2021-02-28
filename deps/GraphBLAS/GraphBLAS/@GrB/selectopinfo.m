function selectopinfo (op)
%GRB.SELECTOPINFO list the details of a GraphBLAS select operator.
%
% Usage
%   GrB.selectopinfo
%   GrB.selectopinfo (op)
%
% For GrB.selectop(op), the op must be one of the following strings.
% Some of the operators have equivalent synonyms.
%
%   op                      MATLAB equivalent
%   --------                -----------------
%   'tril'                  C = tril (A,b)
%   'triu'                  C = triu (A,b)
%   'diag'                  C = diag (A,b)
%   'offdiag'               C = entries not in diag(A,b)
%   '~=0' 'nonzero'         C = A (A ~= 0)
%   '==0' 'zero'            C = A (A == 0)
%   '>0'  'positive'        C = A (A >  0)
%   '>=0' 'nonnegative'     C = A (A >= 0)
%   '<0'  'negative'        C = A (A <  0)
%   '<=0' 'nonpositive'     C = A (A <= 0)
%   '~='                    C = A (A ~= b)
%   '=='                    C = A (A == b)
%   '>'                     C = A (A >  b)
%   '>='                    C = A (A >= b)
%   '<'                     C = A (A <  b)
%   '<='                    C = A (A <= b)
%
% All select operators are type-generic, so no '.' appears, as they
% do for other operators.
%
% Example:
%
%   GrB.selectopinfo ;
%   GrB.selectopinfo ('tril') ;
%
% See also GrB.binopinfo, GrB.descriptorinfo, GrB.monoidinfo,
% GrB.semiringinfo, GrB.unopinfo.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    help GrB.selectopinfo
else
    gbselectopinfo (op) ;
end

