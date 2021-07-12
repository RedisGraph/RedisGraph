function gbtest71
%GBTEST71 test GrB.selectopinfo

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

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

nops = length (ops) ;
for k = 1:nops
    GrB.selectopinfo (ops {k}) ;
end

fprintf ('\n\n') ;
GrB.selectopinfo

fprintf ('# of select ops: %d\n', nops) ;
fprintf ('gbtest71: all tests passed\n') ;

