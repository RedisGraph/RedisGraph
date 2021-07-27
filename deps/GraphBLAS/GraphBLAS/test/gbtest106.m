function gbtest106
%GBTEST106 test build

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

rng ('default') ;

A = GrB.build (1:5, 1:5, true, 5, 5, 'xor') ;
assert (isequal (A, logical (speye (5)))) ;

fprintf ('\ngbtest106: all tests passed\n') ;

