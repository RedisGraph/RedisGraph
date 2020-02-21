function gbtest97
%GBTEST97 test A*x performance

max_nthreads = GrB.threads ;
threads = [1 2 4 8 16 20 32 40 64] ;

n = 1e5 ;
nz = 2e6 ;
d = nz / n^2 ;
G = GrB.random (n,n,d) ;
A = double (G) ;
% warmup to make sure the GrB library is loaded
y = GrB (rand (2)) * GrB (rand (2)) ;

ntrials = 10 ;

for test = 1:4

    if (test == 1)
        X = 'sparse (rand (n,1))' ;
        x =  sparse (rand (n,1)) ;
    elseif (test == 2)
        X = 'rand (n,1)' ;
        x =  rand (n,1) ;
    elseif (test == 3)
        X = 'sprand (n,1,0.5)' ;
        x =  sprand (n,1,0.5) ;
    else
        X = 'sprand (n,1,0.05)' ;
        x =  sprand (n,1,0.05) ;
    end

    fprintf ('\n\n========================\n') ;
    fprintf ('in MATLAB: y = A*x where x = %s\n', X) ;

    tic
    for trial = 1:ntrials
        y = A*x ;
    end
    tmatlab = toc ;
    fprintf ('MATLAB time: %8.4f sec\n', tmatlab) ;
    ymatlab = y ;

    fprintf ('\nGrB: y = A*x where x = %s\n', X) ;

    for nthreads = threads
        if (nthreads > max_nthreads)
            break ;
        end
        GrB.threads (nthreads) ;
        tic
        for trial = 1:ntrials
            y = G*x ;
        end
        t = toc ;
        if (nthreads == 1)
            t1 = t ;
        end
        fprintf (...
            'threads: %2d GrB time: %8.4f speedup vs MATLAB: %8.2f  vs: GrB(1 thread) %8.2f\n', ...
            nthreads, t, tmatlab / t, t1 / t) ;
        assert (norm (y-ymatlab, 1) / norm (ymatlab,1) < 1e-12)
    end

end
