function [T, Tprep, N, Nedges, Ntri, File] = tri_results
id = 0 ;
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
T_prep (1,1,1) =   0.00172177 ;
T_prep (1,1,2) =   0.00159929 ;
T_prep (1,1,4) =    0.0099232 ;
T_prep (1,1,8) =   0.00061516 ;
T_prep (1,1,16) =  0.000420875 ;
T_prep (1,1,32) =  0.000394518 ;
T_prep (1,1,64) =  0.000395483 ;
T_prep (1,1,128) =  0.000392887 ;
T_prep (1,1,160) =  0.000395406 ;
T_prep (2,1,1) =   0.00175747 ;
T_prep (2,1,2) =   0.00155222 ;
T_prep (2,1,4) =  0.000967097 ;
T_prep (2,1,8) =  0.000579378 ;
T_prep (2,1,16) =  0.000405272 ;
T_prep (2,1,32) =   0.00038354 ;
T_prep (2,1,64) =  0.000380456 ;
T_prep (2,1,128) =  0.000378498 ;
T_prep (2,1,160) =  0.000379262 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0122288 ;
Time (1,1,2) =   0.00754383 ;
Time (1,1,4) =   0.00379281 ;
Time (1,1,8) =   0.00190407 ;
Time (1,1,16) =   0.00183442 ;
Time (1,1,32) =   0.00467137 ;
Time (1,1,64) =   0.00585735 ;
Time (1,1,128) =    0.0292829 ;
Time (1,1,160) =    0.0208727 ;
Time (2,1,1) =     0.196432 ;
Time (2,1,2) =     0.096601 ;
Time (2,1,4) =    0.0461234 ;
Time (2,1,8) =    0.0212591 ;
Time (2,1,16) =       0.0147 ;
Time (2,1,32) =    0.0123689 ;
Time (2,1,64) =    0.0113109 ;
Time (2,1,128) =    0.0244843 ;
Time (2,1,160) =    0.0335951 ;
Time (3,1,1) =     0.579493 ;
Time (3,1,2) =     0.283587 ;
Time (3,1,4) =     0.142558 ;
Time (3,1,8) =    0.0704943 ;
Time (3,1,16) =    0.0479423 ;
Time (3,1,32) =    0.0423779 ;
Time (3,1,64) =    0.0916506 ;
Time (3,1,128) =    0.0426447 ;
Time (3,1,160) =    0.0539626 ;
Time (4,1,1) =    0.0872232 ;
Time (4,1,2) =    0.0428946 ;
Time (4,1,4) =    0.0209487 ;
Time (4,1,8) =   0.00979565 ;
Time (4,1,16) =   0.00570474 ;
Time (4,1,32) =   0.00573493 ;
Time (4,1,64) =   0.00630966 ;
Time (4,1,128) =   0.00505143 ;
Time (4,1,160) =   0.00567714 ;
Time (5,1,1) =    0.0748151 ;
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
T_prep (1,1,1) =   0.00215169 ;
T_prep (1,1,2) =   0.00213172 ;
T_prep (1,1,4) =    0.0109282 ;
T_prep (1,1,8) =   0.00133665 ;
T_prep (1,1,16) =   0.00121882 ;
T_prep (1,1,32) =    0.0012298 ;
T_prep (1,1,64) =   0.00123333 ;
T_prep (1,1,128) =   0.00123885 ;
T_prep (1,1,160) =   0.00125613 ;
T_prep (2,1,1) =   0.00253202 ;
T_prep (2,1,2) =   0.00214532 ;
T_prep (2,1,4) =   0.00243376 ;
T_prep (2,1,8) =   0.00118696 ;
T_prep (2,1,16) =   0.00117818 ;
T_prep (2,1,32) =   0.00117486 ;
T_prep (2,1,64) =   0.00117802 ;
T_prep (2,1,128) =   0.00118144 ;
T_prep (2,1,160) =   0.00118114 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0118194 ;
Time (1,1,2) =   0.00882767 ;
Time (1,1,4) =    0.0053405 ;
Time (1,1,8) =   0.00239646 ;
Time (1,1,16) =   0.00230489 ;
Time (1,1,32) =   0.00213822 ;
Time (1,1,64) =   0.00371994 ;
Time (1,1,128) =     0.023929 ;
Time (1,1,160) =    0.0422437 ;
Time (2,1,1) =     0.199444 ;
Time (2,1,2) =     0.090692 ;
Time (2,1,4) =    0.0425724 ;
Time (2,1,8) =    0.0198362 ;
Time (2,1,16) =    0.0181026 ;
Time (2,1,32) =   0.00869824 ;
Time (2,1,64) =    0.0049364 ;
Time (2,1,128) =    0.0041164 ;
Time (2,1,160) =    0.0109694 ;
Time (3,1,1) =     0.435986 ;
Time (3,1,2) =     0.212671 ;
Time (3,1,4) =       0.1016 ;
Time (3,1,8) =    0.0486389 ;
Time (3,1,16) =    0.0237879 ;
Time (3,1,32) =    0.0144228 ;
Time (3,1,64) =    0.0184733 ;
Time (3,1,128) =     0.011262 ;
Time (3,1,160) =    0.0281922 ;
Time (4,1,1) =    0.0909414 ;
Time (4,1,2) =    0.0445853 ;
Time (4,1,4) =    0.0225091 ;
Time (4,1,8) =    0.0112915 ;
Time (4,1,16) =   0.00584894 ;
Time (4,1,32) =   0.00385881 ;
Time (4,1,64) =    0.0035955 ;
Time (4,1,128) =   0.00403222 ;
Time (4,1,160) =    0.0105378 ;
Time (5,1,1) =     0.283936 ;
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
T_prep (1,1,1) =   0.00174613 ;
T_prep (1,1,2) =   0.00174408 ;
T_prep (1,1,4) =    0.0102386 ;
T_prep (1,1,8) =  0.000839096 ;
T_prep (1,1,16) =  0.000633582 ;
T_prep (1,1,32) =  0.000613992 ;
T_prep (1,1,64) =  0.000616338 ;
T_prep (1,1,128) =  0.000665922 ;
T_prep (1,1,160) =  0.000674273 ;
T_prep (2,1,1) =   0.00203371 ;
T_prep (2,1,2) =   0.00174673 ;
T_prep (2,1,4) =   0.00128628 ;
T_prep (2,1,8) =  0.000759095 ;
T_prep (2,1,16) =  0.000693751 ;
T_prep (2,1,32) =   0.00070261 ;
T_prep (2,1,64) =  0.000696656 ;
T_prep (2,1,128) =  0.000699433 ;
T_prep (2,1,160) =  0.000700285 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.018514 ;
Time (1,1,2) =    0.0123019 ;
Time (1,1,4) =   0.00677538 ;
Time (1,1,8) =   0.00316501 ;
Time (1,1,16) =   0.00288289 ;
Time (1,1,32) =   0.00472351 ;
Time (1,1,64) =    0.0151164 ;
Time (1,1,128) =    0.0285686 ;
Time (1,1,160) =    0.0240511 ;
Time (2,1,1) =     0.398338 ;
Time (2,1,2) =     0.195772 ;
Time (2,1,4) =     0.089314 ;
Time (2,1,8) =    0.0427568 ;
Time (2,1,16) =    0.0366971 ;
Time (2,1,32) =    0.0367389 ;
Time (2,1,64) =    0.0324279 ;
Time (2,1,128) =    0.0370532 ;
Time (2,1,160) =    0.0347489 ;
Time (3,1,1) =      1.33743 ;
Time (3,1,2) =     0.708084 ;
Time (3,1,4) =     0.367683 ;
Time (3,1,8) =     0.234659 ;
Time (3,1,16) =     0.155909 ;
Time (3,1,32) =     0.133575 ;
Time (3,1,64) =     0.139907 ;
Time (3,1,128) =     0.210402 ;
Time (3,1,160) =     0.228959 ;
Time (4,1,1) =     0.128571 ;
Time (4,1,2) =    0.0660288 ;
Time (4,1,4) =    0.0331695 ;
Time (4,1,8) =    0.0166085 ;
Time (4,1,16) =    0.0158287 ;
Time (4,1,32) =    0.0144873 ;
Time (4,1,64) =   0.00938948 ;
Time (4,1,128) =    0.0090725 ;
Time (4,1,160) =   0.00635095 ;
Time (5,1,1) =      0.12141 ;
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
T_prep (1,1,1) =   0.00220339 ;
T_prep (1,1,2) =   0.00219794 ;
T_prep (1,1,4) =    0.0105152 ;
T_prep (1,1,8) =   0.00149857 ;
T_prep (1,1,16) =   0.00129807 ;
T_prep (1,1,32) =   0.00108569 ;
T_prep (1,1,64) =   0.00110022 ;
T_prep (1,1,128) =   0.00109902 ;
T_prep (1,1,160) =   0.00111153 ;
T_prep (2,1,1) =   0.00256478 ;
T_prep (2,1,2) =   0.00220822 ;
T_prep (2,1,4) =   0.00166903 ;
T_prep (2,1,8) =   0.00108518 ;
T_prep (2,1,16) =  0.000830401 ;
T_prep (2,1,32) =  0.000828937 ;
T_prep (2,1,64) =  0.000829672 ;
T_prep (2,1,128) =  0.000837812 ;
T_prep (2,1,160) =  0.000839303 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0217925 ;
Time (1,1,2) =    0.0164594 ;
Time (1,1,4) =   0.00851251 ;
Time (1,1,8) =   0.00388461 ;
Time (1,1,16) =   0.00486799 ;
Time (1,1,32) =    0.0065027 ;
Time (1,1,64) =    0.0112705 ;
Time (1,1,128) =    0.0506854 ;
Time (1,1,160) =     0.025578 ;
Time (2,1,1) =     0.449198 ;
Time (2,1,2) =     0.201236 ;
Time (2,1,4) =    0.0961724 ;
Time (2,1,8) =    0.0442138 ;
Time (2,1,16) =    0.0347344 ;
Time (2,1,32) =    0.0304745 ;
Time (2,1,64) =    0.0407797 ;
Time (2,1,128) =    0.0489277 ;
Time (2,1,160) =    0.0280076 ;
Time (3,1,1) =      1.16894 ;
Time (3,1,2) =     0.591891 ;
Time (3,1,4) =      0.33867 ;
Time (3,1,8) =     0.220315 ;
Time (3,1,16) =     0.170014 ;
Time (3,1,32) =     0.131553 ;
Time (3,1,64) =     0.127014 ;
Time (3,1,128) =     0.283506 ;
Time (3,1,160) =     0.212026 ;
Time (4,1,1) =      0.14859 ;
Time (4,1,2) =    0.0777948 ;
Time (4,1,4) =    0.0392363 ;
Time (4,1,8) =    0.0197891 ;
Time (4,1,16) =    0.0130086 ;
Time (4,1,32) =    0.0133412 ;
Time (4,1,64) =    0.0138385 ;
Time (4,1,128) =     0.010518 ;
Time (4,1,160) =   0.00452247 ;
Time (5,1,1) =     0.139655 ;
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
T_prep (1,1,1) =   0.00733979 ;
T_prep (1,1,2) =   0.00717259 ;
T_prep (1,1,4) =    0.0132894 ;
T_prep (1,1,8) =   0.00418624 ;
T_prep (1,1,16) =   0.00234925 ;
T_prep (1,1,32) =   0.00335234 ;
T_prep (1,1,64) =   0.00294828 ;
T_prep (1,1,128) =   0.00188383 ;
T_prep (1,1,160) =   0.00186422 ;
T_prep (2,1,1) =    0.0200621 ;
T_prep (2,1,2) =    0.0216051 ;
T_prep (2,1,4) =    0.0139994 ;
T_prep (2,1,8) =   0.00816896 ;
T_prep (2,1,16) =    0.0040576 ;
T_prep (2,1,32) =   0.00246876 ;
T_prep (2,1,64) =   0.00185912 ;
T_prep (2,1,128) =   0.00158692 ;
T_prep (2,1,160) =   0.00157906 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =     0.593228 ;
Time (1,1,2) =     0.326745 ;
Time (1,1,4) =     0.162824 ;
Time (1,1,8) =    0.0932065 ;
Time (1,1,16) =    0.0677845 ;
Time (1,1,32) =    0.0702636 ;
Time (1,1,64) =    0.0825883 ;
Time (1,1,128) =    0.0980902 ;
Time (1,1,160) =    0.0882717 ;
Time (2,1,1) =      4.06352 ;
Time (2,1,2) =      2.15187 ;
Time (2,1,4) =     0.782527 ;
Time (2,1,8) =     0.463504 ;
Time (2,1,16) =      0.54339 ;
Time (2,1,32) =     0.303844 ;
Time (2,1,64) =     0.286989 ;
Time (2,1,128) =     0.372933 ;
Time (2,1,160) =     0.213527 ;
Time (3,1,1) =      17.2696 ;
Time (3,1,2) =      8.52598 ;
Time (3,1,4) =      3.39734 ;
Time (3,1,8) =      1.84423 ;
Time (3,1,16) =      1.00018 ;
Time (3,1,32) =     0.637341 ;
Time (3,1,64) =      0.77338 ;
Time (3,1,128) =     0.708011 ;
Time (3,1,160) =     0.612325 ;
Time (4,1,1) =      1.36097 ;
Time (4,1,2) =     0.683428 ;
Time (4,1,4) =     0.314546 ;
Time (4,1,8) =     0.176254 ;
Time (4,1,16) =     0.103766 ;
Time (4,1,32) =     0.108882 ;
Time (4,1,64) =    0.0849441 ;
Time (4,1,128) =    0.0982898 ;
Time (4,1,160) =     0.219638 ;
Time (5,1,1) =      1.29537 ;
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
T_prep (1,1,1) =    0.0832786 ;
T_prep (1,1,2) =    0.0832394 ;
T_prep (1,1,4) =    0.0647605 ;
T_prep (1,1,8) =    0.0482139 ;
T_prep (1,1,16) =    0.0489521 ;
T_prep (1,1,32) =    0.0378229 ;
T_prep (1,1,64) =    0.0450291 ;
T_prep (1,1,128) =    0.0657901 ;
T_prep (1,1,160) =    0.0901113 ;
T_prep (2,1,1) =      0.60908 ;
T_prep (2,1,2) =     0.657463 ;
T_prep (2,1,4) =     0.442365 ;
T_prep (2,1,8) =     0.234065 ;
T_prep (2,1,16) =      0.17469 ;
T_prep (2,1,32) =     0.102708 ;
T_prep (2,1,64) =    0.0778727 ;
T_prep (2,1,128) =    0.0647799 ;
T_prep (2,1,160) =    0.0607464 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =      3.52044 ;
Time (1,1,2) =      1.76475 ;
Time (1,1,4) =     0.860002 ;
Time (1,1,8) =     0.423721 ;
Time (1,1,16) =     0.227632 ;
Time (1,1,32) =     0.131198 ;
Time (1,1,64) =     0.119124 ;
Time (1,1,128) =     0.149866 ;
Time (1,1,160) =     0.165292 ;
Time (2,1,1) =      3.92377 ;
Time (2,1,2) =      2.12131 ;
Time (2,1,4) =      1.01931 ;
Time (2,1,8) =     0.480549 ;
Time (2,1,16) =     0.237128 ;
Time (2,1,32) =     0.116611 ;
Time (2,1,64) =     0.067831 ;
Time (2,1,128) =    0.0705961 ;
Time (2,1,160) =    0.0695417 ;
Time (3,1,1) =      7.18595 ;
Time (3,1,2) =      3.75441 ;
Time (3,1,4) =      1.86724 ;
Time (3,1,8) =     0.904827 ;
Time (3,1,16) =     0.442388 ;
Time (3,1,32) =     0.219984 ;
Time (3,1,64) =     0.125101 ;
Time (3,1,128) =     0.113114 ;
Time (3,1,160) =     0.117255 ;
Time (4,1,1) =      3.62881 ;
Time (4,1,2) =      1.92004 ;
Time (4,1,4) =     0.939712 ;
Time (4,1,8) =     0.438684 ;
Time (4,1,16) =     0.247607 ;
Time (4,1,32) =     0.136795 ;
Time (4,1,64) =     0.110929 ;
Time (4,1,128) =     0.145937 ;
Time (4,1,160) =     0.157412 ;
Time (5,1,1) =      3.29286 ;
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
T_prep (1,1,1) =   0.00533128 ;
T_prep (1,1,2) =   0.00530913 ;
T_prep (1,1,4) =    0.0123212 ;
T_prep (1,1,8) =   0.00185314 ;
T_prep (1,1,16) =   0.00191198 ;
T_prep (1,1,32) =   0.00134487 ;
T_prep (1,1,64) =   0.00128675 ;
T_prep (1,1,128) =     0.001314 ;
T_prep (1,1,160) =   0.00133804 ;
T_prep (2,1,1) =   0.00630471 ;
T_prep (2,1,2) =   0.00546483 ;
T_prep (2,1,4) =   0.00546332 ;
T_prep (2,1,8) =   0.00279296 ;
T_prep (2,1,16) =   0.00148509 ;
T_prep (2,1,32) =   0.00135435 ;
T_prep (2,1,64) =   0.00132286 ;
T_prep (2,1,128) =   0.00133467 ;
T_prep (2,1,160) =   0.00134267 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0194887 ;
Time (1,1,2) =    0.0126948 ;
Time (1,1,4) =   0.00714006 ;
Time (1,1,8) =   0.00395695 ;
Time (1,1,16) =   0.00195887 ;
Time (1,1,32) =   0.00199372 ;
Time (1,1,64) =   0.00345989 ;
Time (1,1,128) =    0.0185475 ;
Time (1,1,160) =    0.0316882 ;
Time (2,1,1) =     0.139093 ;
Time (2,1,2) =    0.0734908 ;
Time (2,1,4) =     0.034775 ;
Time (2,1,8) =    0.0178267 ;
Time (2,1,16) =   0.00873007 ;
Time (2,1,32) =   0.00458344 ;
Time (2,1,64) =   0.00609534 ;
Time (2,1,128) =    0.0184151 ;
Time (2,1,160) =   0.00479449 ;
Time (3,1,1) =     0.188339 ;
Time (3,1,2) =     0.100398 ;
Time (3,1,4) =    0.0479633 ;
Time (3,1,8) =     0.024461 ;
Time (3,1,16) =    0.0115969 ;
Time (3,1,32) =   0.00597034 ;
Time (3,1,64) =   0.00692002 ;
Time (3,1,128) =   0.00367406 ;
Time (3,1,160) =    0.0154982 ;
Time (4,1,1) =     0.112444 ;
Time (4,1,2) =    0.0581958 ;
Time (4,1,4) =    0.0260675 ;
Time (4,1,8) =    0.0135944 ;
Time (4,1,16) =   0.00696451 ;
Time (4,1,32) =   0.00383737 ;
Time (4,1,64) =   0.00447709 ;
Time (4,1,128) =   0.00438581 ;
Time (4,1,160) =    0.0126642 ;
Time (5,1,1) =    0.0856413 ;
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
T_prep (1,1,1) =  8.50661e-05 ;
T_prep (1,1,2) =  8.38274e-05 ;
T_prep (1,1,4) =  8.32211e-05 ;
T_prep (1,1,8) =  8.30665e-05 ;
T_prep (1,1,16) =   8.3074e-05 ;
T_prep (1,1,32) =  8.29278e-05 ;
T_prep (1,1,64) =  8.28486e-05 ;
T_prep (1,1,128) =  8.28439e-05 ;
T_prep (1,1,160) =  8.26642e-05 ;
T_prep (2,1,1) =  8.93241e-05 ;
T_prep (2,1,2) =  8.30023e-05 ;
T_prep (2,1,4) =  8.03042e-05 ;
T_prep (2,1,8) =  7.92136e-05 ;
T_prep (2,1,16) =   7.8789e-05 ;
T_prep (2,1,32) =  7.83894e-05 ;
T_prep (2,1,64) =  7.84667e-05 ;
T_prep (2,1,128) =   7.8219e-05 ;
T_prep (2,1,160) =  7.83829e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000204409 ;
Time (1,1,2) =   0.00913808 ;
Time (1,1,4) =  0.000134269 ;
Time (1,1,8) =  0.000191727 ;
Time (1,1,16) =  0.000335066 ;
Time (1,1,32) =  0.000810037 ;
Time (1,1,64) =   0.00222954 ;
Time (1,1,128) =    0.0396388 ;
Time (1,1,160) =    0.0292245 ;
Time (2,1,1) =   0.00250841 ;
Time (2,1,2) =    0.0011946 ;
Time (2,1,4) =   0.00110941 ;
Time (2,1,8) =   0.00116954 ;
Time (2,1,16) =   0.00115584 ;
Time (2,1,32) =   0.00106059 ;
Time (2,1,64) =    0.0162548 ;
Time (2,1,128) =   0.00674562 ;
Time (2,1,160) =   0.00064766 ;
Time (3,1,1) =   0.00376333 ;
Time (3,1,2) =   0.00272124 ;
Time (3,1,4) =   0.00203726 ;
Time (3,1,8) =    0.0019485 ;
Time (3,1,16) =   0.00181902 ;
Time (3,1,32) =   0.00176924 ;
Time (3,1,64) =      0.00172 ;
Time (3,1,128) =   0.00329123 ;
Time (3,1,160) =   0.00162727 ;
Time (4,1,1) =   0.00157093 ;
Time (4,1,2) =  0.000763775 ;
Time (4,1,4) =   0.00053339 ;
Time (4,1,8) =  0.000537995 ;
Time (4,1,16) =  0.000624751 ;
Time (4,1,32) =   0.00128062 ;
Time (4,1,64) =  0.000617419 ;
Time (4,1,128) =   0.00215557 ;
Time (4,1,160) =  0.000284475 ;
Time (5,1,1) =   0.00152723 ;
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
T_prep (1,1,1) =  0.000761062 ;
T_prep (1,1,2) =  0.000760148 ;
T_prep (1,1,4) =   0.00947682 ;
T_prep (1,1,8) =   0.00038939 ;
T_prep (1,1,16) =  0.000388402 ;
T_prep (1,1,32) =  0.000389406 ;
T_prep (1,1,64) =  0.000387115 ;
T_prep (1,1,128) =  0.000387007 ;
T_prep (1,1,160) =  0.000387843 ;
T_prep (2,1,1) =  0.000847824 ;
T_prep (2,1,2) =  0.000736702 ;
T_prep (2,1,4) =  0.000468294 ;
T_prep (2,1,8) =  0.000400668 ;
T_prep (2,1,16) =  0.000397982 ;
T_prep (2,1,32) =  0.000419151 ;
T_prep (2,1,64) =  0.000399346 ;
T_prep (2,1,128) =  0.000398797 ;
T_prep (2,1,160) =   0.00040034 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00540217 ;
Time (1,1,2) =   0.00335841 ;
Time (1,1,4) =   0.00161057 ;
Time (1,1,8) =   0.00116873 ;
Time (1,1,16) =   0.00146888 ;
Time (1,1,32) =    0.0023417 ;
Time (1,1,64) =   0.00473649 ;
Time (1,1,128) =    0.0235978 ;
Time (1,1,160) =    0.0236769 ;
Time (2,1,1) =    0.0834667 ;
Time (2,1,2) =    0.0492075 ;
Time (2,1,4) =    0.0230388 ;
Time (2,1,8) =     0.012689 ;
Time (2,1,16) =    0.0116279 ;
Time (2,1,32) =    0.0124123 ;
Time (2,1,64) =    0.0106075 ;
Time (2,1,128) =    0.0208374 ;
Time (2,1,160) =    0.0220545 ;
Time (3,1,1) =     0.277274 ;
Time (3,1,2) =     0.149733 ;
Time (3,1,4) =    0.0827538 ;
Time (3,1,8) =    0.0494157 ;
Time (3,1,16) =    0.0403221 ;
Time (3,1,32) =    0.0406969 ;
Time (3,1,64) =    0.0424608 ;
Time (3,1,128) =    0.0906851 ;
Time (3,1,160) =    0.0596359 ;
Time (4,1,1) =    0.0335596 ;
Time (4,1,2) =    0.0186093 ;
Time (4,1,4) =   0.00943022 ;
Time (4,1,8) =   0.00632686 ;
Time (4,1,16) =   0.00428994 ;
Time (4,1,32) =    0.0046295 ;
Time (4,1,64) =   0.00497019 ;
Time (4,1,128) =   0.00513665 ;
Time (4,1,160) =   0.00434534 ;
Time (5,1,1) =    0.0304096 ;
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
T_prep (1,1,1) =  0.000523167 ;
T_prep (1,1,2) =   0.00052223 ;
T_prep (1,1,4) =  0.000521403 ;
T_prep (1,1,8) =  0.000522205 ;
T_prep (1,1,16) =  0.000521895 ;
T_prep (1,1,32) =  0.000522001 ;
T_prep (1,1,64) =  0.000522131 ;
T_prep (1,1,128) =  0.000521845 ;
T_prep (1,1,160) =  0.000521746 ;
T_prep (2,1,1) =  0.000549096 ;
T_prep (2,1,2) =  0.000510162 ;
T_prep (2,1,4) =  0.000509229 ;
T_prep (2,1,8) =  0.000508229 ;
T_prep (2,1,16) =  0.000507902 ;
T_prep (2,1,32) =  0.000531025 ;
T_prep (2,1,64) =  0.000507803 ;
T_prep (2,1,128) =  0.000507808 ;
T_prep (2,1,160) =  0.000507423 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00195272 ;
Time (1,1,2) =    0.0100139 ;
Time (1,1,4) =   0.00068501 ;
Time (1,1,8) =  0.000493414 ;
Time (1,1,16) =   0.00057496 ;
Time (1,1,32) =  0.000956435 ;
Time (1,1,64) =   0.00264913 ;
Time (1,1,128) =    0.0226349 ;
Time (1,1,160) =    0.0277254 ;
Time (2,1,1) =    0.0208155 ;
Time (2,1,2) =    0.0103247 ;
Time (2,1,4) =   0.00523089 ;
Time (2,1,8) =    0.0025214 ;
Time (2,1,16) =   0.00270134 ;
Time (2,1,32) =   0.00223824 ;
Time (2,1,64) =    0.0029579 ;
Time (2,1,128) =   0.00555123 ;
Time (2,1,160) =   0.00316232 ;
Time (3,1,1) =    0.0445187 ;
Time (3,1,2) =    0.0219684 ;
Time (3,1,4) =    0.0128282 ;
Time (3,1,8) =   0.00781821 ;
Time (3,1,16) =   0.00462432 ;
Time (3,1,32) =   0.00455968 ;
Time (3,1,64) =   0.00470291 ;
Time (3,1,128) =    0.0046768 ;
Time (3,1,160) =   0.00697879 ;
Time (4,1,1) =    0.0134912 ;
Time (4,1,2) =   0.00666913 ;
Time (4,1,4) =   0.00392742 ;
Time (4,1,8) =   0.00233671 ;
Time (4,1,16) =   0.00135781 ;
Time (4,1,32) =   0.00132038 ;
Time (4,1,64) =    0.0103251 ;
Time (4,1,128) =   0.00236017 ;
Time (4,1,160) =   0.00153313 ;
Time (5,1,1) =   0.00932815 ;
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
T_prep (1,1,1) =  9.82182e-05 ;
T_prep (1,1,2) =  9.69404e-05 ;
T_prep (1,1,4) =    9.639e-05 ;
T_prep (1,1,8) =  9.61134e-05 ;
T_prep (1,1,16) =  9.59551e-05 ;
T_prep (1,1,32) =  9.63304e-05 ;
T_prep (1,1,64) =  9.60939e-05 ;
T_prep (1,1,128) =   9.5984e-05 ;
T_prep (1,1,160) =  9.61935e-05 ;
T_prep (2,1,1) =  0.000100027 ;
T_prep (2,1,2) =  9.50759e-05 ;
T_prep (2,1,4) =  9.25669e-05 ;
T_prep (2,1,8) =  9.19681e-05 ;
T_prep (2,1,16) =  9.14112e-05 ;
T_prep (2,1,32) =   9.1373e-05 ;
T_prep (2,1,64) =  9.14317e-05 ;
T_prep (2,1,128) =  9.12966e-05 ;
T_prep (2,1,160) =  9.11308e-05 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000254632 ;
Time (1,1,2) =    0.0091398 ;
Time (1,1,4) =  0.000138425 ;
Time (1,1,8) =  0.000211489 ;
Time (1,1,16) =  0.000356439 ;
Time (1,1,32) =   0.00085505 ;
Time (1,1,64) =   0.00205155 ;
Time (1,1,128) =    0.0424482 ;
Time (1,1,160) =    0.0258949 ;
Time (2,1,1) =   0.00275145 ;
Time (2,1,2) =   0.00176973 ;
Time (2,1,4) =   0.00135954 ;
Time (2,1,8) =   0.00116748 ;
Time (2,1,16) =   0.00130925 ;
Time (2,1,32) =   0.00169373 ;
Time (2,1,64) =   0.00101461 ;
Time (2,1,128) =   0.00831778 ;
Time (2,1,160) =   0.00471793 ;
Time (3,1,1) =   0.00467641 ;
Time (3,1,2) =   0.00268594 ;
Time (3,1,4) =   0.00238491 ;
Time (3,1,8) =   0.00199596 ;
Time (3,1,16) =   0.00177157 ;
Time (3,1,32) =   0.00194796 ;
Time (3,1,64) =   0.00186768 ;
Time (3,1,128) =    0.0018298 ;
Time (3,1,160) =   0.00351568 ;
Time (4,1,1) =   0.00199309 ;
Time (4,1,2) =  0.000988812 ;
Time (4,1,4) =    0.0010024 ;
Time (4,1,8) =   0.00082964 ;
Time (4,1,16) =  0.000966364 ;
Time (4,1,32) =  0.000923011 ;
Time (4,1,64) =  0.000910864 ;
Time (4,1,128) =   0.00071692 ;
Time (4,1,160) =  0.000728177 ;
Time (5,1,1) =   0.00153091 ;
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
T_prep (1,1,1) =  0.000477539 ;
T_prep (1,1,2) =  0.000452701 ;
T_prep (1,1,4) =   0.00928367 ;
T_prep (1,1,8) =  0.000272412 ;
T_prep (1,1,16) =  0.000276008 ;
T_prep (1,1,32) =  0.000272755 ;
T_prep (1,1,64) =  0.000275407 ;
T_prep (1,1,128) =   0.00027151 ;
T_prep (1,1,160) =  0.000273135 ;
T_prep (2,1,1) =  0.000503027 ;
T_prep (2,1,2) =  0.000434381 ;
T_prep (2,1,4) =  0.000312365 ;
T_prep (2,1,8) =  0.000294285 ;
T_prep (2,1,16) =  0.000292072 ;
T_prep (2,1,32) =  0.000292344 ;
T_prep (2,1,64) =   0.00029134 ;
T_prep (2,1,128) =  0.000290911 ;
T_prep (2,1,160) =  0.000291031 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00409924 ;
Time (1,1,2) =   0.00228815 ;
Time (1,1,4) =   0.00137833 ;
Time (1,1,8) =   0.00108169 ;
Time (1,1,16) =   0.00164779 ;
Time (1,1,32) =   0.00299506 ;
Time (1,1,64) =   0.00703118 ;
Time (1,1,128) =    0.0300857 ;
Time (1,1,160) =    0.0414169 ;
Time (2,1,1) =    0.0990427 ;
Time (2,1,2) =    0.0488408 ;
Time (2,1,4) =    0.0232465 ;
Time (2,1,8) =    0.0226429 ;
Time (2,1,16) =    0.0215785 ;
Time (2,1,32) =    0.0192538 ;
Time (2,1,64) =    0.0179272 ;
Time (2,1,128) =    0.0152749 ;
Time (2,1,160) =    0.0075024 ;
Time (3,1,1) =     0.290469 ;
Time (3,1,2) =     0.151395 ;
Time (3,1,4) =    0.0807575 ;
Time (3,1,8) =    0.0715253 ;
Time (3,1,16) =    0.0594709 ;
Time (3,1,32) =    0.0659696 ;
Time (3,1,64) =    0.0604369 ;
Time (3,1,128) =    0.0617015 ;
Time (3,1,160) =    0.0872132 ;
Time (4,1,1) =    0.0381432 ;
Time (4,1,2) =    0.0174243 ;
Time (4,1,4) =   0.00835389 ;
Time (4,1,8) =    0.0071084 ;
Time (4,1,16) =   0.00784557 ;
Time (4,1,32) =   0.00766976 ;
Time (4,1,64) =   0.00721821 ;
Time (4,1,128) =   0.00659505 ;
Time (4,1,160) =    0.0128473 ;
Time (5,1,1) =    0.0324228 ;
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
T_prep (1,1,1) =  0.000863755 ;
T_prep (1,1,2) =  0.000863449 ;
T_prep (1,1,4) =   0.00975098 ;
T_prep (1,1,8) =  0.000675009 ;
T_prep (1,1,16) =  0.000665898 ;
T_prep (1,1,32) =  0.000665439 ;
T_prep (1,1,64) =  0.000666771 ;
T_prep (1,1,128) =  0.000668996 ;
T_prep (1,1,160) =  0.000669512 ;
T_prep (2,1,1) =  0.000952451 ;
T_prep (2,1,2) =  0.000840957 ;
T_prep (2,1,4) =  0.000688679 ;
T_prep (2,1,8) =  0.000625157 ;
T_prep (2,1,16) =  0.000626359 ;
T_prep (2,1,32) =  0.000625726 ;
T_prep (2,1,64) =  0.000622813 ;
T_prep (2,1,128) =  0.000627062 ;
T_prep (2,1,160) =  0.000625451 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00567584 ;
Time (1,1,2) =   0.00375137 ;
Time (1,1,4) =   0.00188346 ;
Time (1,1,8) =   0.00145294 ;
Time (1,1,16) =   0.00172687 ;
Time (1,1,32) =   0.00153269 ;
Time (1,1,64) =   0.00548506 ;
Time (1,1,128) =    0.0287391 ;
Time (1,1,160) =    0.0280628 ;
Time (2,1,1) =     0.106454 ;
Time (2,1,2) =     0.057145 ;
Time (2,1,4) =    0.0315938 ;
Time (2,1,8) =    0.0165101 ;
Time (2,1,16) =    0.0127668 ;
Time (2,1,32) =    0.0143374 ;
Time (2,1,64) =    0.0123853 ;
Time (2,1,128) =    0.0111665 ;
Time (2,1,160) =    0.0114268 ;
Time (3,1,1) =     0.284646 ;
Time (3,1,2) =     0.151441 ;
Time (3,1,4) =     0.086434 ;
Time (3,1,8) =    0.0457307 ;
Time (3,1,16) =    0.0410795 ;
Time (3,1,32) =    0.0502886 ;
Time (3,1,64) =    0.0382163 ;
Time (3,1,128) =    0.0385119 ;
Time (3,1,160) =    0.0852609 ;
Time (4,1,1) =    0.0464374 ;
Time (4,1,2) =     0.024075 ;
Time (4,1,4) =    0.0127294 ;
Time (4,1,8) =   0.00796367 ;
Time (4,1,16) =   0.00442442 ;
Time (4,1,32) =   0.00494526 ;
Time (4,1,64) =   0.00470455 ;
Time (4,1,128) =   0.00737762 ;
Time (4,1,160) =   0.00764178 ;
Time (5,1,1) =    0.0457543 ;
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
T_prep (1,1,1) =   0.00028193 ;
T_prep (1,1,2) =  0.000297496 ;
T_prep (1,1,4) =  0.000281459 ;
T_prep (1,1,8) =  0.000281462 ;
T_prep (1,1,16) =  0.000281407 ;
T_prep (1,1,32) =  0.000281453 ;
T_prep (1,1,64) =  0.000281356 ;
T_prep (1,1,128) =  0.000281426 ;
T_prep (1,1,160) =  0.000281285 ;
T_prep (2,1,1) =  0.000297293 ;
T_prep (2,1,2) =  0.000271593 ;
T_prep (2,1,4) =  0.000270561 ;
T_prep (2,1,8) =  0.000270208 ;
T_prep (2,1,16) =  0.000270461 ;
T_prep (2,1,32) =  0.000270491 ;
T_prep (2,1,64) =  0.000270331 ;
T_prep (2,1,128) =  0.000270416 ;
T_prep (2,1,160) =   0.00027026 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00220263 ;
Time (1,1,2) =    0.0102225 ;
Time (1,1,4) =   0.00118068 ;
Time (1,1,8) =   0.00177998 ;
Time (1,1,16) =   0.00206365 ;
Time (1,1,32) =   0.00238352 ;
Time (1,1,64) =   0.00965254 ;
Time (1,1,128) =    0.0249094 ;
Time (1,1,160) =    0.0284213 ;
Time (2,1,1) =    0.0482929 ;
Time (2,1,2) =     0.032786 ;
Time (2,1,4) =    0.0317393 ;
Time (2,1,8) =    0.0296665 ;
Time (2,1,16) =    0.0270235 ;
Time (2,1,32) =      0.02884 ;
Time (2,1,64) =    0.0264932 ;
Time (2,1,128) =    0.0352756 ;
Time (2,1,160) =    0.0429848 ;
Time (3,1,1) =     0.204865 ;
Time (3,1,2) =     0.106324 ;
Time (3,1,4) =     0.106386 ;
Time (3,1,8) =     0.093087 ;
Time (3,1,16) =     0.096197 ;
Time (3,1,32) =    0.0909891 ;
Time (3,1,64) =    0.0880598 ;
Time (3,1,128) =    0.0885353 ;
Time (3,1,160) =    0.0888497 ;
Time (4,1,1) =    0.0205337 ;
Time (4,1,2) =    0.0113009 ;
Time (4,1,4) =     0.010028 ;
Time (4,1,8) =    0.0110994 ;
Time (4,1,16) =    0.0102719 ;
Time (4,1,32) =    0.0101472 ;
Time (4,1,64) =   0.00907464 ;
Time (4,1,128) =    0.0166028 ;
Time (4,1,160) =    0.0138348 ;
Time (5,1,1) =    0.0162247 ;
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
T_prep (1,1,1) =   0.00123709 ;
T_prep (1,1,2) =   0.00119691 ;
T_prep (1,1,4) =   0.00984593 ;
T_prep (1,1,8) =  0.000625347 ;
T_prep (1,1,16) =  0.000600687 ;
T_prep (1,1,32) =  0.000602236 ;
T_prep (1,1,64) =  0.000592828 ;
T_prep (1,1,128) =  0.000600934 ;
T_prep (1,1,160) =  0.000594297 ;
T_prep (2,1,1) =   0.00127883 ;
T_prep (2,1,2) =   0.00117183 ;
T_prep (2,1,4) =   0.00079483 ;
T_prep (2,1,8) =  0.000662158 ;
T_prep (2,1,16) =  0.000644467 ;
T_prep (2,1,32) =  0.000642532 ;
T_prep (2,1,64) =  0.000643932 ;
T_prep (2,1,128) =  0.000646623 ;
T_prep (2,1,160) =  0.000643902 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00526279 ;
Time (1,1,2) =   0.00329484 ;
Time (1,1,4) =    0.0016198 ;
Time (1,1,8) =   0.00108642 ;
Time (1,1,16) =   0.00095525 ;
Time (1,1,32) =   0.00238251 ;
Time (1,1,64) =   0.00515616 ;
Time (1,1,128) =    0.0271972 ;
Time (1,1,160) =      0.04854 ;
Time (2,1,1) =    0.0611335 ;
Time (2,1,2) =    0.0158991 ;
Time (2,1,4) =    0.0117145 ;
Time (2,1,8) =   0.00661662 ;
Time (2,1,16) =    0.0118862 ;
Time (2,1,32) =    0.0116806 ;
Time (2,1,64) =   0.00440477 ;
Time (2,1,128) =   0.00561241 ;
Time (2,1,160) =   0.00445527 ;
Time (3,1,1) =     0.167803 ;
Time (3,1,2) =    0.0485841 ;
Time (3,1,4) =    0.0366572 ;
Time (3,1,8) =    0.0428078 ;
Time (3,1,16) =    0.0206344 ;
Time (3,1,32) =    0.0339566 ;
Time (3,1,64) =    0.0536373 ;
Time (3,1,128) =    0.0370094 ;
Time (3,1,160) =    0.0357821 ;
Time (4,1,1) =    0.0318383 ;
Time (4,1,2) =   0.00658995 ;
Time (4,1,4) =   0.00468408 ;
Time (4,1,8) =   0.00339232 ;
Time (4,1,16) =   0.00315771 ;
Time (4,1,32) =   0.00244777 ;
Time (4,1,64) =   0.00274445 ;
Time (4,1,128) =   0.00148881 ;
Time (4,1,160) =   0.00211696 ;
Time (5,1,1) =    0.0289041 ;
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
T_prep (1,1,1) =    0.0045619 ;
T_prep (1,1,2) =    0.0045661 ;
T_prep (1,1,4) =    0.0120126 ;
T_prep (1,1,8) =   0.00239414 ;
T_prep (1,1,16) =   0.00226343 ;
T_prep (1,1,32) =   0.00181485 ;
T_prep (1,1,64) =   0.00151644 ;
T_prep (1,1,128) =   0.00153535 ;
T_prep (1,1,160) =   0.00155128 ;
T_prep (2,1,1) =   0.00543913 ;
T_prep (2,1,2) =   0.00462818 ;
T_prep (2,1,4) =   0.00481987 ;
T_prep (2,1,8) =   0.00358009 ;
T_prep (2,1,16) =   0.00197792 ;
T_prep (2,1,32) =   0.00156496 ;
T_prep (2,1,64) =   0.00148032 ;
T_prep (2,1,128) =     0.001487 ;
T_prep (2,1,160) =   0.00150625 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0595212 ;
Time (1,1,2) =    0.0434699 ;
Time (1,1,4) =    0.0254249 ;
Time (1,1,8) =    0.0144005 ;
Time (1,1,16) =   0.00864994 ;
Time (1,1,32) =   0.00806137 ;
Time (1,1,64) =    0.0158899 ;
Time (1,1,128) =    0.0504152 ;
Time (1,1,160) =    0.0430212 ;
Time (2,1,1) =      1.40194 ;
Time (2,1,2) =      0.65135 ;
Time (2,1,4) =     0.286207 ;
Time (2,1,8) =     0.135119 ;
Time (2,1,16) =    0.0690475 ;
Time (2,1,32) =    0.0430818 ;
Time (2,1,64) =    0.0514066 ;
Time (2,1,128) =    0.0706108 ;
Time (2,1,160) =    0.0755078 ;
Time (3,1,1) =      1.33529 ;
Time (3,1,2) =     0.593995 ;
Time (3,1,4) =     0.287254 ;
Time (3,1,8) =     0.148162 ;
Time (3,1,16) =    0.0719728 ;
Time (3,1,32) =    0.0653916 ;
Time (3,1,64) =    0.0344031 ;
Time (3,1,128) =    0.0700913 ;
Time (3,1,160) =    0.0651249 ;
Time (4,1,1) =     0.484879 ;
Time (4,1,2) =     0.218848 ;
Time (4,1,4) =     0.108079 ;
Time (4,1,8) =     0.055675 ;
Time (4,1,16) =    0.0238907 ;
Time (4,1,32) =     0.019865 ;
Time (4,1,64) =    0.0169104 ;
Time (4,1,128) =    0.0148744 ;
Time (4,1,160) =    0.0313377 ;
Time (5,1,1) =     0.494808 ;
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
T_prep (1,1,1) =  0.000184031 ;
T_prep (1,1,2) =  0.000182885 ;
T_prep (1,1,4) =  0.000182666 ;
T_prep (1,1,8) =  0.000182434 ;
T_prep (1,1,16) =  0.000182588 ;
T_prep (1,1,32) =  0.000182384 ;
T_prep (1,1,64) =  0.000182654 ;
T_prep (1,1,128) =  0.000182411 ;
T_prep (1,1,160) =  0.000182063 ;
T_prep (2,1,1) =  0.000192672 ;
T_prep (2,1,2) =  0.000177859 ;
T_prep (2,1,4) =  0.000175307 ;
T_prep (2,1,8) =   0.00017436 ;
T_prep (2,1,16) =  0.000173952 ;
T_prep (2,1,32) =  0.000173851 ;
T_prep (2,1,64) =  0.000174152 ;
T_prep (2,1,128) =  0.000173982 ;
T_prep (2,1,160) =  0.000173683 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =  0.000931364 ;
Time (1,1,2) =    0.0096181 ;
Time (1,1,4) =  0.000477485 ;
Time (1,1,8) =   0.00041469 ;
Time (1,1,16) =  0.000847504 ;
Time (1,1,32) =    0.0011097 ;
Time (1,1,64) =   0.00254424 ;
Time (1,1,128) =    0.0273042 ;
Time (1,1,160) =    0.0270036 ;
Time (2,1,1) =    0.0225449 ;
Time (2,1,2) =    0.0106043 ;
Time (2,1,4) =   0.00532091 ;
Time (2,1,8) =   0.00471486 ;
Time (2,1,16) =   0.00472159 ;
Time (2,1,32) =   0.00467979 ;
Time (2,1,64) =   0.00352795 ;
Time (2,1,128) =   0.00766061 ;
Time (2,1,160) =   0.00540922 ;
Time (3,1,1) =    0.0283431 ;
Time (3,1,2) =    0.0176835 ;
Time (3,1,4) =    0.0112265 ;
Time (3,1,8) =    0.0109361 ;
Time (3,1,16) =   0.00964607 ;
Time (3,1,32) =    0.0109729 ;
Time (3,1,64) =    0.0101144 ;
Time (3,1,128) =    0.0102803 ;
Time (3,1,160) =    0.0121136 ;
Time (4,1,1) =   0.00912987 ;
Time (4,1,2) =   0.00460107 ;
Time (4,1,4) =   0.00228834 ;
Time (4,1,8) =    0.0027202 ;
Time (4,1,16) =   0.00203007 ;
Time (4,1,32) =   0.00160793 ;
Time (4,1,64) =   0.00158124 ;
Time (4,1,128) =   0.00170367 ;
Time (4,1,160) =   0.00156372 ;
Time (5,1,1) =    0.0112134 ;
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
T_prep (1,1,1) =   0.00105015 ;
T_prep (1,1,2) =    0.0010476 ;
T_prep (1,1,4) =   0.00975827 ;
T_prep (1,1,8) =  0.000697746 ;
T_prep (1,1,16) =  0.000695752 ;
T_prep (1,1,32) =  0.000695124 ;
T_prep (1,1,64) =  0.000693991 ;
T_prep (1,1,128) =  0.000697194 ;
T_prep (1,1,160) =  0.000699475 ;
T_prep (2,1,1) =   0.00114415 ;
T_prep (2,1,2) =   0.00104675 ;
T_prep (2,1,4) =   0.00077433 ;
T_prep (2,1,8) =  0.000707028 ;
T_prep (2,1,16) =  0.000705439 ;
T_prep (2,1,32) =  0.000703332 ;
T_prep (2,1,64) =  0.000703448 ;
T_prep (2,1,128) =  0.000702973 ;
T_prep (2,1,160) =  0.000704874 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =   0.00298379 ;
Time (1,1,2) =   0.00157102 ;
Time (1,1,4) =   0.00088307 ;
Time (1,1,8) =  0.000652933 ;
Time (1,1,16) =  0.000677209 ;
Time (1,1,32) =  0.000887504 ;
Time (1,1,64) =   0.00215749 ;
Time (1,1,128) =    0.0172231 ;
Time (1,1,160) =    0.0244925 ;
Time (2,1,1) =    0.0243411 ;
Time (2,1,2) =    0.0109989 ;
Time (2,1,4) =   0.00571572 ;
Time (2,1,8) =   0.00320895 ;
Time (2,1,16) =   0.00150203 ;
Time (2,1,32) =   0.00117493 ;
Time (2,1,64) =   0.00180202 ;
Time (2,1,128) =   0.00136881 ;
Time (2,1,160) =    0.0012381 ;
Time (3,1,1) =    0.0331518 ;
Time (3,1,2) =    0.0174933 ;
Time (3,1,4) =   0.00779538 ;
Time (3,1,8) =   0.00461499 ;
Time (3,1,16) =   0.00605642 ;
Time (3,1,32) =     0.009088 ;
Time (3,1,64) =   0.00148166 ;
Time (3,1,128) =   0.00141556 ;
Time (3,1,160) =    0.0023642 ;
Time (4,1,1) =    0.0186444 ;
Time (4,1,2) =    0.0132235 ;
Time (4,1,4) =   0.00428349 ;
Time (4,1,8) =   0.00221026 ;
Time (4,1,16) =   0.00185037 ;
Time (4,1,32) =   0.00106698 ;
Time (4,1,64) =   0.00103085 ;
Time (4,1,128) =  0.000826748 ;
Time (4,1,160) =  0.000881965 ;
Time (5,1,1) =    0.0162797 ;
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
T_prep (1,1,1) =    0.0162696 ;
T_prep (1,1,2) =    0.0162874 ;
T_prep (1,1,4) =    0.0187174 ;
T_prep (1,1,8) =   0.00797976 ;
T_prep (1,1,16) =   0.00556715 ;
T_prep (1,1,32) =   0.00528788 ;
T_prep (1,1,64) =   0.00471785 ;
T_prep (1,1,128) =   0.00417295 ;
T_prep (1,1,160) =   0.00415698 ;
T_prep (2,1,1) =    0.0367283 ;
T_prep (2,1,2) =    0.0367483 ;
T_prep (2,1,4) =    0.0211096 ;
T_prep (2,1,8) =    0.0139737 ;
T_prep (2,1,16) =   0.00819072 ;
T_prep (2,1,32) =   0.00507755 ;
T_prep (2,1,64) =   0.00422121 ;
T_prep (2,1,128) =   0.00418509 ;
T_prep (2,1,160) =   0.00439598 ;
Tprep {id} = T_prep ;
Time = nan (2, 1, 160) ; 
Time (1,1,1) =    0.0655255 ;
Time (1,1,2) =    0.0362846 ;
Time (1,1,4) =     0.014358 ;
Time (1,1,8) =   0.00852266 ;
Time (1,1,16) =   0.00421718 ;
Time (1,1,32) =   0.00251556 ;
Time (1,1,64) =   0.00471895 ;
Time (1,1,128) =    0.0278034 ;
Time (1,1,160) =    0.0436332 ;
Time (2,1,1) =     0.188735 ;
Time (2,1,2) =    0.0909381 ;
Time (2,1,4) =    0.0366032 ;
Time (2,1,8) =    0.0226138 ;
Time (2,1,16) =    0.0100851 ;
Time (2,1,32) =   0.00560976 ;
Time (2,1,64) =   0.00320991 ;
Time (2,1,128) =   0.00891437 ;
Time (2,1,160) =   0.00437485 ;
Time (3,1,1) =     0.132727 ;
Time (3,1,2) =    0.0698302 ;
Time (3,1,4) =    0.0304489 ;
Time (3,1,8) =    0.0157446 ;
Time (3,1,16) =   0.00829323 ;
Time (3,1,32) =   0.00463674 ;
Time (3,1,64) =   0.00930153 ;
Time (3,1,128) =   0.00189781 ;
Time (3,1,160) =    0.0179518 ;
Time (4,1,1) =     0.170086 ;
Time (4,1,2) =    0.0855865 ;
Time (4,1,4) =     0.041516 ;
Time (4,1,8) =    0.0222419 ;
Time (4,1,16) =    0.0102048 ;
Time (4,1,32) =   0.00866638 ;
Time (4,1,64) =   0.00464176 ;
Time (4,1,128) =    0.0109919 ;
Time (4,1,160) =    0.0163081 ;
Time (5,1,1) =     0.154037 ;
T {id} = Time ;

File {id} = filetrim (file) ;

file = ' /users/davis/GraphChallenge/snap/friendster/friendster_adj.tsv.gz ' ;
