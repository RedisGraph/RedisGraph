function test123
%TEST123 test MIS on large matrix

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test123: test MIS on large matrix\n') ;

Prob = ssget (2662)
A = Prob.A ;

% make symmetric and remove self edges
fprintf ('prep (in MATLAB):\n') ;
tic
A = spones (A) ;
A = A+A' ;
A = tril (A,-1) ;
A = A+A' ;
n = size (A,1) ;
toc

ncores = feature ('numcores') ;

for seed = 1:3
    fprintf ('\n') ;

    for nthreads = [1 2 4 8 16 20 32 40 64 128 160 320]
        if (nthreads > 2*ncores)
            break ;
        end
        nthreads_set (nthreads) ;
        tic
        s = GB_mex_mis (A, seed) ;
        t = toc ;

        p = find (s == 1) ;
        S = A (p,p) ;
        % make sure it's independent
        assert (nnz (S) == 0)

        if (nthreads == 1)
            t1 = t ;
        end

        isize = nnz (s) ;
        fprintf ('%3d threads: %8.4f sec, speedup %6.2f', nthreads, t, t1/t) ;
        fprintf (' iset: %d of %d (%8.2f %%)\n', ...
            isize, n, 100 * isize / n) ;
    end
end

fprintf ('amd run time, for comparison:\n') ;
tic
p = amd (A) ;
toc

fprintf ('test123: all tests passed\n') ;
