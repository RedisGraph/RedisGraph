% Test allktruss.m.  Requires ssget.

clear
index = ssget ;
f = find (index.pattern_symmetry == 1) ;
[~,i] = sort (index.nnz (f)) ;
f = f (i) ;

% f = [739 750 2662];
% f = [168 739 750] ; 
% f = [168] 
% f = [ 262 2459 ] ;

matrices = {
'HB/west0067'
'DIMACS10/road_usa'
'SNAP/roadNet-CA'
'McRae/ecology1'
'ND/nd3k'
'ND/nd12k'
'SNAP/soc-LiveJournal1'
'LAW/hollywood-2009'
'LAW/indochina-2004'
'DIMACS10/kron_g500-logn21' } ;

nmat = length (matrices)
skip = [ 1415 ] ;

for kk = 1:nmat

    % get a matrix
    % id = f (kk) ;
    matrix = matrices {kk} ;
    Prob = ssget (matrix, index) ;
    id = Prob.id ;
    A = Prob.A ;
    if (isfield (Prob, 'Zeros')) 
        A = A + Prob.Zeros ;
    end
    fprintf ('\n%4d %4d %-40s\n', kk, id, Prob.name) ;
    clear Prob 

    % make symmetric, binary, and remove the diagonal
    A = spones (A) ;
    A = spones (A + A') ;
    A = A - diag (diag (A)) ;

    n = size (A,1) ;

    % find all the k-trusses
    tstart = tic ;
    % (this is slower):
    % [stats,Cout] = allktruss (A) ;
    stats = allktruss (A) ;
    ttot = toc (tstart) ;

    % print the results
    kmax = stats.kmax ;
    for k = 3:kmax
        ne = stats.nedges (k) ;
        nt = stats.ntri (k) ;
        nsteps  = stats.nsteps (k) ;
        t = stats.time (k) ;
        fprintf ('k %4d sec ne %10d nt %10d nsteps %6d time: %g\n', ...
            k, ne, nt, nsteps, t) ;
    end
    fprintf ('total time: %g\n', ttot) ;
    clear Cout
end

