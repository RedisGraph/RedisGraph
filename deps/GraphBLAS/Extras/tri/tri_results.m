function [T, Tprep, N, Nedges, Ntri, File] = tri_results
id = 0 ;
file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 841 ;
Ntri (id) = 400 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.63937e-06 ;
T_prep (1,1,2) =  2.65334e-06 ;
T_prep (1,1,4) =  2.63657e-06 ;
T_prep (1,1,8) =  2.61515e-06 ;
T_prep (1,1,16) =  2.61515e-06 ;
T_prep (1,1,32) =  2.60305e-06 ;
T_prep (1,1,64) =  2.61609e-06 ;
T_prep (1,1,128) =  2.61515e-06 ;
T_prep (1,1,160) =  2.61329e-06 ;
T_prep (2,1,1) =  2.89083e-06 ;
T_prep (2,1,2) =  2.76603e-06 ;
T_prep (2,1,4) =  2.74833e-06 ;
T_prep (2,1,8) =   2.7176e-06 ;
T_prep (2,1,16) =  2.72505e-06 ;
T_prep (2,1,32) =  2.71481e-06 ;
T_prep (2,1,64) =  2.70735e-06 ;
T_prep (2,1,128) =  2.70549e-06 ;
T_prep (2,1,160) =  2.70735e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  4.68269e-06 ;
Time (1,1,2) =  4.62867e-06 ;
Time (1,1,4) =  4.61657e-06 ;
Time (1,1,8) =  4.62495e-06 ;
Time (1,1,16) =  4.59608e-06 ;
Time (1,1,32) =  4.58211e-06 ;
Time (1,1,64) =  4.59235e-06 ;
Time (1,1,128) =  4.60353e-06 ;
Time (1,1,160) =  4.59608e-06 ;
Time (2,1,1) =  6.91786e-06 ;
Time (2,1,2) =  6.86105e-06 ;
Time (2,1,4) =  6.65803e-06 ;
Time (2,1,8) =  6.70832e-06 ;
Time (2,1,16) =  6.78003e-06 ;
Time (2,1,32) =  6.74184e-06 ;
Time (2,1,64) =  6.67479e-06 ;
Time (2,1,128) =  6.66361e-06 ;
Time (2,1,160) =  6.64406e-06 ;
Time (3,1,1) =  4.11738e-06 ;
Time (3,1,2) =  3.91342e-06 ;
Time (3,1,4) =   3.9069e-06 ;
Time (3,1,8) =  3.92552e-06 ;
Time (3,1,16) =  3.91621e-06 ;
Time (3,1,32) =  3.88734e-06 ;
Time (3,1,64) =  3.93577e-06 ;
Time (3,1,128) =  3.89572e-06 ;
Time (3,1,160) =  3.94415e-06 ;
Time (4,1,1) =  6.22123e-06 ;
Time (4,1,2) =  5.78631e-06 ;
Time (4,1,4) =  5.67734e-06 ;
Time (4,1,8) =  5.66617e-06 ;
Time (4,1,16) =  5.70714e-06 ;
Time (4,1,32) =  5.69317e-06 ;
Time (4,1,64) =  5.63543e-06 ;
Time (4,1,128) =   5.6643e-06 ;
Time (4,1,160) =  5.69504e-06 ;
Time (5,1,1) =  3.03332e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 841 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.89828e-06 ;
T_prep (1,1,2) =  2.86847e-06 ;
T_prep (1,1,4) =  2.87592e-06 ;
T_prep (1,1,8) =  2.85357e-06 ;
T_prep (1,1,16) =  2.83867e-06 ;
T_prep (1,1,32) =   2.8424e-06 ;
T_prep (1,1,64) =  2.83215e-06 ;
T_prep (1,1,128) =  2.84985e-06 ;
T_prep (1,1,160) =  2.84333e-06 ;
T_prep (2,1,1) =  3.06778e-06 ;
T_prep (2,1,2) =  2.88896e-06 ;
T_prep (2,1,4) =  2.83215e-06 ;
T_prep (2,1,8) =  2.82656e-06 ;
T_prep (2,1,16) =  2.83588e-06 ;
T_prep (2,1,32) =  2.81725e-06 ;
T_prep (2,1,64) =  2.84612e-06 ;
T_prep (2,1,128) =   2.8275e-06 ;
T_prep (2,1,160) =  2.80701e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  4.30364e-06 ;
Time (1,1,2) =  4.31389e-06 ;
Time (1,1,4) =  4.25801e-06 ;
Time (1,1,8) =  4.26639e-06 ;
Time (1,1,16) =  4.28315e-06 ;
Time (1,1,32) =  4.29899e-06 ;
Time (1,1,64) =  4.26453e-06 ;
Time (1,1,128) =  4.27291e-06 ;
Time (1,1,160) =  4.27384e-06 ;
Time (2,1,1) =  4.91831e-06 ;
Time (2,1,2) =  4.66779e-06 ;
Time (2,1,4) =  4.62495e-06 ;
Time (2,1,8) =  4.62309e-06 ;
Time (2,1,16) =  4.59142e-06 ;
Time (2,1,32) =  4.58583e-06 ;
Time (2,1,64) =  4.58397e-06 ;
Time (2,1,128) =  4.59794e-06 ;
Time (2,1,160) =  4.57559e-06 ;
Time (3,1,1) =  3.23821e-06 ;
Time (3,1,2) =  2.87313e-06 ;
Time (3,1,4) =   2.8275e-06 ;
Time (3,1,8) =  2.84892e-06 ;
Time (3,1,16) =  2.78279e-06 ;
Time (3,1,32) =  2.81353e-06 ;
Time (3,1,64) =  2.80701e-06 ;
Time (3,1,128) =  2.79117e-06 ;
Time (3,1,160) =  2.81632e-06 ;
Time (4,1,1) =  5.89341e-06 ;
Time (4,1,2) =  5.38304e-06 ;
Time (4,1,4) =  5.38304e-06 ;
Time (4,1,8) =  5.30481e-06 ;
Time (4,1,16) =  5.32623e-06 ;
Time (4,1,32) =  5.28153e-06 ;
Time (4,1,64) =  5.35324e-06 ;
Time (4,1,128) =  5.30016e-06 ;
Time (4,1,160) =  5.35417e-06 ;
Time (5,1,1) =  3.21399e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 442 ;
Nedges (id) = 800 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.75392e-06 ;
T_prep (1,1,2) =  2.78093e-06 ;
T_prep (1,1,4) =  2.79304e-06 ;
T_prep (1,1,8) =  2.80142e-06 ;
T_prep (1,1,16) =  2.80235e-06 ;
T_prep (1,1,32) =  2.79304e-06 ;
T_prep (1,1,64) =  2.78186e-06 ;
T_prep (1,1,128) =  2.78186e-06 ;
T_prep (1,1,160) =  2.78279e-06 ;
T_prep (2,1,1) =  2.69059e-06 ;
T_prep (2,1,2) =  2.66638e-06 ;
T_prep (2,1,4) =  2.64868e-06 ;
T_prep (2,1,8) =  2.63471e-06 ;
T_prep (2,1,16) =  2.61609e-06 ;
T_prep (2,1,32) =  2.63657e-06 ;
T_prep (2,1,64) =  2.64123e-06 ;
T_prep (2,1,128) =  2.64123e-06 ;
T_prep (2,1,160) =  2.63471e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  4.02983e-06 ;
Time (1,1,2) =  4.08199e-06 ;
Time (1,1,4) =  4.06522e-06 ;
Time (1,1,8) =  4.04287e-06 ;
Time (1,1,16) =  4.05964e-06 ;
Time (1,1,32) =  4.04473e-06 ;
Time (1,1,64) =  4.06336e-06 ;
Time (1,1,128) =  4.04753e-06 ;
Time (1,1,160) =  4.04753e-06 ;
Time (2,1,1) =  4.36231e-06 ;
Time (2,1,2) =  4.27943e-06 ;
Time (2,1,4) =  4.22169e-06 ;
Time (2,1,8) =  4.21703e-06 ;
Time (2,1,16) =  4.17978e-06 ;
Time (2,1,32) =  4.23752e-06 ;
Time (2,1,64) =  4.24217e-06 ;
Time (2,1,128) =  4.21796e-06 ;
Time (2,1,160) =  4.23193e-06 ;
Time (3,1,1) =    2.346e-06 ;
Time (3,1,2) =  2.22679e-06 ;
Time (3,1,4) =  2.19233e-06 ;
Time (3,1,8) =  2.24635e-06 ;
Time (3,1,16) =  2.21841e-06 ;
Time (3,1,32) =  2.21562e-06 ;
Time (3,1,64) =  2.18954e-06 ;
Time (3,1,128) =  2.20723e-06 ;
Time (3,1,160) =  2.21096e-06 ;
Time (4,1,1) =  5.45103e-06 ;
Time (4,1,2) =  5.10179e-06 ;
Time (4,1,4) =  5.08595e-06 ;
Time (4,1,8) =  5.08409e-06 ;
Time (4,1,16) =  5.10179e-06 ;
Time (4,1,32) =  5.11948e-06 ;
Time (4,1,64) =  5.09806e-06 ;
Time (4,1,128) =  5.08223e-06 ;
Time (4,1,160) =  5.08595e-06 ;
Time (5,1,1) =  2.71853e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 346 ;
Ntri (id) = 287 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.17626e-06 ;
T_prep (1,1,2) =  1.14739e-06 ;
T_prep (1,1,4) =  1.13714e-06 ;
T_prep (1,1,8) =   1.1446e-06 ;
T_prep (1,1,16) =  1.13901e-06 ;
T_prep (1,1,32) =  1.13901e-06 ;
T_prep (1,1,64) =  1.13063e-06 ;
T_prep (1,1,128) =  1.13063e-06 ;
T_prep (1,1,160) =  1.13342e-06 ;
T_prep (2,1,1) =  1.67079e-06 ;
T_prep (2,1,2) =  1.41002e-06 ;
T_prep (2,1,4) =  1.40164e-06 ;
T_prep (2,1,8) =  1.40257e-06 ;
T_prep (2,1,16) =  1.40723e-06 ;
T_prep (2,1,32) =  1.37649e-06 ;
T_prep (2,1,64) =  1.38953e-06 ;
T_prep (2,1,128) =  1.40164e-06 ;
T_prep (2,1,160) =  1.46311e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  2.30782e-06 ;
Time (1,1,2) =  2.22493e-06 ;
Time (1,1,4) =  2.21841e-06 ;
Time (1,1,8) =  2.16998e-06 ;
Time (1,1,16) =  2.14763e-06 ;
Time (1,1,32) =  2.15974e-06 ;
Time (1,1,64) =  2.14577e-06 ;
Time (1,1,128) =  2.13552e-06 ;
Time (1,1,160) =  2.15322e-06 ;
Time (2,1,1) =  3.75602e-06 ;
Time (2,1,2) =  3.42168e-06 ;
Time (2,1,4) =  3.36953e-06 ;
Time (2,1,8) =  3.36301e-06 ;
Time (2,1,16) =  3.32296e-06 ;
Time (2,1,32) =  3.33413e-06 ;
Time (2,1,64) =  3.35276e-06 ;
Time (2,1,128) =  3.31271e-06 ;
Time (2,1,160) =  3.32389e-06 ;
Time (3,1,1) =  5.60004e-06 ;
Time (3,1,2) =  4.52157e-06 ;
Time (3,1,4) =  4.42471e-06 ;
Time (3,1,8) =  4.41168e-06 ;
Time (3,1,16) =  4.40702e-06 ;
Time (3,1,32) =  4.41168e-06 ;
Time (3,1,64) =  4.44055e-06 ;
Time (3,1,128) =  4.38746e-06 ;
Time (3,1,160) =  4.41354e-06 ;
Time (4,1,1) =  3.27267e-06 ;
Time (4,1,2) =  2.79769e-06 ;
Time (4,1,4) =  2.71946e-06 ;
Time (4,1,8) =  2.68035e-06 ;
Time (4,1,16) =   2.6701e-06 ;
Time (4,1,32) =  2.67196e-06 ;
Time (4,1,64) =  2.66638e-06 ;
Time (4,1,128) =  2.67196e-06 ;
Time (4,1,160) =  2.65613e-06 ;
Time (5,1,1) =  2.13645e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 346 ;
Ntri (id) = 7 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.39326e-06 ;
T_prep (1,1,2) =  1.34204e-06 ;
T_prep (1,1,4) =  1.33086e-06 ;
T_prep (1,1,8) =  1.31968e-06 ;
T_prep (1,1,16) =  1.32248e-06 ;
T_prep (1,1,32) =  1.31875e-06 ;
T_prep (1,1,64) =  1.31596e-06 ;
T_prep (1,1,128) =  1.31503e-06 ;
T_prep (1,1,160) =  1.30851e-06 ;
T_prep (2,1,1) =  1.88965e-06 ;
T_prep (2,1,2) =  1.55624e-06 ;
T_prep (2,1,4) =  1.40816e-06 ;
T_prep (2,1,8) =  1.38301e-06 ;
T_prep (2,1,16) =  1.37556e-06 ;
T_prep (2,1,32) =  1.35694e-06 ;
T_prep (2,1,64) =   1.3439e-06 ;
T_prep (2,1,128) =  1.36346e-06 ;
T_prep (2,1,160) =  1.35694e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  2.38884e-06 ;
Time (1,1,2) =  2.19047e-06 ;
Time (1,1,4) =  2.10572e-06 ;
Time (1,1,8) =  2.05729e-06 ;
Time (1,1,16) =  1.99676e-06 ;
Time (1,1,32) =  2.00607e-06 ;
Time (1,1,64) =  2.01911e-06 ;
Time (1,1,128) =  2.00979e-06 ;
Time (1,1,160) =  2.00979e-06 ;
Time (2,1,1) =  3.27826e-06 ;
Time (2,1,2) =  2.81353e-06 ;
Time (2,1,4) =  2.63099e-06 ;
Time (2,1,8) =  2.57418e-06 ;
Time (2,1,16) =  2.55834e-06 ;
Time (2,1,32) =  2.50433e-06 ;
Time (2,1,64) =  2.51457e-06 ;
Time (2,1,128) =  2.50619e-06 ;
Time (2,1,160) =  2.53692e-06 ;
Time (3,1,1) =  4.97326e-06 ;
Time (3,1,2) =  3.45986e-06 ;
Time (3,1,4) =   3.1637e-06 ;
Time (3,1,8) =  2.95602e-06 ;
Time (3,1,16) =  2.90573e-06 ;
Time (3,1,32) =  2.90759e-06 ;
Time (3,1,64) =  2.85823e-06 ;
Time (3,1,128) =   2.8694e-06 ;
Time (3,1,160) =  2.82843e-06 ;
Time (4,1,1) =  3.35649e-06 ;
Time (4,1,2) =  2.75392e-06 ;
Time (4,1,4) =  2.56952e-06 ;
Time (4,1,8) =  2.52854e-06 ;
Time (4,1,16) =  2.50153e-06 ;
Time (4,1,32) =  2.50805e-06 ;
Time (4,1,64) =  2.49967e-06 ;
Time (4,1,128) =  2.46707e-06 ;
Time (4,1,160) =    2.468e-06 ;
Time (5,1,1) =  2.86102e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 120 ;
Nedges (id) = 240 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  8.26083e-07 ;
T_prep (1,1,2) =   8.3819e-07 ;
T_prep (1,1,4) =  8.33534e-07 ;
T_prep (1,1,8) =  8.34465e-07 ;
T_prep (1,1,16) =  8.33534e-07 ;
T_prep (1,1,32) =  8.35396e-07 ;
T_prep (1,1,64) =  8.27946e-07 ;
T_prep (1,1,128) =  8.19564e-07 ;
T_prep (1,1,160) =  8.20495e-07 ;
T_prep (2,1,1) =  1.03563e-06 ;
T_prep (2,1,2) =   8.8755e-07 ;
T_prep (2,1,4) =  8.60542e-07 ;
T_prep (2,1,8) =  8.46572e-07 ;
T_prep (2,1,16) =   8.4471e-07 ;
T_prep (2,1,32) =  8.41916e-07 ;
T_prep (2,1,64) =  8.45641e-07 ;
T_prep (2,1,128) =  8.54954e-07 ;
T_prep (2,1,160) =  8.45641e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.33179e-06 ;
Time (1,1,2) =  1.36346e-06 ;
Time (1,1,4) =  1.35507e-06 ;
Time (1,1,8) =  1.31968e-06 ;
Time (1,1,16) =  1.31316e-06 ;
Time (1,1,32) =   1.3113e-06 ;
Time (1,1,64) =  1.30944e-06 ;
Time (1,1,128) =  1.32155e-06 ;
Time (1,1,160) =  1.31037e-06 ;
Time (2,1,1) =  1.48173e-06 ;
Time (2,1,2) =  1.39698e-06 ;
Time (2,1,4) =  1.38488e-06 ;
Time (2,1,8) =  1.36252e-06 ;
Time (2,1,16) =  1.36159e-06 ;
Time (2,1,32) =   1.3588e-06 ;
Time (2,1,64) =  1.35042e-06 ;
Time (2,1,128) =  1.35973e-06 ;
Time (2,1,160) =  1.35694e-06 ;
Time (3,1,1) =  9.62056e-07 ;
Time (3,1,2) =  9.28529e-07 ;
Time (3,1,4) =  8.54954e-07 ;
Time (3,1,8) =  8.60542e-07 ;
Time (3,1,16) =   8.5216e-07 ;
Time (3,1,32) =  8.49366e-07 ;
Time (3,1,64) =  8.54023e-07 ;
Time (3,1,128) =  8.40053e-07 ;
Time (3,1,160) =  8.43778e-07 ;
Time (4,1,1) =  1.77603e-06 ;
Time (4,1,2) =  1.69128e-06 ;
Time (4,1,4) =  1.69966e-06 ;
Time (4,1,8) =  1.66334e-06 ;
Time (4,1,16) =  1.65682e-06 ;
Time (4,1,32) =  1.64192e-06 ;
Time (4,1,64) =  1.65589e-06 ;
Time (4,1,128) =  1.67545e-06 ;
Time (4,1,160) =  1.65589e-06 ;
Time (5,1,1) =  1.31316e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 31 ;
Ntri (id) = 12 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.17929e-07 ;
T_prep (1,1,2) =  1.96509e-07 ;
T_prep (1,1,4) =  1.91852e-07 ;
T_prep (1,1,8) =  1.79745e-07 ;
T_prep (1,1,16) =  1.78814e-07 ;
T_prep (1,1,32) =  1.69501e-07 ;
T_prep (1,1,64) =  1.70432e-07 ;
T_prep (1,1,128) =  1.68569e-07 ;
T_prep (1,1,160) =  1.64844e-07 ;
T_prep (2,1,1) =  3.05474e-07 ;
T_prep (2,1,2) =  2.81259e-07 ;
T_prep (2,1,4) =  2.32831e-07 ;
T_prep (2,1,8) =  2.08616e-07 ;
T_prep (2,1,16) =  2.15136e-07 ;
T_prep (2,1,32) =  1.94646e-07 ;
T_prep (2,1,64) =  2.02097e-07 ;
T_prep (2,1,128) =  1.99303e-07 ;
T_prep (2,1,160) =  1.92784e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  5.03846e-07 ;
Time (1,1,2) =  4.35859e-07 ;
Time (1,1,4) =  3.61353e-07 ;
Time (1,1,8) =  3.62284e-07 ;
Time (1,1,16) =   3.5204e-07 ;
Time (1,1,32) =  3.68804e-07 ;
Time (1,1,64) =  3.42727e-07 ;
Time (1,1,128) =  3.53903e-07 ;
Time (1,1,160) =  3.45521e-07 ;
Time (2,1,1) =  6.57514e-07 ;
Time (2,1,2) =  5.08502e-07 ;
Time (2,1,4) =  4.74043e-07 ;
Time (2,1,8) =  4.45172e-07 ;
Time (2,1,16) =  4.47035e-07 ;
Time (2,1,32) =   4.4331e-07 ;
Time (2,1,64) =  4.07919e-07 ;
Time (2,1,128) =   4.1537e-07 ;
Time (2,1,160) =  4.05125e-07 ;
Time (3,1,1) =  3.75323e-07 ;
Time (3,1,2) =  2.68221e-07 ;
Time (3,1,4) =  2.33762e-07 ;
Time (3,1,8) =  2.30968e-07 ;
Time (3,1,16) =  2.18861e-07 ;
Time (3,1,32) =  2.27243e-07 ;
Time (3,1,64) =  2.40281e-07 ;
Time (3,1,128) =  2.30968e-07 ;
Time (3,1,160) =  2.32831e-07 ;
Time (4,1,1) =  6.15604e-07 ;
Time (4,1,2) =  5.30854e-07 ;
Time (4,1,4) =  4.53554e-07 ;
Time (4,1,8) =  4.40516e-07 ;
Time (4,1,16) =  4.26546e-07 ;
Time (4,1,32) =  4.04194e-07 ;
Time (4,1,64) =  4.21889e-07 ;
Time (4,1,128) =  4.25614e-07 ;
Time (4,1,160) =   4.1537e-07 ;
Time (5,1,1) =  4.42378e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 31 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.78465e-07 ;
T_prep (1,1,2) =  1.92784e-07 ;
T_prep (1,1,4) =  1.88127e-07 ;
T_prep (1,1,8) =  1.85333e-07 ;
T_prep (1,1,16) =   1.6205e-07 ;
T_prep (1,1,32) =  1.63913e-07 ;
T_prep (1,1,64) =  1.63913e-07 ;
T_prep (1,1,128) =   1.6205e-07 ;
T_prep (1,1,160) =  1.51806e-07 ;
T_prep (2,1,1) =  3.68804e-07 ;
T_prep (2,1,2) =  3.25963e-07 ;
T_prep (2,1,4) =  2.54251e-07 ;
T_prep (2,1,8) =  2.41213e-07 ;
T_prep (2,1,16) =  1.98372e-07 ;
T_prep (2,1,32) =  1.83471e-07 ;
T_prep (2,1,64) =  1.71363e-07 ;
T_prep (2,1,128) =  1.51806e-07 ;
T_prep (2,1,160) =  1.52737e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  4.66593e-07 ;
Time (1,1,2) =  3.85568e-07 ;
Time (1,1,4) =  3.55765e-07 ;
Time (1,1,8) =  3.62284e-07 ;
Time (1,1,16) =  3.36207e-07 ;
Time (1,1,32) =  3.49246e-07 ;
Time (1,1,64) =  3.39933e-07 ;
Time (1,1,128) =  3.47383e-07 ;
Time (1,1,160) =  3.48315e-07 ;
Time (2,1,1) =  6.66827e-07 ;
Time (2,1,2) =   5.0012e-07 ;
Time (2,1,4) =  4.06057e-07 ;
Time (2,1,8) =  3.83705e-07 ;
Time (2,1,16) =  3.78117e-07 ;
Time (2,1,32) =  3.82774e-07 ;
Time (2,1,64) =  3.61353e-07 ;
Time (2,1,128) =  3.78117e-07 ;
Time (2,1,160) =  3.68804e-07 ;
Time (3,1,1) =  5.52274e-07 ;
Time (3,1,2) =  4.14439e-07 ;
Time (3,1,4) =  3.48315e-07 ;
Time (3,1,8) =  2.86847e-07 ;
Time (3,1,16) =  2.57976e-07 ;
Time (3,1,32) =  2.79397e-07 ;
Time (3,1,64) =  2.79397e-07 ;
Time (3,1,128) =  2.56114e-07 ;
Time (3,1,160) =  2.45869e-07 ;
Time (4,1,1) =  7.47852e-07 ;
Time (4,1,2) =  4.62867e-07 ;
Time (4,1,4) =  4.16301e-07 ;
Time (4,1,8) =  4.25614e-07 ;
Time (4,1,16) =  4.23752e-07 ;
Time (4,1,32) =  4.37722e-07 ;
Time (4,1,64) =  4.12576e-07 ;
Time (4,1,128) =   4.1537e-07 ;
Time (4,1,160) =  4.06057e-07 ;
Time (5,1,1) =  5.71832e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20 ;
Nedges (id) = 24 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.83471e-07 ;
T_prep (1,1,2) =  1.77883e-07 ;
T_prep (1,1,4) =  1.81608e-07 ;
T_prep (1,1,8) =  1.59256e-07 ;
T_prep (1,1,16) =  1.60187e-07 ;
T_prep (1,1,32) =  1.56462e-07 ;
T_prep (1,1,64) =  1.51806e-07 ;
T_prep (1,1,128) =  1.51806e-07 ;
T_prep (1,1,160) =  1.36904e-07 ;
T_prep (2,1,1) =  2.59839e-07 ;
T_prep (2,1,2) =  2.22586e-07 ;
T_prep (2,1,4) =   1.8999e-07 ;
T_prep (2,1,8) =  1.81608e-07 ;
T_prep (2,1,16) =  1.79745e-07 ;
T_prep (2,1,32) =  1.75089e-07 ;
T_prep (2,1,64) =  1.72295e-07 ;
T_prep (2,1,128) =  1.74157e-07 ;
T_prep (2,1,160) =  1.65775e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  2.71015e-07 ;
Time (1,1,2) =  3.04542e-07 ;
Time (1,1,4) =  2.89641e-07 ;
Time (1,1,8) =  2.86847e-07 ;
Time (1,1,16) =  2.98955e-07 ;
Time (1,1,32) =  2.91504e-07 ;
Time (1,1,64) =   2.8871e-07 ;
Time (1,1,128) =  3.04542e-07 ;
Time (1,1,160) =  2.94298e-07 ;
Time (2,1,1) =  4.28408e-07 ;
Time (2,1,2) =  3.17581e-07 ;
Time (2,1,4) =  3.11993e-07 ;
Time (2,1,8) =  3.08268e-07 ;
Time (2,1,16) =  2.99886e-07 ;
Time (2,1,32) =  3.05474e-07 ;
Time (2,1,64) =  2.91504e-07 ;
Time (2,1,128) =  2.95229e-07 ;
Time (2,1,160) =  2.99886e-07 ;
Time (3,1,1) =  2.15136e-07 ;
Time (3,1,2) =  1.65775e-07 ;
Time (3,1,4) =  1.49943e-07 ;
Time (3,1,8) =  1.42492e-07 ;
Time (3,1,16) =  1.32248e-07 ;
Time (3,1,32) =  1.33179e-07 ;
Time (3,1,64) =  1.36904e-07 ;
Time (3,1,128) =   1.4063e-07 ;
Time (3,1,160) =  1.24797e-07 ;
Time (4,1,1) =  5.12227e-07 ;
Time (4,1,2) =  3.80911e-07 ;
Time (4,1,4) =  3.64147e-07 ;
Time (4,1,8) =  3.50177e-07 ;
Time (4,1,16) =   3.3807e-07 ;
Time (4,1,32) =   3.5204e-07 ;
Time (4,1,64) =  3.61353e-07 ;
Time (4,1,128) =  3.41795e-07 ;
Time (4,1,160) =  3.31551e-07 ;
Time (5,1,1) =   3.8743e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 720 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.29105e-06 ;
T_prep (1,1,2) =  2.30502e-06 ;
T_prep (1,1,4) =  2.30782e-06 ;
T_prep (1,1,8) =  2.28081e-06 ;
T_prep (1,1,16) =  2.28267e-06 ;
T_prep (1,1,32) =  2.28733e-06 ;
T_prep (1,1,64) =   2.2864e-06 ;
T_prep (1,1,128) =  2.27708e-06 ;
T_prep (1,1,160) =  2.28547e-06 ;
T_prep (2,1,1) =    2.563e-06 ;
T_prep (2,1,2) =  2.31806e-06 ;
T_prep (2,1,4) =  2.26032e-06 ;
T_prep (2,1,8) =  2.24821e-06 ;
T_prep (2,1,16) =  2.27895e-06 ;
T_prep (2,1,32) =  2.26591e-06 ;
T_prep (2,1,64) =  2.26963e-06 ;
T_prep (2,1,128) =   2.2687e-06 ;
T_prep (2,1,160) =  2.26125e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  3.36953e-06 ;
Time (1,1,2) =  3.43658e-06 ;
Time (1,1,4) =  3.38163e-06 ;
Time (1,1,8) =  3.37046e-06 ;
Time (1,1,16) =  3.37232e-06 ;
Time (1,1,32) =  3.38908e-06 ;
Time (1,1,64) =  3.38908e-06 ;
Time (1,1,128) =  3.37698e-06 ;
Time (1,1,160) =  3.36766e-06 ;
Time (2,1,1) =  3.65917e-06 ;
Time (2,1,2) =  3.59304e-06 ;
Time (2,1,4) =  3.55113e-06 ;
Time (2,1,8) =  3.54182e-06 ;
Time (2,1,16) =  3.54182e-06 ;
Time (2,1,32) =  3.50829e-06 ;
Time (2,1,64) =   3.5353e-06 ;
Time (2,1,128) =  3.52599e-06 ;
Time (2,1,160) =  3.52506e-06 ;
Time (3,1,1) =  2.25566e-06 ;
Time (3,1,2) =  2.18861e-06 ;
Time (3,1,4) =  2.16905e-06 ;
Time (3,1,8) =  2.16346e-06 ;
Time (3,1,16) =  2.13925e-06 ;
Time (3,1,32) =  2.15974e-06 ;
Time (3,1,64) =  2.15694e-06 ;
Time (3,1,128) =  2.14018e-06 ;
Time (3,1,160) =  2.15787e-06 ;
Time (4,1,1) =  4.81401e-06 ;
Time (4,1,2) =  4.44893e-06 ;
Time (4,1,4) =  4.44707e-06 ;
Time (4,1,8) =   4.4331e-06 ;
Time (4,1,16) =  4.42471e-06 ;
Time (4,1,32) =  4.38932e-06 ;
Time (4,1,64) =  4.41633e-06 ;
Time (4,1,128) =   4.4303e-06 ;
Time (4,1,160) =  4.42378e-06 ;
Time (5,1,1) =  2.61329e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 49 ;
Ntri (id) = 20 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.71015e-07 ;
T_prep (1,1,2) =  2.56114e-07 ;
T_prep (1,1,4) =  2.36556e-07 ;
T_prep (1,1,8) =  2.18861e-07 ;
T_prep (1,1,16) =   2.2538e-07 ;
T_prep (1,1,32) =  2.18861e-07 ;
T_prep (1,1,64) =  2.16067e-07 ;
T_prep (1,1,128) =  2.12342e-07 ;
T_prep (1,1,160) =  2.06754e-07 ;
T_prep (2,1,1) =  4.03263e-07 ;
T_prep (2,1,2) =  2.91504e-07 ;
T_prep (2,1,4) =  2.73809e-07 ;
T_prep (2,1,8) =  2.34693e-07 ;
T_prep (2,1,16) =  2.33762e-07 ;
T_prep (2,1,32) =  2.33762e-07 ;
T_prep (2,1,64) =  2.29105e-07 ;
T_prep (2,1,128) =  2.26311e-07 ;
T_prep (2,1,160) =  2.30968e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  5.32717e-07 ;
Time (1,1,2) =  4.97326e-07 ;
Time (1,1,4) =  4.58211e-07 ;
Time (1,1,8) =  4.49829e-07 ;
Time (1,1,16) =  4.38653e-07 ;
Time (1,1,32) =  4.30271e-07 ;
Time (1,1,64) =  4.33996e-07 ;
Time (1,1,128) =  4.32134e-07 ;
Time (1,1,160) =   4.3679e-07 ;
Time (2,1,1) =  8.08388e-07 ;
Time (2,1,2) =  6.67758e-07 ;
Time (2,1,4) =  6.23055e-07 ;
Time (2,1,8) =  6.25849e-07 ;
Time (2,1,16) =  5.86733e-07 ;
Time (2,1,32) =  5.94184e-07 ;
Time (2,1,64) =  5.90459e-07 ;
Time (2,1,128) =  5.65313e-07 ;
Time (2,1,160) =  5.70901e-07 ;
Time (3,1,1) =  4.48897e-07 ;
Time (3,1,2) =   3.6601e-07 ;
Time (3,1,4) =  3.22238e-07 ;
Time (3,1,8) =  3.18512e-07 ;
Time (3,1,16) =  3.21306e-07 ;
Time (3,1,32) =  3.27826e-07 ;
Time (3,1,64) =  3.32482e-07 ;
Time (3,1,128) =  3.13856e-07 ;
Time (3,1,160) =  3.20375e-07 ;
Time (4,1,1) =  7.54371e-07 ;
Time (4,1,2) =  6.09085e-07 ;
Time (4,1,4) =  5.70901e-07 ;
Time (4,1,8) =  5.76489e-07 ;
Time (4,1,16) =  5.37373e-07 ;
Time (4,1,32) =  5.53206e-07 ;
Time (4,1,64) =  5.44824e-07 ;
Time (4,1,128) =  5.30854e-07 ;
Time (4,1,160) =  5.21541e-07 ;
Time (5,1,1) =  4.40516e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 49 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   3.3807e-07 ;
T_prep (1,1,2) =  2.89641e-07 ;
T_prep (1,1,4) =  2.78465e-07 ;
T_prep (1,1,8) =  2.44938e-07 ;
T_prep (1,1,16) =  2.38419e-07 ;
T_prep (1,1,32) =  2.29105e-07 ;
T_prep (1,1,64) =  2.21655e-07 ;
T_prep (1,1,128) =  2.18861e-07 ;
T_prep (1,1,160) =  2.10479e-07 ;
T_prep (2,1,1) =  4.55417e-07 ;
T_prep (2,1,2) =  3.56697e-07 ;
T_prep (2,1,4) =   3.1665e-07 ;
T_prep (2,1,8) =  2.81259e-07 ;
T_prep (2,1,16) =  2.54251e-07 ;
T_prep (2,1,32) =  2.30968e-07 ;
T_prep (2,1,64) =  2.33762e-07 ;
T_prep (2,1,128) =    2.468e-07 ;
T_prep (2,1,160) =  2.22586e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  5.25266e-07 ;
Time (1,1,2) =  4.90807e-07 ;
Time (1,1,4) =  4.37722e-07 ;
Time (1,1,8) =  4.35859e-07 ;
Time (1,1,16) =  4.35859e-07 ;
Time (1,1,32) =  4.48897e-07 ;
Time (1,1,64) =   4.2934e-07 ;
Time (1,1,128) =  4.32134e-07 ;
Time (1,1,160) =  4.34928e-07 ;
Time (2,1,1) =  7.47852e-07 ;
Time (2,1,2) =  5.76489e-07 ;
Time (2,1,4) =  5.41098e-07 ;
Time (2,1,8) =   5.0012e-07 ;
Time (2,1,16) =   4.9267e-07 ;
Time (2,1,32) =  5.05708e-07 ;
Time (2,1,64) =  4.96395e-07 ;
Time (2,1,128) =   5.0012e-07 ;
Time (2,1,160) =  4.94532e-07 ;
Time (3,1,1) =  5.55068e-07 ;
Time (3,1,2) =  3.97675e-07 ;
Time (3,1,4) =   3.8743e-07 ;
Time (3,1,8) =  3.21306e-07 ;
Time (3,1,16) =  3.01749e-07 ;
Time (3,1,32) =  3.00817e-07 ;
Time (3,1,64) =   3.0268e-07 ;
Time (3,1,128) =  2.84985e-07 ;
Time (3,1,160) =  3.04542e-07 ;
Time (4,1,1) =  7.50646e-07 ;
Time (4,1,2) =  5.68107e-07 ;
Time (4,1,4) =  5.19678e-07 ;
Time (4,1,8) =  5.39236e-07 ;
Time (4,1,16) =  5.26197e-07 ;
Time (4,1,32) =  5.10365e-07 ;
Time (4,1,64) =  5.29923e-07 ;
Time (4,1,128) =  5.32717e-07 ;
Time (4,1,160) =  5.16884e-07 ;
Time (5,1,1) =  5.73695e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 30 ;
Nedges (id) = 40 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.29105e-07 ;
T_prep (1,1,2) =  2.23517e-07 ;
T_prep (1,1,4) =  2.09548e-07 ;
T_prep (1,1,8) =  2.16998e-07 ;
T_prep (1,1,16) =  2.09548e-07 ;
T_prep (1,1,32) =  2.08616e-07 ;
T_prep (1,1,64) =  2.05822e-07 ;
T_prep (1,1,128) =  2.04891e-07 ;
T_prep (1,1,160) =   1.8999e-07 ;
T_prep (2,1,1) =  2.95229e-07 ;
T_prep (2,1,2) =  2.30037e-07 ;
T_prep (2,1,4) =  2.15136e-07 ;
T_prep (2,1,8) =  2.27243e-07 ;
T_prep (2,1,16) =  2.13273e-07 ;
T_prep (2,1,32) =  1.98372e-07 ;
T_prep (2,1,64) =  2.01166e-07 ;
T_prep (2,1,128) =  1.92784e-07 ;
T_prep (2,1,160) =   1.9744e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  3.62284e-07 ;
Time (1,1,2) =  4.21889e-07 ;
Time (1,1,4) =  3.98606e-07 ;
Time (1,1,8) =  3.80911e-07 ;
Time (1,1,16) =   3.8743e-07 ;
Time (1,1,32) =  3.74392e-07 ;
Time (1,1,64) =  3.72529e-07 ;
Time (1,1,128) =  3.84636e-07 ;
Time (1,1,160) =  3.82774e-07 ;
Time (2,1,1) =  5.18747e-07 ;
Time (2,1,2) =  4.08851e-07 ;
Time (2,1,4) =  3.98606e-07 ;
Time (2,1,8) =  4.05125e-07 ;
Time (2,1,16) =  3.95812e-07 ;
Time (2,1,32) =  3.97675e-07 ;
Time (2,1,64) =  3.84636e-07 ;
Time (2,1,128) =  3.97675e-07 ;
Time (2,1,160) =  3.86499e-07 ;
Time (3,1,1) =  2.54251e-07 ;
Time (3,1,2) =  2.03028e-07 ;
Time (3,1,4) =  1.77883e-07 ;
Time (3,1,8) =  1.88127e-07 ;
Time (3,1,16) =  1.84402e-07 ;
Time (3,1,32) =  1.74157e-07 ;
Time (3,1,64) =  1.73226e-07 ;
Time (3,1,128) =  1.80677e-07 ;
Time (3,1,160) =  1.76951e-07 ;
Time (4,1,1) =  5.59725e-07 ;
Time (4,1,2) =  4.93601e-07 ;
Time (4,1,4) =  4.74043e-07 ;
Time (4,1,8) =   4.5076e-07 ;
Time (4,1,16) =  4.39584e-07 ;
Time (4,1,32) =  4.65661e-07 ;
Time (4,1,64) =  4.62867e-07 ;
Time (4,1,128) =  4.53554e-07 ;
Time (4,1,160) =  4.47035e-07 ;
Time (5,1,1) =  4.37722e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 104 ;
Ntri (id) = 45 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  4.95464e-07 ;
T_prep (1,1,2) =   5.0012e-07 ;
T_prep (1,1,4) =  4.82425e-07 ;
T_prep (1,1,8) =  4.77768e-07 ;
T_prep (1,1,16) =  4.74975e-07 ;
T_prep (1,1,32) =  4.69387e-07 ;
T_prep (1,1,64) =  4.80562e-07 ;
T_prep (1,1,128) =  4.70318e-07 ;
T_prep (1,1,160) =  4.58211e-07 ;
T_prep (2,1,1) =  5.62519e-07 ;
T_prep (2,1,2) =  4.55417e-07 ;
T_prep (2,1,4) =   4.2934e-07 ;
T_prep (2,1,8) =  4.12576e-07 ;
T_prep (2,1,16) =  4.11645e-07 ;
T_prep (2,1,32) =  4.11645e-07 ;
T_prep (2,1,64) =  4.12576e-07 ;
T_prep (2,1,128) =  4.14439e-07 ;
T_prep (2,1,160) =  4.07919e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  8.37259e-07 ;
Time (1,1,2) =  8.15839e-07 ;
Time (1,1,4) =   7.7486e-07 ;
Time (1,1,8) =  7.71135e-07 ;
Time (1,1,16) =   7.8883e-07 ;
Time (1,1,32) =  7.85105e-07 ;
Time (1,1,64) =   7.7486e-07 ;
Time (1,1,128) =  7.77654e-07 ;
Time (1,1,160) =   7.6741e-07 ;
Time (2,1,1) =  1.23866e-06 ;
Time (2,1,2) =  1.12318e-06 ;
Time (2,1,4) =  1.08033e-06 ;
Time (2,1,8) =  1.06916e-06 ;
Time (2,1,16) =  1.06636e-06 ;
Time (2,1,32) =  1.03097e-06 ;
Time (2,1,64) =  1.01514e-06 ;
Time (2,1,128) =  1.04681e-06 ;
Time (2,1,160) =  1.05146e-06 ;
Time (3,1,1) =  7.59959e-07 ;
Time (3,1,2) =  5.97909e-07 ;
Time (3,1,4) =   5.7742e-07 ;
Time (3,1,8) =  5.70901e-07 ;
Time (3,1,16) =  5.76489e-07 ;
Time (3,1,32) =  5.73695e-07 ;
Time (3,1,64) =  5.69969e-07 ;
Time (3,1,128) =  5.74626e-07 ;
Time (3,1,160) =  5.83939e-07 ;
Time (4,1,1) =  1.15391e-06 ;
Time (4,1,2) =  1.02352e-06 ;
Time (4,1,4) =  9.36911e-07 ;
Time (4,1,8) =  9.21078e-07 ;
Time (4,1,16) =  9.33185e-07 ;
Time (4,1,32) =  9.34117e-07 ;
Time (4,1,64) =  9.18284e-07 ;
Time (4,1,128) =  9.09902e-07 ;
Time (4,1,160) =  9.18284e-07 ;
Time (5,1,1) =  7.31088e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 104 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  5.08502e-07 ;
T_prep (1,1,2) =  4.59142e-07 ;
T_prep (1,1,4) =  4.38653e-07 ;
T_prep (1,1,8) =  4.32134e-07 ;
T_prep (1,1,16) =  4.23752e-07 ;
T_prep (1,1,32) =  4.16301e-07 ;
T_prep (1,1,64) =  4.12576e-07 ;
T_prep (1,1,128) =  4.07919e-07 ;
T_prep (1,1,160) =  4.07919e-07 ;
T_prep (2,1,1) =  6.88247e-07 ;
T_prep (2,1,2) =  5.71832e-07 ;
T_prep (2,1,4) =  5.05708e-07 ;
T_prep (2,1,8) =  4.76837e-07 ;
T_prep (2,1,16) =  4.76837e-07 ;
T_prep (2,1,32) =  4.81494e-07 ;
T_prep (2,1,64) =  4.73112e-07 ;
T_prep (2,1,128) =   4.4331e-07 ;
T_prep (2,1,160) =   4.4331e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  7.86968e-07 ;
Time (1,1,2) =  7.27363e-07 ;
Time (1,1,4) =  7.17118e-07 ;
Time (1,1,8) =  7.13393e-07 ;
Time (1,1,16) =  7.06874e-07 ;
Time (1,1,32) =  7.20844e-07 ;
Time (1,1,64) =  6.96629e-07 ;
Time (1,1,128) =  7.09668e-07 ;
Time (1,1,160) =  7.17118e-07 ;
Time (2,1,1) =  1.06264e-06 ;
Time (2,1,2) =  8.75443e-07 ;
Time (2,1,4) =  8.19564e-07 ;
Time (2,1,8) =  8.07457e-07 ;
Time (2,1,16) =  8.04663e-07 ;
Time (2,1,32) =  7.93487e-07 ;
Time (2,1,64) =  7.98143e-07 ;
Time (2,1,128) =  7.85105e-07 ;
Time (2,1,160) =  7.96281e-07 ;
Time (3,1,1) =  9.63919e-07 ;
Time (3,1,2) =  6.51926e-07 ;
Time (3,1,4) =  6.35162e-07 ;
Time (3,1,8) =  6.13742e-07 ;
Time (3,1,16) =  5.92321e-07 ;
Time (3,1,32) =  5.82077e-07 ;
Time (3,1,64) =  5.68107e-07 ;
Time (3,1,128) =  5.80214e-07 ;
Time (3,1,160) =  5.50412e-07 ;
Time (4,1,1) =  1.07381e-06 ;
Time (4,1,2) =   9.1549e-07 ;
Time (4,1,4) =  8.67061e-07 ;
Time (4,1,8) =  8.60542e-07 ;
Time (4,1,16) =  8.58679e-07 ;
Time (4,1,32) =  8.48435e-07 ;
Time (4,1,64) =   8.3819e-07 ;
Time (4,1,128) =   8.5216e-07 ;
Time (4,1,160) =  8.58679e-07 ;
Time (5,1,1) =  7.92556e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 60 ;
Nedges (id) = 90 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  3.80911e-07 ;
T_prep (1,1,2) =  4.06057e-07 ;
T_prep (1,1,4) =  3.91155e-07 ;
T_prep (1,1,8) =  3.98606e-07 ;
T_prep (1,1,16) =  3.80911e-07 ;
T_prep (1,1,32) =  3.82774e-07 ;
T_prep (1,1,64) =  3.82774e-07 ;
T_prep (1,1,128) =  3.79048e-07 ;
T_prep (1,1,160) =  3.74392e-07 ;
T_prep (2,1,1) =  4.69387e-07 ;
T_prep (2,1,2) =   4.1537e-07 ;
T_prep (2,1,4) =  3.84636e-07 ;
T_prep (2,1,8) =  3.84636e-07 ;
T_prep (2,1,16) =  3.80911e-07 ;
T_prep (2,1,32) =  3.69735e-07 ;
T_prep (2,1,64) =  3.72529e-07 ;
T_prep (2,1,128) =  3.86499e-07 ;
T_prep (2,1,160) =  3.67872e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  6.42613e-07 ;
Time (1,1,2) =  6.34231e-07 ;
Time (1,1,4) =  6.33299e-07 ;
Time (1,1,8) =  6.44475e-07 ;
Time (1,1,16) =  6.38887e-07 ;
Time (1,1,32) =  6.37025e-07 ;
Time (1,1,64) =  6.36093e-07 ;
Time (1,1,128) =   6.2678e-07 ;
Time (1,1,160) =  6.23986e-07 ;
Time (2,1,1) =  8.06525e-07 ;
Time (2,1,2) =  6.82659e-07 ;
Time (2,1,4) =  6.83591e-07 ;
Time (2,1,8) =  6.67758e-07 ;
Time (2,1,16) =  6.74278e-07 ;
Time (2,1,32) =  6.69621e-07 ;
Time (2,1,64) =  6.78003e-07 ;
Time (2,1,128) =  6.64033e-07 ;
Time (2,1,160) =  6.64033e-07 ;
Time (3,1,1) =  4.35859e-07 ;
Time (3,1,2) =  3.66941e-07 ;
Time (3,1,4) =  3.43658e-07 ;
Time (3,1,8) =  3.43658e-07 ;
Time (3,1,16) =  3.43658e-07 ;
Time (3,1,32) =  3.36207e-07 ;
Time (3,1,64) =  3.29688e-07 ;
Time (3,1,128) =  3.26894e-07 ;
Time (3,1,160) =  3.32482e-07 ;
Time (4,1,1) =  8.68924e-07 ;
Time (4,1,2) =    8.028e-07 ;
Time (4,1,4) =  7.72998e-07 ;
Time (4,1,8) =  7.93487e-07 ;
Time (4,1,16) =  7.75792e-07 ;
Time (4,1,32) =  7.72066e-07 ;
Time (4,1,64) =  7.68341e-07 ;
Time (4,1,128) =  7.54371e-07 ;
Time (4,1,160) =  7.63685e-07 ;
Time (5,1,1) =  5.94184e-07 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 313 ;
Ntri (id) = 144 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   1.0645e-06 ;
T_prep (1,1,2) =  1.06078e-06 ;
T_prep (1,1,4) =  1.04494e-06 ;
T_prep (1,1,8) =  1.04774e-06 ;
T_prep (1,1,16) =  1.05053e-06 ;
T_prep (1,1,32) =  1.05146e-06 ;
T_prep (1,1,64) =  1.04308e-06 ;
T_prep (1,1,128) =  1.04588e-06 ;
T_prep (1,1,160) =  1.03936e-06 ;
T_prep (2,1,1) =  1.16974e-06 ;
T_prep (2,1,2) =  1.09244e-06 ;
T_prep (2,1,4) =  1.05519e-06 ;
T_prep (2,1,8) =  1.02352e-06 ;
T_prep (2,1,16) =  1.03842e-06 ;
T_prep (2,1,32) =  1.03936e-06 ;
T_prep (2,1,64) =  1.00769e-06 ;
T_prep (2,1,128) =  1.03284e-06 ;
T_prep (2,1,160) =  1.02911e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.98931e-06 ;
Time (1,1,2) =   1.9623e-06 ;
Time (1,1,4) =  1.94833e-06 ;
Time (1,1,8) =  1.92598e-06 ;
Time (1,1,16) =  1.93901e-06 ;
Time (1,1,32) =  1.93156e-06 ;
Time (1,1,64) =  1.91573e-06 ;
Time (1,1,128) =  1.90549e-06 ;
Time (1,1,160) =  1.93156e-06 ;
Time (2,1,1) =  2.89548e-06 ;
Time (2,1,2) =  2.76417e-06 ;
Time (2,1,4) =  2.68873e-06 ;
Time (2,1,8) =  2.66824e-06 ;
Time (2,1,16) =   2.6701e-06 ;
Time (2,1,32) =   2.6729e-06 ;
Time (2,1,64) =  2.68407e-06 ;
Time (2,1,128) =  2.64216e-06 ;
Time (2,1,160) =   2.6701e-06 ;
Time (3,1,1) =  1.70339e-06 ;
Time (3,1,2) =   1.5432e-06 ;
Time (3,1,4) =  1.52178e-06 ;
Time (3,1,8) =  1.52271e-06 ;
Time (3,1,16) =  1.52364e-06 ;
Time (3,1,32) =  1.51712e-06 ;
Time (3,1,64) =  1.52085e-06 ;
Time (3,1,128) =   1.5283e-06 ;
Time (3,1,160) =  1.53109e-06 ;
Time (4,1,1) =  2.80794e-06 ;
Time (4,1,2) =  2.35718e-06 ;
Time (4,1,4) =  2.31247e-06 ;
Time (4,1,8) =  2.33762e-06 ;
Time (4,1,16) =  2.31527e-06 ;
Time (4,1,32) =  2.36463e-06 ;
Time (4,1,64) =  2.30316e-06 ;
Time (4,1,128) =  2.30409e-06 ;
Time (4,1,160) =  2.29664e-06 ;
Time (5,1,1) =  1.41468e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 313 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.08592e-06 ;
T_prep (1,1,2) =  1.08127e-06 ;
T_prep (1,1,4) =  1.07568e-06 ;
T_prep (1,1,8) =  1.07288e-06 ;
T_prep (1,1,16) =   1.0645e-06 ;
T_prep (1,1,32) =  1.08033e-06 ;
T_prep (1,1,64) =  1.06636e-06 ;
T_prep (1,1,128) =   1.0645e-06 ;
T_prep (1,1,160) =  1.05333e-06 ;
T_prep (2,1,1) =   1.3439e-06 ;
T_prep (2,1,2) =  1.20979e-06 ;
T_prep (2,1,4) =  1.12504e-06 ;
T_prep (2,1,8) =  1.11759e-06 ;
T_prep (2,1,16) =  1.11014e-06 ;
T_prep (2,1,32) =  1.08127e-06 ;
T_prep (2,1,64) =  1.08778e-06 ;
T_prep (2,1,128) =  1.08965e-06 ;
T_prep (2,1,160) =   1.0794e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.81515e-06 ;
Time (1,1,2) =  1.78814e-06 ;
Time (1,1,4) =  1.77044e-06 ;
Time (1,1,8) =  1.77138e-06 ;
Time (1,1,16) =  1.76765e-06 ;
Time (1,1,32) =  1.76206e-06 ;
Time (1,1,64) =  1.78535e-06 ;
Time (1,1,128) =   1.7751e-06 ;
Time (1,1,160) =  1.78162e-06 ;
Time (2,1,1) =  2.21282e-06 ;
Time (2,1,2) =  2.00048e-06 ;
Time (2,1,4) =  1.95485e-06 ;
Time (2,1,8) =  1.93994e-06 ;
Time (2,1,16) =  1.94553e-06 ;
Time (2,1,32) =  1.95112e-06 ;
Time (2,1,64) =  1.92598e-06 ;
Time (2,1,128) =  1.92598e-06 ;
Time (2,1,160) =  1.92598e-06 ;
Time (3,1,1) =  1.77044e-06 ;
Time (3,1,2) =  1.41002e-06 ;
Time (3,1,4) =   1.3262e-06 ;
Time (3,1,8) =  1.33179e-06 ;
Time (3,1,16) =  1.31223e-06 ;
Time (3,1,32) =  1.27871e-06 ;
Time (3,1,64) =  1.30106e-06 ;
Time (3,1,128) =  1.28616e-06 ;
Time (3,1,160) =  1.27871e-06 ;
Time (4,1,1) =  2.71481e-06 ;
Time (4,1,2) =  2.24076e-06 ;
Time (4,1,4) =  2.18395e-06 ;
Time (4,1,8) =  2.17743e-06 ;
Time (4,1,16) =  2.24821e-06 ;
Time (4,1,32) =  2.21096e-06 ;
Time (4,1,64) =  2.20723e-06 ;
Time (4,1,128) =  2.18023e-06 ;
Time (4,1,160) =  2.17743e-06 ;
Time (5,1,1) =  1.65123e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170 ;
Nedges (id) = 288 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.02445e-06 ;
T_prep (1,1,2) =  1.02073e-06 ;
T_prep (1,1,4) =  1.01421e-06 ;
T_prep (1,1,8) =  9.97446e-07 ;
T_prep (1,1,16) =  1.00583e-06 ;
T_prep (1,1,32) =  1.00583e-06 ;
T_prep (1,1,64) =  9.85339e-07 ;
T_prep (1,1,128) =  9.95584e-07 ;
T_prep (1,1,160) =  9.97446e-07 ;
T_prep (2,1,1) =  1.05146e-06 ;
T_prep (2,1,2) =  9.93721e-07 ;
T_prep (2,1,4) =   9.7882e-07 ;
T_prep (2,1,8) =   9.5088e-07 ;
T_prep (2,1,16) =  9.71369e-07 ;
T_prep (2,1,32) =  9.62988e-07 ;
T_prep (2,1,64) =  9.69507e-07 ;
T_prep (2,1,128) =  9.73232e-07 ;
T_prep (2,1,160) =  9.71369e-07 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.64751e-06 ;
Time (1,1,2) =  1.68011e-06 ;
Time (1,1,4) =  1.65403e-06 ;
Time (1,1,8) =  1.67266e-06 ;
Time (1,1,16) =    1.668e-06 ;
Time (1,1,32) =  1.67359e-06 ;
Time (1,1,64) =  1.65775e-06 ;
Time (1,1,128) =  1.66614e-06 ;
Time (1,1,160) =  1.65589e-06 ;
Time (2,1,1) =  1.78721e-06 ;
Time (2,1,2) =  1.71084e-06 ;
Time (2,1,4) =  1.68849e-06 ;
Time (2,1,8) =  1.67731e-06 ;
Time (2,1,16) =  1.67079e-06 ;
Time (2,1,32) =  1.66427e-06 ;
Time (2,1,64) =  1.66055e-06 ;
Time (2,1,128) =  1.67359e-06 ;
Time (2,1,160) =  4.45545e-06 ;
Time (3,1,1) =  3.17954e-06 ;
Time (3,1,2) =  3.09013e-06 ;
Time (3,1,4) =  2.29478e-06 ;
Time (3,1,8) =  2.18954e-06 ;
Time (3,1,16) =  2.53879e-06 ;
Time (3,1,32) =  2.30689e-06 ;
Time (3,1,64) =   2.2836e-06 ;
Time (3,1,128) =  2.27429e-06 ;
Time (3,1,160) =  2.25659e-06 ;
Time (4,1,1) =  3.95253e-06 ;
Time (4,1,2) =  2.07685e-06 ;
Time (4,1,4) =  2.04705e-06 ;
Time (4,1,8) =  2.07219e-06 ;
Time (4,1,16) =  2.03587e-06 ;
Time (4,1,32) =  2.04425e-06 ;
Time (4,1,64) =  2.04891e-06 ;
Time (4,1,128) =  2.05543e-06 ;
Time (4,1,160) =   2.0368e-06 ;
Time (5,1,1) =  1.23587e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 940 ;
Ntri (id) = 821 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  2.84426e-06 ;
T_prep (1,1,2) =  2.85823e-06 ;
T_prep (1,1,4) =  2.84426e-06 ;
T_prep (1,1,8) =  2.84426e-06 ;
T_prep (1,1,16) =  2.85171e-06 ;
T_prep (1,1,32) =  2.84612e-06 ;
T_prep (1,1,64) =  2.82656e-06 ;
T_prep (1,1,128) =   2.8545e-06 ;
T_prep (1,1,160) =   2.8424e-06 ;
T_prep (2,1,1) =  3.27546e-06 ;
T_prep (2,1,2) =  2.95974e-06 ;
T_prep (2,1,4) =  2.92808e-06 ;
T_prep (2,1,8) =  2.89083e-06 ;
T_prep (2,1,16) =  2.91225e-06 ;
T_prep (2,1,32) =  2.89362e-06 ;
T_prep (2,1,64) =  2.90759e-06 ;
T_prep (2,1,128) =   2.9048e-06 ;
T_prep (2,1,160) =  2.88337e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  5.47059e-06 ;
Time (1,1,2) =  5.49387e-06 ;
Time (1,1,4) =  5.44265e-06 ;
Time (1,1,8) =  5.40633e-06 ;
Time (1,1,16) =  5.48828e-06 ;
Time (1,1,32) =  5.43427e-06 ;
Time (1,1,64) =  5.41005e-06 ;
Time (1,1,128) =  5.43334e-06 ;
Time (1,1,160) =  5.43706e-06 ;
Time (2,1,1) =  9.78354e-06 ;
Time (2,1,2) =   9.6336e-06 ;
Time (2,1,4) =  9.47621e-06 ;
Time (2,1,8) =   9.4939e-06 ;
Time (2,1,16) =  9.57586e-06 ;
Time (2,1,32) =  9.56282e-06 ;
Time (2,1,64) =  9.56003e-06 ;
Time (2,1,128) =  9.55351e-06 ;
Time (2,1,160) =  9.55444e-06 ;
Time (3,1,1) =  1.49552e-05 ;
Time (3,1,2) =  1.40956e-05 ;
Time (3,1,4) =  1.38776e-05 ;
Time (3,1,8) =  1.41235e-05 ;
Time (3,1,16) =  1.40201e-05 ;
Time (3,1,32) =  1.43675e-05 ;
Time (3,1,64) =  1.40676e-05 ;
Time (3,1,128) =  1.40695e-05 ;
Time (3,1,160) =  1.39903e-05 ;
Time (4,1,1) =  7.66199e-06 ;
Time (4,1,2) =  7.00355e-06 ;
Time (4,1,4) =  6.90762e-06 ;
Time (4,1,8) =  6.91973e-06 ;
Time (4,1,16) =  6.92904e-06 ;
Time (4,1,32) =  6.91321e-06 ;
Time (4,1,64) =  6.94115e-06 ;
Time (4,1,128) =  6.94953e-06 ;
Time (4,1,160) =  6.92811e-06 ;
Time (5,1,1) =   4.4331e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 300 ;
Nedges (id) = 940 ;
Ntri (id) = 7 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  3.15998e-06 ;
T_prep (1,1,2) =   3.1665e-06 ;
T_prep (1,1,4) =  3.12459e-06 ;
T_prep (1,1,8) =  3.12552e-06 ;
T_prep (1,1,16) =  3.16743e-06 ;
T_prep (1,1,32) =  3.12086e-06 ;
T_prep (1,1,64) =  3.15811e-06 ;
T_prep (1,1,128) =  3.12086e-06 ;
T_prep (1,1,160) =  3.12086e-06 ;
T_prep (2,1,1) =  3.85195e-06 ;
T_prep (2,1,2) =  3.33879e-06 ;
T_prep (2,1,4) =  3.30154e-06 ;
T_prep (2,1,8) =  3.23635e-06 ;
T_prep (2,1,16) =  3.24845e-06 ;
T_prep (2,1,32) =  3.23262e-06 ;
T_prep (2,1,64) =  3.20468e-06 ;
T_prep (2,1,128) =  3.24845e-06 ;
T_prep (2,1,160) =  3.21958e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  4.87361e-06 ;
Time (1,1,2) =  4.75533e-06 ;
Time (1,1,4) =  4.71342e-06 ;
Time (1,1,8) =  4.67338e-06 ;
Time (1,1,16) =  4.68548e-06 ;
Time (1,1,32) =  4.67338e-06 ;
Time (1,1,64) =  4.63333e-06 ;
Time (1,1,128) =  4.67524e-06 ;
Time (1,1,160) =   4.7246e-06 ;
Time (2,1,1) =  6.56303e-06 ;
Time (2,1,2) =  6.06477e-06 ;
Time (2,1,4) =   5.9614e-06 ;
Time (2,1,8) =  5.95301e-06 ;
Time (2,1,16) =  5.87478e-06 ;
Time (2,1,32) =   5.8813e-06 ;
Time (2,1,64) =  5.91204e-06 ;
Time (2,1,128) =  5.87106e-06 ;
Time (2,1,160) =   5.8813e-06 ;
Time (3,1,1) =  8.47131e-06 ;
Time (3,1,2) =  6.56676e-06 ;
Time (3,1,4) =  6.09644e-06 ;
Time (3,1,8) =  5.94649e-06 ;
Time (3,1,16) =  5.92694e-06 ;
Time (3,1,32) =   5.8068e-06 ;
Time (3,1,64) =   5.8338e-06 ;
Time (3,1,128) =  5.80121e-06 ;
Time (3,1,160) =  5.79469e-06 ;
Time (4,1,1) =  6.84988e-06 ;
Time (4,1,2) =  6.12438e-06 ;
Time (4,1,4) =  5.99585e-06 ;
Time (4,1,8) =  5.91204e-06 ;
Time (4,1,16) =  5.95767e-06 ;
Time (4,1,32) =  5.89434e-06 ;
Time (4,1,64) =  5.93718e-06 ;
Time (4,1,128) =  5.90924e-06 ;
Time (4,1,160) =  5.94184e-06 ;
Time (5,1,1) =  5.21354e-06 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 3448 ;
Ntri (id) = 3149 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.11377e-05 ;
T_prep (1,1,2) =  1.11032e-05 ;
T_prep (1,1,4) =  1.11293e-05 ;
T_prep (1,1,8) =  1.11032e-05 ;
T_prep (1,1,16) =  1.11964e-05 ;
T_prep (1,1,32) =  1.11992e-05 ;
T_prep (1,1,64) =  1.11926e-05 ;
T_prep (1,1,128) =  1.12327e-05 ;
T_prep (1,1,160) =  1.11638e-05 ;
T_prep (2,1,1) =  9.93721e-06 ;
T_prep (2,1,2) =  9.84408e-06 ;
T_prep (2,1,4) =  9.69041e-06 ;
T_prep (2,1,8) =  9.74443e-06 ;
T_prep (2,1,16) =  9.67924e-06 ;
T_prep (2,1,32) =  9.65502e-06 ;
T_prep (2,1,64) =  9.67272e-06 ;
T_prep (2,1,128) =  9.62615e-06 ;
T_prep (2,1,160) =  9.70438e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  2.10963e-05 ;
Time (1,1,2) =   0.00904292 ;
Time (1,1,4) =  9.35253e-05 ;
Time (1,1,8) =  0.000169988 ;
Time (1,1,16) =  0.000339141 ;
Time (1,1,32) =  0.000679577 ;
Time (1,1,64) =   0.00245396 ;
Time (1,1,128) =    0.0301697 ;
Time (1,1,160) =    0.0278484 ;
Time (2,1,1) =   0.00046097 ;
Time (2,1,2) =  0.000460817 ;
Time (2,1,4) =   0.00900235 ;
Time (2,1,8) =  0.000480701 ;
Time (2,1,16) =  0.000487583 ;
Time (2,1,32) =  0.000479143 ;
Time (2,1,64) =   0.00103452 ;
Time (2,1,128) =   0.00043515 ;
Time (2,1,160) =   0.00111335 ;
Time (3,1,1) =  0.000566476 ;
Time (3,1,2) =  0.000471961 ;
Time (3,1,4) =   0.00037593 ;
Time (3,1,8) =  0.000362135 ;
Time (3,1,16) =  0.000528744 ;
Time (3,1,32) =   0.00122201 ;
Time (3,1,64) =  0.000404955 ;
Time (3,1,128) =  0.000987905 ;
Time (3,1,160) =   0.00348279 ;
Time (4,1,1) =  0.000268353 ;
Time (4,1,2) =  0.000280197 ;
Time (4,1,4) =  0.000240547 ;
Time (4,1,8) =  0.000216099 ;
Time (4,1,16) =  0.000527469 ;
Time (4,1,32) =   0.00225607 ;
Time (4,1,64) =  0.000313899 ;
Time (4,1,128) =  0.000211772 ;
Time (4,1,160) =  0.000199709 ;
Time (5,1,1) =  0.000217692 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 3448 ;
Ntri (id) = 7 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.01477e-05 ;
T_prep (1,1,2) =  1.01635e-05 ;
T_prep (1,1,4) =  1.01188e-05 ;
T_prep (1,1,8) =  1.01915e-05 ;
T_prep (1,1,16) =  1.00778e-05 ;
T_prep (1,1,32) =  1.01672e-05 ;
T_prep (1,1,64) =  1.01496e-05 ;
T_prep (1,1,128) =  1.01924e-05 ;
T_prep (1,1,160) =  1.01794e-05 ;
T_prep (2,1,1) =  1.06823e-05 ;
T_prep (2,1,2) =  1.01412e-05 ;
T_prep (2,1,4) =  1.01216e-05 ;
T_prep (2,1,8) =  1.01235e-05 ;
T_prep (2,1,16) =  1.01253e-05 ;
T_prep (2,1,32) =  1.01719e-05 ;
T_prep (2,1,64) =  1.01579e-05 ;
T_prep (2,1,128) =  1.01086e-05 ;
T_prep (2,1,160) =  1.00099e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.58772e-05 ;
Time (1,1,2) =   0.00890139 ;
Time (1,1,4) =  9.05069e-05 ;
Time (1,1,8) =  0.000170427 ;
Time (1,1,16) =  0.000376918 ;
Time (1,1,32) =  0.000800644 ;
Time (1,1,64) =   0.00217418 ;
Time (1,1,128) =    0.0304653 ;
Time (1,1,160) =    0.0341788 ;
Time (2,1,1) =  0.000190238 ;
Time (2,1,2) =   0.00025345 ;
Time (2,1,4) =  0.000241758 ;
Time (2,1,8) =  0.000224338 ;
Time (2,1,16) =  0.000212505 ;
Time (2,1,32) =  0.000657365 ;
Time (2,1,64) =  0.000764872 ;
Time (2,1,128) =    0.0128151 ;
Time (2,1,160) =   0.00413935 ;
Time (3,1,1) =  0.000191906 ;
Time (3,1,2) =  0.000209267 ;
Time (3,1,4) =  0.000198958 ;
Time (3,1,8) =  0.000163867 ;
Time (3,1,16) =  0.000681164 ;
Time (3,1,32) =  0.000742474 ;
Time (3,1,64) =  0.000180896 ;
Time (3,1,128) =    0.0229428 ;
Time (3,1,160) =  0.000206634 ;
Time (4,1,1) =  0.000167249 ;
Time (4,1,2) =   0.00013571 ;
Time (4,1,4) =  0.000195185 ;
Time (4,1,8) =  0.000150707 ;
Time (4,1,16) =  0.000301365 ;
Time (4,1,32) =  0.000260248 ;
Time (4,1,64) =  0.000280678 ;
Time (4,1,128) =   0.00050371 ;
Time (4,1,160) =  0.000252618 ;
Time (5,1,1) =  0.000197947 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1020 ;
Nedges (id) = 2880 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  8.54861e-06 ;
T_prep (1,1,2) =  8.53557e-06 ;
T_prep (1,1,4) =  8.55606e-06 ;
T_prep (1,1,8) =  8.48249e-06 ;
T_prep (1,1,16) =  8.50391e-06 ;
T_prep (1,1,32) =  8.56724e-06 ;
T_prep (1,1,64) =  8.53091e-06 ;
T_prep (1,1,128) =  8.51508e-06 ;
T_prep (1,1,160) =  8.55234e-06 ;
T_prep (2,1,1) =  8.55979e-06 ;
T_prep (2,1,2) =  8.18819e-06 ;
T_prep (2,1,4) =  8.24127e-06 ;
T_prep (2,1,8) =  8.24314e-06 ;
T_prep (2,1,16) =    8.123e-06 ;
T_prep (2,1,32) =  8.06712e-06 ;
T_prep (2,1,64) =  8.19098e-06 ;
T_prep (2,1,128) =  8.31299e-06 ;
T_prep (2,1,160) =  8.18539e-06 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.21947e-05 ;
Time (1,1,2) =   0.00882731 ;
Time (1,1,4) =  9.47732e-05 ;
Time (1,1,8) =  0.000139319 ;
Time (1,1,16) =    0.0003199 ;
Time (1,1,32) =  0.000849827 ;
Time (1,1,64) =   0.00198969 ;
Time (1,1,128) =    0.0278256 ;
Time (1,1,160) =    0.0225131 ;
Time (2,1,1) =  0.000117335 ;
Time (2,1,2) =  0.000183703 ;
Time (2,1,4) =  0.000162121 ;
Time (2,1,8) =  0.000154201 ;
Time (2,1,16) =  0.000197671 ;
Time (2,1,32) =  0.000122903 ;
Time (2,1,64) =  0.000962141 ;
Time (2,1,128) =    0.0174584 ;
Time (2,1,160) =  0.000200019 ;
Time (3,1,1) =  9.35392e-05 ;
Time (3,1,2) =  0.000153232 ;
Time (3,1,4) =  9.58405e-05 ;
Time (3,1,8) =  0.000112313 ;
Time (3,1,16) =  0.000178162 ;
Time (3,1,32) =  9.51309e-05 ;
Time (3,1,64) =  8.99276e-05 ;
Time (3,1,128) =   0.00133756 ;
Time (3,1,160) =   0.00808701 ;
Time (4,1,1) =  0.000141639 ;
Time (4,1,2) =  0.000150754 ;
Time (4,1,4) =  0.000138612 ;
Time (4,1,8) =  0.000161783 ;
Time (4,1,16) =  0.000336933 ;
Time (4,1,32) =  0.000605702 ;
Time (4,1,64) =  0.000833938 ;
Time (4,1,128) =   0.00174392 ;
Time (4,1,160) =   0.00948107 ;
Time (5,1,1) =  0.000123288 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4156 ;
Ntri (id) = 2025 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.25673e-05 ;
T_prep (1,1,2) =  1.25375e-05 ;
T_prep (1,1,4) =  1.25123e-05 ;
T_prep (1,1,8) =    1.253e-05 ;
T_prep (1,1,16) =  1.25468e-05 ;
T_prep (1,1,32) =  1.25365e-05 ;
T_prep (1,1,64) =  1.25235e-05 ;
T_prep (1,1,128) =   1.2517e-05 ;
T_prep (1,1,160) =  1.24983e-05 ;
T_prep (2,1,1) =   1.2381e-05 ;
T_prep (2,1,2) =  1.23652e-05 ;
T_prep (2,1,4) =  1.22869e-05 ;
T_prep (2,1,8) =  1.23009e-05 ;
T_prep (2,1,16) =  1.23139e-05 ;
T_prep (2,1,32) =  1.23186e-05 ;
T_prep (2,1,64) =  1.23037e-05 ;
T_prep (2,1,128) =  1.23307e-05 ;
T_prep (2,1,160) =  1.22804e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  2.14335e-05 ;
Time (1,1,2) =   0.00885238 ;
Time (1,1,4) =  9.81595e-05 ;
Time (1,1,8) =  0.000161185 ;
Time (1,1,16) =  0.000357077 ;
Time (1,1,32) =  0.000879036 ;
Time (1,1,64) =   0.00189564 ;
Time (1,1,128) =    0.0282249 ;
Time (1,1,160) =    0.0543378 ;
Time (2,1,1) =  0.000260004 ;
Time (2,1,2) =   0.00108486 ;
Time (2,1,4) =   0.00110445 ;
Time (2,1,8) =  0.000347639 ;
Time (2,1,16) =  0.000430791 ;
Time (2,1,32) =  0.000731157 ;
Time (2,1,64) =  0.000691235 ;
Time (2,1,128) =  0.000874055 ;
Time (2,1,160) =  0.000355566 ;
Time (3,1,1) =  0.000231235 ;
Time (3,1,2) =  0.000147578 ;
Time (3,1,4) =   0.00114409 ;
Time (3,1,8) =  0.000205944 ;
Time (3,1,16) =  0.000428635 ;
Time (3,1,32) =  0.000356585 ;
Time (3,1,64) =  0.000313689 ;
Time (3,1,128) =   0.00024596 ;
Time (3,1,160) =   0.00105861 ;
Time (4,1,1) =  0.000220502 ;
Time (4,1,2) =  0.000179537 ;
Time (4,1,4) =   0.00134894 ;
Time (4,1,8) =  0.000578593 ;
Time (4,1,16) =  0.000655017 ;
Time (4,1,32) =  0.000268678 ;
Time (4,1,64) =  0.000513842 ;
Time (4,1,128) =  0.000339078 ;
Time (4,1,160) =    0.0003523 ;
Time (5,1,1) =  0.000214383 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4156 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.25393e-05 ;
T_prep (1,1,2) =  1.25179e-05 ;
T_prep (1,1,4) =  1.25142e-05 ;
T_prep (1,1,8) =  1.25067e-05 ;
T_prep (1,1,16) =  1.24788e-05 ;
T_prep (1,1,32) =  1.25011e-05 ;
T_prep (1,1,64) =   1.2476e-05 ;
T_prep (1,1,128) =  1.24937e-05 ;
T_prep (1,1,160) =  1.24415e-05 ;
T_prep (2,1,1) =  1.26846e-05 ;
T_prep (2,1,2) =  1.24173e-05 ;
T_prep (2,1,4) =  1.24164e-05 ;
T_prep (2,1,8) =  1.24341e-05 ;
T_prep (2,1,16) =  1.24117e-05 ;
T_prep (2,1,32) =  1.24276e-05 ;
T_prep (2,1,64) =  1.23912e-05 ;
T_prep (2,1,128) =  1.24099e-05 ;
T_prep (2,1,160) =  1.23866e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.96621e-05 ;
Time (1,1,2) =   0.00883774 ;
Time (1,1,4) =  9.79062e-05 ;
Time (1,1,8) =  0.000175104 ;
Time (1,1,16) =  0.000301707 ;
Time (1,1,32) =  0.000624562 ;
Time (1,1,64) =   0.00182879 ;
Time (1,1,128) =    0.0250588 ;
Time (1,1,160) =    0.0212066 ;
Time (2,1,1) =  0.000200211 ;
Time (2,1,2) =  0.000194655 ;
Time (2,1,4) =  0.000193268 ;
Time (2,1,8) =   0.00015474 ;
Time (2,1,16) =  0.000177242 ;
Time (2,1,32) =  0.000501187 ;
Time (2,1,64) =  0.000807267 ;
Time (2,1,128) =     0.010451 ;
Time (2,1,160) =   0.00326961 ;
Time (3,1,1) =  0.000199373 ;
Time (3,1,2) =  0.000122662 ;
Time (3,1,4) =  0.000136591 ;
Time (3,1,8) =  0.000108542 ;
Time (3,1,16) =  0.000100684 ;
Time (3,1,32) =  9.40682e-05 ;
Time (3,1,64) =  0.000517872 ;
Time (3,1,128) =   0.00419948 ;
Time (3,1,160) =   0.00529012 ;
Time (4,1,1) =  0.000189844 ;
Time (4,1,2) =  0.000150152 ;
Time (4,1,4) =  0.000167632 ;
Time (4,1,8) =  0.000121592 ;
Time (4,1,16) =   0.00017483 ;
Time (4,1,32) =  0.000185795 ;
Time (4,1,64) =  0.000259428 ;
Time (4,1,128) =  0.000288061 ;
Time (4,1,160) =  0.000234613 ;
Time (5,1,1) =  0.000168709 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2132 ;
Nedges (id) = 4050 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.23903e-05 ;
T_prep (1,1,2) =  1.23121e-05 ;
T_prep (1,1,4) =    1.234e-05 ;
T_prep (1,1,8) =  1.22925e-05 ;
T_prep (1,1,16) =  1.22618e-05 ;
T_prep (1,1,32) =  1.23093e-05 ;
T_prep (1,1,64) =  1.22292e-05 ;
T_prep (1,1,128) =  1.22907e-05 ;
T_prep (1,1,160) =  1.23093e-05 ;
T_prep (2,1,1) =  1.21538e-05 ;
T_prep (2,1,2) =   1.2123e-05 ;
T_prep (2,1,4) =  1.21091e-05 ;
T_prep (2,1,8) =  1.20727e-05 ;
T_prep (2,1,16) =  1.20997e-05 ;
T_prep (2,1,32) =  1.20429e-05 ;
T_prep (2,1,64) =  1.20765e-05 ;
T_prep (2,1,128) =  1.20625e-05 ;
T_prep (2,1,160) =  1.21025e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.90577e-05 ;
Time (1,1,2) =   0.00887143 ;
Time (1,1,4) =  9.60026e-05 ;
Time (1,1,8) =  0.000143865 ;
Time (1,1,16) =  0.000338045 ;
Time (1,1,32) =  0.000736019 ;
Time (1,1,64) =   0.00194183 ;
Time (1,1,128) =    0.0350574 ;
Time (1,1,160) =    0.0269432 ;
Time (2,1,1) =  0.000165984 ;
Time (2,1,2) =  0.000187878 ;
Time (2,1,4) =   0.00020816 ;
Time (2,1,8) =  0.000274656 ;
Time (2,1,16) =  0.000299536 ;
Time (2,1,32) =   0.00044955 ;
Time (2,1,64) =  0.000945348 ;
Time (2,1,128) =     0.019063 ;
Time (2,1,160) =  0.000625621 ;
Time (3,1,1) =   0.00015218 ;
Time (3,1,2) =  0.000107884 ;
Time (3,1,4) =  8.18064e-05 ;
Time (3,1,8) =  0.000197968 ;
Time (3,1,16) =  8.23708e-05 ;
Time (3,1,32) =  0.000102385 ;
Time (3,1,64) =  0.000136739 ;
Time (3,1,128) =   0.00521507 ;
Time (3,1,160) =   0.00234984 ;
Time (4,1,1) =  0.000195853 ;
Time (4,1,2) =  0.000141617 ;
Time (4,1,4) =  0.000156832 ;
Time (4,1,8) =  0.000169231 ;
Time (4,1,16) =  0.000187716 ;
Time (4,1,32) =  0.000255859 ;
Time (4,1,64) =   0.00017738 ;
Time (4,1,128) =   0.00622632 ;
Time (4,1,160) =  0.000482105 ;
Time (5,1,1) =  0.000133419 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 4320 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.27014e-05 ;
T_prep (1,1,2) =   1.2828e-05 ;
T_prep (1,1,4) =  1.27889e-05 ;
T_prep (1,1,8) =  1.26641e-05 ;
T_prep (1,1,16) =  1.26781e-05 ;
T_prep (1,1,32) =  1.26883e-05 ;
T_prep (1,1,64) =  1.26734e-05 ;
T_prep (1,1,128) =  1.26995e-05 ;
T_prep (1,1,160) =  1.27824e-05 ;
T_prep (2,1,1) =   1.2937e-05 ;
T_prep (2,1,2) =  1.25347e-05 ;
T_prep (2,1,4) =  1.25663e-05 ;
T_prep (2,1,8) =  1.25449e-05 ;
T_prep (2,1,16) =  1.25617e-05 ;
T_prep (2,1,32) =  1.25309e-05 ;
T_prep (2,1,64) =  1.25645e-05 ;
T_prep (2,1,128) =  1.25244e-05 ;
T_prep (2,1,160) =  1.25719e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  1.74288e-05 ;
Time (1,1,2) =   0.00927779 ;
Time (1,1,4) =  9.31639e-05 ;
Time (1,1,8) =  0.000168636 ;
Time (1,1,16) =   0.00035482 ;
Time (1,1,32) =  0.000751152 ;
Time (1,1,64) =    0.0018321 ;
Time (1,1,128) =    0.0242626 ;
Time (1,1,160) =    0.0287193 ;
Time (2,1,1) =  0.000151833 ;
Time (2,1,2) =  0.000205563 ;
Time (2,1,4) =  0.000235208 ;
Time (2,1,8) =  0.000228689 ;
Time (2,1,16) =  0.000193787 ;
Time (2,1,32) =  0.000553576 ;
Time (2,1,64) =   0.00112943 ;
Time (2,1,128) =   0.00554512 ;
Time (2,1,160) =   0.00761129 ;
Time (3,1,1) =  0.000136171 ;
Time (3,1,2) =   0.00012074 ;
Time (3,1,4) =  0.000118528 ;
Time (3,1,8) =  0.000150041 ;
Time (3,1,16) =  0.000283066 ;
Time (3,1,32) =  9.35951e-05 ;
Time (3,1,64) =   0.00121248 ;
Time (3,1,128) =   0.00555222 ;
Time (3,1,160) =  0.000340443 ;
Time (4,1,1) =  0.000177034 ;
Time (4,1,2) =  0.000169249 ;
Time (4,1,4) =  0.000175759 ;
Time (4,1,8) =   0.00018041 ;
Time (4,1,16) =   0.00138692 ;
Time (4,1,32) =   0.00077572 ;
Time (4,1,64) =  0.000807822 ;
Time (4,1,128) =   0.00423251 ;
Time (4,1,160) =  0.000161944 ;
Time (5,1,1) =  0.000158962 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 6583 ;
Ntri (id) = 9107 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  1.84346e-05 ;
T_prep (1,1,2) =  1.82884e-05 ;
T_prep (1,1,4) =  1.84719e-05 ;
T_prep (1,1,8) =   1.8416e-05 ;
T_prep (1,1,16) =  1.84355e-05 ;
T_prep (1,1,32) =  1.84523e-05 ;
T_prep (1,1,64) =  1.84057e-05 ;
T_prep (1,1,128) =  1.85091e-05 ;
T_prep (1,1,160) =  1.84346e-05 ;
T_prep (2,1,1) =  2.10255e-05 ;
T_prep (2,1,2) =  2.06269e-05 ;
T_prep (2,1,4) =  2.06269e-05 ;
T_prep (2,1,8) =  2.05878e-05 ;
T_prep (2,1,16) =  2.06092e-05 ;
T_prep (2,1,32) =  2.06204e-05 ;
T_prep (2,1,64) =  2.06036e-05 ;
T_prep (2,1,128) =  2.06893e-05 ;
T_prep (2,1,160) =  2.06446e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  5.22844e-05 ;
Time (1,1,2) =   0.00895559 ;
Time (1,1,4) =  0.000109707 ;
Time (1,1,8) =  0.000184092 ;
Time (1,1,16) =  0.000329913 ;
Time (1,1,32) =  0.000810759 ;
Time (1,1,64) =   0.00223293 ;
Time (1,1,128) =    0.0203558 ;
Time (1,1,160) =    0.0377737 ;
Time (2,1,1) =   0.00127156 ;
Time (2,1,2) =   0.00118842 ;
Time (2,1,4) =    0.0013628 ;
Time (2,1,8) =   0.00146648 ;
Time (2,1,16) =   0.00155555 ;
Time (2,1,32) =   0.00142583 ;
Time (2,1,64) =   0.00209028 ;
Time (2,1,128) =   0.00960015 ;
Time (2,1,160) =   0.00121398 ;
Time (3,1,1) =   0.00136834 ;
Time (3,1,2) =   0.00119674 ;
Time (3,1,4) =   0.00130552 ;
Time (3,1,8) =   0.00131663 ;
Time (3,1,16) =   0.00137521 ;
Time (3,1,32) =   0.00128893 ;
Time (3,1,64) =   0.00130912 ;
Time (3,1,128) =   0.00126958 ;
Time (3,1,160) =   0.00121419 ;
Time (4,1,1) =  0.000720655 ;
Time (4,1,2) =  0.000623537 ;
Time (4,1,4) =  0.000576138 ;
Time (4,1,8) =  0.000598155 ;
Time (4,1,16) =  0.000623293 ;
Time (4,1,32) =   0.00085255 ;
Time (4,1,64) =  0.000918889 ;
Time (4,1,128) =    0.0026781 ;
Time (4,1,160) =   0.00065244 ;
Time (5,1,1) =  0.000664484 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1200 ;
Nedges (id) = 6583 ;
Ntri (id) = 35 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   1.9352e-05 ;
T_prep (1,1,2) =  1.93063e-05 ;
T_prep (1,1,4) =  1.93305e-05 ;
T_prep (1,1,8) =   1.9324e-05 ;
T_prep (1,1,16) =  1.91089e-05 ;
T_prep (1,1,32) =  1.93221e-05 ;
T_prep (1,1,64) =  1.91526e-05 ;
T_prep (1,1,128) =  1.92896e-05 ;
T_prep (1,1,160) =    1.927e-05 ;
T_prep (2,1,1) =  2.02535e-05 ;
T_prep (2,1,2) =  1.95876e-05 ;
T_prep (2,1,4) =  1.95326e-05 ;
T_prep (2,1,8) =  1.95084e-05 ;
T_prep (2,1,16) =  1.95783e-05 ;
T_prep (2,1,32) =  1.95736e-05 ;
T_prep (2,1,64) =  1.95839e-05 ;
T_prep (2,1,128) =  1.95866e-05 ;
T_prep (2,1,160) =  1.95187e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   3.2533e-05 ;
Time (1,1,2) =   0.00892622 ;
Time (1,1,4) =  9.44026e-05 ;
Time (1,1,8) =  0.000141216 ;
Time (1,1,16) =  0.000351818 ;
Time (1,1,32) =  0.000636353 ;
Time (1,1,64) =   0.00218827 ;
Time (1,1,128) =    0.0176146 ;
Time (1,1,160) =    0.0271436 ;
Time (2,1,1) =  0.000492968 ;
Time (2,1,2) =  0.000447787 ;
Time (2,1,4) =  0.000398402 ;
Time (2,1,8) =  0.000843447 ;
Time (2,1,16) =  0.000746238 ;
Time (2,1,32) =  0.000719503 ;
Time (2,1,64) =   0.00119608 ;
Time (2,1,128) =    0.0195841 ;
Time (2,1,160) =    0.0013553 ;
Time (3,1,1) =  0.000503527 ;
Time (3,1,2) =  0.000367919 ;
Time (3,1,4) =  0.000304293 ;
Time (3,1,8) =  0.000338057 ;
Time (3,1,16) =  0.000260955 ;
Time (3,1,32) =  0.000509351 ;
Time (3,1,64) =  0.000921692 ;
Time (3,1,128) =    0.0043218 ;
Time (3,1,160) =  0.000433925 ;
Time (4,1,1) =  0.000355756 ;
Time (4,1,2) =  0.000300662 ;
Time (4,1,4) =  0.000287084 ;
Time (4,1,8) =  0.000286351 ;
Time (4,1,16) =  0.000316289 ;
Time (4,1,32) =  0.000648968 ;
Time (4,1,64) =   0.00037782 ;
Time (4,1,128) =  0.000909568 ;
Time (4,1,160) =   0.00383883 ;
Time (5,1,1) =  0.000476193 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 14400 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  4.02313e-05 ;
T_prep (1,1,2) =  4.04706e-05 ;
T_prep (1,1,4) =   4.0316e-05 ;
T_prep (1,1,8) =  4.04818e-05 ;
T_prep (1,1,16) =  4.03831e-05 ;
T_prep (1,1,32) =  4.04837e-05 ;
T_prep (1,1,64) =  4.04175e-05 ;
T_prep (1,1,128) =  4.04203e-05 ;
T_prep (1,1,160) =   4.0439e-05 ;
T_prep (2,1,1) =  4.19077e-05 ;
T_prep (2,1,2) =  4.18024e-05 ;
T_prep (2,1,4) =  4.18574e-05 ;
T_prep (2,1,8) =  4.17121e-05 ;
T_prep (2,1,16) =   4.1862e-05 ;
T_prep (2,1,32) =  4.18285e-05 ;
T_prep (2,1,64) =  4.16758e-05 ;
T_prep (2,1,128) =   4.1822e-05 ;
T_prep (2,1,160) =  4.16916e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  5.84824e-05 ;
Time (1,1,2) =   0.00897229 ;
Time (1,1,4) =  0.000101048 ;
Time (1,1,8) =  0.000200555 ;
Time (1,1,16) =   0.00029963 ;
Time (1,1,32) =  0.000675496 ;
Time (1,1,64) =   0.00230603 ;
Time (1,1,128) =    0.0292094 ;
Time (1,1,160) =    0.0292192 ;
Time (2,1,1) =  0.000637513 ;
Time (2,1,2) =  0.000471476 ;
Time (2,1,4) =  0.000380999 ;
Time (2,1,8) =  0.000380709 ;
Time (2,1,16) =  0.000473374 ;
Time (2,1,32) =  0.000946215 ;
Time (2,1,64) =   0.00123158 ;
Time (2,1,128) =    0.0132952 ;
Time (2,1,160) =  0.000479046 ;
Time (3,1,1) =  0.000472498 ;
Time (3,1,2) =    0.0010585 ;
Time (3,1,4) =  0.000134366 ;
Time (3,1,8) =  0.000240197 ;
Time (3,1,16) =  0.000145962 ;
Time (3,1,32) =  0.000232203 ;
Time (3,1,64) =   0.00333152 ;
Time (3,1,128) =   0.00774221 ;
Time (3,1,160) =  0.000148668 ;
Time (4,1,1) =  0.000701093 ;
Time (4,1,2) =  0.000437095 ;
Time (4,1,4) =  0.000445364 ;
Time (4,1,8) =   0.00117558 ;
Time (4,1,16) =  0.000538937 ;
Time (4,1,32) =  0.000535036 ;
Time (4,1,64) =  0.000508296 ;
Time (4,1,128) =  0.000540966 ;
Time (4,1,160) =  0.000837048 ;
Time (5,1,1) =  0.000597766 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/as20000102/as20000102_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 6474 ;
Nedges (id) = 12572 ;
Ntri (id) = 6584 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  8.34148e-05 ;
T_prep (1,1,2) =  8.20244e-05 ;
T_prep (1,1,4) =  8.16351e-05 ;
T_prep (1,1,8) =     8.15e-05 ;
T_prep (1,1,16) =  8.13063e-05 ;
T_prep (1,1,32) =  8.12383e-05 ;
T_prep (1,1,64) =  8.12011e-05 ;
T_prep (1,1,128) =  8.09822e-05 ;
T_prep (1,1,160) =  8.12346e-05 ;
T_prep (2,1,1) =  9.00412e-05 ;
T_prep (2,1,2) =  8.48025e-05 ;
T_prep (2,1,4) =  8.25953e-05 ;
T_prep (2,1,8) =  8.17673e-05 ;
T_prep (2,1,16) =  8.14879e-05 ;
T_prep (2,1,32) =  8.11238e-05 ;
T_prep (2,1,64) =  8.09906e-05 ;
T_prep (2,1,128) =  8.10046e-05 ;
T_prep (2,1,160) =  8.11461e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000204628 ;
Time (1,1,2) =   0.00901549 ;
Time (1,1,4) =  0.000136269 ;
Time (1,1,8) =   0.00018565 ;
Time (1,1,16) =  0.000399916 ;
Time (1,1,32) =  0.000826413 ;
Time (1,1,64) =     0.002109 ;
Time (1,1,128) =    0.0301865 ;
Time (1,1,160) =    0.0357409 ;
Time (2,1,1) =   0.00239175 ;
Time (2,1,2) =   0.00113433 ;
Time (2,1,4) =   0.00202644 ;
Time (2,1,8) =   0.00135404 ;
Time (2,1,16) =   0.00107533 ;
Time (2,1,32) =   0.00104115 ;
Time (2,1,64) =  0.000730689 ;
Time (2,1,128) =   0.00127367 ;
Time (2,1,160) =     0.010771 ;
Time (3,1,1) =   0.00388593 ;
Time (3,1,2) =   0.00216411 ;
Time (3,1,4) =   0.00221282 ;
Time (3,1,8) =  0.000971119 ;
Time (3,1,16) =   0.00199459 ;
Time (3,1,32) =   0.00204065 ;
Time (3,1,64) =   0.00175465 ;
Time (3,1,128) =   0.00162375 ;
Time (3,1,160) =   0.00164893 ;
Time (4,1,1) =   0.00171771 ;
Time (4,1,2) =  0.000532333 ;
Time (4,1,4) =  0.000353061 ;
Time (4,1,8) =  0.000692648 ;
Time (4,1,16) =  0.000653146 ;
Time (4,1,32) =  0.000621644 ;
Time (4,1,64) =  0.000595911 ;
Time (4,1,128) =  0.000935772 ;
Time (4,1,160) =  0.000662042 ;
Time (5,1,1) =   0.00162213 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 15988 ;
Ntri (id) = 15169 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  4.39119e-05 ;
T_prep (1,1,2) =  4.42602e-05 ;
T_prep (1,1,4) =  4.42322e-05 ;
T_prep (1,1,8) =  4.43375e-05 ;
T_prep (1,1,16) =  4.40544e-05 ;
T_prep (1,1,32) =   4.4113e-05 ;
T_prep (1,1,64) =  6.50706e-05 ;
T_prep (1,1,128) =  5.08279e-05 ;
T_prep (1,1,160) =  4.42406e-05 ;
T_prep (2,1,1) =  4.45237e-05 ;
T_prep (2,1,2) =   4.4276e-05 ;
T_prep (2,1,4) =  4.42043e-05 ;
T_prep (2,1,8) =  4.41354e-05 ;
T_prep (2,1,16) =  4.42704e-05 ;
T_prep (2,1,32) =  4.41791e-05 ;
T_prep (2,1,64) =  4.42304e-05 ;
T_prep (2,1,128) =  4.42183e-05 ;
T_prep (2,1,160) =   4.4262e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000111014 ;
Time (1,1,2) =   0.00899226 ;
Time (1,1,4) =  0.000171643 ;
Time (1,1,8) =   0.00026133 ;
Time (1,1,16) =   0.00036474 ;
Time (1,1,32) =  0.000898617 ;
Time (1,1,64) =   0.00237761 ;
Time (1,1,128) =    0.0270571 ;
Time (1,1,160) =    0.0354268 ;
Time (2,1,1) =   0.00231071 ;
Time (2,1,2) =   0.00249865 ;
Time (2,1,4) =   0.00248236 ;
Time (2,1,8) =   0.00244757 ;
Time (2,1,16) =   0.00264977 ;
Time (2,1,32) =   0.00243031 ;
Time (2,1,64) =   0.00217524 ;
Time (2,1,128) =  0.000577217 ;
Time (2,1,160) =  0.000697822 ;
Time (3,1,1) =   0.00214285 ;
Time (3,1,2) =   0.00115899 ;
Time (3,1,4) =   0.00075544 ;
Time (3,1,8) =  0.000743505 ;
Time (3,1,16) =  0.000765353 ;
Time (3,1,32) =  0.000639769 ;
Time (3,1,64) =   0.00135842 ;
Time (3,1,128) =  0.000852542 ;
Time (3,1,160) =   0.00100323 ;
Time (4,1,1) =   0.00132131 ;
Time (4,1,2) =   0.00114304 ;
Time (4,1,4) =    0.0012355 ;
Time (4,1,8) =     0.001173 ;
Time (4,1,16) =   0.00112724 ;
Time (4,1,32) =   0.00110043 ;
Time (4,1,64) =  0.000316441 ;
Time (4,1,128) =   0.00116011 ;
Time (4,1,160) =   0.00115181 ;
Time (5,1,1) =   0.00113592 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4420 ;
Nedges (id) = 15988 ;
Ntri (id) = 7 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  4.42332e-05 ;
T_prep (1,1,2) =  4.42592e-05 ;
T_prep (1,1,4) =  4.44418e-05 ;
T_prep (1,1,8) =  4.44483e-05 ;
T_prep (1,1,16) =  4.44083e-05 ;
T_prep (1,1,32) =  4.44381e-05 ;
T_prep (1,1,64) =  4.44176e-05 ;
T_prep (1,1,128) =    4.448e-05 ;
T_prep (1,1,160) =  4.43757e-05 ;
T_prep (2,1,1) =  4.68399e-05 ;
T_prep (2,1,2) =  4.66555e-05 ;
T_prep (2,1,4) =  4.66518e-05 ;
T_prep (2,1,8) =  4.65242e-05 ;
T_prep (2,1,16) =  4.64916e-05 ;
T_prep (2,1,32) =  4.66425e-05 ;
T_prep (2,1,64) =  4.66416e-05 ;
T_prep (2,1,128) =  4.66779e-05 ;
T_prep (2,1,160) =  4.65941e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  6.84997e-05 ;
Time (1,1,2) =   0.00903715 ;
Time (1,1,4) =   0.00011001 ;
Time (1,1,8) =  0.000241013 ;
Time (1,1,16) =   0.00035274 ;
Time (1,1,32) =  0.000669382 ;
Time (1,1,64) =   0.00221078 ;
Time (1,1,128) =    0.0210807 ;
Time (1,1,160) =    0.0443107 ;
Time (2,1,1) =  0.000772567 ;
Time (2,1,2) =  0.000545347 ;
Time (2,1,4) =  0.000607597 ;
Time (2,1,8) =  0.000568093 ;
Time (2,1,16) =   0.00085071 ;
Time (2,1,32) =  0.000732706 ;
Time (2,1,64) =  0.000703016 ;
Time (2,1,128) =   0.00956437 ;
Time (2,1,160) =   0.00182651 ;
Time (3,1,1) =  0.000803851 ;
Time (3,1,2) =   0.00040607 ;
Time (3,1,4) =  0.000331073 ;
Time (3,1,8) =  0.000366824 ;
Time (3,1,16) =   0.00090809 ;
Time (3,1,32) =   0.00104284 ;
Time (3,1,64) =  0.000281181 ;
Time (3,1,128) =   0.00694368 ;
Time (3,1,160) =  0.000338036 ;
Time (4,1,1) =  0.000749375 ;
Time (4,1,2) =  0.000482688 ;
Time (4,1,4) =  0.000537022 ;
Time (4,1,8) =  0.000552285 ;
Time (4,1,16) =  0.000629136 ;
Time (4,1,32) =  0.000567863 ;
Time (4,1,64) =  0.000629016 ;
Time (4,1,128) =  0.000599721 ;
Time (4,1,160) =   0.00157006 ;
Time (5,1,1) =   0.00117141 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-GrQc/ca-GrQc_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5242 ;
Nedges (id) = 14484 ;
Ntri (id) = 48260 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  9.44287e-05 ;
T_prep (1,1,2) =  9.29506e-05 ;
T_prep (1,1,4) =  9.27886e-05 ;
T_prep (1,1,8) =  9.25492e-05 ;
T_prep (1,1,16) =  9.25725e-05 ;
T_prep (1,1,32) =  9.25716e-05 ;
T_prep (1,1,64) =  9.23267e-05 ;
T_prep (1,1,128) =  9.22978e-05 ;
T_prep (1,1,160) =  9.23974e-05 ;
T_prep (2,1,1) =  0.000102677 ;
T_prep (2,1,2) =  9.87342e-05 ;
T_prep (2,1,4) =  9.63584e-05 ;
T_prep (2,1,8) =  9.54205e-05 ;
T_prep (2,1,16) =    9.502e-05 ;
T_prep (2,1,32) =  9.48859e-05 ;
T_prep (2,1,64) =  9.49781e-05 ;
T_prep (2,1,128) =  9.46559e-05 ;
T_prep (2,1,160) =  9.47928e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000254254 ;
Time (1,1,2) =   0.00907575 ;
Time (1,1,4) =  0.000149458 ;
Time (1,1,8) =  0.000237901 ;
Time (1,1,16) =  0.000429933 ;
Time (1,1,32) =  0.000770066 ;
Time (1,1,64) =   0.00211838 ;
Time (1,1,128) =    0.0314031 ;
Time (1,1,160) =    0.0234534 ;
Time (2,1,1) =   0.00262797 ;
Time (2,1,2) =   0.00158811 ;
Time (2,1,4) =   0.00160333 ;
Time (2,1,8) =   0.00122997 ;
Time (2,1,16) =    0.0011867 ;
Time (2,1,32) =   0.00144133 ;
Time (2,1,64) =   0.00118618 ;
Time (2,1,128) =   0.00792647 ;
Time (2,1,160) =   0.00144339 ;
Time (3,1,1) =   0.00432855 ;
Time (3,1,2) =   0.00299428 ;
Time (3,1,4) =   0.00212204 ;
Time (3,1,8) =   0.00190784 ;
Time (3,1,16) =     0.001838 ;
Time (3,1,32) =   0.00191534 ;
Time (3,1,64) =   0.00177141 ;
Time (3,1,128) =   0.00168437 ;
Time (3,1,160) =   0.00199496 ;
Time (4,1,1) =   0.00196377 ;
Time (4,1,2) =  0.000949378 ;
Time (4,1,4) =  0.000910169 ;
Time (4,1,8) =  0.000690974 ;
Time (4,1,16) =  0.000845062 ;
Time (4,1,32) =   0.00100515 ;
Time (4,1,64) =  0.000653656 ;
Time (4,1,128) =  0.000717445 ;
Time (4,1,160) =  0.000588327 ;
Time (5,1,1) =    0.0016049 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 23040 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  6.26324e-05 ;
T_prep (1,1,2) =  6.25364e-05 ;
T_prep (1,1,4) =  6.22524e-05 ;
T_prep (1,1,8) =  6.21211e-05 ;
T_prep (1,1,16) =  6.22207e-05 ;
T_prep (1,1,32) =  6.21164e-05 ;
T_prep (1,1,64) =  6.26417e-05 ;
T_prep (1,1,128) =   6.2041e-05 ;
T_prep (1,1,160) =   6.2569e-05 ;
T_prep (2,1,1) =  6.20019e-05 ;
T_prep (2,1,2) =   6.1729e-05 ;
T_prep (2,1,4) =  6.13267e-05 ;
T_prep (2,1,8) =  6.16582e-05 ;
T_prep (2,1,16) =  6.17579e-05 ;
T_prep (2,1,32) =  6.16554e-05 ;
T_prep (2,1,64) =  6.17327e-05 ;
T_prep (2,1,128) =  6.16945e-05 ;
T_prep (2,1,160) =  6.16666e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  8.55625e-05 ;
Time (1,1,2) =    0.0089901 ;
Time (1,1,4) =  0.000132816 ;
Time (1,1,8) =  0.000243964 ;
Time (1,1,16) =  0.000310737 ;
Time (1,1,32) =  0.000761197 ;
Time (1,1,64) =   0.00237603 ;
Time (1,1,128) =    0.0312315 ;
Time (1,1,160) =    0.0317057 ;
Time (2,1,1) =  0.000977003 ;
Time (2,1,2) =  0.000594108 ;
Time (2,1,4) =  0.000625188 ;
Time (2,1,8) =  0.000780085 ;
Time (2,1,16) =  0.000789192 ;
Time (2,1,32) =   0.00068945 ;
Time (2,1,64) =  0.000719502 ;
Time (2,1,128) =   0.00238172 ;
Time (2,1,160) =  0.000752914 ;
Time (3,1,1) =  0.000769536 ;
Time (3,1,2) =  0.000311361 ;
Time (3,1,4) =  0.000186002 ;
Time (3,1,8) =  0.000797995 ;
Time (3,1,16) =  0.000199759 ;
Time (3,1,32) =  0.000182959 ;
Time (3,1,64) =     0.011115 ;
Time (3,1,128) =    0.0001974 ;
Time (3,1,160) =  0.000211009 ;
Time (4,1,1) =   0.00100855 ;
Time (4,1,2) =  0.000641218 ;
Time (4,1,4) =  0.000621216 ;
Time (4,1,8) =  0.000733459 ;
Time (4,1,16) =  0.000822657 ;
Time (4,1,32) =  0.000819031 ;
Time (4,1,64) =  0.000731788 ;
Time (4,1,128) =  0.000765276 ;
Time (4,1,160) =  0.000223849 ;
Time (5,1,1) =  0.000895926 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella08/p2p-Gnutella08_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 6301 ;
Nedges (id) = 20777 ;
Ntri (id) = 2383 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000123496 ;
T_prep (1,1,2) =  0.000122585 ;
T_prep (1,1,4) =  0.000122422 ;
T_prep (1,1,8) =   0.00012219 ;
T_prep (1,1,16) =  0.000122095 ;
T_prep (1,1,32) =  0.000122108 ;
T_prep (1,1,64) =  0.000121946 ;
T_prep (1,1,128) =  0.000121816 ;
T_prep (1,1,160) =  0.000121795 ;
T_prep (2,1,1) =  0.000135734 ;
T_prep (2,1,2) =  0.000130902 ;
T_prep (2,1,4) =   0.00012836 ;
T_prep (2,1,8) =  0.000127908 ;
T_prep (2,1,16) =  0.000127262 ;
T_prep (2,1,32) =  0.000127635 ;
T_prep (2,1,64) =  0.000127343 ;
T_prep (2,1,128) =  0.000127048 ;
T_prep (2,1,160) =  0.000127162 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000401072 ;
Time (1,1,2) =   0.00910616 ;
Time (1,1,4) =  0.000203623 ;
Time (1,1,8) =  0.000333314 ;
Time (1,1,16) =  0.000492166 ;
Time (1,1,32) =  0.000829884 ;
Time (1,1,64) =   0.00230164 ;
Time (1,1,128) =    0.0395295 ;
Time (1,1,160) =    0.0275625 ;
Time (2,1,1) =   0.00383179 ;
Time (2,1,2) =   0.00195492 ;
Time (2,1,4) =   0.00165722 ;
Time (2,1,8) =   0.00158881 ;
Time (2,1,16) =   0.00162182 ;
Time (2,1,32) =   0.00205317 ;
Time (2,1,64) =   0.00168351 ;
Time (2,1,128) =    0.0123081 ;
Time (2,1,160) =   0.00151312 ;
Time (3,1,1) =   0.00681348 ;
Time (3,1,2) =   0.00354891 ;
Time (3,1,4) =   0.00195214 ;
Time (3,1,8) =   0.00145168 ;
Time (3,1,16) =   0.00151234 ;
Time (3,1,32) =   0.00135822 ;
Time (3,1,64) =   0.00136565 ;
Time (3,1,128) =   0.00138661 ;
Time (3,1,160) =   0.00251675 ;
Time (4,1,1) =   0.00257849 ;
Time (4,1,2) =   0.00147471 ;
Time (4,1,4) =   0.00106116 ;
Time (4,1,8) =   0.00150987 ;
Time (4,1,16) =   0.00131722 ;
Time (4,1,32) =   0.00107138 ;
Time (4,1,64) =   0.00112845 ;
Time (4,1,128) =   0.00106003 ;
Time (4,1,160) =   0.00169877 ;
Time (5,1,1) =   0.00205342 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010331/oregon1_010331_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10670 ;
Nedges (id) = 22002 ;
Ntri (id) = 17144 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00013897 ;
T_prep (1,1,2) =  0.000138108 ;
T_prep (1,1,4) =  0.000137896 ;
T_prep (1,1,8) =  0.000137625 ;
T_prep (1,1,16) =   0.00013735 ;
T_prep (1,1,32) =  0.000137646 ;
T_prep (1,1,64) =  0.000137748 ;
T_prep (1,1,128) =  0.000137437 ;
T_prep (1,1,160) =  0.000137403 ;
T_prep (2,1,1) =  0.000145937 ;
T_prep (2,1,2) =  0.000141494 ;
T_prep (2,1,4) =  0.000138977 ;
T_prep (2,1,8) =  0.000138449 ;
T_prep (2,1,16) =  0.000137947 ;
T_prep (2,1,32) =  0.000137909 ;
T_prep (2,1,64) =  0.000137801 ;
T_prep (2,1,128) =   0.00013794 ;
T_prep (2,1,160) =  0.000137437 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000586558 ;
Time (1,1,2) =   0.00941794 ;
Time (1,1,4) =  0.000280763 ;
Time (1,1,8) =  0.000278258 ;
Time (1,1,16) =  0.000589141 ;
Time (1,1,32) =   0.00100804 ;
Time (1,1,64) =   0.00258345 ;
Time (1,1,128) =    0.0479262 ;
Time (1,1,160) =    0.0292029 ;
Time (2,1,1) =     0.015735 ;
Time (2,1,2) =   0.00721566 ;
Time (2,1,4) =   0.00395429 ;
Time (2,1,8) =   0.00330216 ;
Time (2,1,16) =   0.00367674 ;
Time (2,1,32) =   0.00346155 ;
Time (2,1,64) =   0.00353386 ;
Time (2,1,128) =   0.00341485 ;
Time (2,1,160) =   0.00591154 ;
Time (3,1,1) =   0.00988055 ;
Time (3,1,2) =   0.00524404 ;
Time (3,1,4) =   0.00522824 ;
Time (3,1,8) =   0.00458787 ;
Time (3,1,16) =   0.00431623 ;
Time (3,1,32) =   0.00429494 ;
Time (3,1,64) =   0.00389232 ;
Time (3,1,128) =   0.00412903 ;
Time (3,1,160) =   0.00374656 ;
Time (4,1,1) =   0.00674306 ;
Time (4,1,2) =   0.00326821 ;
Time (4,1,4) =   0.00229465 ;
Time (4,1,8) =   0.00169279 ;
Time (4,1,16) =   0.00133283 ;
Time (4,1,32) =   0.00142754 ;
Time (4,1,64) =   0.00120526 ;
Time (4,1,128) =   0.00388713 ;
Time (4,1,160) =   0.00133008 ;
Time (5,1,1) =   0.00917598 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010407/oregon1_010407_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10729 ;
Nedges (id) = 21999 ;
Ntri (id) = 15834 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000177599 ;
T_prep (1,1,2) =  0.000170247 ;
T_prep (1,1,4) =  0.000169527 ;
T_prep (1,1,8) =  0.000168554 ;
T_prep (1,1,16) =  0.000139032 ;
T_prep (1,1,32) =  0.000138731 ;
T_prep (1,1,64) =  0.000138447 ;
T_prep (1,1,128) =  0.000138381 ;
T_prep (1,1,160) =  0.000138478 ;
T_prep (2,1,1) =  0.000146395 ;
T_prep (2,1,2) =  0.000141826 ;
T_prep (2,1,4) =  0.000139897 ;
T_prep (2,1,8) =  0.000139404 ;
T_prep (2,1,16) =  0.000138728 ;
T_prep (2,1,32) =  0.000138873 ;
T_prep (2,1,64) =  0.000138563 ;
T_prep (2,1,128) =  0.000138832 ;
T_prep (2,1,160) =  0.000138768 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000588029 ;
Time (1,1,2) =   0.00922309 ;
Time (1,1,4) =  0.000296033 ;
Time (1,1,8) =   0.00034219 ;
Time (1,1,16) =  0.000624109 ;
Time (1,1,32) =   0.00080338 ;
Time (1,1,64) =   0.00279763 ;
Time (1,1,128) =     0.013768 ;
Time (1,1,160) =    0.0314786 ;
Time (2,1,1) =      0.01365 ;
Time (2,1,2) =   0.00731874 ;
Time (2,1,4) =    0.0038955 ;
Time (2,1,8) =   0.00344814 ;
Time (2,1,16) =   0.00359648 ;
Time (2,1,32) =   0.00338443 ;
Time (2,1,64) =   0.00291234 ;
Time (2,1,128) =   0.00294804 ;
Time (2,1,160) =   0.00667796 ;
Time (3,1,1) =   0.00999035 ;
Time (3,1,2) =   0.00567826 ;
Time (3,1,4) =   0.00441896 ;
Time (3,1,8) =   0.00472101 ;
Time (3,1,16) =   0.00409574 ;
Time (3,1,32) =   0.00430002 ;
Time (3,1,64) =   0.00385302 ;
Time (3,1,128) =   0.00411326 ;
Time (3,1,160) =    0.0080776 ;
Time (4,1,1) =   0.00604613 ;
Time (4,1,2) =   0.00314601 ;
Time (4,1,4) =    0.0022081 ;
Time (4,1,8) =   0.00149082 ;
Time (4,1,16) =   0.00156666 ;
Time (4,1,32) =   0.00132335 ;
Time (4,1,64) =   0.00183439 ;
Time (4,1,128) =   0.00179998 ;
Time (4,1,160) =   0.00347542 ;
Time (5,1,1) =   0.00846417 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010414/oregon1_010414_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10790 ;
Nedges (id) = 22469 ;
Ntri (id) = 18237 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00014167 ;
T_prep (1,1,2) =  0.000140785 ;
T_prep (1,1,4) =  0.000140559 ;
T_prep (1,1,8) =  0.000140438 ;
T_prep (1,1,16) =  0.000140343 ;
T_prep (1,1,32) =  0.000140476 ;
T_prep (1,1,64) =  0.000140469 ;
T_prep (1,1,128) =  0.000140402 ;
T_prep (1,1,160) =  0.000140193 ;
T_prep (2,1,1) =  0.000148873 ;
T_prep (2,1,2) =  0.000143827 ;
T_prep (2,1,4) =   0.00014175 ;
T_prep (2,1,8) =  0.000140854 ;
T_prep (2,1,16) =  0.000140582 ;
T_prep (2,1,32) =  0.000140512 ;
T_prep (2,1,64) =  0.000141004 ;
T_prep (2,1,128) =  0.000140612 ;
T_prep (2,1,160) =  0.000140469 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000618753 ;
Time (1,1,2) =   0.00931837 ;
Time (1,1,4) =  0.000317794 ;
Time (1,1,8) =   0.00032643 ;
Time (1,1,16) =  0.000605348 ;
Time (1,1,32) =    0.0010251 ;
Time (1,1,64) =    0.0025463 ;
Time (1,1,128) =    0.0221768 ;
Time (1,1,160) =    0.0224803 ;
Time (2,1,1) =    0.0165025 ;
Time (2,1,2) =   0.00801714 ;
Time (2,1,4) =   0.00430844 ;
Time (2,1,8) =   0.00388775 ;
Time (2,1,16) =   0.00393256 ;
Time (2,1,32) =   0.00325503 ;
Time (2,1,64) =   0.00355538 ;
Time (2,1,128) =   0.00603026 ;
Time (2,1,160) =   0.00904311 ;
Time (3,1,1) =   0.00986071 ;
Time (3,1,2) =   0.00532971 ;
Time (3,1,4) =     0.004466 ;
Time (3,1,8) =   0.00483917 ;
Time (3,1,16) =   0.00446159 ;
Time (3,1,32) =   0.00476982 ;
Time (3,1,64) =   0.00444361 ;
Time (3,1,128) =   0.00500906 ;
Time (3,1,160) =    0.0042619 ;
Time (4,1,1) =    0.0060733 ;
Time (4,1,2) =   0.00314797 ;
Time (4,1,4) =    0.0015853 ;
Time (4,1,8) =   0.00223696 ;
Time (4,1,16) =   0.00154315 ;
Time (4,1,32) =   0.00168757 ;
Time (4,1,64) =   0.00158202 ;
Time (4,1,128) =    0.0164755 ;
Time (4,1,160) =   0.00142533 ;
Time (5,1,1) =    0.0083871 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010421/oregon1_010421_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10859 ;
Nedges (id) = 22747 ;
Ntri (id) = 19108 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000142933 ;
T_prep (1,1,2) =  0.000142019 ;
T_prep (1,1,4) =  0.000141793 ;
T_prep (1,1,8) =  0.000141392 ;
T_prep (1,1,16) =  0.000141364 ;
T_prep (1,1,32) =  0.000141713 ;
T_prep (1,1,64) =  0.000141451 ;
T_prep (1,1,128) =  0.000141443 ;
T_prep (1,1,160) =  0.000141428 ;
T_prep (2,1,1) =  0.000150636 ;
T_prep (2,1,2) =  0.000145179 ;
T_prep (2,1,4) =  0.000143183 ;
T_prep (2,1,8) =  0.000142412 ;
T_prep (2,1,16) =  0.000142026 ;
T_prep (2,1,32) =   0.00014176 ;
T_prep (2,1,64) =  0.000141527 ;
T_prep (2,1,128) =  0.000141916 ;
T_prep (2,1,160) =  0.000141914 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000628232 ;
Time (1,1,2) =   0.00934819 ;
Time (1,1,4) =  0.000330433 ;
Time (1,1,8) =  0.000324302 ;
Time (1,1,16) =   0.00060092 ;
Time (1,1,32) =  0.000941693 ;
Time (1,1,64) =   0.00262832 ;
Time (1,1,128) =    0.0478964 ;
Time (1,1,160) =    0.0268325 ;
Time (2,1,1) =     0.033424 ;
Time (2,1,2) =    0.0153154 ;
Time (2,1,4) =   0.00727594 ;
Time (2,1,8) =   0.00740413 ;
Time (2,1,16) =   0.00724808 ;
Time (2,1,32) =   0.00558563 ;
Time (2,1,64) =   0.00575954 ;
Time (2,1,128) =   0.00441849 ;
Time (2,1,160) =   0.00552716 ;
Time (3,1,1) =    0.0184517 ;
Time (3,1,2) =    0.0107508 ;
Time (3,1,4) =    0.0134044 ;
Time (3,1,8) =    0.0104116 ;
Time (3,1,16) =   0.00922027 ;
Time (3,1,32) =   0.00709893 ;
Time (3,1,64) =   0.00692297 ;
Time (3,1,128) =   0.00526477 ;
Time (3,1,160) =    0.0099086 ;
Time (4,1,1) =    0.0134573 ;
Time (4,1,2) =   0.00823764 ;
Time (4,1,4) =   0.00372274 ;
Time (4,1,8) =   0.00299351 ;
Time (4,1,16) =   0.00358281 ;
Time (4,1,32) =   0.00226119 ;
Time (4,1,64) =   0.00237844 ;
Time (4,1,128) =   0.00151648 ;
Time (4,1,160) =   0.00259224 ;
Time (5,1,1) =    0.0185981 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010428/oregon1_010428_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10886 ;
Nedges (id) = 22493 ;
Ntri (id) = 17645 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00014268 ;
T_prep (1,1,2) =  0.000141404 ;
T_prep (1,1,4) =  0.000141243 ;
T_prep (1,1,8) =  0.000141178 ;
T_prep (1,1,16) =  0.000141106 ;
T_prep (1,1,32) =  0.000141045 ;
T_prep (1,1,64) =  0.000141146 ;
T_prep (1,1,128) =  0.000141045 ;
T_prep (1,1,160) =  0.000141143 ;
T_prep (2,1,1) =  0.000149483 ;
T_prep (2,1,2) =  0.000144777 ;
T_prep (2,1,4) =  0.000141894 ;
T_prep (2,1,8) =  0.000141555 ;
T_prep (2,1,16) =  0.000140926 ;
T_prep (2,1,32) =  0.000140719 ;
T_prep (2,1,64) =  0.000140797 ;
T_prep (2,1,128) =  0.000155305 ;
T_prep (2,1,160) =  0.000141025 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000615624 ;
Time (1,1,2) =   0.00930483 ;
Time (1,1,4) =  0.000324568 ;
Time (1,1,8) =   0.00040107 ;
Time (1,1,16) =  0.000458699 ;
Time (1,1,32) =    0.0010473 ;
Time (1,1,64) =   0.00248527 ;
Time (1,1,128) =    0.0280885 ;
Time (1,1,160) =    0.0269979 ;
Time (2,1,1) =    0.0161566 ;
Time (2,1,2) =   0.00717657 ;
Time (2,1,4) =   0.00394322 ;
Time (2,1,8) =   0.00307822 ;
Time (2,1,16) =   0.00375636 ;
Time (2,1,32) =   0.00315969 ;
Time (2,1,64) =   0.00303583 ;
Time (2,1,128) =   0.00844509 ;
Time (2,1,160) =   0.00281998 ;
Time (3,1,1) =    0.0106883 ;
Time (3,1,2) =   0.00569684 ;
Time (3,1,4) =   0.00423559 ;
Time (3,1,8) =   0.00472859 ;
Time (3,1,16) =   0.00415181 ;
Time (3,1,32) =   0.00430764 ;
Time (3,1,64) =   0.00448197 ;
Time (3,1,128) =   0.00856544 ;
Time (3,1,160) =   0.00444706 ;
Time (4,1,1) =   0.00592864 ;
Time (4,1,2) =   0.00337126 ;
Time (4,1,4) =   0.00170882 ;
Time (4,1,8) =   0.00150837 ;
Time (4,1,16) =   0.00142471 ;
Time (4,1,32) =   0.00138116 ;
Time (4,1,64) =   0.00193032 ;
Time (4,1,128) =   0.00132445 ;
Time (4,1,160) =  0.000886829 ;
Time (5,1,1) =    0.0092586 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010505/oregon1_010505_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10943 ;
Nedges (id) = 22607 ;
Ntri (id) = 17597 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000143659 ;
T_prep (1,1,2) =   0.00014263 ;
T_prep (1,1,4) =  0.000142783 ;
T_prep (1,1,8) =  0.000142458 ;
T_prep (1,1,16) =  0.000142605 ;
T_prep (1,1,32) =  0.000142348 ;
T_prep (1,1,64) =  0.000142436 ;
T_prep (1,1,128) =  0.000142431 ;
T_prep (1,1,160) =  0.000142095 ;
T_prep (2,1,1) =  0.000150973 ;
T_prep (2,1,2) =  0.000145866 ;
T_prep (2,1,4) =  0.000143517 ;
T_prep (2,1,8) =  0.000142951 ;
T_prep (2,1,16) =  0.000142549 ;
T_prep (2,1,32) =   0.00014251 ;
T_prep (2,1,64) =  0.000142309 ;
T_prep (2,1,128) =  0.000141863 ;
T_prep (2,1,160) =  0.000142133 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000620109 ;
Time (1,1,2) =   0.00929921 ;
Time (1,1,4) =   0.00032783 ;
Time (1,1,8) =  0.000347425 ;
Time (1,1,16) =   0.00052113 ;
Time (1,1,32) =   0.00106332 ;
Time (1,1,64) =   0.00300946 ;
Time (1,1,128) =    0.0202027 ;
Time (1,1,160) =     0.044485 ;
Time (2,1,1) =    0.0147995 ;
Time (2,1,2) =   0.00772814 ;
Time (2,1,4) =   0.00362245 ;
Time (2,1,8) =   0.00371882 ;
Time (2,1,16) =   0.00354554 ;
Time (2,1,32) =   0.00323962 ;
Time (2,1,64) =   0.00311599 ;
Time (2,1,128) =   0.00303179 ;
Time (2,1,160) =   0.00505398 ;
Time (3,1,1) =    0.0101009 ;
Time (3,1,2) =   0.00551665 ;
Time (3,1,4) =   0.00394039 ;
Time (3,1,8) =   0.00252531 ;
Time (3,1,16) =   0.00460891 ;
Time (3,1,32) =    0.0044589 ;
Time (3,1,64) =   0.00431406 ;
Time (3,1,128) =   0.00853301 ;
Time (3,1,160) =   0.00428838 ;
Time (4,1,1) =   0.00621843 ;
Time (4,1,2) =    0.0034305 ;
Time (4,1,4) =   0.00214044 ;
Time (4,1,8) =   0.00170562 ;
Time (4,1,16) =   0.00134963 ;
Time (4,1,32) =   0.00136279 ;
Time (4,1,64) =   0.00151094 ;
Time (4,1,128) =    0.0172449 ;
Time (4,1,160) =   0.00348997 ;
Time (5,1,1) =   0.00905187 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010512/oregon1_010512_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11011 ;
Nedges (id) = 22677 ;
Ntri (id) = 17598 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000144348 ;
T_prep (1,1,2) =  0.000143338 ;
T_prep (1,1,4) =  0.000142817 ;
T_prep (1,1,8) =   0.00014292 ;
T_prep (1,1,16) =  0.000142843 ;
T_prep (1,1,32) =   0.00014295 ;
T_prep (1,1,64) =  0.000142744 ;
T_prep (1,1,128) =   0.00014266 ;
T_prep (1,1,160) =  0.000142766 ;
T_prep (2,1,1) =  0.000150822 ;
T_prep (2,1,2) =  0.000146621 ;
T_prep (2,1,4) =  0.000144539 ;
T_prep (2,1,8) =  0.000144063 ;
T_prep (2,1,16) =  0.000143331 ;
T_prep (2,1,32) =  0.000143172 ;
T_prep (2,1,64) =  0.000143101 ;
T_prep (2,1,128) =  0.000142947 ;
T_prep (2,1,160) =  0.000142789 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000620653 ;
Time (1,1,2) =   0.00932018 ;
Time (1,1,4) =  0.000325061 ;
Time (1,1,8) =  0.000367597 ;
Time (1,1,16) =  0.000544053 ;
Time (1,1,32) =   0.00112811 ;
Time (1,1,64) =    0.0025824 ;
Time (1,1,128) =    0.0308381 ;
Time (1,1,160) =    0.0263016 ;
Time (2,1,1) =    0.0148339 ;
Time (2,1,2) =   0.00742358 ;
Time (2,1,4) =   0.00329639 ;
Time (2,1,8) =   0.00316987 ;
Time (2,1,16) =   0.00313047 ;
Time (2,1,32) =   0.00327166 ;
Time (2,1,64) =   0.00258059 ;
Time (2,1,128) =   0.00505697 ;
Time (2,1,160) =     0.006783 ;
Time (3,1,1) =   0.00936968 ;
Time (3,1,2) =   0.00513724 ;
Time (3,1,4) =   0.00435187 ;
Time (3,1,8) =   0.00415905 ;
Time (3,1,16) =   0.00427283 ;
Time (3,1,32) =    0.0042773 ;
Time (3,1,64) =   0.00418874 ;
Time (3,1,128) =   0.00422078 ;
Time (3,1,160) =    0.0130811 ;
Time (4,1,1) =   0.00565551 ;
Time (4,1,2) =   0.00330837 ;
Time (4,1,4) =   0.00173119 ;
Time (4,1,8) =    0.0018038 ;
Time (4,1,16) =   0.00173216 ;
Time (4,1,32) =    0.0011424 ;
Time (4,1,64) =   0.00141574 ;
Time (4,1,128) =   0.00134254 ;
Time (4,1,160) =    0.0012435 ;
Time (5,1,1) =   0.00868597 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010519/oregon1_010519_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11051 ;
Nedges (id) = 22724 ;
Ntri (id) = 17677 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000144697 ;
T_prep (1,1,2) =  0.000143479 ;
T_prep (1,1,4) =  0.000143411 ;
T_prep (1,1,8) =  0.000143282 ;
T_prep (1,1,16) =  0.000143223 ;
T_prep (1,1,32) =  0.000143096 ;
T_prep (1,1,64) =  0.000143242 ;
T_prep (1,1,128) =  0.000143047 ;
T_prep (1,1,160) =  0.000143228 ;
T_prep (2,1,1) =  0.000152619 ;
T_prep (2,1,2) =  0.000147652 ;
T_prep (2,1,4) =  0.000144959 ;
T_prep (2,1,8) =  0.000144535 ;
T_prep (2,1,16) =  0.000143826 ;
T_prep (2,1,32) =  0.000143732 ;
T_prep (2,1,64) =  0.000143775 ;
T_prep (2,1,128) =  0.000143744 ;
T_prep (2,1,160) =  0.000143452 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000624558 ;
Time (1,1,2) =   0.00929733 ;
Time (1,1,4) =  0.000308091 ;
Time (1,1,8) =  0.000345416 ;
Time (1,1,16) =  0.000440398 ;
Time (1,1,32) =   0.00108038 ;
Time (1,1,64) =   0.00265668 ;
Time (1,1,128) =    0.0275332 ;
Time (1,1,160) =     0.032923 ;
Time (2,1,1) =    0.0155068 ;
Time (2,1,2) =   0.00855161 ;
Time (2,1,4) =   0.00379175 ;
Time (2,1,8) =   0.00378958 ;
Time (2,1,16) =   0.00397633 ;
Time (2,1,32) =   0.00330797 ;
Time (2,1,64) =   0.00343779 ;
Time (2,1,128) =   0.00514727 ;
Time (2,1,160) =   0.00474639 ;
Time (3,1,1) =    0.0107667 ;
Time (3,1,2) =   0.00617974 ;
Time (3,1,4) =   0.00344596 ;
Time (3,1,8) =    0.0036422 ;
Time (3,1,16) =   0.00359179 ;
Time (3,1,32) =   0.00347572 ;
Time (3,1,64) =   0.00297257 ;
Time (3,1,128) =   0.00326591 ;
Time (3,1,160) =    0.0032927 ;
Time (4,1,1) =   0.00672873 ;
Time (4,1,2) =   0.00345945 ;
Time (4,1,4) =    0.0024593 ;
Time (4,1,8) =   0.00170752 ;
Time (4,1,16) =   0.00160619 ;
Time (4,1,32) =   0.00137611 ;
Time (4,1,64) =   0.00142371 ;
Time (4,1,128) =   0.00203046 ;
Time (4,1,160) =   0.00370813 ;
Time (5,1,1) =   0.00997919 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon1_010526/oregon1_010526_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11174 ;
Nedges (id) = 23409 ;
Ntri (id) = 19894 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000148266 ;
T_prep (1,1,2) =  0.000147215 ;
T_prep (1,1,4) =  0.000146654 ;
T_prep (1,1,8) =  0.000146789 ;
T_prep (1,1,16) =  0.000146657 ;
T_prep (1,1,32) =  0.000146576 ;
T_prep (1,1,64) =  0.000146625 ;
T_prep (1,1,128) =  0.000146762 ;
T_prep (1,1,160) =  0.000146548 ;
T_prep (2,1,1) =  0.000155326 ;
T_prep (2,1,2) =  0.000150152 ;
T_prep (2,1,4) =  0.000148062 ;
T_prep (2,1,8) =  0.000147045 ;
T_prep (2,1,16) =  0.000146957 ;
T_prep (2,1,32) =  0.000146969 ;
T_prep (2,1,64) =  0.000146872 ;
T_prep (2,1,128) =  0.000146737 ;
T_prep (2,1,160) =  0.000146427 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000651683 ;
Time (1,1,2) =   0.00965769 ;
Time (1,1,4) =  0.000319177 ;
Time (1,1,8) =  0.000417105 ;
Time (1,1,16) =  0.000446843 ;
Time (1,1,32) =   0.00109537 ;
Time (1,1,64) =   0.00281394 ;
Time (1,1,128) =    0.0400546 ;
Time (1,1,160) =    0.0437575 ;
Time (2,1,1) =    0.0168921 ;
Time (2,1,2) =   0.00760479 ;
Time (2,1,4) =   0.00418341 ;
Time (2,1,8) =   0.00375146 ;
Time (2,1,16) =   0.00335051 ;
Time (2,1,32) =   0.00345555 ;
Time (2,1,64) =   0.00343803 ;
Time (2,1,128) =   0.00454405 ;
Time (2,1,160) =   0.00868888 ;
Time (3,1,1) =    0.0107005 ;
Time (3,1,2) =   0.00627104 ;
Time (3,1,4) =   0.00375848 ;
Time (3,1,8) =    0.0036804 ;
Time (3,1,16) =   0.00368871 ;
Time (3,1,32) =    0.0038138 ;
Time (3,1,64) =   0.00365298 ;
Time (3,1,128) =   0.00352561 ;
Time (3,1,160) =   0.00354282 ;
Time (4,1,1) =   0.00635914 ;
Time (4,1,2) =   0.00335947 ;
Time (4,1,4) =   0.00184941 ;
Time (4,1,8) =   0.00205251 ;
Time (4,1,16) =   0.00147117 ;
Time (4,1,32) =    0.0012564 ;
Time (4,1,64) =   0.00118148 ;
Time (4,1,128) =   0.00390786 ;
Time (4,1,160) =   0.00139918 ;
Time (5,1,1) =   0.00902053 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 31036 ;
Ntri (id) = 45013 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  8.31764e-05 ;
T_prep (1,1,2) =  8.30442e-05 ;
T_prep (1,1,4) =  8.31597e-05 ;
T_prep (1,1,8) =   8.3115e-05 ;
T_prep (1,1,16) =  8.30414e-05 ;
T_prep (1,1,32) =  8.31187e-05 ;
T_prep (1,1,64) =  8.31094e-05 ;
T_prep (1,1,128) =   8.2504e-05 ;
T_prep (1,1,160) =  8.30926e-05 ;
T_prep (2,1,1) =   8.3291e-05 ;
T_prep (2,1,2) =  8.28784e-05 ;
T_prep (2,1,4) =  8.26372e-05 ;
T_prep (2,1,8) =  8.28132e-05 ;
T_prep (2,1,16) =  8.27573e-05 ;
T_prep (2,1,32) =  8.26912e-05 ;
T_prep (2,1,64) =  8.28793e-05 ;
T_prep (2,1,128) =  8.27014e-05 ;
T_prep (2,1,160) =  8.28449e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000406279 ;
Time (1,1,2) =   0.00948307 ;
Time (1,1,4) =  0.000603146 ;
Time (1,1,8) =  0.000659775 ;
Time (1,1,16) =  0.000862488 ;
Time (1,1,32) =   0.00142726 ;
Time (1,1,64) =   0.00416041 ;
Time (1,1,128) =    0.0320959 ;
Time (1,1,160) =    0.0289049 ;
Time (2,1,1) =    0.0137628 ;
Time (2,1,2) =    0.0141442 ;
Time (2,1,4) =     0.013444 ;
Time (2,1,8) =    0.0127767 ;
Time (2,1,16) =    0.0135167 ;
Time (2,1,32) =    0.0120183 ;
Time (2,1,64) =    0.0114776 ;
Time (2,1,128) =    0.0108594 ;
Time (2,1,160) =    0.0318501 ;
Time (3,1,1) =   0.00737063 ;
Time (3,1,2) =   0.00402354 ;
Time (3,1,4) =   0.00253605 ;
Time (3,1,8) =   0.00189185 ;
Time (3,1,16) =   0.00182613 ;
Time (3,1,32) =   0.00182216 ;
Time (3,1,64) =   0.00180984 ;
Time (3,1,128) =   0.00240554 ;
Time (3,1,160) =   0.00933781 ;
Time (4,1,1) =   0.00527578 ;
Time (4,1,2) =   0.00579062 ;
Time (4,1,4) =    0.0051174 ;
Time (4,1,8) =   0.00569207 ;
Time (4,1,16) =   0.00447071 ;
Time (4,1,32) =   0.00473766 ;
Time (4,1,64) =   0.00478484 ;
Time (4,1,128) =   0.00446156 ;
Time (4,1,160) =  0.000672588 ;
Time (5,1,1) =    0.0043856 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 5100 ;
Nedges (id) = 31036 ;
Ntri (id) = 35 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  8.41971e-05 ;
T_prep (1,1,2) =  8.46507e-05 ;
T_prep (1,1,4) =  8.40444e-05 ;
T_prep (1,1,8) =  8.46134e-05 ;
T_prep (1,1,16) =  8.40919e-05 ;
T_prep (1,1,32) =  8.45091e-05 ;
T_prep (1,1,64) =  8.43201e-05 ;
T_prep (1,1,128) =  8.38842e-05 ;
T_prep (1,1,160) =  8.45743e-05 ;
T_prep (2,1,1) =  8.52384e-05 ;
T_prep (2,1,2) =  8.47792e-05 ;
T_prep (2,1,4) =  8.47494e-05 ;
T_prep (2,1,8) =  8.43955e-05 ;
T_prep (2,1,16) =  8.46665e-05 ;
T_prep (2,1,32) =  8.46526e-05 ;
T_prep (2,1,64) =  8.46479e-05 ;
T_prep (2,1,128) =  8.48295e-05 ;
T_prep (2,1,160) =  8.44626e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000166079 ;
Time (1,1,2) =   0.00905201 ;
Time (1,1,4) =  0.000205523 ;
Time (1,1,8) =  0.000306807 ;
Time (1,1,16) =  0.000422074 ;
Time (1,1,32) =   0.00106531 ;
Time (1,1,64) =   0.00254364 ;
Time (1,1,128) =    0.0416332 ;
Time (1,1,160) =    0.0259979 ;
Time (2,1,1) =   0.00316359 ;
Time (2,1,2) =   0.00247301 ;
Time (2,1,4) =   0.00270019 ;
Time (2,1,8) =   0.00264032 ;
Time (2,1,16) =   0.00266378 ;
Time (2,1,32) =   0.00262873 ;
Time (2,1,64) =   0.00252953 ;
Time (2,1,128) =   0.00717561 ;
Time (2,1,160) =   0.00243507 ;
Time (3,1,1) =   0.00195902 ;
Time (3,1,2) =   0.00132491 ;
Time (3,1,4) =    0.0013114 ;
Time (3,1,8) =   0.00124266 ;
Time (3,1,16) =   0.00110424 ;
Time (3,1,32) =   0.00131434 ;
Time (3,1,64) =   0.00122024 ;
Time (3,1,128) =   0.00316817 ;
Time (3,1,160) =   0.00118577 ;
Time (4,1,1) =   0.00191758 ;
Time (4,1,2) =   0.00141683 ;
Time (4,1,4) =   0.00165772 ;
Time (4,1,8) =   0.00164643 ;
Time (4,1,16) =   0.00155163 ;
Time (4,1,32) =   0.00167871 ;
Time (4,1,64) =   0.00149278 ;
Time (4,1,128) =   0.00144993 ;
Time (4,1,160) =   0.00439142 ;
Time (5,1,1) =   0.00394775 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella09/p2p-Gnutella09_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8114 ;
Nedges (id) = 26013 ;
Ntri (id) = 2354 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000157227 ;
T_prep (1,1,2) =  0.000156084 ;
T_prep (1,1,4) =  0.000155819 ;
T_prep (1,1,8) =    0.0001556 ;
T_prep (1,1,16) =  0.000155631 ;
T_prep (1,1,32) =  0.000155707 ;
T_prep (1,1,64) =  0.000155682 ;
T_prep (1,1,128) =  0.000155625 ;
T_prep (1,1,160) =  0.000155658 ;
T_prep (2,1,1) =   0.00017084 ;
T_prep (2,1,2) =  0.000186465 ;
T_prep (2,1,4) =  0.000163041 ;
T_prep (2,1,8) =  0.000162935 ;
T_prep (2,1,16) =  0.000162281 ;
T_prep (2,1,32) =  0.000162113 ;
T_prep (2,1,64) =  0.000162281 ;
T_prep (2,1,128) =   0.00016174 ;
T_prep (2,1,160) =  0.000161988 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000484638 ;
Time (1,1,2) =   0.00917123 ;
Time (1,1,4) =  0.000214824 ;
Time (1,1,8) =  0.000237675 ;
Time (1,1,16) =  0.000432202 ;
Time (1,1,32) =  0.000868415 ;
Time (1,1,64) =   0.00247244 ;
Time (1,1,128) =    0.0312369 ;
Time (1,1,160) =    0.0454415 ;
Time (2,1,1) =   0.00425557 ;
Time (2,1,2) =   0.00343497 ;
Time (2,1,4) =   0.00162431 ;
Time (2,1,8) =   0.00247973 ;
Time (2,1,16) =   0.00169195 ;
Time (2,1,32) =   0.00144391 ;
Time (2,1,64) =    0.0027576 ;
Time (2,1,128) =    0.0204216 ;
Time (2,1,160) =   0.00195215 ;
Time (3,1,1) =    0.0080064 ;
Time (3,1,2) =   0.00514914 ;
Time (3,1,4) =   0.00308017 ;
Time (3,1,8) =   0.00200221 ;
Time (3,1,16) =   0.00205604 ;
Time (3,1,32) =   0.00196474 ;
Time (3,1,64) =   0.00196938 ;
Time (3,1,128) =   0.00197767 ;
Time (3,1,160) =   0.00200956 ;
Time (4,1,1) =   0.00326545 ;
Time (4,1,2) =   0.00167269 ;
Time (4,1,4) =   0.00138582 ;
Time (4,1,8) =   0.00117348 ;
Time (4,1,16) =   0.00117317 ;
Time (4,1,32) =   0.00107237 ;
Time (4,1,64) =  0.000967083 ;
Time (4,1,128) =   0.00181516 ;
Time (4,1,160) =   0.00541769 ;
Time (5,1,1) =   0.00254919 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-HepTh/ca-HepTh_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 9877 ;
Nedges (id) = 25973 ;
Ntri (id) = 28339 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000175709 ;
T_prep (1,1,2) =  0.000174529 ;
T_prep (1,1,4) =  0.000174602 ;
T_prep (1,1,8) =  0.000174305 ;
T_prep (1,1,16) =  0.000174755 ;
T_prep (1,1,32) =  0.000174214 ;
T_prep (1,1,64) =  0.000174236 ;
T_prep (1,1,128) =  0.000174128 ;
T_prep (1,1,160) =  0.000173935 ;
T_prep (2,1,1) =  0.000187644 ;
T_prep (2,1,2) =  0.000182345 ;
T_prep (2,1,4) =  0.000180435 ;
T_prep (2,1,8) =  0.000179951 ;
T_prep (2,1,16) =  0.000179184 ;
T_prep (2,1,32) =  0.000179211 ;
T_prep (2,1,64) =  0.000179326 ;
T_prep (2,1,128) =   0.00017925 ;
T_prep (2,1,160) =  0.000179186 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000487277 ;
Time (1,1,2) =     0.009207 ;
Time (1,1,4) =   0.00021093 ;
Time (1,1,8) =  0.000218397 ;
Time (1,1,16) =  0.000435366 ;
Time (1,1,32) =  0.000813109 ;
Time (1,1,64) =   0.00228821 ;
Time (1,1,128) =    0.0200568 ;
Time (1,1,160) =    0.0259915 ;
Time (2,1,1) =   0.00387155 ;
Time (2,1,2) =   0.00185934 ;
Time (2,1,4) =   0.00228821 ;
Time (2,1,8) =   0.00124948 ;
Time (2,1,16) =  0.000896966 ;
Time (2,1,32) =   0.00119745 ;
Time (2,1,64) =  0.000945765 ;
Time (2,1,128) =   0.00327294 ;
Time (2,1,160) =   0.00239265 ;
Time (3,1,1) =   0.00750323 ;
Time (3,1,2) =   0.00368861 ;
Time (3,1,4) =    0.0021735 ;
Time (3,1,8) =   0.00185265 ;
Time (3,1,16) =    0.0013956 ;
Time (3,1,32) =   0.00167496 ;
Time (3,1,64) =   0.00163356 ;
Time (3,1,128) =   0.00157747 ;
Time (3,1,160) =   0.00144097 ;
Time (4,1,1) =   0.00322389 ;
Time (4,1,2) =   0.00147512 ;
Time (4,1,4) =  0.000798889 ;
Time (4,1,8) =    0.0013286 ;
Time (4,1,16) =   0.00109555 ;
Time (4,1,32) =  0.000830265 ;
Time (4,1,64) =  0.000668865 ;
Time (4,1,128) =  0.000609705 ;
Time (4,1,160) =   0.00179073 ;
Time (5,1,1) =    0.0025151 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010331/oregon2_010331_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10900 ;
Nedges (id) = 31180 ;
Ntri (id) = 82856 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000168198 ;
T_prep (1,1,2) =  0.000167341 ;
T_prep (1,1,4) =  0.000167121 ;
T_prep (1,1,8) =   0.00016695 ;
T_prep (1,1,16) =  0.000167015 ;
T_prep (1,1,32) =  0.000166957 ;
T_prep (1,1,64) =  0.000166908 ;
T_prep (1,1,128) =  0.000167087 ;
T_prep (1,1,160) =   0.00016692 ;
T_prep (2,1,1) =  0.000183158 ;
T_prep (2,1,2) =  0.000173204 ;
T_prep (2,1,4) =  0.000171267 ;
T_prep (2,1,8) =  0.000170299 ;
T_prep (2,1,16) =  0.000170317 ;
T_prep (2,1,32) =  0.000170527 ;
T_prep (2,1,64) =  0.000169808 ;
T_prep (2,1,128) =  0.000170447 ;
T_prep (2,1,160) =  0.000169928 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00086861 ;
Time (1,1,2) =   0.00938931 ;
Time (1,1,4) =   0.00039785 ;
Time (1,1,8) =  0.000396896 ;
Time (1,1,16) =  0.000548169 ;
Time (1,1,32) =   0.00114414 ;
Time (1,1,64) =   0.00255752 ;
Time (1,1,128) =    0.0304068 ;
Time (1,1,160) =     0.036906 ;
Time (2,1,1) =     0.020542 ;
Time (2,1,2) =    0.0108363 ;
Time (2,1,4) =   0.00579312 ;
Time (2,1,8) =   0.00450124 ;
Time (2,1,16) =    0.0036895 ;
Time (2,1,32) =   0.00616674 ;
Time (2,1,64) =   0.00359086 ;
Time (2,1,128) =   0.00320125 ;
Time (2,1,160) =   0.00996185 ;
Time (3,1,1) =    0.0253272 ;
Time (3,1,2) =    0.0141804 ;
Time (3,1,4) =   0.00833979 ;
Time (3,1,8) =   0.00878472 ;
Time (3,1,16) =   0.00798485 ;
Time (3,1,32) =   0.00674424 ;
Time (3,1,64) =    0.0134483 ;
Time (3,1,128) =    0.0068447 ;
Time (3,1,160) =    0.0135061 ;
Time (4,1,1) =    0.0082525 ;
Time (4,1,2) =   0.00404867 ;
Time (4,1,4) =   0.00261001 ;
Time (4,1,8) =   0.00234652 ;
Time (4,1,16) =   0.00167643 ;
Time (4,1,32) =     0.001593 ;
Time (4,1,64) =     0.001722 ;
Time (4,1,128) =   0.00147472 ;
Time (4,1,160) =    0.0011485 ;
Time (5,1,1) =    0.0109983 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010407/oregon2_010407_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10981 ;
Nedges (id) = 30855 ;
Ntri (id) = 78138 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000168025 ;
T_prep (1,1,2) =  0.000167017 ;
T_prep (1,1,4) =  0.000166816 ;
T_prep (1,1,8) =  0.000166883 ;
T_prep (1,1,16) =  0.000166543 ;
T_prep (1,1,32) =  0.000166578 ;
T_prep (1,1,64) =   0.00016659 ;
T_prep (1,1,128) =  0.000166593 ;
T_prep (1,1,160) =  0.000166534 ;
T_prep (2,1,1) =  0.000182888 ;
T_prep (2,1,2) =  0.000173044 ;
T_prep (2,1,4) =  0.000171275 ;
T_prep (2,1,8) =  0.000170331 ;
T_prep (2,1,16) =  0.000169941 ;
T_prep (2,1,32) =  0.000169865 ;
T_prep (2,1,64) =  0.000169334 ;
T_prep (2,1,128) =  0.000169703 ;
T_prep (2,1,160) =  0.000169428 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000854447 ;
Time (1,1,2) =   0.00944122 ;
Time (1,1,4) =  0.000493677 ;
Time (1,1,8) =  0.000464567 ;
Time (1,1,16) =   0.00069347 ;
Time (1,1,32) =   0.00110791 ;
Time (1,1,64) =    0.0031032 ;
Time (1,1,128) =    0.0308066 ;
Time (1,1,160) =     0.027956 ;
Time (2,1,1) =    0.0189449 ;
Time (2,1,2) =    0.0108312 ;
Time (2,1,4) =   0.00527013 ;
Time (2,1,8) =   0.00413492 ;
Time (2,1,16) =   0.00411752 ;
Time (2,1,32) =   0.00361309 ;
Time (2,1,64) =   0.00382898 ;
Time (2,1,128) =   0.00333882 ;
Time (2,1,160) =     0.010068 ;
Time (3,1,1) =    0.0233524 ;
Time (3,1,2) =    0.0145326 ;
Time (3,1,4) =    0.0077258 ;
Time (3,1,8) =   0.00816587 ;
Time (3,1,16) =   0.00794523 ;
Time (3,1,32) =   0.00778534 ;
Time (3,1,64) =   0.00693568 ;
Time (3,1,128) =   0.00755611 ;
Time (3,1,160) =   0.00767563 ;
Time (4,1,1) =   0.00722032 ;
Time (4,1,2) =   0.00464625 ;
Time (4,1,4) =   0.00264534 ;
Time (4,1,8) =   0.00183786 ;
Time (4,1,16) =   0.00153106 ;
Time (4,1,32) =   0.00164898 ;
Time (4,1,64) =   0.00157631 ;
Time (4,1,128) =   0.00361543 ;
Time (4,1,160) =   0.00145964 ;
Time (5,1,1) =    0.0108144 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010505/oregon2_010505_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11157 ;
Nedges (id) = 30943 ;
Ntri (id) = 72182 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000170126 ;
T_prep (1,1,2) =  0.000169318 ;
T_prep (1,1,4) =  0.000168928 ;
T_prep (1,1,8) =  0.000169355 ;
T_prep (1,1,16) =  0.000169037 ;
T_prep (1,1,32) =  0.000168996 ;
T_prep (1,1,64) =  0.000168716 ;
T_prep (1,1,128) =  0.000168893 ;
T_prep (1,1,160) =  0.000168626 ;
T_prep (2,1,1) =  0.000191313 ;
T_prep (2,1,2) =  0.000175565 ;
T_prep (2,1,4) =  0.000173105 ;
T_prep (2,1,8) =  0.000172476 ;
T_prep (2,1,16) =  0.000172026 ;
T_prep (2,1,32) =  0.000172335 ;
T_prep (2,1,64) =  0.000172266 ;
T_prep (2,1,128) =  0.000172229 ;
T_prep (2,1,160) =  0.000217303 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000859166 ;
Time (1,1,2) =    0.0094436 ;
Time (1,1,4) =  0.000478012 ;
Time (1,1,8) =  0.000425872 ;
Time (1,1,16) =  0.000630982 ;
Time (1,1,32) =    0.0012626 ;
Time (1,1,64) =   0.00263174 ;
Time (1,1,128) =    0.0221352 ;
Time (1,1,160) =    0.0284702 ;
Time (2,1,1) =    0.0483958 ;
Time (2,1,2) =    0.0191811 ;
Time (2,1,4) =    0.0111921 ;
Time (2,1,8) =   0.00740077 ;
Time (2,1,16) =   0.00685879 ;
Time (2,1,32) =   0.00792444 ;
Time (2,1,64) =   0.00701207 ;
Time (2,1,128) =   0.00574779 ;
Time (2,1,160) =   0.00399416 ;
Time (3,1,1) =    0.0449425 ;
Time (3,1,2) =    0.0276414 ;
Time (3,1,4) =    0.0148339 ;
Time (3,1,8) =    0.0106731 ;
Time (3,1,16) =    0.0112581 ;
Time (3,1,32) =    0.0118317 ;
Time (3,1,64) =    0.0102365 ;
Time (3,1,128) =    0.0057394 ;
Time (3,1,160) =    0.0113416 ;
Time (4,1,1) =    0.0172787 ;
Time (4,1,2) =   0.00779707 ;
Time (4,1,4) =   0.00425509 ;
Time (4,1,8) =   0.00386329 ;
Time (4,1,16) =   0.00536376 ;
Time (4,1,32) =   0.00317249 ;
Time (4,1,64) =    0.0025273 ;
Time (4,1,128) =   0.00189864 ;
Time (4,1,160) =   0.00130716 ;
Time (5,1,1) =    0.0226406 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010414/oregon2_010414_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11019 ;
Nedges (id) = 31761 ;
Ntri (id) = 88905 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000171043 ;
T_prep (1,1,2) =  0.000193102 ;
T_prep (1,1,4) =  0.000169666 ;
T_prep (1,1,8) =  0.000169695 ;
T_prep (1,1,16) =  0.000169598 ;
T_prep (1,1,32) =  0.000169658 ;
T_prep (1,1,64) =  0.000169662 ;
T_prep (1,1,128) =  0.000169497 ;
T_prep (1,1,160) =  0.000169928 ;
T_prep (2,1,1) =  0.000185956 ;
T_prep (2,1,2) =  0.000176291 ;
T_prep (2,1,4) =  0.000174166 ;
T_prep (2,1,8) =  0.000173552 ;
T_prep (2,1,16) =  0.000173031 ;
T_prep (2,1,32) =   0.00017296 ;
T_prep (2,1,64) =   0.00017269 ;
T_prep (2,1,128) =   0.00017298 ;
T_prep (2,1,160) =  0.000172925 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00088972 ;
Time (1,1,2) =   0.00951871 ;
Time (1,1,4) =  0.000496844 ;
Time (1,1,8) =  0.000435072 ;
Time (1,1,16) =   0.00076474 ;
Time (1,1,32) =   0.00123326 ;
Time (1,1,64) =   0.00295474 ;
Time (1,1,128) =    0.0398633 ;
Time (1,1,160) =    0.0254475 ;
Time (2,1,1) =    0.0218718 ;
Time (2,1,2) =    0.0105324 ;
Time (2,1,4) =   0.00592162 ;
Time (2,1,8) =   0.00471864 ;
Time (2,1,16) =   0.00409035 ;
Time (2,1,32) =   0.00348373 ;
Time (2,1,64) =    0.0050395 ;
Time (2,1,128) =   0.00691579 ;
Time (2,1,160) =   0.00590267 ;
Time (3,1,1) =    0.0267496 ;
Time (3,1,2) =    0.0157303 ;
Time (3,1,4) =    0.0084915 ;
Time (3,1,8) =   0.00885486 ;
Time (3,1,16) =   0.00871259 ;
Time (3,1,32) =   0.00904184 ;
Time (3,1,64) =   0.00837413 ;
Time (3,1,128) =   0.00856273 ;
Time (3,1,160) =   0.00884052 ;
Time (4,1,1) =   0.00882991 ;
Time (4,1,2) =   0.00480266 ;
Time (4,1,4) =   0.00288757 ;
Time (4,1,8) =   0.00238548 ;
Time (4,1,16) =   0.00159687 ;
Time (4,1,32) =   0.00214231 ;
Time (4,1,64) =   0.00265852 ;
Time (4,1,128) =   0.00385984 ;
Time (4,1,160) =   0.00386408 ;
Time (5,1,1) =    0.0114878 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010421/oregon2_010421_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11080 ;
Nedges (id) = 31538 ;
Ntri (id) = 82129 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000171141 ;
T_prep (1,1,2) =  0.000169968 ;
T_prep (1,1,4) =  0.000169725 ;
T_prep (1,1,8) =  0.000183875 ;
T_prep (1,1,16) =  0.000169812 ;
T_prep (1,1,32) =  0.000169653 ;
T_prep (1,1,64) =  0.000169516 ;
T_prep (1,1,128) =  0.000169676 ;
T_prep (1,1,160) =  0.000169389 ;
T_prep (2,1,1) =  0.000193618 ;
T_prep (2,1,2) =  0.000177094 ;
T_prep (2,1,4) =  0.000174418 ;
T_prep (2,1,8) =  0.000173813 ;
T_prep (2,1,16) =  0.000173706 ;
T_prep (2,1,32) =  0.000173457 ;
T_prep (2,1,64) =  0.000173728 ;
T_prep (2,1,128) =  0.000173152 ;
T_prep (2,1,160) =  0.000173118 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000884579 ;
Time (1,1,2) =   0.00942291 ;
Time (1,1,4) =  0.000498617 ;
Time (1,1,8) =  0.000467148 ;
Time (1,1,16) =  0.000723585 ;
Time (1,1,32) =   0.00110855 ;
Time (1,1,64) =   0.00292467 ;
Time (1,1,128) =     0.020503 ;
Time (1,1,160) =    0.0274258 ;
Time (2,1,1) =    0.0209631 ;
Time (2,1,2) =    0.0104261 ;
Time (2,1,4) =   0.00389034 ;
Time (2,1,8) =   0.00377093 ;
Time (2,1,16) =   0.00322237 ;
Time (2,1,32) =   0.00371439 ;
Time (2,1,64) =   0.00377516 ;
Time (2,1,128) =   0.00312403 ;
Time (2,1,160) =     0.010658 ;
Time (3,1,1) =    0.0253546 ;
Time (3,1,2) =    0.0169396 ;
Time (3,1,4) =    0.0079208 ;
Time (3,1,8) =   0.00741956 ;
Time (3,1,16) =    0.0082895 ;
Time (3,1,32) =   0.00881751 ;
Time (3,1,64) =   0.00836011 ;
Time (3,1,128) =   0.00811982 ;
Time (3,1,160) =   0.00836943 ;
Time (4,1,1) =   0.00860077 ;
Time (4,1,2) =   0.00475336 ;
Time (4,1,4) =   0.00185986 ;
Time (4,1,8) =   0.00278683 ;
Time (4,1,16) =   0.00190534 ;
Time (4,1,32) =   0.00137561 ;
Time (4,1,64) =   0.00170594 ;
Time (4,1,128) =   0.00134819 ;
Time (4,1,160) =   0.00133381 ;
Time (5,1,1) =    0.0108792 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010428/oregon2_010428_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11113 ;
Nedges (id) = 31434 ;
Ntri (id) = 78000 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000171271 ;
T_prep (1,1,2) =  0.000169949 ;
T_prep (1,1,4) =  0.000169865 ;
T_prep (1,1,8) =  0.000169836 ;
T_prep (1,1,16) =  0.000169783 ;
T_prep (1,1,32) =  0.000169635 ;
T_prep (1,1,64) =  0.000169672 ;
T_prep (1,1,128) =  0.000169494 ;
T_prep (1,1,160) =   0.00016949 ;
T_prep (2,1,1) =  0.000192249 ;
T_prep (2,1,2) =  0.000176405 ;
T_prep (2,1,4) =  0.000174227 ;
T_prep (2,1,8) =  0.000173506 ;
T_prep (2,1,16) =   0.00017283 ;
T_prep (2,1,32) =  0.000172783 ;
T_prep (2,1,64) =  0.000172665 ;
T_prep (2,1,128) =  0.000172763 ;
T_prep (2,1,160) =   0.00017294 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000880569 ;
Time (1,1,2) =   0.00948031 ;
Time (1,1,4) =  0.000494665 ;
Time (1,1,8) =  0.000446577 ;
Time (1,1,16) =  0.000833265 ;
Time (1,1,32) =   0.00117206 ;
Time (1,1,64) =   0.00295904 ;
Time (1,1,128) =    0.0193312 ;
Time (1,1,160) =    0.0266321 ;
Time (2,1,1) =     0.020217 ;
Time (2,1,2) =    0.0102919 ;
Time (2,1,4) =   0.00554041 ;
Time (2,1,8) =   0.00449956 ;
Time (2,1,16) =     0.004233 ;
Time (2,1,32) =   0.00362584 ;
Time (2,1,64) =   0.00349236 ;
Time (2,1,128) =   0.00399071 ;
Time (2,1,160) =   0.00582299 ;
Time (3,1,1) =     0.023148 ;
Time (3,1,2) =    0.0138449 ;
Time (3,1,4) =   0.00631935 ;
Time (3,1,8) =   0.00689862 ;
Time (3,1,16) =   0.00670375 ;
Time (3,1,32) =    0.0067022 ;
Time (3,1,64) =   0.00608013 ;
Time (3,1,128) =    0.0121127 ;
Time (3,1,160) =    0.0065281 ;
Time (4,1,1) =   0.00798117 ;
Time (4,1,2) =    0.0040771 ;
Time (4,1,4) =   0.00152738 ;
Time (4,1,8) =   0.00168208 ;
Time (4,1,16) =   0.00160549 ;
Time (4,1,32) =   0.00225738 ;
Time (4,1,64) =    0.0015384 ;
Time (4,1,128) =   0.00433927 ;
Time (4,1,160) =   0.00119221 ;
Time (5,1,1) =    0.0105367 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010512/oregon2_010512_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11260 ;
Nedges (id) = 31303 ;
Ntri (id) = 72866 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000171908 ;
T_prep (1,1,2) =  0.000170862 ;
T_prep (1,1,4) =   0.00017064 ;
T_prep (1,1,8) =  0.000170517 ;
T_prep (1,1,16) =   0.00017029 ;
T_prep (1,1,32) =  0.000170389 ;
T_prep (1,1,64) =  0.000170466 ;
T_prep (1,1,128) =  0.000170552 ;
T_prep (1,1,160) =  0.000170236 ;
T_prep (2,1,1) =  0.000193011 ;
T_prep (2,1,2) =  0.000177228 ;
T_prep (2,1,4) =  0.000175599 ;
T_prep (2,1,8) =  0.000190336 ;
T_prep (2,1,16) =  0.000174131 ;
T_prep (2,1,32) =  0.000174076 ;
T_prep (2,1,64) =  0.000173593 ;
T_prep (2,1,128) =  0.000173569 ;
T_prep (2,1,160) =  0.000173513 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000870958 ;
Time (1,1,2) =   0.00946862 ;
Time (1,1,4) =  0.000471053 ;
Time (1,1,8) =  0.000402197 ;
Time (1,1,16) =  0.000617175 ;
Time (1,1,32) =   0.00536296 ;
Time (1,1,64) =   0.00285578 ;
Time (1,1,128) =    0.0309107 ;
Time (1,1,160) =    0.0282562 ;
Time (2,1,1) =    0.0220082 ;
Time (2,1,2) =    0.0110237 ;
Time (2,1,4) =   0.00650462 ;
Time (2,1,8) =   0.00484267 ;
Time (2,1,16) =   0.00473981 ;
Time (2,1,32) =   0.00382244 ;
Time (2,1,64) =   0.00378084 ;
Time (2,1,128) =   0.00363728 ;
Time (2,1,160) =   0.00306682 ;
Time (3,1,1) =    0.0245366 ;
Time (3,1,2) =     0.012503 ;
Time (3,1,4) =   0.00641597 ;
Time (3,1,8) =   0.00831552 ;
Time (3,1,16) =   0.00766958 ;
Time (3,1,32) =   0.00709169 ;
Time (3,1,64) =   0.00743814 ;
Time (3,1,128) =    0.0073171 ;
Time (3,1,160) =   0.00971643 ;
Time (4,1,1) =   0.00887151 ;
Time (4,1,2) =   0.00470873 ;
Time (4,1,4) =   0.00246243 ;
Time (4,1,8) =   0.00185551 ;
Time (4,1,16) =   0.00192146 ;
Time (4,1,32) =   0.00153341 ;
Time (4,1,64) =   0.00209082 ;
Time (4,1,128) =   0.00404364 ;
Time (4,1,160) =   0.00353393 ;
Time (5,1,1) =    0.0120002 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010519/oregon2_010519_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11375 ;
Nedges (id) = 32287 ;
Ntri (id) = 83709 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000176271 ;
T_prep (1,1,2) =  0.000175152 ;
T_prep (1,1,4) =  0.000174696 ;
T_prep (1,1,8) =  0.000174623 ;
T_prep (1,1,16) =  0.000174531 ;
T_prep (1,1,32) =  0.000174546 ;
T_prep (1,1,64) =  0.000174527 ;
T_prep (1,1,128) =  0.000174822 ;
T_prep (1,1,160) =  0.000174407 ;
T_prep (2,1,1) =  0.000207869 ;
T_prep (2,1,2) =  0.000180994 ;
T_prep (2,1,4) =    0.0001789 ;
T_prep (2,1,8) =   0.00017791 ;
T_prep (2,1,16) =  0.000177625 ;
T_prep (2,1,32) =  0.000177428 ;
T_prep (2,1,64) =  0.000177829 ;
T_prep (2,1,128) =  0.000176976 ;
T_prep (2,1,160) =  0.000177377 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000910001 ;
Time (1,1,2) =   0.00977724 ;
Time (1,1,4) =   0.00037495 ;
Time (1,1,8) =  0.000433511 ;
Time (1,1,16) =  0.000646189 ;
Time (1,1,32) =    0.0010354 ;
Time (1,1,64) =   0.00277671 ;
Time (1,1,128) =    0.0184143 ;
Time (1,1,160) =    0.0312122 ;
Time (2,1,1) =    0.0216101 ;
Time (2,1,2) =    0.0104504 ;
Time (2,1,4) =   0.00562688 ;
Time (2,1,8) =   0.00403493 ;
Time (2,1,16) =   0.00358813 ;
Time (2,1,32) =   0.00340939 ;
Time (2,1,64) =   0.00379252 ;
Time (2,1,128) =   0.00350452 ;
Time (2,1,160) =   0.00269552 ;
Time (3,1,1) =    0.0257341 ;
Time (3,1,2) =    0.0151985 ;
Time (3,1,4) =    0.0108603 ;
Time (3,1,8) =   0.00958721 ;
Time (3,1,16) =   0.00928702 ;
Time (3,1,32) =     0.010591 ;
Time (3,1,64) =   0.00990048 ;
Time (3,1,128) =   0.00983436 ;
Time (3,1,160) =    0.0269313 ;
Time (4,1,1) =   0.00874508 ;
Time (4,1,2) =   0.00463323 ;
Time (4,1,4) =   0.00263788 ;
Time (4,1,8) =   0.00176551 ;
Time (4,1,16) =   0.00161632 ;
Time (4,1,32) =   0.00165003 ;
Time (4,1,64) =   0.00237165 ;
Time (4,1,128) =   0.00155849 ;
Time (4,1,160) =   0.00200559 ;
Time (5,1,1) =    0.0107353 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41809 ;
Ntri (id) = 20736 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000120796 ;
T_prep (1,1,2) =  0.000120828 ;
T_prep (1,1,4) =    0.0001211 ;
T_prep (1,1,8) =  0.000120702 ;
T_prep (1,1,16) =  0.000120469 ;
T_prep (1,1,32) =  0.000121045 ;
T_prep (1,1,64) =  0.000120996 ;
T_prep (1,1,128) =  0.000121135 ;
T_prep (1,1,160) =  0.000121161 ;
T_prep (2,1,1) =  0.000129779 ;
T_prep (2,1,2) =  0.000118886 ;
T_prep (2,1,4) =  0.000118803 ;
T_prep (2,1,8) =  0.000118704 ;
T_prep (2,1,16) =  0.000118854 ;
T_prep (2,1,32) =  0.000118759 ;
T_prep (2,1,64) =  0.000118838 ;
T_prep (2,1,128) =  0.000118826 ;
T_prep (2,1,160) =  0.000118715 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000210664 ;
Time (1,1,2) =    0.0090926 ;
Time (1,1,4) =  0.000214902 ;
Time (1,1,8) =   0.00026321 ;
Time (1,1,16) =  0.000442507 ;
Time (1,1,32) =  0.000948967 ;
Time (1,1,64) =   0.00295191 ;
Time (1,1,128) =    0.0254391 ;
Time (1,1,160) =    0.0269304 ;
Time (2,1,1) =   0.00262169 ;
Time (2,1,2) =   0.00257046 ;
Time (2,1,4) =   0.00235588 ;
Time (2,1,8) =   0.00212092 ;
Time (2,1,16) =   0.00239376 ;
Time (2,1,32) =   0.00230983 ;
Time (2,1,64) =   0.00226846 ;
Time (2,1,128) =   0.00224119 ;
Time (2,1,160) =   0.00594837 ;
Time (3,1,1) =   0.00208629 ;
Time (3,1,2) =  0.000934997 ;
Time (3,1,4) =  0.000486557 ;
Time (3,1,8) =  0.000350992 ;
Time (3,1,16) =  0.000213012 ;
Time (3,1,32) =  0.000142281 ;
Time (3,1,64) =  0.000150441 ;
Time (3,1,128) =  0.000188407 ;
Time (3,1,160) =  0.000252633 ;
Time (4,1,1) =   0.00244024 ;
Time (4,1,2) =   0.00187261 ;
Time (4,1,4) =   0.00194439 ;
Time (4,1,8) =    0.0019579 ;
Time (4,1,16) =   0.00240749 ;
Time (4,1,32) =   0.00169829 ;
Time (4,1,64) =   0.00175439 ;
Time (4,1,128) =   0.00172813 ;
Time (4,1,160) =   0.00159424 ;
Time (5,1,1) =   0.00168678 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41472 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00012039 ;
T_prep (1,1,2) =  0.000120426 ;
T_prep (1,1,4) =  0.000120278 ;
T_prep (1,1,8) =  0.000120268 ;
T_prep (1,1,16) =  0.000120258 ;
T_prep (1,1,32) =  0.000120264 ;
T_prep (1,1,64) =  0.000120255 ;
T_prep (1,1,128) =  0.000120003 ;
T_prep (1,1,160) =  0.000120301 ;
T_prep (2,1,1) =  0.000122379 ;
T_prep (2,1,2) =  0.000118074 ;
T_prep (2,1,4) =  0.000118054 ;
T_prep (2,1,8) =  0.000118128 ;
T_prep (2,1,16) =  0.000118067 ;
T_prep (2,1,32) =  0.000118017 ;
T_prep (2,1,64) =   0.00011801 ;
T_prep (2,1,128) =  0.000117968 ;
T_prep (2,1,160) =  0.000118043 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000188623 ;
Time (1,1,2) =   0.00904084 ;
Time (1,1,4) =  0.000188201 ;
Time (1,1,8) =  0.000279563 ;
Time (1,1,16) =  0.000469705 ;
Time (1,1,32) =  0.000836369 ;
Time (1,1,64) =   0.00230731 ;
Time (1,1,128) =    0.0317676 ;
Time (1,1,160) =    0.0356432 ;
Time (2,1,1) =   0.00146089 ;
Time (2,1,2) =   0.00109771 ;
Time (2,1,4) =   0.00119633 ;
Time (2,1,8) =   0.00127852 ;
Time (2,1,16) =   0.00126629 ;
Time (2,1,32) =   0.00137385 ;
Time (2,1,64) =   0.00124677 ;
Time (2,1,128) =   0.00668605 ;
Time (2,1,160) =    0.0011851 ;
Time (3,1,1) =   0.00137607 ;
Time (3,1,2) =  0.000635443 ;
Time (3,1,4) =  0.000422167 ;
Time (3,1,8) =  0.000755183 ;
Time (3,1,16) =  0.000142953 ;
Time (3,1,32) =  0.000983839 ;
Time (3,1,64) =  0.000134571 ;
Time (3,1,128) =   0.00823622 ;
Time (3,1,160) =  0.000163037 ;
Time (4,1,1) =   0.00180425 ;
Time (4,1,2) =    0.0011129 ;
Time (4,1,4) =   0.00184288 ;
Time (4,1,8) =   0.00192283 ;
Time (4,1,16) =    0.0012636 ;
Time (4,1,32) =   0.00128603 ;
Time (4,1,64) =   0.00131422 ;
Time (4,1,128) =    0.0012917 ;
Time (4,1,160) =   0.00240228 ;
Time (5,1,1) =   0.00156341 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/oregon2_010526/oregon2_010526_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 11461 ;
Nedges (id) = 32730 ;
Ntri (id) = 89541 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000178303 ;
T_prep (1,1,2) =  0.000177409 ;
T_prep (1,1,4) =  0.000177031 ;
T_prep (1,1,8) =  0.000176818 ;
T_prep (1,1,16) =  0.000177027 ;
T_prep (1,1,32) =  0.000176976 ;
T_prep (1,1,64) =  0.000177045 ;
T_prep (1,1,128) =  0.000177227 ;
T_prep (1,1,160) =  0.000176967 ;
T_prep (2,1,1) =  0.000209097 ;
T_prep (2,1,2) =  0.000182847 ;
T_prep (2,1,4) =  0.000180446 ;
T_prep (2,1,8) =  0.000179657 ;
T_prep (2,1,16) =  0.000179456 ;
T_prep (2,1,32) =  0.000178985 ;
T_prep (2,1,64) =   0.00017901 ;
T_prep (2,1,128) =   0.00020508 ;
T_prep (2,1,160) =    0.0001792 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000931061 ;
Time (1,1,2) =   0.00950188 ;
Time (1,1,4) =  0.000478115 ;
Time (1,1,8) =  0.000408303 ;
Time (1,1,16) =   0.00081968 ;
Time (1,1,32) =   0.00109977 ;
Time (1,1,64) =    0.0055472 ;
Time (1,1,128) =    0.0280586 ;
Time (1,1,160) =    0.0291222 ;
Time (2,1,1) =    0.0236844 ;
Time (2,1,2) =    0.0112337 ;
Time (2,1,4) =   0.00510583 ;
Time (2,1,8) =   0.00517437 ;
Time (2,1,16) =   0.00418978 ;
Time (2,1,32) =   0.00373576 ;
Time (2,1,64) =   0.00387719 ;
Time (2,1,128) =    0.0104637 ;
Time (2,1,160) =   0.00405602 ;
Time (3,1,1) =    0.0267573 ;
Time (3,1,2) =    0.0183784 ;
Time (3,1,4) =    0.0105624 ;
Time (3,1,8) =    0.0109993 ;
Time (3,1,16) =    0.0103395 ;
Time (3,1,32) =    0.0103577 ;
Time (3,1,64) =    0.0102108 ;
Time (3,1,128) =    0.0100251 ;
Time (3,1,160) =    0.0198281 ;
Time (4,1,1) =   0.00971643 ;
Time (4,1,2) =   0.00499201 ;
Time (4,1,4) =   0.00280402 ;
Time (4,1,8) =   0.00153343 ;
Time (4,1,16) =   0.00162121 ;
Time (4,1,32) =    0.0018638 ;
Time (4,1,64) =   0.00134336 ;
Time (4,1,128) =   0.00220064 ;
Time (4,1,160) =   0.00112222 ;
Time (5,1,1) =     0.011772 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella06/p2p-Gnutella06_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8717 ;
Nedges (id) = 31525 ;
Ntri (id) = 1142 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000179921 ;
T_prep (1,1,2) =  0.000178939 ;
T_prep (1,1,4) =  0.000178853 ;
T_prep (1,1,8) =  0.000178345 ;
T_prep (1,1,16) =  0.000189746 ;
T_prep (1,1,32) =  0.000178494 ;
T_prep (1,1,64) =  0.000178576 ;
T_prep (1,1,128) =  0.000178391 ;
T_prep (1,1,160) =  0.000178251 ;
T_prep (2,1,1) =  0.000201158 ;
T_prep (2,1,2) =  0.000192076 ;
T_prep (2,1,4) =  0.000189854 ;
T_prep (2,1,8) =  0.000189209 ;
T_prep (2,1,16) =  0.000188898 ;
T_prep (2,1,32) =  0.000189007 ;
T_prep (2,1,64) =  0.000188449 ;
T_prep (2,1,128) =  0.000188605 ;
T_prep (2,1,160) =   0.00018846 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000580595 ;
Time (1,1,2) =   0.00921315 ;
Time (1,1,4) =   0.00023606 ;
Time (1,1,8) =  0.000297244 ;
Time (1,1,16) =  0.000542871 ;
Time (1,1,32) =  0.000885521 ;
Time (1,1,64) =   0.00218204 ;
Time (1,1,128) =    0.0304604 ;
Time (1,1,160) =    0.0310084 ;
Time (2,1,1) =   0.00509946 ;
Time (2,1,2) =   0.00238337 ;
Time (2,1,4) =   0.00152292 ;
Time (2,1,8) =   0.00164444 ;
Time (2,1,16) =   0.00187091 ;
Time (2,1,32) =   0.00155102 ;
Time (2,1,64) =   0.00142214 ;
Time (2,1,128) =   0.00194836 ;
Time (2,1,160) =    0.0025891 ;
Time (3,1,1) =   0.00889994 ;
Time (3,1,2) =   0.00495169 ;
Time (3,1,4) =   0.00263802 ;
Time (3,1,8) =   0.00160781 ;
Time (3,1,16) =   0.00166646 ;
Time (3,1,32) =   0.00143147 ;
Time (3,1,64) =   0.00154573 ;
Time (3,1,128) =   0.00153058 ;
Time (3,1,160) =   0.00152542 ;
Time (4,1,1) =   0.00356511 ;
Time (4,1,2) =   0.00200898 ;
Time (4,1,4) =   0.00125496 ;
Time (4,1,8) =   0.00121969 ;
Time (4,1,16) =   0.00120147 ;
Time (4,1,32) =   0.00112144 ;
Time (4,1,64) =   0.00103366 ;
Time (4,1,128) =  0.000674531 ;
Time (4,1,160) =   0.00183643 ;
Time (5,1,1) =   0.00302654 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-81-256-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 21074 ;
Nedges (id) = 41809 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000120887 ;
T_prep (1,1,2) =  0.000120565 ;
T_prep (1,1,4) =   0.00012041 ;
T_prep (1,1,8) =  0.000120571 ;
T_prep (1,1,16) =  0.000120824 ;
T_prep (1,1,32) =  0.000120682 ;
T_prep (1,1,64) =  0.000120824 ;
T_prep (1,1,128) =  0.000120464 ;
T_prep (1,1,160) =  0.000120182 ;
T_prep (2,1,1) =  0.000124546 ;
T_prep (2,1,2) =   0.00011897 ;
T_prep (2,1,4) =  0.000118979 ;
T_prep (2,1,8) =  0.000118989 ;
T_prep (2,1,16) =  0.000119094 ;
T_prep (2,1,32) =  0.000119052 ;
T_prep (2,1,64) =     0.000119 ;
T_prep (2,1,128) =   0.00011903 ;
T_prep (2,1,160) =    0.0001189 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000190414 ;
Time (1,1,2) =   0.00908849 ;
Time (1,1,4) =  0.000184359 ;
Time (1,1,8) =  0.000257991 ;
Time (1,1,16) =  0.000398402 ;
Time (1,1,32) =  0.000996896 ;
Time (1,1,64) =   0.00266598 ;
Time (1,1,128) =    0.0276316 ;
Time (1,1,160) =    0.0219178 ;
Time (2,1,1) =   0.00152708 ;
Time (2,1,2) =   0.00125988 ;
Time (2,1,4) =   0.00209721 ;
Time (2,1,8) =   0.00114272 ;
Time (2,1,16) =   0.00144227 ;
Time (2,1,32) =   0.00202294 ;
Time (2,1,64) =   0.00289852 ;
Time (2,1,128) =   0.00129471 ;
Time (2,1,160) =   0.00121508 ;
Time (3,1,1) =   0.00147474 ;
Time (3,1,2) =  0.000805697 ;
Time (3,1,4) =  0.000445419 ;
Time (3,1,8) =  0.000369333 ;
Time (3,1,16) =  0.000214524 ;
Time (3,1,32) =  0.000777401 ;
Time (3,1,64) =   0.00111797 ;
Time (3,1,128) =  0.000565605 ;
Time (3,1,160) =   0.00330862 ;
Time (4,1,1) =   0.00201808 ;
Time (4,1,2) =   0.00141548 ;
Time (4,1,4) =    0.0015108 ;
Time (4,1,8) =   0.00144282 ;
Time (4,1,16) =   0.00143675 ;
Time (4,1,32) =  0.000324848 ;
Time (4,1,64) =  0.000373287 ;
Time (4,1,128) =  0.000341339 ;
Time (4,1,160) =  0.000348543 ;
Time (5,1,1) =   0.00180395 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella05/p2p-Gnutella05_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8846 ;
Nedges (id) = 31839 ;
Ntri (id) = 1112 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000184205 ;
T_prep (1,1,2) =  0.000183185 ;
T_prep (1,1,4) =   0.00018266 ;
T_prep (1,1,8) =  0.000182641 ;
T_prep (1,1,16) =  0.000182424 ;
T_prep (1,1,32) =  0.000182525 ;
T_prep (1,1,64) =  0.000182481 ;
T_prep (1,1,128) =  0.000182375 ;
T_prep (1,1,160) =  0.000182703 ;
T_prep (2,1,1) =  0.000213094 ;
T_prep (2,1,2) =   0.00019657 ;
T_prep (2,1,4) =  0.000193707 ;
T_prep (2,1,8) =  0.000193322 ;
T_prep (2,1,16) =  0.000193134 ;
T_prep (2,1,32) =   0.00019305 ;
T_prep (2,1,64) =  0.000192402 ;
T_prep (2,1,128) =  0.000193142 ;
T_prep (2,1,160) =  0.000192759 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000587787 ;
Time (1,1,2) =   0.00924973 ;
Time (1,1,4) =  0.000238476 ;
Time (1,1,8) =  0.000342862 ;
Time (1,1,16) =  0.000476011 ;
Time (1,1,32) =   0.00113011 ;
Time (1,1,64) =   0.00220635 ;
Time (1,1,128) =    0.0285536 ;
Time (1,1,160) =    0.0340789 ;
Time (2,1,1) =   0.00511054 ;
Time (2,1,2) =    0.0036305 ;
Time (2,1,4) =   0.00258346 ;
Time (2,1,8) =     0.001539 ;
Time (2,1,16) =   0.00135644 ;
Time (2,1,32) =   0.00200555 ;
Time (2,1,64) =   0.00160364 ;
Time (2,1,128) =    0.0012599 ;
Time (2,1,160) =    0.0127155 ;
Time (3,1,1) =   0.00918591 ;
Time (3,1,2) =   0.00581376 ;
Time (3,1,4) =   0.00377924 ;
Time (3,1,8) =   0.00205354 ;
Time (3,1,16) =   0.00173428 ;
Time (3,1,32) =   0.00155601 ;
Time (3,1,64) =   0.00167857 ;
Time (3,1,128) =   0.00162103 ;
Time (3,1,160) =   0.00202461 ;
Time (4,1,1) =   0.00388794 ;
Time (4,1,2) =   0.00218362 ;
Time (4,1,4) =    0.0017431 ;
Time (4,1,8) =    0.0012986 ;
Time (4,1,16) =   0.00104575 ;
Time (4,1,32) =   0.00107851 ;
Time (4,1,64) =    0.0010285 ;
Time (4,1,128) =  0.000755638 ;
Time (4,1,160) =    0.0016336 ;
Time (5,1,1) =   0.00305762 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella04/p2p-Gnutella04_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 10876 ;
Nedges (id) = 39994 ;
Ntri (id) = 934 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000225267 ;
T_prep (1,1,2) =  0.000245297 ;
T_prep (1,1,4) =  0.000224601 ;
T_prep (1,1,8) =  0.000224246 ;
T_prep (1,1,16) =  0.000224034 ;
T_prep (1,1,32) =  0.000224217 ;
T_prep (1,1,64) =  0.000224113 ;
T_prep (1,1,128) =  0.000224317 ;
T_prep (1,1,160) =  0.000224167 ;
T_prep (2,1,1) =  0.000265578 ;
T_prep (2,1,2) =  0.000242325 ;
T_prep (2,1,4) =  0.000240029 ;
T_prep (2,1,8) =  0.000239884 ;
T_prep (2,1,16) =  0.000239119 ;
T_prep (2,1,32) =  0.000239447 ;
T_prep (2,1,64) =   0.00023903 ;
T_prep (2,1,128) =  0.000239088 ;
T_prep (2,1,160) =   0.00023905 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000738315 ;
Time (1,1,2) =   0.00935239 ;
Time (1,1,4) =   0.00025834 ;
Time (1,1,8) =  0.000308931 ;
Time (1,1,16) =  0.000676493 ;
Time (1,1,32) =  0.000988754 ;
Time (1,1,64) =   0.00242505 ;
Time (1,1,128) =    0.0269577 ;
Time (1,1,160) =    0.0346429 ;
Time (2,1,1) =   0.00626637 ;
Time (2,1,2) =   0.00317099 ;
Time (2,1,4) =   0.00195209 ;
Time (2,1,8) =   0.00169595 ;
Time (2,1,16) =   0.00151571 ;
Time (2,1,32) =   0.00161151 ;
Time (2,1,64) =   0.00237588 ;
Time (2,1,128) =    0.0013933 ;
Time (2,1,160) =   0.00248443 ;
Time (3,1,1) =    0.0114529 ;
Time (3,1,2) =   0.00636723 ;
Time (3,1,4) =   0.00379927 ;
Time (3,1,8) =   0.00201698 ;
Time (3,1,16) =   0.00172076 ;
Time (3,1,32) =   0.00152113 ;
Time (3,1,64) =   0.00158647 ;
Time (3,1,128) =   0.00285003 ;
Time (3,1,160) =   0.00594406 ;
Time (4,1,1) =   0.00424738 ;
Time (4,1,2) =   0.00238162 ;
Time (4,1,4) =   0.00133387 ;
Time (4,1,8) =   0.00174415 ;
Time (4,1,16) =   0.00176503 ;
Time (4,1,32) =   0.00116944 ;
Time (4,1,64) =   0.00234182 ;
Time (4,1,128) =   0.00132748 ;
Time (4,1,160) =   0.00104931 ;
Time (5,1,1) =   0.00373978 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/as-caida20071105/as-caida20071105_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26475 ;
Nedges (id) = 53381 ;
Ntri (id) = 36365 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000367511 ;
T_prep (1,1,2) =  0.000367022 ;
T_prep (1,1,4) =  0.000366881 ;
T_prep (1,1,8) =  0.000366622 ;
T_prep (1,1,16) =  0.000366528 ;
T_prep (1,1,32) =  0.000366503 ;
T_prep (1,1,64) =  0.000367033 ;
T_prep (1,1,128) =  0.000367393 ;
T_prep (1,1,160) =    0.0003667 ;
T_prep (2,1,1) =  0.000391889 ;
T_prep (2,1,2) =  0.000372256 ;
T_prep (2,1,4) =   0.00037139 ;
T_prep (2,1,8) =  0.000369865 ;
T_prep (2,1,16) =  0.000369143 ;
T_prep (2,1,32) =  0.000369653 ;
T_prep (2,1,64) =  0.000369513 ;
T_prep (2,1,128) =  0.000369417 ;
T_prep (2,1,160) =  0.000369757 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00171801 ;
Time (1,1,2) =   0.00999531 ;
Time (1,1,4) =  0.000590605 ;
Time (1,1,8) =  0.000479912 ;
Time (1,1,16) =  0.000684295 ;
Time (1,1,32) =   0.00107088 ;
Time (1,1,64) =     0.003079 ;
Time (1,1,128) =    0.0242926 ;
Time (1,1,160) =    0.0219589 ;
Time (2,1,1) =    0.0415466 ;
Time (2,1,2) =    0.0195466 ;
Time (2,1,4) =    0.0100047 ;
Time (2,1,8) =    0.0068534 ;
Time (2,1,16) =   0.00676529 ;
Time (2,1,32) =   0.00736322 ;
Time (2,1,64) =    0.0119468 ;
Time (2,1,128) =    0.0153803 ;
Time (2,1,160) =   0.00610235 ;
Time (3,1,1) =    0.0349168 ;
Time (3,1,2) =    0.0194731 ;
Time (3,1,4) =    0.0155679 ;
Time (3,1,8) =    0.0130288 ;
Time (3,1,16) =    0.0117421 ;
Time (3,1,32) =    0.0117974 ;
Time (3,1,64) =    0.0118304 ;
Time (3,1,128) =    0.0113565 ;
Time (3,1,160) =    0.0117218 ;
Time (4,1,1) =    0.0169701 ;
Time (4,1,2) =   0.00935701 ;
Time (4,1,4) =   0.00435663 ;
Time (4,1,8) =   0.00329404 ;
Time (4,1,16) =   0.00289077 ;
Time (4,1,32) =   0.00279935 ;
Time (4,1,64) =   0.00240775 ;
Time (4,1,128) =   0.00530828 ;
Time (4,1,160) =   0.00513936 ;
Time (5,1,1) =    0.0217135 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella25/p2p-Gnutella25_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 22687 ;
Nedges (id) = 54705 ;
Ntri (id) = 806 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000377204 ;
T_prep (1,1,2) =  0.000376525 ;
T_prep (1,1,4) =   0.00037625 ;
T_prep (1,1,8) =  0.000375981 ;
T_prep (1,1,16) =  0.000389247 ;
T_prep (1,1,32) =  0.000376432 ;
T_prep (1,1,64) =  0.000376061 ;
T_prep (1,1,128) =  0.000376179 ;
T_prep (1,1,160) =   0.00037615 ;
T_prep (2,1,1) =  0.000417351 ;
T_prep (2,1,2) =  0.000388382 ;
T_prep (2,1,4) =  0.000386499 ;
T_prep (2,1,8) =    0.0003855 ;
T_prep (2,1,16) =  0.000385361 ;
T_prep (2,1,32) =  0.000385058 ;
T_prep (2,1,64) =  0.000384953 ;
T_prep (2,1,128) =  0.000385146 ;
T_prep (2,1,160) =  0.000384722 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00101027 ;
Time (1,1,2) =   0.00947816 ;
Time (1,1,4) =  0.000357109 ;
Time (1,1,8) =  0.000420543 ;
Time (1,1,16) =  0.000473808 ;
Time (1,1,32) =  0.000828837 ;
Time (1,1,64) =   0.00211874 ;
Time (1,1,128) =    0.0308929 ;
Time (1,1,160) =    0.0262121 ;
Time (2,1,1) =   0.00823868 ;
Time (2,1,2) =   0.00381185 ;
Time (2,1,4) =   0.00218682 ;
Time (2,1,8) =    0.0012325 ;
Time (2,1,16) =   0.00137101 ;
Time (2,1,32) =   0.00151522 ;
Time (2,1,64) =  0.000854705 ;
Time (2,1,128) =    0.0036583 ;
Time (2,1,160) =   0.00090826 ;
Time (3,1,1) =    0.0110603 ;
Time (3,1,2) =   0.00562847 ;
Time (3,1,4) =   0.00327076 ;
Time (3,1,8) =   0.00171774 ;
Time (3,1,16) =   0.00122981 ;
Time (3,1,32) =   0.00120732 ;
Time (3,1,64) =   0.00120101 ;
Time (3,1,128) =   0.00151928 ;
Time (3,1,160) =   0.00121734 ;
Time (4,1,1) =   0.00594897 ;
Time (4,1,2) =   0.00316362 ;
Time (4,1,4) =   0.00206013 ;
Time (4,1,8) =   0.00210705 ;
Time (4,1,16) =   0.00166395 ;
Time (4,1,32) =  0.000989598 ;
Time (4,1,64) =   0.00126387 ;
Time (4,1,128) =   0.00119675 ;
Time (4,1,160) =  0.000981988 ;
Time (5,1,1) =   0.00524938 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella24/p2p-Gnutella24_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26518 ;
Nedges (id) = 65369 ;
Ntri (id) = 986 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000443714 ;
T_prep (1,1,2) =  0.000443027 ;
T_prep (1,1,4) =  0.000442847 ;
T_prep (1,1,8) =  0.000442806 ;
T_prep (1,1,16) =  0.000442584 ;
T_prep (1,1,32) =  0.000442646 ;
T_prep (1,1,64) =  0.000442617 ;
T_prep (1,1,128) =   0.00044271 ;
T_prep (1,1,160) =  0.000442348 ;
T_prep (2,1,1) =  0.000509535 ;
T_prep (2,1,2) =  0.000461338 ;
T_prep (2,1,4) =  0.000458443 ;
T_prep (2,1,8) =  0.000458308 ;
T_prep (2,1,16) =   0.00045778 ;
T_prep (2,1,32) =  0.000458367 ;
T_prep (2,1,64) =  0.000457896 ;
T_prep (2,1,128) =  0.000457679 ;
T_prep (2,1,160) =  0.000472132 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00124186 ;
Time (1,1,2) =   0.00984945 ;
Time (1,1,4) =  0.000434584 ;
Time (1,1,8) =  0.000361574 ;
Time (1,1,16) =  0.000514667 ;
Time (1,1,32) =   0.00103696 ;
Time (1,1,64) =    0.0020924 ;
Time (1,1,128) =    0.0239913 ;
Time (1,1,160) =    0.0278996 ;
Time (2,1,1) =    0.0100829 ;
Time (2,1,2) =   0.00533889 ;
Time (2,1,4) =   0.00437604 ;
Time (2,1,8) =   0.00224794 ;
Time (2,1,16) =    0.0018535 ;
Time (2,1,32) =  0.000876923 ;
Time (2,1,64) =   0.00146854 ;
Time (2,1,128) =   0.00103543 ;
Time (2,1,160) =  0.000914339 ;
Time (3,1,1) =    0.0151175 ;
Time (3,1,2) =   0.00803091 ;
Time (3,1,4) =   0.00389676 ;
Time (3,1,8) =   0.00236116 ;
Time (3,1,16) =    0.0017134 ;
Time (3,1,32) =   0.00141511 ;
Time (3,1,64) =   0.00356582 ;
Time (3,1,128) =   0.00142173 ;
Time (3,1,160) =   0.00138485 ;
Time (4,1,1) =   0.00824049 ;
Time (4,1,2) =   0.00422231 ;
Time (4,1,4) =   0.00214093 ;
Time (4,1,8) =   0.00127134 ;
Time (4,1,16) =   0.00094713 ;
Time (4,1,32) =   0.00103457 ;
Time (4,1,64) =   0.00130711 ;
Time (4,1,128) =   0.00102879 ;
Time (4,1,160) =  0.000902141 ;
Time (5,1,1) =   0.00669876 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/facebook_combined/facebook_combined_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4039 ;
Nedges (id) = 88234 ;
Ntri (id) = 1612010 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000265523 ;
T_prep (1,1,2) =  0.000265254 ;
T_prep (1,1,4) =   0.00026526 ;
T_prep (1,1,8) =  0.000265122 ;
T_prep (1,1,16) =  0.000264748 ;
T_prep (1,1,32) =  0.000265052 ;
T_prep (1,1,64) =  0.000264469 ;
T_prep (1,1,128) =  0.000265031 ;
T_prep (1,1,160) =  0.000264925 ;
T_prep (2,1,1) =  0.000314935 ;
T_prep (2,1,2) =  0.000282274 ;
T_prep (2,1,4) =  0.000281829 ;
T_prep (2,1,8) =  0.000281365 ;
T_prep (2,1,16) =  0.000281621 ;
T_prep (2,1,32) =  0.000281368 ;
T_prep (2,1,64) =  0.000294363 ;
T_prep (2,1,128) =  0.000280953 ;
T_prep (2,1,160) =  0.000280908 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0022031 ;
Time (1,1,2) =    0.0102006 ;
Time (1,1,4) =   0.00117403 ;
Time (1,1,8) =   0.00182361 ;
Time (1,1,16) =   0.00197525 ;
Time (1,1,32) =   0.00248158 ;
Time (1,1,64) =   0.00625069 ;
Time (1,1,128) =     0.022547 ;
Time (1,1,160) =    0.0265766 ;
Time (2,1,1) =    0.0557559 ;
Time (2,1,2) =    0.0319277 ;
Time (2,1,4) =    0.0304471 ;
Time (2,1,8) =    0.0308343 ;
Time (2,1,16) =    0.0284096 ;
Time (2,1,32) =    0.0269902 ;
Time (2,1,64) =    0.0264932 ;
Time (2,1,128) =      0.01356 ;
Time (2,1,160) =   0.00771701 ;
Time (3,1,1) =     0.224203 ;
Time (3,1,2) =     0.113886 ;
Time (3,1,4) =    0.0989003 ;
Time (3,1,8) =    0.0972679 ;
Time (3,1,16) =    0.0927848 ;
Time (3,1,32) =    0.0959045 ;
Time (3,1,64) =    0.0931325 ;
Time (3,1,128) =     0.168556 ;
Time (3,1,160) =     0.167037 ;
Time (4,1,1) =     0.022192 ;
Time (4,1,2) =    0.0111376 ;
Time (4,1,4) =    0.0100856 ;
Time (4,1,8) =   0.00971085 ;
Time (4,1,16) =    0.0108088 ;
Time (4,1,32) =   0.00953697 ;
Time (4,1,64) =   0.00939451 ;
Time (4,1,128) =   0.00915629 ;
Time (4,1,160) =    0.0249346 ;
Time (5,1,1) =    0.0185619 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 129600 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00036823 ;
T_prep (1,1,2) =  0.000368191 ;
T_prep (1,1,4) =   0.00937388 ;
T_prep (1,1,8) =  0.000405098 ;
T_prep (1,1,16) =  0.000403746 ;
T_prep (1,1,32) =  0.000402697 ;
T_prep (1,1,64) =  0.000402489 ;
T_prep (1,1,128) =  0.000403478 ;
T_prep (1,1,160) =  0.000404513 ;
T_prep (2,1,1) =  0.000445656 ;
T_prep (2,1,2) =  0.000355367 ;
T_prep (2,1,4) =  0.000453462 ;
T_prep (2,1,8) =   0.00046098 ;
T_prep (2,1,16) =   0.00045499 ;
T_prep (2,1,32) =  0.000456706 ;
T_prep (2,1,64) =  0.000454445 ;
T_prep (2,1,128) =  0.000455216 ;
T_prep (2,1,160) =  0.000454499 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00051066 ;
Time (1,1,2) =  0.000325844 ;
Time (1,1,4) =  0.000345986 ;
Time (1,1,8) =  0.000440242 ;
Time (1,1,16) =   0.00056305 ;
Time (1,1,32) =   0.00110505 ;
Time (1,1,64) =   0.00364933 ;
Time (1,1,128) =    0.0258624 ;
Time (1,1,160) =    0.0255148 ;
Time (2,1,1) =   0.00434465 ;
Time (2,1,2) =    0.0028854 ;
Time (2,1,4) =   0.00306553 ;
Time (2,1,8) =   0.00290564 ;
Time (2,1,16) =   0.00298254 ;
Time (2,1,32) =   0.00283023 ;
Time (2,1,64) =   0.00257974 ;
Time (2,1,128) =   0.00377162 ;
Time (2,1,160) =   0.00260784 ;
Time (3,1,1) =   0.00404539 ;
Time (3,1,2) =   0.00221506 ;
Time (3,1,4) =    0.0011523 ;
Time (3,1,8) =  0.000652625 ;
Time (3,1,16) =  0.000762867 ;
Time (3,1,32) =  0.000263219 ;
Time (3,1,64) =  0.000227453 ;
Time (3,1,128) =  0.000218851 ;
Time (3,1,160) =   0.00026643 ;
Time (4,1,1) =   0.00566132 ;
Time (4,1,2) =    0.0035095 ;
Time (4,1,4) =   0.00336122 ;
Time (4,1,8) =   0.00297021 ;
Time (4,1,16) =   0.00339168 ;
Time (4,1,32) =   0.00316175 ;
Time (4,1,64) =   0.00300107 ;
Time (4,1,128) =   0.00257378 ;
Time (4,1,160) =    0.0027642 ;
Time (5,1,1) =   0.00448511 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella30/p2p-Gnutella30_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36682 ;
Nedges (id) = 88328 ;
Ntri (id) = 1590 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000594863 ;
T_prep (1,1,2) =   0.00059389 ;
T_prep (1,1,4) =  0.000593984 ;
T_prep (1,1,8) =  0.000593591 ;
T_prep (1,1,16) =  0.000593402 ;
T_prep (1,1,32) =  0.000593775 ;
T_prep (1,1,64) =  0.000593401 ;
T_prep (1,1,128) =   0.00059365 ;
T_prep (1,1,160) =  0.000593792 ;
T_prep (2,1,1) =  0.000684431 ;
T_prep (2,1,2) =  0.000616256 ;
T_prep (2,1,4) =  0.000614128 ;
T_prep (2,1,8) =  0.000612887 ;
T_prep (2,1,16) =  0.000628286 ;
T_prep (2,1,32) =  0.000612969 ;
T_prep (2,1,64) =  0.000612808 ;
T_prep (2,1,128) =  0.000612301 ;
T_prep (2,1,160) =  0.000612472 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00169235 ;
Time (1,1,2) =   0.00986216 ;
Time (1,1,4) =  0.000554292 ;
Time (1,1,8) =  0.000441224 ;
Time (1,1,16) =  0.000515244 ;
Time (1,1,32) =  0.000771963 ;
Time (1,1,64) =   0.00217297 ;
Time (1,1,128) =    0.0217169 ;
Time (1,1,160) =    0.0324124 ;
Time (2,1,1) =    0.0130759 ;
Time (2,1,2) =   0.00689507 ;
Time (2,1,4) =   0.00345826 ;
Time (2,1,8) =   0.00201495 ;
Time (2,1,16) =   0.00122776 ;
Time (2,1,32) =   0.00251254 ;
Time (2,1,64) =   0.00126247 ;
Time (2,1,128) =   0.00155573 ;
Time (2,1,160) =  0.000895554 ;
Time (3,1,1) =    0.0205472 ;
Time (3,1,2) =    0.0109576 ;
Time (3,1,4) =   0.00511548 ;
Time (3,1,8) =   0.00248852 ;
Time (3,1,16) =   0.00183295 ;
Time (3,1,32) =   0.00143476 ;
Time (3,1,64) =   0.00149804 ;
Time (3,1,128) =   0.00338305 ;
Time (3,1,160) =   0.00135803 ;
Time (4,1,1) =    0.0113848 ;
Time (4,1,2) =   0.00561827 ;
Time (4,1,4) =   0.00421041 ;
Time (4,1,8) =   0.00495749 ;
Time (4,1,16) =   0.00191928 ;
Time (4,1,32) =  0.000810179 ;
Time (4,1,64) =   0.00116757 ;
Time (4,1,128) =   0.00124557 ;
Time (4,1,160) =    0.0016139 ;
Time (5,1,1) =   0.00879016 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 137164 ;
Ntri (id) = 133321 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00036532 ;
T_prep (1,1,2) =  0.000367416 ;
T_prep (1,1,4) =   0.00971465 ;
T_prep (1,1,8) =  0.000426992 ;
T_prep (1,1,16) =  0.000428681 ;
T_prep (1,1,32) =  0.000427377 ;
T_prep (1,1,64) =  0.000429497 ;
T_prep (1,1,128) =  0.000428309 ;
T_prep (1,1,160) =  0.000428883 ;
T_prep (2,1,1) =  0.000478398 ;
T_prep (2,1,2) =  0.000372949 ;
T_prep (2,1,4) =  0.000481281 ;
T_prep (2,1,8) =  0.000481909 ;
T_prep (2,1,16) =  0.000482775 ;
T_prep (2,1,32) =  0.000482728 ;
T_prep (2,1,64) =   0.00048338 ;
T_prep (2,1,128) =  0.000482865 ;
T_prep (2,1,160) =  0.000492609 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00163495 ;
Time (1,1,2) =   0.00200997 ;
Time (1,1,4) =   0.00203523 ;
Time (1,1,8) =   0.00212126 ;
Time (1,1,16) =   0.00234165 ;
Time (1,1,32) =   0.00346095 ;
Time (1,1,64) =    0.0116086 ;
Time (1,1,128) =    0.0527467 ;
Time (1,1,160) =    0.0319122 ;
Time (2,1,1) =    0.0539609 ;
Time (2,1,2) =    0.0419443 ;
Time (2,1,4) =    0.0500297 ;
Time (2,1,8) =    0.0450407 ;
Time (2,1,16) =    0.0442866 ;
Time (2,1,32) =    0.0451311 ;
Time (2,1,64) =    0.0584752 ;
Time (2,1,128) =    0.0390898 ;
Time (2,1,160) =    0.0381418 ;
Time (3,1,1) =    0.0252422 ;
Time (3,1,2) =    0.0127114 ;
Time (3,1,4) =   0.00315347 ;
Time (3,1,8) =   0.00291267 ;
Time (3,1,16) =   0.00141555 ;
Time (3,1,32) =   0.00129584 ;
Time (3,1,64) =     0.001225 ;
Time (3,1,128) =   0.00169064 ;
Time (3,1,160) =      0.00184 ;
Time (4,1,1) =    0.0214517 ;
Time (4,1,2) =    0.0192708 ;
Time (4,1,4) =    0.0192662 ;
Time (4,1,8) =    0.0021602 ;
Time (4,1,16) =    0.0184579 ;
Time (4,1,32) =    0.0161216 ;
Time (4,1,64) =    0.0160782 ;
Time (4,1,128) =    0.0153712 ;
Time (4,1,160) =    0.0153574 ;
Time (5,1,1) =    0.0177335 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-16-25-81-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36244 ;
Nedges (id) = 137164 ;
Ntri (id) = 7 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000382893 ;
T_prep (1,1,2) =  0.000383299 ;
T_prep (1,1,4) =   0.00938519 ;
T_prep (1,1,8) =  0.000414114 ;
T_prep (1,1,16) =  0.000412685 ;
T_prep (1,1,32) =  0.000412798 ;
T_prep (1,1,64) =  0.000413902 ;
T_prep (1,1,128) =  0.000412533 ;
T_prep (1,1,160) =  0.000412472 ;
T_prep (2,1,1) =  0.000454181 ;
T_prep (2,1,2) =  0.000375146 ;
T_prep (2,1,4) =  0.000463787 ;
T_prep (2,1,8) =  0.000464064 ;
T_prep (2,1,16) =  0.000465742 ;
T_prep (2,1,32) =  0.000465346 ;
T_prep (2,1,64) =  0.000466179 ;
T_prep (2,1,128) =  0.000465891 ;
T_prep (2,1,160) =  0.000467701 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000599313 ;
Time (1,1,2) =  0.000358101 ;
Time (1,1,4) =  0.000350325 ;
Time (1,1,8) =  0.000442866 ;
Time (1,1,16) =  0.000774872 ;
Time (1,1,32) =   0.00113835 ;
Time (1,1,64) =   0.00344147 ;
Time (1,1,128) =    0.0273682 ;
Time (1,1,160) =    0.0332533 ;
Time (2,1,1) =   0.00676061 ;
Time (2,1,2) =   0.00374496 ;
Time (2,1,4) =   0.00391247 ;
Time (2,1,8) =   0.00261763 ;
Time (2,1,16) =   0.00294244 ;
Time (2,1,32) =   0.00309953 ;
Time (2,1,64) =   0.00279585 ;
Time (2,1,128) =    0.0027388 ;
Time (2,1,160) =   0.00535363 ;
Time (3,1,1) =   0.00540908 ;
Time (3,1,2) =   0.00295184 ;
Time (3,1,4) =   0.00163819 ;
Time (3,1,8) =   0.00107403 ;
Time (3,1,16) =  0.000877927 ;
Time (3,1,32) =   0.00081336 ;
Time (3,1,64) =  0.000761123 ;
Time (3,1,128) =  0.000711559 ;
Time (3,1,160) =   0.00122499 ;
Time (4,1,1) =   0.00707501 ;
Time (4,1,2) =   0.00337008 ;
Time (4,1,4) =   0.00340345 ;
Time (4,1,8) =   0.00339094 ;
Time (4,1,16) =   0.00309746 ;
Time (4,1,32) =   0.00330574 ;
Time (4,1,64) =   0.00289222 ;
Time (4,1,128) =   0.00288843 ;
Time (4,1,160) =   0.00280717 ;
Time (5,1,1) =    0.0256059 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 138240 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000365198 ;
T_prep (1,1,2) =  0.000365588 ;
T_prep (1,1,4) =   0.00936151 ;
T_prep (1,1,8) =  0.000359577 ;
T_prep (1,1,16) =  0.000357284 ;
T_prep (1,1,32) =  0.000358082 ;
T_prep (1,1,64) =  0.000356863 ;
T_prep (1,1,128) =  0.000358384 ;
T_prep (1,1,160) =  0.000357576 ;
T_prep (2,1,1) =  0.000459687 ;
T_prep (2,1,2) =   0.00035665 ;
T_prep (2,1,4) =  0.000425523 ;
T_prep (2,1,8) =  0.000425628 ;
T_prep (2,1,16) =  0.000423726 ;
T_prep (2,1,32) =   0.00042479 ;
T_prep (2,1,64) =  0.000427296 ;
T_prep (2,1,128) =  0.000427976 ;
T_prep (2,1,160) =  0.000426756 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000501363 ;
Time (1,1,2) =  0.000282199 ;
Time (1,1,4) =  0.000263855 ;
Time (1,1,8) =  0.000346342 ;
Time (1,1,16) =  0.000555855 ;
Time (1,1,32) =  0.000826077 ;
Time (1,1,64) =   0.00306803 ;
Time (1,1,128) =    0.0474184 ;
Time (1,1,160) =    0.0284162 ;
Time (2,1,1) =   0.00398283 ;
Time (2,1,2) =   0.00205067 ;
Time (2,1,4) =   0.00209585 ;
Time (2,1,8) =   0.00211545 ;
Time (2,1,16) =   0.00196788 ;
Time (2,1,32) =   0.00210669 ;
Time (2,1,64) =   0.00205131 ;
Time (2,1,128) =   0.00196713 ;
Time (2,1,160) =   0.00190081 ;
Time (3,1,1) =   0.00379461 ;
Time (3,1,2) =   0.00192742 ;
Time (3,1,4) =   0.00137612 ;
Time (3,1,8) =  0.000724209 ;
Time (3,1,16) =  0.000591761 ;
Time (3,1,32) =  0.000606809 ;
Time (3,1,64) =  0.000584586 ;
Time (3,1,128) =  0.000657745 ;
Time (3,1,160) =   0.00115347 ;
Time (4,1,1) =    0.0055093 ;
Time (4,1,2) =   0.00220795 ;
Time (4,1,4) =   0.00299251 ;
Time (4,1,8) =   0.00287658 ;
Time (4,1,16) =   0.00221633 ;
Time (4,1,32) =   0.00220585 ;
Time (4,1,64) =   0.00225054 ;
Time (4,1,128) =   0.00210143 ;
Time (4,1,160) =   0.00201648 ;
Time (5,1,1) =   0.00436525 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 144000 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000380794 ;
T_prep (1,1,2) =  0.000381477 ;
T_prep (1,1,4) =   0.00932923 ;
T_prep (1,1,8) =  0.000373327 ;
T_prep (1,1,16) =   0.00037351 ;
T_prep (1,1,32) =  0.000371858 ;
T_prep (1,1,64) =  0.000372768 ;
T_prep (1,1,128) =  0.000372328 ;
T_prep (1,1,160) =    0.0003724 ;
T_prep (2,1,1) =  0.000445006 ;
T_prep (2,1,2) =  0.000383412 ;
T_prep (2,1,4) =  0.000453628 ;
T_prep (2,1,8) =  0.000466705 ;
T_prep (2,1,16) =  0.000453785 ;
T_prep (2,1,32) =  0.000454808 ;
T_prep (2,1,64) =  0.000456131 ;
T_prep (2,1,128) =  0.000456491 ;
T_prep (2,1,160) =  0.000457017 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000525964 ;
Time (1,1,2) =  0.000296822 ;
Time (1,1,4) =  0.000314253 ;
Time (1,1,8) =  0.000397521 ;
Time (1,1,16) =  0.000617176 ;
Time (1,1,32) =   0.00102012 ;
Time (1,1,64) =   0.00280606 ;
Time (1,1,128) =     0.023972 ;
Time (1,1,160) =    0.0327864 ;
Time (2,1,1) =   0.00430761 ;
Time (2,1,2) =    0.0023637 ;
Time (2,1,4) =   0.00216673 ;
Time (2,1,8) =   0.00242956 ;
Time (2,1,16) =   0.00251125 ;
Time (2,1,32) =    0.0024401 ;
Time (2,1,64) =   0.00233702 ;
Time (2,1,128) =   0.00634715 ;
Time (2,1,160) =   0.00657451 ;
Time (3,1,1) =   0.00370117 ;
Time (3,1,2) =    0.0019732 ;
Time (3,1,4) =  0.000756938 ;
Time (3,1,8) =  0.000525361 ;
Time (3,1,16) =  0.000872631 ;
Time (3,1,32) =  0.000551036 ;
Time (3,1,64) =  0.000928907 ;
Time (3,1,128) =   0.00046264 ;
Time (3,1,160) =  0.000823009 ;
Time (4,1,1) =   0.00575552 ;
Time (4,1,2) =   0.00355065 ;
Time (4,1,4) =   0.00281002 ;
Time (4,1,8) =   0.00240197 ;
Time (4,1,16) =  0.000681435 ;
Time (4,1,32) =   0.00065007 ;
Time (4,1,64) =   0.00131245 ;
Time (4,1,128) =  0.000975503 ;
Time (4,1,160) =  0.000533799 ;
Time (5,1,1) =   0.00426629 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-CondMat/ca-CondMat_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 23133 ;
Nedges (id) = 93439 ;
Ntri (id) = 173361 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000499979 ;
T_prep (1,1,2) =  0.000499737 ;
T_prep (1,1,4) =  0.000499007 ;
T_prep (1,1,8) =  0.000499307 ;
T_prep (1,1,16) =  0.000498722 ;
T_prep (1,1,32) =  0.000499076 ;
T_prep (1,1,64) =  0.000499324 ;
T_prep (1,1,128) =  0.000519515 ;
T_prep (1,1,160) =  0.000506499 ;
T_prep (2,1,1) =  0.000580156 ;
T_prep (2,1,2) =  0.000534076 ;
T_prep (2,1,4) =   0.00053242 ;
T_prep (2,1,8) =  0.000531469 ;
T_prep (2,1,16) =  0.000530959 ;
T_prep (2,1,32) =  0.000531089 ;
T_prep (2,1,64) =  0.000531149 ;
T_prep (2,1,128) =  0.000530392 ;
T_prep (2,1,160) =  0.000531015 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00195497 ;
Time (1,1,2) =   0.00996158 ;
Time (1,1,4) =  0.000580757 ;
Time (1,1,8) =  0.000472491 ;
Time (1,1,16) =  0.000566493 ;
Time (1,1,32) =   0.00101251 ;
Time (1,1,64) =    0.0023336 ;
Time (1,1,128) =    0.0231296 ;
Time (1,1,160) =      0.02638 ;
Time (2,1,1) =     0.021187 ;
Time (2,1,2) =    0.0091804 ;
Time (2,1,4) =   0.00611531 ;
Time (2,1,8) =    0.0043079 ;
Time (2,1,16) =   0.00236904 ;
Time (2,1,32) =   0.00229689 ;
Time (2,1,64) =   0.00442831 ;
Time (2,1,128) =   0.00499326 ;
Time (2,1,160) =   0.00421875 ;
Time (3,1,1) =    0.0473233 ;
Time (3,1,2) =    0.0238684 ;
Time (3,1,4) =    0.0125311 ;
Time (3,1,8) =   0.00732173 ;
Time (3,1,16) =   0.00535918 ;
Time (3,1,32) =   0.00463798 ;
Time (3,1,64) =   0.00433232 ;
Time (3,1,128) =   0.00448968 ;
Time (3,1,160) =   0.00407917 ;
Time (4,1,1) =    0.0142305 ;
Time (4,1,2) =   0.00639875 ;
Time (4,1,4) =    0.0035973 ;
Time (4,1,8) =   0.00191404 ;
Time (4,1,16) =   0.00152425 ;
Time (4,1,32) =   0.00121715 ;
Time (4,1,64) =   0.00185065 ;
Time (4,1,128) =   0.00135503 ;
Time (4,1,160) =   0.00107713 ;
Time (5,1,1) =    0.0113604 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-HepPh/ca-HepPh_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 12008 ;
Nedges (id) = 118489 ;
Ntri (id) = 3358499 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00043075 ;
T_prep (1,1,2) =  0.000428549 ;
T_prep (1,1,4) =   0.00922562 ;
T_prep (1,1,8) =  0.000271231 ;
T_prep (1,1,16) =  0.000269787 ;
T_prep (1,1,32) =  0.000268661 ;
T_prep (1,1,64) =  0.000268477 ;
T_prep (1,1,128) =  0.000267746 ;
T_prep (1,1,160) =  0.000268499 ;
T_prep (2,1,1) =  0.000539374 ;
T_prep (2,1,2) =  0.000456025 ;
T_prep (2,1,4) =  0.000328054 ;
T_prep (2,1,8) =  0.000296122 ;
T_prep (2,1,16) =  0.000292259 ;
T_prep (2,1,32) =  0.000291446 ;
T_prep (2,1,64) =  0.000291232 ;
T_prep (2,1,128) =  0.000292258 ;
T_prep (2,1,160) =  0.000292122 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00411649 ;
Time (1,1,2) =   0.00228659 ;
Time (1,1,4) =   0.00137973 ;
Time (1,1,8) =   0.00126127 ;
Time (1,1,16) =   0.00238974 ;
Time (1,1,32) =   0.00285562 ;
Time (1,1,64) =   0.00624081 ;
Time (1,1,128) =    0.0293483 ;
Time (1,1,160) =    0.0299967 ;
Time (2,1,1) =     0.099455 ;
Time (2,1,2) =    0.0539452 ;
Time (2,1,4) =    0.0254466 ;
Time (2,1,8) =    0.0235386 ;
Time (2,1,16) =    0.0216429 ;
Time (2,1,32) =    0.0205684 ;
Time (2,1,64) =    0.0238569 ;
Time (2,1,128) =    0.0214367 ;
Time (2,1,160) =    0.0379886 ;
Time (3,1,1) =     0.300988 ;
Time (3,1,2) =     0.182635 ;
Time (3,1,4) =     0.104406 ;
Time (3,1,8) =    0.0766495 ;
Time (3,1,16) =    0.0665629 ;
Time (3,1,32) =    0.0584182 ;
Time (3,1,64) =    0.0607507 ;
Time (3,1,128) =    0.0624844 ;
Time (3,1,160) =    0.0706346 ;
Time (4,1,1) =    0.0376746 ;
Time (4,1,2) =    0.0193196 ;
Time (4,1,4) =   0.00895191 ;
Time (4,1,8) =   0.00698506 ;
Time (4,1,16) =   0.00683987 ;
Time (4,1,32) =   0.00678404 ;
Time (4,1,64) =   0.00743662 ;
Time (4,1,128) =   0.00625704 ;
Time (4,1,160) =   0.00718539 ;
Time (5,1,1) =     0.032807 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 175873 ;
Ntri (id) = 264799 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000490137 ;
T_prep (1,1,2) =  0.000490994 ;
T_prep (1,1,4) =   0.00946908 ;
T_prep (1,1,8) =  0.000445937 ;
T_prep (1,1,16) =  0.000446431 ;
T_prep (1,1,32) =  0.000445359 ;
T_prep (1,1,64) =  0.000445896 ;
T_prep (1,1,128) =  0.000448152 ;
T_prep (1,1,160) =  0.000456007 ;
T_prep (2,1,1) =  0.000576777 ;
T_prep (2,1,2) =   0.00045582 ;
T_prep (2,1,4) =  0.000549897 ;
T_prep (2,1,8) =  0.000549089 ;
T_prep (2,1,16) =  0.000548814 ;
T_prep (2,1,32) =  0.000548315 ;
T_prep (2,1,64) =  0.000546834 ;
T_prep (2,1,128) =  0.000548651 ;
T_prep (2,1,160) =  0.000548101 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00488268 ;
Time (1,1,2) =   0.00666821 ;
Time (1,1,4) =   0.00664626 ;
Time (1,1,8) =   0.00683954 ;
Time (1,1,16) =    0.0153136 ;
Time (1,1,32) =    0.0100805 ;
Time (1,1,64) =    0.0281423 ;
Time (1,1,128) =    0.0974597 ;
Time (1,1,160) =    0.0826474 ;
Time (2,1,1) =     0.171694 ;
Time (2,1,2) =     0.154851 ;
Time (2,1,4) =     0.152477 ;
Time (2,1,8) =     0.151023 ;
Time (2,1,16) =     0.135718 ;
Time (2,1,32) =     0.142089 ;
Time (2,1,64) =      0.13099 ;
Time (2,1,128) =     0.242053 ;
Time (2,1,160) =     0.134975 ;
Time (3,1,1) =    0.0556621 ;
Time (3,1,2) =    0.0297419 ;
Time (3,1,4) =    0.0149807 ;
Time (3,1,8) =    0.0100371 ;
Time (3,1,16) =   0.00785348 ;
Time (3,1,32) =   0.00761677 ;
Time (3,1,64) =   0.00757024 ;
Time (3,1,128) =   0.00761629 ;
Time (3,1,160) =    0.0150548 ;
Time (4,1,1) =    0.0595011 ;
Time (4,1,2) =     0.055808 ;
Time (4,1,4) =    0.0536016 ;
Time (4,1,8) =    0.0562312 ;
Time (4,1,16) =    0.0548851 ;
Time (4,1,32) =    0.0501431 ;
Time (4,1,64) =     0.049078 ;
Time (4,1,128) =   0.00961804 ;
Time (4,1,160) =   0.00980795 ;
Time (5,1,1) =    0.0513073 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 26520 ;
Nedges (id) = 175873 ;
Ntri (id) = 35 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00045862 ;
T_prep (1,1,2) =  0.000460759 ;
T_prep (1,1,4) =   0.00936033 ;
T_prep (1,1,8) =  0.000380376 ;
T_prep (1,1,16) =  0.000381048 ;
T_prep (1,1,32) =  0.000379099 ;
T_prep (1,1,64) =  0.000379265 ;
T_prep (1,1,128) =  0.000378541 ;
T_prep (1,1,160) =  0.000378584 ;
T_prep (2,1,1) =   0.00055506 ;
T_prep (2,1,2) =  0.000468859 ;
T_prep (2,1,4) =  0.000473904 ;
T_prep (2,1,8) =  0.000474977 ;
T_prep (2,1,16) =  0.000476426 ;
T_prep (2,1,32) =  0.000491636 ;
T_prep (2,1,64) =  0.000477788 ;
T_prep (2,1,128) =  0.000477844 ;
T_prep (2,1,160) =   0.00047734 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      0.00123 ;
Time (1,1,2) =   0.00124237 ;
Time (1,1,4) =   0.00127803 ;
Time (1,1,8) =   0.00132175 ;
Time (1,1,16) =   0.00219799 ;
Time (1,1,32) =   0.00189148 ;
Time (1,1,64) =   0.00534784 ;
Time (1,1,128) =    0.0218548 ;
Time (1,1,160) =    0.0540281 ;
Time (2,1,1) =    0.0309341 ;
Time (2,1,2) =    0.0249038 ;
Time (2,1,4) =    0.0230561 ;
Time (2,1,8) =     0.021814 ;
Time (2,1,16) =    0.0210941 ;
Time (2,1,32) =    0.0199308 ;
Time (2,1,64) =    0.0193588 ;
Time (2,1,128) =     0.018522 ;
Time (2,1,160) =    0.0188176 ;
Time (3,1,1) =    0.0108122 ;
Time (3,1,2) =   0.00645338 ;
Time (3,1,4) =   0.00383827 ;
Time (3,1,8) =   0.00223438 ;
Time (3,1,16) =   0.00219567 ;
Time (3,1,32) =    0.0018831 ;
Time (3,1,64) =   0.00242208 ;
Time (3,1,128) =    0.0019589 ;
Time (3,1,160) =   0.00371487 ;
Time (4,1,1) =    0.0156285 ;
Time (4,1,2) =    0.0101803 ;
Time (4,1,4) =   0.00711268 ;
Time (4,1,8) =   0.00755644 ;
Time (4,1,16) =   0.00744685 ;
Time (4,1,32) =   0.00794941 ;
Time (4,1,64) =   0.00595758 ;
Time (4,1,128) =    0.0201828 ;
Time (4,1,160) =   0.00690549 ;
Time (5,1,1) =     0.077275 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/p2p-Gnutella31/p2p-Gnutella31_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 62586 ;
Nedges (id) = 147892 ;
Ntri (id) = 2024 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00101444 ;
T_prep (1,1,2) =   0.00101429 ;
T_prep (1,1,4) =   0.00967502 ;
T_prep (1,1,8) =  0.000707041 ;
T_prep (1,1,16) =  0.000704507 ;
T_prep (1,1,32) =  0.000702476 ;
T_prep (1,1,64) =  0.000704102 ;
T_prep (1,1,128) =  0.000702929 ;
T_prep (1,1,160) =  0.000703049 ;
T_prep (2,1,1) =   0.00120708 ;
T_prep (2,1,2) =   0.00107225 ;
T_prep (2,1,4) =  0.000759923 ;
T_prep (2,1,8) =    0.0007179 ;
T_prep (2,1,16) =   0.00071388 ;
T_prep (2,1,32) =  0.000708955 ;
T_prep (2,1,64) =   0.00070939 ;
T_prep (2,1,128) =  0.000712058 ;
T_prep (2,1,160) =  0.000714632 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00301126 ;
Time (1,1,2) =   0.00156875 ;
Time (1,1,4) =  0.000885285 ;
Time (1,1,8) =  0.000650093 ;
Time (1,1,16) =  0.000563595 ;
Time (1,1,32) =  0.000959796 ;
Time (1,1,64) =   0.00208437 ;
Time (1,1,128) =    0.0313947 ;
Time (1,1,160) =    0.0249532 ;
Time (2,1,1) =     0.023472 ;
Time (2,1,2) =    0.0121782 ;
Time (2,1,4) =   0.00607328 ;
Time (2,1,8) =   0.00270792 ;
Time (2,1,16) =   0.00273692 ;
Time (2,1,32) =   0.00172048 ;
Time (2,1,64) =  0.000863187 ;
Time (2,1,128) =   0.00164563 ;
Time (2,1,160) =   0.00189887 ;
Time (3,1,1) =    0.0311705 ;
Time (3,1,2) =    0.0168053 ;
Time (3,1,4) =   0.00766895 ;
Time (3,1,8) =   0.00413114 ;
Time (3,1,16) =   0.00260441 ;
Time (3,1,32) =   0.00200837 ;
Time (3,1,64) =   0.00149066 ;
Time (3,1,128) =   0.00198075 ;
Time (3,1,160) =   0.00138007 ;
Time (4,1,1) =    0.0193095 ;
Time (4,1,2) =   0.00933704 ;
Time (4,1,4) =   0.00522024 ;
Time (4,1,8) =   0.00250702 ;
Time (4,1,16) =   0.00155623 ;
Time (4,1,32) =   0.00179234 ;
Time (4,1,64) =   0.00129142 ;
Time (4,1,128) =   0.00375927 ;
Time (4,1,160) =  0.000911945 ;
Time (5,1,1) =     0.015093 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 217255 ;
Ntri (id) = 155 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000563583 ;
T_prep (1,1,2) =  0.000559343 ;
T_prep (1,1,4) =   0.00970126 ;
T_prep (1,1,8) =  0.000313004 ;
T_prep (1,1,16) =  0.000293429 ;
T_prep (1,1,32) =  0.000294732 ;
T_prep (1,1,64) =  0.000293576 ;
T_prep (1,1,128) =  0.000293863 ;
T_prep (1,1,160) =  0.000294094 ;
T_prep (2,1,1) =  0.000752671 ;
T_prep (2,1,2) =  0.000579345 ;
T_prep (2,1,4) =   0.00055061 ;
T_prep (2,1,8) =  0.000393712 ;
T_prep (2,1,16) =  0.000386781 ;
T_prep (2,1,32) =  0.000388486 ;
T_prep (2,1,64) =  0.000386062 ;
T_prep (2,1,128) =   0.00038693 ;
T_prep (2,1,160) =  0.000387482 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00216343 ;
Time (1,1,2) =   0.00282062 ;
Time (1,1,4) =   0.00261758 ;
Time (1,1,8) =   0.00263319 ;
Time (1,1,16) =   0.00227167 ;
Time (1,1,32) =   0.00593849 ;
Time (1,1,64) =    0.0113306 ;
Time (1,1,128) =    0.0319253 ;
Time (1,1,160) =     0.026657 ;
Time (2,1,1) =    0.0148292 ;
Time (2,1,2) =    0.0120352 ;
Time (2,1,4) =    0.0405777 ;
Time (2,1,8) =    0.0446862 ;
Time (2,1,16) =     0.044716 ;
Time (2,1,32) =    0.0387213 ;
Time (2,1,64) =    0.0397825 ;
Time (2,1,128) =    0.0384201 ;
Time (2,1,160) =    0.0718251 ;
Time (3,1,1) =   0.00242814 ;
Time (3,1,2) =   0.00293796 ;
Time (3,1,4) =   0.00625666 ;
Time (3,1,8) =   0.00569867 ;
Time (3,1,16) =    0.0037752 ;
Time (3,1,32) =   0.00549889 ;
Time (3,1,64) =   0.00968215 ;
Time (3,1,128) =   0.00536441 ;
Time (3,1,160) =   0.00534124 ;
Time (4,1,1) =   0.00322048 ;
Time (4,1,2) =   0.00333903 ;
Time (4,1,4) =   0.00293424 ;
Time (4,1,8) =    0.0155483 ;
Time (4,1,16) =    0.0148587 ;
Time (4,1,32) =    0.0145888 ;
Time (4,1,64) =    0.0146104 ;
Time (4,1,128) =   0.00783792 ;
Time (4,1,160) =    0.0020823 ;
Time (5,1,1) =   0.00633085 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 20400 ;
Nedges (id) = 217255 ;
Ntri (id) = 465427 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00056675 ;
T_prep (1,1,2) =  0.000565418 ;
T_prep (1,1,4) =   0.00973213 ;
T_prep (1,1,8) =  0.000495951 ;
T_prep (1,1,16) =  0.000475128 ;
T_prep (1,1,32) =  0.000472629 ;
T_prep (1,1,64) =  0.000472972 ;
T_prep (1,1,128) =  0.000475405 ;
T_prep (1,1,160) =  0.000474126 ;
T_prep (2,1,1) =  0.000736793 ;
T_prep (2,1,2) =  0.000597941 ;
T_prep (2,1,4) =  0.000657663 ;
T_prep (2,1,8) =   0.00058192 ;
T_prep (2,1,16) =  0.000581156 ;
T_prep (2,1,32) =  0.000581987 ;
T_prep (2,1,64) =  0.000580713 ;
T_prep (2,1,128) =  0.000582998 ;
T_prep (2,1,160) =   0.00061537 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00845813 ;
Time (1,1,2) =    0.0121753 ;
Time (1,1,4) =    0.0121948 ;
Time (1,1,8) =    0.0123104 ;
Time (1,1,16) =    0.0128901 ;
Time (1,1,32) =    0.0289584 ;
Time (1,1,64) =    0.0787429 ;
Time (1,1,128) =      0.16893 ;
Time (1,1,160) =     0.164387 ;
Time (2,1,1) =     0.323381 ;
Time (2,1,2) =     0.294532 ;
Time (2,1,4) =     0.292025 ;
Time (2,1,8) =     0.290848 ;
Time (2,1,16) =     0.282089 ;
Time (2,1,32) =     0.266744 ;
Time (2,1,64) =      0.25669 ;
Time (2,1,128) =     0.475622 ;
Time (2,1,160) =    0.0514739 ;
Time (3,1,1) =    0.0984639 ;
Time (3,1,2) =    0.0537724 ;
Time (3,1,4) =    0.0351932 ;
Time (3,1,8) =    0.0229416 ;
Time (3,1,16) =    0.0212437 ;
Time (3,1,32) =    0.0217452 ;
Time (3,1,64) =    0.0203583 ;
Time (3,1,128) =    0.0474098 ;
Time (3,1,160) =    0.0530269 ;
Time (4,1,1) =     0.107281 ;
Time (4,1,2) =     0.101308 ;
Time (4,1,4) =    0.0998992 ;
Time (4,1,8) =    0.0988663 ;
Time (4,1,16) =    0.0956717 ;
Time (4,1,32) =    0.0936326 ;
Time (4,1,64) =    0.0256442 ;
Time (4,1,128) =    0.0899865 ;
Time (4,1,160) =    0.0886431 ;
Time (5,1,1) =    0.0954296 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/email-Enron/email-Enron_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 36692 ;
Nedges (id) = 183831 ;
Ntri (id) = 727044 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000837694 ;
T_prep (1,1,2) =   0.00082681 ;
T_prep (1,1,4) =   0.00975322 ;
T_prep (1,1,8) =  0.000671144 ;
T_prep (1,1,16) =  0.000665425 ;
T_prep (1,1,32) =  0.000664954 ;
T_prep (1,1,64) =  0.000665779 ;
T_prep (1,1,128) =  0.000668659 ;
T_prep (1,1,160) =  0.000672249 ;
T_prep (2,1,1) =  0.000997771 ;
T_prep (2,1,2) =    0.0008795 ;
T_prep (2,1,4) =  0.000687003 ;
T_prep (2,1,8) =  0.000622345 ;
T_prep (2,1,16) =  0.000623438 ;
T_prep (2,1,32) =  0.000623718 ;
T_prep (2,1,64) =  0.000629427 ;
T_prep (2,1,128) =  0.000621154 ;
T_prep (2,1,160) =   0.00062156 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0056562 ;
Time (1,1,2) =   0.00371653 ;
Time (1,1,4) =   0.00189384 ;
Time (1,1,8) =   0.00167466 ;
Time (1,1,16) =   0.00130471 ;
Time (1,1,32) =   0.00166928 ;
Time (1,1,64) =   0.00423489 ;
Time (1,1,128) =    0.0226949 ;
Time (1,1,160) =    0.0258222 ;
Time (2,1,1) =     0.129083 ;
Time (2,1,2) =    0.0624424 ;
Time (2,1,4) =    0.0305311 ;
Time (2,1,8) =    0.0172076 ;
Time (2,1,16) =     0.010898 ;
Time (2,1,32) =    0.0130856 ;
Time (2,1,64) =    0.0102981 ;
Time (2,1,128) =    0.0122831 ;
Time (2,1,160) =    0.0106373 ;
Time (3,1,1) =     0.322453 ;
Time (3,1,2) =       0.1654 ;
Time (3,1,4) =    0.0916083 ;
Time (3,1,8) =     0.046495 ;
Time (3,1,16) =    0.0405786 ;
Time (3,1,32) =    0.0383197 ;
Time (3,1,64) =    0.0593385 ;
Time (3,1,128) =    0.0836587 ;
Time (3,1,160) =    0.0774005 ;
Time (4,1,1) =    0.0536342 ;
Time (4,1,2) =    0.0264853 ;
Time (4,1,4) =    0.0139131 ;
Time (4,1,8) =   0.00908423 ;
Time (4,1,16) =   0.00472321 ;
Time (4,1,32) =   0.00525703 ;
Time (4,1,64) =   0.00485729 ;
Time (4,1,128) =   0.00503591 ;
Time (4,1,160) =   0.00436554 ;
Time (5,1,1) =    0.0505908 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-260610-65536_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 65536 ;
Nedges (id) = 260610 ;
Ntri (id) = 260100 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000604204 ;
T_prep (1,1,2) =  0.000605931 ;
T_prep (1,1,4) =   0.00951003 ;
T_prep (1,1,8) =  0.000354971 ;
T_prep (1,1,16) =  0.000303594 ;
T_prep (1,1,32) =  0.000303658 ;
T_prep (1,1,64) =  0.000303155 ;
T_prep (1,1,128) =  0.000303298 ;
T_prep (1,1,160) =   0.00030375 ;
T_prep (2,1,1) =  0.000821406 ;
T_prep (2,1,2) =  0.000677505 ;
T_prep (2,1,4) =  0.000422529 ;
T_prep (2,1,8) =  0.000308193 ;
T_prep (2,1,16) =  0.000301421 ;
T_prep (2,1,32) =  0.000301431 ;
T_prep (2,1,64) =  0.000301907 ;
T_prep (2,1,128) =  0.000301232 ;
T_prep (2,1,160) =  0.000302334 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00150391 ;
Time (1,1,2) =  0.000895126 ;
Time (1,1,4) =  0.000538331 ;
Time (1,1,8) =  0.000381019 ;
Time (1,1,16) =  0.000532437 ;
Time (1,1,32) =  0.000809489 ;
Time (1,1,64) =   0.00222903 ;
Time (1,1,128) =   0.00724381 ;
Time (1,1,160) =    0.0548135 ;
Time (2,1,1) =    0.0358924 ;
Time (2,1,2) =    0.0169673 ;
Time (2,1,4) =   0.00742026 ;
Time (2,1,8) =   0.00401215 ;
Time (2,1,16) =   0.00208246 ;
Time (2,1,32) =    0.0016483 ;
Time (2,1,64) =   0.00128138 ;
Time (2,1,128) =   0.00112981 ;
Time (2,1,160) =   0.00103115 ;
Time (3,1,1) =    0.0313956 ;
Time (3,1,2) =    0.0166465 ;
Time (3,1,4) =   0.00788073 ;
Time (3,1,8) =   0.00366661 ;
Time (3,1,16) =   0.00180829 ;
Time (3,1,32) =   0.00114959 ;
Time (3,1,64) =  0.000804694 ;
Time (3,1,128) =   0.00386629 ;
Time (3,1,160) =  0.000473681 ;
Time (4,1,1) =    0.0252912 ;
Time (4,1,2) =    0.0126297 ;
Time (4,1,4) =   0.00524034 ;
Time (4,1,8) =   0.00280834 ;
Time (4,1,16) =   0.00199173 ;
Time (4,1,32) =    0.0013734 ;
Time (4,1,64) =   0.00114099 ;
Time (4,1,128) =  0.000977335 ;
Time (4,1,160) =   0.00130255 ;
Time (5,1,1) =    0.0162003 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/ca-AstroPh/ca-AstroPh_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 18772 ;
Nedges (id) = 198050 ;
Ntri (id) = 1351441 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000715215 ;
T_prep (1,1,2) =  0.000714453 ;
T_prep (1,1,4) =   0.00944266 ;
T_prep (1,1,8) =   0.00038646 ;
T_prep (1,1,16) =  0.000384934 ;
T_prep (1,1,32) =  0.000384859 ;
T_prep (1,1,64) =  0.000383804 ;
T_prep (1,1,128) =  0.000383765 ;
T_prep (1,1,160) =  0.000384724 ;
T_prep (2,1,1) =  0.000881903 ;
T_prep (2,1,2) =  0.000779417 ;
T_prep (2,1,4) =  0.000396968 ;
T_prep (2,1,8) =  0.000380689 ;
T_prep (2,1,16) =  0.000376774 ;
T_prep (2,1,32) =   0.00037638 ;
T_prep (2,1,64) =  0.000379082 ;
T_prep (2,1,128) =  0.000377527 ;
T_prep (2,1,160) =  0.000377193 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00542121 ;
Time (1,1,2) =   0.00284229 ;
Time (1,1,4) =   0.00164168 ;
Time (1,1,8) =   0.00117295 ;
Time (1,1,16) =   0.00113947 ;
Time (1,1,32) =   0.00210506 ;
Time (1,1,64) =   0.00529205 ;
Time (1,1,128) =    0.0210182 ;
Time (1,1,160) =    0.0299304 ;
Time (2,1,1) =    0.0915487 ;
Time (2,1,2) =    0.0504394 ;
Time (2,1,4) =    0.0242531 ;
Time (2,1,8) =    0.0231079 ;
Time (2,1,16) =     0.011788 ;
Time (2,1,32) =    0.0123161 ;
Time (2,1,64) =    0.0112248 ;
Time (2,1,128) =    0.0119248 ;
Time (2,1,160) =    0.0243368 ;
Time (3,1,1) =     0.330905 ;
Time (3,1,2) =     0.167481 ;
Time (3,1,4) =    0.0838548 ;
Time (3,1,8) =    0.0542748 ;
Time (3,1,16) =    0.0401156 ;
Time (3,1,32) =    0.0756629 ;
Time (3,1,64) =    0.0723387 ;
Time (3,1,128) =    0.0773231 ;
Time (3,1,160) =    0.0678694 ;
Time (4,1,1) =    0.0436498 ;
Time (4,1,2) =    0.0223923 ;
Time (4,1,4) =    0.0109254 ;
Time (4,1,8) =   0.00560942 ;
Time (4,1,16) =   0.00484218 ;
Time (4,1,32) =   0.00475458 ;
Time (4,1,64) =   0.00481772 ;
Time (4,1,128) =    0.0102978 ;
Time (4,1,160) =   0.00430796 ;
Time (5,1,1) =    0.0372497 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/loc-brightkite_edges/loc-brightkite_edges_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 58228 ;
Nedges (id) = 214078 ;
Ntri (id) = 494728 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00112287 ;
T_prep (1,1,2) =   0.00112259 ;
T_prep (1,1,4) =   0.00973894 ;
T_prep (1,1,8) =  0.000609748 ;
T_prep (1,1,16) =  0.000588714 ;
T_prep (1,1,32) =  0.000586802 ;
T_prep (1,1,64) =  0.000586463 ;
T_prep (1,1,128) =  0.000586792 ;
T_prep (1,1,160) =  0.000599534 ;
T_prep (2,1,1) =   0.00133214 ;
T_prep (2,1,2) =   0.00120013 ;
T_prep (2,1,4) =  0.000785735 ;
T_prep (2,1,8) =   0.00064984 ;
T_prep (2,1,16) =  0.000645918 ;
T_prep (2,1,32) =  0.000646079 ;
T_prep (2,1,64) =  0.000643373 ;
T_prep (2,1,128) =  0.000648646 ;
T_prep (2,1,160) =  0.000646038 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00521088 ;
Time (1,1,2) =   0.00277064 ;
Time (1,1,4) =   0.00158244 ;
Time (1,1,8) =   0.00113636 ;
Time (1,1,16) =  0.000961537 ;
Time (1,1,32) =    0.0023856 ;
Time (1,1,64) =    0.0049535 ;
Time (1,1,128) =    0.0296768 ;
Time (1,1,160) =    0.0465028 ;
Time (2,1,1) =    0.0768981 ;
Time (2,1,2) =    0.0384995 ;
Time (2,1,4) =    0.0172148 ;
Time (2,1,8) =    0.0137757 ;
Time (2,1,16) =    0.0132413 ;
Time (2,1,32) =    0.0124055 ;
Time (2,1,64) =    0.0127677 ;
Time (2,1,128) =    0.0151935 ;
Time (2,1,160) =   0.00981789 ;
Time (3,1,1) =     0.211564 ;
Time (3,1,2) =    0.0977901 ;
Time (3,1,4) =    0.0590792 ;
Time (3,1,8) =    0.0439528 ;
Time (3,1,16) =    0.0426426 ;
Time (3,1,32) =    0.0395377 ;
Time (3,1,64) =    0.0725397 ;
Time (3,1,128) =    0.0365506 ;
Time (3,1,160) =    0.0380403 ;
Time (4,1,1) =    0.0391982 ;
Time (4,1,2) =    0.0185904 ;
Time (4,1,4) =    0.0080756 ;
Time (4,1,8) =   0.00412261 ;
Time (4,1,16) =   0.00256716 ;
Time (4,1,32) =   0.00240583 ;
Time (4,1,64) =    0.0026001 ;
Time (4,1,128) =   0.00300716 ;
Time (4,1,160) =   0.00564448 ;
Time (5,1,1) =    0.0336825 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320000 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000951212 ;
T_prep (1,1,2) =  0.000953237 ;
T_prep (1,1,4) =    0.0101178 ;
T_prep (1,1,8) =   0.00101569 ;
T_prep (1,1,16) =  0.000922473 ;
T_prep (1,1,32) =  0.000918317 ;
T_prep (1,1,64) =  0.000924009 ;
T_prep (1,1,128) =  0.000927058 ;
T_prep (1,1,160) =  0.000927295 ;
T_prep (2,1,1) =   0.00115784 ;
T_prep (2,1,2) =  0.000958493 ;
T_prep (2,1,4) =   0.00120871 ;
T_prep (2,1,8) =   0.00110022 ;
T_prep (2,1,16) =   0.00108108 ;
T_prep (2,1,32) =   0.00107814 ;
T_prep (2,1,64) =   0.00108659 ;
T_prep (2,1,128) =   0.00109236 ;
T_prep (2,1,160) =   0.00108719 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00146519 ;
Time (1,1,2) =   0.00106171 ;
Time (1,1,4) =   0.00107806 ;
Time (1,1,8) =   0.00108879 ;
Time (1,1,16) =    0.0018833 ;
Time (1,1,32) =   0.00257369 ;
Time (1,1,64) =   0.00766082 ;
Time (1,1,128) =     0.040553 ;
Time (1,1,160) =    0.0266005 ;
Time (2,1,1) =    0.0119298 ;
Time (2,1,2) =    0.0096787 ;
Time (2,1,4) =   0.00997916 ;
Time (2,1,8) =   0.00983852 ;
Time (2,1,16) =    0.0100522 ;
Time (2,1,32) =   0.00123163 ;
Time (2,1,64) =   0.00941944 ;
Time (2,1,128) =   0.00883277 ;
Time (2,1,160) =   0.00809488 ;
Time (3,1,1) =     0.011676 ;
Time (3,1,2) =   0.00508831 ;
Time (3,1,4) =   0.00268637 ;
Time (3,1,8) =     0.001213 ;
Time (3,1,16) =  0.000607522 ;
Time (3,1,32) =  0.000780915 ;
Time (3,1,64) =  0.000475087 ;
Time (3,1,128) =   0.00120667 ;
Time (3,1,160) =   0.00100798 ;
Time (4,1,1) =    0.0171631 ;
Time (4,1,2) =   0.00973759 ;
Time (4,1,4) =    0.0101831 ;
Time (4,1,8) =    0.0103641 ;
Time (4,1,16) =   0.00985942 ;
Time (4,1,32) =    0.0100961 ;
Time (4,1,64) =   0.00187606 ;
Time (4,1,128) =   0.00190519 ;
Time (4,1,160) =    0.0018799 ;
Time (5,1,1) =    0.0121701 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320881 ;
Ntri (id) = 160000 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000924002 ;
T_prep (1,1,2) =  0.000923538 ;
T_prep (1,1,4) =    0.0101074 ;
T_prep (1,1,8) =   0.00102375 ;
T_prep (1,1,16) =  0.000907151 ;
T_prep (1,1,32) =  0.000911763 ;
T_prep (1,1,64) =   0.00091585 ;
T_prep (1,1,128) =  0.000916454 ;
T_prep (1,1,160) =  0.000921628 ;
T_prep (2,1,1) =   0.00115363 ;
T_prep (2,1,2) =  0.000929446 ;
T_prep (2,1,4) =    0.0012041 ;
T_prep (2,1,8) =   0.00111935 ;
T_prep (2,1,16) =   0.00108376 ;
T_prep (2,1,32) =   0.00107909 ;
T_prep (2,1,64) =   0.00108224 ;
T_prep (2,1,128) =    0.0010909 ;
T_prep (2,1,160) =   0.00108874 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0016618 ;
Time (1,1,2) =   0.00129792 ;
Time (1,1,4) =   0.00125984 ;
Time (1,1,8) =   0.00129195 ;
Time (1,1,16) =   0.00152025 ;
Time (1,1,32) =   0.00272072 ;
Time (1,1,64) =   0.00872729 ;
Time (1,1,128) =    0.0324843 ;
Time (1,1,160) =    0.0184126 ;
Time (2,1,1) =    0.0211353 ;
Time (2,1,2) =    0.0183769 ;
Time (2,1,4) =    0.0194013 ;
Time (2,1,8) =    0.0187007 ;
Time (2,1,16) =    0.0178947 ;
Time (2,1,32) =    0.0169254 ;
Time (2,1,64) =     0.016083 ;
Time (2,1,128) =    0.0164679 ;
Time (2,1,160) =    0.0458612 ;
Time (3,1,1) =    0.0162661 ;
Time (3,1,2) =   0.00741683 ;
Time (3,1,4) =   0.00364259 ;
Time (3,1,8) =   0.00173622 ;
Time (3,1,16) =  0.000794951 ;
Time (3,1,32) =  0.000498185 ;
Time (3,1,64) =   0.00102015 ;
Time (3,1,128) =  0.000261302 ;
Time (3,1,160) =  0.000424714 ;
Time (4,1,1) =    0.0205211 ;
Time (4,1,2) =    0.0133627 ;
Time (4,1,4) =    0.0136565 ;
Time (4,1,8) =     0.013762 ;
Time (4,1,16) =    0.0140054 ;
Time (4,1,32) =    0.0123032 ;
Time (4,1,64) =    0.0122779 ;
Time (4,1,128) =    0.0042544 ;
Time (4,1,160) =   0.00283496 ;
Time (5,1,1) =    0.0132938 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-256-625-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 160882 ;
Nedges (id) = 320881 ;
Ntri (id) = 1 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =  0.000969169 ;
T_prep (1,1,2) =  0.000961153 ;
T_prep (1,1,4) =    0.0102622 ;
T_prep (1,1,8) =  0.000990792 ;
T_prep (1,1,16) =  0.000911532 ;
T_prep (1,1,32) =  0.000916522 ;
T_prep (1,1,64) =  0.000920852 ;
T_prep (1,1,128) =  0.000922175 ;
T_prep (1,1,160) =  0.000923918 ;
T_prep (2,1,1) =   0.00120217 ;
T_prep (2,1,2) =   0.00096406 ;
T_prep (2,1,4) =    0.0012369 ;
T_prep (2,1,8) =   0.00108222 ;
T_prep (2,1,16) =   0.00107966 ;
T_prep (2,1,32) =   0.00108401 ;
T_prep (2,1,64) =   0.00108699 ;
T_prep (2,1,128) =   0.00108472 ;
T_prep (2,1,160) =   0.00108506 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00146994 ;
Time (1,1,2) =   0.00110153 ;
Time (1,1,4) =   0.00106151 ;
Time (1,1,8) =   0.00108933 ;
Time (1,1,16) =   0.00132857 ;
Time (1,1,32) =   0.00157048 ;
Time (1,1,64) =   0.00803675 ;
Time (1,1,128) =    0.0143087 ;
Time (1,1,160) =    0.0252733 ;
Time (2,1,1) =    0.0117525 ;
Time (2,1,2) =   0.00979316 ;
Time (2,1,4) =   0.00998972 ;
Time (2,1,8) =   0.00936843 ;
Time (2,1,16) =   0.00935119 ;
Time (2,1,32) =   0.00877332 ;
Time (2,1,64) =   0.00837492 ;
Time (2,1,128) =   0.00814416 ;
Time (2,1,160) =    0.0160772 ;
Time (3,1,1) =    0.0108269 ;
Time (3,1,2) =   0.00600829 ;
Time (3,1,4) =   0.00307515 ;
Time (3,1,8) =   0.00152704 ;
Time (3,1,16) =  0.000873072 ;
Time (3,1,32) =  0.000705486 ;
Time (3,1,64) =  0.000530922 ;
Time (3,1,128) =   0.00104109 ;
Time (3,1,160) =   0.00034676 ;
Time (4,1,1) =    0.0163873 ;
Time (4,1,2) =    0.0109428 ;
Time (4,1,4) =    0.0106475 ;
Time (4,1,8) =    0.0108569 ;
Time (4,1,16) =    0.0101873 ;
Time (4,1,32) =     0.010046 ;
Time (4,1,64) =   0.00919021 ;
Time (4,1,128) =    0.0103438 ;
Time (4,1,160) =    0.0179543 ;
Time (5,1,1) =     0.014878 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-HepTh/cit-HepTh_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 27770 ;
Nedges (id) = 352285 ;
Ntri (id) = 1478735 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00122715 ;
T_prep (1,1,2) =   0.00122475 ;
T_prep (1,1,4) =   0.00989454 ;
T_prep (1,1,8) =   0.00067472 ;
T_prep (1,1,16) =  0.000514455 ;
T_prep (1,1,32) =  0.000515753 ;
T_prep (1,1,64) =  0.000516229 ;
T_prep (1,1,128) =  0.000519739 ;
T_prep (1,1,160) =  0.000518476 ;
T_prep (2,1,1) =   0.00151552 ;
T_prep (2,1,2) =   0.00135607 ;
T_prep (2,1,4) =   0.00131211 ;
T_prep (2,1,8) =  0.000510541 ;
T_prep (2,1,16) =  0.000500187 ;
T_prep (2,1,32) =  0.000497136 ;
T_prep (2,1,64) =   0.00049813 ;
T_prep (2,1,128) =  0.000499478 ;
T_prep (2,1,160) =  0.000503216 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0120705 ;
Time (1,1,2) =   0.00803052 ;
Time (1,1,4) =   0.00440437 ;
Time (1,1,8) =   0.00240117 ;
Time (1,1,16) =   0.00223871 ;
Time (1,1,32) =   0.00337353 ;
Time (1,1,64) =    0.0070146 ;
Time (1,1,128) =    0.0209097 ;
Time (1,1,160) =    0.0360027 ;
Time (2,1,1) =     0.266484 ;
Time (2,1,2) =     0.132331 ;
Time (2,1,4) =    0.0614292 ;
Time (2,1,8) =    0.0276854 ;
Time (2,1,16) =    0.0202053 ;
Time (2,1,32) =    0.0390813 ;
Time (2,1,64) =    0.0187027 ;
Time (2,1,128) =    0.0433594 ;
Time (2,1,160) =    0.0216569 ;
Time (3,1,1) =      0.61664 ;
Time (3,1,2) =     0.316099 ;
Time (3,1,4) =     0.170742 ;
Time (3,1,8) =    0.0901751 ;
Time (3,1,16) =    0.0616278 ;
Time (3,1,32) =    0.0598987 ;
Time (3,1,64) =      0.11341 ;
Time (3,1,128) =    0.0886138 ;
Time (3,1,160) =     0.167274 ;
Time (4,1,1) =     0.111014 ;
Time (4,1,2) =    0.0549241 ;
Time (4,1,4) =    0.0249753 ;
Time (4,1,8) =    0.0122889 ;
Time (4,1,16) =   0.00901109 ;
Time (4,1,32) =   0.00907056 ;
Time (4,1,64) =    0.0137261 ;
Time (4,1,128) =    0.0186399 ;
Time (4,1,160) =    0.0205112 ;
Time (5,1,1) =    0.0945689 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Epinions1/soc-Epinions1_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 75879 ;
Nedges (id) = 405740 ;
Ntri (id) = 1624481 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00170423 ;
T_prep (1,1,2) =    0.0017058 ;
T_prep (1,1,4) =    0.0101979 ;
T_prep (1,1,8) =  0.000807096 ;
T_prep (1,1,16) =  0.000605023 ;
T_prep (1,1,32) =  0.000582752 ;
T_prep (1,1,64) =  0.000587847 ;
T_prep (1,1,128) =  0.000591581 ;
T_prep (1,1,160) =  0.000589159 ;
T_prep (2,1,1) =   0.00204902 ;
T_prep (2,1,2) =   0.00183433 ;
T_prep (2,1,4) =    0.0014543 ;
T_prep (2,1,8) =  0.000763248 ;
T_prep (2,1,16) =  0.000698975 ;
T_prep (2,1,32) =  0.000695355 ;
T_prep (2,1,64) =  0.000691994 ;
T_prep (2,1,128) =  0.000730858 ;
T_prep (2,1,160) =   0.00072373 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0186235 ;
Time (1,1,2) =     0.012358 ;
Time (1,1,4) =   0.00611188 ;
Time (1,1,8) =   0.00316826 ;
Time (1,1,16) =   0.00266167 ;
Time (1,1,32) =   0.00639873 ;
Time (1,1,64) =    0.0153146 ;
Time (1,1,128) =    0.0266493 ;
Time (1,1,160) =    0.0327283 ;
Time (2,1,1) =     0.408332 ;
Time (2,1,2) =     0.182977 ;
Time (2,1,4) =    0.0852188 ;
Time (2,1,8) =    0.0422082 ;
Time (2,1,16) =    0.0345587 ;
Time (2,1,32) =    0.0355256 ;
Time (2,1,64) =    0.0233345 ;
Time (2,1,128) =    0.0846208 ;
Time (2,1,160) =    0.0365042 ;
Time (3,1,1) =      1.30219 ;
Time (3,1,2) =     0.716567 ;
Time (3,1,4) =       0.3694 ;
Time (3,1,8) =     0.258431 ;
Time (3,1,16) =     0.182876 ;
Time (3,1,32) =     0.153356 ;
Time (3,1,64) =     0.143091 ;
Time (3,1,128) =     0.319691 ;
Time (3,1,160) =     0.223629 ;
Time (4,1,1) =     0.123754 ;
Time (4,1,2) =    0.0677503 ;
Time (4,1,4) =    0.0342466 ;
Time (4,1,8) =     0.016926 ;
Time (4,1,16) =    0.0130087 ;
Time (4,1,32) =    0.0137756 ;
Time (4,1,64) =     0.011348 ;
Time (4,1,128) =   0.00990193 ;
Time (4,1,160) =   0.00800804 ;
Time (5,1,1) =      0.11994 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/email-EuAll/email-EuAll_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 265214 ;
Nedges (id) = 364481 ;
Ntri (id) = 267313 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0021104 ;
T_prep (1,1,2) =    0.0021264 ;
T_prep (1,1,4) =    0.0106196 ;
T_prep (1,1,8) =   0.00116728 ;
T_prep (1,1,16) =   0.00103601 ;
T_prep (1,1,32) =   0.00103888 ;
T_prep (1,1,64) =   0.00104287 ;
T_prep (1,1,128) =   0.00105612 ;
T_prep (1,1,160) =   0.00105711 ;
T_prep (2,1,1) =   0.00259858 ;
T_prep (2,1,2) =    0.0022175 ;
T_prep (2,1,4) =   0.00158712 ;
T_prep (2,1,8) =   0.00103898 ;
T_prep (2,1,16) =   0.00103118 ;
T_prep (2,1,32) =   0.00102502 ;
T_prep (2,1,64) =   0.00103429 ;
T_prep (2,1,128) =   0.00103645 ;
T_prep (2,1,160) =   0.00105498 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0119097 ;
Time (1,1,2) =   0.00786712 ;
Time (1,1,4) =   0.00355481 ;
Time (1,1,8) =   0.00184258 ;
Time (1,1,16) =   0.00170795 ;
Time (1,1,32) =   0.00205484 ;
Time (1,1,64) =   0.00363665 ;
Time (1,1,128) =    0.0343365 ;
Time (1,1,160) =    0.0254677 ;
Time (2,1,1) =     0.187245 ;
Time (2,1,2) =     0.090496 ;
Time (2,1,4) =    0.0451712 ;
Time (2,1,8) =     0.023156 ;
Time (2,1,16) =   0.00933447 ;
Time (2,1,32) =   0.00577554 ;
Time (2,1,64) =   0.00402646 ;
Time (2,1,128) =    0.0079274 ;
Time (2,1,160) =   0.00503899 ;
Time (3,1,1) =      0.39935 ;
Time (3,1,2) =     0.205448 ;
Time (3,1,4) =     0.105999 ;
Time (3,1,8) =    0.0529416 ;
Time (3,1,16) =    0.0230437 ;
Time (3,1,32) =    0.0143324 ;
Time (3,1,64) =    0.0105285 ;
Time (3,1,128) =   0.00902037 ;
Time (3,1,160) =   0.00913015 ;
Time (4,1,1) =     0.083451 ;
Time (4,1,2) =      0.04329 ;
Time (4,1,4) =    0.0218741 ;
Time (4,1,8) =    0.0105213 ;
Time (4,1,16) =   0.00555522 ;
Time (4,1,32) =     0.003418 ;
Time (4,1,64) =   0.00273575 ;
Time (4,1,128) =   0.00337786 ;
Time (4,1,160) =   0.00389037 ;
Time (5,1,1) =     0.262778 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-HepPh/cit-HepPh_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 34546 ;
Nedges (id) = 420877 ;
Ntri (id) = 1276868 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00149829 ;
T_prep (1,1,2) =   0.00149914 ;
T_prep (1,1,4) =   0.00997761 ;
T_prep (1,1,8) =  0.000929821 ;
T_prep (1,1,16) =  0.000713413 ;
T_prep (1,1,32) =  0.000667203 ;
T_prep (1,1,64) =  0.000673353 ;
T_prep (1,1,128) =  0.000671437 ;
T_prep (1,1,160) =   0.00067063 ;
T_prep (2,1,1) =   0.00185971 ;
T_prep (2,1,2) =   0.00166451 ;
T_prep (2,1,4) =   0.00124986 ;
T_prep (2,1,8) =  0.000575378 ;
T_prep (2,1,16) =  0.000531124 ;
T_prep (2,1,32) =  0.000521332 ;
T_prep (2,1,64) =  0.000524248 ;
T_prep (2,1,128) =  0.000533585 ;
T_prep (2,1,160) =   0.00052791 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0121083 ;
Time (1,1,2) =   0.00856214 ;
Time (1,1,4) =    0.0047895 ;
Time (1,1,8) =   0.00235637 ;
Time (1,1,16) =   0.00165405 ;
Time (1,1,32) =    0.0028574 ;
Time (1,1,64) =   0.00577217 ;
Time (1,1,128) =    0.0274451 ;
Time (1,1,160) =    0.0598063 ;
Time (2,1,1) =     0.201856 ;
Time (2,1,2) =     0.098414 ;
Time (2,1,4) =     0.047402 ;
Time (2,1,8) =    0.0207071 ;
Time (2,1,16) =    0.0136117 ;
Time (2,1,32) =     0.011308 ;
Time (2,1,64) =    0.0104671 ;
Time (2,1,128) =    0.0370164 ;
Time (2,1,160) =    0.0215683 ;
Time (3,1,1) =      0.58651 ;
Time (3,1,2) =     0.295983 ;
Time (3,1,4) =     0.142517 ;
Time (3,1,8) =    0.0827657 ;
Time (3,1,16) =    0.0453881 ;
Time (3,1,32) =     0.043525 ;
Time (3,1,64) =    0.0607097 ;
Time (3,1,128) =     0.102797 ;
Time (3,1,160) =     0.073582 ;
Time (4,1,1) =    0.0953441 ;
Time (4,1,2) =    0.0469702 ;
Time (4,1,4) =    0.0214374 ;
Time (4,1,8) =    0.0102973 ;
Time (4,1,16) =   0.00715635 ;
Time (4,1,32) =   0.00565378 ;
Time (4,1,64) =   0.00665322 ;
Time (4,1,128) =     0.010055 ;
Time (4,1,160) =   0.00644967 ;
Time (5,1,1) =    0.0804119 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Slashdot0811/soc-Slashdot0811_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 77360 ;
Nedges (id) = 469180 ;
Ntri (id) = 551724 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00197177 ;
T_prep (1,1,2) =   0.00197233 ;
T_prep (1,1,4) =    0.0103176 ;
T_prep (1,1,8) =   0.00127269 ;
T_prep (1,1,16) =   0.00109092 ;
T_prep (1,1,32) =  0.000956274 ;
T_prep (1,1,64) =  0.000955581 ;
T_prep (1,1,128) =  0.000974575 ;
T_prep (1,1,160) =  0.000971111 ;
T_prep (2,1,1) =   0.00247787 ;
T_prep (2,1,2) =   0.00213191 ;
T_prep (2,1,4) =   0.00170674 ;
T_prep (2,1,8) =   0.00109128 ;
T_prep (2,1,16) =  0.000838801 ;
T_prep (2,1,32) =   0.00082641 ;
T_prep (2,1,64) =  0.000823855 ;
T_prep (2,1,128) =  0.000821888 ;
T_prep (2,1,160) =  0.000828022 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0198049 ;
Time (1,1,2) =    0.0149585 ;
Time (1,1,4) =   0.00801925 ;
Time (1,1,8) =    0.0045788 ;
Time (1,1,16) =   0.00548842 ;
Time (1,1,32) =   0.00494412 ;
Time (1,1,64) =    0.0117605 ;
Time (1,1,128) =    0.0477335 ;
Time (1,1,160) =    0.0242169 ;
Time (2,1,1) =     0.382259 ;
Time (2,1,2) =     0.175236 ;
Time (2,1,4) =    0.0830643 ;
Time (2,1,8) =    0.0375603 ;
Time (2,1,16) =     0.032451 ;
Time (2,1,32) =    0.0320161 ;
Time (2,1,64) =    0.0199832 ;
Time (2,1,128) =    0.0491623 ;
Time (2,1,160) =    0.0367501 ;
Time (3,1,1) =       1.0621 ;
Time (3,1,2) =     0.560027 ;
Time (3,1,4) =       0.3001 ;
Time (3,1,8) =     0.213938 ;
Time (3,1,16) =     0.167699 ;
Time (3,1,32) =     0.135845 ;
Time (3,1,64) =     0.144259 ;
Time (3,1,128) =     0.318786 ;
Time (3,1,160) =     0.249003 ;
Time (4,1,1) =     0.135814 ;
Time (4,1,2) =    0.0685266 ;
Time (4,1,4) =    0.0343441 ;
Time (4,1,8) =    0.0169016 ;
Time (4,1,16) =    0.0127109 ;
Time (4,1,32) =    0.0137578 ;
Time (4,1,64) =    0.0113582 ;
Time (4,1,128) =   0.00918174 ;
Time (4,1,160) =   0.00481376 ;
Time (5,1,1) =      0.12973 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/soc-Slashdot0902/soc-Slashdot0902_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 82168 ;
Nedges (id) = 504230 ;
Ntri (id) = 602592 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00211205 ;
T_prep (1,1,2) =   0.00214799 ;
T_prep (1,1,4) =    0.0104765 ;
T_prep (1,1,8) =  0.000959204 ;
T_prep (1,1,16) =   0.00118379 ;
T_prep (1,1,32) =   0.00105738 ;
T_prep (1,1,64) =   0.00105598 ;
T_prep (1,1,128) =   0.00107691 ;
T_prep (1,1,160) =   0.00107351 ;
T_prep (2,1,1) =   0.00263669 ;
T_prep (2,1,2) =   0.00230506 ;
T_prep (2,1,4) =   0.00181507 ;
T_prep (2,1,8) =   0.00126725 ;
T_prep (2,1,16) =  0.000962792 ;
T_prep (2,1,32) =  0.000909411 ;
T_prep (2,1,64) =  0.000921079 ;
T_prep (2,1,128) =  0.000923659 ;
T_prep (2,1,160) =  0.000922889 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0217727 ;
Time (1,1,2) =    0.0164448 ;
Time (1,1,4) =   0.00783487 ;
Time (1,1,8) =   0.00401592 ;
Time (1,1,16) =   0.00410754 ;
Time (1,1,32) =   0.00554852 ;
Time (1,1,64) =    0.0117948 ;
Time (1,1,128) =    0.0312119 ;
Time (1,1,160) =    0.0656396 ;
Time (2,1,1) =     0.471761 ;
Time (2,1,2) =     0.211785 ;
Time (2,1,4) =    0.0962735 ;
Time (2,1,8) =    0.0432666 ;
Time (2,1,16) =    0.0319052 ;
Time (2,1,32) =     0.029512 ;
Time (2,1,64) =    0.0295145 ;
Time (2,1,128) =    0.0187588 ;
Time (2,1,160) =    0.0478938 ;
Time (3,1,1) =      1.26891 ;
Time (3,1,2) =     0.638801 ;
Time (3,1,4) =     0.362067 ;
Time (3,1,8) =     0.225101 ;
Time (3,1,16) =     0.166165 ;
Time (3,1,32) =     0.237475 ;
Time (3,1,64) =     0.128329 ;
Time (3,1,128) =     0.109406 ;
Time (3,1,160) =      0.33918 ;
Time (4,1,1) =     0.151509 ;
Time (4,1,2) =    0.0778308 ;
Time (4,1,4) =    0.0392147 ;
Time (4,1,8) =    0.0174945 ;
Time (4,1,16) =    0.0127214 ;
Time (4,1,32) =    0.0101189 ;
Time (4,1,64) =   0.00982777 ;
Time (4,1,128) =    0.0113244 ;
Time (4,1,160) =   0.00412998 ;
Time (5,1,1) =     0.137787 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-1045506-262144_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 262144 ;
Nedges (id) = 1045506 ;
Ntri (id) = 1044484 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0025859 ;
T_prep (1,1,2) =   0.00244195 ;
T_prep (1,1,4) =    0.0110729 ;
T_prep (1,1,8) =   0.00264599 ;
T_prep (1,1,16) =    0.0013358 ;
T_prep (1,1,32) =   0.00143022 ;
T_prep (1,1,64) =   0.00103856 ;
T_prep (1,1,128) =   0.00106612 ;
T_prep (1,1,160) =   0.00108406 ;
T_prep (2,1,1) =   0.00354372 ;
T_prep (2,1,2) =   0.00269456 ;
T_prep (2,1,4) =   0.00536213 ;
T_prep (2,1,8) =   0.00285249 ;
T_prep (2,1,16) =   0.00159082 ;
T_prep (2,1,32) =   0.00103708 ;
T_prep (2,1,64) =   0.00104973 ;
T_prep (2,1,128) =   0.00106219 ;
T_prep (2,1,160) =   0.00106251 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00634159 ;
Time (1,1,2) =   0.00453385 ;
Time (1,1,4) =   0.00343224 ;
Time (1,1,8) =   0.00227279 ;
Time (1,1,16) =    0.0011646 ;
Time (1,1,32) =   0.00123071 ;
Time (1,1,64) =   0.00312219 ;
Time (1,1,128) =    0.0285128 ;
Time (1,1,160) =    0.0504004 ;
Time (2,1,1) =     0.131486 ;
Time (2,1,2) =    0.0594016 ;
Time (2,1,4) =    0.0322339 ;
Time (2,1,8) =    0.0176935 ;
Time (2,1,16) =   0.00771993 ;
Time (2,1,32) =   0.00435042 ;
Time (2,1,64) =   0.00343284 ;
Time (2,1,128) =   0.00575573 ;
Time (2,1,160) =    0.0036442 ;
Time (3,1,1) =     0.127103 ;
Time (3,1,2) =    0.0687837 ;
Time (3,1,4) =    0.0342202 ;
Time (3,1,8) =    0.0151167 ;
Time (3,1,16) =   0.00571962 ;
Time (3,1,32) =   0.00311852 ;
Time (3,1,64) =   0.00251288 ;
Time (3,1,128) =    0.0158144 ;
Time (3,1,160) =   0.00107462 ;
Time (4,1,1) =    0.0996775 ;
Time (4,1,2) =    0.0473629 ;
Time (4,1,4) =    0.0254553 ;
Time (4,1,8) =    0.0116691 ;
Time (4,1,16) =    0.0182683 ;
Time (4,1,32) =   0.00591832 ;
Time (4,1,64) =   0.00454241 ;
Time (4,1,128) =   0.00234031 ;
Time (4,1,160) =   0.00238466 ;
Time (5,1,1) =    0.0677218 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1152000 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00300552 ;
T_prep (1,1,2) =   0.00300563 ;
T_prep (1,1,4) =    0.0128376 ;
T_prep (1,1,8) =   0.00246369 ;
T_prep (1,1,16) =   0.00255566 ;
T_prep (1,1,32) =   0.00330967 ;
T_prep (1,1,64) =   0.00275541 ;
T_prep (1,1,128) =   0.00274662 ;
T_prep (1,1,160) =   0.00274477 ;
T_prep (2,1,1) =   0.00546321 ;
T_prep (2,1,2) =   0.00544531 ;
T_prep (2,1,4) =   0.00483403 ;
T_prep (2,1,8) =   0.00356166 ;
T_prep (2,1,16) =   0.00244993 ;
T_prep (2,1,32) =   0.00231531 ;
T_prep (2,1,64) =   0.00230458 ;
T_prep (2,1,128) =    0.0023268 ;
T_prep (2,1,160) =   0.00236909 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00556879 ;
Time (1,1,2) =   0.00379911 ;
Time (1,1,4) =   0.00222847 ;
Time (1,1,8) =   0.00159554 ;
Time (1,1,16) =   0.00154048 ;
Time (1,1,32) =   0.00223361 ;
Time (1,1,64) =   0.00757494 ;
Time (1,1,128) =    0.0167217 ;
Time (1,1,160) =    0.0256304 ;
Time (2,1,1) =    0.0369393 ;
Time (2,1,2) =    0.0189945 ;
Time (2,1,4) =    0.0092573 ;
Time (2,1,8) =    0.0102087 ;
Time (2,1,16) =    0.0097593 ;
Time (2,1,32) =    0.0092634 ;
Time (2,1,64) =   0.00892815 ;
Time (2,1,128) =   0.00985676 ;
Time (2,1,160) =    0.0175321 ;
Time (3,1,1) =    0.0374124 ;
Time (3,1,2) =    0.0190235 ;
Time (3,1,4) =   0.00874792 ;
Time (3,1,8) =   0.00493764 ;
Time (3,1,16) =   0.00451931 ;
Time (3,1,32) =   0.00284946 ;
Time (3,1,64) =   0.00254815 ;
Time (3,1,128) =   0.00462237 ;
Time (3,1,160) =   0.00218297 ;
Time (4,1,1) =    0.0526638 ;
Time (4,1,2) =    0.0202439 ;
Time (4,1,4) =    0.0116352 ;
Time (4,1,8) =    0.0094968 ;
Time (4,1,16) =    0.0104903 ;
Time (4,1,32) =    0.0102074 ;
Time (4,1,64) =   0.00956716 ;
Time (4,1,128) =    0.0099534 ;
Time (4,1,160) =    0.0287036 ;
Time (5,1,1) =    0.0403249 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/loc-gowalla_edges/loc-gowalla_edges_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 196591 ;
Nedges (id) = 950327 ;
Ntri (id) = 2273138 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00436678 ;
T_prep (1,1,2) =   0.00435371 ;
T_prep (1,1,4) =     0.012137 ;
T_prep (1,1,8) =   0.00240218 ;
T_prep (1,1,16) =   0.00219565 ;
T_prep (1,1,32) =   0.00156354 ;
T_prep (1,1,64) =   0.00128999 ;
T_prep (1,1,128) =   0.00133218 ;
T_prep (1,1,160) =   0.00134109 ;
T_prep (2,1,1) =   0.00563705 ;
T_prep (2,1,2) =   0.00481866 ;
T_prep (2,1,4) =   0.00617902 ;
T_prep (2,1,8) =   0.00273722 ;
T_prep (2,1,16) =   0.00204726 ;
T_prep (2,1,32) =   0.00142229 ;
T_prep (2,1,64) =   0.00133377 ;
T_prep (2,1,128) =   0.00133669 ;
T_prep (2,1,160) =    0.0013489 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      0.05998 ;
Time (1,1,2) =    0.0497591 ;
Time (1,1,4) =    0.0278109 ;
Time (1,1,8) =    0.0139137 ;
Time (1,1,16) =   0.00820519 ;
Time (1,1,32) =   0.00797454 ;
Time (1,1,64) =    0.0168074 ;
Time (1,1,128) =    0.0491425 ;
Time (1,1,160) =     0.054117 ;
Time (2,1,1) =      1.44628 ;
Time (2,1,2) =     0.607936 ;
Time (2,1,4) =     0.300383 ;
Time (2,1,8) =     0.149916 ;
Time (2,1,16) =    0.0651227 ;
Time (2,1,32) =    0.0685643 ;
Time (2,1,64) =    0.0713062 ;
Time (2,1,128) =    0.0530306 ;
Time (2,1,160) =    0.0471493 ;
Time (3,1,1) =      1.25869 ;
Time (3,1,2) =     0.603984 ;
Time (3,1,4) =     0.305812 ;
Time (3,1,8) =      0.15096 ;
Time (3,1,16) =    0.0690344 ;
Time (3,1,32) =    0.0487111 ;
Time (3,1,64) =    0.0488225 ;
Time (3,1,128) =    0.0513397 ;
Time (3,1,160) =    0.0876711 ;
Time (4,1,1) =     0.411675 ;
Time (4,1,2) =     0.399942 ;
Time (4,1,4) =     0.139503 ;
Time (4,1,8) =    0.0532073 ;
Time (4,1,16) =    0.0274562 ;
Time (4,1,32) =    0.0183111 ;
Time (4,1,64) =    0.0153747 ;
Time (4,1,128) =    0.0123713 ;
Time (4,1,160) =    0.0263315 ;
Time (5,1,1) =     0.386484 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0302/amazon0302_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 262111 ;
Nedges (id) = 899792 ;
Ntri (id) = 717719 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00521327 ;
T_prep (1,1,2) =   0.00523071 ;
T_prep (1,1,4) =     0.012324 ;
T_prep (1,1,8) =   0.00177599 ;
T_prep (1,1,16) =   0.00182464 ;
T_prep (1,1,32) =   0.00135241 ;
T_prep (1,1,64) =   0.00130852 ;
T_prep (1,1,128) =     0.001337 ;
T_prep (1,1,160) =   0.00133991 ;
T_prep (2,1,1) =   0.00637619 ;
T_prep (2,1,2) =   0.00557687 ;
T_prep (2,1,4) =   0.00463654 ;
T_prep (2,1,8) =   0.00229005 ;
T_prep (2,1,16) =   0.00143684 ;
T_prep (2,1,32) =   0.00136572 ;
T_prep (2,1,64) =   0.00133479 ;
T_prep (2,1,128) =   0.00133772 ;
T_prep (2,1,160) =   0.00134503 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0197937 ;
Time (1,1,2) =    0.0141153 ;
Time (1,1,4) =   0.00754707 ;
Time (1,1,8) =   0.00411846 ;
Time (1,1,16) =   0.00202177 ;
Time (1,1,32) =   0.00193737 ;
Time (1,1,64) =   0.00388672 ;
Time (1,1,128) =    0.0250984 ;
Time (1,1,160) =    0.0261219 ;
Time (2,1,1) =     0.144905 ;
Time (2,1,2) =    0.0730427 ;
Time (2,1,4) =     0.030193 ;
Time (2,1,8) =    0.0151333 ;
Time (2,1,16) =   0.00727702 ;
Time (2,1,32) =   0.00376013 ;
Time (2,1,64) =   0.00240606 ;
Time (2,1,128) =   0.00266825 ;
Time (2,1,160) =   0.00257577 ;
Time (3,1,1) =     0.164239 ;
Time (3,1,2) =    0.0907311 ;
Time (3,1,4) =    0.0409549 ;
Time (3,1,8) =     0.021271 ;
Time (3,1,16) =    0.0104805 ;
Time (3,1,32) =   0.00594731 ;
Time (3,1,64) =    0.0036986 ;
Time (3,1,128) =   0.00270585 ;
Time (3,1,160) =    0.0142671 ;
Time (4,1,1) =    0.0950802 ;
Time (4,1,2) =    0.0504055 ;
Time (4,1,4) =    0.0229908 ;
Time (4,1,8) =    0.0118536 ;
Time (4,1,16) =   0.00587178 ;
Time (4,1,32) =   0.00353294 ;
Time (4,1,64) =    0.0028092 ;
Time (4,1,128) =   0.00319508 ;
Time (4,1,160) =   0.00294118 ;
Time (5,1,1) =    0.0763719 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1582861 ;
Ntri (id) = 3548463 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00425504 ;
T_prep (1,1,2) =   0.00407733 ;
T_prep (1,1,4) =    0.0135995 ;
T_prep (1,1,8) =    0.0030318 ;
T_prep (1,1,16) =   0.00276888 ;
T_prep (1,1,32) =   0.00501486 ;
T_prep (1,1,64) =   0.00409717 ;
T_prep (1,1,128) =   0.00403099 ;
T_prep (1,1,160) =   0.00400361 ;
T_prep (2,1,1) =   0.00766762 ;
T_prep (2,1,2) =   0.00740117 ;
T_prep (2,1,4) =   0.00698463 ;
T_prep (2,1,8) =   0.00536326 ;
T_prep (2,1,16) =   0.00388684 ;
T_prep (2,1,32) =   0.00362858 ;
T_prep (2,1,64) =   0.00343179 ;
T_prep (2,1,128) =   0.00342643 ;
T_prep (2,1,160) =   0.00349294 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.385249 ;
Time (1,1,2) =     0.358543 ;
Time (1,1,4) =     0.177045 ;
Time (1,1,8) =     0.195686 ;
Time (1,1,16) =     0.162002 ;
Time (1,1,32) =     0.307682 ;
Time (1,1,64) =     0.321261 ;
Time (1,1,128) =       1.7503 ;
Time (1,1,160) =      0.80146 ;
Time (2,1,1) =      6.79258 ;
Time (2,1,2) =      2.07916 ;
Time (2,1,4) =      1.05963 ;
Time (2,1,8) =      2.16542 ;
Time (2,1,16) =      2.05546 ;
Time (2,1,32) =      1.05305 ;
Time (2,1,64) =      2.10358 ;
Time (2,1,128) =     0.626206 ;
Time (2,1,160) =      4.17156 ;
Time (3,1,1) =      0.61216 ;
Time (3,1,2) =     0.202069 ;
Time (3,1,4) =     0.133412 ;
Time (3,1,8) =      0.11496 ;
Time (3,1,16) =    0.0972375 ;
Time (3,1,32) =    0.0790287 ;
Time (3,1,64) =    0.0911775 ;
Time (3,1,128) =     0.179273 ;
Time (3,1,160) =     0.144499 ;
Time (4,1,1) =       1.7294 ;
Time (4,1,2) =      0.65976 ;
Time (4,1,4) =      0.34084 ;
Time (4,1,8) =     0.708968 ;
Time (4,1,16) =     0.338883 ;
Time (4,1,32) =     0.355788 ;
Time (4,1,64) =      0.42648 ;
Time (4,1,128) =     0.317971 ;
Time (4,1,160) =     0.502077 ;
Time (5,1,1) =      1.70726 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-4-5-9-16-25-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 132600 ;
Nedges (id) = 1582861 ;
Ntri (id) = 155 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00422681 ;
T_prep (1,1,2) =   0.00402937 ;
T_prep (1,1,4) =    0.0125893 ;
T_prep (1,1,8) =    0.0023914 ;
T_prep (1,1,16) =   0.00293937 ;
T_prep (1,1,32) =   0.00334908 ;
T_prep (1,1,64) =   0.00254671 ;
T_prep (1,1,128) =   0.00259431 ;
T_prep (1,1,160) =   0.00254436 ;
T_prep (2,1,1) =   0.00745722 ;
T_prep (2,1,2) =   0.00745937 ;
T_prep (2,1,4) =   0.00669618 ;
T_prep (2,1,8) =   0.00449676 ;
T_prep (2,1,16) =   0.00267123 ;
T_prep (2,1,32) =   0.00235082 ;
T_prep (2,1,64) =   0.00225611 ;
T_prep (2,1,128) =   0.00232022 ;
T_prep (2,1,160) =   0.00228961 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.071389 ;
Time (1,1,2) =    0.0423307 ;
Time (1,1,4) =    0.0276777 ;
Time (1,1,8) =    0.0349364 ;
Time (1,1,16) =    0.0353715 ;
Time (1,1,32) =    0.0395774 ;
Time (1,1,64) =     0.065935 ;
Time (1,1,128) =     0.188861 ;
Time (1,1,160) =     0.147978 ;
Time (2,1,1) =      1.43035 ;
Time (2,1,2) =     0.659243 ;
Time (2,1,4) =        0.376 ;
Time (2,1,8) =     0.232809 ;
Time (2,1,16) =     0.221885 ;
Time (2,1,32) =      0.22176 ;
Time (2,1,64) =      0.22212 ;
Time (2,1,128) =     0.276972 ;
Time (2,1,160) =     0.427037 ;
Time (3,1,1) =     0.117398 ;
Time (3,1,2) =    0.0603052 ;
Time (3,1,4) =    0.0314021 ;
Time (3,1,8) =    0.0207117 ;
Time (3,1,16) =    0.0167159 ;
Time (3,1,32) =    0.0139485 ;
Time (3,1,64) =    0.0154158 ;
Time (3,1,128) =    0.0133984 ;
Time (3,1,160) =   0.00633128 ;
Time (4,1,1) =     0.426714 ;
Time (4,1,2) =     0.212889 ;
Time (4,1,4) =     0.122429 ;
Time (4,1,8) =    0.0758044 ;
Time (4,1,16) =    0.0775673 ;
Time (4,1,32) =     0.076089 ;
Time (4,1,64) =    0.0781982 ;
Time (4,1,128) =     0.061895 ;
Time (4,1,160) =     0.166462 ;
Time (5,1,1) =      1.73983 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2073600 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00585014 ;
T_prep (1,1,2) =   0.00576142 ;
T_prep (1,1,4) =    0.0154101 ;
T_prep (1,1,8) =   0.00565097 ;
T_prep (1,1,16) =   0.00533775 ;
T_prep (1,1,32) =    0.0102783 ;
T_prep (1,1,64) =     0.018544 ;
T_prep (1,1,128) =    0.0171482 ;
T_prep (1,1,160) =     0.017136 ;
T_prep (2,1,1) =    0.0190742 ;
T_prep (2,1,2) =    0.0206461 ;
T_prep (2,1,4) =    0.0182634 ;
T_prep (2,1,8) =    0.0152541 ;
T_prep (2,1,16) =    0.0137487 ;
T_prep (2,1,32) =    0.0124822 ;
T_prep (2,1,64) =    0.0138972 ;
T_prep (2,1,128) =    0.0131964 ;
T_prep (2,1,160) =    0.0131306 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.020577 ;
Time (1,1,2) =    0.0128569 ;
Time (1,1,4) =    0.0097859 ;
Time (1,1,8) =      0.01436 ;
Time (1,1,16) =   0.00973173 ;
Time (1,1,32) =    0.0151374 ;
Time (1,1,64) =    0.0262082 ;
Time (1,1,128) =    0.0365736 ;
Time (1,1,160) =    0.0537702 ;
Time (2,1,1) =    0.0685274 ;
Time (2,1,2) =    0.0350431 ;
Time (2,1,4) =    0.0312412 ;
Time (2,1,8) =     0.031953 ;
Time (2,1,16) =     0.030907 ;
Time (2,1,32) =    0.0305052 ;
Time (2,1,64) =    0.0304581 ;
Time (2,1,128) =   0.00455725 ;
Time (2,1,160) =   0.00621114 ;
Time (3,1,1) =    0.0706778 ;
Time (3,1,2) =    0.0350471 ;
Time (3,1,4) =    0.0163173 ;
Time (3,1,8) =   0.00783719 ;
Time (3,1,16) =   0.00418404 ;
Time (3,1,32) =    0.0023527 ;
Time (3,1,64) =   0.00224778 ;
Time (3,1,128) =   0.00150956 ;
Time (3,1,160) =   0.00262082 ;
Time (4,1,1) =    0.0948715 ;
Time (4,1,2) =    0.0370388 ;
Time (4,1,4) =    0.0300053 ;
Time (4,1,8) =    0.0322622 ;
Time (4,1,16) =   0.00832422 ;
Time (4,1,32) =    0.0314252 ;
Time (4,1,64) =   0.00645046 ;
Time (4,1,128) =    0.0063826 ;
Time (4,1,160) =    0.0069354 ;
Time (5,1,1) =    0.0793013 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2132284 ;
Ntri (id) = 2102761 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00581437 ;
T_prep (1,1,2) =   0.00585196 ;
T_prep (1,1,4) =    0.0156757 ;
T_prep (1,1,8) =   0.00592812 ;
T_prep (1,1,16) =   0.00582164 ;
T_prep (1,1,32) =    0.0180698 ;
T_prep (1,1,64) =    0.0191943 ;
T_prep (1,1,128) =    0.0179954 ;
T_prep (1,1,160) =     0.018098 ;
T_prep (2,1,1) =    0.0196883 ;
T_prep (2,1,2) =    0.0209961 ;
T_prep (2,1,4) =     0.018784 ;
T_prep (2,1,8) =     0.015918 ;
T_prep (2,1,16) =    0.0142169 ;
T_prep (2,1,32) =    0.0155224 ;
T_prep (2,1,64) =     0.014312 ;
T_prep (2,1,128) =    0.0138653 ;
T_prep (2,1,160) =    0.0138004 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.202565 ;
Time (1,1,2) =    0.0949434 ;
Time (1,1,4) =    0.0940531 ;
Time (1,1,8) =    0.0940585 ;
Time (1,1,16) =    0.0939919 ;
Time (1,1,32) =     0.097056 ;
Time (1,1,64) =     0.131652 ;
Time (1,1,128) =     0.230444 ;
Time (1,1,160) =      1.12766 ;
Time (2,1,1) =      1.47429 ;
Time (2,1,2) =     0.561043 ;
Time (2,1,4) =     0.560776 ;
Time (2,1,8) =      1.29807 ;
Time (2,1,16) =     0.543608 ;
Time (2,1,32) =      1.40004 ;
Time (2,1,64) =     0.647568 ;
Time (2,1,128) =      1.90365 ;
Time (2,1,160) =      1.02317 ;
Time (3,1,1) =     0.333222 ;
Time (3,1,2) =     0.128471 ;
Time (3,1,4) =    0.0565105 ;
Time (3,1,8) =    0.0293349 ;
Time (3,1,16) =    0.0162672 ;
Time (3,1,32) =    0.0119396 ;
Time (3,1,64) =    0.0135542 ;
Time (3,1,128) =    0.0145512 ;
Time (3,1,160) =    0.0231469 ;
Time (4,1,1) =     0.571107 ;
Time (4,1,2) =     0.402266 ;
Time (4,1,4) =     0.422026 ;
Time (4,1,8) =     0.520023 ;
Time (4,1,16) =      0.29579 ;
Time (4,1,32) =     0.517058 ;
Time (4,1,64) =     0.254872 ;
Time (4,1,128) =     0.620642 ;
Time (4,1,160) =     0.407686 ;
Time (5,1,1) =     0.525207 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-25-81-256-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 547924 ;
Nedges (id) = 2132284 ;
Ntri (id) = 7 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00581356 ;
T_prep (1,1,2) =   0.00580959 ;
T_prep (1,1,4) =    0.0153883 ;
T_prep (1,1,8) =   0.00558525 ;
T_prep (1,1,16) =   0.00544589 ;
T_prep (1,1,32) =   0.00989687 ;
T_prep (1,1,64) =     0.018222 ;
T_prep (1,1,128) =    0.0169707 ;
T_prep (1,1,160) =    0.0169625 ;
T_prep (2,1,1) =    0.0192766 ;
T_prep (2,1,2) =    0.0209125 ;
T_prep (2,1,4) =    0.0180916 ;
T_prep (2,1,8) =    0.0151141 ;
T_prep (2,1,16) =    0.0154497 ;
T_prep (2,1,32) =    0.0140766 ;
T_prep (2,1,64) =    0.0130022 ;
T_prep (2,1,128) =    0.0126652 ;
T_prep (2,1,160) =    0.0126125 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0274571 ;
Time (1,1,2) =    0.0143981 ;
Time (1,1,4) =    0.0101277 ;
Time (1,1,8) =   0.00973537 ;
Time (1,1,16) =    0.0102537 ;
Time (1,1,32) =    0.0112591 ;
Time (1,1,64) =     0.025657 ;
Time (1,1,128) =    0.0463475 ;
Time (1,1,160) =    0.0767367 ;
Time (2,1,1) =     0.142557 ;
Time (2,1,2) =    0.0802758 ;
Time (2,1,4) =     0.055893 ;
Time (2,1,8) =    0.0462093 ;
Time (2,1,16) =    0.0511645 ;
Time (2,1,32) =    0.0456144 ;
Time (2,1,64) =    0.0630353 ;
Time (2,1,128) =    0.0442146 ;
Time (2,1,160) =    0.0459817 ;
Time (3,1,1) =    0.0816554 ;
Time (3,1,2) =    0.0435001 ;
Time (3,1,4) =     0.021627 ;
Time (3,1,8) =    0.0109276 ;
Time (3,1,16) =   0.00521113 ;
Time (3,1,32) =    0.0030554 ;
Time (3,1,64) =   0.00183966 ;
Time (3,1,128) =   0.00276174 ;
Time (3,1,160) =   0.00161473 ;
Time (4,1,1) =     0.111748 ;
Time (4,1,2) =    0.0474399 ;
Time (4,1,4) =    0.0322881 ;
Time (4,1,8) =    0.0310671 ;
Time (4,1,16) =     0.032635 ;
Time (4,1,32) =    0.0202894 ;
Time (4,1,64) =    0.0172521 ;
Time (4,1,128) =    0.0174848 ;
Time (4,1,160) =     0.018012 ;
Time (5,1,1) =      1.72912 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-PA/roadNet-PA_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1088092 ;
Nedges (id) = 1541898 ;
Ntri (id) = 67150 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0130421 ;
T_prep (1,1,2) =     0.013084 ;
T_prep (1,1,4) =    0.0188163 ;
T_prep (1,1,8) =   0.00668851 ;
T_prep (1,1,16) =   0.00376795 ;
T_prep (1,1,32) =   0.00415774 ;
T_prep (1,1,64) =   0.00281698 ;
T_prep (1,1,128) =   0.00281398 ;
T_prep (1,1,160) =   0.00280115 ;
T_prep (2,1,1) =     0.017853 ;
T_prep (2,1,2) =    0.0170517 ;
T_prep (2,1,4) =    0.0174547 ;
T_prep (2,1,8) =   0.00999241 ;
T_prep (2,1,16) =   0.00536628 ;
T_prep (2,1,32) =    0.0028988 ;
T_prep (2,1,64) =   0.00287944 ;
T_prep (2,1,128) =   0.00292832 ;
T_prep (2,1,160) =   0.00288277 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0344599 ;
Time (1,1,2) =    0.0224991 ;
Time (1,1,4) =    0.0132538 ;
Time (1,1,8) =   0.00611715 ;
Time (1,1,16) =     0.003275 ;
Time (1,1,32) =   0.00195842 ;
Time (1,1,64) =   0.00431521 ;
Time (1,1,128) =    0.0327795 ;
Time (1,1,160) =    0.0356767 ;
Time (2,1,1) =     0.156339 ;
Time (2,1,2) =    0.0682177 ;
Time (2,1,4) =    0.0326163 ;
Time (2,1,8) =    0.0162276 ;
Time (2,1,16) =   0.00910955 ;
Time (2,1,32) =    0.0050738 ;
Time (2,1,64) =   0.00568458 ;
Time (2,1,128) =   0.00379224 ;
Time (2,1,160) =    0.0139449 ;
Time (3,1,1) =      0.11664 ;
Time (3,1,2) =    0.0532718 ;
Time (3,1,4) =    0.0268049 ;
Time (3,1,8) =     0.013017 ;
Time (3,1,16) =   0.00679591 ;
Time (3,1,32) =   0.00343817 ;
Time (3,1,64) =   0.00205548 ;
Time (3,1,128) =   0.00654334 ;
Time (3,1,160) =     0.015815 ;
Time (4,1,1) =     0.157581 ;
Time (4,1,2) =      0.07383 ;
Time (4,1,4) =    0.0352946 ;
Time (4,1,8) =    0.0179832 ;
Time (4,1,16) =    0.0245235 ;
Time (4,1,32) =    0.0117111 ;
Time (4,1,64) =    0.0040599 ;
Time (4,1,128) =    0.0270624 ;
Time (4,1,160) =    0.0119981 ;
Time (5,1,1) =     0.139601 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2332800 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00636225 ;
T_prep (1,1,2) =   0.00637345 ;
T_prep (1,1,4) =    0.0153051 ;
T_prep (1,1,8) =   0.00557194 ;
T_prep (1,1,16) =   0.00493238 ;
T_prep (1,1,32) =   0.00526962 ;
T_prep (1,1,64) =    0.0157191 ;
T_prep (1,1,128) =    0.0143423 ;
T_prep (1,1,160) =    0.0142736 ;
T_prep (2,1,1) =    0.0204884 ;
T_prep (2,1,2) =    0.0218789 ;
T_prep (2,1,4) =    0.0187402 ;
T_prep (2,1,8) =    0.0150649 ;
T_prep (2,1,16) =    0.0115648 ;
T_prep (2,1,32) =   0.00944422 ;
T_prep (2,1,64) =    0.0111258 ;
T_prep (2,1,128) =    0.0108035 ;
T_prep (2,1,160) =    0.0107952 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0216844 ;
Time (1,1,2) =    0.0140734 ;
Time (1,1,4) =   0.00809054 ;
Time (1,1,8) =   0.00756711 ;
Time (1,1,16) =   0.00755056 ;
Time (1,1,32) =    0.0120744 ;
Time (1,1,64) =    0.0151759 ;
Time (1,1,128) =    0.0563034 ;
Time (1,1,160) =    0.0338688 ;
Time (2,1,1) =    0.0765905 ;
Time (2,1,2) =    0.0348562 ;
Time (2,1,4) =    0.0243779 ;
Time (2,1,8) =    0.0232554 ;
Time (2,1,16) =    0.0253437 ;
Time (2,1,32) =    0.0246288 ;
Time (2,1,64) =    0.0236838 ;
Time (2,1,128) =    0.0045313 ;
Time (2,1,160) =    0.0341843 ;
Time (3,1,1) =    0.0736678 ;
Time (3,1,2) =    0.0162251 ;
Time (3,1,4) =     0.010854 ;
Time (3,1,8) =   0.00818765 ;
Time (3,1,16) =   0.00390928 ;
Time (3,1,32) =   0.00362136 ;
Time (3,1,64) =   0.00330369 ;
Time (3,1,128) =   0.00928553 ;
Time (3,1,160) =   0.00320594 ;
Time (4,1,1) =    0.0966216 ;
Time (4,1,2) =    0.0178048 ;
Time (4,1,4) =    0.0283077 ;
Time (4,1,8) =   0.00838364 ;
Time (4,1,16) =   0.00803552 ;
Time (4,1,32) =    0.0255533 ;
Time (4,1,64) =   0.00747302 ;
Time (4,1,128) =    0.0076523 ;
Time (4,1,160) =   0.00752271 ;
Time (5,1,1) =     0.076459 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2606125 ;
Ntri (id) = 4059175 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00676676 ;
T_prep (1,1,2) =    0.0067422 ;
T_prep (1,1,4) =    0.0160538 ;
T_prep (1,1,8) =   0.00629231 ;
T_prep (1,1,16) =   0.00599575 ;
T_prep (1,1,32) =   0.00596725 ;
T_prep (1,1,64) =    0.0229443 ;
T_prep (1,1,128) =    0.0210596 ;
T_prep (1,1,160) =    0.0209625 ;
T_prep (2,1,1) =     0.033058 ;
T_prep (2,1,2) =      0.03465 ;
T_prep (2,1,4) =    0.0333459 ;
T_prep (2,1,8) =    0.0275859 ;
T_prep (2,1,16) =    0.0209869 ;
T_prep (2,1,32) =    0.0171852 ;
T_prep (2,1,64) =    0.0182327 ;
T_prep (2,1,128) =    0.0175232 ;
T_prep (2,1,160) =    0.0176999 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.314431 ;
Time (1,1,2) =     0.558036 ;
Time (1,1,4) =     0.596496 ;
Time (1,1,8) =     0.482567 ;
Time (1,1,16) =     0.635345 ;
Time (1,1,32) =     0.879853 ;
Time (1,1,64) =     0.592038 ;
Time (1,1,128) =     0.805994 ;
Time (1,1,160) =      1.86151 ;
Time (2,1,1) =         6.84 ;
Time (2,1,2) =      3.30948 ;
Time (2,1,4) =      3.30043 ;
Time (2,1,8) =      3.29753 ;
Time (2,1,16) =      3.29861 ;
Time (2,1,32) =      3.29586 ;
Time (2,1,64) =      3.39324 ;
Time (2,1,128) =      4.17302 ;
Time (2,1,160) =      4.00911 ;
Time (3,1,1) =     0.687292 ;
Time (3,1,2) =     0.352086 ;
Time (3,1,4) =     0.195438 ;
Time (3,1,8) =     0.112742 ;
Time (3,1,16) =     0.071291 ;
Time (3,1,32) =    0.0549901 ;
Time (3,1,64) =    0.0515991 ;
Time (3,1,128) =    0.0522289 ;
Time (3,1,160) =    0.0546148 ;
Time (4,1,1) =      2.30874 ;
Time (4,1,2) =      1.19787 ;
Time (4,1,4) =      1.20097 ;
Time (4,1,8) =      1.19873 ;
Time (4,1,16) =      1.19767 ;
Time (4,1,32) =      1.19748 ;
Time (4,1,64) =      1.19621 ;
Time (4,1,128) =      1.49431 ;
Time (4,1,160) =      1.41501 ;
Time (5,1,1) =      2.30331 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-9-16-25-81-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 362440 ;
Nedges (id) = 2606125 ;
Ntri (id) = 35 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00678208 ;
T_prep (1,1,2) =   0.00684832 ;
T_prep (1,1,4) =    0.0157099 ;
T_prep (1,1,8) =   0.00574063 ;
T_prep (1,1,16) =   0.00617962 ;
T_prep (1,1,32) =   0.00774702 ;
T_prep (1,1,64) =    0.0155325 ;
T_prep (1,1,128) =    0.0138488 ;
T_prep (1,1,160) =     0.013961 ;
T_prep (2,1,1) =    0.0228378 ;
T_prep (2,1,2) =    0.0258502 ;
T_prep (2,1,4) =    0.0197316 ;
T_prep (2,1,8) =    0.0159243 ;
T_prep (2,1,16) =    0.0119731 ;
T_prep (2,1,32) =   0.00965657 ;
T_prep (2,1,64) =    0.0112913 ;
T_prep (2,1,128) =    0.0105341 ;
T_prep (2,1,160) =    0.0105298 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.137569 ;
Time (1,1,2) =      0.10038 ;
Time (1,1,4) =     0.076617 ;
Time (1,1,8) =    0.0914926 ;
Time (1,1,16) =    0.0708951 ;
Time (1,1,32) =    0.0604847 ;
Time (1,1,64) =    0.0922131 ;
Time (1,1,128) =     0.134618 ;
Time (1,1,160) =     0.338065 ;
Time (2,1,1) =     0.985142 ;
Time (2,1,2) =     0.482943 ;
Time (2,1,4) =     0.374809 ;
Time (2,1,8) =     0.397267 ;
Time (2,1,16) =      0.39246 ;
Time (2,1,32) =     0.387815 ;
Time (2,1,64) =       0.7826 ;
Time (2,1,128) =     0.757472 ;
Time (2,1,160) =     0.735689 ;
Time (3,1,1) =     0.121396 ;
Time (3,1,2) =    0.0616753 ;
Time (3,1,4) =    0.0321043 ;
Time (3,1,8) =    0.0173116 ;
Time (3,1,16) =   0.00749901 ;
Time (3,1,32) =   0.00641039 ;
Time (3,1,64) =   0.00586579 ;
Time (3,1,128) =   0.00547727 ;
Time (3,1,160) =   0.00592005 ;
Time (4,1,1) =     0.372978 ;
Time (4,1,2) =     0.183547 ;
Time (4,1,4) =     0.152549 ;
Time (4,1,8) =     0.146056 ;
Time (4,1,16) =      0.12826 ;
Time (4,1,32) =     0.119132 ;
Time (4,1,64) =       0.1295 ;
Time (4,1,128) =     0.138903 ;
Time (4,1,160) =     0.245216 ;
Time (5,1,1) =      3.64282 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-TX/roadNet-TX_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1379917 ;
Nedges (id) = 1921660 ;
Ntri (id) = 82869 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0167317 ;
T_prep (1,1,2) =    0.0166424 ;
T_prep (1,1,4) =    0.0190878 ;
T_prep (1,1,8) =   0.00858599 ;
T_prep (1,1,16) =   0.00465879 ;
T_prep (1,1,32) =   0.00429534 ;
T_prep (1,1,64) =   0.00467798 ;
T_prep (1,1,128) =   0.00393525 ;
T_prep (1,1,160) =   0.00395836 ;
T_prep (2,1,1) =    0.0334526 ;
T_prep (2,1,2) =    0.0333289 ;
T_prep (2,1,4) =    0.0245245 ;
T_prep (2,1,8) =    0.0138308 ;
T_prep (2,1,16) =   0.00785933 ;
T_prep (2,1,32) =   0.00476486 ;
T_prep (2,1,64) =   0.00392183 ;
T_prep (2,1,128) =   0.00388551 ;
T_prep (2,1,160) =   0.00388517 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0590512 ;
Time (1,1,2) =    0.0368669 ;
Time (1,1,4) =    0.0194862 ;
Time (1,1,8) =   0.00881807 ;
Time (1,1,16) =   0.00459813 ;
Time (1,1,32) =   0.00267351 ;
Time (1,1,64) =   0.00476837 ;
Time (1,1,128) =    0.0285632 ;
Time (1,1,160) =    0.0268853 ;
Time (2,1,1) =     0.154427 ;
Time (2,1,2) =    0.0779301 ;
Time (2,1,4) =    0.0381475 ;
Time (2,1,8) =    0.0191344 ;
Time (2,1,16) =   0.00935377 ;
Time (2,1,32) =   0.00482174 ;
Time (2,1,64) =   0.00746379 ;
Time (2,1,128) =   0.00381175 ;
Time (2,1,160) =   0.00316306 ;
Time (3,1,1) =     0.107198 ;
Time (3,1,2) =     0.058279 ;
Time (3,1,4) =    0.0403862 ;
Time (3,1,8) =    0.0164752 ;
Time (3,1,16) =   0.00742051 ;
Time (3,1,32) =   0.00479245 ;
Time (3,1,64) =   0.00264665 ;
Time (3,1,128) =   0.00401151 ;
Time (3,1,160) =   0.00132607 ;
Time (4,1,1) =     0.144659 ;
Time (4,1,2) =    0.0752406 ;
Time (4,1,4) =     0.037983 ;
Time (4,1,8) =    0.0192117 ;
Time (4,1,16) =   0.00994465 ;
Time (4,1,32) =   0.00594927 ;
Time (4,1,64) =   0.00527619 ;
Time (4,1,128) =    0.0111844 ;
Time (4,1,160) =   0.00991097 ;
Time (5,1,1) =     0.131943 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-4188162-1048576_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1048576 ;
Nedges (id) = 4188162 ;
Ntri (id) = 4186116 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0102608 ;
T_prep (1,1,2) =    0.0101897 ;
T_prep (1,1,4) =    0.0164527 ;
T_prep (1,1,8) =   0.00730668 ;
T_prep (1,1,16) =   0.00542901 ;
T_prep (1,1,32) =   0.00462299 ;
T_prep (1,1,64) =   0.00812819 ;
T_prep (1,1,128) =    0.0118654 ;
T_prep (1,1,160) =   0.00954297 ;
T_prep (2,1,1) =    0.0906326 ;
T_prep (2,1,2) =     0.103555 ;
T_prep (2,1,4) =    0.0511914 ;
T_prep (2,1,8) =    0.0282216 ;
T_prep (2,1,16) =    0.0224775 ;
T_prep (2,1,32) =    0.0133362 ;
T_prep (2,1,64) =    0.0087397 ;
T_prep (2,1,128) =   0.00946585 ;
T_prep (2,1,160) =    0.0087136 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.244141 ;
Time (1,1,2) =     0.122453 ;
Time (1,1,4) =    0.0590693 ;
Time (1,1,8) =      0.02691 ;
Time (1,1,16) =   0.00967758 ;
Time (1,1,32) =   0.00596277 ;
Time (1,1,64) =   0.00331729 ;
Time (1,1,128) =    0.0179395 ;
Time (1,1,160) =    0.0457568 ;
Time (2,1,1) =     0.540118 ;
Time (2,1,2) =     0.269373 ;
Time (2,1,4) =     0.128107 ;
Time (2,1,8) =    0.0574667 ;
Time (2,1,16) =    0.0165492 ;
Time (2,1,32) =    0.0101952 ;
Time (2,1,64) =     0.027625 ;
Time (2,1,128) =    0.0121955 ;
Time (2,1,160) =     0.007491 ;
Time (3,1,1) =     0.518469 ;
Time (3,1,2) =      0.27234 ;
Time (3,1,4) =     0.124557 ;
Time (3,1,8) =    0.0590581 ;
Time (3,1,16) =     0.016854 ;
Time (3,1,32) =   0.00991509 ;
Time (3,1,64) =   0.00479947 ;
Time (3,1,128) =   0.00701472 ;
Time (3,1,160) =   0.00287891 ;
Time (4,1,1) =      0.36519 ;
Time (4,1,2) =     0.187891 ;
Time (4,1,4) =    0.0903946 ;
Time (4,1,8) =    0.0427114 ;
Time (4,1,16) =    0.0133102 ;
Time (4,1,32) =   0.00806746 ;
Time (4,1,64) =   0.00588118 ;
Time (4,1,128) =   0.00986426 ;
Time (4,1,160) =   0.00846015 ;
Time (5,1,1) =     0.239631 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/flickrEdges/flickrEdges_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 105938 ;
Nedges (id) = 2316948 ;
Ntri (id) = 107987357 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =   0.00694227 ;
T_prep (1,1,2) =   0.00684254 ;
T_prep (1,1,4) =    0.0131552 ;
T_prep (1,1,8) =   0.00339782 ;
T_prep (1,1,16) =   0.00245854 ;
T_prep (1,1,32) =   0.00254232 ;
T_prep (1,1,64) =   0.00265594 ;
T_prep (1,1,128) =   0.00169991 ;
T_prep (1,1,160) =    0.0017372 ;
T_prep (2,1,1) =    0.0179018 ;
T_prep (2,1,2) =    0.0185316 ;
T_prep (2,1,4) =    0.0140269 ;
T_prep (2,1,8) =   0.00795349 ;
T_prep (2,1,16) =   0.00399847 ;
T_prep (2,1,32) =   0.00301298 ;
T_prep (2,1,64) =   0.00182409 ;
T_prep (2,1,128) =   0.00175269 ;
T_prep (2,1,160) =   0.00175182 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.571974 ;
Time (1,1,2) =     0.354074 ;
Time (1,1,4) =     0.193203 ;
Time (1,1,8) =     0.108167 ;
Time (1,1,16) =    0.0777944 ;
Time (1,1,32) =    0.0544817 ;
Time (1,1,64) =    0.0970849 ;
Time (1,1,128) =     0.226809 ;
Time (1,1,160) =     0.168278 ;
Time (2,1,1) =      4.39553 ;
Time (2,1,2) =        2.419 ;
Time (2,1,4) =      1.13736 ;
Time (2,1,8) =     0.415594 ;
Time (2,1,16) =     0.379776 ;
Time (2,1,32) =     0.308436 ;
Time (2,1,64) =     0.321417 ;
Time (2,1,128) =     0.173057 ;
Time (2,1,160) =     0.114208 ;
Time (3,1,1) =      17.9403 ;
Time (3,1,2) =      8.61716 ;
Time (3,1,4) =      4.11652 ;
Time (3,1,8) =       1.8753 ;
Time (3,1,16) =     0.994954 ;
Time (3,1,32) =     0.709278 ;
Time (3,1,64) =     0.739635 ;
Time (3,1,128) =     0.865371 ;
Time (3,1,160) =     0.979139 ;
Time (4,1,1) =      1.44246 ;
Time (4,1,2) =     0.703269 ;
Time (4,1,4) =     0.367338 ;
Time (4,1,8) =     0.190166 ;
Time (4,1,16) =      0.11941 ;
Time (4,1,32) =    0.0846716 ;
Time (4,1,64) =     0.088963 ;
Time (4,1,128) =     0.109825 ;
Time (4,1,160) =    0.0991353 ;
Time (5,1,1) =      1.37456 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0312/amazon0312_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 400727 ;
Nedges (id) = 2349869 ;
Ntri (id) = 3686467 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0113659 ;
T_prep (1,1,2) =     0.011366 ;
T_prep (1,1,4) =    0.0145757 ;
T_prep (1,1,8) =   0.00340593 ;
T_prep (1,1,16) =   0.00344906 ;
T_prep (1,1,32) =   0.00355943 ;
T_prep (1,1,64) =   0.00314335 ;
T_prep (1,1,128) =   0.00227642 ;
T_prep (1,1,160) =   0.00232378 ;
T_prep (2,1,1) =    0.0289057 ;
T_prep (2,1,2) =     0.030167 ;
T_prep (2,1,4) =    0.0189122 ;
T_prep (2,1,8) =   0.00927401 ;
T_prep (2,1,16) =   0.00472014 ;
T_prep (2,1,32) =   0.00351069 ;
T_prep (2,1,64) =   0.00255176 ;
T_prep (2,1,128) =   0.00241431 ;
T_prep (2,1,160) =   0.00239407 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.172312 ;
Time (1,1,2) =    0.0983099 ;
Time (1,1,4) =    0.0487437 ;
Time (1,1,8) =    0.0249179 ;
Time (1,1,16) =    0.0124748 ;
Time (1,1,32) =   0.00658247 ;
Time (1,1,64) =   0.00702824 ;
Time (1,1,128) =    0.0852088 ;
Time (1,1,160) =     0.084961 ;
Time (2,1,1) =     0.559312 ;
Time (2,1,2) =     0.277274 ;
Time (2,1,4) =     0.139551 ;
Time (2,1,8) =    0.0639458 ;
Time (2,1,16) =    0.0311535 ;
Time (2,1,32) =    0.0161228 ;
Time (2,1,64) =    0.0118942 ;
Time (2,1,128) =   0.00752496 ;
Time (2,1,160) =    0.0105231 ;
Time (3,1,1) =     0.837424 ;
Time (3,1,2) =      0.41661 ;
Time (3,1,4) =     0.201299 ;
Time (3,1,8) =     0.100183 ;
Time (3,1,16) =    0.0509282 ;
Time (3,1,32) =    0.0322192 ;
Time (3,1,64) =    0.0191333 ;
Time (3,1,128) =    0.0140551 ;
Time (3,1,160) =    0.0214507 ;
Time (4,1,1) =     0.330633 ;
Time (4,1,2) =      0.16218 ;
Time (4,1,4) =    0.0820057 ;
Time (4,1,8) =    0.0408925 ;
Time (4,1,16) =    0.0207309 ;
Time (4,1,32) =    0.0113903 ;
Time (4,1,64) =   0.00740033 ;
Time (4,1,128) =    0.0116046 ;
Time (4,1,160) =    0.0124206 ;
Time (5,1,1) =      0.26584 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0505/amazon0505_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 410236 ;
Nedges (id) = 2439437 ;
Ntri (id) = 3951063 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0116867 ;
T_prep (1,1,2) =    0.0117003 ;
T_prep (1,1,4) =     0.015862 ;
T_prep (1,1,8) =   0.00613706 ;
T_prep (1,1,16) =   0.00282628 ;
T_prep (1,1,32) =   0.00286329 ;
T_prep (1,1,64) =   0.00392537 ;
T_prep (1,1,128) =   0.00258618 ;
T_prep (1,1,160) =    0.0024913 ;
T_prep (2,1,1) =    0.0298989 ;
T_prep (2,1,2) =     0.030669 ;
T_prep (2,1,4) =    0.0161587 ;
T_prep (2,1,8) =     0.013316 ;
T_prep (2,1,16) =   0.00651406 ;
T_prep (2,1,32) =   0.00368896 ;
T_prep (2,1,64) =   0.00239257 ;
T_prep (2,1,128) =   0.00229042 ;
T_prep (2,1,160) =    0.0023355 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      0.16863 ;
Time (1,1,2) =    0.0923113 ;
Time (1,1,4) =    0.0465936 ;
Time (1,1,8) =    0.0243261 ;
Time (1,1,16) =    0.0124884 ;
Time (1,1,32) =   0.00665683 ;
Time (1,1,64) =   0.00698479 ;
Time (1,1,128) =    0.0258919 ;
Time (1,1,160) =    0.0710699 ;
Time (2,1,1) =     0.499956 ;
Time (2,1,2) =     0.277444 ;
Time (2,1,4) =     0.136599 ;
Time (2,1,8) =    0.0631249 ;
Time (2,1,16) =    0.0311984 ;
Time (2,1,32) =    0.0156373 ;
Time (2,1,64) =    0.0114534 ;
Time (2,1,128) =     0.011653 ;
Time (2,1,160) =   0.00602663 ;
Time (3,1,1) =     0.855699 ;
Time (3,1,2) =     0.434453 ;
Time (3,1,4) =     0.179784 ;
Time (3,1,8) =    0.0995649 ;
Time (3,1,16) =    0.0554546 ;
Time (3,1,32) =    0.0287641 ;
Time (3,1,64) =    0.0192962 ;
Time (3,1,128) =    0.0142742 ;
Time (3,1,160) =     0.013502 ;
Time (4,1,1) =     0.298118 ;
Time (4,1,2) =     0.155012 ;
Time (4,1,4) =    0.0732181 ;
Time (4,1,8) =    0.0381028 ;
Time (4,1,16) =     0.019136 ;
Time (4,1,32) =    0.0103871 ;
Time (4,1,64) =   0.00688043 ;
Time (4,1,128) =   0.00897387 ;
Time (4,1,160) =   0.00795015 ;
Time (5,1,1) =     0.252209 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/amazon0601/amazon0601_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 403394 ;
Nedges (id) = 2443408 ;
Ntri (id) = 3986507 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0116189 ;
T_prep (1,1,2) =     0.011633 ;
T_prep (1,1,4) =    0.0148011 ;
T_prep (1,1,8) =   0.00410262 ;
T_prep (1,1,16) =   0.00360363 ;
T_prep (1,1,32) =   0.00356614 ;
T_prep (1,1,64) =   0.00342273 ;
T_prep (1,1,128) =   0.00194996 ;
T_prep (1,1,160) =   0.00194731 ;
T_prep (2,1,1) =     0.029552 ;
T_prep (2,1,2) =    0.0299247 ;
T_prep (2,1,4) =    0.0171612 ;
T_prep (2,1,8) =   0.00927561 ;
T_prep (2,1,16) =   0.00660558 ;
T_prep (2,1,32) =   0.00349007 ;
T_prep (2,1,64) =   0.00194676 ;
T_prep (2,1,128) =   0.00193379 ;
T_prep (2,1,160) =   0.00193067 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.162263 ;
Time (1,1,2) =    0.0965633 ;
Time (1,1,4) =    0.0464077 ;
Time (1,1,8) =    0.0236495 ;
Time (1,1,16) =    0.0121174 ;
Time (1,1,32) =   0.00671706 ;
Time (1,1,64) =   0.00709082 ;
Time (1,1,128) =    0.0379592 ;
Time (1,1,160) =    0.0829198 ;
Time (2,1,1) =      0.61671 ;
Time (2,1,2) =     0.317023 ;
Time (2,1,4) =      0.16004 ;
Time (2,1,8) =    0.0713208 ;
Time (2,1,16) =    0.0323311 ;
Time (2,1,32) =    0.0167164 ;
Time (2,1,64) =    0.0138228 ;
Time (2,1,128) =    0.0207853 ;
Time (2,1,160) =    0.0131975 ;
Time (3,1,1) =       1.0867 ;
Time (3,1,2) =     0.533924 ;
Time (3,1,4) =     0.275713 ;
Time (3,1,8) =      0.12744 ;
Time (3,1,16) =    0.0652917 ;
Time (3,1,32) =    0.0450112 ;
Time (3,1,64) =    0.0202006 ;
Time (3,1,128) =    0.0315081 ;
Time (3,1,160) =    0.0264923 ;
Time (4,1,1) =     0.363153 ;
Time (4,1,2) =     0.180682 ;
Time (4,1,4) =    0.0923873 ;
Time (4,1,8) =    0.0449918 ;
Time (4,1,16) =    0.0202523 ;
Time (4,1,32) =    0.0200764 ;
Time (4,1,64) =   0.00699292 ;
Time (4,1,128) =    0.0117855 ;
Time (4,1,160) =    0.0174216 ;
Time (5,1,1) =     0.274425 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/roadNet-CA/roadNet-CA_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1965206 ;
Nedges (id) = 2766607 ;
Ntri (id) = 120676 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0234956 ;
T_prep (1,1,2) =    0.0235299 ;
T_prep (1,1,4) =    0.0228437 ;
T_prep (1,1,8) =    0.0119594 ;
T_prep (1,1,16) =   0.00634628 ;
T_prep (1,1,32) =   0.00476367 ;
T_prep (1,1,64) =   0.00870064 ;
T_prep (1,1,128) =   0.00683891 ;
T_prep (1,1,160) =   0.00675973 ;
T_prep (2,1,1) =    0.0705656 ;
T_prep (2,1,2) =    0.0789285 ;
T_prep (2,1,4) =     0.040554 ;
T_prep (2,1,8) =    0.0226188 ;
T_prep (2,1,16) =    0.0138623 ;
T_prep (2,1,32) =   0.00942071 ;
T_prep (2,1,64) =   0.00730911 ;
T_prep (2,1,128) =   0.00718411 ;
T_prep (2,1,160) =   0.00718267 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.129112 ;
Time (1,1,2) =    0.0667417 ;
Time (1,1,4) =    0.0340142 ;
Time (1,1,8) =    0.0170196 ;
Time (1,1,16) =   0.00926915 ;
Time (1,1,32) =   0.00511129 ;
Time (1,1,64) =   0.00539793 ;
Time (1,1,128) =    0.0310408 ;
Time (1,1,160) =    0.0490338 ;
Time (2,1,1) =     0.261994 ;
Time (2,1,2) =     0.118297 ;
Time (2,1,4) =    0.0576401 ;
Time (2,1,8) =    0.0457755 ;
Time (2,1,16) =    0.0155311 ;
Time (2,1,32) =   0.00774675 ;
Time (2,1,64) =   0.00556196 ;
Time (2,1,128) =    0.0146293 ;
Time (2,1,160) =    0.0057504 ;
Time (3,1,1) =     0.186103 ;
Time (3,1,2) =    0.0911104 ;
Time (3,1,4) =    0.0450676 ;
Time (3,1,8) =    0.0207034 ;
Time (3,1,16) =    0.0110722 ;
Time (3,1,32) =   0.00685373 ;
Time (3,1,64) =   0.00374898 ;
Time (3,1,128) =   0.00471533 ;
Time (3,1,160) =   0.00404487 ;
Time (4,1,1) =     0.264362 ;
Time (4,1,2) =      0.11445 ;
Time (4,1,4) =    0.0567626 ;
Time (4,1,8) =     0.027554 ;
Time (4,1,16) =     0.014312 ;
Time (4,1,32) =   0.00898556 ;
Time (4,1,64) =    0.0141574 ;
Time (4,1,128) =    0.0152058 ;
Time (4,1,160) =    0.0190683 ;
Time (5,1,1) =     0.196909 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale18-ef16/graph500-scale18-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 174147 ;
Nedges (id) = 3800348 ;
Ntri (id) = 82287285 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0115195 ;
T_prep (1,1,2) =    0.0114654 ;
T_prep (1,1,4) =    0.0171173 ;
T_prep (1,1,8) =   0.00912284 ;
T_prep (1,1,16) =   0.00422869 ;
T_prep (1,1,32) =   0.00635442 ;
T_prep (1,1,64) =   0.00767349 ;
T_prep (1,1,128) =   0.00940288 ;
T_prep (1,1,160) =    0.0081205 ;
T_prep (2,1,1) =    0.0580555 ;
T_prep (2,1,2) =    0.0630367 ;
T_prep (2,1,4) =    0.0231502 ;
T_prep (2,1,8) =   0.00999563 ;
T_prep (2,1,16) =    0.0169736 ;
T_prep (2,1,32) =    0.0113744 ;
T_prep (2,1,64) =   0.00830546 ;
T_prep (2,1,128) =   0.00822105 ;
T_prep (2,1,160) =   0.00816082 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      1.48837 ;
Time (1,1,2) =     0.776892 ;
Time (1,1,4) =     0.396409 ;
Time (1,1,8) =     0.201235 ;
Time (1,1,16) =     0.144794 ;
Time (1,1,32) =     0.350994 ;
Time (1,1,64) =     0.308084 ;
Time (1,1,128) =     0.321142 ;
Time (1,1,160) =     0.317036 ;
Time (2,1,1) =      15.1161 ;
Time (2,1,2) =      7.59389 ;
Time (2,1,4) =       3.8084 ;
Time (2,1,8) =      1.94042 ;
Time (2,1,16) =     0.991498 ;
Time (2,1,32) =     0.704376 ;
Time (2,1,64) =     0.734338 ;
Time (2,1,128) =      0.90709 ;
Time (2,1,160) =      0.96366 ;
Time (3,1,1) =      54.9584 ;
Time (3,1,2) =      28.2548 ;
Time (3,1,4) =      14.7653 ;
Time (3,1,8) =      7.95315 ;
Time (3,1,16) =      6.06326 ;
Time (3,1,32) =      5.59147 ;
Time (3,1,64) =      5.63001 ;
Time (3,1,128) =      5.82583 ;
Time (3,1,160) =      6.06477 ;
Time (4,1,1) =      5.86051 ;
Time (4,1,2) =      3.02258 ;
Time (4,1,4) =      1.49639 ;
Time (4,1,8) =     0.784355 ;
Time (4,1,16) =     0.438737 ;
Time (4,1,32) =     0.399741 ;
Time (4,1,64) =     0.414393 ;
Time (4,1,128) =      0.43514 ;
Time (4,1,160) =     0.419326 ;
Time (5,1,1) =      5.87163 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 6912000 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0175652 ;
T_prep (1,1,2) =     0.017565 ;
T_prep (1,1,4) =    0.0233324 ;
T_prep (1,1,8) =    0.0109219 ;
T_prep (1,1,16) =    0.0113677 ;
T_prep (1,1,32) =    0.0213481 ;
T_prep (1,1,64) =    0.0387418 ;
T_prep (1,1,128) =    0.0556049 ;
T_prep (1,1,160) =    0.0559611 ;
T_prep (2,1,1) =     0.171492 ;
T_prep (2,1,2) =     0.192094 ;
T_prep (2,1,4) =     0.136751 ;
T_prep (2,1,8) =    0.0952042 ;
T_prep (2,1,16) =    0.0756139 ;
T_prep (2,1,32) =    0.0525785 ;
T_prep (2,1,64) =     0.043086 ;
T_prep (2,1,128) =    0.0365162 ;
T_prep (2,1,160) =    0.0361089 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.174836 ;
Time (1,1,2) =    0.0628776 ;
Time (1,1,4) =    0.0347561 ;
Time (1,1,8) =    0.0183048 ;
Time (1,1,16) =    0.0125634 ;
Time (1,1,32) =    0.0227804 ;
Time (1,1,64) =   0.00760497 ;
Time (1,1,128) =    0.0120369 ;
Time (1,1,160) =    0.0110396 ;
Time (2,1,1) =     0.206694 ;
Time (2,1,2) =    0.0890393 ;
Time (2,1,4) =    0.0466892 ;
Time (2,1,8) =    0.0257692 ;
Time (2,1,16) =    0.0273299 ;
Time (2,1,32) =    0.0227991 ;
Time (2,1,64) =   0.00754443 ;
Time (2,1,128) =    0.0157106 ;
Time (2,1,160) =   0.00668747 ;
Time (3,1,1) =     0.208123 ;
Time (3,1,2) =    0.0763352 ;
Time (3,1,4) =    0.0429582 ;
Time (3,1,8) =    0.0219799 ;
Time (3,1,16) =    0.0120481 ;
Time (3,1,32) =   0.00877481 ;
Time (3,1,64) =   0.00903916 ;
Time (3,1,128) =    0.0113051 ;
Time (3,1,160) =    0.0104237 ;
Time (4,1,1) =     0.264872 ;
Time (4,1,2) =    0.0813542 ;
Time (4,1,4) =    0.0452802 ;
Time (4,1,8) =     0.024639 ;
Time (4,1,16) =    0.0178438 ;
Time (4,1,32) =   0.00803944 ;
Time (4,1,64) =    0.0118956 ;
Time (4,1,128) =    0.0155086 ;
Time (4,1,160) =    0.0207771 ;
Time (5,1,1) =     0.211967 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 11080030 ;
Ntri (id) = 35882427 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0279916 ;
T_prep (1,1,2) =    0.0281318 ;
T_prep (1,1,4) =     0.036052 ;
T_prep (1,1,8) =     0.019479 ;
T_prep (1,1,16) =      0.01739 ;
T_prep (1,1,32) =    0.0333505 ;
T_prep (1,1,64) =    0.0862484 ;
T_prep (1,1,128) =     0.208348 ;
T_prep (1,1,160) =     0.176695 ;
T_prep (2,1,1) =     0.330322 ;
T_prep (2,1,2) =     0.356718 ;
T_prep (2,1,4) =     0.297294 ;
T_prep (2,1,8) =     0.210572 ;
T_prep (2,1,16) =      0.16619 ;
T_prep (2,1,32) =     0.120942 ;
T_prep (2,1,64) =     0.101658 ;
T_prep (2,1,128) =     0.086177 ;
T_prep (2,1,160) =     0.108471 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      48.2856 ;
Time (1,1,2) =      23.9071 ;
Time (1,1,4) =      9.37466 ;
Time (1,1,8) =      5.02939 ;
Time (1,1,16) =      4.57335 ;
Time (1,1,32) =      3.43424 ;
Time (1,1,64) =      4.38229 ;
Time (1,1,128) =      5.96967 ;
Time (1,1,160) =       7.9664 ;
Time (2,1,1) =      112.534 ;
Time (2,1,2) =      58.7754 ;
Time (2,1,4) =      32.0351 ;
Time (2,1,8) =       16.711 ;
Time (2,1,16) =       18.071 ;
Time (2,1,32) =      19.0156 ;
Time (2,1,64) =      20.7147 ;
Time (2,1,128) =      17.5035 ;
Time (2,1,160) =      18.4129 ;
Time (3,1,1) =      7.69172 ;
Time (3,1,2) =      3.83907 ;
Time (3,1,4) =      1.95748 ;
Time (3,1,8) =      1.19362 ;
Time (3,1,16) =     0.992027 ;
Time (3,1,32) =     0.930914 ;
Time (3,1,64) =      1.02082 ;
Time (3,1,128) =     0.966315 ;
Time (3,1,160) =      1.07017 ;
Time (4,1,1) =      46.8209 ;
Time (4,1,2) =      23.9153 ;
Time (4,1,4) =      12.5142 ;
Time (4,1,8) =       7.7008 ;
Time (4,1,16) =      7.94311 ;
Time (4,1,32) =      7.60499 ;
Time (4,1,64) =      9.25543 ;
Time (4,1,128) =      9.49766 ;
Time (4,1,160) =      7.41467 ;
Time (5,1,1) =      47.3017 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-3-4-5-9-16-25-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 530400 ;
Nedges (id) = 11080030 ;
Ntri (id) = 651 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0278268 ;
T_prep (1,1,2) =    0.0278488 ;
T_prep (1,1,4) =    0.0307346 ;
T_prep (1,1,8) =    0.0152534 ;
T_prep (1,1,16) =    0.0231325 ;
T_prep (1,1,32) =     0.034976 ;
T_prep (1,1,64) =    0.0432044 ;
T_prep (1,1,128) =    0.0854667 ;
T_prep (1,1,160) =    0.0701411 ;
T_prep (2,1,1) =     0.292285 ;
T_prep (2,1,2) =     0.338392 ;
T_prep (2,1,4) =      0.21183 ;
T_prep (2,1,8) =     0.116496 ;
T_prep (2,1,16) =    0.0885463 ;
T_prep (2,1,32) =    0.0588087 ;
T_prep (2,1,64) =    0.0551077 ;
T_prep (2,1,128) =    0.0393545 ;
T_prep (2,1,160) =    0.0418397 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      8.29162 ;
Time (1,1,2) =      3.12004 ;
Time (1,1,4) =      1.66643 ;
Time (1,1,8) =      0.74105 ;
Time (1,1,16) =     0.426476 ;
Time (1,1,32) =     0.699697 ;
Time (1,1,64) =     0.486524 ;
Time (1,1,128) =     0.657357 ;
Time (1,1,160) =     0.572254 ;
Time (2,1,1) =      25.0661 ;
Time (2,1,2) =      11.3344 ;
Time (2,1,4) =      5.30852 ;
Time (2,1,8) =      2.75492 ;
Time (2,1,16) =       1.8649 ;
Time (2,1,32) =      1.10568 ;
Time (2,1,64) =      2.01348 ;
Time (2,1,128) =      1.11874 ;
Time (2,1,160) =      1.27175 ;
Time (3,1,1) =     0.955595 ;
Time (3,1,2) =     0.515088 ;
Time (3,1,4) =     0.255142 ;
Time (3,1,8) =     0.140106 ;
Time (3,1,16) =    0.0857725 ;
Time (3,1,32) =     0.057128 ;
Time (3,1,64) =    0.0464695 ;
Time (3,1,128) =     0.052588 ;
Time (3,1,160) =    0.0493527 ;
Time (4,1,1) =      8.04885 ;
Time (4,1,2) =      4.07072 ;
Time (4,1,4) =      2.01073 ;
Time (4,1,8) =      1.15738 ;
Time (4,1,16) =      0.68438 ;
Time (4,1,32) =     0.400683 ;
Time (4,1,64) =     0.440705 ;
Time (4,1,128) =     0.449284 ;
Time (4,1,160) =     0.477448 ;
Time (5,1,1) =      30.5732 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale19-ef16/graph500-scale19-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 335318 ;
Nedges (id) = 7729675 ;
Ntri (id) = 186288972 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.023221 ;
T_prep (1,1,2) =    0.0231758 ;
T_prep (1,1,4) =    0.0235388 ;
T_prep (1,1,8) =    0.0110538 ;
T_prep (1,1,16) =    0.0148557 ;
T_prep (1,1,32) =    0.0122281 ;
T_prep (1,1,64) =     0.016858 ;
T_prep (1,1,128) =    0.0286678 ;
T_prep (1,1,160) =    0.0537245 ;
T_prep (2,1,1) =     0.212269 ;
T_prep (2,1,2) =    0.0274112 ;
T_prep (2,1,4) =     0.109243 ;
T_prep (2,1,8) =    0.0861615 ;
T_prep (2,1,16) =    0.0459321 ;
T_prep (2,1,32) =    0.0319859 ;
T_prep (2,1,64) =    0.0186268 ;
T_prep (2,1,128) =     0.018247 ;
T_prep (2,1,160) =    0.0150773 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      11.3366 ;
Time (1,1,2) =      7.74564 ;
Time (1,1,4) =      4.21005 ;
Time (1,1,8) =      2.14981 ;
Time (1,1,16) =      1.36734 ;
Time (1,1,32) =      1.07735 ;
Time (1,1,64) =      1.22939 ;
Time (1,1,128) =      1.23575 ;
Time (1,1,160) =      1.22855 ;
Time (2,1,1) =      34.9357 ;
Time (2,1,2) =      18.4702 ;
Time (2,1,4) =      9.25631 ;
Time (2,1,8) =      4.66615 ;
Time (2,1,16) =      2.37424 ;
Time (2,1,32) =      2.07509 ;
Time (2,1,64) =      2.42491 ;
Time (2,1,128) =      2.40397 ;
Time (2,1,160) =      2.58636 ;
Time (3,1,1) =      146.512 ;
Time (3,1,2) =      80.3057 ;
Time (3,1,4) =      40.2622 ;
Time (3,1,8) =      21.4992 ;
Time (3,1,16) =      13.0937 ;
Time (3,1,32) =      11.2396 ;
Time (3,1,64) =      11.5157 ;
Time (3,1,128) =      10.3796 ;
Time (3,1,160) =      10.7966 ;
Time (4,1,1) =      17.2551 ;
Time (4,1,2) =       8.7634 ;
Time (4,1,4) =      4.51542 ;
Time (4,1,8) =      2.29127 ;
Time (4,1,16) =      1.36659 ;
Time (4,1,32) =      1.16786 ;
Time (4,1,64) =      1.11264 ;
Time (4,1,128) =      1.22639 ;
Time (4,1,160) =      1.20234 ;
Time (5,1,1) =      17.0497 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-16764930-4194304_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4194304 ;
Nedges (id) = 16764930 ;
Ntri (id) = 16760836 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0390725 ;
T_prep (1,1,2) =    0.0391761 ;
T_prep (1,1,4) =    0.0416704 ;
T_prep (1,1,8) =    0.0300528 ;
T_prep (1,1,16) =      0.02768 ;
T_prep (1,1,32) =     0.026467 ;
T_prep (1,1,64) =    0.0353767 ;
T_prep (1,1,128) =     0.090211 ;
T_prep (1,1,160) =    0.0987501 ;
T_prep (2,1,1) =     0.998844 ;
T_prep (2,1,2) =     0.966232 ;
T_prep (2,1,4) =     0.482728 ;
T_prep (2,1,8) =     0.265692 ;
T_prep (2,1,16) =      0.15828 ;
T_prep (2,1,32) =     0.139541 ;
T_prep (2,1,64) =    0.0925508 ;
T_prep (2,1,128) =     0.084199 ;
T_prep (2,1,160) =    0.0692851 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      1.83382 ;
Time (1,1,2) =     0.947771 ;
Time (1,1,4) =     0.638037 ;
Time (1,1,8) =     0.128739 ;
Time (1,1,16) =    0.0530126 ;
Time (1,1,32) =    0.0441715 ;
Time (1,1,64) =    0.0240601 ;
Time (1,1,128) =     0.031745 ;
Time (1,1,160) =    0.0502641 ;
Time (2,1,1) =      2.22804 ;
Time (2,1,2) =     0.987421 ;
Time (2,1,4) =      0.47336 ;
Time (2,1,8) =     0.212544 ;
Time (2,1,16) =    0.0790724 ;
Time (2,1,32) =    0.0366023 ;
Time (2,1,64) =    0.0344006 ;
Time (2,1,128) =    0.0194699 ;
Time (2,1,160) =    0.0255615 ;
Time (3,1,1) =      2.15532 ;
Time (3,1,2) =      0.97968 ;
Time (3,1,4) =     0.463835 ;
Time (3,1,8) =     0.221628 ;
Time (3,1,16) =    0.0871758 ;
Time (3,1,32) =    0.0405188 ;
Time (3,1,64) =    0.0198037 ;
Time (3,1,128) =     0.018866 ;
Time (3,1,160) =    0.0110627 ;
Time (4,1,1) =      1.46391 ;
Time (4,1,2) =     0.667118 ;
Time (4,1,4) =     0.325609 ;
Time (4,1,8) =     0.148579 ;
Time (4,1,16) =    0.0613822 ;
Time (4,1,32) =    0.0314489 ;
Time (4,1,64) =    0.0199364 ;
Time (4,1,128) =    0.0251913 ;
Time (4,1,160) =    0.0285267 ;
Time (5,1,1) =     0.847493 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-Bk.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 23328000 ;
Ntri (id) = 0 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0591044 ;
T_prep (1,1,2) =    0.0589541 ;
T_prep (1,1,4) =    0.0642517 ;
T_prep (1,1,8) =      0.20148 ;
T_prep (1,1,16) =     0.125845 ;
T_prep (1,1,32) =     0.150299 ;
T_prep (1,1,64) =       0.2006 ;
T_prep (1,1,128) =     0.252199 ;
T_prep (1,1,160) =      0.47185 ;
T_prep (2,1,1) =      0.61931 ;
T_prep (2,1,2) =     0.739981 ;
T_prep (2,1,4) =     0.589319 ;
T_prep (2,1,8) =     0.390853 ;
T_prep (2,1,16) =     0.291533 ;
T_prep (2,1,32) =     0.257057 ;
T_prep (2,1,64) =     0.234423 ;
T_prep (2,1,128) =     0.180559 ;
T_prep (2,1,160) =     0.179492 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.567468 ;
Time (1,1,2) =      0.28599 ;
Time (1,1,4) =      0.14268 ;
Time (1,1,8) =    0.0630118 ;
Time (1,1,16) =    0.0417959 ;
Time (1,1,32) =    0.0354839 ;
Time (1,1,64) =     0.111625 ;
Time (1,1,128) =    0.0941412 ;
Time (1,1,160) =    0.0491039 ;
Time (2,1,1) =     0.570788 ;
Time (2,1,2) =     0.291249 ;
Time (2,1,4) =     0.147471 ;
Time (2,1,8) =    0.0693766 ;
Time (2,1,16) =    0.0968963 ;
Time (2,1,32) =    0.0643044 ;
Time (2,1,64) =    0.0946812 ;
Time (2,1,128) =    0.0533273 ;
Time (2,1,160) =    0.0603464 ;
Time (3,1,1) =     0.707126 ;
Time (3,1,2) =     0.327126 ;
Time (3,1,4) =      0.16999 ;
Time (3,1,8) =    0.0818066 ;
Time (3,1,16) =    0.0562084 ;
Time (3,1,32) =    0.0383428 ;
Time (3,1,64) =    0.0325448 ;
Time (3,1,128) =    0.0307381 ;
Time (3,1,160) =    0.0321894 ;
Time (4,1,1) =     0.771653 ;
Time (4,1,2) =     0.323645 ;
Time (4,1,4) =     0.161045 ;
Time (4,1,8) =     0.118506 ;
Time (4,1,16) =    0.0923375 ;
Time (4,1,32) =     0.118264 ;
Time (4,1,64) =     0.124144 ;
Time (4,1,128) =    0.0661962 ;
Time (4,1,160) =    0.0700842 ;
Time (5,1,1) =     0.626782 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale20-ef16/graph500-scale20-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 645820 ;
Nedges (id) = 15680861 ;
Ntri (id) = 419349784 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.045221 ;
T_prep (1,1,2) =    0.0454543 ;
T_prep (1,1,4) =    0.0367138 ;
T_prep (1,1,8) =    0.0258085 ;
T_prep (1,1,16) =    0.0261014 ;
T_prep (1,1,32) =    0.0297153 ;
T_prep (1,1,64) =    0.0445206 ;
T_prep (1,1,128) =     0.107284 ;
T_prep (1,1,160) =    0.0602208 ;
T_prep (2,1,1) =     0.471138 ;
T_prep (2,1,2) =     0.539883 ;
T_prep (2,1,4) =     0.293117 ;
T_prep (2,1,8) =     0.172542 ;
T_prep (2,1,16) =     0.116054 ;
T_prep (2,1,32) =    0.0672397 ;
T_prep (2,1,64) =    0.0599737 ;
T_prep (2,1,128) =     0.051157 ;
T_prep (2,1,160) =    0.0722173 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      53.0991 ;
Time (1,1,2) =      28.1774 ;
Time (1,1,4) =      13.5675 ;
Time (1,1,8) =       6.9897 ;
Time (1,1,16) =       3.6285 ;
Time (1,1,32) =       3.2116 ;
Time (1,1,64) =       3.1734 ;
Time (1,1,128) =      3.40807 ;
Time (1,1,160) =      3.42185 ;
Time (2,1,1) =      116.789 ;
Time (2,1,2) =      57.8999 ;
Time (2,1,4) =      29.1777 ;
Time (2,1,8) =      14.2293 ;
Time (2,1,16) =        6.886 ;
Time (2,1,32) =      5.55892 ;
Time (2,1,64) =      5.29397 ;
Time (2,1,128) =      5.80532 ;
Time (2,1,160) =      6.26461 ;
Time (3,1,1) =      390.882 ;
Time (3,1,2) =      200.275 ;
Time (3,1,4) =      103.997 ;
Time (3,1,8) =      52.6817 ;
Time (3,1,16) =      27.6901 ;
Time (3,1,32) =      18.7433 ;
Time (3,1,64) =      14.4683 ;
Time (3,1,128) =      14.0886 ;
Time (3,1,160) =       14.028 ;
Time (4,1,1) =      55.3148 ;
Time (4,1,2) =        28.56 ;
Time (4,1,4) =      14.9688 ;
Time (4,1,8) =      7.40482 ;
Time (4,1,16) =      3.79284 ;
Time (4,1,32) =      3.24401 ;
Time (4,1,64) =      3.10573 ;
Time (4,1,128) =      3.28362 ;
Time (4,1,160) =      3.29129 ;
Time (5,1,1) =      54.5937 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-B2k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 28667380 ;
Ntri (id) = 155 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0721092 ;
T_prep (1,1,2) =    0.0718218 ;
T_prep (1,1,4) =    0.0698594 ;
T_prep (1,1,8) =    0.0532011 ;
T_prep (1,1,16) =    0.0541105 ;
T_prep (1,1,32) =    0.0644738 ;
T_prep (1,1,64) =     0.248475 ;
T_prep (1,1,128) =     0.316039 ;
T_prep (1,1,160) =     0.180395 ;
T_prep (2,1,1) =     0.454689 ;
T_prep (2,1,2) =     0.534037 ;
T_prep (2,1,4) =     0.422792 ;
T_prep (2,1,8) =     0.368811 ;
T_prep (2,1,16) =     0.272878 ;
T_prep (2,1,32) =     0.238791 ;
T_prep (2,1,64) =     0.209502 ;
T_prep (2,1,128) =     0.184316 ;
T_prep (2,1,160) =     0.231048 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =       33.916 ;
Time (1,1,2) =      15.9885 ;
Time (1,1,4) =      8.43511 ;
Time (1,1,8) =      4.77135 ;
Time (1,1,16) =      2.74755 ;
Time (1,1,32) =      2.08342 ;
Time (1,1,64) =      1.12623 ;
Time (1,1,128) =      1.34224 ;
Time (1,1,160) =      1.32157 ;
Time (2,1,1) =      105.932 ;
Time (2,1,2) =      45.6107 ;
Time (2,1,4) =      25.2467 ;
Time (2,1,8) =      13.9929 ;
Time (2,1,16) =      8.32081 ;
Time (2,1,32) =      4.85543 ;
Time (2,1,64) =      2.96806 ;
Time (2,1,128) =       3.4859 ;
Time (2,1,160) =      3.88329 ;
Time (3,1,1) =      1.88047 ;
Time (3,1,2) =     0.925838 ;
Time (3,1,4) =     0.483513 ;
Time (3,1,8) =     0.258589 ;
Time (3,1,16) =     0.159448 ;
Time (3,1,32) =     0.116743 ;
Time (3,1,64) =     0.101411 ;
Time (3,1,128) =     0.101214 ;
Time (3,1,160) =     0.102316 ;
Time (4,1,1) =      34.5926 ;
Time (4,1,2) =      16.7406 ;
Time (4,1,4) =      9.02837 ;
Time (4,1,8) =      4.96668 ;
Time (4,1,16) =      2.89281 ;
Time (4,1,32) =      2.12834 ;
Time (4,1,64) =      1.17749 ;
Time (4,1,128) =      1.28857 ;
Time (4,1,160) =      1.27763 ;
Time (5,1,1) =      313.632 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc3/Theory-5-9-16-25-81-B1k.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2174640 ;
Nedges (id) = 28667380 ;
Ntri (id) = 66758995 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0748044 ;
T_prep (1,1,2) =    0.0748548 ;
T_prep (1,1,4) =    0.0814495 ;
T_prep (1,1,8) =    0.0563877 ;
T_prep (1,1,16) =    0.0672844 ;
T_prep (1,1,32) =    0.0999919 ;
T_prep (1,1,64) =     0.290284 ;
T_prep (1,1,128) =     0.363597 ;
T_prep (1,1,160) =      0.45877 ;
T_prep (2,1,1) =      0.81315 ;
T_prep (2,1,2) =     0.849456 ;
T_prep (2,1,4) =     0.777141 ;
T_prep (2,1,8) =     0.583773 ;
T_prep (2,1,16) =     0.455327 ;
T_prep (2,1,32) =     0.402488 ;
T_prep (2,1,64) =     0.358589 ;
T_prep (2,1,128) =     0.298604 ;
T_prep (2,1,160) =     0.517003 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      257.362 ;
Time (1,1,2) =      144.577 ;
Time (1,1,4) =      74.4517 ;
Time (1,1,8) =      36.5753 ;
Time (1,1,16) =      28.4392 ;
Time (1,1,32) =       27.861 ;
Time (1,1,64) =      30.7862 ;
Time (1,1,128) =       33.132 ;
Time (1,1,160) =      32.7738 ;
Time (2,1,1) =      737.674 ;
Time (2,1,2) =       347.85 ;
Time (2,1,4) =      178.728 ;
Time (2,1,8) =      89.7548 ;
Time (2,1,16) =      58.9214 ;
Time (2,1,32) =      60.3646 ;
Time (2,1,64) =      69.6603 ;
Time (2,1,128) =      74.9743 ;
Time (2,1,160) =      74.8837 ;
Time (3,1,1) =      16.6983 ;
Time (3,1,2) =      8.41932 ;
Time (3,1,4) =      4.59171 ;
Time (3,1,8) =      2.80163 ;
Time (3,1,16) =      1.98432 ;
Time (3,1,32) =      1.60075 ;
Time (3,1,64) =      1.59457 ;
Time (3,1,128) =      1.68963 ;
Time (3,1,160) =      1.69473 ;
Time (4,1,1) =      280.805 ;
Time (4,1,2) =      151.925 ;
Time (4,1,4) =      80.9746 ;
Time (4,1,8) =      38.2056 ;
Time (4,1,16) =      27.1351 ;
Time (4,1,32) =      27.0765 ;
Time (4,1,64) =      32.4939 ;
Time (4,1,128) =      33.7084 ;
Time (4,1,160) =       33.983 ;
Time (5,1,1) =      273.616 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/cit-Patents/cit-Patents_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 3774768 ;
Nedges (id) = 16518947 ;
Ntri (id) = 7515023 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0790478 ;
T_prep (1,1,2) =    0.0792229 ;
T_prep (1,1,4) =    0.0661735 ;
T_prep (1,1,8) =    0.0686856 ;
T_prep (1,1,16) =    0.0424842 ;
T_prep (1,1,32) =    0.0331386 ;
T_prep (1,1,64) =    0.0367283 ;
T_prep (1,1,128) =    0.0796782 ;
T_prep (1,1,160) =    0.0774689 ;
T_prep (2,1,1) =      0.57127 ;
T_prep (2,1,2) =       0.6535 ;
T_prep (2,1,4) =     0.400931 ;
T_prep (2,1,8) =     0.232817 ;
T_prep (2,1,16) =     0.144412 ;
T_prep (2,1,32) =    0.0897583 ;
T_prep (2,1,64) =    0.0943976 ;
T_prep (2,1,128) =    0.0548489 ;
T_prep (2,1,160) =    0.0559969 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      3.63307 ;
Time (1,1,2) =      1.68996 ;
Time (1,1,4) =     0.928475 ;
Time (1,1,8) =     0.486194 ;
Time (1,1,16) =     0.247897 ;
Time (1,1,32) =     0.136124 ;
Time (1,1,64) =     0.121704 ;
Time (1,1,128) =     0.148442 ;
Time (1,1,160) =     0.165454 ;
Time (2,1,1) =      4.14654 ;
Time (2,1,2) =      2.17029 ;
Time (2,1,4) =      0.98729 ;
Time (2,1,8) =     0.464452 ;
Time (2,1,16) =     0.232164 ;
Time (2,1,32) =     0.115546 ;
Time (2,1,64) =    0.0698457 ;
Time (2,1,128) =    0.0709471 ;
Time (2,1,160) =     0.070693 ;
Time (3,1,1) =      7.23693 ;
Time (3,1,2) =      3.78072 ;
Time (3,1,4) =      1.91209 ;
Time (3,1,8) =     0.908294 ;
Time (3,1,16) =     0.447217 ;
Time (3,1,32) =     0.218023 ;
Time (3,1,64) =     0.123332 ;
Time (3,1,128) =     0.111797 ;
Time (3,1,160) =     0.114317 ;
Time (4,1,1) =      3.68922 ;
Time (4,1,2) =       1.9762 ;
Time (4,1,4) =     0.982578 ;
Time (4,1,8) =     0.464476 ;
Time (4,1,16) =     0.253847 ;
Time (4,1,32) =     0.134491 ;
Time (4,1,64) =     0.117543 ;
Time (4,1,128) =     0.152736 ;
Time (4,1,160) =     0.163391 ;
Time (5,1,1) =      3.32216 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale21-ef16/graph500-scale21-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 1243072 ;
Nedges (id) = 31731650 ;
Ntri (id) = 935100883 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =      0.11296 ;
T_prep (1,1,2) =    0.0910747 ;
T_prep (1,1,4) =     0.068396 ;
T_prep (1,1,8) =    0.0518657 ;
T_prep (1,1,16) =    0.0576651 ;
T_prep (1,1,32) =    0.0527249 ;
T_prep (1,1,64) =     0.071507 ;
T_prep (1,1,128) =      0.14136 ;
T_prep (1,1,160) =     0.140059 ;
T_prep (2,1,1) =     0.815436 ;
T_prep (2,1,2) =     0.937506 ;
T_prep (2,1,4) =     0.497029 ;
T_prep (2,1,8) =      0.29083 ;
T_prep (2,1,16) =     0.176214 ;
T_prep (2,1,32) =     0.128517 ;
T_prep (2,1,64) =     0.120592 ;
T_prep (2,1,128) =     0.136607 ;
T_prep (2,1,160) =     0.108493 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      152.245 ;
Time (1,1,2) =      77.5817 ;
Time (1,1,4) =      41.3354 ;
Time (1,1,8) =      21.5425 ;
Time (1,1,16) =      11.3283 ;
Time (1,1,32) =      6.46144 ;
Time (1,1,64) =      4.62546 ;
Time (1,1,128) =      5.65262 ;
Time (1,1,160) =       6.1139 ;
Time (2,1,1) =      314.125 ;
Time (2,1,2) =      146.405 ;
Time (2,1,4) =      80.0231 ;
Time (2,1,8) =      39.5446 ;
Time (2,1,16) =      19.5962 ;
Time (2,1,32) =      11.2151 ;
Time (2,1,64) =      7.03024 ;
Time (2,1,128) =      7.97841 ;
Time (2,1,160) =      7.05471 ;
Time (3,1,1) =      1006.02 ;
Time (3,1,2) =       521.32 ;
Time (3,1,4) =      253.231 ;
Time (3,1,8) =      126.251 ;
Time (3,1,16) =      68.4251 ;
Time (3,1,32) =      43.1826 ;
Time (3,1,64) =      30.8842 ;
Time (3,1,128) =      28.5479 ;
Time (3,1,160) =       29.982 ;
Time (4,1,1) =      169.073 ;
Time (4,1,2) =       88.345 ;
Time (4,1,4) =      44.7785 ;
Time (4,1,8) =      22.9776 ;
Time (4,1,16) =      11.8338 ;
Time (4,1,32) =      7.00493 ;
Time (4,1,64) =      4.96114 ;
Time (4,1,128) =      6.02415 ;
Time (4,1,160) =      6.17958 ;
Time (5,1,1) =      160.603 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-67084290-16777216_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 16777216 ;
Nedges (id) = 67084290 ;
Ntri (id) = 67076100 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.157343 ;
T_prep (1,1,2) =     0.157165 ;
T_prep (1,1,4) =     0.136248 ;
T_prep (1,1,8) =     0.130115 ;
T_prep (1,1,16) =     0.133844 ;
T_prep (1,1,32) =     0.125939 ;
T_prep (1,1,64) =     0.158944 ;
T_prep (1,1,128) =      0.20143 ;
T_prep (1,1,160) =     0.250096 ;
T_prep (2,1,1) =      1.31283 ;
T_prep (2,1,2) =      1.44374 ;
T_prep (2,1,4) =     0.832993 ;
T_prep (2,1,8) =     0.511768 ;
T_prep (2,1,16) =     0.334554 ;
T_prep (2,1,32) =     0.249946 ;
T_prep (2,1,64) =      0.21888 ;
T_prep (2,1,128) =     0.211247 ;
T_prep (2,1,160) =     0.191898 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      3.89146 ;
Time (1,1,2) =      1.92398 ;
Time (1,1,4) =     0.959053 ;
Time (1,1,8) =     0.503398 ;
Time (1,1,16) =     0.245138 ;
Time (1,1,32) =     0.123906 ;
Time (1,1,64) =    0.0655972 ;
Time (1,1,128) =    0.0455691 ;
Time (1,1,160) =    0.0614443 ;
Time (2,1,1) =       5.6189 ;
Time (2,1,2) =      2.87119 ;
Time (2,1,4) =      1.42478 ;
Time (2,1,8) =     0.722911 ;
Time (2,1,16) =     0.351106 ;
Time (2,1,32) =      0.17653 ;
Time (2,1,64) =    0.0829396 ;
Time (2,1,128) =    0.0440254 ;
Time (2,1,160) =    0.0377498 ;
Time (3,1,1) =        5.825 ;
Time (3,1,2) =      3.21121 ;
Time (3,1,4) =      1.56831 ;
Time (3,1,8) =     0.793079 ;
Time (3,1,16) =     0.384126 ;
Time (3,1,32) =     0.192215 ;
Time (3,1,64) =    0.0890914 ;
Time (3,1,128) =    0.0474632 ;
Time (3,1,160) =     0.047714 ;
Time (4,1,1) =      4.32177 ;
Time (4,1,2) =      2.27461 ;
Time (4,1,4) =      1.10222 ;
Time (4,1,8) =     0.570668 ;
Time (4,1,16) =     0.278831 ;
Time (4,1,32) =     0.143697 ;
Time (4,1,64) =    0.0830272 ;
Time (4,1,128) =    0.0615024 ;
Time (4,1,160) =    0.0639014 ;
Time (5,1,1) =      3.01308 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale22-ef16/graph500-scale22-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 2393285 ;
Nedges (id) = 64097004 ;
Ntri (id) = 2067392370 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.183148 ;
T_prep (1,1,2) =     0.183239 ;
T_prep (1,1,4) =     0.129801 ;
T_prep (1,1,8) =     0.110047 ;
T_prep (1,1,16) =     0.128956 ;
T_prep (1,1,32) =     0.113348 ;
T_prep (1,1,64) =     0.165841 ;
T_prep (1,1,128) =     0.226024 ;
T_prep (1,1,160) =     0.241192 ;
T_prep (2,1,1) =      1.62014 ;
T_prep (2,1,2) =      1.77105 ;
T_prep (2,1,4) =     0.993158 ;
T_prep (2,1,8) =     0.588518 ;
T_prep (2,1,16) =     0.387672 ;
T_prep (2,1,32) =     0.271554 ;
T_prep (2,1,64) =     0.356562 ;
T_prep (2,1,128) =     0.181518 ;
T_prep (2,1,160) =     0.154935 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      439.775 ;
Time (1,1,2) =       219.09 ;
Time (1,1,4) =      116.329 ;
Time (1,1,8) =      67.1691 ;
Time (1,1,16) =      33.5481 ;
Time (1,1,32) =      17.7719 ;
Time (1,1,64) =      13.8408 ;
Time (1,1,128) =      23.4346 ;
Time (1,1,160) =      26.0246 ;
Time (2,1,1) =      835.295 ;
Time (2,1,2) =      411.452 ;
Time (2,1,4) =      206.236 ;
Time (2,1,8) =      98.0811 ;
Time (2,1,16) =      50.0331 ;
Time (2,1,32) =      24.8326 ;
Time (2,1,64) =      13.1143 ;
Time (2,1,128) =      14.8791 ;
Time (2,1,160) =      17.4125 ;
Time (3,1,1) =      2776.22 ;
Time (3,1,2) =      1308.68 ;
Time (3,1,4) =      715.906 ;
Time (3,1,8) =      349.398 ;
Time (3,1,16) =      188.981 ;
Time (3,1,32) =      133.414 ;
Time (3,1,64) =      118.993 ;
Time (3,1,128) =      125.152 ;
Time (3,1,160) =      126.671 ;
Time (4,1,1) =      455.592 ;
Time (4,1,2) =      240.309 ;
Time (4,1,4) =      128.857 ;
Time (4,1,8) =       69.297 ;
Time (4,1,16) =      34.4874 ;
Time (4,1,32) =      18.3621 ;
Time (4,1,64) =      12.6673 ;
Time (4,1,128) =      22.5026 ;
Time (4,1,160) =      26.7564 ;
Time (5,1,1) =      450.809 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/V2a.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 55042369 ;
Nedges (id) = 58608800 ;
Ntri (id) = 1443 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.291128 ;
T_prep (1,1,2) =     0.289549 ;
T_prep (1,1,4) =     0.377639 ;
T_prep (1,1,8) =     0.290362 ;
T_prep (1,1,16) =     0.254084 ;
T_prep (1,1,32) =     0.231726 ;
T_prep (1,1,64) =     0.303251 ;
T_prep (1,1,128) =     0.479185 ;
T_prep (1,1,160) =     0.501058 ;
T_prep (2,1,1) =      2.61815 ;
T_prep (2,1,2) =      3.24679 ;
T_prep (2,1,4) =      1.70501 ;
T_prep (2,1,8) =     0.963926 ;
T_prep (2,1,16) =     0.745016 ;
T_prep (2,1,32) =     0.700197 ;
T_prep (2,1,64) =     0.533316 ;
T_prep (2,1,128) =     0.467843 ;
T_prep (2,1,160) =     0.470649 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      5.70683 ;
Time (1,1,2) =      2.66123 ;
Time (1,1,4) =      1.15058 ;
Time (1,1,8) =     0.627286 ;
Time (1,1,16) =     0.382261 ;
Time (1,1,32) =     0.280487 ;
Time (1,1,64) =     0.312434 ;
Time (1,1,128) =     0.441974 ;
Time (1,1,160) =     0.423451 ;
Time (2,1,1) =      5.80901 ;
Time (2,1,2) =      2.72017 ;
Time (2,1,4) =      1.24713 ;
Time (2,1,8) =     0.620874 ;
Time (2,1,16) =     0.320922 ;
Time (2,1,32) =     0.175178 ;
Time (2,1,64) =      0.11853 ;
Time (2,1,128) =     0.115649 ;
Time (2,1,160) =     0.122328 ;
Time (3,1,1) =      5.52357 ;
Time (3,1,2) =      2.84968 ;
Time (3,1,4) =      1.36998 ;
Time (3,1,8) =     0.685753 ;
Time (3,1,16) =     0.354961 ;
Time (3,1,32) =     0.195335 ;
Time (3,1,64) =     0.155478 ;
Time (3,1,128) =     0.147198 ;
Time (3,1,160) =     0.152528 ;
Time (4,1,1) =      6.05981 ;
Time (4,1,2) =      3.08386 ;
Time (4,1,4) =      1.56157 ;
Time (4,1,8) =     0.799379 ;
Time (4,1,16) =     0.432647 ;
Time (4,1,32) =       0.2831 ;
Time (4,1,64) =     0.294764 ;
Time (4,1,128) =     0.426826 ;
Time (4,1,160) =     0.496308 ;
Time (5,1,1) =      5.84588 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/U1a.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 67716231 ;
Nedges (id) = 69389281 ;
Ntri (id) = 325 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.321688 ;
T_prep (1,1,2) =     0.321436 ;
T_prep (1,1,4) =     0.376944 ;
T_prep (1,1,8) =     0.368302 ;
T_prep (1,1,16) =     0.338985 ;
T_prep (1,1,32) =     0.253976 ;
T_prep (1,1,64) =     0.312491 ;
T_prep (1,1,128) =      0.64242 ;
T_prep (1,1,160) =     0.595965 ;
T_prep (2,1,1) =      2.79461 ;
T_prep (2,1,2) =      3.32484 ;
T_prep (2,1,4) =      1.93357 ;
T_prep (2,1,8) =      1.25502 ;
T_prep (2,1,16) =     0.839483 ;
T_prep (2,1,32) =     0.649123 ;
T_prep (2,1,64) =     0.580028 ;
T_prep (2,1,128) =     0.555741 ;
T_prep (2,1,160) =     0.541826 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      6.40778 ;
Time (1,1,2) =      3.48048 ;
Time (1,1,4) =      1.73046 ;
Time (1,1,8) =     0.906652 ;
Time (1,1,16) =     0.481354 ;
Time (1,1,32) =     0.340812 ;
Time (1,1,64) =     0.375466 ;
Time (1,1,128) =     0.491037 ;
Time (1,1,160) =      0.61781 ;
Time (2,1,1) =      6.24939 ;
Time (2,1,2) =      3.49245 ;
Time (2,1,4) =      1.62935 ;
Time (2,1,8) =     0.820099 ;
Time (2,1,16) =     0.407783 ;
Time (2,1,32) =     0.218406 ;
Time (2,1,64) =     0.180111 ;
Time (2,1,128) =      0.18181 ;
Time (2,1,160) =     0.193923 ;
Time (3,1,1) =      6.41003 ;
Time (3,1,2) =      3.60819 ;
Time (3,1,4) =      1.80316 ;
Time (3,1,8) =     0.908852 ;
Time (3,1,16) =     0.452571 ;
Time (3,1,32) =     0.279471 ;
Time (3,1,64) =     0.248465 ;
Time (3,1,128) =     0.238195 ;
Time (3,1,160) =     0.249704 ;
Time (4,1,1) =      7.06292 ;
Time (4,1,2) =      3.39226 ;
Time (4,1,4) =      1.83598 ;
Time (4,1,8) =     0.925606 ;
Time (4,1,16) =     0.501551 ;
Time (4,1,32) =     0.327317 ;
Time (4,1,64) =     0.339999 ;
Time (4,1,128) =     0.444443 ;
Time (4,1,160) =     0.593664 ;
Time (5,1,1) =      6.86799 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale23-ef16/graph500-scale23-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 4606314 ;
Nedges (id) = 129250705 ;
Ntri (id) = 4549133002 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.363771 ;
T_prep (1,1,2) =     0.363867 ;
T_prep (1,1,4) =     0.223822 ;
T_prep (1,1,8) =     0.180114 ;
T_prep (1,1,16) =      0.14504 ;
T_prep (1,1,32) =     0.164034 ;
T_prep (1,1,64) =     0.242761 ;
T_prep (1,1,128) =     0.390032 ;
T_prep (1,1,160) =     0.482406 ;
T_prep (2,1,1) =      3.05184 ;
T_prep (2,1,2) =      3.52868 ;
T_prep (2,1,4) =      1.79891 ;
T_prep (2,1,8) =     0.906444 ;
T_prep (2,1,16) =      0.65421 ;
T_prep (2,1,32) =     0.435984 ;
T_prep (2,1,64) =     0.337086 ;
T_prep (2,1,128) =     0.299931 ;
T_prep (2,1,160) =     0.293731 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =         1378 ;
Time (1,1,2) =      773.321 ;
Time (1,1,4) =      376.289 ;
Time (1,1,8) =       199.75 ;
Time (1,1,16) =      110.592 ;
Time (1,1,32) =      81.9175 ;
Time (1,1,64) =      66.6403 ;
Time (1,1,128) =      128.931 ;
Time (1,1,160) =      154.052 ;
Time (2,1,1) =      2453.82 ;
Time (2,1,2) =      1197.09 ;
Time (2,1,4) =      587.868 ;
Time (2,1,8) =      294.599 ;
Time (2,1,16) =      144.252 ;
Time (2,1,32) =      68.9523 ;
Time (2,1,64) =       43.339 ;
Time (2,1,128) =      34.8247 ;
Time (2,1,160) =      35.9954 ;
Time (3,1,1) =      7178.55 ;
Time (3,1,2) =      3654.55 ;
Time (3,1,4) =      1829.77 ;
Time (3,1,8) =      909.407 ;
Time (3,1,16) =      468.734 ;
Time (3,1,32) =      235.842 ;
Time (3,1,64) =      152.193 ;
Time (3,1,128) =      134.777 ;
Time (3,1,160) =      135.483 ;
Time (4,1,1) =       1463.6 ;
Time (4,1,2) =      731.634 ;
Time (4,1,4) =      396.041 ;
Time (4,1,8) =      218.866 ;
Time (4,1,16) =       115.58 ;
Time (4,1,32) =      78.3492 ;
Time (4,1,64) =      66.5562 ;
Time (4,1,128) =      130.592 ;
Time (4,1,160) =      155.005 ;
Time (5,1,1) =      1417.59 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-268386306-67108864_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 67108864 ;
Nedges (id) = 268386306 ;
Ntri (id) = 268369924 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.627728 ;
T_prep (1,1,2) =     0.638085 ;
T_prep (1,1,4) =      0.54603 ;
T_prep (1,1,8) =     0.421383 ;
T_prep (1,1,16) =     0.501868 ;
T_prep (1,1,32) =     0.530122 ;
T_prep (1,1,64) =     0.578139 ;
T_prep (1,1,128) =     0.839482 ;
T_prep (1,1,160) =     0.776725 ;
T_prep (2,1,1) =      6.78601 ;
T_prep (2,1,2) =      8.19406 ;
T_prep (2,1,4) =      3.79188 ;
T_prep (2,1,8) =      1.95945 ;
T_prep (2,1,16) =      1.25132 ;
T_prep (2,1,32) =      0.91545 ;
T_prep (2,1,64) =     0.882307 ;
T_prep (2,1,128) =     0.806494 ;
T_prep (2,1,160) =     0.804367 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      16.4885 ;
Time (1,1,2) =      7.97185 ;
Time (1,1,4) =      3.81886 ;
Time (1,1,8) =      1.97883 ;
Time (1,1,16) =      1.01358 ;
Time (1,1,32) =     0.573363 ;
Time (1,1,64) =     0.411302 ;
Time (1,1,128) =     0.361965 ;
Time (1,1,160) =     0.386076 ;
Time (2,1,1) =      25.0303 ;
Time (2,1,2) =      12.2432 ;
Time (2,1,4) =      5.84897 ;
Time (2,1,8) =       2.9623 ;
Time (2,1,16) =      1.46288 ;
Time (2,1,32) =     0.707257 ;
Time (2,1,64) =     0.334557 ;
Time (2,1,128) =     0.167519 ;
Time (2,1,160) =     0.143576 ;
Time (3,1,1) =      25.0241 ;
Time (3,1,2) =      13.1638 ;
Time (3,1,4) =      6.34472 ;
Time (3,1,8) =      3.22863 ;
Time (3,1,16) =       1.5809 ;
Time (3,1,32) =     0.753952 ;
Time (3,1,64) =     0.352898 ;
Time (3,1,128) =     0.198121 ;
Time (3,1,160) =     0.193349 ;
Time (4,1,1) =      18.9223 ;
Time (4,1,2) =      9.56354 ;
Time (4,1,4) =      4.52153 ;
Time (4,1,8) =      2.33249 ;
Time (4,1,16) =        1.193 ;
Time (4,1,32) =     0.653939 ;
Time (4,1,64) =     0.429241 ;
Time (4,1,128) =     0.344319 ;
Time (4,1,160) =     0.385188 ;
Time (5,1,1) =      12.6648 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/P1a.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 139353211 ;
Nedges (id) = 148914992 ;
Ntri (id) = 3412 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.744744 ;
T_prep (1,1,2) =     0.740621 ;
T_prep (1,1,4) =     0.781306 ;
T_prep (1,1,8) =     0.606691 ;
T_prep (1,1,16) =     0.691633 ;
T_prep (1,1,32) =     0.620925 ;
T_prep (1,1,64) =     0.848122 ;
T_prep (1,1,128) =      1.23606 ;
T_prep (1,1,160) =      1.25967 ;
T_prep (2,1,1) =       5.9326 ;
T_prep (2,1,2) =      6.88716 ;
T_prep (2,1,4) =      4.10641 ;
T_prep (2,1,8) =      2.47212 ;
T_prep (2,1,16) =      1.75907 ;
T_prep (2,1,32) =      1.37863 ;
T_prep (2,1,64) =      1.24409 ;
T_prep (2,1,128) =      1.21725 ;
T_prep (2,1,160) =       1.1588 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      13.6675 ;
Time (1,1,2) =      7.16084 ;
Time (1,1,4) =      3.89933 ;
Time (1,1,8) =      2.02632 ;
Time (1,1,16) =      1.07662 ;
Time (1,1,32) =     0.714314 ;
Time (1,1,64) =     0.759209 ;
Time (1,1,128) =      1.09945 ;
Time (1,1,160) =      1.22484 ;
Time (2,1,1) =      13.4292 ;
Time (2,1,2) =       7.2317 ;
Time (2,1,4) =      3.58907 ;
Time (2,1,8) =      1.81903 ;
Time (2,1,16) =        0.904 ;
Time (2,1,32) =     0.491517 ;
Time (2,1,64) =     0.392335 ;
Time (2,1,128) =     0.391713 ;
Time (2,1,160) =     0.418587 ;
Time (3,1,1) =      13.8621 ;
Time (3,1,2) =       7.4409 ;
Time (3,1,4) =       3.7656 ;
Time (3,1,8) =      1.90167 ;
Time (3,1,16) =     0.957769 ;
Time (3,1,32) =     0.564062 ;
Time (3,1,64) =     0.503668 ;
Time (3,1,128) =     0.489331 ;
Time (3,1,160) =     0.509305 ;
Time (4,1,1) =      15.3003 ;
Time (4,1,2) =      7.79788 ;
Time (4,1,4) =      4.07856 ;
Time (4,1,8) =      2.13515 ;
Time (4,1,16) =      1.13309 ;
Time (4,1,32) =     0.711445 ;
Time (4,1,64) =     0.739365 ;
Time (4,1,128) =      1.03595 ;
Time (4,1,160) =      1.21204 ;
Time (5,1,1) =       14.914 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/A2a.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 170728175 ;
Nedges (id) = 180292586 ;
Ntri (id) = 3858 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.879544 ;
T_prep (1,1,2) =     0.887635 ;
T_prep (1,1,4) =      1.07921 ;
T_prep (1,1,8) =     0.833068 ;
T_prep (1,1,16) =     0.827771 ;
T_prep (1,1,32) =     0.555985 ;
T_prep (1,1,64) =     0.973738 ;
T_prep (1,1,128) =      1.36294 ;
T_prep (1,1,160) =      1.39581 ;
T_prep (2,1,1) =      7.62165 ;
T_prep (2,1,2) =      8.79434 ;
T_prep (2,1,4) =      5.47582 ;
T_prep (2,1,8) =      3.12003 ;
T_prep (2,1,16) =      2.10224 ;
T_prep (2,1,32) =       1.6236 ;
T_prep (2,1,64) =      1.51271 ;
T_prep (2,1,128) =      1.45934 ;
T_prep (2,1,160) =      1.33665 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      17.6378 ;
Time (1,1,2) =      9.21679 ;
Time (1,1,4) =      4.75418 ;
Time (1,1,8) =        2.421 ;
Time (1,1,16) =      1.31786 ;
Time (1,1,32) =     0.865878 ;
Time (1,1,64) =     0.922537 ;
Time (1,1,128) =       1.3252 ;
Time (1,1,160) =      1.52263 ;
Time (2,1,1) =      17.0892 ;
Time (2,1,2) =      9.24525 ;
Time (2,1,4) =      4.50709 ;
Time (2,1,8) =      2.26992 ;
Time (2,1,16) =      1.12687 ;
Time (2,1,32) =     0.604004 ;
Time (2,1,64) =     0.484788 ;
Time (2,1,128) =      0.49341 ;
Time (2,1,160) =     0.521188 ;
Time (3,1,1) =       17.075 ;
Time (3,1,2) =      9.47048 ;
Time (3,1,4) =      4.63384 ;
Time (3,1,8) =      2.37843 ;
Time (3,1,16) =      1.19496 ;
Time (3,1,32) =     0.681453 ;
Time (3,1,64) =     0.585169 ;
Time (3,1,128) =      0.56568 ;
Time (3,1,160) =     0.582692 ;
Time (4,1,1) =      19.0607 ;
Time (4,1,2) =      9.87656 ;
Time (4,1,4) =      4.87509 ;
Time (4,1,8) =      2.60513 ;
Time (4,1,16) =      1.38618 ;
Time (4,1,32) =     0.862386 ;
Time (4,1,64) =     0.855644 ;
Time (4,1,128) =      1.24284 ;
Time (4,1,160) =      1.38474 ;
Time (5,1,1) =      19.1161 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc6/V1r.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 214005017 ;
Nedges (id) = 232705452 ;
Ntri (id) = 49 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =      1.18466 ;
T_prep (1,1,2) =      1.17357 ;
T_prep (1,1,4) =       1.2522 ;
T_prep (1,1,8) =      1.00246 ;
T_prep (1,1,16) =     0.641361 ;
T_prep (1,1,32) =     0.801172 ;
T_prep (1,1,64) =      1.09397 ;
T_prep (1,1,128) =      1.53438 ;
T_prep (1,1,160) =      1.81038 ;
T_prep (2,1,1) =      9.58174 ;
T_prep (2,1,2) =      11.7697 ;
T_prep (2,1,4) =       6.7112 ;
T_prep (2,1,8) =      4.43557 ;
T_prep (2,1,16) =      2.77359 ;
T_prep (2,1,32) =      2.14466 ;
T_prep (2,1,64) =      1.76633 ;
T_prep (2,1,128) =      1.62974 ;
T_prep (2,1,160) =      1.58589 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      16.8194 ;
Time (1,1,2) =       8.9827 ;
Time (1,1,4) =      4.51497 ;
Time (1,1,8) =      2.24219 ;
Time (1,1,16) =      1.23403 ;
Time (1,1,32) =     0.821002 ;
Time (1,1,64) =     0.839109 ;
Time (1,1,128) =      1.10755 ;
Time (1,1,160) =      1.25744 ;
Time (2,1,1) =      17.0473 ;
Time (2,1,2) =      8.99966 ;
Time (2,1,4) =      4.25297 ;
Time (2,1,8) =      2.00732 ;
Time (2,1,16) =      1.04688 ;
Time (2,1,32) =     0.529304 ;
Time (2,1,64) =     0.353869 ;
Time (2,1,128) =     0.355462 ;
Time (2,1,160) =     0.374046 ;
Time (3,1,1) =      14.6147 ;
Time (3,1,2) =      8.07636 ;
Time (3,1,4) =      3.81617 ;
Time (3,1,8) =      1.85109 ;
Time (3,1,16) =     0.947431 ;
Time (3,1,32) =     0.471579 ;
Time (3,1,64) =     0.342117 ;
Time (3,1,128) =     0.331243 ;
Time (3,1,160) =     0.335154 ;
Time (4,1,1) =      18.5556 ;
Time (4,1,2) =      9.55403 ;
Time (4,1,4) =      4.93532 ;
Time (4,1,8) =      2.51188 ;
Time (4,1,16) =      1.34385 ;
Time (4,1,32) =     0.830775 ;
Time (4,1,64) =     0.790406 ;
Time (4,1,128) =     0.965169 ;
Time (4,1,160) =      1.27834 ;
Time (5,1,1) =      16.4768 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale24-ef16/graph500-scale24-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 8860450 ;
Nedges (id) = 260261843 ;
Ntri (id) = 9936161560 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.729791 ;
T_prep (1,1,2) =     0.732899 ;
T_prep (1,1,4) =     0.415689 ;
T_prep (1,1,8) =     0.288743 ;
T_prep (1,1,16) =     0.370259 ;
T_prep (1,1,32) =     0.397771 ;
T_prep (1,1,64) =     0.428054 ;
T_prep (1,1,128) =     0.686332 ;
T_prep (1,1,160) =     0.559052 ;
T_prep (2,1,1) =      5.22972 ;
T_prep (2,1,2) =      6.57977 ;
T_prep (2,1,4) =      3.47828 ;
T_prep (2,1,8) =      1.83575 ;
T_prep (2,1,16) =      1.34719 ;
T_prep (2,1,32) =      0.89433 ;
T_prep (2,1,64) =     0.838579 ;
T_prep (2,1,128) =     0.621975 ;
T_prep (2,1,160) =      0.57464 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =       4445.9 ;
Time (1,1,2) =       2486.4 ;
Time (1,1,4) =       1245.4 ;
Time (1,1,8) =       658.96 ;
Time (1,1,16) =      392.679 ;
Time (1,1,32) =      271.621 ;
Time (1,1,64) =      249.241 ;
Time (1,1,128) =      441.806 ;
Time (1,1,160) =        516.4 ;
Time (2,1,1) =      7101.31 ;
Time (2,1,2) =      3710.46 ;
Time (2,1,4) =      1832.86 ;
Time (2,1,8) =       900.31 ;
Time (2,1,16) =      440.017 ;
Time (2,1,32) =      212.404 ;
Time (2,1,64) =      105.873 ;
Time (2,1,128) =      76.4467 ;
Time (2,1,160) =      89.0382 ;
Time (3,1,1) =      17437.7 ;
Time (3,1,2) =      8987.79 ;
Time (3,1,4) =      4375.21 ;
Time (3,1,8) =      2184.61 ;
Time (3,1,16) =       1177.2 ;
Time (3,1,32) =      862.891 ;
Time (3,1,64) =      683.788 ;
Time (3,1,128) =      712.008 ;
Time (3,1,160) =      702.142 ;
Time (4,1,1) =      4641.32 ;
Time (4,1,2) =      2590.49 ;
Time (4,1,4) =      1293.03 ;
Time (4,1,8) =      659.708 ;
Time (4,1,16) =      372.131 ;
Time (4,1,32) =      226.722 ;
Time (4,1,64) =      256.258 ;
Time (4,1,128) =      450.874 ;
Time (4,1,160) =       516.12 ;
Time (5,1,1) =      4582.12 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/image-grid/g-1073643522-268435456_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 268435456 ;
Nedges (id) = 1073643522 ;
Ntri (id) = 1073610756 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =       2.6216 ;
T_prep (1,1,2) =      2.63436 ;
T_prep (1,1,4) =      2.15323 ;
T_prep (1,1,8) =      1.79785 ;
T_prep (1,1,16) =      1.73116 ;
T_prep (1,1,32) =      1.42992 ;
T_prep (1,1,64) =       1.9694 ;
T_prep (1,1,128) =      2.64915 ;
T_prep (1,1,160) =      2.90138 ;
T_prep (2,1,1) =      27.8774 ;
T_prep (2,1,2) =       32.256 ;
T_prep (2,1,4) =      15.4881 ;
T_prep (2,1,8) =      9.66873 ;
T_prep (2,1,16) =      5.63925 ;
T_prep (2,1,32) =      3.78032 ;
T_prep (2,1,64) =      3.14056 ;
T_prep (2,1,128) =       3.0756 ;
T_prep (2,1,160) =      3.00012 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      65.5488 ;
Time (1,1,2) =      32.0842 ;
Time (1,1,4) =      16.6423 ;
Time (1,1,8) =       8.2109 ;
Time (1,1,16) =      3.99596 ;
Time (1,1,32) =       2.3105 ;
Time (1,1,64) =      1.60334 ;
Time (1,1,128) =      1.46154 ;
Time (1,1,160) =      1.45109 ;
Time (2,1,1) =      85.2888 ;
Time (2,1,2) =      48.2869 ;
Time (2,1,4) =      24.4143 ;
Time (2,1,8) =      11.3493 ;
Time (2,1,16) =      5.59027 ;
Time (2,1,32) =      2.84002 ;
Time (2,1,64) =      1.42925 ;
Time (2,1,128) =     0.825125 ;
Time (2,1,160) =     0.722103 ;
Time (3,1,1) =      93.7007 ;
Time (3,1,2) =      54.9157 ;
Time (3,1,4) =      26.9725 ;
Time (3,1,8) =       12.266 ;
Time (3,1,16) =      6.00491 ;
Time (3,1,32) =      3.04897 ;
Time (3,1,64) =      1.44326 ;
Time (3,1,128) =     0.658966 ;
Time (3,1,160) =     0.551759 ;
Time (4,1,1) =       70.071 ;
Time (4,1,2) =      36.5563 ;
Time (4,1,4) =      18.5983 ;
Time (4,1,8) =       8.2619 ;
Time (4,1,16) =      4.59316 ;
Time (4,1,32) =      2.60339 ;
Time (4,1,64) =      1.68039 ;
Time (4,1,128) =      1.40929 ;
Time (4,1,160) =      1.33901 ;
Time (5,1,1) =      51.2792 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/graph500-scale25-ef16/graph500-scale25-ef16_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 17043780 ;
Nedges (id) = 523467448 ;
Ntri (id) = 21575375802 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =      1.46665 ;
T_prep (1,1,2) =      1.48628 ;
T_prep (1,1,4) =      1.00577 ;
T_prep (1,1,8) =     0.656146 ;
T_prep (1,1,16) =     0.533049 ;
T_prep (1,1,32) =     0.842145 ;
T_prep (1,1,64) =      1.00604 ;
T_prep (1,1,128) =      1.27742 ;
T_prep (1,1,160) =      1.80621 ;
T_prep (2,1,1) =      13.0927 ;
T_prep (2,1,2) =      13.5564 ;
T_prep (2,1,4) =      7.01365 ;
T_prep (2,1,8) =      5.22476 ;
T_prep (2,1,16) =      2.70671 ;
T_prep (2,1,32) =      1.86153 ;
T_prep (2,1,64) =       1.6911 ;
T_prep (2,1,128) =      1.46792 ;
T_prep (2,1,160) =       1.5344 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      13006.5 ;
Time (1,1,2) =      7200.53 ;
Time (1,1,4) =      3683.66 ;
Time (1,1,8) =      1993.59 ;
Time (1,1,16) =      1147.49 ;
Time (1,1,32) =      805.682 ;
Time (1,1,64) =      928.752 ;
Time (1,1,128) =      1411.09 ;
Time (1,1,160) =      1539.18 ;
Time (2,1,1) =      18106.9 ;
Time (2,1,2) =      9475.18 ;
Time (2,1,4) =      4602.96 ;
Time (2,1,8) =       2274.9 ;
Time (2,1,16) =      1121.57 ;
Time (2,1,32) =      553.282 ;
Time (2,1,64) =      283.995 ;
Time (2,1,128) =      275.926 ;
Time (2,1,160) =       361.03 ;
Time (3,1,1) =      47300.5 ;
Time (3,1,2) =      24372.7 ;
Time (3,1,4) =      12062.4 ;
Time (3,1,8) =      5960.03 ;
Time (3,1,16) =      2933.47 ;
Time (3,1,32) =      1862.52 ;
Time (3,1,64) =      1416.43 ;
Time (3,1,128) =      1265.46 ;
Time (3,1,160) =      1265.75 ;
Time (4,1,1) =      13511.6 ;
Time (4,1,2) =      7318.84 ;
Time (4,1,4) =      3779.27 ;
Time (4,1,8) =      1981.72 ;
Time (4,1,16) =      1136.33 ;
Time (4,1,32) =      742.709 ;
Time (4,1,64) =      914.093 ;
Time (4,1,128) =      1403.21 ;
Time (4,1,160) =      1541.41 ;
Time (5,1,1) =        13547 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/friendster/friendster_adj.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 119432957 ;
Nedges (id) = 1799999986 ;
Ntri (id) = 191716 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =      5.72374 ;
T_prep (1,1,2) =      5.79657 ;
T_prep (1,1,4) =       5.2623 ;
T_prep (1,1,8) =      3.57307 ;
T_prep (1,1,16) =      2.57492 ;
T_prep (1,1,32) =       3.0524 ;
T_prep (1,1,64) =      3.59425 ;
T_prep (1,1,128) =      3.45737 ;
T_prep (1,1,160) =      3.80088 ;
T_prep (2,1,1) =      42.0684 ;
T_prep (2,1,2) =      47.8789 ;
T_prep (2,1,4) =      38.9921 ;
T_prep (2,1,8) =      21.1834 ;
T_prep (2,1,16) =      14.6608 ;
T_prep (2,1,32) =      8.11484 ;
T_prep (2,1,64) =      4.50374 ;
T_prep (2,1,128) =      3.24748 ;
T_prep (2,1,160) =      3.23309 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      688.321 ;
Time (1,1,2) =      371.274 ;
Time (1,1,4) =      188.149 ;
Time (1,1,8) =      100.375 ;
Time (1,1,16) =      55.1791 ;
Time (1,1,32) =      33.2479 ;
Time (1,1,64) =       28.382 ;
Time (1,1,128) =      28.8251 ;
Time (1,1,160) =      29.1792 ;
Time (2,1,1) =      692.247 ;
Time (2,1,2) =      355.973 ;
Time (2,1,4) =      196.188 ;
Time (2,1,8) =      99.7352 ;
Time (2,1,16) =      54.0408 ;
Time (2,1,32) =       31.884 ;
Time (2,1,64) =      25.7543 ;
Time (2,1,128) =      29.7031 ;
Time (2,1,160) =      36.4465 ;
Time (3,1,1) =      1394.24 ;
Time (3,1,2) =      733.806 ;
Time (3,1,4) =      350.888 ;
Time (3,1,8) =      173.874 ;
Time (3,1,16) =       87.382 ;
Time (3,1,32) =      43.7005 ;
Time (3,1,64) =      24.7221 ;
Time (3,1,128) =      22.3999 ;
Time (3,1,160) =      22.4794 ;
Time (4,1,1) =      749.832 ;
Time (4,1,2) =      386.627 ;
Time (4,1,4) =      206.433 ;
Time (4,1,8) =      107.099 ;
Time (4,1,16) =      56.9783 ;
Time (4,1,32) =      33.8177 ;
Time (4,1,64) =      28.7483 ;
Time (4,1,128) =      29.1583 ;
Time (4,1,160) =      29.3929 ;
Time (5,1,1) =      866.102 ;
T {id} = Time ;

File {id} = filetrim (file) ;

%{
file = ' /users/davis/GraphChallenge/synthetic/gc5/201512012345.v18571154_e38040320.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 18571154 ;
Nedges (id) = 19020160 ;
Ntri (id) = 2 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0691577 ;
T_prep (1,1,2) =    0.0688683 ;
T_prep (1,1,4) =     0.106273 ;
T_prep (1,1,8) =    0.0951969 ;
T_prep (1,1,16) =     0.136994 ;
T_prep (1,1,32) =     0.255138 ;
T_prep (1,1,64) =     0.382503 ;
T_prep (1,1,128) =     0.935902 ;
T_prep (1,1,160) =     0.986024 ;
T_prep (2,1,1) =     0.810332 ;
T_prep (2,1,2) =     0.947309 ;
T_prep (2,1,4) =     0.791803 ;
T_prep (2,1,8) =      0.61294 ;
T_prep (2,1,16) =     0.600486 ;
T_prep (2,1,32) =     0.282876 ;
T_prep (2,1,64) =     0.509971 ;
T_prep (2,1,128) =     0.336471 ;
T_prep (2,1,160) =     0.572378 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      2.86223 ;
Time (1,1,2) =       1.4531 ;
Time (1,1,4) =     0.674579 ;
Time (1,1,8) =     0.496243 ;
Time (1,1,16) =     0.304071 ;
Time (1,1,32) =      0.31389 ;
Time (1,1,64) =     0.176807 ;
Time (1,1,128) =     0.292298 ;
Time (1,1,160) =     0.382507 ;
Time (2,1,1) =      5.60288 ;
Time (2,1,2) =      2.75677 ;
Time (2,1,4) =      1.49547 ;
Time (2,1,8) =     0.844711 ;
Time (2,1,16) =     0.534666 ;
Time (2,1,32) =     0.360187 ;
Time (2,1,64) =     0.283883 ;
Time (2,1,128) =     0.259359 ;
Time (2,1,160) =     0.311939 ;
Time (3,1,1) =     0.779652 ;
Time (3,1,2) =     0.405246 ;
Time (3,1,4) =      0.21808 ;
Time (3,1,8) =     0.190834 ;
Time (3,1,16) =     0.188989 ;
Time (3,1,32) =     0.184526 ;
Time (3,1,64) =     0.133163 ;
Time (3,1,128) =     0.223038 ;
Time (3,1,160) =     0.230038 ;
Time (4,1,1) =      3.07702 ;
Time (4,1,2) =      1.55615 ;
Time (4,1,4) =     0.886306 ;
Time (4,1,8) =     0.582237 ;
Time (4,1,16) =     0.415242 ;
Time (4,1,32) =      0.36022 ;
Time (4,1,64) =     0.326542 ;
Time (4,1,128) =     0.473456 ;
Time (4,1,160) =     0.599716 ;
Time (5,1,1) =        9e+99 ;
T {id} = Time ;

File {id} = filetrim (file) ;
%}

file = ' /users/davis/GraphChallenge/synthetic/gc5/201512012345.v18571154_e38040320.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 18571154 ;
Nedges (id) = 19020160 ;
Ntri (id) = 2 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =    0.0700281 ;
T_prep (1,1,2) =    0.0696149 ;
T_prep (1,1,4) =     0.104408 ;
T_prep (1,1,8) =      0.10032 ;
T_prep (1,1,16) =     0.146441 ;
T_prep (1,1,32) =     0.222462 ;
T_prep (1,1,64) =     0.404043 ;
T_prep (1,1,128) =     0.783922 ;
T_prep (1,1,160) =      1.06212 ;
T_prep (2,1,1) =     0.789536 ;
T_prep (2,1,2) =     0.911193 ;
T_prep (2,1,4) =     0.557177 ;
T_prep (2,1,8) =     0.316777 ;
T_prep (2,1,16) =     0.591215 ;
T_prep (2,1,32) =     0.511988 ;
T_prep (2,1,64) =     0.541441 ;
T_prep (2,1,128) =     0.664344 ;
T_prep (2,1,160) =     0.710944 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      2.68241 ;
Time (1,1,2) =      1.38004 ;
Time (1,1,4) =      0.73797 ;
Time (1,1,8) =     0.456224 ;
Time (1,1,16) =     0.356639 ;
Time (1,1,32) =     0.252278 ;
Time (1,1,64) =     0.267536 ;
Time (1,1,128) =     0.311088 ;
Time (1,1,160) =     0.412104 ;
Time (2,1,1) =      5.55651 ;
Time (2,1,2) =      2.64698 ;
Time (2,1,4) =      1.40441 ;
Time (2,1,8) =     0.808592 ;
Time (2,1,16) =     0.505115 ;
Time (2,1,32) =     0.360425 ;
Time (2,1,64) =     0.286405 ;
Time (2,1,128) =     0.290638 ;
Time (2,1,160) =     0.295212 ;
Time (3,1,1) =     0.790173 ;
Time (3,1,2) =     0.422467 ;
Time (3,1,4) =     0.216204 ;
Time (3,1,8) =      0.19527 ;
Time (3,1,16) =     0.130009 ;
Time (3,1,32) =     0.116203 ;
Time (3,1,64) =     0.202979 ;
Time (3,1,128) =     0.225171 ;
Time (3,1,160) =     0.349501 ;
Time (4,1,1) =      3.11142 ;
Time (4,1,2) =      1.51712 ;
Time (4,1,4) =     0.858108 ;
Time (4,1,8) =      0.54497 ;
Time (4,1,16) =     0.395913 ;
Time (4,1,32) =     0.327711 ;
Time (4,1,64) =     0.332909 ;
Time (4,1,128) =     0.484252 ;
Time (4,1,160) =     0.612753 ;
Time (5,1,1) =        9e+99 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020000.v35991342_e74485420.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 35991342 ;
Nedges (id) = 37242710 ;
Ntri (id) = 2 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.132245 ;
T_prep (1,1,2) =     0.132194 ;
T_prep (1,1,4) =     0.202965 ;
T_prep (1,1,8) =     0.218795 ;
T_prep (1,1,16) =     0.524262 ;
T_prep (1,1,32) =     0.532712 ;
T_prep (1,1,64) =     0.298231 ;
T_prep (1,1,128) =      1.56709 ;
T_prep (1,1,160) =      1.00247 ;
T_prep (2,1,1) =      1.37869 ;
T_prep (2,1,2) =      1.84056 ;
T_prep (2,1,4) =      1.38976 ;
T_prep (2,1,8) =      1.12669 ;
T_prep (2,1,16) =      1.05998 ;
T_prep (2,1,32) =      0.95524 ;
T_prep (2,1,64) =     0.981792 ;
T_prep (2,1,128) =      1.01701 ;
T_prep (2,1,160) =     0.933572 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      9.57013 ;
Time (1,1,2) =      4.81282 ;
Time (1,1,4) =       2.6397 ;
Time (1,1,8) =       1.5559 ;
Time (1,1,16) =       1.0161 ;
Time (1,1,32) =     0.749339 ;
Time (1,1,64) =     0.700644 ;
Time (1,1,128) =     0.824185 ;
Time (1,1,160) =     0.879943 ;
Time (2,1,1) =      22.2723 ;
Time (2,1,2) =      10.5049 ;
Time (2,1,4) =      5.59939 ;
Time (2,1,8) =      2.93138 ;
Time (2,1,16) =      1.59624 ;
Time (2,1,32) =      1.00881 ;
Time (2,1,64) =     0.779496 ;
Time (2,1,128) =     0.698007 ;
Time (2,1,160) =     0.735389 ;
Time (3,1,1) =      1.66638 ;
Time (3,1,2) =     0.886018 ;
Time (3,1,4) =     0.452607 ;
Time (3,1,8) =     0.397378 ;
Time (3,1,16) =     0.247079 ;
Time (3,1,32) =     0.399908 ;
Time (3,1,64) =      0.42709 ;
Time (3,1,128) =     0.423791 ;
Time (3,1,160) =     0.442652 ;
Time (4,1,1) =      10.4758 ;
Time (4,1,2) =      5.18672 ;
Time (4,1,4) =      2.82868 ;
Time (4,1,8) =      1.64055 ;
Time (4,1,16) =      1.09231 ;
Time (4,1,32) =     0.831555 ;
Time (4,1,64) =     0.839769 ;
Time (4,1,128) =     0.869376 ;
Time (4,1,160) =     0.915444 ;
Time (5,1,1) =        9e+99 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020030.v68863315_e143414960.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 68863315 ;
Nedges (id) = 71707480 ;
Ntri (id) = 6 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.250981 ;
T_prep (1,1,2) =     0.250846 ;
T_prep (1,1,4) =     0.404274 ;
T_prep (1,1,8) =     0.318777 ;
T_prep (1,1,16) =     0.396064 ;
T_prep (1,1,32) =     0.812457 ;
T_prep (1,1,64) =        1.446 ;
T_prep (1,1,128) =      2.76631 ;
T_prep (1,1,160) =      1.60097 ;
T_prep (2,1,1) =      2.75238 ;
T_prep (2,1,2) =      3.20621 ;
T_prep (2,1,4) =       1.8325 ;
T_prep (2,1,8) =      1.89242 ;
T_prep (2,1,16) =      1.47053 ;
T_prep (2,1,32) =      1.75169 ;
T_prep (2,1,64) =      1.97571 ;
T_prep (2,1,128) =      1.83353 ;
T_prep (2,1,160) =      1.82704 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =       34.457 ;
Time (1,1,2) =      18.2423 ;
Time (1,1,4) =      9.43647 ;
Time (1,1,8) =      5.09646 ;
Time (1,1,16) =      3.39951 ;
Time (1,1,32) =      2.81145 ;
Time (1,1,64) =      2.25758 ;
Time (1,1,128) =      2.38902 ;
Time (1,1,160) =      2.39252 ;
Time (2,1,1) =      82.4828 ;
Time (2,1,2) =      39.3957 ;
Time (2,1,4) =      20.3503 ;
Time (2,1,8) =      11.2488 ;
Time (2,1,16) =      6.23864 ;
Time (2,1,32) =      3.90594 ;
Time (2,1,64) =      2.98963 ;
Time (2,1,128) =      2.81406 ;
Time (2,1,160) =      3.42306 ;
Time (3,1,1) =      3.39809 ;
Time (3,1,2) =      1.78259 ;
Time (3,1,4) =     0.901757 ;
Time (3,1,8) =     0.729823 ;
Time (3,1,16) =     0.742752 ;
Time (3,1,32) =     0.793647 ;
Time (3,1,64) =     0.839559 ;
Time (3,1,128) =     0.812356 ;
Time (3,1,160) =     0.827098 ;
Time (4,1,1) =      36.9354 ;
Time (4,1,2) =      18.7587 ;
Time (4,1,4) =      10.0278 ;
Time (4,1,8) =      5.72051 ;
Time (4,1,16) =      3.80291 ;
Time (4,1,32) =      2.70444 ;
Time (4,1,64) =      2.33833 ;
Time (4,1,128) =      2.56186 ;
Time (4,1,160) =      2.52974 ;
Time (5,1,1) =        9e+99 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020130.v128568730_e270234840.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 128568730 ;
Nedges (id) = 135117420 ;
Ntri (id) = 10 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =      0.46847 ;
T_prep (1,1,2) =     0.468198 ;
T_prep (1,1,4) =     0.709144 ;
T_prep (1,1,8) =     0.699723 ;
T_prep (1,1,16) =     0.756727 ;
T_prep (1,1,32) =       1.4973 ;
T_prep (1,1,64) =      2.56032 ;
T_prep (1,1,128) =      3.85919 ;
T_prep (1,1,160) =      4.78737 ;
T_prep (2,1,1) =      5.31402 ;
T_prep (2,1,2) =      5.99012 ;
T_prep (2,1,4) =      4.72328 ;
T_prep (2,1,8) =      3.58808 ;
T_prep (2,1,16) =        3.509 ;
T_prep (2,1,32) =      2.57459 ;
T_prep (2,1,64) =      3.37579 ;
T_prep (2,1,128) =      3.50227 ;
T_prep (2,1,160) =      3.49905 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      112.767 ;
Time (1,1,2) =      57.6106 ;
Time (1,1,4) =      29.4461 ;
Time (1,1,8) =      16.9596 ;
Time (1,1,16) =      11.1897 ;
Time (1,1,32) =      7.06417 ;
Time (1,1,64) =      5.71137 ;
Time (1,1,128) =      4.90749 ;
Time (1,1,160) =      5.40766 ;
Time (2,1,1) =      286.716 ;
Time (2,1,2) =      138.105 ;
Time (2,1,4) =      74.1326 ;
Time (2,1,8) =       36.366 ;
Time (2,1,16) =      19.9186 ;
Time (2,1,32) =       11.584 ;
Time (2,1,64) =      7.78681 ;
Time (2,1,128) =      6.59507 ;
Time (2,1,160) =      8.90788 ;
Time (3,1,1) =      7.01489 ;
Time (3,1,2) =      3.60013 ;
Time (3,1,4) =      1.84372 ;
Time (3,1,8) =      1.44386 ;
Time (3,1,16) =      1.32665 ;
Time (3,1,32) =      1.31598 ;
Time (3,1,64) =      1.57533 ;
Time (3,1,128) =      1.57983 ;
Time (3,1,160) =      1.68455 ;
Time (4,1,1) =      123.828 ;
Time (4,1,2) =      62.2546 ;
Time (4,1,4) =      33.3198 ;
Time (4,1,8) =      19.2011 ;
Time (4,1,16) =      12.1583 ;
Time (4,1,32) =      7.39401 ;
Time (4,1,64) =      5.45432 ;
Time (4,1,128) =      5.41413 ;
Time (4,1,160) =      5.60346 ;
Time (5,1,1) =        9e+99 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/synthetic/gc5/201512020330.v226196185_e480047894.tsv.gz ' ;

%%%%%%%%%%%%%%%%%%%%
id = id + 1 ;
N (id) = 226196185 ;
Nedges (id) = 240023945 ;
Ntri (id) = 26 ;
T_prep = nan (2, 1, 160) ; 
% prep: 1:L, 2:U, 3:Lperm, 4:Uperm
% T_prep (1, prep_method, nthreads): for all tri methods
% T_prep (2, prep_method, nthreads): just for tri_dot
% tri_method: 1:mark 2:bit 3:dot 4:logmark 5:simple
% Time (tri_method, prep_method, nthreads)
T_prep (1,1,1) =     0.848128 ;
T_prep (1,1,2) =     0.839799 ;
T_prep (1,1,4) =      1.50079 ;
T_prep (1,1,8) =      1.57966 ;
T_prep (1,1,16) =      2.17907 ;
T_prep (1,1,32) =      2.53564 ;
T_prep (1,1,64) =       2.0425 ;
T_prep (1,1,128) =      6.77657 ;
T_prep (1,1,160) =      6.35754 ;
T_prep (2,1,1) =      9.45816 ;
T_prep (2,1,2) =      11.0944 ;
T_prep (2,1,4) =      9.35063 ;
T_prep (2,1,8) =       7.8702 ;
T_prep (2,1,16) =      4.80363 ;
T_prep (2,1,32) =      5.76358 ;
T_prep (2,1,64) =       5.0586 ;
T_prep (2,1,128) =      5.62313 ;
T_prep (2,1,160) =      5.97607 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      465.723 ;
Time (1,1,2) =       256.89 ;
Time (1,1,4) =      126.377 ;
Time (1,1,8) =      65.4441 ;
Time (1,1,16) =      36.0429 ;
Time (1,1,32) =      22.6584 ;
Time (1,1,64) =       14.289 ;
Time (1,1,128) =      11.8142 ;
Time (1,1,160) =      12.0407 ;
Time (2,1,1) =      1036.99 ;
Time (2,1,2) =      484.705 ;
Time (2,1,4) =       264.86 ;
Time (2,1,8) =      139.664 ;
Time (2,1,16) =      71.0749 ;
Time (2,1,32) =      37.9128 ;
Time (2,1,64) =      24.0726 ;
Time (2,1,128) =      15.5865 ;
Time (2,1,160) =      25.0329 ;
Time (3,1,1) =      14.7772 ;
Time (3,1,2) =      8.02965 ;
Time (3,1,4) =      3.67438 ;
Time (3,1,8) =      2.78143 ;
Time (3,1,16) =      2.69285 ;
Time (3,1,32) =      2.35618 ;
Time (3,1,64) =      2.33084 ;
Time (3,1,128) =       2.9955 ;
Time (3,1,160) =      3.06326 ;
Time (4,1,1) =      521.197 ;
Time (4,1,2) =      273.147 ;
Time (4,1,4) =      138.667 ;
Time (4,1,8) =      71.1577 ;
Time (4,1,16) =      39.3824 ;
Time (4,1,32) =      23.5991 ;
Time (4,1,64) =        14.31 ;
Time (4,1,128) =      12.5483 ;
Time (4,1,160) =      12.1243 ;
Time (5,1,1) =        9e+99 ;
T {id} = Time ;

File {id} = filetrim (file) ;

