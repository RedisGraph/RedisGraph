function logstat (testscript, threads)
%LOGSTAT run a GraphBLAS test and log the results to log.txt 

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[debug, compact, malloc, covered] = GB_mex_debug ;

clast = grb_get_coverage ;

if (nargin < 2)
    % by default, use 4 threads and a tiny chunk size of 1
    threads {1} = [4 1] ;
end

ntrials = length (threads) ;

for trial = 1:ntrials

    nthreads_and_chunk = threads {trial} ;
    nthreads = nthreads_and_chunk (1) ;
    chunk    = nthreads_and_chunk (2) ;
    nthreads_set (nthreads, chunk) ;

    if (nargin == 0)
        f = fopen ('log.txt', 'a') ;
        fprintf (f, '\n----------------------------------------------') ;
        if (debug)
            fprintf (f, ' [debug]') ;
        end
        if (compact)
            fprintf (f, ' [compact]') ;
        end
        if (malloc)
            fprintf (f, ' [malloc]') ;
        end
        if (covered)
            fprintf (f, ' [cover]') ;
        end
        fprintf (f, '\n') ;
        fclose (f) ;
        return
    end

    fprintf ('\n======== test: %-10s ', testscript) ;

    if (debug)
        fprintf (' [debug]') ;
    end
    if (compact)
        fprintf (' [compact]') ;
    end
    if (malloc)
        fprintf (' [malloc]') ;
    end
    if (covered)
        fprintf (' [cover]') ;
    end
    fprintf (' [nthreads: %d chunk: %g]', nthreads, chunk) ;
    fprintf ('\n') ;

    t1 = tic ;
    runtest (testscript)
    t = toc (t1) ;

    f = fopen ('log.txt', 'a') ;

    s = datestr (now) ;
    fprintf (   '%s %-10s %7.1f sec ', s, testscript, t) ;
    fprintf (f, '%s %-10s %7.1f sec ', s, testscript, t) ;

    if (~isempty (strfind (pwd, 'Tcov')))
        global GraphBLAS_debug GraphBLAS_grbcov
        save grbstat GraphBLAS_debug GraphBLAS_grbcov testscript
        if (isempty (GraphBLAS_debug))
            GraphBLAS_debug = false ;
        end
        % fprintf ('malloc debug: %d\n', GraphBLAS_debug) ;
        if (~isempty (GraphBLAS_grbcov))
            c = sum (GraphBLAS_grbcov > 0) ;
            n = length (GraphBLAS_grbcov) ;
            if (c == n)
                fprintf (   'coverage: %5d :   all %5d (full 100%% rate: %8.2f/sec)', ...
                    c - clast, n, (c-clast) / t) ;
                fprintf (f, 'coverage: %5d :   all %5d (full 100%% rate: %8.2f/sec)', ...
                    c - clast, n, (c-clast) / t) ;
            else
                fprintf (   'coverage: %5d : %5d of %5d (%5.1f%% rate: %8.2f/sec)', ...
                    c - clast, c, n, 100 * (c/n), (c-clast) / t) ;
                fprintf (f, 'coverage: %5d : %5d of %5d (%5.1f%% rate: %8.2f/sec)', ...
                    c - clast, c, n, 100 * (c/n), (c-clast) / t) ;
            end
            if (debug)
                fprintf (' [debug]') ;
            end
            if (compact)
                fprintf (' [compact]') ;
            end
            if (malloc)
                fprintf (' [malloc]') ;
            end
            if (covered)
                fprintf (' [cover]') ;
            end
        end
    end

    fprintf (   '\n') ;
    fprintf (f, '\n') ;
    fclose (f) ;
end
