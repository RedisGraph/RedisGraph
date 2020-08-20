function test71(f)
%TEST71 performance comparison of triangle counting methods
% Considers MATLAB:Sandia, GraphBLAS:Sandia, and GraphBLAS:Sandia2 only.
%
% Requires ssget and SuiteSparse/MATLAB_Tools/SSMULT.
% To compile SSMULT in MATLAB, do:
%
%   cd SuiteSparse/MATLAB_Tools/SSMULT
%   ssmult_install
%
% Then add the the path to your MATLAB path.
% ssget is at http://sparse.tamu.edu
%
% This test saves its results in test71_results.mat, so it
% can be restarted.  Results already obtained will be skipped.
%
% Can also run a list of matrices.  For example:
% test71([936 2662])
%
% Edit ll_memory_limit and nz_limit to match the memory on your machine.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[save save_chunk] = nthreads_get ;
chunk = 4096 ;
nthreads = feature ('numcores') ;
nthreads_set (nthreads, chunk) ;

L = sparse (1) ;
try
    ssmultsym (L,L) ;
catch
    here = pwd ;
    cd ../../MATLAB_Tools/SSMULT
    ssmult_install
    cd (here) ;
end

% matrices are too big for some methods.  Edit memory sizes as needed.
if (ismac || ispc)
    % assume this is a laptop with limited memory
    results = 'test71_results.mat' ;
    ll_memory_limit = 12e9 ;       % 12 GB memory
    nz_limit = 500e6 ;
else
    % assume this is a large server
    results = 'test71_results_big.mat' ;
    ll_memory_limit = 700e9 ;      % 700 GB memory
    nz_limit = inf ;
end

index = ssget ;

if (nargin == 0)
    % get all square matrices and sort by nnz(A)
    f = find (index.nrows == index.ncols) ;
    [~, i] = sort (index.nnz (f)) ;
    f = f (i) ;

    Kokkos = [
    2292 0.00441 79.9
    2293 0.00502 72.5
    2289 0.00580 70.0
    2284 0.00390 108 
    2286 0.00611 76.8
    2287 0.00630 80.1
    2305 0.0754  30.7
    2306 0.0177 133
    2307 0.0184 132
    2294 0.497 31.5
    2285 0.733 58.5
    1842 0.232 199
    750 nan nan
    1904 nan nan
    2482 nan nan
    916 nan nan
    2276 nan nan
    2662 nan nan
    ] ;

    f = Kokkos (:,1)' ;

    % f = 1323
    % f = f (1)
    figure (1)
end

nmat = length (f) ;

% try
%     load (results) ;
% catch
    T = nan (nmat, 3) ;
    Nedges = nan (nmat, 1) ;
    Nnodes = nan (nmat, 1) ;
    LLnz   = nan (nmat, 1) ;
    LLmem  = nan (nmat, 1) ;
    LLflops= nan (nmat, 1) ;
    Ntri   = nan (nmat, 1) ;
% end

% merge with prior results
if (nargin == 0)
    try
        old = load ('test71_results.mat') ;
        ok = ~isnan (old.Nnodes) ;
        Nnodes (ok) = old.Nnodes (ok) ;
        Nedges (ok) = old.Nedges (ok) ;
        ok = ~isnan (old.LLnz) ;
        LLnz (ok) = old.LLnz (ok) ;
        LLmem (ok) = old.LLmem (ok) ;
        LLflops (ok) = old.LLflops (ok) ;
        ok = ~isnan (Ntri) ;
        Ntri (ok) = old.Ntri (ok) ;
    catch
    end
end

tstart = cputime ;
x = sparse (0) ;

for k = 1:nmat

    % skip if already done
    if (all (~isnan (T (k,:))))
        continue ;
    end

    % get the problem
    id = f (k) ;
    if (index.nnz (id) > nz_limit)
        fprintf ('\n%s/%s : too big\n', index.Group{id}, index.Name{id}) ;
        continue ;
    end
    Prob = ssget (id, index) ;
    A = Prob.A ;
    name = Prob.name ;
    clear Prob

    % remove the diagonal and extract L and U
    A = spones (A) ;
    A = spones (A+A') ;
    L = tril (A,-1) ;
    U = triu (A,1) ;
    n = size (A,1) ;
    nz = nnz (L) ;
    clear A

    fprintf ('\nid %4d Matrix %s\n', id, name) ;
    fprintf ('n %d edges %d\n', n, nz) ;
    diary off
    diary on

    % do the work in unint32 inside GraphBLAS.  The mexFunction
    % typecasts Lint and Uint from double (the sparse L and U)
    % into uin32, when it creates GraphBLAS matrices.  This
    % typecast time does not count against the run time of
    % GraphBLAS, since outside of MATLAB the matrices would never
    % be in double precision.
    Lint.matrix = L ;
    Lint.class = 'uint32' ;

    Uint.matrix = U ;
    Uint.class = 'uint32' ;

    Nedges (k) = nz ;
    Nnodes (k) = n ;

    % count the triangles in MATLAB and in GraphBLAS

    [nt1 t1] = GB_mex_tricount (3, x, x, Uint, x) ;      % C<L>=L*L
    fprintf ('triangles: %d\n', nt1) ;
    fprintf ('GraphBLAS outer product: %14.6f sec (rate %6.2f million/sec)', t1, 1e-6*nz/t1) ;
    T (k,1) = t1 ;
    Ntri (k) = nt1 ;

    fprintf ('\n') ;

    [nt2 t2] = GB_mex_tricount (5, x, x, Uint, Lint) ;  % C<U>=L'*U
    fprintf ('GraphBLAS dot   product: %14.6f sec (rate %6.2f million/sec)', t2, 1e-6*nz/t2) ;
    T (k,2) = t2 ;
    assert (nt1 == nt2) ;

    fprintf ('\n') ;
    clear Uint Lint

    % get llnz, flops, and memory for L*L
    if (isnan (LLnz (k)))
        llsymbolic = ssmultsym (L, L) ;
        LLnz (k) = llsymbolic.nz ;
        LLmem (k) = llsymbolic.memory ;
        LLflops (k) = llsymbolic.flops ;
    end

    fprintf ('nnz(L*L) %g flops %g memory %g (GB)\n', ...
        LLnz (k), LLflops (k), LLmem (k) / 1e9) ;

    clear L Lint Uint

    % MATLAB, but only do it once
    ok = true ;
    t3 = T (k,3) ;
    if (isnan (t3))
        t3 = inf ;
        try
            if (LLmem (k) < ll_memory_limit)
                tic
                nt3 = sum (sum ((U * U) .* U)) ;
                t3 = toc ;
                ok = (nt1 == nt3) ;
            end
        catch
        end
    end
    assert (ok) ;

    fprintf ('MATLAB (U*U).*U:         %14.6f sec (rate %6.2f million/sec)\n', t3, 1e-6*nz/t3) ;
    T (k,3) = t3 ;

    clear L

    % save the results and redraw the plot, but wait at least 5 seconds
    tnow = cputime - tstart ;
    if (nargin == 0 && tnow > 5)
        diary off
        diary on
        save (results, 'T', 'Nedges', 'Nnodes', 'f', ...
            'LLnz', 'LLmem', 'LLflops', 'Ntri') ; ;
        test71_plot (T, Nedges, Nnodes, LLnz, LLmem, LLflops, Ntri, f) ;
        tstart = cputime ;
    end

end

if (nargin == 0)
    test71_plot ;
end
nthreads_set (save, save_chunk) ;
