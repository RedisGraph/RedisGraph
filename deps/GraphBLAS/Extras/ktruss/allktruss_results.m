function [T, File, N, Nedges, Kmax] = allktruss_results
id = 0 ;
file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 841 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00896674 ;
T (1,2,id) =  0.000258044 ;
T (1,4,id) =  0.000257569 ;
T (1,8,id) =    0.0002575 ;
T (1,16,id) =  0.000257447 ;
T (1,32,id) =  0.000295462 ;
T (1,64,id) =  0.000257779 ;
T (1,128,id) =  0.000257222 ;
T (1,160,id) =  0.000257251 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 841 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00891291 ;
T (1,2,id) =  0.000196568 ;
T (1,4,id) =  0.000196267 ;
T (1,8,id) =   0.00019623 ;
T (1,16,id) =  0.000196001 ;
T (1,32,id) =  0.000196031 ;
T (1,64,id) =  0.000195955 ;
T (1,128,id) =  0.000196015 ;
T (1,160,id) =  0.000195905 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 800 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00893932 ;
T (1,2,id) =   0.00018553 ;
T (1,4,id) =  0.000185328 ;
T (1,8,id) =  0.000185322 ;
T (1,16,id) =   0.00018496 ;
T (1,32,id) =  0.000185018 ;
T (1,64,id) =  0.000185236 ;
T (1,128,id) =  0.000185065 ;
T (1,160,id) =  0.000184885 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 346 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 7 ;
T (1,1,id) =   0.00886242 ;
T (1,2,id) =  8.23671e-05 ;
T (1,4,id) =  8.12244e-05 ;
T (1,8,id) =  8.08751e-05 ;
T (1,16,id) =  8.07391e-05 ;
T (1,32,id) =  8.13249e-05 ;
T (1,64,id) =  8.10344e-05 ;
T (1,128,id) =  8.10614e-05 ;
T (1,160,id) =   8.1067e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 346 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =   0.00876287 ;
T (1,2,id) =   3.7875e-05 ;
T (1,4,id) =  3.70648e-05 ;
T (1,8,id) =  3.69381e-05 ;
T (1,16,id) =  3.65153e-05 ;
T (1,32,id) =  3.64939e-05 ;
T (1,64,id) =  3.65479e-05 ;
T (1,128,id) =  3.64874e-05 ;
T (1,160,id) =  3.66196e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 240 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00879358 ;
T (1,2,id) =  2.11252e-05 ;
T (1,4,id) =   2.0856e-05 ;
T (1,8,id) =  2.07741e-05 ;
T (1,16,id) =  2.07499e-05 ;
T (1,32,id) =  2.04816e-05 ;
T (1,64,id) =   2.0721e-05 ;
T (1,128,id) =  2.05394e-05 ;
T (1,160,id) =  2.08654e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 31 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00875139 ;
T (1,2,id) =  1.54767e-05 ;
T (1,4,id) =  1.49673e-05 ;
T (1,8,id) =  1.47289e-05 ;
T (1,16,id) =  1.45538e-05 ;
T (1,32,id) =  1.45389e-05 ;
T (1,64,id) =   1.4645e-05 ;
T (1,128,id) =  1.44877e-05 ;
T (1,160,id) =  1.45491e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 31 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00870772 ;
T (1,2,id) =  1.27014e-05 ;
T (1,4,id) =  1.22096e-05 ;
T (1,8,id) =  1.21119e-05 ;
T (1,16,id) =  1.17868e-05 ;
T (1,32,id) =  1.19647e-05 ;
T (1,64,id) =  1.18883e-05 ;
T (1,128,id) =  1.18148e-05 ;
T (1,160,id) =  1.22115e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 24 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00880038 ;
T (1,2,id) =  9.58517e-06 ;
T (1,4,id) =  8.97702e-06 ;
T (1,8,id) =  8.97143e-06 ;
T (1,16,id) =  9.10275e-06 ;
T (1,32,id) =  9.18005e-06 ;
T (1,64,id) =  9.04594e-06 ;
T (1,128,id) =  8.87457e-06 ;
T (1,160,id) =  9.07946e-06 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 720 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00880344 ;
T (1,2,id) =  7.58218e-05 ;
T (1,4,id) =  7.58385e-05 ;
T (1,8,id) =  7.53999e-05 ;
T (1,16,id) =  7.53524e-05 ;
T (1,32,id) =  7.49184e-05 ;
T (1,64,id) =  7.47563e-05 ;
T (1,128,id) =  7.48057e-05 ;
T (1,160,id) =   7.5385e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 49 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00872731 ;
T (1,2,id) =  1.76234e-05 ;
T (1,4,id) =  1.69119e-05 ;
T (1,8,id) =  1.67266e-05 ;
T (1,16,id) =  1.65002e-05 ;
T (1,32,id) =  1.64472e-05 ;
T (1,64,id) =  1.65151e-05 ;
T (1,128,id) =  1.65114e-05 ;
T (1,160,id) =  1.62898e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 49 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00872247 ;
T (1,2,id) =  1.37007e-05 ;
T (1,4,id) =  1.34204e-05 ;
T (1,8,id) =  1.31959e-05 ;
T (1,16,id) =  1.31056e-05 ;
T (1,32,id) =  1.30683e-05 ;
T (1,64,id) =  1.31307e-05 ;
T (1,128,id) =  1.32229e-05 ;
T (1,160,id) =  1.31289e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 40 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00875355 ;
T (1,2,id) =  1.00229e-05 ;
T (1,4,id) =  9.77982e-06 ;
T (1,8,id) =    9.696e-06 ;
T (1,16,id) =  9.60659e-06 ;
T (1,32,id) =  9.43337e-06 ;
T (1,64,id) =  9.48273e-06 ;
T (1,128,id) =  9.45479e-06 ;
T (1,160,id) =  9.44454e-06 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 104 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00875619 ;
T (1,2,id) =  2.30651e-05 ;
T (1,4,id) =  2.28342e-05 ;
T (1,8,id) =  2.25622e-05 ;
T (1,16,id) =  2.25492e-05 ;
T (1,32,id) =  2.23815e-05 ;
T (1,64,id) =  2.24002e-05 ;
T (1,128,id) =  2.23862e-05 ;
T (1,160,id) =  2.25827e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 104 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00873085 ;
T (1,2,id) =  1.78749e-05 ;
T (1,4,id) =  1.72462e-05 ;
T (1,8,id) =  1.71475e-05 ;
T (1,16,id) =  1.71149e-05 ;
T (1,32,id) =   1.7154e-05 ;
T (1,64,id) =  1.72285e-05 ;
T (1,128,id) =  1.74511e-05 ;
T (1,160,id) =  1.69724e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 90 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =    0.0087109 ;
T (1,2,id) =  1.35675e-05 ;
T (1,4,id) =  1.30935e-05 ;
T (1,8,id) =  1.32211e-05 ;
T (1,16,id) =  1.31419e-05 ;
T (1,32,id) =  1.30832e-05 ;
T (1,64,id) =  1.29277e-05 ;
T (1,128,id) =  1.31335e-05 ;
T (1,160,id) =  1.31475e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 313 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00876859 ;
T (1,2,id) =  6.10985e-05 ;
T (1,4,id) =  6.07697e-05 ;
T (1,8,id) =  6.03404e-05 ;
T (1,16,id) =  6.04298e-05 ;
T (1,32,id) =  6.05164e-05 ;
T (1,64,id) =  6.02398e-05 ;
T (1,128,id) =  6.01802e-05 ;
T (1,160,id) =  6.03339e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 313 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =   0.00875565 ;
T (1,2,id) =  4.38327e-05 ;
T (1,4,id) =    4.368e-05 ;
T (1,8,id) =  4.32869e-05 ;
T (1,16,id) =   4.3299e-05 ;
T (1,32,id) =  4.30411e-05 ;
T (1,64,id) =  4.31556e-05 ;
T (1,128,id) =  4.32422e-05 ;
T (1,160,id) =  4.29535e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 288 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00877734 ;
T (1,2,id) =  3.76087e-05 ;
T (1,4,id) =  3.70657e-05 ;
T (1,8,id) =  3.72706e-05 ;
T (1,16,id) =  3.71914e-05 ;
T (1,32,id) =   3.6926e-05 ;
T (1,64,id) =  3.71411e-05 ;
T (1,128,id) =  3.70024e-05 ;
T (1,160,id) =  3.70452e-05 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 940 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 8 ;
T (1,1,id) =   0.00897467 ;
T (1,2,id) =  0.000286102 ;
T (1,4,id) =  0.000268277 ;
T (1,8,id) =  0.000267698 ;
T (1,16,id) =  0.000267531 ;
T (1,32,id) =  0.000267472 ;
T (1,64,id) =  0.000267235 ;
T (1,128,id) =  0.000266104 ;
T (1,160,id) =  0.000273933 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 940 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =   0.00882046 ;
T (1,2,id) =  0.000108533 ;
T (1,4,id) =  0.000107594 ;
T (1,8,id) =  0.000107486 ;
T (1,16,id) =  0.000107273 ;
T (1,32,id) =  0.000107158 ;
T (1,64,id) =  0.000107369 ;
T (1,128,id) =  0.000107402 ;
T (1,160,id) =  0.000107231 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 3448 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 9 ;
T (1,1,id) =    0.0111284 ;
T (1,2,id) =   0.00202815 ;
T (1,4,id) =   0.00204456 ;
T (1,8,id) =   0.00211742 ;
T (1,16,id) =   0.00238912 ;
T (1,32,id) =   0.00373162 ;
T (1,64,id) =    0.0148525 ;
T (1,128,id) =    0.0570612 ;
T (1,160,id) =    0.0359875 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 3448 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =   0.00977213 ;
T (1,2,id) =  0.000919501 ;
T (1,4,id) =  0.000955553 ;
T (1,8,id) =   0.00103027 ;
T (1,16,id) =   0.00117316 ;
T (1,32,id) =   0.00264952 ;
T (1,64,id) =   0.00562743 ;
T (1,128,id) =    0.0295102 ;
T (1,160,id) =    0.0273461 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 2880 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00956481 ;
T (1,2,id) =  0.000787999 ;
T (1,4,id) =   0.00082915 ;
T (1,8,id) =  0.000897923 ;
T (1,16,id) =   0.00102664 ;
T (1,32,id) =   0.00179597 ;
T (1,64,id) =   0.00578601 ;
T (1,128,id) =    0.0168351 ;
T (1,160,id) =    0.0317071 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4156 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =    0.0134912 ;
T (1,2,id) =   0.00257218 ;
T (1,4,id) =   0.00253802 ;
T (1,8,id) =   0.00258834 ;
T (1,16,id) =   0.00284792 ;
T (1,32,id) =    0.0045139 ;
T (1,64,id) =    0.0140797 ;
T (1,128,id) =    0.0477942 ;
T (1,160,id) =    0.0571381 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4156 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =    0.0130128 ;
T (1,2,id) =    0.0022366 ;
T (1,4,id) =   0.00202899 ;
T (1,8,id) =   0.00302213 ;
T (1,16,id) =   0.00426377 ;
T (1,32,id) =   0.00674314 ;
T (1,64,id) =    0.0143067 ;
T (1,128,id) =    0.0302031 ;
T (1,160,id) =    0.0531629 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4050 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =    0.0129672 ;
T (1,2,id) =     0.002182 ;
T (1,4,id) =   0.00264085 ;
T (1,8,id) =   0.00284115 ;
T (1,16,id) =   0.00221848 ;
T (1,32,id) =   0.00585989 ;
T (1,64,id) =    0.0133775 ;
T (1,128,id) =    0.0435949 ;
T (1,160,id) =    0.0628726 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 4320 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =   0.00955095 ;
T (1,2,id) =  0.000648642 ;
T (1,4,id) =  0.000684993 ;
T (1,8,id) =  0.000734456 ;
T (1,16,id) =  0.000881717 ;
T (1,32,id) =   0.00150904 ;
T (1,64,id) =   0.00548847 ;
T (1,128,id) =     0.030671 ;
T (1,160,id) =    0.0306281 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 6583 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 11 ;
T (1,1,id) =    0.0143789 ;
T (1,2,id) =   0.00485511 ;
T (1,4,id) =    0.0049061 ;
T (1,8,id) =   0.00495416 ;
T (1,16,id) =   0.00533164 ;
T (1,32,id) =   0.00859326 ;
T (1,64,id) =    0.0303624 ;
T (1,128,id) =    0.0594176 ;
T (1,160,id) =    0.0600551 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 6583 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =   0.00999708 ;
T (1,2,id) =  0.000993313 ;
T (1,4,id) =   0.00103714 ;
T (1,8,id) =   0.00110134 ;
T (1,16,id) =   0.00122242 ;
T (1,32,id) =   0.00153075 ;
T (1,64,id) =   0.00737859 ;
T (1,128,id) =    0.0360699 ;
T (1,160,id) =    0.0451368 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 14400 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =    0.0237781 ;
T (1,2,id) =   0.00812053 ;
T (1,4,id) =   0.00484782 ;
T (1,8,id) =   0.00505546 ;
T (1,16,id) =   0.00733852 ;
T (1,32,id) =    0.0136964 ;
T (1,64,id) =    0.0246046 ;
T (1,128,id) =    0.0467091 ;
T (1,160,id) =    0.0574572 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/as20000102/as20000102_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 6474 ;
Nedges (id) = 12572 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 11 ;
T (1,1,id) =    0.0223098 ;
T (1,2,id) =   0.00809267 ;
T (1,4,id) =   0.00604968 ;
T (1,8,id) =   0.00564359 ;
T (1,16,id) =   0.00609027 ;
T (1,32,id) =    0.0131666 ;
T (1,64,id) =    0.0318606 ;
T (1,128,id) =     0.124952 ;
T (1,160,id) =     0.122121 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 15988 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 13 ;
T (1,1,id) =    0.0358047 ;
T (1,2,id) =    0.0145459 ;
T (1,4,id) =    0.0112338 ;
T (1,8,id) =    0.0103008 ;
T (1,16,id) =    0.0107204 ;
T (1,32,id) =    0.0219033 ;
T (1,64,id) =    0.0617531 ;
T (1,128,id) =     0.145984 ;
T (1,160,id) =     0.208075 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 15988 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0244967 ;
T (1,2,id) =   0.00862725 ;
T (1,4,id) =   0.00536118 ;
T (1,8,id) =   0.00409748 ;
T (1,16,id) =   0.00601094 ;
T (1,32,id) =     0.010459 ;
T (1,64,id) =    0.0242302 ;
T (1,128,id) =    0.0514148 ;
T (1,160,id) =    0.0572082 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-GrQc/ca-GrQc_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5242 ;
Nedges (id) = 14484 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 45 ;
T (1,1,id) =    0.0315571 ;
T (1,2,id) =    0.0148764 ;
T (1,4,id) =    0.0109273 ;
T (1,8,id) =    0.0134167 ;
T (1,16,id) =    0.0149273 ;
T (1,32,id) =    0.0241367 ;
T (1,64,id) =     0.050742 ;
T (1,128,id) =     0.168167 ;
T (1,160,id) =     0.161221 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 23040 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =    0.0227255 ;
T (1,2,id) =   0.00814225 ;
T (1,4,id) =   0.00551343 ;
T (1,8,id) =   0.00518886 ;
T (1,16,id) =   0.00727634 ;
T (1,32,id) =    0.0114037 ;
T (1,64,id) =    0.0183643 ;
T (1,128,id) =    0.0590851 ;
T (1,160,id) =    0.0828631 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella08/p2p-Gnutella08_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 6301 ;
Nedges (id) = 20777 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =     0.012392 ;
T (1,2,id) =    0.0021561 ;
T (1,4,id) =   0.00159382 ;
T (1,8,id) =   0.00151306 ;
T (1,16,id) =   0.00171585 ;
T (1,32,id) =   0.00319854 ;
T (1,64,id) =   0.00769339 ;
T (1,128,id) =    0.0184293 ;
T (1,160,id) =    0.0715431 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010331/oregon1_010331_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10670 ;
Nedges (id) = 22002 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 17 ;
T (1,1,id) =    0.0506365 ;
T (1,2,id) =    0.0234902 ;
T (1,4,id) =    0.0159672 ;
T (1,8,id) =      0.01855 ;
T (1,16,id) =    0.0221081 ;
T (1,32,id) =    0.0274829 ;
T (1,64,id) =    0.0584313 ;
T (1,128,id) =     0.109024 ;
T (1,160,id) =      0.13008 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010407/oregon1_010407_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10729 ;
Nedges (id) = 21999 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 15 ;
T (1,1,id) =    0.0498316 ;
T (1,2,id) =    0.0227986 ;
T (1,4,id) =    0.0159519 ;
T (1,8,id) =    0.0189704 ;
T (1,16,id) =    0.0186859 ;
T (1,32,id) =    0.0289034 ;
T (1,64,id) =    0.0545833 ;
T (1,128,id) =     0.106973 ;
T (1,160,id) =     0.166712 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010414/oregon1_010414_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10790 ;
Nedges (id) = 22469 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 16 ;
T (1,1,id) =    0.0542196 ;
T (1,2,id) =    0.0250321 ;
T (1,4,id) =     0.017075 ;
T (1,8,id) =    0.0169675 ;
T (1,16,id) =    0.0234488 ;
T (1,32,id) =    0.0280926 ;
T (1,64,id) =    0.0692693 ;
T (1,128,id) =     0.119753 ;
T (1,160,id) =     0.177994 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010421/oregon1_010421_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10859 ;
Nedges (id) = 22747 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 16 ;
T (1,1,id) =    0.0545078 ;
T (1,2,id) =    0.0252104 ;
T (1,4,id) =    0.0174017 ;
T (1,8,id) =     0.017715 ;
T (1,16,id) =    0.0235216 ;
T (1,32,id) =    0.0283313 ;
T (1,64,id) =     0.067138 ;
T (1,128,id) =     0.144406 ;
T (1,160,id) =     0.174362 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010428/oregon1_010428_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10886 ;
Nedges (id) = 22493 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 16 ;
T (1,1,id) =    0.0544161 ;
T (1,2,id) =    0.0252515 ;
T (1,4,id) =    0.0173275 ;
T (1,8,id) =     0.017371 ;
T (1,16,id) =    0.0222281 ;
T (1,32,id) =    0.0314774 ;
T (1,64,id) =    0.0691943 ;
T (1,128,id) =     0.188582 ;
T (1,160,id) =     0.177755 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010505/oregon1_010505_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10943 ;
Nedges (id) = 22607 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 15 ;
T (1,1,id) =    0.0524153 ;
T (1,2,id) =    0.0238681 ;
T (1,4,id) =    0.0179759 ;
T (1,8,id) =     0.020832 ;
T (1,16,id) =     0.023625 ;
T (1,32,id) =    0.0343452 ;
T (1,64,id) =    0.0623386 ;
T (1,128,id) =     0.169796 ;
T (1,160,id) =     0.179423 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010512/oregon1_010512_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11011 ;
Nedges (id) = 22677 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 16 ;
T (1,1,id) =    0.0546147 ;
T (1,2,id) =    0.0252078 ;
T (1,4,id) =    0.0186181 ;
T (1,8,id) =    0.0218685 ;
T (1,16,id) =    0.0216275 ;
T (1,32,id) =    0.0313633 ;
T (1,64,id) =    0.0641708 ;
T (1,128,id) =     0.155361 ;
T (1,160,id) =      0.16473 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010519/oregon1_010519_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11051 ;
Nedges (id) = 22724 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 16 ;
T (1,1,id) =    0.0532811 ;
T (1,2,id) =    0.0249418 ;
T (1,4,id) =    0.0177176 ;
T (1,8,id) =    0.0180668 ;
T (1,16,id) =    0.0193609 ;
T (1,32,id) =    0.0268924 ;
T (1,64,id) =    0.0641352 ;
T (1,128,id) =     0.121562 ;
T (1,160,id) =      0.12646 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010526/oregon1_010526_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11174 ;
Nedges (id) = 23409 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 15 ;
T (1,1,id) =    0.0585221 ;
T (1,2,id) =    0.0277054 ;
T (1,4,id) =    0.0205242 ;
T (1,8,id) =    0.0236072 ;
T (1,16,id) =    0.0252923 ;
T (1,32,id) =    0.0356198 ;
T (1,64,id) =    0.0733045 ;
T (1,128,id) =     0.193875 ;
T (1,160,id) =     0.190839 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 31036 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 13 ;
T (1,1,id) =    0.0732118 ;
T (1,2,id) =    0.0331973 ;
T (1,4,id) =    0.0271648 ;
T (1,8,id) =    0.0265594 ;
T (1,16,id) =    0.0284492 ;
T (1,32,id) =    0.0435534 ;
T (1,64,id) =     0.164279 ;
T (1,128,id) =      0.27883 ;
T (1,160,id) =     0.292386 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 31036 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =    0.0273977 ;
T (1,2,id) =    0.0108326 ;
T (1,4,id) =   0.00783003 ;
T (1,8,id) =   0.00682293 ;
T (1,16,id) =   0.00654352 ;
T (1,32,id) =    0.0139796 ;
T (1,64,id) =    0.0282609 ;
T (1,128,id) =    0.0657264 ;
T (1,160,id) =     0.126398 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella09/p2p-Gnutella09_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8114 ;
Nedges (id) = 26013 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =    0.0132436 ;
T (1,2,id) =   0.00266645 ;
T (1,4,id) =   0.00190721 ;
T (1,8,id) =   0.00196904 ;
T (1,16,id) =   0.00216771 ;
T (1,32,id) =   0.00299308 ;
T (1,64,id) =   0.00766447 ;
T (1,128,id) =    0.0755171 ;
T (1,160,id) =    0.0406347 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-HepTh/ca-HepTh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 9877 ;
Nedges (id) = 25973 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 33 ;
T (1,1,id) =     0.031381 ;
T (1,2,id) =    0.0136815 ;
T (1,4,id) =   0.00975401 ;
T (1,8,id) =   0.00801327 ;
T (1,16,id) =   0.00841741 ;
T (1,32,id) =    0.0135253 ;
T (1,64,id) =     0.027366 ;
T (1,128,id) =     0.123666 ;
T (1,160,id) =     0.123322 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010331/oregon2_010331_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10900 ;
Nedges (id) = 31180 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 26 ;
T (1,1,id) =     0.275493 ;
T (1,2,id) =     0.145973 ;
T (1,4,id) =    0.0959591 ;
T (1,8,id) =     0.128217 ;
T (1,16,id) =     0.110275 ;
T (1,32,id) =      0.17153 ;
T (1,64,id) =     0.357827 ;
T (1,128,id) =     0.605463 ;
T (1,160,id) =     0.669217 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010407/oregon2_010407_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10981 ;
Nedges (id) = 30855 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 25 ;
T (1,1,id) =     0.246445 ;
T (1,2,id) =      0.12677 ;
T (1,4,id) =    0.0822344 ;
T (1,8,id) =    0.0970999 ;
T (1,16,id) =     0.112071 ;
T (1,32,id) =     0.160766 ;
T (1,64,id) =     0.306554 ;
T (1,128,id) =     0.523025 ;
T (1,160,id) =      0.59017 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010505/oregon2_010505_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11157 ;
Nedges (id) = 30943 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 22 ;
T (1,1,id) =     0.242552 ;
T (1,2,id) =     0.138334 ;
T (1,4,id) =     0.087329 ;
T (1,8,id) =    0.0958906 ;
T (1,16,id) =     0.111072 ;
T (1,32,id) =     0.176612 ;
T (1,64,id) =     0.322418 ;
T (1,128,id) =     0.520292 ;
T (1,160,id) =     0.595167 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010414/oregon2_010414_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11019 ;
Nedges (id) = 31761 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 25 ;
T (1,1,id) =     0.330135 ;
T (1,2,id) =     0.179831 ;
T (1,4,id) =     0.123711 ;
T (1,8,id) =     0.128997 ;
T (1,16,id) =     0.157131 ;
T (1,32,id) =      0.23581 ;
T (1,64,id) =     0.448011 ;
T (1,128,id) =     0.749771 ;
T (1,160,id) =     0.943367 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010421/oregon2_010421_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11080 ;
Nedges (id) = 31538 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 23 ;
T (1,1,id) =     0.278275 ;
T (1,2,id) =     0.155192 ;
T (1,4,id) =     0.101091 ;
T (1,8,id) =    0.0958488 ;
T (1,16,id) =     0.124947 ;
T (1,32,id) =     0.196882 ;
T (1,64,id) =     0.395334 ;
T (1,128,id) =      0.41214 ;
T (1,160,id) =     0.494946 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010428/oregon2_010428_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11113 ;
Nedges (id) = 31434 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 22 ;
T (1,1,id) =      0.24929 ;
T (1,2,id) =     0.138062 ;
T (1,4,id) =    0.0898903 ;
T (1,8,id) =    0.0962475 ;
T (1,16,id) =     0.110724 ;
T (1,32,id) =     0.179926 ;
T (1,64,id) =     0.323959 ;
T (1,128,id) =     0.578761 ;
T (1,160,id) =     0.608178 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010512/oregon2_010512_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11260 ;
Nedges (id) = 31303 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 22 ;
T (1,1,id) =     0.225182 ;
T (1,2,id) =     0.128398 ;
T (1,4,id) =    0.0825821 ;
T (1,8,id) =      0.10042 ;
T (1,16,id) =     0.108116 ;
T (1,32,id) =     0.150628 ;
T (1,64,id) =     0.299138 ;
T (1,128,id) =     0.537952 ;
T (1,160,id) =     0.590516 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010519/oregon2_010519_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11375 ;
Nedges (id) = 32287 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 25 ;
T (1,1,id) =     0.305691 ;
T (1,2,id) =     0.181573 ;
T (1,4,id) =     0.128482 ;
T (1,8,id) =     0.106207 ;
T (1,16,id) =     0.117615 ;
T (1,32,id) =     0.189068 ;
T (1,64,id) =     0.383122 ;
T (1,128,id) =     0.706584 ;
T (1,160,id) =     0.747559 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41809 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =     0.425296 ;
T (1,2,id) =     0.216239 ;
T (1,4,id) =     0.121668 ;
T (1,8,id) =    0.0764984 ;
T (1,16,id) =    0.0571498 ;
T (1,32,id) =    0.0778761 ;
T (1,64,id) =     0.165335 ;
T (1,128,id) =     0.425606 ;
T (1,160,id) =     0.539155 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41472 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =     0.405876 ;
T (1,2,id) =     0.206508 ;
T (1,4,id) =     0.113698 ;
T (1,8,id) =    0.0704798 ;
T (1,16,id) =    0.0440766 ;
T (1,32,id) =     0.085567 ;
T (1,64,id) =     0.124329 ;
T (1,128,id) =     0.267768 ;
T (1,160,id) =     0.293208 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010526/oregon2_010526_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11461 ;
Nedges (id) = 32730 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 26 ;
T (1,1,id) =     0.295191 ;
T (1,2,id) =     0.176876 ;
T (1,4,id) =     0.127344 ;
T (1,8,id) =     0.104957 ;
T (1,16,id) =     0.117872 ;
T (1,32,id) =     0.212626 ;
T (1,64,id) =      0.39915 ;
T (1,128,id) =     0.689934 ;
T (1,160,id) =     0.789085 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella06/p2p-Gnutella06_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8717 ;
Nedges (id) = 31525 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0121645 ;
T (1,2,id) =   0.00196977 ;
T (1,4,id) =   0.00139617 ;
T (1,8,id) =   0.00119581 ;
T (1,16,id) =    0.0014029 ;
T (1,32,id) =   0.00209786 ;
T (1,64,id) =   0.00510465 ;
T (1,128,id) =    0.0375686 ;
T (1,160,id) =    0.0476487 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41809 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =     0.405374 ;
T (1,2,id) =     0.206858 ;
T (1,4,id) =     0.113957 ;
T (1,8,id) =     0.065371 ;
T (1,16,id) =    0.0442276 ;
T (1,32,id) =    0.0710308 ;
T (1,64,id) =     0.125006 ;
T (1,128,id) =      0.33323 ;
T (1,160,id) =     0.329904 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella05/p2p-Gnutella05_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8846 ;
Nedges (id) = 31839 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0122676 ;
T (1,2,id) =   0.00204279 ;
T (1,4,id) =   0.00142459 ;
T (1,8,id) =   0.00127172 ;
T (1,16,id) =   0.00139935 ;
T (1,32,id) =   0.00223822 ;
T (1,64,id) =   0.00572416 ;
T (1,128,id) =    0.0360369 ;
T (1,160,id) =    0.0524145 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella04/p2p-Gnutella04_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10876 ;
Nedges (id) = 39994 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0125447 ;
T (1,2,id) =   0.00210687 ;
T (1,4,id) =   0.00155694 ;
T (1,8,id) =   0.00131658 ;
T (1,16,id) =   0.00156334 ;
T (1,32,id) =    0.0025384 ;
T (1,64,id) =   0.00512098 ;
T (1,128,id) =    0.0597023 ;
T (1,160,id) =     0.055399 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/as-caida20071105/as-caida20071105_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26475 ;
Nedges (id) = 53381 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 17 ;
T (1,1,id) =     0.143086 ;
T (1,2,id) =    0.0737604 ;
T (1,4,id) =    0.0501793 ;
T (1,8,id) =    0.0427385 ;
T (1,16,id) =    0.0446296 ;
T (1,32,id) =    0.0567115 ;
T (1,64,id) =     0.113298 ;
T (1,128,id) =     0.214196 ;
T (1,160,id) =     0.319907 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella25/p2p-Gnutella25_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 22687 ;
Nedges (id) = 54705 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0142573 ;
T (1,2,id) =   0.00325365 ;
T (1,4,id) =   0.00226253 ;
T (1,8,id) =   0.00190842 ;
T (1,16,id) =   0.00186647 ;
T (1,32,id) =   0.00252915 ;
T (1,64,id) =   0.00731169 ;
T (1,128,id) =    0.0833423 ;
T (1,160,id) =    0.0876733 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella24/p2p-Gnutella24_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26518 ;
Nedges (id) = 65369 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =     0.015435 ;
T (1,2,id) =   0.00390566 ;
T (1,4,id) =   0.00252961 ;
T (1,8,id) =   0.00205238 ;
T (1,16,id) =   0.00222043 ;
T (1,32,id) =   0.00404854 ;
T (1,64,id) =   0.00786154 ;
T (1,128,id) =    0.0981778 ;
T (1,160,id) =    0.0491502 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/facebook_combined/facebook_combined_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4039 ;
Nedges (id) = 88234 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 98 ;
T (1,1,id) =      13.6345 ;
T (1,2,id) =      8.69397 ;
T (1,4,id) =      8.48782 ;
T (1,8,id) =      8.65297 ;
T (1,16,id) =       9.4291 ;
T (1,32,id) =      13.4371 ;
T (1,64,id) =      23.1507 ;
T (1,128,id) =      49.8205 ;
T (1,160,id) =      64.9264 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 129600 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      1.07325 ;
T (1,2,id) =     0.536178 ;
T (1,4,id) =     0.273641 ;
T (1,8,id) =     0.152374 ;
T (1,16,id) =     0.100914 ;
T (1,32,id) =    0.0739153 ;
T (1,64,id) =     0.189554 ;
T (1,128,id) =     0.552224 ;
T (1,160,id) =     0.565161 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella30/p2p-Gnutella30_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36682 ;
Nedges (id) = 88328 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0174905 ;
T (1,2,id) =   0.00498406 ;
T (1,4,id) =   0.00336361 ;
T (1,8,id) =   0.00253378 ;
T (1,16,id) =   0.00262231 ;
T (1,32,id) =   0.00370094 ;
T (1,64,id) =   0.00882139 ;
T (1,128,id) =    0.0612219 ;
T (1,160,id) =    0.0419998 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 137164 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 20 ;
T (1,1,id) =      1.48883 ;
T (1,2,id) =     0.745618 ;
T (1,4,id) =      0.39757 ;
T (1,8,id) =     0.233663 ;
T (1,16,id) =     0.189912 ;
T (1,32,id) =      0.25394 ;
T (1,64,id) =     0.568455 ;
T (1,128,id) =      1.10826 ;
T (1,160,id) =     0.781043 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 137164 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =      1.09077 ;
T (1,2,id) =     0.544162 ;
T (1,4,id) =      0.29542 ;
T (1,8,id) =     0.159042 ;
T (1,16,id) =     0.119743 ;
T (1,32,id) =    0.0982356 ;
T (1,64,id) =      0.19962 ;
T (1,128,id) =     0.533773 ;
T (1,160,id) =      0.51579 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 138240 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =     0.171992 ;
T (1,2,id) =    0.0829698 ;
T (1,4,id) =    0.0452957 ;
T (1,8,id) =    0.0302156 ;
T (1,16,id) =    0.0222816 ;
T (1,32,id) =     0.024511 ;
T (1,64,id) =    0.0494825 ;
T (1,128,id) =     0.201421 ;
T (1,160,id) =      0.19598 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 144000 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =     0.444899 ;
T (1,2,id) =      0.22225 ;
T (1,4,id) =     0.121923 ;
T (1,8,id) =    0.0768365 ;
T (1,16,id) =    0.0532525 ;
T (1,32,id) =    0.0690862 ;
T (1,64,id) =     0.113715 ;
T (1,128,id) =      0.36037 ;
T (1,160,id) =      0.39254 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-CondMat/ca-CondMat_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 23133 ;
Nedges (id) = 93439 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 27 ;
T (1,1,id) =     0.312921 ;
T (1,2,id) =     0.165548 ;
T (1,4,id) =     0.103238 ;
T (1,8,id) =    0.0646463 ;
T (1,16,id) =     0.065417 ;
T (1,32,id) =    0.0751136 ;
T (1,64,id) =     0.162627 ;
T (1,128,id) =     0.368355 ;
T (1,160,id) =     0.260513 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-HepPh/ca-HepPh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 12008 ;
Nedges (id) = 118489 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 240 ;
T (1,1,id) =      8.42882 ;
T (1,2,id) =      4.38632 ;
T (1,4,id) =      2.40019 ;
T (1,8,id) =      1.51234 ;
T (1,16,id) =      1.69029 ;
T (1,32,id) =      2.99504 ;
T (1,64,id) =      5.64439 ;
T (1,128,id) =      8.29946 ;
T (1,160,id) =      8.89355 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 175873 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 20 ;
T (1,1,id) =      1.28519 ;
T (1,2,id) =     0.643579 ;
T (1,4,id) =     0.372497 ;
T (1,8,id) =     0.204092 ;
T (1,16,id) =     0.180606 ;
T (1,32,id) =     0.239584 ;
T (1,64,id) =     0.599023 ;
T (1,128,id) =     0.908228 ;
T (1,160,id) =      1.07176 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 175873 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =     0.522706 ;
T (1,2,id) =       0.2574 ;
T (1,4,id) =     0.133607 ;
T (1,8,id) =      0.08059 ;
T (1,16,id) =    0.0555211 ;
T (1,32,id) =    0.0640427 ;
T (1,64,id) =     0.154449 ;
T (1,128,id) =     0.346436 ;
T (1,160,id) =      0.51196 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella31/p2p-Gnutella31_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 62586 ;
Nedges (id) = 147892 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0234862 ;
T (1,2,id) =   0.00826007 ;
T (1,4,id) =   0.00549851 ;
T (1,8,id) =   0.00456491 ;
T (1,16,id) =   0.00394199 ;
T (1,32,id) =    0.0051779 ;
T (1,64,id) =    0.0112682 ;
T (1,128,id) =    0.0550957 ;
T (1,160,id) =    0.0743724 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 217255 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 7 ;
T (1,1,id) =     0.278751 ;
T (1,2,id) =     0.136095 ;
T (1,4,id) =    0.0891792 ;
T (1,8,id) =    0.0536255 ;
T (1,16,id) =    0.0566516 ;
T (1,32,id) =    0.0563387 ;
T (1,64,id) =     0.104275 ;
T (1,128,id) =     0.486174 ;
T (1,160,id) =     0.360642 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 217255 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 20 ;
T (1,1,id) =      1.54827 ;
T (1,2,id) =     0.774366 ;
T (1,4,id) =     0.433472 ;
T (1,8,id) =     0.278196 ;
T (1,16,id) =     0.277133 ;
T (1,32,id) =     0.522934 ;
T (1,64,id) =     0.641447 ;
T (1,128,id) =      1.46349 ;
T (1,160,id) =      1.80382 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/email-Enron/email-Enron_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36692 ;
Nedges (id) = 183831 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 23 ;
T (1,1,id) =      4.52415 ;
T (1,2,id) =      2.31514 ;
T (1,4,id) =      1.26147 ;
T (1,8,id) =     0.937884 ;
T (1,16,id) =      1.01139 ;
T (1,32,id) =      1.32662 ;
T (1,64,id) =      2.87595 ;
T (1,128,id) =      4.25863 ;
T (1,160,id) =       3.9252 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-260610-65536_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 65536 ;
Nedges (id) = 260610 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0219702 ;
T (1,2,id) =   0.00824096 ;
T (1,4,id) =   0.00652124 ;
T (1,8,id) =   0.00583398 ;
T (1,16,id) =   0.00548493 ;
T (1,32,id) =   0.00841986 ;
T (1,64,id) =    0.0265157 ;
T (1,128,id) =    0.0646661 ;
T (1,160,id) =     0.110982 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-AstroPh/ca-AstroPh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 18772 ;
Nedges (id) = 198050 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 58 ;
T (1,1,id) =       7.9195 ;
T (1,2,id) =      4.18058 ;
T (1,4,id) =      2.21584 ;
T (1,8,id) =      1.41713 ;
T (1,16,id) =      1.15085 ;
T (1,32,id) =      1.71983 ;
T (1,64,id) =      3.42333 ;
T (1,128,id) =      4.64042 ;
T (1,160,id) =      4.49055 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/loc-brightkite_edges/loc-brightkite_edges_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 58228 ;
Nedges (id) = 214078 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 44 ;
T (1,1,id) =      2.63613 ;
T (1,2,id) =      1.42407 ;
T (1,4,id) =      1.09468 ;
T (1,8,id) =       1.0008 ;
T (1,16,id) =      1.12452 ;
T (1,32,id) =      1.43122 ;
T (1,64,id) =       2.6198 ;
T (1,128,id) =      4.77321 ;
T (1,160,id) =      5.55964 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320000 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      23.2921 ;
T (1,2,id) =      11.6911 ;
T (1,4,id) =      5.92685 ;
T (1,8,id) =      3.03116 ;
T (1,16,id) =      1.63743 ;
T (1,32,id) =       1.1381 ;
T (1,64,id) =      1.11626 ;
T (1,128,id) =      1.37959 ;
T (1,160,id) =      1.57261 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320881 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      23.8353 ;
T (1,2,id) =      12.0294 ;
T (1,4,id) =      6.08224 ;
T (1,8,id) =      3.16755 ;
T (1,16,id) =      1.73611 ;
T (1,32,id) =      1.37995 ;
T (1,64,id) =      1.76984 ;
T (1,128,id) =      1.85445 ;
T (1,160,id) =      2.37097 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320881 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      23.3885 ;
T (1,2,id) =      11.7296 ;
T (1,4,id) =      5.90908 ;
T (1,8,id) =      3.03494 ;
T (1,16,id) =      1.67424 ;
T (1,32,id) =      1.32616 ;
T (1,64,id) =       1.9055 ;
T (1,128,id) =      1.26709 ;
T (1,160,id) =      1.63941 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-HepTh/cit-HepTh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 27770 ;
Nedges (id) = 352285 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 31 ;
T (1,1,id) =      12.7937 ;
T (1,2,id) =      6.57578 ;
T (1,4,id) =      3.57511 ;
T (1,8,id) =      2.02187 ;
T (1,16,id) =      1.72666 ;
T (1,32,id) =      2.37187 ;
T (1,64,id) =      4.63472 ;
T (1,128,id) =      6.19391 ;
T (1,160,id) =      6.47174 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Epinions1/soc-Epinions1_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 75879 ;
Nedges (id) = 405740 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 34 ;
T (1,1,id) =      16.7755 ;
T (1,2,id) =      8.70778 ;
T (1,4,id) =      4.65515 ;
T (1,8,id) =      2.82935 ;
T (1,16,id) =      2.16071 ;
T (1,32,id) =      2.36544 ;
T (1,64,id) =      3.99393 ;
T (1,128,id) =       5.7295 ;
T (1,160,id) =       6.3244 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/email-EuAll/email-EuAll_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 265214 ;
Nedges (id) = 364481 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 21 ;
T (1,1,id) =      1.99081 ;
T (1,2,id) =      1.05483 ;
T (1,4,id) =     0.606597 ;
T (1,8,id) =     0.384882 ;
T (1,16,id) =     0.286161 ;
T (1,32,id) =     0.585399 ;
T (1,64,id) =     0.498415 ;
T (1,128,id) =      1.11705 ;
T (1,160,id) =      1.49492 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-HepPh/cit-HepPh_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 34546 ;
Nedges (id) = 420877 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 26 ;
T (1,1,id) =      8.51619 ;
T (1,2,id) =      4.37316 ;
T (1,4,id) =      2.33009 ;
T (1,8,id) =      1.40043 ;
T (1,16,id) =       1.0849 ;
T (1,32,id) =      1.02809 ;
T (1,64,id) =      1.87521 ;
T (1,128,id) =      3.54215 ;
T (1,160,id) =      3.68758 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Slashdot0811/soc-Slashdot0811_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 77360 ;
Nedges (id) = 469180 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 36 ;
T (1,1,id) =      4.04752 ;
T (1,2,id) =      2.07619 ;
T (1,4,id) =      1.22425 ;
T (1,8,id) =     0.816174 ;
T (1,16,id) =     0.744722 ;
T (1,32,id) =      1.05633 ;
T (1,64,id) =      1.87959 ;
T (1,128,id) =      2.66643 ;
T (1,160,id) =      2.66485 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Slashdot0902/soc-Slashdot0902_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 82168 ;
Nedges (id) = 504230 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 37 ;
T (1,1,id) =      4.52052 ;
T (1,2,id) =      2.32608 ;
T (1,4,id) =      1.30932 ;
T (1,8,id) =     0.837237 ;
T (1,16,id) =     0.768571 ;
T (1,32,id) =     0.984153 ;
T (1,64,id) =      1.74539 ;
T (1,128,id) =       2.4841 ;
T (1,160,id) =      2.46568 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-1045506-262144_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 262144 ;
Nedges (id) = 1045506 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =    0.0623617 ;
T (1,2,id) =    0.0346798 ;
T (1,4,id) =    0.0275235 ;
T (1,8,id) =    0.0229234 ;
T (1,16,id) =    0.0217311 ;
T (1,32,id) =    0.0338099 ;
T (1,64,id) =    0.0963001 ;
T (1,128,id) =     0.185783 ;
T (1,160,id) =     0.256731 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1152000 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      8.69847 ;
T (1,2,id) =      4.36773 ;
T (1,4,id) =      2.22079 ;
T (1,8,id) =      1.15686 ;
T (1,16,id) =     0.694737 ;
T (1,32,id) =     0.570187 ;
T (1,64,id) =     0.596309 ;
T (1,128,id) =      0.52556 ;
T (1,160,id) =     0.803388 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/loc-gowalla_edges/loc-gowalla_edges_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 196591 ;
Nedges (id) = 950327 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 30 ;
T (1,1,id) =      20.9004 ;
T (1,2,id) =      10.6427 ;
T (1,4,id) =      5.53803 ;
T (1,8,id) =      3.02684 ;
T (1,16,id) =      2.06212 ;
T (1,32,id) =       2.4115 ;
T (1,64,id) =       4.5728 ;
T (1,128,id) =      6.52434 ;
T (1,160,id) =      7.02459 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0302/amazon0302_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 262111 ;
Nedges (id) = 899792 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 8 ;
T (1,1,id) =     0.979186 ;
T (1,2,id) =     0.509604 ;
T (1,4,id) =     0.302029 ;
T (1,8,id) =     0.216742 ;
T (1,16,id) =      0.15447 ;
T (1,32,id) =     0.193948 ;
T (1,64,id) =      0.53802 ;
T (1,128,id) =     0.716808 ;
T (1,160,id) =     0.736163 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1582861 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 29 ;
T (1,1,id) =      45.5668 ;
T (1,2,id) =      22.7762 ;
T (1,4,id) =      11.6381 ;
T (1,8,id) =      6.10271 ;
T (1,16,id) =      3.51891 ;
T (1,32,id) =       2.8183 ;
T (1,64,id) =      4.06943 ;
T (1,128,id) =       10.522 ;
T (1,160,id) =      11.9904 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1582861 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 7 ;
T (1,1,id) =      11.5782 ;
T (1,2,id) =       5.8131 ;
T (1,4,id) =      2.94442 ;
T (1,8,id) =      1.53016 ;
T (1,16,id) =     0.885469 ;
T (1,32,id) =     0.717949 ;
T (1,64,id) =      0.95988 ;
T (1,128,id) =      1.05289 ;
T (1,160,id) =      1.10476 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2073600 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      260.039 ;
T (1,2,id) =      129.797 ;
T (1,4,id) =      65.2517 ;
T (1,8,id) =      32.8471 ;
T (1,16,id) =      17.0916 ;
T (1,32,id) =      10.4133 ;
T (1,64,id) =      8.55363 ;
T (1,128,id) =      8.83027 ;
T (1,160,id) =      10.0159 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2132284 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 29 ;
T (1,1,id) =      307.812 ;
T (1,2,id) =      154.666 ;
T (1,4,id) =      78.1081 ;
T (1,8,id) =       39.519 ;
T (1,16,id) =      20.9101 ;
T (1,32,id) =      12.4831 ;
T (1,64,id) =      11.5201 ;
T (1,128,id) =      15.6723 ;
T (1,160,id) =      16.8945 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2132284 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =      260.559 ;
T (1,2,id) =      131.439 ;
T (1,4,id) =      65.1628 ;
T (1,8,id) =      33.0017 ;
T (1,16,id) =       17.148 ;
T (1,32,id) =      10.3499 ;
T (1,64,id) =      8.99117 ;
T (1,128,id) =      9.05474 ;
T (1,160,id) =      10.2436 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-PA/roadNet-PA_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1088092 ;
Nedges (id) = 1541898 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =      0.14646 ;
T (1,2,id) =    0.0823455 ;
T (1,4,id) =    0.0584402 ;
T (1,8,id) =     0.062069 ;
T (1,16,id) =    0.0544554 ;
T (1,32,id) =    0.0565415 ;
T (1,64,id) =      0.14903 ;
T (1,128,id) =     0.273765 ;
T (1,160,id) =     0.249736 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2332800 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      96.7276 ;
T (1,2,id) =      48.0162 ;
T (1,4,id) =      24.1988 ;
T (1,8,id) =      12.3296 ;
T (1,16,id) =      6.45751 ;
T (1,32,id) =      4.30366 ;
T (1,64,id) =      4.17453 ;
T (1,128,id) =        4.299 ;
T (1,160,id) =      4.06891 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2606125 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 29 ;
T (1,1,id) =      180.364 ;
T (1,2,id) =      90.6031 ;
T (1,4,id) =      45.9674 ;
T (1,8,id) =      23.4946 ;
T (1,16,id) =      12.5109 ;
T (1,32,id) =      8.05306 ;
T (1,64,id) =      8.72197 ;
T (1,128,id) =      15.9832 ;
T (1,160,id) =      19.8144 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2606125 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =      100.679 ;
T (1,2,id) =      50.4733 ;
T (1,4,id) =      25.4787 ;
T (1,8,id) =      13.0529 ;
T (1,16,id) =      6.80285 ;
T (1,32,id) =      4.19067 ;
T (1,64,id) =      3.98904 ;
T (1,128,id) =       4.7659 ;
T (1,160,id) =      4.69854 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-TX/roadNet-TX_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1379917 ;
Nedges (id) = 1921660 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =     0.177397 ;
T (1,2,id) =     0.101053 ;
T (1,4,id) =    0.0732258 ;
T (1,8,id) =    0.0573871 ;
T (1,16,id) =    0.0508762 ;
T (1,32,id) =    0.0702024 ;
T (1,64,id) =     0.223016 ;
T (1,128,id) =      0.30292 ;
T (1,160,id) =      0.41121 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-4188162-1048576_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1048576 ;
Nedges (id) = 4188162 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =     0.221168 ;
T (1,2,id) =     0.146227 ;
T (1,4,id) =      0.10752 ;
T (1,8,id) =      0.08932 ;
T (1,16,id) =    0.0801886 ;
T (1,32,id) =     0.129548 ;
T (1,64,id) =     0.233972 ;
T (1,128,id) =     0.416521 ;
T (1,160,id) =     0.707234 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/flickrEdges/flickrEdges_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 105938 ;
Nedges (id) = 2316948 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 575 ;
T (1,1,id) =      4317.07 ;
T (1,2,id) =      2184.77 ;
T (1,4,id) =       1122.8 ;
T (1,8,id) =      579.966 ;
T (1,16,id) =      310.577 ;
T (1,32,id) =      347.251 ;
T (1,64,id) =      612.211 ;
T (1,128,id) =      1135.71 ;
T (1,160,id) =      1345.75 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0312/amazon0312_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 400727 ;
Nedges (id) = 2349869 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 12 ;
T (1,1,id) =      13.5043 ;
T (1,2,id) =      6.49237 ;
T (1,4,id) =      3.55214 ;
T (1,8,id) =      2.04825 ;
T (1,16,id) =       1.8345 ;
T (1,32,id) =      1.88598 ;
T (1,64,id) =      3.69351 ;
T (1,128,id) =      4.97214 ;
T (1,160,id) =      5.22681 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0505/amazon0505_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 410236 ;
Nedges (id) = 2439437 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 12 ;
T (1,1,id) =      14.6735 ;
T (1,2,id) =      7.40945 ;
T (1,4,id) =      4.08553 ;
T (1,8,id) =      2.36228 ;
T (1,16,id) =      1.77821 ;
T (1,32,id) =      2.54771 ;
T (1,64,id) =       2.4744 ;
T (1,128,id) =      5.14821 ;
T (1,160,id) =      5.54852 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0601/amazon0601_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 403394 ;
Nedges (id) = 2443408 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 12 ;
T (1,1,id) =       12.437 ;
T (1,2,id) =      6.27868 ;
T (1,4,id) =        3.526 ;
T (1,8,id) =      2.02135 ;
T (1,16,id) =      1.48133 ;
T (1,32,id) =      2.05967 ;
T (1,64,id) =      3.37399 ;
T (1,128,id) =      5.09306 ;
T (1,160,id) =      5.21785 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-CA/roadNet-CA_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1965206 ;
Nedges (id) = 2766607 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =     0.255167 ;
T (1,2,id) =     0.148338 ;
T (1,4,id) =     0.106718 ;
T (1,8,id) =    0.0838897 ;
T (1,16,id) =    0.0731075 ;
T (1,32,id) =     0.150056 ;
T (1,64,id) =     0.320647 ;
T (1,128,id) =     0.438233 ;
T (1,160,id) =     0.481844 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale18-ef16/graph500-scale18-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 174147 ;
Nedges (id) = 3800348 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 160 ;
T (1,1,id) =      4022.26 ;
T (1,2,id) =      2052.54 ;
T (1,4,id) =      1072.89 ;
T (1,8,id) =      574.688 ;
T (1,16,id) =      379.203 ;
T (1,32,id) =      397.585 ;
T (1,64,id) =      582.199 ;
T (1,128,id) =      1020.85 ;
T (1,160,id) =      1238.27 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 6912000 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      104.297 ;
T (1,2,id) =      52.2073 ;
T (1,4,id) =       26.209 ;
T (1,8,id) =      13.3171 ;
T (1,16,id) =      6.95648 ;
T (1,32,id) =      4.81021 ;
T (1,64,id) =      4.48926 ;
T (1,128,id) =      4.26037 ;
T (1,160,id) =      4.13254 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 11080030 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 63 ;
T (1,1,id) =      1168.69 ;
T (1,2,id) =      600.903 ;
T (1,4,id) =      311.353 ;
T (1,8,id) =      161.573 ;
T (1,16,id) =      87.3518 ;
T (1,32,id) =      58.5225 ;
T (1,64,id) =      61.3947 ;
T (1,128,id) =      116.454 ;
T (1,160,id) =      149.759 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 11080030 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 8 ;
T (1,1,id) =      173.728 ;
T (1,2,id) =      87.1452 ;
T (1,4,id) =      43.8591 ;
T (1,8,id) =      22.4845 ;
T (1,16,id) =      11.9468 ;
T (1,32,id) =      7.43031 ;
T (1,64,id) =      7.17181 ;
T (1,128,id) =      9.23574 ;
T (1,160,id) =      9.66307 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale19-ef16/graph500-scale19-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 335318 ;
Nedges (id) = 7729675 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 214 ;
T (1,1,id) =      13057.2 ;
T (1,2,id) =       6729.5 ;
T (1,4,id) =      3535.54 ;
T (1,8,id) =      1822.97 ;
T (1,16,id) =      1050.93 ;
T (1,32,id) =      983.469 ;
T (1,64,id) =      1434.62 ;
T (1,128,id) =      2588.93 ;
T (1,160,id) =      3191.23 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-16764930-4194304_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4194304 ;
Nedges (id) = 16764930 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =     0.855091 ;
T (1,2,id) =     0.544162 ;
T (1,4,id) =     0.406684 ;
T (1,8,id) =     0.353421 ;
T (1,16,id) =     0.310543 ;
T (1,32,id) =      0.33857 ;
T (1,64,id) =      1.55762 ;
T (1,128,id) =      2.85292 ;
T (1,160,id) =      3.05054 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-Bk.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 23328000 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 3 ;
T (1,1,id) =      3082.96 ;
T (1,2,id) =      1651.11 ;
T (1,4,id) =      912.823 ;
T (1,8,id) =      526.621 ;
T (1,16,id) =      349.051 ;
T (1,32,id) =      202.177 ;
T (1,64,id) =      147.983 ;
T (1,128,id) =      106.673 ;
T (1,160,id) =      99.5894 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale20-ef16/graph500-scale20-ef16_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 645820 ;
Nedges (id) = 15680861 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 285 ;
T (1,1,id) =        44551 ;
T (1,2,id) =      23223.7 ;
T (1,4,id) =      12137.4 ;
T (1,8,id) =      6156.43 ;
T (1,16,id) =      3293.32 ;
T (1,32,id) =      2877.55 ;
T (1,64,id) =      4023.32 ;
T (1,128,id) =      7564.58 ;
T (1,160,id) =      9539.46 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-B2k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 28667380 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 7 ;
T (1,1,id) =      3594.82 ;
T (1,2,id) =      1932.84 ;
T (1,4,id) =      1066.17 ;
T (1,8,id) =      610.342 ;
T (1,16,id) =      407.613 ;
T (1,32,id) =      241.763 ;
T (1,64,id) =      170.743 ;
T (1,128,id) =      122.453 ;
T (1,160,id) =      122.574 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-B1k.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 28667380 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 85 ;
T (1,1,id) =      9803.11 ;
T (1,2,id) =      5395.52 ;
T (1,4,id) =      2910.09 ;
T (1,8,id) =      1639.86 ;
T (1,16,id) =      950.174 ;
T (1,32,id) =      616.821 ;
T (1,64,id) =      471.678 ;
T (1,128,id) =      416.497 ;
T (1,160,id) =      433.061 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-Patents/cit-Patents_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 3774768 ;
Nedges (id) = 16518947 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 37 ;
T (1,1,id) =      59.5397 ;
T (1,2,id) =      32.5786 ;
T (1,4,id) =      18.2487 ;
T (1,8,id) =      11.0605 ;
T (1,16,id) =      9.95694 ;
T (1,32,id) =      9.02624 ;
T (1,64,id) =      9.29766 ;
T (1,128,id) =      16.9567 ;
T (1,160,id) =      27.5845 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale21-ef16/graph500-scale21-ef16_adj.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-67084290-16777216_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 16777216 ;
Nedges (id) = 67084290 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =      3.42676 ;
T (1,2,id) =      2.21008 ;
T (1,4,id) =      1.64032 ;
T (1,8,id) =      1.46414 ;
T (1,16,id) =      1.31031 ;
T (1,32,id) =      1.36099 ;
T (1,64,id) =      5.11525 ;
T (1,128,id) =      11.6675 ;
T (1,160,id) =      11.3921 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale22-ef16/graph500-scale22-ef16_adj.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/gc6/V2a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 55042369 ;
Nedges (id) = 58608800 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      7.08185 ;
T (1,2,id) =       4.4275 ;
T (1,4,id) =      2.60206 ;
T (1,8,id) =      1.97633 ;
T (1,16,id) =      2.17671 ;
T (1,32,id) =      3.00461 ;
T (1,64,id) =      3.53026 ;
T (1,128,id) =      5.87189 ;
T (1,160,id) =      8.67424 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/U1a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 67716231 ;
Nedges (id) = 69389281 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      9.64385 ;
T (1,2,id) =      5.13199 ;
T (1,4,id) =       3.0627 ;
T (1,8,id) =      2.43569 ;
T (1,16,id) =      2.58199 ;
T (1,32,id) =      2.47743 ;
T (1,64,id) =      4.15124 ;
T (1,128,id) =      7.53517 ;
T (1,160,id) =      10.3673 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale23-ef16/graph500-scale23-ef16_adj.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-268386306-67108864_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 67108864 ;
Nedges (id) = 268386306 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =      14.0598 ;
T (1,2,id) =      9.31096 ;
T (1,4,id) =      7.07757 ;
T (1,8,id) =      5.42716 ;
T (1,16,id) =      5.48361 ;
T (1,32,id) =      8.77984 ;
T (1,64,id) =      19.7201 ;
T (1,128,id) =      34.8034 ;
T (1,160,id) =      41.9522 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/P1a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 139353211 ;
Nedges (id) = 148914992 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      19.8653 ;
T (1,2,id) =      11.0306 ;
T (1,4,id) =      6.94273 ;
T (1,8,id) =      5.48841 ;
T (1,16,id) =      5.25357 ;
T (1,32,id) =      5.99685 ;
T (1,64,id) =      11.1221 ;
T (1,128,id) =      19.3754 ;
T (1,160,id) =      22.0426 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/A2a.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170728175 ;
Nedges (id) = 180292586 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      23.8366 ;
T (1,2,id) =       13.701 ;
T (1,4,id) =       7.7053 ;
T (1,8,id) =       6.8874 ;
T (1,16,id) =      6.50705 ;
T (1,32,id) =      6.51311 ;
T (1,64,id) =      7.55905 ;
T (1,128,id) =      21.9834 ;
T (1,160,id) =      25.9868 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/V1r.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 214005017 ;
Nedges (id) = 232705452 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 4 ;
T (1,1,id) =      15.8887 ;
T (1,2,id) =      10.2235 ;
T (1,4,id) =      6.19965 ;
T (1,8,id) =      5.51504 ;
T (1,16,id) =      5.34073 ;
T (1,32,id) =      7.09256 ;
T (1,64,id) =      15.1111 ;
T (1,128,id) =      20.5942 ;
T (1,160,id) =      27.1122 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale24-ef16/graph500-scale24-ef16_adj.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-1073643522-268435456_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 268435456 ;
Nedges (id) = 1073643522 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 5 ;
T (1,1,id) =      57.9981 ;
T (1,2,id) =      39.5611 ;
T (1,4,id) =      28.2131 ;
T (1,8,id) =      22.6554 ;
T (1,16,id) =      21.5392 ;
T (1,32,id) =      24.5843 ;
T (1,64,id) =      60.8065 ;
T (1,128,id) =      123.238 ;
T (1,160,id) =      172.573 ;
File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale25-ef16/graph500-scale25-ef16_adj.tsv.gz ';
id = id + 1 ;
file = ' /users/davis/GraphChallenge/snap/friendster/friendster_adj.tsv.gz ';

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 119432957 ;
Nedges (id) = 1799999986 ;
% T (keep, nthreads, id) = time for all-k-truss
T (1:2, 1:160, id) = nan ;
Kmax (id) = 6 ;
T (1,1,id) =      10765.1 ;
T (1,2,id) =      5111.73 ;
T (1,4,id) =      2958.99 ;
T (1,8,id) =      2582.99 ;
T (1,16,id) =      3119.79 ;
T (1,32,id) =       2148.1 ;
T (1,64,id) =      1838.66 ;
T (1,128,id) =      2017.87 ;
T (1,160,id) =      2051.17 ;
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
