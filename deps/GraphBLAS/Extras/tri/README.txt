tri:  triangle counting, various methods (excluding GraphBLAS).
For the GraphBLAS version see GraphBLAS/Demo/Source/tricount.c
For the MATLAB version, see GraphBLAS/Demo/MATLAB/tricount.m

Tim Davis, June 23, 2018

To compile and run a very small matrix, first edit the Makefile and change
the compiler.  I'm using "gcc-6" on the Mac, which is gcc 6.2.0_1 "Homebrew",
since clang does not support OpenMP.  On the IBM Power, I'm using xlc 18.

Then do:

    make

Next, you may wish to edit MAXTHREADS in tri_main.c, which controls how many
threads are used.  See the comments in that file for the description of the
systems I use and the # of threads I test with.

To run larger matrices, one from GraphChallenge.org and one from
the SuiteSparse collection:

    ./go1 > go1_out.txt

The output of go1 is in go1_out.txt.

To run the whole GraphChallenge

    ./go > go_out.txt

A summary of the results is printed stderr as the computation proceeds.

The GraphChallenge/ssget matrices are triplet files that I created from the
SuiteSparse Matrix Collection, converting them into the GraphChallenge triplet
format.  They appear in ~/GraphChallenge and are not part of this distribution.

GraphChallenge/snap and GraphChallenge/synthetic are all matrices from
GraphChallenge.org.  I'm using the *_adj.tsv format, but I have gzip'd them to
save space.  So the scripts gunzip them and pipe to stdin of the test program.

Files:

    Makefile
    make_output.txt         output of 'make'
    README.txt              this file

    go                      run all the matrices
    go1                     run 2 matrices (ok, the name is bad..)
    go1_out.txt             output of go1 on cholesky.cse.tamu.edu
    gocage                  run cage15, a large matrix
    gocage_out.txt          output of gocage on cholesky.cse.tamu.edu
    a.awk                   use to summary output files from tri_main:
                            awk -f a.awk < output.txt

    tri_def.h               include file
    tri_dot_template.c      template for two dot product versions
    tri_functions.c         all methods created here
    tri_main.c              main test program
    tri_prep.c              prepare L, U, or permuted L and U
    tri_read.c              read a matrix
    tri_run                 shell script to run a set of matrices
    tri_simple.c            very simple method, sequential, no frills
    tri_template.c          template for 8 outer-product versions

    bcsstk01                test matrix

    ~/GraphChallenge        a symbolic link to the GraphChallenge matrices

    fbest.m                 print an entry in a latex table
    filetrim.m              trim a filename
    go4                     run 4 problems
    gosub                   run subset for HPEC18 paper
    gosub_cholesky.out      output of gosub
    gosub_chol_friendster_out.txt

    tfr.m                   process results
    tres.m                  process results

    tri_graphblas.awk       process GraphBLAS output
    tri_grb_output.txt      output of tricount in GraphBLAS
    tri_grb_results.m       summary results from GraphBLAS
    tri_results.m           summary results from tri_main.c

    GraphChallenge/snap:

        amazon0302
        amazon0312
        amazon0505
        amazon0601
        as-caida20071105
        as20000102
        ca-AstroPh
        ca-CondMat
        ca-GrQc
        ca-HepPh
        ca-HepTh
        cit-HepPh
        cit-HepTh
        cit-Patents
        email-Enron
        email-EuAll
        facebook_combined
        flickrEdges
        friendster
        loc-brightkite_edges
        loc-gowalla_edges
        oregon1_010331
        oregon1_010407
        oregon1_010414
        oregon1_010421
        oregon1_010428
        oregon1_010505
        oregon1_010512
        oregon1_010519
        oregon1_010526
        oregon2_010331
        oregon2_010407
        oregon2_010414
        oregon2_010421
        oregon2_010428
        oregon2_010505
        oregon2_010512
        oregon2_010519
        oregon2_010526
        p2p-Gnutella04
        p2p-Gnutella05
        p2p-Gnutella06
        p2p-Gnutella08
        p2p-Gnutella09
        p2p-Gnutella24
        p2p-Gnutella25
        p2p-Gnutella30
        p2p-Gnutella31
        roadNet-CA
        roadNet-PA
        roadNet-TX
        soc-Epinions1
        soc-Slashdot0811
        soc-Slashdot0902

    GraphChallenge/ssget:

        DIMACS10
        Freescale
        Gleich
        Mallya
        SNAP
        vanHeukelum

    GraphChallenge/synthetic:

        graph500-scale18-ef16
        graph500-scale19-ef16
        graph500-scale20-ef16
        graph500-scale21-ef16
        graph500-scale22-ef16
        graph500-scale23-ef16
        graph500-scale24-ef16
        graph500-scale25-ef16
        image-grid

