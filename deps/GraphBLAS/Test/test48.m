function test48
%TEST48 performance test of GrB_mxm

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% d = struct ('inp1', 'tran', 'inp0', 'tran') ;
rng ('default') ;

dt_auto = struct ('inp0', 'tran') ;
dt_dot  = struct ('inp0', 'tran', 'axb', 'dot') ;
dt_gus  = struct ('inp0', 'tran', 'axb', 'gustavson') ;
dt_heap = struct ('inp0', 'tran', 'axb', 'heap') ;

da_auto = struct ;
da_dot  = struct ('axb', 'dot') ;
da_gus  = struct ('axb', 'gustavson') ;
da_heap = struct ('axb', 'heap') ;

dtn_auto = struct ('inp0', 'tran') ;
dtn_dot  = struct ('inp0', 'tran', 'axb', 'dot') ;
dtn_gus  = struct ('inp0', 'tran', 'axb', 'gustavson') ;
dtn_heap = struct ('inp0', 'tran', 'axb', 'heap') ;

dtt_auto = struct ('inp0', 'tran', 'inp1', 'tran') ;
dtt_dot  = struct ('inp0', 'tran', 'inp1', 'tran', 'axb', 'dot') ;
dtt_gus  = struct ('inp0', 'tran', 'inp1', 'tran', 'axb', 'gustavson') ;
dtt_heap = struct ('inp0', 'tran', 'inp1', 'tran', 'axb', 'heap') ;

semiring.multiply = 'times' ;
semiring.add = 'plus' ;
semiring.class = 'double' ;

xnz_list = [0 100 1000 5000 72000 -1] ;

for pp = 0:2

    if (pp == 1)
        Prob = ssget (939)
        A = Prob.A ;
    elseif (pp == 2)
        Prob = ssget (2662)
        A = Prob.A ;
    else
        A = sparse (rand (2000)) ;
    end

    n = size (A,1) ;
    A (1,2) = 1 ;

    for ncols = [1 2 3 4 8 16]

        fprintf ('\n================ ncols %d\n', ncols) ;
        w = sparse (n,ncols) ;

        fprintf ('\n------------------------------ C = A''*x\n') ;
        for xnz = xnz_list % [0 100:200:1000 2000:20000:72000 -1]

            if (xnz == 0)
                x = sparse (n,ncols) ;
                x (1:ncols,1:ncols) = speye (ncols) ;
            elseif (xnz > 0)
                x = sprand (n, ncols, xnz/n) ;
            else
                x = sparse (rand (n, ncols)) ;
            end

            % tic
            ca = GB_mex_mxm (w, [],[], semiring, A, x, dt_auto) ;
            % t = toc ;
            [ta auto_method] = gbresults ;

            % tic
            c1 = GB_mex_mxm (w, [],[], semiring, A, x, dt_dot) ;
            % t = toc ;
            [t method] = gbresults ;
            assert (isequal (method, 'dot')) ;

            % tic
            cg = GB_mex_mxm (w, [],[], semiring, A, x, dt_gus) ;
            % t = toc ;
            [tg method] = gbresults ;
            assert (isequal (method, 'Gustavson')) ;

            % tic
            ch = GB_mex_mxm (w, [],[], semiring, A, x, dt_heap) ;
            % t = toc ;
            [th method] = gbresults ;
            assert (isequal (method, 'heap')) ;

            tic
            c0 = A'*x ;
            t2 = toc ;

            assert (isequal_roundoff (c0, ca.matrix)) ;
            assert (isequal_roundoff (c0, cg.matrix)) ;
            assert (isequal_roundoff (c0, c1.matrix)) ;
            assert (isequal_roundoff (c0, ch.matrix)) ;

            fprintf ('%8d : ', nnz (x)) ;
            fprintf ('auto: %10.4f(%s) dot: %10.4f gus: %10.4f heap: %10.4f MATLAB %10.4f', ...
                ta, auto_method(1), t, tg, th, t2) ;
            fprintf (' speedup auto: %10.2f dot: %10.2f gus: %10.2f heap: %10.2f\n', ...
                t2/ta, t2/t, t2/tg, t2/th) ;

        end

        tic
        c0 = A'*full(x) ;
        toc

        fprintf ('\n------------------------------ C = A*x\n') ;
        for xnz = xnz_list % [0 100:200:1000 2000:20000:72000 -1]

            if (xnz == 0)
                x = sparse (n,ncols) ;
                x (1:ncols,1:ncols) = speye (ncols) ;
            elseif (xnz > 0)
                x = sprand (n, ncols, xnz/n) ;
            else
                x = sparse (rand (n, ncols)) ;
            end

            % tic
            ca = GB_mex_mxm (w, [],[], semiring, A, x, da_auto) ;
            % t = toc ;
            [ta auto_method] = gbresults ;

            % tic
            c1 = GB_mex_mxm (w, [],[], semiring, A, x, da_dot) ;
            % t = toc ;
            [t method] = gbresults ;
            assert (isequal (method, 'dot')) ;

            % tic
            cg = GB_mex_mxm (w, [],[], semiring, A, x, da_gus) ;
            % t = toc ;
            [tg method] = gbresults ;
            assert (isequal (method, 'Gustavson')) ;

            % tic
            ch = GB_mex_mxm (w, [],[], semiring, A, x, da_heap) ;
            % t = toc ;
            [th method] = gbresults ;
            assert (isequal (method, 'heap')) ;

            tic
            c0 = A*x ;
            t2 = toc ;

            assert (isequal_roundoff (c0, ca.matrix)) ;
            assert (isequal_roundoff (c0, cg.matrix)) ;
            assert (isequal_roundoff (c0, c1.matrix)) ;
            assert (isequal_roundoff (c0, ch.matrix)) ;

            fprintf ('%8d : ', nnz (x)) ;
            fprintf ('auto: %10.4f(%s) dot: %10.4f gus: %10.4f heap: %10.4f MATLAB %10.4f', ...
                ta, auto_method(1), t, tg, th, t2) ;
            fprintf (' speedup auto: %10.2f dot: %10.2f gus: %10.2f heap: %10.2f\n', ...
                t2/ta, t2/t, t2/tg, t2/th) ;

        end

        tic
        c0 = A*full(x) ;
        toc

        w = sparse (ncols,n) ;

        fprintf ('\n------------------------------ C = x''*A\n') ;
        for xnz = xnz_list % [0 100:200:1000 2000:20000:72000 -1]

            if (xnz == 0)
                x = sparse (n,ncols) ;
                x (1:ncols,1:ncols) = speye (ncols) ;
            elseif (xnz > 0)
                x = sprand (n, ncols, xnz/n) ;
            else
                x = sparse (rand (n, ncols)) ;
            end

            % tic
            ca = GB_mex_mxm (w, [],[], semiring, x, A, dtn_auto) ;
            % t = toc ;
            [ta auto_method] = gbresults ;

            % tic
            c1 = GB_mex_mxm (w, [],[], semiring, x, A, dtn_dot) ;
            % t = toc ;
            [t method] = gbresults ;
            assert (isequal (method, 'dot')) ;

            % tic
            cg = GB_mex_mxm (w, [],[], semiring, x, A, dtn_gus) ;
            % t = toc ;
            [tg method] = gbresults ;
            assert (isequal (method, 'Gustavson')) ;

            % tic
            ch = GB_mex_mxm (w, [],[], semiring, x, A, dtn_heap) ;
            % t = toc ;
            [th method] = gbresults ;
            assert (isequal (method, 'heap')) ;

            tic
            c0 = x'*A ;
            t2 = toc ;

            % norm (c0 - ca.matrix, 1)
            assert (isequal_roundoff (c0, ca.matrix)) ;
            assert (isequal_roundoff (c0, cg.matrix)) ;
            assert (isequal_roundoff (c0, c1.matrix)) ;
            assert (isequal_roundoff (c0, ch.matrix)) ;

            fprintf ('%8d : ', nnz (x)) ;
            fprintf ('auto: %10.4f(%s) dot: %10.4f gus: %10.4f heap: %10.4f MATLAB %10.4f', ...
                ta, auto_method(1), t, tg, th, t2) ;
            fprintf (' speedup auto: %10.2f dot: %10.2f gus: %10.2f heap: %10.2f\n', ...
                t2/ta, t2/t, t2/tg, t2/th) ;

        end

        tic
        c0 = full(x')*A ;
        toc

        fprintf ('\n------------------------------ C = x''*A''\n') ;
        for xnz = xnz_list % [0 100:200:1000 2000:20000:72000 -1]

            if (xnz == 0)
                x = sparse (n,ncols) ;
                x (1:ncols,1:ncols) = speye (ncols) ;
            elseif (xnz > 0)
                x = sprand (n, ncols, xnz/n) ;
            else
                x = sparse (rand (n, ncols)) ;
            end

            % tic
            ca = GB_mex_mxm (w, [],[], semiring, x, A, dtt_auto) ;
            % t = toc ;
            [ta auto_method] = gbresults ;

            % tic
            c1 = GB_mex_mxm (w, [],[], semiring, x, A, dtt_dot) ;
            % t = toc ;
            [t method] = gbresults ;
            assert (isequal (method, 'dot')) ;

            % tic
            cg = GB_mex_mxm (w, [],[], semiring, x, A, dtt_gus) ;
            % t = toc ;
            [tg method] = gbresults ;
            assert (isequal (method, 'Gustavson')) ;

            % tic
            ch = GB_mex_mxm (w, [],[], semiring, x, A, dtt_heap) ;
            % t = toc ;
            [th method] = gbresults ;
            assert (isequal (method, 'heap')) ;

            tic
            c0 = x'*A' ;
            t2 = toc ;

            assert (isequal_roundoff (c0, ca.matrix)) ;
            assert (isequal_roundoff (c0, cg.matrix)) ;
            assert (isequal_roundoff (c0, c1.matrix)) ;
            assert (isequal_roundoff (c0, ch.matrix)) ;

            fprintf ('%8d : ', nnz (x)) ;
            fprintf ('auto: %10.4f(%s) dot: %10.4f gus: %10.4f heap: %10.4f MATLAB %10.4f', ...
                ta, auto_method(1), t, tg, th, t2) ;
            fprintf (' speedup auto: %10.2f dot: %10.2f gus: %10.2f heap: %10.2f\n', ...
                t2/ta, t2/t, t2/tg, t2/th) ;

        end

        tic
        c0 = full(x')*A' ;
        toc

        w = sparse (n,ncols) ;

        fprintf ('\n------------------------------ C = A''*x''\n') ;
        for xnz = xnz_list % [0 100:200:1000 2000:20000:72000 -1]

            if (xnz == 0)
                x = sparse (n,ncols) ;
                x (1:ncols,1:ncols) = speye (ncols) ;
            elseif (xnz > 0)
                x = sprand (n, ncols, xnz/n) ;
            else
                x = sparse (rand (n, ncols)) ;
            end
            x=x' ;

            % tic
            ca = GB_mex_mxm (w, [],[], semiring, A, x, dtt_auto) ;
            % t = toc ;
            [ta auto_method] = gbresults ;

            % tic
            c1 = GB_mex_mxm (w, [],[], semiring, A, x, dtt_dot) ;
            % t = toc ;
            [t method] = gbresults ;
            assert (isequal (method, 'dot')) ;

            % tic
            cg = GB_mex_mxm (w, [],[], semiring, A, x, dtt_gus) ;
            % t = toc ;
            [tg method] = gbresults ;
            assert (isequal (method, 'Gustavson')) ;

            % tic
            ch = GB_mex_mxm (w, [],[], semiring, A, x, dtt_heap) ;
            % t = toc ;
            [th method] = gbresults ;
            assert (isequal (method, 'heap')) ;

            tic
            c0 = A'*x' ;
            t2 = toc ;

            assert (isequal_roundoff (c0, ca.matrix)) ;
            assert (isequal_roundoff (c0, cg.matrix)) ;
            assert (isequal_roundoff (c0, c1.matrix)) ;
            assert (isequal_roundoff (c0, ch.matrix)) ;

            fprintf ('%8d : ', nnz (x)) ;
            fprintf ('auto: %10.4f(%s) dot: %10.4f gus: %10.4f heap: %10.4f MATLAB %10.4f', ...
                ta, auto_method(1), t, tg, th, t2) ;
            fprintf (' speedup auto: %10.2f dot: %10.2f gus: %10.2f heap: %10.2f\n', ...
                t2/ta, t2/t, t2/tg, t2/th) ;

        end

        tic
        c0 = A'*full(x') ;
        toc

    end
end

fprintf ('\ntest48: all tests passed\n') ;

