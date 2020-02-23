%GAP run GAP benchmarks

clear all
rng ('default') ;

matrices = {
    'GAP/GAP-kron'
    'GAP/GAP-urand'
    'GAP/GAP-twitter'
    'GAP/GAP-web'
    'GAP/GAP-road' } ;

matrices = { 'HB/west0067' } ;
matrices = { 'HB/west0067', 'LAW/indochina-2004' } ;

for k = 1:length(matrices)

    %---------------------------------------------------------------------------
    % get the GAP problem
    %---------------------------------------------------------------------------

    Prob = ssget (matrices {k}) ;
    A_col = GrB (Prob.A, 'by col', 'int32') ;
    A_row = GrB (Prob.A, 'by row', 'int32') ;
    n = size (Prob.A,1) ;
    try
        sources = Prob.aux.sources' ;
    catch
        sources = randperm (n, 64) ;
    end
    fprintf ('\n%s: nodes: %g million  nvals: %g million\n', ...
        Prob.name, n / 1e6, nnz (Prob.A) / 1e6) ;

    % fix west0067
    if (n == 67)
        A_col = spones (A_col) ;
        A_row = spones (A_row) ;
    end

    clear Prob

    %---------------------------------------------------------------------------
    % BFS
    %---------------------------------------------------------------------------

    fprintf ('\nBFS tests:\n') ;
    ntrials = length (sources) ;
    tot = 0 ;
    for s = sources
        tic
        [v pi] = GrB.bfs (A_row, s) ;
        t = toc ;
        tot = tot + t ;
        fprintf ('source: %8d  #visited %8d  levels: %8d time: %g\n', ...
            s, nnz (v), max (v), t) ;
    end
    fprintf ('average bfs time: %g (%d trials)\n', tot / ntrials, ntrials) ;
    if (n < 1000)
        pi (s) = 0 ;
        treeplot (double (pi))
    end

    %---------------------------------------------------------------------------
    % PageRank
    %---------------------------------------------------------------------------

    % Note that the GrB.pagerank is slightly different than the GAP pagerank.
    % The GAP benchmark ignores nodes with zero out-degree.  The GrB.pagerank
    % matches the MATLAB @graph/centrality (A, 'pagerank') method, which
    % handles such nodes properly.

    fprintf ('\nPageRank tests:\n') ;
    ntrials = length (sources) ;
    opts.type = 'single' ;
    ntrials = 1 ; % 16 ;
    tot = 0 ;
    for trial = 1:ntrials
        tic
        r = GrB.pagerank (A_col, opts) ;
        t = toc ;
        tot = tot + t ;
        fprintf ('pagerank time: %g\n', t) ;
    end
    fprintf ('average pagerank time: %g (%d trials)\n', tot/ntrials, ntrials) ;

    if (n < 8*1e6)
        fprintf ('\nCompare with built-in MATLAB pagerank:\n') ;
        G = digraph (double (A_col)) ;
        tic
        rmatlab = centrality (G, 'pagerank') ;
        t = toc ;
        fprintf ('MATLAB time: %g sec (one trial)\n', t) ;

        [r1, i1] = sort (full (double (r))) ;
        [r2, i2] = sort (full (double (rmatlab))) ;

        for k = 1:10
            fprintf ('rank: %2d GrB: node %5d (%10.4e)', k, i1 (k), r1 (k)) ;
            fprintf (' MATLAB: node %5d (%10.4e)\n', i2 (k), r2 (k)) ;
        end
    end

    clear r rmatlab

    %---------------------------------------------------------------------------
    % triangle count
    %---------------------------------------------------------------------------

    fprintf ('\nTriangle Count tests:\n') ;

    % matrix must be symmetric
    S = GrB (A_row + A_row', 'logical') ;

    ntrials = 1 ; % 3 ;
    tot = 0 ;
    for trials = 1:ntrials
        tic
        c = GrB.tricount (S, s) ;
        t = toc ;
        tot = tot + t ;
        fprintf ('# of triangles: %8d time: %g\n', c, t) ;
    end
    fprintf ('average tricount time: %g (%d trials)\n', ...
        tot / ntrials, ntrials) ;

    clear S

end

