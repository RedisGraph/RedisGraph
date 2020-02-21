function [t method] = grbresults
%GRBRESULTS return time taken by last GraphBLAS function, and AxB method

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

global GraphBLAS_results
t = GraphBLAS_results (1) ;
method = GraphBLAS_results (2) ;

if method == 0
    method = 'auto' ;
elseif method == 1001 || method == 4
    method = 'Gustavson' ;
elseif method == 1002 || method == 5
    method = 'heap' ;
elseif method == 1003 || method == 6
    method = 'dot' ;
elseif method == 1004
    method = 'hash' ;
elseif method == 1005
    method = 'saxpy' ;
else
    error ('invalid method') ;
end


