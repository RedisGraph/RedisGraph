function [T, File, N, Nedges]  = ktruss_results
id = 0 ;
file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 841 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0093507 ;
Time (3,2) =  0.000242933 ;
Time (3,4) =  0.000242558 ;
Time (3,8) =  0.000242219 ;
Time (3,16) =  0.000242336 ;
Time (3,32) =  0.000242255 ;
Time (3,64) =  0.000253271 ;
Time (3,128) =  0.000242504 ;
Time (3,160) =  0.000242201 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 841 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00889126 ;
Time (3,2) =  0.000190451 ;
Time (3,4) =  0.000189913 ;
Time (3,8) =  0.000189849 ;
Time (3,16) =  0.000189781 ;
Time (3,32) =  0.000189791 ;
Time (3,64) =  0.000189945 ;
Time (3,128) =  0.000189644 ;
Time (3,160) =  0.000189646 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 800 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00889979 ;
Time (3,2) =  0.000186865 ;
Time (3,4) =  0.000185861 ;
Time (3,8) =  0.000186143 ;
Time (3,16) =  0.000185828 ;
Time (3,32) =  0.000186049 ;
Time (3,64) =  0.000185925 ;
Time (3,128) =  0.000185918 ;
Time (3,160) =  0.000185829 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 346 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00878341 ;
Time (3,2) =  5.31366e-05 ;
Time (3,4) =  5.24158e-05 ;
Time (3,8) =  5.22379e-05 ;
Time (3,16) =  5.22994e-05 ;
Time (3,32) =  5.22789e-05 ;
Time (3,64) =  5.20423e-05 ;
Time (3,128) =   5.8176e-05 ;
Time (3,160) =  5.69057e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 346 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00880844 ;
Time (3,2) =  3.06573e-05 ;
Time (3,4) =  2.99858e-05 ;
Time (3,8) =  2.97679e-05 ;
Time (3,16) =  2.97362e-05 ;
Time (3,32) =  2.94317e-05 ;
Time (3,64) =  2.95015e-05 ;
Time (3,128) =  2.97111e-05 ;
Time (3,160) =  2.95844e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 240 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00879504 ;
Time (3,2) =  2.20938e-05 ;
Time (3,4) =  2.18749e-05 ;
Time (3,8) =  2.16207e-05 ;
Time (3,16) =   2.1575e-05 ;
Time (3,32) =  2.15657e-05 ;
Time (3,64) =  2.15592e-05 ;
Time (3,128) =  2.14176e-05 ;
Time (3,160) =  2.17343e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 31 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00869876 ;
Time (3,2) =  1.07661e-05 ;
Time (3,4) =  1.03489e-05 ;
Time (3,8) =  1.01607e-05 ;
Time (3,16) =  1.01449e-05 ;
Time (3,32) =  9.91393e-06 ;
Time (3,64) =  1.00331e-05 ;
Time (3,128) =  1.02585e-05 ;
Time (3,160) =  1.00629e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 31 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00870913 ;
Time (3,2) =  1.04858e-05 ;
Time (3,4) =  9.98285e-06 ;
Time (3,8) =  9.75002e-06 ;
Time (3,16) =  9.63639e-06 ;
Time (3,32) =  9.75095e-06 ;
Time (3,64) =  9.77237e-06 ;
Time (3,128) =  9.82266e-06 ;
Time (3,160) =  9.73325e-06 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 24 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00880242 ;
Time (3,2) =  9.55723e-06 ;
Time (3,4) =  9.34862e-06 ;
Time (3,8) =  8.98633e-06 ;
Time (3,16) =  8.89041e-06 ;
Time (3,32) =  8.90531e-06 ;
Time (3,64) =  9.14745e-06 ;
Time (3,128) =  9.02079e-06 ;
Time (3,160) =  8.98167e-06 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 720 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00878806 ;
Time (3,2) =   7.5425e-05 ;
Time (3,4) =  7.56541e-05 ;
Time (3,8) =  7.46893e-05 ;
Time (3,16) =  7.47452e-05 ;
Time (3,32) =  7.47638e-05 ;
Time (3,64) =  7.49687e-05 ;
Time (3,128) =  7.51801e-05 ;
Time (3,160) =  7.50422e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 49 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00873025 ;
Time (3,2) =  1.19898e-05 ;
Time (3,4) =  1.13966e-05 ;
Time (3,8) =  1.15698e-05 ;
Time (3,16) =  1.14469e-05 ;
Time (3,32) =  1.12737e-05 ;
Time (3,64) =  1.12969e-05 ;
Time (3,128) =  1.13435e-05 ;
Time (3,160) =  1.13053e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 49 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00874405 ;
Time (3,2) =  1.13482e-05 ;
Time (3,4) =  1.07605e-05 ;
Time (3,8) =  1.09999e-05 ;
Time (3,16) =  1.06022e-05 ;
Time (3,32) =  1.04764e-05 ;
Time (3,64) =  1.04941e-05 ;
Time (3,128) =  1.04066e-05 ;
Time (3,160) =   1.0578e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 40 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00880038 ;
Time (3,2) =  1.06487e-05 ;
Time (3,4) =  1.04848e-05 ;
Time (3,8) =  1.03088e-05 ;
Time (3,16) =  1.03749e-05 ;
Time (3,32) =  1.04569e-05 ;
Time (3,64) =  1.01794e-05 ;
Time (3,128) =  1.01542e-05 ;
Time (3,160) =  1.00322e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 104 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00875213 ;
Time (3,2) =  1.68411e-05 ;
Time (3,4) =  1.65151e-05 ;
Time (3,8) =  1.64509e-05 ;
Time (3,16) =  1.63168e-05 ;
Time (3,32) =  1.62991e-05 ;
Time (3,64) =  1.63568e-05 ;
Time (3,128) =  1.61426e-05 ;
Time (3,160) =  1.61231e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 104 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00874484 ;
Time (3,2) =  1.50623e-05 ;
Time (3,4) =  1.45761e-05 ;
Time (3,8) =  1.43126e-05 ;
Time (3,16) =   1.4375e-05 ;
Time (3,32) =  1.45324e-05 ;
Time (3,64) =  1.42753e-05 ;
Time (3,128) =   1.4443e-05 ;
Time (3,160) =  1.42902e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 90 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00875808 ;
Time (3,2) =  1.35219e-05 ;
Time (3,4) =  1.32974e-05 ;
Time (3,8) =  1.31577e-05 ;
Time (3,16) =  1.31615e-05 ;
Time (3,32) =  1.31791e-05 ;
Time (3,64) =  1.33133e-05 ;
Time (3,128) =  1.31903e-05 ;
Time (3,160) =  1.30394e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 313 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00878444 ;
Time (3,2) =  5.22826e-05 ;
Time (3,4) =  5.18048e-05 ;
Time (3,8) =  5.18868e-05 ;
Time (3,16) =  5.18244e-05 ;
Time (3,32) =  5.18523e-05 ;
Time (3,64) =  5.17545e-05 ;
Time (3,128) =    5.168e-05 ;
Time (3,160) =   5.1816e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 313 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00875158 ;
Time (3,2) =  3.98783e-05 ;
Time (3,4) =  3.93111e-05 ;
Time (3,8) =   3.9475e-05 ;
Time (3,16) =  3.92674e-05 ;
Time (3,32) =  3.93651e-05 ;
Time (3,64) =   3.9109e-05 ;
Time (3,128) =   3.9096e-05 ;
Time (3,160) =  3.92031e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 288 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00876202 ;
Time (3,2) =  3.77269e-05 ;
Time (3,4) =  3.75938e-05 ;
Time (3,8) =  3.73172e-05 ;
Time (3,16) =  3.73926e-05 ;
Time (3,32) =  3.72585e-05 ;
Time (3,64) =  3.72818e-05 ;
Time (3,128) =  3.71682e-05 ;
Time (3,160) =  3.73265e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 940 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00896873 ;
Time (3,2) =  0.000209486 ;
Time (3,4) =  0.000209034 ;
Time (3,8) =  0.000209082 ;
Time (3,16) =  0.000208824 ;
Time (3,32) =  0.000208793 ;
Time (3,64) =  0.000208906 ;
Time (3,128) =  0.000208717 ;
Time (3,160) =  0.000221932 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 940 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00880451 ;
Time (3,2) =  9.90564e-05 ;
Time (3,4) =  9.82815e-05 ;
Time (3,8) =  9.78671e-05 ;
Time (3,16) =  9.81558e-05 ;
Time (3,32) =  9.76752e-05 ;
Time (3,64) =  9.78494e-05 ;
Time (3,128) =  9.76035e-05 ;
Time (3,160) =  9.79407e-05 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 3448 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0104901 ;
Time (3,2) =   0.00179835 ;
Time (3,4) =   0.00182448 ;
Time (3,8) =   0.00185399 ;
Time (3,16) =   0.00213697 ;
Time (3,32) =   0.00473384 ;
Time (3,64) =    0.0148354 ;
Time (3,128) =    0.0466205 ;
Time (3,160) =    0.0439261 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 3448 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00969835 ;
Time (3,2) =  0.000889075 ;
Time (3,4) =  0.000929411 ;
Time (3,8) =  0.000980819 ;
Time (3,16) =   0.00121659 ;
Time (3,32) =   0.00160923 ;
Time (3,64) =    0.0147777 ;
Time (3,128) =    0.0188072 ;
Time (3,160) =    0.0316389 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 2880 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00958484 ;
Time (3,2) =  0.000789658 ;
Time (3,4) =  0.000825242 ;
Time (3,8) =  0.000878215 ;
Time (3,16) =   0.00106111 ;
Time (3,32) =   0.00187521 ;
Time (3,64) =   0.00572989 ;
Time (3,128) =    0.0456754 ;
Time (3,160) =    0.0332433 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4156 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0134243 ;
Time (3,2) =    0.0025267 ;
Time (3,4) =   0.00250584 ;
Time (3,8) =   0.00318571 ;
Time (3,16) =   0.00450859 ;
Time (3,32) =   0.00473495 ;
Time (3,64) =    0.0139164 ;
Time (3,128) =    0.0398613 ;
Time (3,160) =    0.0525789 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4156 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0128638 ;
Time (3,2) =   0.00221142 ;
Time (3,4) =   0.00263452 ;
Time (3,8) =   0.00337363 ;
Time (3,16) =   0.00223419 ;
Time (3,32) =   0.00471823 ;
Time (3,64) =     0.011459 ;
Time (3,128) =    0.0213596 ;
Time (3,160) =     0.054389 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4050 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0129883 ;
Time (3,2) =   0.00221057 ;
Time (3,4) =   0.00200233 ;
Time (3,8) =    0.0020413 ;
Time (3,16) =   0.00298891 ;
Time (3,32) =   0.00383742 ;
Time (3,64) =    0.0106074 ;
Time (3,128) =    0.0475099 ;
Time (3,160) =     0.072201 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 4320 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00949016 ;
Time (3,2) =  0.000645688 ;
Time (3,4) =  0.000686822 ;
Time (3,8) =  0.000722803 ;
Time (3,16) =  0.000967737 ;
Time (3,32) =   0.00127423 ;
Time (3,64) =   0.00511441 ;
Time (3,128) =    0.0389821 ;
Time (3,160) =    0.0295256 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 6583 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0126457 ;
Time (3,2) =   0.00352938 ;
Time (3,4) =   0.00358886 ;
Time (3,8) =    0.0036765 ;
Time (3,16) =   0.00385521 ;
Time (3,32) =   0.00432914 ;
Time (3,64) =    0.0197077 ;
Time (3,128) =    0.0500894 ;
Time (3,160) =    0.0571839 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 6583 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =   0.00997934 ;
Time (3,2) =  0.000959249 ;
Time (3,4) =  0.000993427 ;
Time (3,8) =   0.00104092 ;
Time (3,16) =    0.0012503 ;
Time (3,32) =   0.00209164 ;
Time (3,64) =   0.00683293 ;
Time (3,128) =    0.0308157 ;
Time (3,160) =    0.0337315 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 14400 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0235534 ;
Time (3,2) =   0.00810332 ;
Time (3,4) =   0.00484334 ;
Time (3,8) =   0.00394089 ;
Time (3,16) =   0.00737522 ;
Time (3,32) =    0.0100857 ;
Time (3,64) =    0.0219661 ;
Time (3,128) =    0.0312811 ;
Time (3,160) =    0.0983352 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/as20000102/as20000102_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 6474 ;
Nedges (id) = 12572 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0158956 ;
Time (3,2) =   0.00410936 ;
Time (3,4) =   0.00294883 ;
Time (3,8) =   0.00302515 ;
Time (3,16) =   0.00362259 ;
Time (3,32) =   0.00733917 ;
Time (3,64) =    0.0262437 ;
Time (3,128) =    0.0327778 ;
Time (3,160) =    0.0597102 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 15988 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0347091 ;
Time (3,2) =    0.0137737 ;
Time (3,4) =   0.00939692 ;
Time (3,8) =     0.011752 ;
Time (3,16) =   0.00996832 ;
Time (3,32) =    0.0192782 ;
Time (3,64) =    0.0476973 ;
Time (3,128) =     0.165541 ;
Time (3,160) =     0.181475 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 15988 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0243372 ;
Time (3,2) =   0.00854159 ;
Time (3,4) =   0.00528761 ;
Time (3,8) =   0.00626268 ;
Time (3,16) =    0.0075701 ;
Time (3,32) =   0.00817912 ;
Time (3,64) =      0.03057 ;
Time (3,128) =     0.058477 ;
Time (3,160) =     0.110965 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-GrQc/ca-GrQc_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5242 ;
Nedges (id) = 14484 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0117192 ;
Time (3,2) =   0.00179233 ;
Time (3,4) =   0.00130675 ;
Time (3,8) =    0.0012413 ;
Time (3,16) =   0.00190615 ;
Time (3,32) =   0.00240651 ;
Time (3,64) =   0.00601825 ;
Time (3,128) =    0.0571106 ;
Time (3,160) =    0.0464376 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 23040 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0227549 ;
Time (3,2) =   0.00813943 ;
Time (3,4) =     0.005522 ;
Time (3,8) =   0.00570726 ;
Time (3,16) =   0.00524416 ;
Time (3,32) =    0.0058419 ;
Time (3,64) =    0.0196238 ;
Time (3,128) =    0.0519772 ;
Time (3,160) =    0.0528934 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella08/p2p-Gnutella08_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 6301 ;
Nedges (id) = 20777 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0110176 ;
Time (3,2) =   0.00125031 ;
Time (3,4) =  0.000895101 ;
Time (3,8) =   0.00082655 ;
Time (3,16) =   0.00134421 ;
Time (3,32) =   0.00203045 ;
Time (3,64) =   0.00499005 ;
Time (3,128) =    0.0486057 ;
Time (3,160) =    0.0753034 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010331/oregon1_010331_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10670 ;
Nedges (id) = 22002 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0274228 ;
Time (3,2) =   0.00971197 ;
Time (3,4) =   0.00571313 ;
Time (3,8) =   0.00485772 ;
Time (3,16) =   0.00596866 ;
Time (3,32) =    0.0109368 ;
Time (3,64) =    0.0170006 ;
Time (3,128) =    0.0557308 ;
Time (3,160) =     0.080961 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010407/oregon1_010407_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10729 ;
Nedges (id) = 21999 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0276417 ;
Time (3,2) =    0.0097825 ;
Time (3,4) =   0.00571122 ;
Time (3,8) =   0.00512698 ;
Time (3,16) =   0.00533301 ;
Time (3,32) =    0.0102815 ;
Time (3,64) =    0.0178114 ;
Time (3,128) =    0.0974802 ;
Time (3,160) =    0.0771359 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010414/oregon1_010414_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10790 ;
Nedges (id) = 22469 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0284524 ;
Time (3,2) =    0.0101699 ;
Time (3,4) =    0.0060926 ;
Time (3,8) =   0.00401234 ;
Time (3,16) =   0.00454067 ;
Time (3,32) =   0.00778056 ;
Time (3,64) =    0.0204666 ;
Time (3,128) =     0.067177 ;
Time (3,160) =     0.063385 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010421/oregon1_010421_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10859 ;
Nedges (id) = 22747 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0288209 ;
Time (3,2) =    0.0103695 ;
Time (3,4) =   0.00628572 ;
Time (3,8) =   0.00469762 ;
Time (3,16) =    0.0074049 ;
Time (3,32) =    0.0120978 ;
Time (3,64) =    0.0187485 ;
Time (3,128) =    0.0768636 ;
Time (3,160) =    0.0604947 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010428/oregon1_010428_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10886 ;
Nedges (id) = 22493 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0288615 ;
Time (3,2) =    0.0104769 ;
Time (3,4) =   0.00603847 ;
Time (3,8) =   0.00530271 ;
Time (3,16) =   0.00690549 ;
Time (3,32) =    0.0111947 ;
Time (3,64) =    0.0177808 ;
Time (3,128) =    0.0911819 ;
Time (3,160) =     0.100552 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010505/oregon1_010505_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10943 ;
Nedges (id) = 22607 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0288253 ;
Time (3,2) =    0.0105553 ;
Time (3,4) =   0.00616833 ;
Time (3,8) =   0.00473009 ;
Time (3,16) =   0.00697107 ;
Time (3,32) =    0.0106289 ;
Time (3,64) =    0.0183399 ;
Time (3,128) =    0.0785627 ;
Time (3,160) =     0.117321 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010512/oregon1_010512_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11011 ;
Nedges (id) = 22677 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0289704 ;
Time (3,2) =    0.0105224 ;
Time (3,4) =   0.00639929 ;
Time (3,8) =   0.00517092 ;
Time (3,16) =   0.00637842 ;
Time (3,32) =   0.00802747 ;
Time (3,64) =    0.0182551 ;
Time (3,128) =    0.0645117 ;
Time (3,160) =    0.0961921 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010519/oregon1_010519_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11051 ;
Nedges (id) = 22724 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0291115 ;
Time (3,2) =    0.0108376 ;
Time (3,4) =   0.00666335 ;
Time (3,8) =   0.00439739 ;
Time (3,16) =   0.00539298 ;
Time (3,32) =   0.00845802 ;
Time (3,64) =     0.016612 ;
Time (3,128) =    0.0557511 ;
Time (3,160) =    0.0583895 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010526/oregon1_010526_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11174 ;
Nedges (id) = 23409 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0299195 ;
Time (3,2) =    0.0111584 ;
Time (3,4) =   0.00660501 ;
Time (3,8) =   0.00528178 ;
Time (3,16) =    0.0063168 ;
Time (3,32) =   0.00665607 ;
Time (3,64) =      0.01757 ;
Time (3,128) =     0.077788 ;
Time (3,160) =    0.0852325 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 31036 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0632131 ;
Time (3,2) =    0.0275127 ;
Time (3,4) =    0.0214701 ;
Time (3,8) =    0.0208751 ;
Time (3,16) =    0.0217879 ;
Time (3,32) =    0.0323732 ;
Time (3,64) =    0.0934431 ;
Time (3,128) =     0.213762 ;
Time (3,160) =     0.199714 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 31036 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0271779 ;
Time (3,2) =    0.0107503 ;
Time (3,4) =   0.00771933 ;
Time (3,8) =   0.00573907 ;
Time (3,16) =    0.0101537 ;
Time (3,32) =   0.00947774 ;
Time (3,64) =    0.0298129 ;
Time (3,128) =    0.0549244 ;
Time (3,160) =     0.122501 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella09/p2p-Gnutella09_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8114 ;
Nedges (id) = 26013 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0114452 ;
Time (3,2) =   0.00148217 ;
Time (3,4) =  0.000964352 ;
Time (3,8) =   0.00105056 ;
Time (3,16) =   0.00136643 ;
Time (3,32) =   0.00182524 ;
Time (3,64) =    0.0044075 ;
Time (3,128) =    0.0560472 ;
Time (3,160) =    0.0420656 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-HepTh/ca-HepTh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 9877 ;
Nedges (id) = 25973 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0137658 ;
Time (3,2) =   0.00266758 ;
Time (3,4) =   0.00176585 ;
Time (3,8) =   0.00144993 ;
Time (3,16) =   0.00173814 ;
Time (3,32) =   0.00261788 ;
Time (3,64) =   0.00589375 ;
Time (3,128) =    0.0804297 ;
Time (3,160) =    0.0654507 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010331/oregon2_010331_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10900 ;
Nedges (id) = 31180 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0344619 ;
Time (3,2) =    0.0131368 ;
Time (3,4) =   0.00795486 ;
Time (3,8) =   0.00773155 ;
Time (3,16) =   0.00759175 ;
Time (3,32) =    0.0113145 ;
Time (3,64) =    0.0249462 ;
Time (3,128) =    0.0836437 ;
Time (3,160) =     0.118184 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010407/oregon2_010407_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10981 ;
Nedges (id) = 30855 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0340289 ;
Time (3,2) =    0.0130474 ;
Time (3,4) =   0.00741012 ;
Time (3,8) =   0.00773314 ;
Time (3,16) =    0.0103881 ;
Time (3,32) =     0.010282 ;
Time (3,64) =    0.0237304 ;
Time (3,128) =    0.0759865 ;
Time (3,160) =    0.0843679 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010505/oregon2_010505_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11157 ;
Nedges (id) = 30943 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.034532 ;
Time (3,2) =    0.0130711 ;
Time (3,4) =   0.00785039 ;
Time (3,8) =   0.00711422 ;
Time (3,16) =   0.00802866 ;
Time (3,32) =    0.0115709 ;
Time (3,64) =    0.0210987 ;
Time (3,128) =    0.0802963 ;
Time (3,160) =    0.0654876 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010414/oregon2_010414_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11019 ;
Nedges (id) = 31761 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0352735 ;
Time (3,2) =    0.0135602 ;
Time (3,4) =   0.00793724 ;
Time (3,8) =   0.00965449 ;
Time (3,16) =    0.0101247 ;
Time (3,32) =    0.0164656 ;
Time (3,64) =     0.025777 ;
Time (3,128) =     0.087777 ;
Time (3,160) =    0.0937295 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010421/oregon2_010421_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11080 ;
Nedges (id) = 31538 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0350937 ;
Time (3,2) =    0.0134759 ;
Time (3,4) =   0.00864785 ;
Time (3,8) =   0.00817608 ;
Time (3,16) =   0.00927384 ;
Time (3,32) =    0.0133533 ;
Time (3,64) =    0.0229889 ;
Time (3,128) =    0.0969181 ;
Time (3,160) =     0.100603 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010428/oregon2_010428_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11113 ;
Nedges (id) = 31434 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0350835 ;
Time (3,2) =    0.0133177 ;
Time (3,4) =   0.00783251 ;
Time (3,8) =   0.00923822 ;
Time (3,16) =   0.00773925 ;
Time (3,32) =   0.00979985 ;
Time (3,64) =    0.0263586 ;
Time (3,128) =    0.0781548 ;
Time (3,160) =    0.0886359 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010512/oregon2_010512_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11260 ;
Nedges (id) = 31303 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0349412 ;
Time (3,2) =     0.013279 ;
Time (3,4) =   0.00781125 ;
Time (3,8) =   0.00689378 ;
Time (3,16) =   0.00941735 ;
Time (3,32) =    0.0149195 ;
Time (3,64) =    0.0224141 ;
Time (3,128) =    0.0643801 ;
Time (3,160) =    0.0921711 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010519/oregon2_010519_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11375 ;
Nedges (id) = 32287 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0363285 ;
Time (3,2) =    0.0141951 ;
Time (3,4) =   0.00863549 ;
Time (3,8) =   0.00753869 ;
Time (3,16) =   0.00881576 ;
Time (3,32) =    0.0119122 ;
Time (3,64) =    0.0215327 ;
Time (3,128) =    0.0610501 ;
Time (3,160) =     0.080623 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41809 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.424892 ;
Time (3,2) =     0.215857 ;
Time (3,4) =     0.121412 ;
Time (3,8) =    0.0699866 ;
Time (3,16) =    0.0501131 ;
Time (3,32) =    0.0922802 ;
Time (3,64) =     0.147886 ;
Time (3,128) =     0.373349 ;
Time (3,160) =      0.43748 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41472 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.404606 ;
Time (3,2) =     0.206535 ;
Time (3,4) =     0.112856 ;
Time (3,8) =    0.0685401 ;
Time (3,16) =    0.0490118 ;
Time (3,32) =    0.0809567 ;
Time (3,64) =     0.122157 ;
Time (3,128) =      0.31567 ;
Time (3,160) =      0.35597 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010526/oregon2_010526_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11461 ;
Nedges (id) = 32730 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.037297 ;
Time (3,2) =    0.0148454 ;
Time (3,4) =    0.0087565 ;
Time (3,8) =   0.00621034 ;
Time (3,16) =   0.00995113 ;
Time (3,32) =   0.00951845 ;
Time (3,64) =    0.0224472 ;
Time (3,128) =    0.0707122 ;
Time (3,160) =    0.0923302 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella06/p2p-Gnutella06_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8717 ;
Nedges (id) = 31525 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0115746 ;
Time (3,2) =    0.0015459 ;
Time (3,4) =   0.00102421 ;
Time (3,8) =  0.000910257 ;
Time (3,16) =   0.00120355 ;
Time (3,32) =   0.00172182 ;
Time (3,64) =   0.00467277 ;
Time (3,128) =    0.0287641 ;
Time (3,160) =    0.0639212 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41809 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.405307 ;
Time (3,2) =     0.206544 ;
Time (3,4) =     0.113246 ;
Time (3,8) =    0.0688813 ;
Time (3,16) =     0.044171 ;
Time (3,32) =     0.104955 ;
Time (3,64) =     0.114203 ;
Time (3,128) =     0.309948 ;
Time (3,160) =     0.315535 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella05/p2p-Gnutella05_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8846 ;
Nedges (id) = 31839 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0116155 ;
Time (3,2) =   0.00160128 ;
Time (3,4) =    0.0010952 ;
Time (3,8) =  0.000999094 ;
Time (3,16) =    0.0013684 ;
Time (3,32) =   0.00198843 ;
Time (3,64) =   0.00489317 ;
Time (3,128) =    0.0174483 ;
Time (3,160) =    0.0536915 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella04/p2p-Gnutella04_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10876 ;
Nedges (id) = 39994 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0121991 ;
Time (3,2) =     0.001835 ;
Time (3,4) =   0.00126286 ;
Time (3,8) =   0.00102255 ;
Time (3,16) =   0.00141839 ;
Time (3,32) =   0.00210975 ;
Time (3,64) =   0.00547105 ;
Time (3,128) =    0.0600371 ;
Time (3,160) =     0.073001 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/as-caida20071105/as-caida20071105_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26475 ;
Nedges (id) = 53381 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0557073 ;
Time (3,2) =     0.023872 ;
Time (3,4) =    0.0141424 ;
Time (3,8) =    0.0106311 ;
Time (3,16) =   0.00817117 ;
Time (3,32) =    0.0109534 ;
Time (3,64) =    0.0218789 ;
Time (3,128) =    0.0738658 ;
Time (3,160) =      0.11388 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella25/p2p-Gnutella25_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 22687 ;
Nedges (id) = 54705 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0131922 ;
Time (3,2) =   0.00250216 ;
Time (3,4) =   0.00169912 ;
Time (3,8) =   0.00145138 ;
Time (3,16) =   0.00135428 ;
Time (3,32) =   0.00204432 ;
Time (3,64) =   0.00485568 ;
Time (3,128) =    0.0500168 ;
Time (3,160) =     0.073106 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella24/p2p-Gnutella24_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26518 ;
Nedges (id) = 65369 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0145301 ;
Time (3,2) =   0.00303022 ;
Time (3,4) =    0.0018645 ;
Time (3,8) =   0.00142159 ;
Time (3,16) =   0.00157181 ;
Time (3,32) =   0.00240084 ;
Time (3,64) =   0.00582789 ;
Time (3,128) =    0.0420556 ;
Time (3,160) =    0.0499297 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/facebook_combined/facebook_combined_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4039 ;
Nedges (id) = 88234 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0829269 ;
Time (3,2) =    0.0386986 ;
Time (3,4) =    0.0329655 ;
Time (3,8) =    0.0671025 ;
Time (3,16) =    0.0410689 ;
Time (3,32) =    0.0655248 ;
Time (3,64) =     0.124667 ;
Time (3,128) =     0.232954 ;
Time (3,160) =     0.176462 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 129600 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.07297 ;
Time (3,2) =       0.5354 ;
Time (3,4) =     0.273544 ;
Time (3,8) =     0.152927 ;
Time (3,16) =      0.12094 ;
Time (3,32) =     0.110496 ;
Time (3,64) =     0.182403 ;
Time (3,128) =     0.408975 ;
Time (3,160) =     0.447585 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella30/p2p-Gnutella30_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36682 ;
Nedges (id) = 88328 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0161454 ;
Time (3,2) =   0.00403768 ;
Time (3,4) =    0.0026848 ;
Time (3,8) =   0.00218384 ;
Time (3,16) =   0.00220357 ;
Time (3,32) =   0.00299581 ;
Time (3,64) =   0.00756641 ;
Time (3,128) =    0.0603206 ;
Time (3,160) =     0.038889 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 137164 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       1.4719 ;
Time (3,2) =     0.736075 ;
Time (3,4) =     0.393926 ;
Time (3,8) =     0.222933 ;
Time (3,16) =     0.195796 ;
Time (3,32) =     0.197979 ;
Time (3,64) =     0.319613 ;
Time (3,128) =       1.0137 ;
Time (3,160) =      1.63547 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 137164 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.09014 ;
Time (3,2) =     0.543668 ;
Time (3,4) =     0.279777 ;
Time (3,8) =     0.156154 ;
Time (3,16) =     0.121129 ;
Time (3,32) =     0.112099 ;
Time (3,64) =     0.185972 ;
Time (3,128) =     0.533725 ;
Time (3,160) =     0.568171 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 138240 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.171924 ;
Time (3,2) =    0.0830894 ;
Time (3,4) =      0.04542 ;
Time (3,8) =    0.0311353 ;
Time (3,16) =    0.0218536 ;
Time (3,32) =    0.0342395 ;
Time (3,64) =    0.0522495 ;
Time (3,128) =      0.26398 ;
Time (3,160) =     0.224875 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 144000 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      0.44432 ;
Time (3,2) =     0.221915 ;
Time (3,4) =     0.116238 ;
Time (3,8) =    0.0679478 ;
Time (3,16) =    0.0638867 ;
Time (3,32) =    0.0723454 ;
Time (3,64) =     0.101777 ;
Time (3,128) =     0.351747 ;
Time (3,160) =     0.373349 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-CondMat/ca-CondMat_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 23133 ;
Nedges (id) = 93439 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0348109 ;
Time (3,2) =    0.0138121 ;
Time (3,4) =   0.00830936 ;
Time (3,8) =   0.00509025 ;
Time (3,16) =   0.00430992 ;
Time (3,32) =   0.00664546 ;
Time (3,64) =    0.0130467 ;
Time (3,128) =    0.0636529 ;
Time (3,160) =    0.0667354 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-HepPh/ca-HepPh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 12008 ;
Nedges (id) = 118489 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.112903 ;
Time (3,2) =    0.0538657 ;
Time (3,4) =     0.039751 ;
Time (3,8) =    0.0297696 ;
Time (3,16) =    0.0251605 ;
Time (3,32) =    0.0422601 ;
Time (3,64) =    0.0812053 ;
Time (3,128) =     0.190999 ;
Time (3,160) =     0.192849 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 175873 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.17884 ;
Time (3,2) =     0.593964 ;
Time (3,4) =       0.3194 ;
Time (3,8) =     0.199798 ;
Time (3,16) =     0.144582 ;
Time (3,32) =     0.221978 ;
Time (3,64) =     0.543958 ;
Time (3,128) =     0.946298 ;
Time (3,160) =      1.11539 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 175873 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.512033 ;
Time (3,2) =     0.256895 ;
Time (3,4) =     0.133251 ;
Time (3,8) =    0.0809736 ;
Time (3,16) =    0.0776793 ;
Time (3,32) =    0.0650504 ;
Time (3,64) =     0.126733 ;
Time (3,128) =      0.37101 ;
Time (3,160) =     0.467928 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella31/p2p-Gnutella31_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 62586 ;
Nedges (id) = 147892 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0212309 ;
Time (3,2) =   0.00675853 ;
Time (3,4) =   0.00436349 ;
Time (3,8) =   0.00311087 ;
Time (3,16) =   0.00269288 ;
Time (3,32) =   0.00473273 ;
Time (3,64) =   0.00828711 ;
Time (3,128) =    0.0440965 ;
Time (3,160) =    0.0654344 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 217255 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.278105 ;
Time (3,2) =      0.14338 ;
Time (3,4) =    0.0773842 ;
Time (3,8) =    0.0488485 ;
Time (3,16) =    0.0365784 ;
Time (3,32) =    0.0741512 ;
Time (3,64) =     0.123271 ;
Time (3,128) =     0.429206 ;
Time (3,160) =     0.330997 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 217255 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.13494 ;
Time (3,2) =     0.563388 ;
Time (3,4) =     0.300551 ;
Time (3,8) =     0.172071 ;
Time (3,16) =     0.160704 ;
Time (3,32) =      0.31631 ;
Time (3,64) =     0.839502 ;
Time (3,128) =     0.996806 ;
Time (3,160) =      1.07397 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/email-Enron/email-Enron_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36692 ;
Nedges (id) = 183831 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      0.14495 ;
Time (3,2) =    0.0691044 ;
Time (3,4) =    0.0397681 ;
Time (3,8) =     0.035312 ;
Time (3,16) =    0.0320414 ;
Time (3,32) =    0.0364893 ;
Time (3,64) =    0.0811839 ;
Time (3,128) =     0.168704 ;
Time (3,160) =     0.197436 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-260610-65536_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 65536 ;
Nedges (id) = 260610 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0155287 ;
Time (3,2) =   0.00361639 ;
Time (3,4) =   0.00255196 ;
Time (3,8) =   0.00202858 ;
Time (3,16) =    0.0021249 ;
Time (3,32) =   0.00521332 ;
Time (3,64) =     0.012165 ;
Time (3,128) =    0.0526173 ;
Time (3,160) =     0.114801 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-AstroPh/ca-AstroPh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 18772 ;
Nedges (id) = 198050 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.123682 ;
Time (3,2) =     0.059907 ;
Time (3,4) =    0.0362101 ;
Time (3,8) =     0.024727 ;
Time (3,16) =    0.0174631 ;
Time (3,32) =    0.0230717 ;
Time (3,64) =     0.049217 ;
Time (3,128) =    0.0945168 ;
Time (3,160) =     0.147992 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/loc-brightkite_edges/loc-brightkite_edges_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 58228 ;
Nedges (id) = 214078 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0923406 ;
Time (3,2) =      0.04322 ;
Time (3,4) =    0.0290134 ;
Time (3,8) =     0.020673 ;
Time (3,16) =    0.0203094 ;
Time (3,32) =    0.0225374 ;
Time (3,64) =    0.0548241 ;
Time (3,128) =     0.111185 ;
Time (3,160) =     0.138319 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320000 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       23.297 ;
Time (3,2) =      11.7144 ;
Time (3,4) =      5.93802 ;
Time (3,8) =      3.04842 ;
Time (3,16) =      1.66077 ;
Time (3,32) =      1.29272 ;
Time (3,64) =      1.64575 ;
Time (3,128) =      1.27518 ;
Time (3,160) =      1.51235 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320881 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      23.7941 ;
Time (3,2) =      11.8744 ;
Time (3,4) =       6.0965 ;
Time (3,8) =      3.19055 ;
Time (3,16) =      1.75373 ;
Time (3,32) =      1.14884 ;
Time (3,64) =      1.19439 ;
Time (3,128) =      1.93557 ;
Time (3,160) =      1.97322 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320881 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       23.464 ;
Time (3,2) =      11.7307 ;
Time (3,4) =      5.91425 ;
Time (3,8) =      3.02102 ;
Time (3,16) =      1.64959 ;
Time (3,32) =      1.29378 ;
Time (3,64) =      1.72571 ;
Time (3,128) =      1.20806 ;
Time (3,160) =      1.18098 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-HepTh/cit-HepTh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 27770 ;
Nedges (id) = 352285 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.249466 ;
Time (3,2) =      0.12286 ;
Time (3,4) =    0.0654386 ;
Time (3,8) =    0.0421039 ;
Time (3,16) =     0.038696 ;
Time (3,32) =    0.0388652 ;
Time (3,64) =    0.0853796 ;
Time (3,128) =     0.199401 ;
Time (3,160) =     0.220284 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Epinions1/soc-Epinions1_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 75879 ;
Nedges (id) = 405740 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.378663 ;
Time (3,2) =     0.189904 ;
Time (3,4) =     0.100851 ;
Time (3,8) =     0.073682 ;
Time (3,16) =    0.0617277 ;
Time (3,32) =    0.0746577 ;
Time (3,64) =     0.127795 ;
Time (3,128) =      0.23943 ;
Time (3,160) =     0.311636 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/email-EuAll/email-EuAll_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 265214 ;
Nedges (id) = 364481 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.516913 ;
Time (3,2) =     0.236714 ;
Time (3,4) =     0.121707 ;
Time (3,8) =    0.0821668 ;
Time (3,16) =    0.0493099 ;
Time (3,32) =    0.0522339 ;
Time (3,64) =    0.0745565 ;
Time (3,128) =     0.140877 ;
Time (3,160) =     0.191922 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-HepPh/cit-HepPh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 34546 ;
Nedges (id) = 420877 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      0.21981 ;
Time (3,2) =     0.107105 ;
Time (3,4) =    0.0593083 ;
Time (3,8) =    0.0356224 ;
Time (3,16) =     0.029265 ;
Time (3,32) =    0.0309938 ;
Time (3,64) =    0.0688174 ;
Time (3,128) =     0.138544 ;
Time (3,160) =     0.186235 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Slashdot0811/soc-Slashdot0811_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 77360 ;
Nedges (id) = 469180 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.282259 ;
Time (3,2) =     0.139931 ;
Time (3,4) =    0.0862256 ;
Time (3,8) =    0.0574889 ;
Time (3,16) =    0.0496384 ;
Time (3,32) =    0.0723133 ;
Time (3,64) =     0.103803 ;
Time (3,128) =     0.163829 ;
Time (3,160) =     0.237065 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Slashdot0902/soc-Slashdot0902_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 82168 ;
Nedges (id) = 504230 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.313408 ;
Time (3,2) =     0.155184 ;
Time (3,4) =    0.0960056 ;
Time (3,8) =    0.0758768 ;
Time (3,16) =    0.0634018 ;
Time (3,32) =    0.0586729 ;
Time (3,64) =     0.109524 ;
Time (3,128) =      0.25787 ;
Time (3,160) =     0.213493 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-1045506-262144_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 262144 ;
Nedges (id) = 1045506 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =    0.0354106 ;
Time (3,2) =    0.0148174 ;
Time (3,4) =    0.0104366 ;
Time (3,8) =   0.00814945 ;
Time (3,16) =   0.00699974 ;
Time (3,32) =    0.0108724 ;
Time (3,64) =    0.0416461 ;
Time (3,128) =    0.0688723 ;
Time (3,160) =      0.10792 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1152000 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      8.70432 ;
Time (3,2) =      4.36394 ;
Time (3,4) =      2.21899 ;
Time (3,8) =       1.1934 ;
Time (3,16) =     0.610236 ;
Time (3,32) =     0.506022 ;
Time (3,64) =     0.575578 ;
Time (3,128) =     0.634331 ;
Time (3,160) =     0.911112 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/loc-gowalla_edges/loc-gowalla_edges_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 196591 ;
Nedges (id) = 950327 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.16977 ;
Time (3,2) =      0.58268 ;
Time (3,4) =     0.301335 ;
Time (3,8) =     0.187086 ;
Time (3,16) =      0.11282 ;
Time (3,32) =     0.102785 ;
Time (3,64) =     0.192702 ;
Time (3,128) =     0.406443 ;
Time (3,160) =     0.513417 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0302/amazon0302_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 262111 ;
Nedges (id) = 899792 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.232935 ;
Time (3,2) =     0.113496 ;
Time (3,4) =    0.0694247 ;
Time (3,8) =    0.0441284 ;
Time (3,16) =    0.0303421 ;
Time (3,32) =    0.0347701 ;
Time (3,64) =    0.0748737 ;
Time (3,128) =     0.177231 ;
Time (3,160) =     0.241243 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1582861 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       37.825 ;
Time (3,2) =      18.9997 ;
Time (3,4) =      9.64983 ;
Time (3,8) =      4.90901 ;
Time (3,16) =      2.77205 ;
Time (3,32) =      2.19433 ;
Time (3,64) =      2.72084 ;
Time (3,128) =      6.68183 ;
Time (3,160) =      8.43276 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1582861 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      11.6148 ;
Time (3,2) =      5.80791 ;
Time (3,4) =      2.93472 ;
Time (3,8) =      1.54341 ;
Time (3,16) =     0.913135 ;
Time (3,32) =     0.834372 ;
Time (3,64) =      1.20184 ;
Time (3,128) =     0.980878 ;
Time (3,160) =     0.983866 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2073600 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      258.696 ;
Time (3,2) =      129.815 ;
Time (3,4) =       64.914 ;
Time (3,8) =      32.8326 ;
Time (3,16) =      17.0576 ;
Time (3,32) =      10.1346 ;
Time (3,64) =      8.58177 ;
Time (3,128) =      8.83703 ;
Time (3,160) =      9.00754 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2132284 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      307.322 ;
Time (3,2) =      154.592 ;
Time (3,4) =      78.2784 ;
Time (3,8) =      39.9975 ;
Time (3,16) =      20.9503 ;
Time (3,32) =      12.3974 ;
Time (3,64) =      11.5522 ;
Time (3,128) =      14.1713 ;
Time (3,160) =      16.4062 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2132284 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      260.039 ;
Time (3,2) =      130.563 ;
Time (3,4) =      65.1224 ;
Time (3,8) =      33.1858 ;
Time (3,16) =      17.2139 ;
Time (3,32) =      10.2627 ;
Time (3,64) =      8.68935 ;
Time (3,128) =      9.00885 ;
Time (3,160) =      10.2271 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-PA/roadNet-PA_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1088092 ;
Nedges (id) = 1541898 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.103607 ;
Time (3,2) =     0.055436 ;
Time (3,4) =    0.0373133 ;
Time (3,8) =     0.028405 ;
Time (3,16) =    0.0263944 ;
Time (3,32) =    0.0363534 ;
Time (3,64) =     0.101064 ;
Time (3,128) =     0.186402 ;
Time (3,160) =      0.16538 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2332800 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       96.682 ;
Time (3,2) =      48.0909 ;
Time (3,4) =      24.0927 ;
Time (3,8) =      12.2796 ;
Time (3,16) =      6.43946 ;
Time (3,32) =      4.01594 ;
Time (3,64) =       3.6777 ;
Time (3,128) =       4.3499 ;
Time (3,160) =         4.16 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2606125 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       174.92 ;
Time (3,2) =      87.7473 ;
Time (3,4) =      44.1468 ;
Time (3,8) =      22.5601 ;
Time (3,16) =      11.9493 ;
Time (3,32) =      8.94347 ;
Time (3,64) =      7.56079 ;
Time (3,128) =      13.2631 ;
Time (3,160) =      15.3724 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2606125 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      100.664 ;
Time (3,2) =      50.3916 ;
Time (3,4) =      25.4214 ;
Time (3,8) =      12.9789 ;
Time (3,16) =      6.79805 ;
Time (3,32) =      4.56349 ;
Time (3,64) =      3.38121 ;
Time (3,128) =      4.78048 ;
Time (3,160) =      4.98812 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-TX/roadNet-TX_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1379917 ;
Nedges (id) = 1921660 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      0.12704 ;
Time (3,2) =    0.0687145 ;
Time (3,4) =    0.0493576 ;
Time (3,8) =    0.0405297 ;
Time (3,16) =    0.0339601 ;
Time (3,32) =    0.0461492 ;
Time (3,64) =    0.0985398 ;
Time (3,128) =     0.177048 ;
Time (3,160) =     0.193628 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-4188162-1048576_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1048576 ;
Nedges (id) = 4188162 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.114454 ;
Time (3,2) =    0.0581869 ;
Time (3,4) =     0.035344 ;
Time (3,8) =    0.0268763 ;
Time (3,16) =    0.0248465 ;
Time (3,32) =    0.0379802 ;
Time (3,64) =     0.120381 ;
Time (3,128) =     0.230657 ;
Time (3,160) =      0.29132 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/flickrEdges/flickrEdges_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 105938 ;
Nedges (id) = 2316948 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      6.43685 ;
Time (3,2) =      3.41784 ;
Time (3,4) =      1.66746 ;
Time (3,8) =     0.908039 ;
Time (3,16) =     0.661907 ;
Time (3,32) =     0.706622 ;
Time (3,64) =      1.23461 ;
Time (3,128) =      2.06068 ;
Time (3,160) =      2.08551 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0312/amazon0312_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 400727 ;
Nedges (id) = 2349869 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.02132 ;
Time (3,2) =     0.505187 ;
Time (3,4) =     0.256302 ;
Time (3,8) =     0.152104 ;
Time (3,16) =     0.108643 ;
Time (3,32) =      0.15077 ;
Time (3,64) =     0.278829 ;
Time (3,128) =     0.450839 ;
Time (3,160) =     0.447454 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0505/amazon0505_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 410236 ;
Nedges (id) = 2439437 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.03952 ;
Time (3,2) =     0.506617 ;
Time (3,4) =     0.288364 ;
Time (3,8) =     0.161769 ;
Time (3,16) =     0.120066 ;
Time (3,32) =     0.161457 ;
Time (3,64) =     0.284179 ;
Time (3,128) =     0.452662 ;
Time (3,160) =     0.486259 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0601/amazon0601_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 403394 ;
Nedges (id) = 2443408 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.04985 ;
Time (3,2) =     0.508595 ;
Time (3,4) =     0.268794 ;
Time (3,8) =     0.164888 ;
Time (3,16) =     0.115409 ;
Time (3,32) =      0.13204 ;
Time (3,64) =     0.228005 ;
Time (3,128) =     0.449229 ;
Time (3,160) =     0.429259 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-CA/roadNet-CA_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1965206 ;
Nedges (id) = 2766607 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      0.18426 ;
Time (3,2) =     0.102545 ;
Time (3,4) =    0.0741183 ;
Time (3,8) =    0.0527419 ;
Time (3,16) =    0.0479753 ;
Time (3,32) =    0.0657046 ;
Time (3,64) =     0.182883 ;
Time (3,128) =     0.302614 ;
Time (3,160) =     0.325729 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale18-ef16/graph500-scale18-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 174147 ;
Nedges (id) = 3800348 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       24.922 ;
Time (3,2) =      12.9534 ;
Time (3,4) =      6.86452 ;
Time (3,8) =      3.58636 ;
Time (3,16) =      2.17883 ;
Time (3,32) =      1.99656 ;
Time (3,64) =      3.89761 ;
Time (3,128) =      5.11734 ;
Time (3,160) =      5.91206 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 6912000 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      104.125 ;
Time (3,2) =      52.1546 ;
Time (3,4) =      26.3397 ;
Time (3,8) =      13.3098 ;
Time (3,16) =      6.95511 ;
Time (3,32) =      4.32689 ;
Time (3,64) =      3.88691 ;
Time (3,128) =      4.17159 ;
Time (3,160) =      4.78732 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 11080030 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      825.793 ;
Time (3,2) =      429.825 ;
Time (3,4) =      222.732 ;
Time (3,8) =      115.923 ;
Time (3,16) =      61.6138 ;
Time (3,32) =       38.226 ;
Time (3,64) =      33.3867 ;
Time (3,128) =      47.5497 ;
Time (3,160) =      62.1349 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 11080030 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      173.763 ;
Time (3,2) =      87.0633 ;
Time (3,4) =      44.0309 ;
Time (3,8) =      22.4716 ;
Time (3,16) =      11.8237 ;
Time (3,32) =      7.86624 ;
Time (3,64) =      7.26816 ;
Time (3,128) =      8.88717 ;
Time (3,160) =      9.60792 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale19-ef16/graph500-scale19-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 335318 ;
Nedges (id) = 7729675 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      73.5842 ;
Time (3,2) =      37.3659 ;
Time (3,4) =      19.8035 ;
Time (3,8) =      10.5685 ;
Time (3,16) =      5.87693 ;
Time (3,32) =      4.96113 ;
Time (3,64) =      6.67414 ;
Time (3,128) =      11.1768 ;
Time (3,160) =      12.5976 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-16764930-4194304_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4194304 ;
Nedges (id) = 16764930 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =     0.454083 ;
Time (3,2) =       0.2334 ;
Time (3,4) =     0.144168 ;
Time (3,8) =     0.135219 ;
Time (3,16) =     0.100972 ;
Time (3,32) =     0.142511 ;
Time (3,64) =     0.567446 ;
Time (3,128) =     0.748592 ;
Time (3,160) =     0.829623 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 23328000 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      3113.02 ;
Time (3,2) =      1658.01 ;
Time (3,4) =      915.452 ;
Time (3,8) =      530.099 ;
Time (3,16) =      352.964 ;
Time (3,32) =      203.464 ;
Time (3,64) =      141.395 ;
Time (3,128) =      107.282 ;
Time (3,160) =      98.9674 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale20-ef16/graph500-scale20-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 645820 ;
Nedges (id) = 15680861 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       220.68 ;
Time (3,2) =      118.286 ;
Time (3,4) =      59.5588 ;
Time (3,8) =      30.4544 ;
Time (3,16) =      16.5858 ;
Time (3,32) =      12.7103 ;
Time (3,64) =      14.0411 ;
Time (3,128) =      24.8719 ;
Time (3,160) =      34.4406 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 28667380 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       3602.4 ;
Time (3,2) =      1942.72 ;
Time (3,4) =      1064.95 ;
Time (3,8) =      617.275 ;
Time (3,16) =      416.479 ;
Time (3,32) =      243.668 ;
Time (3,64) =      169.829 ;
Time (3,128) =      133.056 ;
Time (3,160) =        125.7 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 28667380 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      8960.97 ;
Time (3,2) =      4942.88 ;
Time (3,4) =      2679.23 ;
Time (3,8) =       1525.7 ;
Time (3,16) =      891.151 ;
Time (3,32) =      584.092 ;
Time (3,64) =      429.999 ;
Time (3,128) =      343.202 ;
Time (3,160) =      344.391 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-Patents/cit-Patents_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 3774768 ;
Nedges (id) = 16518947 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      11.6535 ;
Time (3,2) =      6.19726 ;
Time (3,4) =      3.34655 ;
Time (3,8) =       2.6094 ;
Time (3,16) =      2.25554 ;
Time (3,32) =      1.43684 ;
Time (3,64) =      1.89783 ;
Time (3,128) =      2.84095 ;
Time (3,160) =       3.1777 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale21-ef16/graph500-scale21-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1243072 ;
Nedges (id) = 31731650 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =       685.09 ;
Time (3,2) =      349.387 ;
Time (3,4) =      177.251 ;
Time (3,8) =      90.0703 ;
Time (3,16) =       50.497 ;
Time (3,32) =      39.5101 ;
Time (3,64) =      53.4911 ;
Time (3,128) =      113.401 ;
Time (3,160) =      148.216 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-67084290-16777216_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 16777216 ;
Nedges (id) = 67084290 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1.69677 ;
Time (3,2) =     0.944051 ;
Time (3,4) =     0.627936 ;
Time (3,8) =     0.414705 ;
Time (3,16) =     0.569987 ;
Time (3,32) =     0.335902 ;
Time (3,64) =      1.11754 ;
Time (3,128) =      3.33592 ;
Time (3,160) =      3.52626 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale22-ef16/graph500-scale22-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2393285 ;
Nedges (id) = 64097004 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      1988.12 ;
Time (3,2) =       1008.6 ;
Time (3,4) =      513.907 ;
Time (3,8) =      263.004 ;
Time (3,16) =      145.684 ;
Time (3,32) =      131.288 ;
Time (3,64) =      235.283 ;
Time (3,128) =       604.12 ;
Time (3,160) =      732.397 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/V2a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 55042369 ;
Nedges (id) = 58608800 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      6.10816 ;
Time (3,2) =      3.43691 ;
Time (3,4) =      2.23954 ;
Time (3,8) =      1.98498 ;
Time (3,16) =      1.89596 ;
Time (3,32) =      2.20569 ;
Time (3,64) =      3.36682 ;
Time (3,128) =       4.8354 ;
Time (3,160) =      6.16523 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/U1a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 67716231 ;
Nedges (id) = 69389281 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      7.28726 ;
Time (3,2) =      4.22466 ;
Time (3,4) =      2.67605 ;
Time (3,8) =      2.05956 ;
Time (3,16) =      2.00367 ;
Time (3,32) =      2.90813 ;
Time (3,64) =       2.7785 ;
Time (3,128) =      6.82208 ;
Time (3,160) =      8.21771 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale23-ef16/graph500-scale23-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4606314 ;
Nedges (id) = 129250705 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      6063.94 ;
Time (3,2) =      3140.55 ;
Time (3,4) =      1581.43 ;
Time (3,8) =       815.23 ;
Time (3,16) =      465.981 ;
Time (3,32) =      589.308 ;
Time (3,64) =      1456.19 ;
Time (3,128) =      2522.63 ;
Time (3,160) =       2725.3 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-268386306-67108864_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 67108864 ;
Nedges (id) = 268386306 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      6.90681 ;
Time (3,2) =      4.05308 ;
Time (3,4) =      2.29494 ;
Time (3,8) =       1.6449 ;
Time (3,16) =      1.40724 ;
Time (3,32) =      2.24847 ;
Time (3,64) =      7.63115 ;
Time (3,128) =      9.43306 ;
Time (3,160) =      11.7424 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/P1a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 139353211 ;
Nedges (id) = 148914992 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      17.6988 ;
Time (3,2) =      8.78384 ;
Time (3,4) =      5.65644 ;
Time (3,8) =      4.87841 ;
Time (3,16) =       4.7557 ;
Time (3,32) =      5.37712 ;
Time (3,64) =       8.7399 ;
Time (3,128) =      11.7733 ;
Time (3,160) =      15.3016 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/A2a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170728175 ;
Nedges (id) = 180292586 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      21.0891 ;
Time (3,2) =      11.8599 ;
Time (3,4) =      6.47449 ;
Time (3,8) =      5.14339 ;
Time (3,16) =      5.14708 ;
Time (3,32) =      5.59176 ;
Time (3,64) =      8.36305 ;
Time (3,128) =      16.0435 ;
Time (3,160) =      19.8863 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/V1r.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 214005017 ;
Nedges (id) = 232705452 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      12.5895 ;
Time (3,2) =      7.61762 ;
Time (3,4) =      4.68552 ;
Time (3,8) =      3.86778 ;
Time (3,16) =      4.10119 ;
Time (3,32) =      5.16586 ;
Time (3,64) =      10.9733 ;
Time (3,128) =      14.3186 ;
Time (3,160) =      19.1759 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale24-ef16/graph500-scale24-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8860450 ;
Nedges (id) = 260261843 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      21084.2 ;
Time (3,2) =      10840.8 ;
Time (3,4) =      5606.41 ;
Time (3,8) =      3156.66 ;
Time (3,16) =      2524.42 ;
Time (3,32) =      3616.42 ;
Time (3,64) =       6719.1 ;
Time (3,128) =      8621.23 ;
Time (3,160) =      8898.15 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-1073643522-268435456_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 268435456 ;
Nedges (id) = 1073643522 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      27.7898 ;
Time (3,2) =      15.1516 ;
Time (3,4) =      10.0062 ;
Time (3,8) =      6.38577 ;
Time (3,16) =      5.37024 ;
Time (3,32) =        8.474 ;
Time (3,64) =      23.7901 ;
Time (3,128) =       27.713 ;
Time (3,160) =      41.0829 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale25-ef16/graph500-scale25-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 17043780 ;
Nedges (id) = 523467448 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      83095.7 ;
Time (3,2) =      39708.7 ;
Time (3,4) =      20373.3 ;
Time (3,8) =      15444.2 ;
Time (3,16) =      15598.8 ;
Time (3,32) =      18803.5 ;
Time (3,64) =      23338.7 ;
Time (3,128) =      26184.4 ;
Time (3,160) =      26433.3 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/friendster/friendster_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 119432957 ;
Nedges (id) = 1799999986 ;
Time  = nan (2,160) ;
% Time (3:kmax, nthreads) = time for each k-truss
Time = [Time ; nan(1,160)] ;
Time (3,1) =      10564.4 ;
Time (3,2) =      5206.03 ;
Time (3,4) =      3066.44 ;
Time (3,8) =      2617.21 ;
Time (3,16) =      3149.77 ;
Time (3,32) =      2152.89 ;
Time (3,64) =      1834.79 ;
Time (3,128) =      2017.54 ;
Time (3,160) =      2033.32 ;
T {id} = Time ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc5/201512012345.v18571154_e38040320.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020000.v35991342_e74485420.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020030.v68863315_e143414960.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020130.v128568730_e270234840.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020330.v226196185_e480047894.tsv.gz ';
id = id + 1 ;
