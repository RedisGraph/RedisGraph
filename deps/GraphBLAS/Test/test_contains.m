function s = test_contains (text, pattern)
%TEST_CONTAINS same as contains (text, pattern)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

s = ~isempty (strfind (text, pattern)) ;
