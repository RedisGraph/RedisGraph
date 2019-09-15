% Test ktruss.m, allktruss.m, and the MATLAB mexFunction interfaces for
% ktruss.c and allktruss.c

clear
mex -largeArrayDims ...
    -I../GraphBLAS/Demo/Include -I../GraphBLAS/Include ...
    ktruss_mex.c ktruss.c ktruss_ntriangles.c

mex -largeArrayDims ...
    -I../GraphBLAS/Demo/Include -I../GraphBLAS/Include ...
    allktruss_mex.c allktruss.c ktruss_ntriangles.c

index = ssget ;
f = find (index.pattern_symmetry == 1) ;
[~,i] = sort (index.nnz (f)) ;
f = f (i) ;

% f = [739 750 2662];
  f = [168 739 750] ; 
% f = [168] 

nmat = length (f)
skip = [ 1415 ] ;

% nmat = 9
for kk = 1:nmat
    id = f (kk) ;
    Prob = ssget (id, index) ;
    A = Prob.A ;
    if (isfield (Prob, 'Zeros')) 
        A = A + Prob.Zeros ;
    end
    fprintf ('\n%4d %4d %-40s\n', kk, id, Prob.name) ;
    clear Prob 
    A = spones (A) ;
    A = spones (A + A') ;
    A = A - diag (diag (A)) ;
    assert (isequal (A, A')) ;

    n = size (A,1) ;
    stats1.kmax = 0 ;
    stats1.time_mex = zeros (1,n+1) ;
    stats1.time_matlab = zeros (1,n+1) ;
    stats1.ntri = zeros (1,n+1) ;
    stats1.nedges = zeros (1,n+1) ;
    stats1.nsteps = zeros (1,n+1) ;

    clear AllC1 AllC2

    % find all the k-trusses
    k = 3 ;
    while (1)

        tic
        [C2,nsteps] = ktruss_mex (A,k) ;
        t2 = toc ;
        stats1.time_mex (k) = t2 ;

        ne = nnz (C2) / 2 ;
        nt = full (sum (sum (C2))) / 6 ;

        stats1.nedges (k) = ne ;
        stats1.ntri (k) = nt ;
        stats1.nsteps (k) = nsteps ;

        fprintf ('k %4d %12.6f sec ne %10d nt %10d ', k, t2, ne, nt) ;

        if (~any (id == skip))
            tic
            C1 = ktruss (A,k) ;
            t1 = toc ;
            stats1.time_matlab (k) = t1 ;
            fprintf ('MATLAB %12.6f sec speedup %7.2f', t1, t1/t2) ;
            assert (isequal (C1, C2)) ;
        end

        fprintf ('\n') ;

        AllC1 {k} = C2 ;

        if (ne == 0)
            break ;
        end

        % assert (spok (C2) == 1)
        clear C1 C2
        k = k + 1 ;
    end

    stats1.kmax = k ;
    stats1.time_mex = stats1.time_mex (1:k) ;
    stats1.time_matlab = stats1.time_matlab (1:k) ;
    stats1.ntri = stats1.ntri (1:k) ;
    stats1.nedges = stats1.nedges (1:k) ;
    stats1.nsteps = stats1.nsteps (1:k) ;

    fprintf ('anyk total steps %6d times: mex %12.6f matlab %12.6f\n', ...
        sum (stats1.nsteps), sum (stats1.time_mex), sum (stats1.time_matlab)) ;

    % find all the k-trusses in MATLAB
    % [stats, AllC2] = allktruss (A) ;
    [stats, AllC2] = allktruss (A) ;

%   for k = 3:max (length (AllC1), length (AllC2))
%       k
%       assert (isequal (AllC1 {k}, AllC2 {k})) ;
%   end

    % assert (isequal (AllC1, AllC2)) ;
    assert (isequal (stats1.kmax, stats.kmax)) ;
    assert (isequal (stats1.nedges, stats.nedges)) ;
    assert (isequal (stats1.ntri, stats.ntri)) ;
    % assert (isequal (stats1.nsteps, stats.nsteps)) ;

    % find all the k-trusses in C
    tic
    [stats3, AllC3] = allktruss_mex (A) ;
    t = toc ;

    fprintf ('allk total steps %6d times: mex %12.6f matlab %12.6f\n', ...
        sum (stats.nsteps), t, sum (stats.time)) ;

%   for k = 1:stats1.kmax-1
%       k
%       c2 = AllC2 {k} ;
%       c3 = AllC3 {k} ;
%       assert (isequal (c2, c3)) ;
%   end

    assert (isequal (AllC2, AllC3)) ;

    % stats1
    % stats3
    assert (isequal (stats.kmax,   stats3.kmax)) ;
    assert (isequal (stats.nedges, stats3.nedges)) ;
    assert (isequal (stats.ntri,   stats3.ntris)) ;
    assert (isequal (stats.nsteps, stats3.nsteps)) ;

end

