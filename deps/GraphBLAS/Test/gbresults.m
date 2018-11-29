function [t method] = gbresults
%GBRESULTS return time taken by last GraphBLAS function, and AxB method
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
else
    error ('invalid method') ;
end


