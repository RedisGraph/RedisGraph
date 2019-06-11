function c = lcc_graphblas (A)
%LCC_GRAPHBLAS compute LCC in GraphBLAS
% c = lcc_graphblas (A)
mwrite ('A.mtx', A) ;
system ('./build/lcctest < A.mtx > lcc_results') ;
c = load ('lcc_results') ;
delete ('A.mtx') ;
delete ('lcc_results') ;

