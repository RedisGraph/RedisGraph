function gb_error (varargin)
%GB_ERROR report an error

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

error ('GrB:error', varargin {:}) ;

