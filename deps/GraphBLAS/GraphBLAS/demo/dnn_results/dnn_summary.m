% MATLAB dnn_gb.m
% threads:   1 time:      23.53 sec speedup:     1.00 rate:      10.03 billion
% threads:   1 time:      67.52 sec speedup:     1.00 rate:      13.98 billion
% threads:   1 time:     242.19 sec speedup:     1.00 rate:      15.59 billion
% threads:   1 time:      97.65 sec speedup:     1.00 rate:       9.66 billion
% threads:   1 time:     292.59 sec speedup:     1.00 rate:      12.90 billion
% threads:   1 time:    1075.60 sec speedup:     1.00 rate:      14.04 billion
% threads:   1 time:     766.19 sec speedup:     1.00 rate:       4.93 billion
% threads:   1 time:    2683.75 sec speedup:     1.00 rate:       5.63 billion
% threads:   1 time:   10381.39 sec speedup:     1.00 rate:       5.82 billion
% threads:   1 time:    3777.13 sec speedup:     1.00 rate:       4.00 billion
% threads:   1 time:   13816.56 sec speedup:     1.00 rate:       4.37 billion
% threads:   1 time:   54701.46 sec speedup:     1.00 rate:       4.42 billion

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

TM1 =  [
      23.53
      67.52
     242.19
      97.65
     292.59
    1075.60
     766.19
    2683.75
   10381.39
    3777.13
   13816.56
   54701.46 ] ;

% threads:  40 time:       2.75 sec speedup:     8.54 rate:      85.64 billion
% threads:  40 time:       9.32 sec speedup:     7.25 rate:     101.27 billion
% threads:  40 time:      34.29 sec speedup:     7.06 rate:     110.08 billion
% threads:  40 time:       9.98 sec speedup:     9.78 rate:      94.54 billion
% threads:  40 time:      30.62 sec speedup:     9.56 rate:     123.28 billion
% threads:  40 time:     116.95 sec speedup:     9.20 rate:     129.11 billion
% threads:  40 time:      58.44 sec speedup:    13.11 rate:      64.60 billion
% threads:  40 time:     200.74 sec speedup:    13.37 rate:      75.22 billion
% threads:  40 time:     782.76 sec speedup:    13.26 rate:      77.16 billion
% threads:  40 time:     254.38 sec speedup:    14.85 rate:      59.36 billion
% threads:  40 time:     970.67 sec speedup:    14.23 rate:      62.22 billion
% threads:  40 time:    3829.45 sec speedup:    14.28 rate:      63.09 billion

TM40 = [
       2.75
       9.32
      34.29
       9.98
      30.62
     116.95
      58.44
     200.74
     782.76
     254.38
     970.67
    3829.45 ] ;

% % LAGRAPH, pure C
% nthreads  1: soln time        24.20 sec                  rate     9.7483 (1e9 edges/sec) 
% nthreads  1: soln time        68.24 sec                  rate    13.8302 (1e9 edges/sec) 
% nthreads  1: soln time       243.30 sec                  rate    15.5156 (1e9 edges/sec) 
% nthreads  1: soln time       108.10 sec                  rate     8.7299 (1e9 edges/sec) 
% nthreads  1: soln time       330.14 sec                  rate    11.4343 (1e9 edges/sec) 
% nthreads  1: soln time      1222.49 sec                  rate    12.3514 (1e9 edges/sec) 
% nthreads  1: soln time       741.36 sec                  rate     5.0918 (1e9 edges/sec) 
% nthreads  1: soln time      2552.07 sec                  rate     5.9166 (1e9 edges/sec) 
% nthreads  1: soln time      9783.12 sec                  rate     6.1737 (1e9 edges/sec) 
% nthreads  1: soln time      4536.25 sec                  rate     3.3286 (1e9 edges/sec) 
% nthreads  1: soln time     16446.86 sec                  rate     3.6723 (1e9 edges/sec) 
% nthreads  1: soln time     65492.28 sec                  rate     3.6889 (1e9 edges/sec) 

TL1 = [
   24.20
   68.24
  243.30
  108.10
  330.14
 1222.49
  741.36
 2552.07
 9783.12
 4536.25
16446.86
65492.28 ] ;

% nthreads 40: soln time         2.35 sec speedup    10.30 rate   100.3991 (1e9 edges/sec) 
% nthreads 40: soln time         4.51 sec speedup    15.12 rate   209.0649 (1e9 edges/sec) 
% nthreads 40: soln time        16.40 sec speedup    14.84 rate   230.1753 (1e9 edges/sec) 
% nthreads 40: soln time         9.33 sec speedup    11.59 rate   101.1921 (1e9 edges/sec) 
% nthreads 40: soln time        30.72 sec speedup    10.75 rate   122.8900 (1e9 edges/sec) 
% nthreads 40: soln time       117.64 sec speedup    10.39 rate   128.3516 (1e9 edges/sec) 
% nthreads 40: soln time        51.01 sec speedup    14.53 rate    74.0045 (1e9 edges/sec) 
% nthreads 40: soln time       174.99 sec speedup    14.58 rate    86.2887 (1e9 edges/sec) 
% nthreads 40: soln time       690.10 sec speedup    14.18 rate    87.5206 (1e9 edges/sec) 
% nthreads 40: soln time       245.33 sec speedup    18.49 rate    61.5469 (1e9 edges/sec) 
% nthreads 40: soln time       926.40 sec speedup    17.75 rate    65.1962 (1e9 edges/sec) 
% nthreads 40: soln time      3743.31 sec speedup    17.50 rate    64.5397 (1e9 edges/sec) 

TL40 = [
   2.35
   4.51
  16.40
   9.33
  30.72
 117.64
  51.01
 174.99
 690.10
 245.33
 926.40
3743.31 ] ;

% DNN sizes:
% # edges in all layers: 3.93216 million
% # edges in all layers: 15.7286 million
% # edges in all layers: 62.9146 million
% # edges in all layers: 15.7286 million
% # edges in all layers: 62.9146 million
% # edges in all layers: 251.658 million
% # edges in all layers: 62.9146 million
% # edges in all layers: 251.658 million
% # edges in all layers: 1006.63 million
% # edges in all layers: 251.658 million
% # edges in all layers: 1006.63 million
% # edges in all layers: 4026.53 million

DNN_edges = [
 3.93216 * 1e6
 15.7286 * 1e6
 62.9146 * 1e6
 15.7286 * 1e6
 62.9146 * 1e6
 251.658 * 1e6
 62.9146 * 1e6
 251.658 * 1e6
 1006.63 * 1e6
 251.658 * 1e6
 1006.63 * 1e6
 4026.53 * 1e6 ] ;

nfeatures = 60000 ;

fprintf ('%% Run time [1 thread: MATLAB LAGraph M/L]  [40 threads: MATLAB LAGraph M/L]\n') ;
for prob = 1:12

    if (TM1 (prob) < TL1 (prob))
        fprintf ('{\\bf %8.0f }&      %8.0f  & %8.2f     ', ...
        TM1 (prob), TL1 (prob), TM1 (prob) / TL1 (prob)) ;
    else
        fprintf ('     %8.0f  & {\\bf %8.0f }& %8.2f     ', ...
        TM1 (prob), TL1 (prob), TM1 (prob) / TL1 (prob)) ;
    end

    if (TM40 (prob) < TL40 (prob))
        fprintf ('{\\bf %8.0f }&      %8.0f  & %8.2f     ', ...
        TM40 (prob), TL40 (prob), TM40 (prob) / TL40 (prob)) ;
    else
        fprintf ('     %8.0f  & {\\bf %8.0f }& %8.2f     ', ...
        TM40 (prob), TL40 (prob), TM40 (prob) / TL40 (prob)) ;
    end

    fprintf ('\n') ;

end

RM1  = 1e-9 * (DNN_edges * nfeatures) ./ TM1 ;
RM40 = 1e-9 * (DNN_edges * nfeatures) ./ TM40 ;

RL1  = 1e-9 * (DNN_edges * nfeatures) ./ TL1 ;
RL40 = 1e-9 * (DNN_edges * nfeatures) ./ TL40 ;

fprintf ('\n\n') ;
fprintf ('%% Rate(1e9)[1 thread: MATLAB LAGraph M/L]  [40 threads: MATLAB LAGraph M/L]\n') ;
% rate (in 1e9)
for prob = 1:12

    if (RM1 (prob) > RL1 (prob))
        fprintf ('{\\bf %8.1f }&      %8.1f  & %8.2f     ', ...
        RM1 (prob), RL1 (prob), RM1 (prob) / RL1 (prob)) ;
    else
        fprintf ('     %8.1f  & {\\bf %8.1f }& %8.2f     ', ...
        RM1 (prob), RL1 (prob), RM1 (prob) / RL1 (prob)) ;
    end

    if (RM40 (prob) > RL40 (prob))
        fprintf ('{\\bf %8.1f }&      %8.1f  & %8.2f     ', ...
        RM40 (prob), RL40 (prob), RM40 (prob) / RL40 (prob)) ;
    else
        fprintf ('     %8.1f  & {\\bf %8.1f }& %8.2f     ', ...
        RM40 (prob), RL40 (prob), RM40 (prob) / RL40 (prob)) ;
    end

    fprintf ('\n') ;

end

