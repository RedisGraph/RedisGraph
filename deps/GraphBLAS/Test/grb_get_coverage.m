function c = grb_get_coverage
%GRB_GET_COVERAGE return current statement coverage

c = 0 ;
try
    global GraphBLAS_debug GraphBLAS_grbcov
    c = sum (GraphBLAS_grbcov > 0) ;
catch
end

