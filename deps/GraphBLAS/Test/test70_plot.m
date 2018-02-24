function test70_plot (T, Nedges, Nnodes)
%TEST70_PLOT plot the results from test70

if (nargin == 0)
    % load test70_results
    load test70_results_big
end

k = max (find (~isnan (Nnodes))) ;

subplot (3,3,1)
loglog ( ...
    Nedges (1:k), T (1:k,1), 'ro', ...
    Nedges (1:k), T (1:k,2), 'bo', ...
    Nedges (1:k), T (1:k,3), 'ko', ...
    Nedges (1:k), T (1:k,4), 'go', ...
    Nedges (1:k), T (1:k,5), 'r+', ...
    Nedges (1:k), T (1:k,6), 'b+', ...
    Nedges (1:k), T (1:k,7), 'k+', ...
    Nedges (1:k), T (1:k,8), 'g+', ...
    Nedges (1:k), T (1:k,9), 'c+') ;
legend ( ...
'MATLAB:minitri', 'MATLAB:Burkhardt', 'MATLAB:Cohen', 'MATLAB:Sandia', ...
'GB:minitri', 'GB:Burkhardt', 'GB:Cohen', 'GB:Sandia', 'GB:Sandia2', ...
'Location', 'NorthWest')
xlabel ('# of edges') ;
ylabel ('run time') ;

tbase = T (1:k,9) ;
e = max (Nedges (1:k)) ;

subplot (3,3,2)
loglog (Nedges (1:k), T (1:k,1) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:minitri time / GB:Sandia2 time') ;

subplot (3,3,3)
loglog (Nedges (1:k), T (1:k,2) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Burkhardt time / GB:Sandia2 time') ;

subplot (3,3,4)
loglog (Nedges (1:k), T (1:k,3) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Cohen time / GB:Sandia2 time') ;

subplot (3,3,5)
loglog (Nedges (1:k), T (1:k,4) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Sandia time / GB:Sandia2 time') ;

subplot (3,3,6)
loglog (Nedges (1:k), T (1:k,5) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:minitri time / GB:Sandia2 time') ;

subplot (3,3,7)
loglog (Nedges (1:k), T (1:k,6) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Burkhardt time / GB:Sandia2 time') ;

subplot (3,3,8)
loglog (Nedges (1:k), T (1:k,7) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Cohen time / GB:Sandia2 time') ;

subplot (3,3,9)
loglog (Nedges (1:k), T (1:k,8) ./ T (1:k,9), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Sandia time / GB:Sandia2 time') ;

drawnow

