
clear

% get file names and graph sizes
[~, ~, N, Nedges, ~, File] = tri_results ;
nmat = length (File)

% load all results
[T_allktruss_grb, File2,  N2,  Nedges2,  Kmax] = allktruss_grb_results ;

nmat1 = length (N2) ;
% assert (isequal (N (1:nmat1), N2)) ;
% assert (isequal (Nedges (1:nmat1), Nedges2)) ;
% assert (isequal (File (1:nmat1), File2)) ;
for id = (nmat1+1):nmat
    T_allktruss_grb (id) = nan ;
    Kmax (id) = nan ;
end

[T_allktruss, File2, N2, Nedges2, Kmax2] = allktruss_results ;


nmat1 = length (N2) ;
% assert (isequal (N (1:nmat1), N2)) ;
% assert (isequal (Nedges (1:nmat1), Nedges2)) ;
% assert (isequal (File (1:nmat1), File2)) ;
for id = (nmat1+1):nmat
    T_allktruss (:,:,id) = nan ;
    Kmax2 (id) = nan ;
end

% assert (isequalwithequalnans (Kmax, Kmax2)) ;

[T_ktruss_grb, File2, N2, Nedges2] = ktruss_grb_results ;

nmat1 = length (N2) ;
% assert (isequal (N (1:nmat1), N2)) ;
% assert (isequal (Nedges (1:nmat1), Nedges2)) ;
% assert (isequal (File (1:nmat1), File2)) ;
for id = (nmat1+1):nmat
    T_ktruss_grb {id} = nan (3,1) ;
end

[T_ktruss,     File2, N2, Nedges2]  = ktruss_results ;

nmat1 = length (N2) ;
% assert (isequal (N (1:nmat1), N2)) ;
% assert (isequal (Nedges (1:nmat1), Nedges2)) ;
% assert (isequal (File (1:nmat1), File2)) ;
for id = (nmat1+1):nmat
    T_ktruss {id} = nan (3,160) ;
end

clear N2 Nedges2 Kmax2 File2
whos

% find the # of threads used

Time = T_allktruss ;
nthreads_max = size (Time, 2)
threads = [ ] ;
for nth = 1:nthreads_max
    if (all (all (all (~isnan (Time (1, nth, 1))))))
        threads = [threads nth] ;
    end
end

threads
% nmat = size (Time, 3)
esort = elist ;

which_keep = 1 ; % time in allktruss without keeping each k-truss

fprintf ('id | 3-truss                           | allktruss\n') ;
fprintf ('   | grb seq para                      | kmax | grb seq para\n') ;

% for id = 1:nmat
for kkk = 1:length (esort)
    id = esort (kkk) ;
    e = Nedges (id) ;
    if (e < 2e5)
        continue ;
    end

    n = N (id) ;
    e = Nedges (id) ;
    file = fixmawi (File {id}) ;
%   fprintf ('%10d %12d : ', n, e) ;

%   fprintf ('%3d %30s %10d %12d ', id, file, N(id), Nedges(id)) ;
    what = kkk ;
    % what = id ;
    fprintf ('%3d& %24s & %6.2f & %7.2f ', ...
        what, file, n/1e6, e/1e6) ;

    % k-truss in GraphBLAS: 1D array of size kmax, just one thread
    Tk_grb = T_ktruss_grb {id} ;
    Tk_grb_3rate = 1e-6 * e / Tk_grb (3) ;      % 3-truss rate in GrB

    % k-truss in pure C, Tk (3:kmax, threads)
    Tk = T_ktruss {id} ;
    Tk_seq =      Tk (:, 1) ;                   % 1 thread
    Tk_par = min (Tk (:, threads), [ ], 2) ;    % best of all threads
    Tk_seq_3rate = 1e-6 * e / Tk_seq (3) ;      % sequential 3-truss rate
    Tk_par_3rate = 1e-6 * e / Tk_par (3) ;      % best parallel 3-truss rate

    fprintf (' & %8.1f  & %8.1f  & %8.1f ', ... % id, ...
        Tk_grb_3rate, Tk_seq_3rate, Tk_par_3rate) ; 

    kmax = Kmax (id) ;
    fprintf (' & %4d ', kmax) ;

    % all-k-truss in GraphBLAS
    Tall_grb = T_allktruss_grb (which_keep,id) ;
    Tall_grb_rate = 1e-6 * (kmax-2) * e / Tall_grb ; % all-truss rate in GrB
    fprintf (' & %8.1f ', Tall_grb_rate) ;

    % all-k-truss in pure C
    Tall = T_allktruss (:,:,id) ;
    Tall_seq =      Tall (which_keep,1)  ;  % 1 thread
    Tall_par = min (Tall (which_keep,:)) ;  % best of all threads

    Tall_seq_rate = 1e-6 * (kmax-2) * e / Tall_seq ; % sequential all-k rate
    Tall_par_rate = 1e-6 * (kmax-2) * e / Tall_par ; % best parallel all-k rate

    fprintf (' & %8.1f ', Tall_seq_rate) ;
    fprintf (' & %8.1f ', Tall_par_rate) ;

    fprintf ('\\\\\n') ;
end

