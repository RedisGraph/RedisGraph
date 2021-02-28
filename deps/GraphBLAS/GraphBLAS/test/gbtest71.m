function gbtest71
%GBTEST71 test GrB.selectopinfo

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.


ops = {
    'tril'
    'triu'
    'diag'
    'offdiag'
    '~=0'
    'nonzero'
    '==0'
    'zero'
    '>0'
    'positive'
    '>=0'
    'nonnegative'
    '<0'
    'negative'
    '<=0'
    'nonpositive'
    '~='
    '=='
    '>'
    '>='
    '<'
    '<=' } ;

for k = 1:length(ops)
    GrB.selectopinfo (ops {k}) ;
end

fprintf ('\n\n') ;
GrB.selectopinfo

fprintf ('gbtest71: all tests passed\n') ;

