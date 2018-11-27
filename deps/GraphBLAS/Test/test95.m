function test95
%TEST95 performance test for GrB_transpose

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\ntest95 performance tests : GrB_transpose \n') ;
rng ('default') ;

Prob = ssget (2662)
A = Prob.A ;
[m n] = size (A) ;
Cin = sparse (n, m) ;
A (1,2) =1 ;

ntrials = 10 ;

tic
for trial = 1:ntrials
    C1 = A' ;
end
tmsum = toc ;
fprintf ('MATLAB time: %g per trial: %g\n', tmsum, tmsum / ntrials) ;

% C = Cin + A'
for trial = 1:ntrials
    C = GB_mex_transpose (Cin, [ ], 'plus', A) ;
    tg (trial) = gbresults ;
end
tg
tgsum = sum (tg) ;

fprintf ('GraphBLAS time: %g\n', tgsum, tgsum / ntrials) ;
assert (isequal (C1, C.matrix)) ;
fprintf ('speedup over MATLAB: %g\n', tmsum / tgsum) ;

% sum across the rows
yin = sparse (rand (m,1)) ;
fprintf ('row sum:\n') ;
tic
y2 = yin + (sum (A,2)) ;
t1 = toc ;

y = GB_mex_reduce_to_vector (yin, [ ], 'plus', 'plus', A) ;
t2 = gbresults ;
fprintf ('MATLAB: %g GraphBLAS %g speedup %g\n', t1, t2, t1/t2) ;
assert (isequal (y.matrix, y2))
