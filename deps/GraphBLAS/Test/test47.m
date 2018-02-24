function test47
%TEST47 prformance test of GrB_vxm

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

d = struct ('inp1', 'tran', 'inp0', 'tran') ;
Prob = ssget (939) ;
% Prob = ssget (2662) ;
A = Prob.A ;

n = size (A,1) ;
A (1,2) = 1 ;

w = sparse (n,1) ;
semiring.multiply = 'times' ;
semiring.add = 'plus' ;
semiring.class = 'double' ;

%{
fprintf ('\nMATLAB C=A'' time:\n') ;
tic
C=A' ;
toc
%}

Xnz = [ ]  ;
T = [ ]

for xnz = [100:100:1000 2000:1000:72000]

    x = sprand (n, 1, xnz/n) ;

    tic
    c1 = GB_mex_vxm (w, [],[], semiring, x, A, [ ]) ;
    t = toc ;

    tic
    c0 = x'*A ;
    t2 = toc ;

    assert (isequal (c0', c1.matrix)) ;

    Xnz = [Xnz nnz(x)] ;
    T = [T t] ;

    fprintf ('%d : %g MATLAB %g speedup %g\n', nnz(x), t, t2, t2/t) ;
end

fprintf ('\ntest47: all tests passed\n') ;

