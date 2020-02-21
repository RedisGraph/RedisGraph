function test70_plot (T, Nedges, Nnodes)
%TEST70_PLOT plot the results from test70

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    load test70_results
    % load test70_results_big
end

k = max (find (~isnan (Nnodes))) ;

subplot (3,4,1)
loglog ( ...
    Nedges (1:k), T (1:k,1), 'ro', ...
    Nedges (1:k), T (1:k,2), 'bo', ...
    Nedges (1:k), T (1:k,3), 'ko', ...
    Nedges (1:k), T (1:k,4), 'go', ...
    Nedges (1:k), T (1:k,5), 'co', ...
    Nedges (1:k), T (1:k,6), 'mo', ...
    Nedges (1:k), T (1:k,7), 'ro', ...
    Nedges (1:k), T (1:k,8), 'b+', ...
    Nedges (1:k), T (1:k,9), 'k+', ...
    Nedges (1:k), T (1:k,10),'g+', ...
    Nedges (1:k), T (1:k,11),'c+', ...
    Nedges (1:k), T (1:k,12),'m+') ;
legend ( ...
'MATLAB:minitri', ...
'MATLAB:Burkhardt', ...
'MATLAB:Cohen', ...
'MATLAB:Sandia', ...
'MATLAB:SandiaL', ...
'MATLAB:SandiaDot', ...
'GB:minitri', ...
'GB:Burkhardt', ...
'GB:Cohen', ...
'GB:Sandia', ...
'GB:SandiaL', ...
'GB:SandiaDot', ...
'Location', 'NorthWest')

xlabel ('# of edges') ;
ylabel ('run time') ;

e = max (Nedges (1:k)) ;

subplot (3,4,2)
loglog (Nedges (1:k), T (1:k,1) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:minitri time / GB:SandiaL time') ;

subplot (3,4,3)
loglog (Nedges (1:k), T (1:k,2) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Burkhardt time / GB:SandiaL time') ;

subplot (3,4,4)
loglog (Nedges (1:k), T (1:k,3) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Cohen time / GB:SandiaL time') ;

subplot (3,4,5)
loglog (Nedges (1:k), T (1:k,4) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:Sandia time / GB:SandiaL time') ;

subplot (3,4,6)
loglog (Nedges (1:k), T (1:k,5) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:SandiaL time / GB:SandiaL time') ;

subplot (3,4,7)
loglog (Nedges (1:k), T (1:k,6) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('MATLAB:SandiaDot time / GB:SandiaL time') ;

%-------------------------------------------------------------------------------

subplot (3,4,8)
loglog (Nedges (1:k), T (1:k,7) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:minitri time / GB:SandiaL time') ;

subplot (3,4,9)
loglog (Nedges (1:k), T (1:k,8) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Burkhardt time / GB:SandiaL time') ;

subplot (3,4,10)
loglog (Nedges (1:k), T (1:k,9) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Cohen time / GB:SandiaL time') ;

subplot (3,4,11)
loglog (Nedges (1:k), T (1:k,10) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:Sandia time / GB:SandiaL time') ;

subplot (3,4,12)
loglog (Nedges (1:k), T (1:k,12) ./ T (1:k,11), 'ko', [1 e], [1 1], 'k-') ;
xlabel ('# of edges') ;
ylabel ('GB:SandiaDot time / GB:SandiaL time') ;

drawnow

