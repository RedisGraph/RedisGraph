function test89
%TEST89 performance test of complex A*B

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[save save_chunk] = nthreads_get ;
chunk = 4096 ;
nthreads = feature ('numcores') ;
nthreads_set (nthreads, chunk) ;

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
        % 1003: dot

    for k = [false true]
        GB_builtin_complex_set (k) ;

        if (k)
            % GraphBLAS is fast
            fprintf ('\nbuilt-in GxB_FC64 complex type:\n') ;
        else
            % GraphBLAS is slower than it could be because the complex type is
            % user-defined.
            fprintf ('\nuser-defined Complex type:\n') ;
        end

        % This uses the default method, which selects Gustavson's method:

        C2 = GB_mex_AxB (A, B) ;
        tg = grbresults ;
        err = norm (C1-C2,1) ;
        fprintf ('GraphBLAS %g speedup %g err: %g\n', tg, tm/tg, err) ;

        % these are expected to be slower still; they do not use the default method
        % (Gustavson) which is selected by the auto-strategy.

        C2 = GB_mex_AxB (A, B, 0, 0, 1004) ;
        tg = grbresults ;
        err = norm (C1-C2,1) ;
        fprintf ('GraphBLAS %g speedup %g (hash) err: %g\n', tg, tm/tg, err) ;

        C2 = GB_mex_AxB (A, B, 0, 0, 1003) ;
        tg = grbresults ;
        err = norm (C1-C2,1) ;
        fprintf ('GraphBLAS %g speedup %g (dot) err: %g\n', tg, tm/tg, err) ;

    end

end
nthreads_set (save, save_chunk) ;
