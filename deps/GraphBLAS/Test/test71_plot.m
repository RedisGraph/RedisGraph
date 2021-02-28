function test71_plot (T, Nedges, Nnodes, LLnz, LLmem, LLflops, Ntri, f)
%TEST71_PLOT plot the results from test71

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    if (ismac || ispc)
        load test71_results
    else
        load test71_results_big
    end
end

% k = max (find (~isnan (Nnodes))) ;
k = max (find (any (~isnan (T), 2))) ;

subplot (2,4,1)
loglog ( ...
    Nedges (1:k), T (1:k,1), 'ro', ...
    Nedges (1:k), T (1:k,2), 'g+', ...
    Nedges (1:k), T (1:k,3), 'bs') ;
legend ('GB:Sandia', 'GB:Sandia2', 'MATLAB:Sandia', 'Location', 'NorthWest')
xlabel ('# of edges') ;
ylabel ('run time') ;

e = max (Nedges (1:k), [], 1, 'omitnan') ;
e = 10^ceil (log10 (e)) ;
l = max (LLnz (1:k),   [], 1, 'omitnan') ;
l = 10^ceil (log10 (l)) ;
mx = max (Ntri (1:k), Nedges (1:k)) ;
x = max (mx, [], 1, 'omitnan') ;
x = 10^ceil (log10 (x)) ;

subplot (2,4,2)
loglog (Nedges (1:k), Nedges (1:k)./ T (1:k,1)  , 'ro') ;
axis([1 e 1e4 1e9]) ;
legend ('GB:Sandia', 'Location', 'NorthWest')
xlabel ('# of edges') ;
ylabel ('rate (#edges/time)') ;

subplot (2,4,3)
loglog (Nedges (1:k), Nedges (1:k)./ T (1:k,2)  , 'g+') ;
axis([1 e 1e4 1e9]) ;
legend ('GB:Sandia2', 'Location', 'NorthWest')
xlabel ('# of edges') ;
ylabel ('rate (#edges/time)') ;

subplot (2,4,4)
loglog (Nedges (1:k), Nedges (1:k)./ T (1:k,3)  , 'bs') ;
axis([1 e 1e4 1e9]) ;
legend ('MATLAB:Sandia', 'Location', 'NorthWest')
xlabel ('# of edges') ;
ylabel ('rate (#edges/time)') ;


subplot (2,4,5)
loglog (Nedges (1:k), T (1:k,3) ./ T (1:k,1), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Sandia time / GB:Sandia time') ;

subplot (2,4,6)
loglog (Nedges (1:k), T (1:k,2) ./ T (1:k,1), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Sandia2 time / GB:Sandia time') ;

subplot (2,4,7)
loglog (LLnz (1:k), T (1:k,2) ./ T (1:k,1), 'ko', [1 l], [1 1], 'k-') ;
xlabel ('nnz(L*L)') ;
ylabel ('GB:Sandia2 time / GB:Sandia time') ;

r = l/e ;
subplot (2,4,8)
loglog (LLnz (1:k) ./ Nedges(1:k), ...
    T (1:k,2) ./ T (1:k,1), 'ko', [1 r], [1 1], 'k-') ;
xlabel ('nnz(L*L)/nnz(L)') ;
ylabel ('GB:Sandia2 time / GB:Sandia time') ;

drawnow

