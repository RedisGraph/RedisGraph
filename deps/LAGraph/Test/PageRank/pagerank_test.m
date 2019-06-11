function [r,irank,iters] = pagerank_test (A, filename) ;

if (nargin < 2)
    filename = 'A.mtx' ;
end

tol = 1e-5 ;
itermax = 100 ;

% test the MATLAB version
tic ;
[r,irank,iters] = dpagerank2 (A, tol, itermax) ;
toc 

fprintf ('\ntesting LAGraph_pagerank:\n') ;

% TODO: for now, this requires mread and mwrite from SuiteSparse/CHOLMOD (see
% http://suitesparse.com)  It would be better to write a MATLAB interface to
% LAGraph_mmread and LAGraph_mmwrite, so this test could be self-contained.

outfile = 'rg' ;
mwrite (filename, sparse (A)) ;
system (sprintf ('./build/ptest < %s > %s', filename, outfile)) ; 
load ('rg') ;
irank_grb = rg (:,1) + 1 ;
rank_grb  = rg (:,2) ;

n = size (A,1) ;

iii = [irank irank_grb (irank - irank_grb) (r-rank_grb)] ;
iii (1:min(100,n),:)
err_irank = nnz (irank ~= irank_grb)
err_rank  = norm (r - rank_grb)
