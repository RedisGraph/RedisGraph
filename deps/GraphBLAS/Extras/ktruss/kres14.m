
clear

% load all results
[T_allktruss_grb, File,  N,  Nedges,  Kmax] = allktruss_grb_results ;

[T_allktruss,     File2, N2, Nedges2, Kmax2] = allktruss_results ;

assert (isequal (N, N2)) ;
assert (isequal (Nedges, Nedges2)) ;
assert (isequal (Kmax, Kmax2)) ;
assert (isequal (File, File2)) ;

[T_ktruss_grb, File2, N2, Nedges2] = ktruss_grb_results ;

%{
assert (isequal (N, N2)) ;
assert (isequal (Nedges, Nedges2)) ;
% assert (isequal (Kmax, Kmax2)) ;
assert (isequal (File, File2)) ;
%}

[T_ktruss,     File2, N2, Nedges2]  = ktruss_results ;

%{
assert (isequal (N, N2)) ;
assert (isequal (Nedges, Nedges2)) ;
% assert (isequal (Kmax, Kmax2)) ;
assert (isequal (File, File2)) ;
%}

clear N2 Nedges2 Kmax2 File2

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
nmat = 19

which_keep = 1 ; % time in allktruss without keeping each k-truss

fprintf ('id | 3-truss                           | allktruss\n') ;
fprintf ('   | grb seq para                      | kmax | grb seq para\n') ;
for id = 1:nmat

    n = N (id) ;
    e = Nedges (id) ;
    fprintf ('%10d %12d : ', n, e) ;

    % k-truss in GraphBLAS: 1D array of size kmax, just one thread
    Tk_grb = T_ktruss_grb {id} ;
    Tk_grb_3rate = 1e-6 * e / Tk_grb (3) ;      % 3-truss rate in GrB

    % k-truss in pure C, Tk (3:kmax, threads)
    Tk = T_ktruss {id} ;
    Tk_seq =      Tk (:, 1) ;                   % 1 thread
    Tk_par = min (Tk (:, threads), [ ], 2) ;    % best of all threads
    Tk_seq_3rate = 1e-6 * e / Tk_seq (3) ;      % sequential 3-truss rate
    Tk_par_3rate = 1e-6 * e / Tk_par (3) ;      % best parallel 3-truss rate

    fprintf ('%2d & %8.1f  & %8.1f  & %8.1f ', id, ...
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

