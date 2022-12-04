function test01
%TEST01 test GraphBLAS error handling

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

GB_mex_about2 ;
GB_mex_about ;
GB_mex_errors ;
GB_mex_about3 ;
GB_mex_about4 ;
GB_mex_about5 ;
GB_mex_about6 ;
GB_mex_about7 ;
GB_mex_about8 ;
if (~ispc)
    GB_mex_about9 ;
end

fprintf ('\ntest01: all tests passed\n') ;

