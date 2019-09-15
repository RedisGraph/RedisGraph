
clear

[T_grb, Tprep_grb, N, Nedges, Ntri] = tri_grb_results ;

% T_grb = (1:nmat, 1:2)
% T_grb = (id, 1): outer product method, prep is Tprep_grb (id,1)
% T_grb = (id, 2): doe product method,   prep is Tprep_grb (id,2)

% T_grb_total = (id, 1): outer product method, incl prep
% T_grb_total = (id, 2): doe product method,   incl prep
T_grb_total = T_grb + Tprep_grb ;

[T, Tprep, N2, Nedges2, Ntri2, File] = tri_results ;

assert (isequal (N, N2))
assert (isequal (Nedges, Nedges2))
assert (isequal (Ntri, Ntri2))
clear N2 Nedges2 Ntri2

nmat = length (N) ;

% collect the tri_main results

% find the # of threads used

Time = T {1} ;
nthreads_max = size (Time, 3)
threads = [ ] ;
for nth = 1:nthreads_max
    if (all (all (all (~isnan (Time (1:4,:,nth))))))
        threads = [threads nth] ;
    end
end

threads

% sort by # edges
% [ignore, esort] = sort (Nedges) ;
% esort = 1:nmat ;
esort = elist ;

% use only prep method 1:L (which is prep:0 in tri_main.c)

T_total  = nan (nmat, 4, length (threads)) ;
T_best   = nan (nmat, 4) ;
T_simple = nan (nmat, 1) ;

for id = 1:nmat

    Time   = T {id} ;
    T_prep = Tprep {id} ;
    for kth = 1:length(threads)
        nth = threads (kth) ;
        tprep = T_prep (1,1,nth) ;
        % 1:mark
        T_total (id, 1, kth) = Time (1, 1, nth) + tprep ;
        % 2:bit
        T_total (id, 2, kth) = Time (2, 1, nth) + tprep ;
        % 3:dot
        T_total (id, 3, kth) = Time (3, 1, nth) + tprep + T_prep (2, 1, nth) ;
        % 4:logmark
        T_total (id, 4, kth) = Time (4, 1, nth) + tprep ;
    end

    % fastest for all threads
    T_best (id, 1) = min (T_total (id, 1, :)) ; % 1:mark
    T_best (id, 2) = min (T_total (id, 2, :)) ; % 2:bit
    T_best (id, 3) = min (T_total (id, 3, :)) ; % 3:dot
    T_best (id, 4) = min (T_total (id, 4, :)) ; % 4:logmark

    % 5:simple
    tprep = T_prep (1,1,1) ;
    T_simple (id) = Time (5, 1, 1) + tprep ;
end

fprintf ('rate (1 thread):\n') ;
fprintf ('outer:GrB | simple mark bit logm | dot:GrB dot | ') ;
fprintf ('parallel mark bit logm dot\n') ;

% for id = 1:nmat
for kkk = 1:length (esort)
    id = esort (kkk) ;
    e = Nedges (id) ;
    if (e < 2e5)
        continue ;
    end

    file = fixmawi (File {id}) ;

%   fprintf ('%3d %30s %10d %12d ', id, file, N(id), Nedges(id)) ;
    what = kkk ;
    % what = id ;
    fprintf ('%3d& %24s & %6.2f & %7.2f ', ...
        what, file, N(id)/1e6, Nedges(id)/1e6) ;

    [tbest, i] = min ([
        (T_grb_total (id, 1)),      % outer, GrB
        (T_simple (id)),            % simple
        (T_total (id, 1, 1)),       % mark
        (T_total (id, 2, 1)),       % bit
        (T_total (id, 4, 1))        % logmark
        (T_grb_total (id, 2)),      % dot, GrB
        (T_total (id, 3, 1))        % dot
        ]) ;

    fprintf (fbest(i,1), 1e-6*e/T_grb_total (id, 1)) ;    % outer, GrB
    if (T_simple (id) > 1e20)
        fprintf (' &         -    ') ;                  % simple (failure)
    else
        fprintf (fbest(i,2), 1e-6*e/T_simple (id)) ;          % simple
    end
    fprintf (fbest(i,3), 1e-6*e/T_total (id, 1, 1)) ;     % mark
    fprintf (fbest(i,4), 1e-6*e/T_total (id, 2, 1)) ;     % bit
    fprintf (fbest(i,5), 1e-6*e/T_total (id, 4, 1)) ;     % logmark
    fprintf (fbest(i,6), 1e-6*e/T_grb_total (id, 2)) ;    % dot, GrB
    fprintf (fbest(i,7), 1e-6*e/T_total (id, 3, 1)) ;     % dot

%     fprintf ('\n') ;
% end

% fprintf ('rate (best of all threads):\n') ;
% fprintf ('outer: mark bit logm | dot\n') ;
% for id = 1:nmat
%   e = Nedges (id) ;
%   fprintf ('%3d: %40s ', id, File {id}) ;

    [ibest, i] = min (T_best (id, [1 2 4 3])) ;

    fprintf (fbest (i,1), 1e-6*e/T_best (id, 1)) ; % mark
    fprintf (fbest (i,2), 1e-6*e/T_best (id, 2)) ; % bit
    fprintf (fbest (i,3), 1e-6*e/T_best (id, 4)) ; % logmark
    fprintf (fbest (i,4), 1e-6*e/T_best (id, 3)) ; % dot

    fprintf ('\\\\ \n') ;
end

