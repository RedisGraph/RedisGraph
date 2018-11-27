function test89
%TEST89 performance test of complex A*B

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
Prob = ssget (936) 
A = Prob.A ;
% A = sprandn (1000, 1000, 0.3) ;
[i j x] = find (A) ;
n = size (A,1) ;
nz = length (x) ;
x = x .* (rand (nz,1) + 1i * rand (nz,1)) ;
y = x .* (rand (nz,1) + 1i * rand (nz,1)) ;
A = sparse (i,j,x,n,n) ;
B = sparse (i,j,y,n,n) ;
clear x y i j


for do_real = 0:1

    if (do_real)
        fprintf ('real:\n') ;
        A = real (A) ;
        B = real (B) ;
    else
        fprintf ('complex:\n') ;
    end

    fprintf ('start MATLAB\n') ;
    tic
    C1 = A*B ;
    tm = toc ;
    fprintf ('MATLAB %g\n', tm) ;

        % 1001: Gustavson
        % 1002: heap
        % 1003: dot

    % GraphBLAS is slower than MATLAB because the complex type is user-defined.
    % This uses the default method, which selects Gustavson's method:

    C2 = GB_mex_AxB (A, B) ;
    tg = gbresults ;
    err = norm (C1-C2,1)
    fprintf ('GraphBLAS %g slowdown %g\n', tg, tg/tm) ;

    % these are expected to be slower still; they do not use the default method
    % (Gustavson) which is selected by the auto-strategy.

    C2 = GB_mex_AxB (A, B, 0, 0, 1002) ;
    tg = gbresults ;
    err = norm (C1-C2,1)
    fprintf ('GraphBLAS %g slowdown %g (heap)\n', tg, tg/tm) ;


    C2 = GB_mex_AxB (A, B, 0, 0, 1003) ;
    tg = gbresults ;
    err = norm (C1-C2,1)
    fprintf ('GraphBLAS %g slowdown %g (dot)\n', tg, tg/tm) ;

end
