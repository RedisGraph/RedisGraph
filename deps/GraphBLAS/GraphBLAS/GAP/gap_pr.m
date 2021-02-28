function gap_pr
%GAP_PR run pagerank for the GAP benchmark

rng ('default') ;

% warmup, to make sure GrB library is loaded
C = GrB (1) * GrB (1) + 1 ;
clear C

% smaller test matrices:
matrices = { 'HB/west0067', 'LAW/indochina-2004' } ;

% the GAP test matrices:
matrices = {
    'GAP/GAP-kron'
    'GAP/GAP-urand'
    'GAP/GAP-twitter'
    'GAP/GAP-web'
    'GAP/GAP-road'
    } ;

[status, result] = system ('hostname') ;
clear status
if (isequal (result (1:5), 'hyper'))
    fprintf ('hypersparse: %d threads\n', GrB.threads (40)) ;
elseif (isequal (result (1:5), 'slash'))
    fprintf ('slash: %d threads\n', GrB.threads (8)) ;
else
    fprintf ('default: %d threads\n', GrB.threads) ;
end
clear result

for k = 1:length(matrices)

    %---------------------------------------------------------------------------
    % get the GAP problem
    %---------------------------------------------------------------------------

    t1 = tic ;
    clear A Prob d
    Prob = ssget (matrices {k}) ;
    A = GrB (Prob.A, 'by col', 'logical') ;
    n = size (Prob.A,1) ;
    fprintf ('\n%s: nodes: %g million  nvals: %g million\n', ...
        Prob.name, n / 1e6, nnz (Prob.A) / 1e6) ;
    clear Prob
    t1 = toc (t1) ;
    fprintf ('load time: %g sec\n', t1) ;

    t1 = tic ;
    d = GrB.entries (A, 'row', 'degree') ;
    sinks = find (d == 0) ;
    if (length (sinks) > 0)
        d (sinks) = 1 ;
    end
    clear sinks
    d = GrB (d, 'single') ;
    t1 = toc (t1) ;
    fprintf ('degree time: %g sec\n', t1) ;

    ntrials = 16 ;
    % ntrials = 1 ;

    %---------------------------------------------------------------------------
    % PageRank with gap_pagerank
    %---------------------------------------------------------------------------

    fprintf ('\nGAP PageRank tests:\n') ;
    tot = 0 ;
    for trial = 1:ntrials
        tstart = tic ;
        [g, iter] = gap_pagerank (A, d) ;
        t = toc (tstart) ;
        tot = tot + t ;
        fprintf ('trial: %2d GAP pagerank time: %g iter: %d\n', trial, t, iter);
    end
    fprintf ('avg gap_pagerank time:  %g (%d trials)\n', tot/ntrials, ntrials) ;

    clear d

    %---------------------------------------------------------------------------
    % PageRank with GrB.pagerank
    %---------------------------------------------------------------------------

    % Note that GrB.pagerank is slightly different than the GAP pagerank.
    % The GAP benchmark ignores nodes with zero out-degree.  The GrB.pagerank
    % matches the MATLAB @graph/centrality (A, 'pagerank') method, which
    % handles such nodes properly.

    fprintf ('\nGrB PageRank tests:\n') ;
    opts.type = 'single' ;

    tot = 0 ;
    for trial = 1:ntrials
        tstart = tic ;
        [r stats] = GrB.pagerank (A, opts) ;
        t = toc (tstart) ;
        tot = tot + t ;
        fprintf ('trial: %2d GrB.pagerank time: %g = (%g + %g) iter: %d\n', ...
            trial, t, stats.tinit, stats.trank, stats.iter) ;
    end
    fprintf ('avg GrB.pagerank time:  %g (%d trials)\n', tot/ntrials, ntrials) ;

    %---------------------------------------------------------------------------
    % PageRank with MATLAB
    %---------------------------------------------------------------------------

    % if (n < 24*1e6)
    try
        fprintf ('\nCompare with built-in MATLAB pagerank:\n') ;
        A = double (A) ;
        G = digraph (A) ;
        clear A
        tic
        rmatlab = centrality (G, 'pagerank') ;
        t = toc ;
        fprintf ('MATLAB time: %g sec (one trial)\n', t) ;
        clear G

        [r1, i1] = sort (full (double (r))) ;
        [r2, i2] = sort (full (double (rmatlab))) ;
        [r3, i3] = sort (full (double (g))) ;

        for k = 1:10
            fprintf ('rank: %2d GrB: node %8d (%10.4e)', k, i1 (k), r1 (k)) ;
            fprintf (' MATLAB: node %8d (%10.4e)', i2 (k), r2 (k)) ;
            fprintf (' GAP: node %8d (%10.4e)\n', i3 (k), r3 (k)) ;
        end
    catch me
        me
        printf ('MATLAB failed\n') ;
    end

    clear G r g rmatlab d A r1 r2 r3 i1 i2 i3
end

