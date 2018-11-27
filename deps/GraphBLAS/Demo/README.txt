SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

This is the GraphBLAS/Demo folder.  It contains a set of simple demo programs
that illustrate the use of GraphBLAS.  To compile and run the demos, see
../README.txt.

--------------------------------------------------------------------------------
Files in this folder:

README.txt              this file
demo                    run all demos
tri_run                 run tricount on a set of matrices
go                      run tricount on GraphChallenge matrices
go2                     run tricount on two large matrices
go3                     run tricount on a few large matrices

--------------------------------------------------------------------------------
in Demo/Source:
--------------------------------------------------------------------------------

bfs5m.c                 breadth-first search using assign-scalar
bfs5m_check.c           as above, but with error checking
bfs6.c                  breadth-first serach using apply
bfs6_check.c            as above, but with error checking
bfs_level.c             assign level for bfs6
get_matrix.c            get a matrix (file, Wathen, or random)
mis.c                   maximal independent set
mis_check.c             as above, but with error checking
mis_score.c             random score for mis
random_matrix.c         create a random matrix
read_matrix.c           read a matrix from a file (Matrix/*)
simple_rand.c           a very simple random number generator
simple_timer.c          a simple yet portable timer
tricount.c              six triangle counting methods using GraphBLAS
usercomplex.c           user-defined double complex type
wathen.c                GraphBLAS version of the MATLAB wathen.m
dpagerank.c             PageRank via real semiring
dpagerank2.c            PageRank via pagegrank semiring
drowscale.c             row scaling for dpagerank
ipagerank.c             PageRank via integer semiring
irowscale.c             row scaling for ipagerank

--------------------------------------------------------------------------------
in Demo/Program:
--------------------------------------------------------------------------------

bfs_demo.c              demo program to test bfs
complex_demo.c          demo program to test complex type
kron_demo.c             demo program to test GxB_kron
mis_demo.c              demo program to test mis
tri_demo.c              demo program to test tricount
simple_demo.c           demo program to test simple_rand and simple_timer
wildtype_demo.c         demo program with arbitrary struct as user-defined type
pagerank_demo.c         demo program to test dpagerank and ipagerank
openmp_demo.c           demo program using OpenMP
pthread_demo.c          demo program using POSIX pthreads

--------------------------------------------------------------------------------
in Demo/Output:
--------------------------------------------------------------------------------

bfs_demo.out            output of bfs_demo
complex_demo_out.m      output of complex_demo, run in MATLAB to check results
mis_demo.out            output of mis_demo
simple_test.out         output of simple_demo
tri_demo.out            output of tri_demo
wildtype_demo.out       output of wildtype_demo
pagerank_demo.out       output of pagerank_demo

6 output files from openmp_demo.c:
user_openmp_gb_openmp.out    user threads: OpenMP, GraphBLAS: OpenMP
user_openmp_gb_pthread.out   user threads: OpenMP, GraphBLAS: POSIX 
user_openmp_gb_none.out      user threads: OpenMP, GraphBLAS: none, expect fail
user_none_gb_openmp.out      user threads: none, GraphBLAS: OpenMP
user_none_gb_pthread.out     user threads: none, GraphBLAS: POSIX 
user_none_gb_none.out        user threads: none, GraphBLAS: none (OK)

3 output files from pthread_demo.c:
user_pthread_gb_openmp.out   user threads: POSIX, GraphBLAS: OpenMP
user_pthread_gb_pthread.out  user threads: POSIX, GraphBLAS: POSIX 
user_pthread_gb_none.out     user threads: POSIX, GraphBLAS: none, expect fail

Note that two of the above files show expected failures: synchronization
failures (queue invalid) and mangled error messages, because the user
application is multithreaded but GraphBLAS was compiled with no threading
library and is thus not thread-safe.

Output/V220:            output files from Nov 22, 2018 (Version 2.2.0)
    go_out_cholesky.txt

Output/V210             output files from Sept 11, 2018 (Version 2.1.0)
    go2_out_laptop.txt
    go3_out_laptop.txt
    go_out_cholesky.txt
    go_out_laptop.txt

    bfs_demo.out
    mis_demo.out
    pagerank_demo.out
    simple_demo.out
    tri_demo.out
    wildtype_demo.out

Output/V112:            output files from Dec 23, 2017 (Version 1.1.2
                        of SuiteSparse:GraphBLAS); same as filenames above
    README.txt
    go_out_cholesky.txt
    go_out_laptop.txt
    go3_out_laptop.txt

--------------------------------------------------------------------------------
in Demo/Include:
--------------------------------------------------------------------------------

demos.h                 include file for all demos
simple_rand.h           include file for simple_rand.c
simple_timer.h          include file for simple_timer
usercomplex.h           include file for usercomplex.h

--------------------------------------------------------------------------------
in Demo/MATLAB:
--------------------------------------------------------------------------------

Contents.m              contents of Demo/MATLAB
tricount.m              five triangle counting methods using MATLAB
adj_to_edges.m          convert adjacency matrix to incidence matrix
edges_to_adj.m          convert incidence matrix to adjacency matrix
check_adj.m             check an adjaceny matrix
tri_matlab.m            run a set of GraphChallenge matrices
tri_matlab_out.txt      output of tri_matlab.m
kron_demo.m             kron demo
kron_test.m             kron test
dpagerank.m             pagerank with real values
ipagerank.m             pagerank with integers
rowscale.m              rowscaling for dpagerank.m and ipagerank.m

--------------------------------------------------------------------------------
in Demo/Matrix:
--------------------------------------------------------------------------------

folder with test matrices, with 0-based indices. Many of these are derived from
the Harwell/Boeing matrix collection.  Contains:

    2blocks
    ash219
    bcsstk01
    bcsstk16
    eye3
    fs_183_1
    huge
    ibm32a
    ibm32b
    lp_afiro
    mbeacxc
    t1
    t2
    west0067

