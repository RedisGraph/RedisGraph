function test71_table
%TEST71_TABLE print the table for triangle counting results

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

load test71_results

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
]

index = ssget ;
nmat = size (Kokkos,1) ;

for k = 1:nmat
    id = Kokkos (k,1) ;
    i = find (f == id) ;
    fprintf ('%%------------------------------------\n') ;
    fprintf ('%s/%s & %d & %d & %d &\n', ...
        index.Group{id}, index.Name{id}, ...
        Nnodes (i), Nedges (i), Ntri (i)) ;

    t_matlab  = T (i,3) ;
    t_grbdot   = T (i,2) ;
    t_grbouter = T (i,1) ;
    t_kokkos  = Kokkos (k,2) ;

    e = Nedges (i) / 1e6 ;

    fprintf ('%10.3f & %8.2f &  %% MATLAB\n',   t_matlab,  e/t_matlab) ;
    fprintf ('%10.3f & %8.2f &  %% GB:dot\n',   t_grbdot,   e/t_grbdot) ;
    fprintf ('%10.3f & %8.2f &  %% GB:outer\n', t_grbouter, e/t_grbouter) ;
    fprintf ('%10.3f & %8.2f & %8.1f \\\\ %% Kokkos\n', t_kokkos,  e/t_kokkos, t_grbouter / t_kokkos) ;

    % r_kokkos   = Kokkos (k,3) ;
    % t_kokkos = e / r_kokkos ;
    % fprintf ('%10.4f & %8.2f \\\\ %% Kokkos\n',   t_kokkos,  e/t_kokkos) ;

end


