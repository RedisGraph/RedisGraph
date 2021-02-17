function t = grbresults
%GRBRESULTS return time taken by last GraphBLAS function

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

global GraphBLAS_results
t = GraphBLAS_results (1) ;

