function gap_tc
%GAP_TC run tricount for the GAP benchmark

diary on
rng ('default') ;

% warmup, to make sure GrB library is loaded
C = GrB (1) * GrB (1) + 1 ;
clear C

% the GAP test matrices:
matrices = {
    'GAP/GAP-road'
    'GAP/GAP-web'
    'GAP/GAP-urand'
    'GAP/GAP-twitter'
    'GAP/GAP-kron'
    } ;

% smaller test matrices:
matrices = { 'HB/west0067', 'SNAP/roadNet-CA', ...
    'GAP/GAP-road', ...
    'GAP/GAP-web', ...
    'GAP/GAP-urand', ...
    'GAP/GAP-twitter', ...
    'GAP/GAP-kron' }

matrices = { 'HB/west0067', 'SNAP/roadNet-CA' , ...
    'SNAP/com-Orkut', 'LAW/indochina-2004' }

index = ssget ;
f = find (index.nrows == index.ncols & index.nnz > 5e6 & index.isReal) ;
[~,i] = sort (index.nnz (f)) ;
matrices = f (i) ;

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

% winners = zeros (16,1) ;  
% total   = zeros (16,1) ;  
% tbest   = 0 ;

for k = 152:length(matrices)

    %---------------------------------------------------------------------------
    % get the GAP problem
    %---------------------------------------------------------------------------

try

    id = matrices (k) ;
    GrB.burble (0) ;
    t1 = tic ;
    clear A Prob
    Prob = ssget (id, index) ;
    A = GrB (Prob.A, 'by row', 'logical') ;
    name = Prob.name ;
    clear Prob
    A = spones (A) ;
    A = A|A' ;
    n = size (A,1) ;
    fprintf ('\n%s: nodes: %g million  nvals: %g million\n', ...
        name, n / 1e6, nnz (A) / 1e6) ;
    t1 = toc (t1) ;
    fprintf ('load time: %g sec\n', t1) ;

    ntrials = 1 ;

    %---------------------------------------------------------------------------
    % triangle count
    %---------------------------------------------------------------------------

    fprintf ('\nGrB.tricount  tests:\n') ;

    tot = 0 ;
    for trial = 1:ntrials
        tstart = tic ;
        s = GrB.tricount (A) ;
        t = toc (tstart) ;
        tot = tot + t ;
        fprintf ('trial: %2d GrB.tricount  time: %8.3f\n', trial, t) ;
    end
    fprintf ('avg GrB.tricount time:  %10.3f (%d trials)\n', ...
        tot/ntrials, ntrials) ;
    fprintf ('triangles: %d\n', full (s)) ;

    %---------------------------------------------------------------------------
    % triangle count with permutations
    %---------------------------------------------------------------------------

    [c times best] = tric (A, s) ;
    clear A

    all_times = sum (times, 2) ;
    total = total + all_times ;
    winners (best) = winners (best) + 1 ;
    tbest = tbest + all_times (best) ;

    for k = 1:16
        if (total (k) < inf)
            fprintf ('%2d   %10.3f : %d\n', k, total (k), winners (k)) ;
        end
    end
    fprintf ('best %10.3f\n', tbest) ;
    save gap_tc_results winners total tbest k
    diary off
    diary on

catch me
    k
    disp (me.message)
end

end


