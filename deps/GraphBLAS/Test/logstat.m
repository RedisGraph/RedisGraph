function logstat (testscript)
%LOGSTAT run a GraphBLAS test and log the results to log.txt 

%  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
%  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[debug, compact, malloc, covered] = GB_mex_debug ;

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

fprintf ('\n======== test: %-8s ', testscript) ;

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
fprintf ('\n') ;

t1 = cputime ;
runtest (testscript)
t = cputime - t1 ;

f = fopen ('log.txt', 'a') ;

s = datestr (now) ;
fprintf (   '%s %-7s %7.1f sec ', s, testscript, t) ;
fprintf (f, '%s %-7s %7.1f sec ', s, testscript, t) ;

if (~isempty (strfind (pwd, 'Tcov')))
    global GraphBLAS_debug GraphBLAS_gbcov
    save gbstat GraphBLAS_debug GraphBLAS_gbcov testscript
    if (isempty (GraphBLAS_debug))
        GraphBLAS_debug = false ;
    end
    % fprintf ('malloc debug: %d\n', GraphBLAS_debug) ;
    if (~isempty (GraphBLAS_gbcov))
        c = sum (GraphBLAS_gbcov > 0) ;
        n = length (GraphBLAS_gbcov) ;
        fprintf (   'coverage: %5d of %5d (%5.1f%%)', c, n, 100 * (c/n)) ;
        fprintf (f, 'coverage: %5d of %5d (%5.1f%%)', c, n, 100 * (c/n)) ;
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

