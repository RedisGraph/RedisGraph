//------------------------------------------------------------------------------
// GB_AxB_dot3_cuda: compute C<M> = A'*B in parallel, on the GPU(s)
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This function only computes C<M>=A'*B on the GPUs.  The mask must be
// present, and not complemented.  The mask is always applied.

extern "C" 
{
  #include "GB_mxm.h"
}
#include "GB_cuda.h"


#include "templates/GB_jit_AxB_dot3_phase1.cu.jit"
#include "templates/GB_jit_AxB_dot3_phase2.cu.jit"
// the 5 kernels for the 5 buckets:
#include "templates/GB_jit_AxB_dot3_phase3_dndn.cu.jit"
#include "templates/GB_jit_AxB_dot3_phase3_vsvs.cu.jit"
#include "templates/GB_jit_AxB_dot3_phase3_vssp.cu.jit"
#include "templates/GB_jit_AxB_dot3_phase3_spdn.cu.jit"
#include "templates/GB_jit_AxB_dot3_phase3_mp.cu.jit"
#include "templates/GB_jit_AxB_dot3_phase3_warpix.cu.jit"
#include "templates/reduceNonZombiesWarp.cu.jit"

#include "GB_jit_launcher.h"


const std::vector<std::string> header_names ={};


#define GB_FREE_WORK                                                    \
{                                                                       \
    GB_cuda_free (Nanobuckets) ;    Nanobuckets = NULL ;                    \
    GB_cuda_free (Blockbucket) ;    Blockbucket = NULL ;                    \
    GB_cuda_free (Bucket);          Bucket      = NULL;                     \
    GB_cuda_free (Bucketp);         Bucketp     = NULL;                     \
    GB_cuda_free (offset);          offset      = NULL;                     \
}

#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_FREE_WORK ;                                                      \
    GrB_Matrix_free (Chandle) ;                                         \
}

GrB_Info GB_AxB_dot3_cuda           // C<M> = A'*B using dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;

    ASSERT_MATRIX_OK (M, "M for dot3 cuda A'*B", GB0) ;
    ASSERT_MATRIX_OK (A, "A for dot3 cuda A'*B", GB0) ;
    ASSERT_MATRIX_OK (B, "B for dot3 cuda A'*B", GB0) ;

    ASSERT (!GB_PENDING (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!GB_ZOMBIES (M)) ;

    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    ASSERT (!GB_PENDING (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for dot3 numeric A'*B", GB0) ;

    ASSERT (A->vlen == B->vlen) ;
    GBURBLE ("(GPU dot3) ") ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    int ntasks = 0, number_of_sms = 0 ;
    int64_t *Nanobuckets = NULL, *Blockbucket = NULL ;
    int64_t *Bucket = NULL;
    int64_t *Bucketp = NULL;
    int64_t *offset = NULL;
    (*Chandle) = NULL ;

    // just in case M is jumbled and we don't handle it yet (TODO)
    GB_MATRIX_WAIT (M) ;
    ASSERT (!GB_JUMBLED (M)) ;

    int device = -1;

    cudaSetDevice( 0 ) ;

    cudaGetDevice(&device);

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mh = M->h ;
    // const int64_t *restrict Mi = M->i ;
    // const GB_void *restrict Mx = M->x ;
    // const size_t msize = M->type->size ;
    const int64_t mvlen = M->vlen ;
    const int64_t mvdim = M->vdim ;
    const int64_t mnz = GB_NNZ (M) ;
    const int64_t mnvec = M->nvec ;
    const bool M_is_hyper = GB_IS_HYPERSPARSE( M ) ;

    const int64_t anz = GB_NNZ (A) ;
    const int64_t anvec = A->nvec ;

    const int64_t bnz = GB_NNZ (B) ;
    const int64_t bnvec = B->nvec ;

    //--------------------------------------------------------------------------
    // allocate C, the same size and # of entries as M
    //--------------------------------------------------------------------------

    // FUTURE: ctype need not be the op->ztype
    GrB_Type ctype = semiring->add->op->ztype ;
    int64_t cvlen = mvlen ;
    int64_t cvdim = mvdim ;
    int64_t cnz = mnz ;
    int64_t cnvec = mnvec ;

    // TODO tell GB_CREATE where to put the data: CPU or GPU (via
    // cudaMemAdvise), but this works as-is.
    int sparsity = (M_is_hyper) ? GxB_HYPERSPARSE : GxB_SPARSE ;
    info = GB_new_bix (Chandle, // sparse or hyper (from M), new header
        ctype, cvlen, cvdim, GB_Ap_malloc, true,
        sparsity, false, M->hyper_switch, cnvec,
        cnz+1,  // add one to cnz for GB_cumsum of Cwork 
        true, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (info) ;
    }

    GrB_Matrix C = (*Chandle) ;
    //int64_t *Citemp =  C->i ;        
    //auto *Cxtemp = C->x ;        
    //cudaMalloc ((void**) &(C->i), cnz * sizeof( int64_t) ); 
    //cudaMalloc ((void**) &(C->x), cnz * C->type->size ); 
    cudaMemAdvise( C->i, cnz * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, device); 
    cudaMemAdvise( C->x, cnz * C->type->size , cudaMemAdviseSetPreferredLocation, device); 

    int64_t *restrict Cp = M->p ;
    int64_t *restrict Ch = M->h ;
    // int64_t *restrict Ci = C->i ;
    // use C->i as workspace

    //--------------------------------------------------------------------------
    // copy Mp and Mh into C
    //--------------------------------------------------------------------------

    //cudaMemcpy (Cp, Mp, (cnvec+1) * sizeof (int64_t), cudaMemcpyDefault) ;
    if (M_is_hyper)
    { 
        //cudaMemcpy (Ch, Mh, cnvec * sizeof (int64_t), cudaMemcpyDefault) ;
    }
    C->magic = GB_MAGIC ;
    C->nvec_nonempty = M->nvec_nonempty ;
    C->nvec = M->nvec ;

    GBURBLE ("(GPU C created and copied from M) ") ;
    //--------------------------------------------------------------------------
    // stringify the semiring and the mask
    //--------------------------------------------------------------------------

    char semiring_name [GB_CUDA_STRLEN+2] ;
    char semiring_code [GB_CUDA_STRLEN+2] ;
    char mask_name [GB_CUDA_STRLEN+2] ;

    GB_cuda_stringify_semiring (semiring, flipxy,
        ctype, A->type, B->type, M->type, Mask_struct,  // matrix types
        true, semiring_name, semiring_code, mask_name) ;

    GBURBLE ("(GPU stringified) ") ;
    //--------------------------------------------------------------------------
    // construct the tasks for phase1 and phase2
    //--------------------------------------------------------------------------

    // on the CPU: nthreads = GB_nthreads (cnz, chunk, nthreads_max) ;
    // on the GPU:

    // # of threads in phase1 and phase2 kernel launches must be the same
    #define chunksize 128 
    #define SYMBOLIC_PHASE_NTHREADS 32 
    #define NBUCKETS (GB_BUCKET_MERGEPATH + 1)

    number_of_sms = GB_Global_gpu_sm_get (0) ;
    // C and M have cnz entries, so create ...
    //ntasks = ( (mnvec +7)/8   + SYMBOLIC_PHASE_NTHREADS -1 )/SYMBOLIC_PHASE_NTHREADS;
    ntasks =  ( mnz +chunksize -1)/chunksize;
    // Idea is to have each task work on a continguous block of columns of C
    ntasks = GB_IMIN( ntasks,  128*number_of_sms) ;    // ntasks will be grid.x

    GBURBLE ("(GPU mnz=%ld mnvec=%ld blockDim=32, nblock= %d) ", mnz, mnvec, ntasks ) ;

    std::cout<< "ntasks, nthreads = " <<ntasks<<","<<SYMBOLIC_PHASE_NTHREADS<<std::endl; 
    //--------------------------------------------------------------------------
    // phase1 and phase2: place each C(i,j) in a bucket
    //--------------------------------------------------------------------------

    cudaMalloc ((void**) &Nanobuckets,
        NBUCKETS * SYMBOLIC_PHASE_NTHREADS * ntasks * sizeof (int64_t)) ;

    //Nanobuckets = (int64_t*)GB_cuda_malloc (
    //    NBUCKETS * SYMBOLIC_PHASE_NTHREADS * ntasks * sizeof (int64_t)) ;
    //cudaMemAdvise( Nanobuckets, NBUCKETS * SYMBOLIC_PHASE_NTHREADS * ntasks
    //                           * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, device); 
    /*
    */

    cudaMalloc ((void**) &Blockbucket,
        NBUCKETS * ntasks* sizeof (int64_t) ) ;
    //Blockbucket = (int64_t*)GB_cuda_malloc ( NBUCKETS * ntasks* sizeof (int64_t) ) ;
    //cudaMemAdvise( Blockbucket, NBUCKETS * ntasks
    //                           * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, device); 
    /*
    */

    cudaMalloc ((void**) &Bucket, cnz*sizeof(int64_t));
    //Bucket = (int64_t*)GB_cuda_malloc ( cnz*sizeof(int64_t) );
    //cudaMemAdvise( Bucket, cnz * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, device); 
    /*
    */

    //cudaMallocManaged ((void**) &Bucketp, (NBUCKETS+1)*sizeof(int64_t)) ;
    Bucketp = (int64_t*)GB_cuda_malloc( (NBUCKETS+1)*sizeof(int64_t) ) ;
    cudaMemAdvise( Bucketp, (NBUCKETS+1) * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, cudaCpuDeviceId); 
    cudaMemAdvise( Bucketp, (NBUCKETS+1) * sizeof ( int64_t), cudaMemAdviseSetAccessedBy, device); 

    //cudaMallocManaged ((void**) &offset, (NBUCKETS)*sizeof(int64_t)) ;
    offset = (int64_t*)GB_cuda_malloc( (NBUCKETS)*sizeof(int64_t) ) ;
    cudaMemAdvise( offset, NBUCKETS * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, cudaCpuDeviceId); 
    cudaMemAdvise( offset, NBUCKETS * sizeof ( int64_t), cudaMemAdviseSetAccessedBy, device); 

    memset( offset, 0, NBUCKETS * sizeof(int64_t) ); 
    
  /* 
    if (Blockbucket == NULL || Nanobuckets == NULL || Bucket == NULL || Bucketp == NULL )
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY) ;
    }
    */
    

    //--------------------------------------------------------------------------
    // Pre-fetch arrays that will be used on the device
    //--------------------------------------------------------------------------

    
    //cudaMemPrefetchAsync( Nanobuckets, NBUCKETS * SYMBOLIC_PHASE_NTHREADS
    //                     * ntasks * sizeof (int64_t), device, NULL) ;

    //cudaMemPrefetchAsync( Blockbucket, NBUCKETS * ntasks 
    //                        * sizeof (int64_t), device, NULL) ;

    //cudaMemPrefetchAsync( Bucket, cnz * sizeof (int64_t), device, NULL) ;
    

    /*
    
    //cudaStream_t stream_data;
    //cudaStreamCreate ( &stream_data);
    */
    /* 
    cudaMemAdvise( M->p, (mnvec+1) * sizeof (int64_t), cudaMemAdviseSetPreferredLocation, device) ;
    cudaMemAdvise( M->i, mnz * sizeof ( int64_t), cudaMemAdviseSetPreferredLocation, device); 
    cudaMemAdvise( M->x, mnz * M->type->size, cudaMemAdviseSetPreferredLocation,device) ;
    
    cudaMemAdvise( M->p, (mnvec+1) * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( M->i, mnz * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( M->x, mnz * M->type->size, cudaMemAdviseSetReadMostly,device) ;
    */

    cudaMemPrefetchAsync( M->p, (mnvec+1) * sizeof (int64_t), device, NULL) ; //stream_data) ;
    cudaMemPrefetchAsync( M->i, mnz * sizeof (int64_t), device, NULL ) ; //stream_data) ;
    cudaMemPrefetchAsync( M->x, mnz * M->type->size, device, NULL ) ; //stream_data) ;
    /*
    cudaMemAdvise( C->p, (mnvec+1) * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( C->i, mnz * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( C->x, mnz * C->type->size, cudaMemAdviseSetReadMostly,device) ;
    */
    //cudaMemPrefetchAsync( C->p, (mnvec+1) * sizeof (int64_t), device, NULL) ; //stream_data) ;
    cudaMemPrefetchAsync( C->i, mnz * sizeof (int64_t), device, NULL ); //stream_data) ;
    cudaMemPrefetchAsync( C->x, mnz * C->type->size, device, NULL ); //stream_data) ;
    
    /*
    cudaMemAdvise( A->p, (anvec+1) * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( A->i, anz * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( A->x, anz * A->type->size, cudaMemAdviseSetReadMostly,device) ;
    */
    cudaMemPrefetchAsync( A->p, (anvec+1) * sizeof (int64_t), device, NULL); // stream_data) ;
    cudaMemPrefetchAsync( A->i, anz * sizeof (int64_t), device, NULL ) ; //stream_data) ;
    cudaMemPrefetchAsync( A->x, anz * A->type->size, device, NULL ) ; //stream_data) ;

    /*
    cudaMemAdvise( B->p, (bnvec+1) * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( B->i, bnz * sizeof (int64_t), cudaMemAdviseSetReadMostly, device) ;
    cudaMemAdvise( B->x, bnz * B->type->size, cudaMemAdviseSetReadMostly, device) ;
    */
    cudaMemPrefetchAsync( B->p, (bnvec+1) * sizeof (int64_t), device, NULL) ; //stream_data) ;
    cudaMemPrefetchAsync( B->i, bnz * sizeof (int64_t), device, NULL ) ; //stream_data) ;
    cudaMemPrefetchAsync( B->x, bnz * B->type->size, device, NULL ) ; //stream_data) ;

    

    // The work to compute C(i,j) is held in Ci [p], if C(i,j) appears in
    // as the pth entry in C.
    GB_callback mysemiring;
    const char *header_name = (const char *)"mySemiRing.h";
    mysemiring.load_string(header_name, semiring_code ) ;
    SR_callback_ptr = &mysemiring;


    //cudaStream_t stream_AxB;
    //cudaStreamCreate ( &stream_AxB);
    //----------------------------------------------------------------------
    // phase1: assign each C(i,j) to a bucket, and count them
    //----------------------------------------------------------------------
    dim3 grid( ntasks) ; 
    dim3 p2grid( (ntasks +  SYMBOLIC_PHASE_NTHREADS -1)
                          / (SYMBOLIC_PHASE_NTHREADS) ) ; 
    dim3 block( SYMBOLIC_PHASE_NTHREADS ) ;

    std::string base_name = "GB_jit_AxB_dot3_";
    std::string Opname = "phase1_" ;
    
    jitify::experimental::KernelLauncher phase1Kernel =
    jit::launcher( base_name + Opname + mask_name,
                   templates_GB_jit_AxB_dot3_phase1_cu,
                   header_names,
                   compiler_flags,
                   callback_wrapper) //,
                   //stream_AxB)
               .set_kernel_inst("GB_AxB_cuda_dot3_phase1",
                                {M->type->name})
               .configure(grid, block); 

    //----------------------------------------------------------------------
    // phase2: cumsum across the blockbuckets, propagate to thread level
    //----------------------------------------------------------------------
    base_name = "GB_jit_AxB_dot3_";
    Opname = "phase2";
    jitify::experimental::KernelLauncher phase2Kernel =
    jit::launcher( base_name + Opname,
                   templates_GB_jit_AxB_dot3_phase2_cu,
                   header_names,
                   compiler_flags,
                   callback_wrapper) //,
                   //stream_AxB)
               .set_kernel_inst("GB_AxB_dot3_phase2",
                                {})
               .configure(p2grid, block);

    base_name = "GB_jit_AxB_dot3_";
    Opname = "phase2";
    jitify::experimental::KernelLauncher phase2endKernel =
    jit::launcher( base_name + Opname,
                   templates_GB_jit_AxB_dot3_phase2_cu,
                   header_names,
                   compiler_flags,
                   callback_wrapper) //,
                   //stream_AxB)
               .set_kernel_inst("GB_AxB_dot3_phase2end",
                                {})
               .configure(grid, block);


    phase1Kernel.launch(
                        Nanobuckets,       // array of size NBUCKETS-blockDim.x-by-gridDim.x
                        Blockbucket,       // bucket counts, of size NBUCKETS-by-gridDim.x
                                           // input/output:
                        C,                 // final output matrix
                                           // inputs, not modified:
                        M,                 // mask matrix
                        A,                 // input matrix
                        B                  // input matrix
                    );


    // cudaDeviceSynchronize();


    GBURBLE ("(GPU phase1 done) ") ;
    //for (int i = 0; i< cnz; i++){
    //  printf("C[%d] = %ld\n", i , Ci[i]);
    //}
    //----------------------------------------------------------------------
    // phase2: cumsum across the blockbuckets, propagate to thread level
    //----------------------------------------------------------------------
    int nblock = ntasks;

    phase2Kernel.launch(                    // input
                        Nanobuckets,       // array of size NBUCKETS-blockDim.x-by-gridDim.x
                        Blockbucket,       // bucket counts, of size NBUCKETS-by-gridDim.x
                                           // input/output:
                        Bucketp,           // global bucket cumsum, of size NBUCKETS+1
                        Bucket,            // global buckets, of size cnz (== mnz)
                        offset,
                        C,                 // final output matrix
                                           // inputs, not modified:
                        cnz,               // number of entries in mask and output matrix
                        nblock
                    );

    cudaDeviceSynchronize();
    //cudaMemPrefetchAsync( offset, (NBUCKETS) * sizeof (int64_t), cudaCpuDeviceId, NULL) ;

    int64_t s= 0;
    for ( int bucket = 0 ; bucket < NBUCKETS+1; ++bucket)
    {
       Bucketp[bucket] = s; 
       s+= offset[bucket];
       //printf("bucketp[%d] = %ld\n", bucket, Bucketp[bucket]);
    }

    GBURBLE ("(GPU phase2 done) ") ;

    phase2endKernel.launch(                    // input
                        Nanobuckets,       // array of size NBUCKETS-blockDim.x-by-gridDim.x
                        Blockbucket,       // bucket counts, of size NBUCKETS-by-gridDim.x
                                           // input/output:
                        Bucketp,           // global bucket cumsum, of size NBUCKETS+1
                        Bucket,            // global buckets, of size cnz (== mnz)
                        offset,
                        C,                 // final output matrix
                                           // inputs, not modified:
                        cnz                // number of entries in mask and output matrix
                    );

    cudaDeviceSynchronize();

    GBURBLE ("(GPU phase2end done) ") ;
    /* 
    for (int i = 0; i< cnz; i++){
      printf("C[%d],Bucket = %ld,%ld\n", i , Ci[i], Bucket[i]);
    }
    */
    
    //----------------------------------------------------------------------
    // phase3: do the numerical work
    //----------------------------------------------------------------------

    base_name = "GB_jit_";
    std::string kernel_name = "AxB_dot3_phase3_";
    C->nzombies = Bucketp[1];  //set pre-zombie counts

    for ( int bucket = 1 ; bucket < NBUCKETS; ++bucket)
    {
        std::string Opname = "";
        int sz = 0 ;

        const char*  jit_template;

        int64_t start = Bucketp[bucket];
        int64_t end = Bucketp[bucket+1];

        //if( (end-start>0)  && (start == Bucketp[1]) ) start = Bucketp[0]; //add in zombie slots

        int64_t Cnz = end- start;

        int gridsz, blocksz;

        //Nothing to do, next bucket
        if ( Cnz == 0 ) continue;

        GBURBLE ("\n\n(GPU phase3 bucket,bucketsize= %d,%ld) ",bucket,Cnz) ;

        switch (bucket)
        {

            //--------------------------------------------------------------
            // not a bucket ... bring out your dead:
            //--------------------------------------------------------------

            case GB_BUCKET_ZOMBIE : // C(i,j) is a zombie (not a bucket)
                break ;

            //--------------------------------------------------------------
            // CUDA kernel: dndn, handles a single bucket:
            //--------------------------------------------------------------

            // both A(:,i) and B(:,j) are dense
            case GB_BUCKET_DNDN :
                Opname = "dndn" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_dndn_cu;
                blocksz = 32;
                gridsz = ( Cnz -1 + blocksz)/blocksz;
                break ;

            //--------------------------------------------------------------
            // CUDA kernel: spdn, handles 4 buckets:
            //--------------------------------------------------------------

            // A(:,i) is dense and B(:,j) is very sparse (< 256 entries)
            case GB_BUCKET_DNVS :
            // A(:,i) is very sparse (< 256 entries) and B(:,j) is dense
            case GB_BUCKET_VSDN :
                sz = 64 ;
                Opname = "spdn" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_spdn_cu;
                blocksz = 32;
                gridsz = ( Cnz -1 + blocksz)/blocksz;
                break ;

            // A(:,i) is dense and B(:,j) is sparse (>= 256 entries)
            case GB_BUCKET_DNSP :
            // A(:,i) is sparse (>= 256 entries) and B(:,j) is dense
            case GB_BUCKET_SPDN :
                sz = 256 ;
                Opname = "spdn" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_spdn_cu;
                blocksz = 32;
                gridsz = ( Cnz -1 + blocksz)/blocksz;
                break ;

            //--------------------------------------------------------------
            // CUDA kernel: vssp, handles 1 bucket, uses binary search:
            //--------------------------------------------------------------

            // A(:,i) is very sparse compared to B(:,j), or visa versa
            case GB_BUCKET_VSSP :
                Opname = "vssp" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_vssp_cu;
                blocksz = 32;
                gridsz = ( Cnz -1 + blocksz)/blocksz;
                break ;

            //--------------------------------------------------------------
            // CUDA kernel: vsvs, handles 4 buckets:
            //--------------------------------------------------------------

            // let len = nnz (A (:,i) + nnz (B (:,j)), then:
            
            case GB_BUCKET_VSVS_256 : sz += 256-64 ;
            case GB_BUCKET_VSVS_64 :  sz += 64-16  ;
            case GB_BUCKET_VSVS_16 :  sz += 16-4   ;
            case GB_BUCKET_VSVS_4 :   sz += 4      ;
                Opname = "vsvs" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_vsvs_cu;
                blocksz = 1024;
                gridsz = GB_IMIN( 1024*number_of_sms, ( Cnz  + blocksz -1 )/blocksz);
                gridsz =  ( Cnz  + blocksz -1 )/blocksz;
                /*
                Opname = "warpix" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_warpix_cu;
                blocksz = 32;
                gridsz =  GB_IMIN( (mnvec+15)/16, 256*number_of_sms);
                */
                break ;
            
            //--------------------------------------------------------------
            // CUDA kernel: mp, use the merge-path method:
            //--------------------------------------------------------------

            case GB_BUCKET_MERGEPATH :
                Opname = "mp" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_mp_cu;
                blocksz = 32;
                gridsz = ( Cnz -1 + blocksz)/blocksz;
                break ;

            case GB_BUCKET_WARP_IX :   sz = 32      ;
                Opname = "warpix" ;
                jit_template = templates_GB_jit_AxB_dot3_phase3_warpix_cu;
                blocksz = 32;
                gridsz =  GB_IMIN( (mnvec+15)/16, 256*number_of_sms);
                break ;

            default:
                break ;
        }

        dim3 grid(gridsz);
        dim3 block(blocksz);

        std::cout<< "Kernel name =" <<Opname<<std::endl; 
        GBURBLE ("(GPU phase3 launch st,end=%ld,%ld nblocks,blocksize= %d,%d )\n",start,end,gridsz,blocksz) ;
        jit::launcher( base_name + kernel_name + Opname + "_" + semiring_name,
                       jit_template,
                       header_names,
                       compiler_flags,
                       callback_wrapper)
                   .set_kernel_inst(kernel_name + Opname,
                                    { ctype->name,
                                      A->type->name,
                                      B->type->name,
                                      semiring->multiply->xtype->name,
                                      semiring->multiply->ytype->name,
                                      semiring->multiply->ztype->name  })
                   .configure(grid, block) //if commented, use implicit 1D configure in launch
                   .launch(
                            start,   // input/output:
                            end, // global bucket cumsum, of size NBUCKETS+1
                            Bucket,            // global buckets, of size cnz (== mnz)
                            C,                 // final output matrix
                                               // inputs, not modified:
                            M,                 // Mi used for column index
                            A,                 // A matrix
                            B,                 // B matrix
                            sz                 // only used for sparse-sparse cases

                        );

        cudaDeviceSynchronize();
    }
    GBURBLE ("(GPU phase3 done) ") ;
    
    std::string reduce_kernel_name = "reduceNonZombiesWarp";
    const char*  jit_template;
    #define red_blocksz 1024
    jit_template = templates_reduceNonZombiesWarp_cu;
    int num_reduce_blocks = GB_IMIN( 32*number_of_sms, (cnz + red_blocksz -1)/ red_blocksz  ) ;
    dim3 red_grid( num_reduce_blocks ) ; 
    dim3 red_block( red_blocksz ) ;

    int32_t *block_sum;
    //cudaMallocManaged ((void**) &block_sum, (num_reduce_blocks)*sizeof(int32_t)) ;
    block_sum = (int32_t*)GB_cuda_malloc( (num_reduce_blocks)*sizeof(int32_t)) ;

    GBURBLE ("(GPU reduce launch nblocks,blocksize= %d,%d )\n", num_reduce_blocks, red_blocksz) ;
    jit::launcher( reduce_kernel_name + "_" + semiring_name,
                   jit_template,
                   header_names,
                   compiler_flags,
                   callback_wrapper)
                   .set_kernel_inst( reduce_kernel_name , { ctype->name })
                   .configure(red_grid, red_block) //if commented, use implicit 1D configure in launch
                   .launch(
                            C->i,   // index vector, only sum up values >= 0
                            C->x,   // input pointer to vector to reduce, with zombies
                            block_sum,             // Block sums on return 
                            (unsigned int)cnz      // length of vector to reduce to scalar

                        );

    cudaDeviceSynchronize();

    int32_t num_triangles = 0;
    for (int i = 0; i< num_reduce_blocks; i++){
       //printf("block%d num_triangles = %d\n", i, block_sum[i] );
       num_triangles += block_sum[i] ;
    }
    printf("num_triangles = %d\n",  num_triangles );

    GB_cuda_free( block_sum ); 
    //cudaMemPrefetchAsync( C->p, (mnvec+1) * sizeof (int64_t), cudaCpuDeviceId, NULL) ; //stream_data ) ;
    //cudaMemPrefetchAsync( C->i, cnz * sizeof (int64_t), cudaCpuDeviceId, NULL ) ; //stream_data ) ;
    //cudaMemPrefetchAsync( C->x, cnz * sizeof (int32_t), cudaCpuDeviceId, NULL ) ; //stream_data ) ;
    /*
    cudaMemcpy( Citemp, C->i, cnz * sizeof( int64_t), cudaMemcpyDefault );    
    cudaMemcpy( Cxtemp, C->x, cnz * C->type->size, cudaMemcpyDefault );    
    GB_cuda_free( C->i);
    GB_cuda_free( C->x);
    C->i = Citemp;
    C->x = Cxtemp;
    */

    cudaDeviceSynchronize();

    return GrB_SUCCESS; 
}

