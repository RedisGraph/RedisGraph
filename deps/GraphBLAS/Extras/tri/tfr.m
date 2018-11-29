function tfr

n =  119432957  ; 
e =  1799999986  ; 

% % /users/davis/GraphChallenge/snap/friendster/friendster_adj.tsv.gz
% S prep:0 time      5.47770 threads   1
% S prep:0 time      6.35170 threads   2
% S prep:0 time      5.11135 threads   4
% S prep:0 time      3.28445 threads   8
% S prep:0 time      2.12595 threads  16
% S prep:0 time      2.26119 threads  32
% S prep:0 time      2.87851 threads  64
% S prep:0 time      2.95568 threads 128
% S prep:0 time      2.56526 threads 160
% R prep:0 time     36.25617 threads   1
% R prep:0 time     40.96442 threads   2
% R prep:0 time     30.68297 threads   4
% R prep:0 time     18.12771 threads   8
% R prep:0 time      9.67377 threads  16
% R prep:0 time      6.09739 threads  32
% R prep:0 time      3.60461 threads  64
% R prep:0 time      2.53687 threads 128
% R prep:0 time      2.29291 threads 160

% 0: tri_mark     nthreads   1 : 191716   689.914489 sec rate    2.61
% 0: tri_mark     nthreads   2 : 191716   375.982723 sec rate    4.79
% 0: tri_mark     nthreads   4 : 191716   190.253594 sec rate    9.46
% 0: tri_mark     nthreads   8 : 191716   103.394895 sec rate   17.41
% 0: tri_mark     nthreads  16 : 191716    57.626319 sec rate   31.24
% 0: tri_mark     nthreads  32 : 191716    33.136969 sec rate   54.32
% 0: tri_mark     nthreads  64 : 191716    28.329136 sec rate   63.54
% 0: tri_mark     nthreads 128 : 191716    27.959363 sec rate   64.38
% 0: tri_mark     nthreads 160 : 191716    28.105840 sec rate   64.04

% 0: tri_bit      nthreads   1 : 191716   703.886985 sec rate    2.56
% 0: tri_bit      nthreads   2 : 191716   353.912515 sec rate    5.09
% 0: tri_bit      nthreads   4 : 191716   196.024546 sec rate    9.18
% 0: tri_bit      nthreads   8 : 191716   103.655962 sec rate   17.37
% 0: tri_bit      nthreads  16 : 191716    56.037665 sec rate   32.12
% 0: tri_bit      nthreads  32 : 191716    31.546347 sec rate   57.06
% 0: tri_bit      nthreads  64 : 191716    24.537434 sec rate   73.36
% 0: tri_bit      nthreads 128 : 191716    27.026594 sec rate   66.60
% 0: tri_bit      nthreads 160 : 191716    34.024947 sec rate   52.90

% 0: tri_dot      nthreads   1 : 191716  1346.620320 sec rate    1.34
% 0: tri_dot      nthreads   2 : 191716   689.779232 sec rate    2.61
% 0: tri_dot      nthreads   4 : 191716   336.581647 sec rate    5.35
% 0: tri_dot      nthreads   8 : 191716   167.496258 sec rate   10.75
% 0: tri_dot      nthreads  16 : 191716    81.753185 sec rate   22.02
% 0: tri_dot      nthreads  32 : 191716    40.821216 sec rate   44.09
% 0: tri_dot      nthreads  64 : 191716    22.172620 sec rate   81.18
% 0: tri_dot      nthreads 128 : 191716    19.144363 sec rate   94.02
% 0: tri_dot      nthreads 160 : 191716    19.075288 sec rate   94.36

% 0: tri_logmark  nthreads   1 : 191716  2637.025384 sec rate    0.68
% 0: tri_logmark  nthreads   2 : 191716  1272.340471 sec rate    1.41
% 0: tri_logmark  nthreads   4 : 191716   644.875558 sec rate    2.79
% 0: tri_logmark  nthreads   8 : 191716   329.216188 sec rate    5.47
% 0: tri_logmark  nthreads  16 : 191716   159.329162 sec rate   11.30
% 0: tri_logmark  nthreads  32 : 191716    79.701649 sec rate   22.58
% 0: tri_logmark  nthreads  64 : 191716    41.861751 sec rate   43.00
% 0: tri_logmark  nthreads 128 : 191716    26.203552 sec rate   68.69
% 0: tri_logmark  nthreads 160 : 191716    23.642465 sec rate   76.13

% 0: tri_simple   nthreads   1 : 191716   830.263628 sec rate    2.17

fprintf ('friendster: 1 thread\n') ;

id = 1
N (id) =  119432957  ; 
Nedges (id) =  1799999986  ; 
Tprep (id,1) =  15.144969  ; % outer product prep
Tprep (id,2) = Tprep (id,1) +  18.179867  ; % dot product prep
Ntri (id) =  191716  ; 
T (id,2) =  424.531732  ;  % dot product 
T (id,1) =  351.595748  ; % outer product 

t = T (id,1) ;
p = Tprep (id,1) ;
fprintf ('GrB    %12.6f\n', 1e-6 * e / (t+p)) ;

t = 830.263628 ;
p = 5.47770 ;
fprintf ('simple %12.6f\n', 1e-6 * e / (t+p)) ;

t = 689.914489 ;
p = 5.47770 ;
fprintf ('mark  %12.6f\n', 1e-6 * e / (t+p)) ;

t = 703.886985 ;
p = 5.47770 ;
fprintf ('bit   %12.6f\n', 1e-6 * e / (t+p)) ;

t = T (id,2) ;
p = Tprep (id,2) ;
fprintf ('GrBdot %12.6f\n', 1e-6 * e / (t+p)) ;

t = 1346.620320  ;
p = 5.47770 + 36.25617 ;
fprintf ('dot   %12.6f\n', 1e-6 * e / (t+p)) ;

fprintf ('friendster: multiple threads\n') ;

% S prep:0 time      2.95568 threads 128
% 0: tri_mark     nthreads 128 : 191716    27.959363 sec rate   64.38
t = 27.959363 ;
p = 2.95568 ;
fprintf ('mark  %12.6f\n', 1e-6 * e / (t+p)) ;

% S prep:0 time      2.87851 threads  64
% 0: tri_bit      nthreads  64 : 191716    24.537434 sec rate   73.36
p = 2.87851 ;
t = 24.537434 ;
fprintf ('bit   %12.6f\n', 1e-6 * e / (t+p)) ;

% S prep:0 time      2.56526 threads 160
% R prep:0 time      2.29291 threads 160
% 0: tri_dot      nthreads 160 : 191716    19.075288 sec rate   94.36
p = 2.56526 + 2.29291 ;
t = 19.075288 ;
fprintf ('dot   %12.6f\n', 1e-6 * e / (t+p)) ;

end
