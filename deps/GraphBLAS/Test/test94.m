function test94
%TEST94 test pagerank

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

T = load ('../Demo/Matrix/west0067') ;
n = max (max (T (:, 1:2))) + 1 ;
A = sparse (1+T(:,1), 1+T(:,2), T(:,3), n, n) ;

A = spones (A) ;
assert (isequal (A, spones (A)))

n = size (A, 1) ;
A = spones (A + speye (n)) ;

rng default
tic ;
[r, ir] = dpagerank (A) ;
toc

rng default
tic ;
[r2, ir2, iters] = dpagerank2 (A) ;
toc
iters

ir_diff = length (find (ir ~= ir2))

C.matrix = A ;
C.class = 'logical' ;

tic ;
[r3, ir3] = GB_mex_dpagerank (C) ;
toc
t = grbresults

ir_diff = length (find (ir2 ~= ir3))

summary = [((r2-r3)')./(r2')  ir2' ir3' ir2'-ir3'] ; 

C.is_csc = false ;

for method = [0 1001 1002 1003]
    fprintf ('------------------ method: %d\n', method) ;
    tic ;
    [r4, ir4] = GB_mex_dpagerank (C, method) ;
    toc
    t = grbresults
    assert (norm (r4 - r3) < 1e-5) ;
    assert (isequal (ir3, ir4)) ;
end

k = min (300, n) ;
summary (1:k,:) 
