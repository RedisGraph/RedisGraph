function gbtest107
%GBTEST107 test cell2mat error handling

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

rng ('default') ;

ok = true ;
try
    C = GrB.cell2mat ('crud') ; %#ok<*NASGU>
    ok = false ;
catch me
    fprintf ('error expected: %s\n', me.message) ;
end
assert (ok) ;

try
    S = cell (2,2,2) ;
    C = GrB.cell2mat (S) ;
    ok = false ;
catch me
    fprintf ('error expected: %s\n', me.message) ;
end
assert (ok) ;

fprintf ('\ngbtest107: all tests passed\n') ;

