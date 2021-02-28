function gap_bfs
%GAP_BFS run bfs for the GAP benchmark

rng ('default') ;

% warmup, to make sure GrB library is loaded
C = GrB (1) * GrB (1) + 1 ;
clear C

% smaller test matrices:
% matrices = { 'HB/west0067', 'LAW/indochina-2004' } ;
matrices = { 'HB/west0067' } ;
matrices = { 'LAW/indochina-2004' } ;

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
    A = GrB (Prob.A, 'by row', 'logical') ;
    n = size (Prob.A,1) ;
    try
        sources = Prob.aux.sources ;
    catch
        sources = randperm (n, 64) ;
    end
    fprintf ('\n%s: nodes: %g million  nvals: %g million\n', ...
        Prob.name, n / 1e6, nnz (Prob.A) / 1e6) ;
    clear Prob
    t1 = toc (t1) ;
    fprintf ('load time: %g sec\n', t1) ;

    ntrials = length (sources) ;

    %---------------------------------------------------------------------------
    % BFS with GrB.bfs
    %---------------------------------------------------------------------------

    fprintf ('\nGrB.bfs  tests:\n') ;

    tot = 0 ;
    for trial = 1:ntrials
        s = sources (trial) ;
        tstart = tic ;
        [v, parent] = GrB.bfs (A, s) ;
        % v = GrB.bfs (A, s) ;
        t = toc (tstart) ;
        tot = tot + t ;
        fprintf ('trial: %2d source: %8d GrB.bfs  time: %8.3f ', trial, s, t) ;
        fprintf ('visited: %8d depth: %8d\n', nnz (v), max (v)) ;
        % pause
    end
    fprintf ('avg GrB.bfs  time:  %g (%d trials)\n', tot/ntrials, ntrials) ;

    %---------------------------------------------------------------------------
    % BFS with MATLAB
    %---------------------------------------------------------------------------

    % if (n < 24*1e6)
    try
        fprintf ('\nCompare with built-in MATLAB bfs:\n') ;
        A = GrB (A, 'by col') ;
        A = double (A) ;
        G = digraph (A) ;
        clear A

        tot = 0 ;
        for trial = 1:ntrials
            s = sources (trial) ;
            tstart = tic ;
            [table, edgetonew] = bfsearch (G, s, 'edgetonew') ;
            % [nodes] = bfsearch (G, s) ;
            t = toc (tstart) ;
            tot = tot + t ;
            fprintf ('trial: %2d source: %8d GrB.bfs  time: %8.3f ', ...
                trial, s, t) ;
            fprintf ('visited: %8d\n', 1 + size (table, 1)) ;
        end
        fprintf ('avg bfsearch time:  %g (%d trials)\n', tot/ntrials, ntrials) ;

        clear G

    catch me
        me
        printf ('MATLAB failed\n') ;
    end

    % clear G table parent v nodes edgetonew
end

