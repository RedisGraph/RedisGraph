function grbcov
%GRBCOV compile, run, and evaluate test coverage

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

clear all
tstart = tic ;
system ('make purge') ;
grbmake ;
testcov ;
grbshow ;
ttotal = toc (tstart) ;

fprintf ('\nTotal time, incl compilation: %8.2f minutes\n', ttotal / 60) ;

