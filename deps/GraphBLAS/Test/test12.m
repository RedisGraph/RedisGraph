function test12 (cover)
%TEST12 test Wathen matrix generation
%
% Usage: test12(cover)
%
% if cover=1, do quick statement coverage tests
% if cover=0, run larger problems

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 1)
    cover = 1 ;
end

if (cover)
    nn = [20] ;
else
    nn = [200 400 800] ;
end

rng ('default') ;

A = GB_mex_wathen (2,2) ;
assert (spok (A) == 1) ;
assert (nnz (A-A') == 0) ;

% this test is too slow when debugging
debug = GB_mex_debug ;

fprintf ('\nWathen matrices:\n') ;

    for nx = [1 5 10] % 1:20
        fprintf ('%d:', nx) ;
        for ny = [1 5 10] % 1:20
            fprintf ('%d', ny) ;

            for scale = 0:1
                % reset the random number generator so RHO can be found
                rng ('default') ;
                rho = 100 * rand (nx,ny) ;
                rng ('default') ;
                A = gallery ('wathen' ,nx, ny, scale) ;
                anorm = norm (A,1) ;
                for method = 0:3
                    fprintf ('.') ;
                    B = GB_mex_wathen (nx, ny, method, scale, rho) ;
                    assert (norm (A-B,1) < 16 * eps (norm (A,1))) ;
                end
            end
        end
    end

    for nx = nn
        for ny = nn
            rho = 100 * rand (nx,ny) ;

            for scale = 0:1
                fprintf ('\n') ;
                % reset the random number generator so RHO can be found
                % and given to GB_mex_wathen.m
                rng ('default') ;
                rho = 100 * rand (nx,ny) ;
                rng ('default') ;
                tic
                A = gallery ('wathen' ,nx, ny, scale) ;
                t1 = toc ;
                n = size (A,1) ;
                nz = nnz (A) ;
                for method = 0:3
                    tic
                    B = GB_mex_wathen (nx, ny, method, scale, rho) ;
                    t2 = toc ;
                    assert (norm (A-B,1) < 16 * eps (norm (A,1))) ;
                    fprintf ('nx %d ny %d n %d nz %d MATLAB %10.4f GB %10.4f speedup %g\n', ...
                    nx, ny, n, nz, t1, t2, t1/t2) ;
                end
            end
        end
    end

% end

fprintf ('test12: all tests passed\n') ;

