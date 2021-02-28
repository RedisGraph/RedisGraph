function c = grb_clear_coverage
%GRB_CLEAR_COVERAGE clear current statement coverage

try
    global GraphBLAS_debug GraphBLAS_grbcov
    GraphBLAS_grbcov (:) = 0 ;
catch
end

