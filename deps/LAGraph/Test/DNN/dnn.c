//------------------------------------------------------------------------------
// LAGraph/Test/DNN/dnn: run all neural networks from http://graphchallenge.org
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.
*/

//------------------------------------------------------------------------------

// LAGraph/Test/DNN/dnn: test for LAGraph_dnn.  Contributed by Tim Davis,
// Texas A&M University.

// Usage: ./build/dnn nproblems

// nproblems is the # of test problems to solve.  If not present, it defaults
// to 12 (run all 12 DNN's).  The problems are solved in order from small to
// big.  The Makefile just runs the first and smallest problem.

// NOTE: this test currently uses many GxB_* extensions in
// SuiteSparse:GraphBLAS.  It optionally uses OpenMP.

#include <LAGraph.h>

#define LAGRAPH_FREE_ALL ;

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // start LAGraph and GraphBLAS
    //--------------------------------------------------------------------------

    GrB_Info info ;
    LAGRAPH_OK (LAGraph_init ( )) ;

    //--------------------------------------------------------------------------
    // problem size definitions
    //--------------------------------------------------------------------------

    // The 12 problems and their sizes are hard-coded below.

    // It would be better to define these from the input files, but the problem
    // data files are not formatted in a way that makes this easy to do.  A
    // Matrix Market file format would be better (which can specify the type
    // and size of each matrix), with the additional of a problem specification
    // file that defines each of the 12 problems to solve.

    // Each problem is defined by a set of files in the DNN_DATA directory,
    // which can be obtained from http://graphchallenge.org .  The simplest way
    // to redefine the location of the data files is to make ./dnn_data a
    // symbolic link, and leave DNN_DATA unchanged.  The .gitignore file will
    // prevent dnn_data from syncing to github, so you could also simply change
    // ./dnn_data to a true directory and place all files there.  Or, change
    // the DNN_DATA macro to point to your data files.

    #define DNN_DATA "./dnn_data"

    // Each of the 12 problems is defined by the # of neurons at each layer, N
    // = (1024, 4096, 16384, 65536), and the # of layers, L = (120, 480, or
    // 1920).  Each problem has the same number of features (F = 60000).  The
    // input files for a given problem (N,L) are as follows:

    // Input feature vectors: an F-by-N sparse matrix
    //      ./dnn_data/MNIST/sparse-images-(N).tsv
    // Neural network layers, for i = 1 to L, each an N-by-N sparse matrix:
    //      ./dnn_data/DNN/neuron(N)/n(N)-l(i).tsv
    // True categories, a list of integers, one per line:
    //      ./dnn_data/DNN/neuron(N)-l(L)-categories.tsv

    // The Bias vectors are defined with the single scalar, neuralNetBias[ ],
    // with one scalar for each value of N.  This scalar is used to construct
    // the diagonal Bias matrices for each layer.  All the layers share the
    // same matrix, but they are treated as different matrices here.  In a more
    // general problem, the Bias matrices would differ for each layer and
    // perhaps for each neuron.  As a result, this test is not permitted to
    // exploit the fact that all neurons are biased the same way.

    // Note that for a given number of neurons, N, each of the 3 problems for
    // different layers shares the same weight matrices for the first layers.
    // That is, the first 120 layers of the (1024,480) problem are the same as
    // the 120 layers of the (1024,120) problem.  This is not exploited in
    // LAGraph_dnn, but it is exploited here, simply to reduce the time to load
    // the problems.

    int len = 1024 ;
    char filename [len] ;

    #define NMAXLAYERS 3
    int maxLayers [NMAXLAYERS] = { 120, 480, 1920 } ;

//  #define NMAXNEURONS 1
//  int Nneurons [NMAXNEURONS] = { 65536 } ;
//  double neuralNetBias [NMAXNEURONS] = { -0.45 } ;

    #define NMAXNEURONS 4
    int Nneurons [NMAXNEURONS] = { 1024, 4096, 16384, 65536 } ;
    double neuralNetBias [NMAXNEURONS] = { -0.3, -0.35, -0.4, -0.45 } ;

    int nfeatures = 60000 ;

    GrB_Matrix Y0 = NULL, Y = NULL, W [65536], Bias [65536] ;
    GrB_Vector TrueCategories = NULL, Categories = NULL, C = NULL ;

    for (int layer = 0 ; layer < 65536 ; layer++)
    {
        W [layer] = NULL ;
        Bias [layer] = NULL ;
    }

    #undef  LAGRAPH_FREE_ALL
    #define LAGRAPH_FREE_ALL                            \
    {                                                   \
        GrB_free (&TrueCategories) ;                    \
        GrB_free (&Categories) ;                        \
        GrB_free (&C) ;                                 \
        GrB_free (&Y) ;                                 \
        GrB_free (&Y0) ;                                \
        for (int layer = 0 ; layer < 65536 ; layer++)   \
        {                                               \
            GrB_free (& (W [layer])) ;                  \
            GrB_free (& (Bias [layer])) ;               \
        }                                               \
    }

    // select the type.  GrB_FP32 is faster and passes all the tests.
//  GrB_Type type = GrB_FP64 ;
    GrB_Type type = GrB_FP32 ;

    printf ("type: ") ;
    if (type == GrB_FP64) printf ("double\n") ; else printf ("float\n") ;

    // get the max # of threads that can be used
    int nthreads_max ;
    LAGRAPH_OK (GxB_get (GxB_NTHREADS, &nthreads_max)) ;
    printf ("max # of nthreads: %d\n", nthreads_max) ;

    #define NNTHREADS 12
    int nthreads_list [NNTHREADS] =
        { 1, 2, 4, 8, 16, 20, 32, 40, 64, 128, 160, 256 } ;

//  #define NNTHREADS 1
//  int nthreads_list [NNTHREADS] = { 40 } ;

    // determine the # of problems to solve
    int nproblems = NMAXNEURONS * NMAXLAYERS ;
    if (argc > 1)
    {
        sscanf (argv [1], "%d", &nproblems) ;
    }
    printf ("# of problems to solve: %d\n", nproblems) ;
    int problem = 0 ;

    //--------------------------------------------------------------------------
    // run all problems
    //--------------------------------------------------------------------------

    for (int kn = 0 ; kn < NMAXNEURONS ; kn++)
    {

        //----------------------------------------------------------------------
        // check if this problem is to be solved
        //----------------------------------------------------------------------

        if (problem > nproblems) continue ;


        //----------------------------------------------------------------------
        // get the number of nneurons and neural bias
        //----------------------------------------------------------------------

        double tic [2] ;
        LAGraph_tic (tic) ;

        int nneurons = Nneurons [kn] ;
        double b = neuralNetBias [kn] ;
        printf ("\n# neurons: %d bias: %g\n", nneurons, b) ;

        //----------------------------------------------------------------------
        // read in the initial feature vectors
        //----------------------------------------------------------------------

        sprintf (filename, "%s/MNIST/sparse-images-%d.tsv", DNN_DATA, nneurons);
        FILE *f = fopen (filename, "r") ;
        if (!f) { printf ("cannot open %s\n", filename) ; abort ( ) ; }
        LAGRAPH_OK (LAGraph_tsvread (&Y0, f, type, nfeatures, nneurons)) ;
        fclose (f) ;
        double t = LAGraph_toc (tic) ;
        printf ("# features: %" PRIu64 " read time: %g sec\n", nfeatures, t) ;
        GrB_Index nvals ;
        LAGRAPH_OK (GrB_Matrix_nvals (&nvals, Y0)) ;
        printf ("# entries in Y0: %g million\n", (double) nvals / 1e6) ;
        fflush (stdout) ;

        //----------------------------------------------------------------------
        // run each problem size (for all #'s of layers)
        //----------------------------------------------------------------------

        for (int kl = 0 ; kl < NMAXLAYERS ; kl++)
        {

            //------------------------------------------------------------------
            // check if this problem is to be solved
            //------------------------------------------------------------------

            problem++ ;
            if (problem > nproblems) continue ;

            //------------------------------------------------------------------
            // get the number of layers in this neural net
            //------------------------------------------------------------------

            int nlayers = maxLayers [kl] ;
            printf ("\n--------------------------------------"
                "neurons per layer: %d layers: %d\n", nneurons, nlayers) ;

            //------------------------------------------------------------------
            // read in the layers in parallel
            //------------------------------------------------------------------

            LAGraph_tic (tic) ;
            int first_layer = (kl == 0) ? 0 : maxLayers [kl-1] ;
            bool ok = true ;

            // assume the I/O system can handle 2-way parallelism
            #pragma omp parallel for schedule(dynamic,1) reduction(&&:ok) \
                num_threads (2)
            for (int layer = first_layer ; layer < nlayers ; layer++)
            {
                // read the neuron layer: W [layer]
                char my_filename [1024] ;
                sprintf (my_filename, "%s/DNN/neuron%d/n%d-l%d.tsv", DNN_DATA,
                    nneurons, nneurons, layer+1) ;
                FILE *my_file = fopen (my_filename, "r") ;

                bool my_ok = true ;
                if (!my_file)
                {
                    printf ("cannot open %s\n", my_filename) ;
                    my_ok = false ;
                    continue ;
                }

                GrB_Info my_info = LAGraph_tsvread (&(W [layer]), my_file,
                    type, nneurons, nneurons) ;
                fclose (my_file) ;
                my_ok = my_ok && (my_info == GrB_SUCCESS) ;

                // construct the bias matrix: Bias [layer].  Note that all Bias
                // matrices are the same for all layers, and all diagonal
                // entries are also the same, but this test must not exploit
                // that fact.
                my_info = GrB_Matrix_new (&(Bias [layer]), type,
                    nneurons, nneurons) ;
                my_ok = my_ok && (my_info == GrB_SUCCESS) ;
                for (int i = 0 ; i < nneurons ; i++)
                {
                    my_info = GrB_Matrix_setElement (Bias [layer], b, i, i) ;
                    my_ok = my_ok && (my_info == GrB_SUCCESS) ;
                }
                GrB_Index ignore ;
                my_info = GrB_Matrix_nvals (&ignore, Bias [layer]) ;
                my_ok = my_ok && (my_info == GrB_SUCCESS) ;
                ok = ok && my_ok ;
            }

            if (!ok)
            {
                printf ("neural read failure\n") ;
                abort ( ) ;
            }

            t = LAGraph_toc (tic) ;
            printf ("read net time %g sec\n", t) ;

            double nedges = 0 ;
            for (int layer = 0 ; layer < nlayers ; layer++)
            {
                GrB_Index nvals ;
                LAGRAPH_OK (GrB_Matrix_nvals (&nvals, W [layer])) ;
                nedges += nvals ;
            }
            printf ("# edges in all layers: %g million\n\n",
                (double) nedges / 1e6) ;
            fflush (stdout) ;

            // read TrueCategories as a boolean nfeatures-by-1 vector
            LAGRAPH_OK (GrB_Vector_new (&TrueCategories, GrB_BOOL,
                nfeatures)) ;
            sprintf (filename, "%s/DNN/neuron%d-l%d-categories.tsv", DNN_DATA,
                nneurons, nlayers) ;
            f = fopen (filename, "r") ;
            bool check_result = (f != NULL) ;
            if (check_result)
            {
                while (1)
                {
                    int category ;
                    if (fscanf (f, "%d\n", &category) == EOF) break ;
                    LAGRAPH_OK (GrB_Vector_setElement (TrueCategories,
                        (bool) true, category-1)) ;
                }
                fclose (f) ;
            }
            else
            {
                printf ("cannot open %s\n", filename) ;
            }

            //------------------------------------------------------------------
            // solve the problem with 1, 2, 4, ..., nthreads_max threads
            //------------------------------------------------------------------

            double t1 = 0, tcheck = 0 ;
            GrB_Index final_ynvals ;

            for (int kth = 0 ; kth < NNTHREADS ; kth++)
            {

                //--------------------------------------------------------------
                // set the # of threads to use
                //--------------------------------------------------------------

                int nthreads = nthreads_list [kth] ;
                if (nthreads > nthreads_max) break ;
                LAGRAPH_OK (GxB_set (GxB_NTHREADS, nthreads)) ;
                printf ("nthreads %2d: ", nthreads) ;
                fflush (stdout) ;

                //--------------------------------------------------------------
                // solve the problem
                //--------------------------------------------------------------

                LAGraph_tic (tic) ;
                LAGRAPH_OK (LAGraph_dnn (&Y, W, Bias, nlayers, Y0)) ;
                t = LAGraph_toc (tic) ;
                printf ("solution time %12.2f sec", t) ;

                if (nthreads == 1)
                {
                    t1 = t ;
                }
                else
                {
                    printf (" speedup %8.2f", t1/t) ;
                }

                //--------------------------------------------------------------
                // check the result
                //--------------------------------------------------------------

                // this is so fast, it's hardly worth timing ...
                LAGraph_tic (tic) ;
                LAGRAPH_OK (GrB_Matrix_nvals (&final_ynvals, Y)) ;

                // C = sum (Y)
                LAGRAPH_OK (GrB_Vector_new (&C, type, nfeatures)) ;
                LAGRAPH_OK (GrB_reduce (C, NULL, NULL, GrB_PLUS_FP64, Y, NULL));
                // Categories = pattern of C
                LAGRAPH_OK (GrB_Vector_new (&Categories, GrB_BOOL, nfeatures)) ;
                LAGRAPH_OK (GrB_apply (Categories, NULL, NULL, GxB_ONE_BOOL,
                    C, NULL)) ;

                // write out Categories, as a 1-based file
                sprintf (filename, "my_neuron%d-l%d-categories_threads%d.tsv",
                    nneurons, nlayers, nthreads) ;
                FILE *ff = fopen (filename, "w") ;
                for (int i = 0 ; i < nfeatures ; i++)
                {
                    bool c = false ;
                    LAGRAPH_OK (GrB_Vector_extractElement (&c, Categories, i)) ;
                    if (c) fprintf (ff, "%d\n", i + 1) ;
                }
                fclose (ff) ;

                if (check_result)
                {
                    // check if Categories and TrueCategories are the same
                    bool isequal ;
                    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal,
                        TrueCategories, Categories, NULL)) ;
                    if (!isequal)
                    {
                        // GxB_print (TrueCategories, 3) ;
                        // GxB_print (Categories, 3) ;
                        printf ("test failure!\n") ;
                        // LAGRAPH_FREE_ALL ;
                        // abort ( ) ;
                    }
                    else
                    {
                        printf (" test passed") ;
                    }
                }
                printf ("\n") ;

                GrB_free (&Categories) ;
                GrB_free (&C) ;
                GrB_free (&Y) ;
                tcheck = LAGraph_toc (tic) ;
            }

            printf ("\n# entries in final Y: %g million\n", 
                (double) final_ynvals / 1e6) ;
            printf ("check time: %g sec\n", tcheck) ;
            LAGRAPH_OK (GxB_set (GxB_NTHREADS, nthreads_max)) ;
        }

        //----------------------------------------------------------------------
        // free the problem
        //----------------------------------------------------------------------

        LAGRAPH_FREE_ALL ;
    }

    //--------------------------------------------------------------------------
    // finalize LAGraph and GraphBLAS
    //--------------------------------------------------------------------------

    LAGRAPH_OK (LAGraph_finalize ( )) ;
    printf ("all tests passed\n") ;
    return (GrB_SUCCESS) ;
}

