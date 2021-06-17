// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <cmath>
#include <random>
#include <algorithm>
#include "jitFactory.hpp"
#include "../binary_search.h"
#include "GpuTimer.h"
#include "gtest/gtest.h"

//Operations for test results on CPU
template<typename T> T myOP_plus( T a, T b) { return  a + b;}
template<typename T> T myOP_min ( T a, T b) { return  a < b ? a : b;}
template<typename T> T myOP_max ( T a, T b) { return  a > b ? a : b;}
template<typename T> T myOP_first ( T a, T b) { return  a ;}
template<typename T> T myOP_second ( T a, T b) { return  b ;}
template<typename T> T myOP_times ( T a, T b) { return  a * b ;}

template<typename T> T (*myOpPTR)(T a, T b);
template<typename T> T (*ADD_ptr)(T a, T b);
template<typename T> T (*MUL_ptr)(T a, T b);


//Test generators using jitify
template <typename T>
bool test_reducefactoryUM( unsigned int N, std::string OP) ;

template <typename T1,typename T2,typename T3>
bool test_dndotfactoryUM( unsigned int N, std::string SEMI_RING) ;

template <typename T1,typename T2,typename T3>
bool test_spdotfactoryUM( unsigned int N, unsigned int xn, unsigned int yn, std::string SEMI_RING) ;

//AxB_dot3_phase1 kernels
template <typename T_C, typename T_M, typename T_A,typename T_B>
bool test_AxB_dot3_phase1_factory( int64_t , int64_t , int64_t , int64_t ) ;

//AxB_dot3_phase2 kernels
template <typename T_C>
bool test_AxB_dot3_phase2_factory( int , int64_t , int64_t , int64_t, int64_t ) ;

template <typename T_C>
bool test_AxB_dot3_phase2end_factory( int , int64_t , int64_t , int64_t ) ;

//AxB_dot3_phase3 kernels
template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_dndn_factory( int , int64_t , int64_t , int64_t , std::string&) ;

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_vsvs_factory( int , int64_t , int64_t , int64_t , std::string&) ;

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_spdn_factory( int , int64_t , int64_t , int64_t , std::string&) ;

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_vssp_factory( int , int64_t , int64_t , int64_t , std::string&) ;

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_mp_factory( int , int64_t , int64_t , int64_t , std::string&) ;

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_warp_factory( int , int64_t , int64_t , int64_t , std::string&) ;


//Fixture to generate valid inputs and hold them for tests
class AxB_dot3_Test : public ::testing::Test
{
   void SetUp() 
   {


   }

   void TearDown()
   {

   }

}

// Test generator code, to allow parameterized tests
// Uses jitFactory, dataFactory and GB_jit 
template <typename T_C, typename T_M, typename T_A,typename T_B>
bool test_AxB_dot3_phase1_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz) {

int gpuID; 
cudaGetDevice( &gpuID);

std::cout<< "found device "<<gpuID<<std::endl;

phase1launchFactory<T_C, T_M, T_A, T_B> p1lF(); 

SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;
int64_t Annz = N*N;
int64_t Bnnz = N*N;
int64_t Cnz = N; 
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Annz, Bnnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;

       int nthrd = 32;
       int sz = 4;
       //int m = 256/sz;
       int nblck = Cnz; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       p1lF.jitGridBlockLaunch( nblck, nthrd, nanobuckets, Bucket,
                                C, M, A, B);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       return true;

}

template <typename T_C, typename T_M, typename T_A,typename T_B>
bool test_AxB_dot3_phase2_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz) {

int gpuID; 
cudaGetDevice( &gpuID);

std::cout<< "found device "<<gpuID<<std::endl;

phase2launchFactory<T_C, T_M, T_A, T_B> p2lF(); 

SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;
int64_t Annz = N*N;
int64_t Bnnz = N*N;
int64_t Cnz = N; 
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Annz, Bnnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;

       int nthrd = 32;
       int sz = 4;
       //int m = 256/sz;
       int nblck = Cnz; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       p1lF.jitGridBlockLaunch( nblck, nthrd, nanobuckets, Bucket,
                                C, M, A, B);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       return true;
}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_full_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Generates three randomized matrices, builds buckets and calls a kernel.
// This is the full version as called in SuiteSparse:GraphBLAS 

phase1launchFactory<T_C, T_M, T_A, T_B> p1lF(); 
phase2launchFactory<T_C> p2lF(); 
launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "dndn"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
int gpuID; 
cudaGetDevice( &gpuID);

std::cout<< "found device "<<gpuID<<std::endl;

T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
   std::cout << "Plus Times (+,*) semiring"<<std::endl;
   MONOID_IDENTITY = 0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
   std::cout << "Min Plus Times (min,+) semiring"<<std::endl;
   MONOID_IDENTITY = std::numeric_limits<T_Z>::max();
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
   MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   std::cout << "Max Plus Times (max,+) semiring"<<std::endl;
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
// The testBucket arg tells the generator which bucket we want to exercise
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G( testBucket);
int64_t Annz = N*N;
int64_t Bnnz = N*N;
int64_t Cnz = N; 
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Annz, Bnnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;

// Set clear zombie count
C->zombie_count = 0;

//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;

bool result = false;

   // Phase 1
       int nthrd = 32;
       int sz = 4;
       //int m = 256/sz;
       int nblck = Cnz; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       p1lF.jitGridBlockLaunch( nblck, nthrd, nanobuckets, Bucket,
                                C, M, A, B);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

   // Phase 2
       int nthrd = 32;
       int sz = 4;
       //int m = 256/sz;
       int nblck = Cnz; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       p2lF.jitGridBlockLaunch( nblck, nthrd, nanobuckets, Bucket, bucketp, 
                                C);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;


for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs > 0) std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for dense-dense kernels
       int nthrd = 32;
       int sz = 4;
       //int m = 256/sz;
       int nblck = Cnz; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            //std::cout<<"Cx[i] = "<<Cx[i]<<std::endl;
            X_valid[i] = Cx[i];
            Cx[i] = 0;
            i_valid[i] = C->i[i]; 
       }
       G.loadCj(); 

       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;
        int64_t i = M->i[pC] ;          // row index of C(i,j)

        // get C(i,j)
        int64_t k = (C->i [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

        // xvp, xvi, xvals:  A(:,i)
        // xvp is Ap [i] and Ap [i+1]
        int64_t pA_start = A->p [i] ;
        int64_t pA_end   = A->p [i+1] ;
        // indices are in Ai [pA_start ... pA_end-1]
        // values  are in Ax [pA_start ... pA_end-1]

        // yvp, yvi, yvals:  B(:,j)
        // yvp is Bp [j] and Bp [j+1]
        int64_t pB_start = B->p [j] ;
        int64_t pB_end   = B->p [j+1] ;
        // indices are in Bi [pB_start ... pB_end-1]
        // values  are in Bx [pB_start ... pB_end-1]
        k = pA_start;
        int64_t l = pB_start;
        T_Z cij = MONOID_IDENTITY;
        while( k < pA_end && l < pB_end) {
           //std::cout<<" A*B="<< (*MUL_ptr<T_Z>) ( (T_Z)Ax[k] , (T_Z) Bx[l]) <<std::endl ;
           cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)Ax[k] , (T_Z) Bx[l]) ) ;
           k++;
           l++;
           //std::cout<<"Ak = "<< Ax[k]<< " Bl = "<< Bx[l]<< "sum ="<<sum<<std::endl;
        }
        //std::cout<< " dot  = "<< sum << std::endl;

        // output for this dot product is
        
        if (cij == MONOID_IDENTITY) {
            C->i [pC] = -1;//GB_FLIP (i)
            C->zombie_count++;
        }
        else {
            Cx [pC] = (T_C)cij;
            C->i [pC] = i;
        }
    }
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             int64_t i =  C->i[l];
             //std::cout<<i<<","<<j<<","<<l <<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
             if (i >= 0) 
                err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count CPU = "<<C->get_zombie_count()<<" zGPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_dndn_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Assumes all matrices are square so far, so only N dimension given.
// Sparsity is dense here so Anz = Bnz = N*N. 
// Generates three randomized matrices, builds buckets and calls a kernel.


launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "dndn"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
int gpuID; 
cudaGetDevice( &gpuID);

std::cout<< "found device "<<gpuID<<std::endl;

T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
   std::cout << "Plus Times (+,*) semiring"<<std::endl;
   MONOID_IDENTITY = 0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
   std::cout << "Min Plus Times (min,+) semiring"<<std::endl;
   MONOID_IDENTITY = std::numeric_limits<T_Z>::max();
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
   MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   std::cout << "Max Plus Times (max,+) semiring"<<std::endl;
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;
int64_t Annz = N*N;
int64_t Bnnz = N*N;
int64_t Cnz = N; 
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Annz, Bnnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;

// Set clear zombie count
C->zombie_count = 0;

//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;

bool result = false;

for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs > 0) std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for dense-dense kernels
       int nthrd = 32;
       int sz = 4;
       //int m = 256/sz;
       int nblck = Cnz; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            //std::cout<<"Cx[i] = "<<Cx[i]<<std::endl;
            X_valid[i] = Cx[i];
            Cx[i] = 0;
            i_valid[i] = C->i[i]; 
       }
       G.loadCj(); 

       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;
        int64_t i = M->i[pC] ;          // row index of C(i,j)

        // get C(i,j)
        int64_t k = (C->i [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

        // xvp, xvi, xvals:  A(:,i)
        // xvp is Ap [i] and Ap [i+1]
        int64_t pA_start = A->p [i] ;
        int64_t pA_end   = A->p [i+1] ;
        // indices are in Ai [pA_start ... pA_end-1]
        // values  are in Ax [pA_start ... pA_end-1]

        // yvp, yvi, yvals:  B(:,j)
        // yvp is Bp [j] and Bp [j+1]
        int64_t pB_start = B->p [j] ;
        int64_t pB_end   = B->p [j+1] ;
        // indices are in Bi [pB_start ... pB_end-1]
        // values  are in Bx [pB_start ... pB_end-1]
        k = pA_start;
        int64_t l = pB_start;
        T_Z cij = MONOID_IDENTITY;
        while( k < pA_end && l < pB_end) {
           //std::cout<<" A*B="<< (*MUL_ptr<T_Z>) ( (T_Z)Ax[k] , (T_Z) Bx[l]) <<std::endl ;
           cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)Ax[k] , (T_Z) Bx[l]) ) ;
           k++;
           l++;
           //std::cout<<"Ak = "<< Ax[k]<< " Bl = "<< Bx[l]<< "sum ="<<sum<<std::endl;
        }
        //std::cout<< " dot  = "<< sum << std::endl;

        // output for this dot product is
        
        if (cij == MONOID_IDENTITY) {
            C->i [pC] = -1;//GB_FLIP (i)
            C->zombie_count++;
        }
        else {
            Cx [pC] = (T_C)cij;
            C->i [pC] = i;
        }
    }
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             int64_t i =  C->i[l];
             //std::cout<<i<<","<<j<<","<<l <<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
             if (i >= 0) 
                err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count CPU = "<<C->get_zombie_count()<<" zGPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_vsvs_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Assumes all matrices are square so far, so only N dimension given.
// Sparsity is controlled by Anz and Bnz vs N*N. 
// Generates three randomized matrices, builds buckets and calls a kernel.


launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "vsvs"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
int gpuID; 
cudaGetDevice( &gpuID);
std::cout<< "found device "<<gpuID<<std::endl;

//T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
   //MONOID_IDENTITY =(T_Z)0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::max(); 
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;
int64_t Cnz = N;
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Anz, Bnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;
int64_t *Ci = C->i;
int64_t *Mi = M->i;
int64_t *Ai = A->i;
int64_t *Bi = B->i;
int64_t *Ap = A->p;
int64_t *Bp = B->p;

//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;

bool result = false;

for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs > 0) std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for v.sparse-v.sparse kernels
       int nthrd = 32;
       int sz = Anz/N;
       int m = 256/sz;
       int nblck = (Cnz -1 + m*nthrd )/(m*nthrd) ; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       //std::cout<<"returned from kernel"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            X_valid[i] = Cx[i];
            Cx[i] = 0;
            i_valid[i] = Ci[i]; 
       }
       G.loadCj(); 
       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;
        int64_t i = Mi[pC] ;          // row index of C(i,j)

        // get C(i,j)
        int64_t k = (Ci [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

        // xvp, xvi, xvals:  A(:,i)
        // xvp is Ap [i] and Ap [i+1]
        int64_t pA_start = Ap [i] ;
        int64_t pA_end   = Ap [i+1] ;
        // indices are in Ai [pA_start ... pA_end-1]
        // values  are in Ax [pA_start ... pA_end-1]

        // yvp, yvi, yvals:  B(:,j)
        // yvp is Bp [j] and Bp [j+1]
        int64_t pB_start = Bp [j] ;
        int64_t pB_end   = Bp [j+1] ;
        // indices are in Bi [pB_start ... pB_end-1]
        // values  are in Bx [pB_start ... pB_end-1]
        k = pA_start;
        int64_t l = pB_start;
        T_Z cij ;
        bool cij_exists = false;
        while( k < pA_end && l < pB_end) {
            if ( Ai[k] < Bi[l]) ++k;
            else if ( Ai[k] > Bi[l]) ++l;
            else {
                if (cij_exists) {
                   cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( Ax[k] , Bx[l] ) );
                }
                else{
                   cij_exists = true;
                   cij = (*MUL_ptr<T_Z>)( Ax[k], Bx[l]);
                }
                k++;
                l++;
            }
        }
        //std::cout<< " dot  = "<< sum << std::endl;

        // output for this dot product is
        
        if (cij_exists) {
            Ci [pC] = i;
            Cx[pC] = (T_C)cij;
        }
        else {
            Ci [pC] = -1;//GB_FLIP (i)
            C->zombie_count++;
        }
    }
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             //std::cout<<i<<","<<j<<","<<l <<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
             if (Ci[l] > 0)
                err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count GPU = "<<C->get_zombie_count()<<" zCPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_vssp_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Assumes all matrices are square so far, so only N dimension given.
// Sparsity is controlled by Anz and Bnz vs N*N. 
// Generates three randomized matrices, builds buckets and calls a kernel.

launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "vssp"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
int gpuID; 
cudaGetDevice( &gpuID);
std::cout<< "found device "<<gpuID<<std::endl;

//T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
   //MONOID_IDENTITY =(T_Z)0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::max();
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;

int64_t Cnz = N;
float Cnzpercent = (float)( Cnz)/(N*N);

G.init(N, Anz, Bnz, Cnzpercent );

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;
int64_t *Ci = C->i;
int64_t *Mi = M->i;
int64_t *Ai = A->i;
int64_t *Bi = B->i;
int64_t *Ap = A->p;
int64_t *Bp = B->p;


//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;
int zc = 0;

bool result = false;

for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs == 0) continue; 
    std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for v.sparse-dense kernels
       int nthrd = 32;
       int sz = 4; 
       //int m = 256/sz;
       int nblck = (Cnz -1 + nthrd )/(nthrd) ; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       //std::cout<<"returned from kernel"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            X_valid[i] = Cx[i];
            Cx[i] = 0;
            i_valid[i] = C->i[i]; 
       }
       G.loadCj(); 


       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;

        int64_t i = Mi[pC] ;          // row index of C(i,j)
        // get C(i,j)
        int64_t k = (Ci [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

        int64_t pA      = Ap[i];
        int64_t pA_end  = Ap[i+1];
        int64_t nnzA = pA_end - pA;

        int64_t pB      = Bp[j]; 
        int64_t pB_end  = Bp[j+1]; 
        int64_t nnzB = pB_end - pB;

        //Search for each nonzero in the smaller vector to find intersection 
        bool cij_exists = false;

        T_A aki;
        T_B bkj;
        T_Z cij;

        if (nnzA <= nnzB) {
            //----------------------------------------------------------------------
            // A(:,i) is very sparse compared to B(:,j)
            //----------------------------------------------------------------------

            while (pA < pA_end && pB < pB_end)
            {
                int64_t ia = Ai [pA] ;
                int64_t ib = Bi [pB] ;
                if (ia < ib)
                { 
                    // A(ia,i) appears before B(ib,j)
                    pA++ ;
                }
                else if (ib < ia)
                { 
                    // B(ib,j) appears before A(ia,i)
                    // discard all entries B(ib:ia-1,j)
                    int64_t pleft = pB + 1 ;
                    int64_t pright = pB_end - 1 ;
                    GB_BINARY_TRIM_SEARCH (ia, Bi, pleft, pright) ;
                    //ASSERT (pleft > pB) ;
                    pB = pleft ;
                }
                else // ia == ib == k
                { 
                    // A(k,i) and B(k,j) are the next entries to merge
                    #if defined ( GB_PHASE_1_OF_2 )
                    cij_exists = true ;
                    break ;
                    #else
                    GB_GETA (aki, Ax, pA) ;             /* aki = A(k,i) */          
                    GB_GETB (bkj, Bx, pB) ;             /* bkj = B(k,j) */         
                    if (cij_exists)                                                 
                    {                                                               
                        cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)aki , (T_Z)bkj ) );
                        /* cij += aki * bkj */      
                    }                                                               
                    else                                                           
                    {                                                               
                        /* cij = A(k,i) * B(k,j), and add to the pattern */         
                        cij_exists = true ;                                         
                        cij=  (*MUL_ptr<T_Z>)( (T_Z)aki, (T_Z)bkj) ;     
                        /* cij = aki * bkj */       
                    }                                                               
                    //GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                    pA++ ;
                    pB++ ;
                    #endif
                }
            }
        }
        else {
            //----------------------------------------------------------------------
            // B(:,j) is very sparse compared to A(:,i)
            //----------------------------------------------------------------------

            while (pA < pA_end && pB < pB_end)
            {
                int64_t ia = Ai [pA] ;
                int64_t ib = Bi [pB] ;
                if (ia < ib)
                { 
                    // A(ia,i) appears before B(ib,j)
                    // discard all entries A(ia:ib-1,i)
                    int64_t pleft = pA + 1 ;
                    int64_t pright = pA_end - 1 ;
                    GB_BINARY_TRIM_SEARCH (ib, Ai, pleft, pright) ;
                    //ASSERT (pleft > pA) ;
                    pA = pleft ;
                }
                else if (ib < ia)
                { 
                    // B(ib,j) appears before A(ia,i)
                    pB++ ;
                }
                else // ia == ib == k
                { 
                    // A(k,i) and B(k,j) are the next entries to merge
                    #if defined ( GB_PHASE_1_OF_2 )
                    cij_exists = true ;
                    break ;
                    #else
                    GB_GETA (aki, Ax, pA) ;             /* aki = A(k,i) */          
                    GB_GETB (bkj, Bx, pB) ;             /* bkj = B(k,j) */         
                    if (cij_exists)                                                 
                    {                                                               
                        cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)aki , (T_Z)bkj ) );
                        /* cij += aki * bkj */      \
                    }                                                               
                    else                                                           
                    {                                                               
                        /* cij = A(k,i) * B(k,j), and add to the pattern */         
                        cij_exists = true ;                                         
                        cij=  (*MUL_ptr<T_Z>)( (T_Z)aki, (T_Z)bkj) ;     
                    }                                                               
                    //GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                    pA++ ;
                    pB++ ;
                    #endif
                }
            }

        }
        if ( cij_exists){
           Ci[pair] = i;
           Cx[pair] = (T_C)cij;
        }
        else {
           zc++; 
           //printf(" %lld, %lld is zombie %d!\n",i,j,zc);
           Ci[pair] = GB_FLIP( i );
        }

    }
       C->zombie_count = zc;
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             int64_t i = Ci[l];
             //std::cout<<i<<","<<j<<","<<l <<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
             if (i > 0){ //not a zombie!
                 err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
             }
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count GPU = "<<C->get_zombie_count()<<" zCPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_spdn_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Assumes all matrices are square so far, so only N dimension given.
// Sparsity is controlled by Anz and Bnz vs N*N. 
// Generates three randomized matrices, builds buckets and calls a kernel.

launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "spdn"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
int gpuID; 
cudaGetDevice( &gpuID);
std::cout<< "found device "<<gpuID<<std::endl;

//T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
  // MONOID_IDENTITY =(T_Z)0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
  // MONOID_IDENTITY = std::numeric_limits<T_Z>::max();
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
  // MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;

int64_t Cnz = N;
float Cnzpercent = (float)( Cnz)/(N*N);

//spdn case means B should be dense -> Bnz = N*N;
G.init(N, Anz, N*N, Cnzpercent );

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;
int64_t *Ci = C->i;
int64_t *Mi = M->i;
int64_t *Ai = A->i;
int64_t *Bi = B->i;
int64_t *Ap = A->p;
int64_t *Bp = B->p;


//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;

bool result = false;

for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs == 0) continue; 
    std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for v.sparse-dense kernels
       int nthrd = 32;
       int sz = Anz/N;
       int m = 256/sz;
       int nblck = (Cnz -1 + m*nthrd )/(m*nthrd) ; 
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       //std::cout<<"returned from kernel"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            X_valid[i] = Cx[i];
            Cx[i] = 0;
            i_valid[i] = Ci[i]; 
       }
       G.loadCj(); 
       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;
        int64_t i = Mi[pC] ;          // row index of C(i,j)

        // get C(i,j)
        //int64_t k = (Ci [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        //int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

         int64_t pA = Ap[i];
         int64_t pA_end   = Ap[i+1];
         int64_t nnzA   = pA_end - pA;
         int64_t pB = Bp[i];
         int64_t pB_end   = Bp[i+1];
         int64_t nnzB   = pB_end - pB;
         T_A aki;
         T_B bkj;
         T_Z cij;

         if( nnzA == A->vlen) // A is dense
         {
            int64_t k = Bi [pB] ;               // first row index of B(:,j)
            // cij = A(k,i) * B(k,j)
            GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
            GB_GETB (bkj, Bx, pB  ) ;           // bkj = B(k,j)
            cij = (*MUL_ptr<T_Z>)( aki, bkj) ;           // cij = aki * bkj

            for (int64_t p = pB+1 ; p < pB_end ; p++)
            { 
                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
                int64_t k = Bi [p] ;                // next row index of B(:,j)
                // cij += A(k,i) * B(k,j)
                GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
                GB_GETB (bkj, Bx, p   ) ;           // bkj = B(k,j)
                cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)aki, (T_Z)bkj) );
            }

         }
         if( nnzB == B->vlen) // B is dense
         {
            int64_t k = Ai [pA] ;               // first row index of A(:,i)
            // cij = A(k,i) * B(k,j)
            GB_GETA (aki, Ax, pA  ) ;           // aki = A(k,i)
            GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
            cij = (*MUL_ptr<T_Z>)( aki, bkj) ;           // cij = aki * bkj

            for (int64_t p = pA+1 ; p < pA_end ; p++)
            { 
                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
                int64_t k = Ai [p] ;                // next row index of A(:,i)
                // cij += A(k,i) * B(k,j)
                GB_GETA (aki, Ax, p   ) ;           // aki = A(k,i)
                GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
                cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)aki, (T_Z)bkj) );
            }
         }

         Ci[pair] = i;
         Cx[pair] = cij;
        
      }
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             int64_t i =  Ci[l];
         //std::cout<<i<<","<<j<<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
             if (i >=0 )
                err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count GPU = "<<C->get_zombie_count()<<" zCPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_mp_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Assumes all matrices are square so far, so only N dimension given.
// Sparsity is dense here so Anz = Bnz = N*N. 
// Generates three randomized matrices, builds buckets and calls a kernel.


launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "mp"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
//int gpuID; 
//cudaGetDevice( &gpuID);

//std::cout<< "found device "<<gpuID<<std::endl;

//T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
   std::cout << "Plus Times (+,*) semiring"<<std::endl;
   //MONOID_IDENTITY = 0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
   std::cout << "Min Plus Times (min,+) semiring"<<std::endl;
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::max();
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   std::cout << "Max Plus Times (max,+) semiring"<<std::endl;
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;
int64_t Annz = Anz;
int64_t Bnnz = Bnz;
int64_t Cnz = N; 
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Annz, Bnnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;
int64_t *Ci = C->i;
int64_t *Mi = M->i;
int64_t *Ai = A->i;
int64_t *Bi = B->i;
int64_t *Ap = A->p;
int64_t *Bp = B->p;

// Set clear zombie count
C->zombie_count = 0;

//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;

bool result = false;

for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs > 0) std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for merge-path kernel 
       int nthrd = 32;
       int nblck = Cnz; 
       int sz = 0;
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       //std::cout<<"returned from kernel"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            //std::cout<<"Cx[i] = "<<Cx[i]<<std::endl;
            X_valid[i] = Cx[i];
            i_valid[i] = C->i[i]; 
            // clear values for next test
            Cx[i] = 0;
       }
       G.loadCj(); 

       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;
        int64_t i = Mi[pC] ;          // row index of C(i,j)

        // get C(i,j)
        int64_t k = (Ci [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

        int64_t pA_start = Ap [i] ;
        int64_t pA_end   = Ap [i+1] ;

        int64_t pB_start = Bp [j] ;
        int64_t pB_end   = Bp [j+1] ;
        // NOTE: this test code is NOT doing merge-path. This is just a 
        // single-threaded linear merge for correctness testing.
        k = pA_start;
        int64_t l = pB_start;
        T_Z cij ;
        bool cij_exists = false;
        while( k < pA_end && l < pB_end) {
           if      ( Ai[k] < Bi[l] ) k += 1;
           else if ( Ai[k] > Bi[l] ) l += 1; 
           else {
             if (cij_exists) {
               //std::cout<<" A*B="<< (*MUL_ptr<T_Z>) ( (T_Z)Ax[k] , (T_Z) Bx[l]) <<std::endl ;
               cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)Ax[k] , (T_Z) Bx[l]) ) ;
             } 
             else {
               cij_exists = true; 
               cij = (*MUL_ptr<T_Z>)( (T_Z)Ax[k], (T_Z)Bx[l] ) ;
             }

             k++;
             l++;
           }
           //std::cout<<"Ak = "<< Ax[k]<< " Bl = "<< Bx[l]<< "sum ="<<sum<<std::endl;
        }
        //std::cout<< " dot  = "<< sum << std::endl;

        // output for this dot product is
        
        if (cij_exists) {
            Cx [pC] = (T_C)cij;
            Ci [pC] = i;
        }
        else {
            C->i [pC] = -1;//GB_FLIP (i)
            C->zombie_count++;
        }
    }
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             
             if (Ci[l] > 0) {
                //std::cout<<j<<","<<l <<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
                err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
             }
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count CPU = "<<C->get_zombie_count()<<" zGPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T_C, typename T_M, typename T_A,typename T_B, typename T_X, typename T_Y, typename T_Z>
bool test_AxB_dot3_warp_factory( int TB, int64_t N, int64_t Anz, int64_t Bnz, std::string& SEMI_RING) {
// Assumes all matrices are square so far, so only N dimension given.
// Sparsity is dense here so Anz = Bnz = N*N. 
// Generates three randomized matrices, builds buckets and calls a kernel.


launchFactory<T_C, T_M, T_A, T_B, T_X, T_Z > lF(SEMI_RING, "warp"); 

int testBucket = TB;

//unsigned seed = 13372801;
//std::mt19937 r; //random number generator Mersenne Twister
//r.seed(seed);
//int gpuID; 
//cudaGetDevice( &gpuID);

//std::cout<< "found device "<<gpuID<<std::endl;

//T_Z MONOID_IDENTITY;
if (SEMI_RING == "PLUS_TIMES") {
   std::cout << "Plus Times (+,*) semiring"<<std::endl;
   //MONOID_IDENTITY = 0;
   ADD_ptr<T_Z> = myOP_plus<T_Z>;
   MUL_ptr<T_Z> = myOP_times<T_Z>;

}
else if(SEMI_RING == "MIN_PLUS") {
   std::cout << "Min Plus Times (min,+) semiring"<<std::endl;
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::max();
   ADD_ptr<T_Z> = myOP_min<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;

}
else if(SEMI_RING == "MAX_PLUS") {
   //MONOID_IDENTITY = std::numeric_limits<T_Z>::min();
   std::cout << "Max Plus Times (max,+) semiring"<<std::endl;
   ADD_ptr<T_Z> = myOP_max<T_Z>;
   MUL_ptr<T_Z> = myOP_plus<T_Z>;
}

//Generate test data and setup for using a jitify kernel with 'bucket' interface
SpGEMM_problem_generator<T_C, T_M, T_A, T_B> G;
int64_t Cnz = N; 
float Cnzpercent = (float) Cnz/(N*N);

G.init(N, Anz, Bnz, Cnzpercent);

G.fill_buckets( testBucket); // all elements go to testbucket= TB 

matrix<T_C>* C = G.getCptr();
matrix<T_M>* M = G.getMptr();
matrix<T_A>* A = G.getAptr();
matrix<T_B>* B = G.getBptr();

T_C *Cx = C->x;
T_A *Ax = A->x;
T_B *Bx = B->x;
int64_t *Ci = C->i;
int64_t *Mi = M->i;
int64_t *Ai = A->i;
int64_t *Bi = B->i;
int64_t *Ap = A->p;
int64_t *Bp = B->p;

// Set clear zombie count
C->zombie_count = 0;

//std::cout<<"got all matrices"<<std::endl;
int64_t *Bucket = G.getBucket();
int64_t *BucketStart = G.getBucketStart();

int zc_valid = 0;

bool result = false;

for (int b =0; b < 12; ++b) {// loop on buckets

    int64_t b_start = BucketStart [b] ;
    int64_t b_end   = BucketStart [b+1] ;
    int64_t nvecs = b_end - b_start ;
    if (nvecs > 0) std::cout<< "bucket "<<b<<" has "<<nvecs<<" dots to do"<<std::endl;

    T_C *X_valid  = (T_C*) malloc( Cnz*sizeof(T_C));
    int64_t *i_valid = (int64_t*)malloc( Cnz *sizeof(int64_t));
    if (b == TB) { //test cases for merge-path kernel 
       int nthrd = 32;
       int nblck = (Cnz + nthrd -1)/nthrd ; 
       int sz = 0;
       std::cout<< nblck<< " blocks of "<<nthrd<<" threads, "<<b_start<<","<<b_end<<std::endl;

       GpuTimer kernTimer;
       kernTimer.Start();
       lF.jitGridBlockLaunch( nblck, nthrd, b_start, b_end, Bucket,
                                C, M, A, B, sz);

       kernTimer.Stop();
       std::cout<<"returned from kernel "<<kernTimer.Elapsed()<<"ms"<<std::endl;

       //std::cout<<"returned from kernel"<<std::endl;
     
       zc_valid = C->zombie_count;
       C->zombie_count = 0;
       for (int i =0 ; i< Cnz; ++i) {
            //std::cout<<"Cx[i] = "<<Cx[i]<<std::endl;
            X_valid[i] = Cx[i];
            i_valid[i] = C->i[i]; 
            // clear values for next test
            Cx[i] = 0;
       }
       G.loadCj(); 

       for (int64_t pair = b_start ; pair < b_end ; pair++) {
       
        // get the kth entry in bucket b
        //std::cout<< " pair ="<<pair<<std::endl;
        int64_t pC = (Bucket == nullptr) ? pair : Bucket [pair] ;
        int64_t i = Mi[pC] ;          // row index of C(i,j)

        // get C(i,j)
        int64_t k = (Ci [pC] >> 4) ;    // col index of C(i,j)
        //ASSERT ((C->i [pC] & 4) == b) ;
        int64_t j = (C->h == nullptr) ? k : C->h [k] ; // Mh has been copied into Ch
        //std::cout<<" found dot "<<pair<<" at ("<<i<<","<<j<<")"<<std::endl;

        int64_t pA_start = Ap [i] ;
        int64_t pA_end   = Ap [i+1] ;

        int64_t pB_start = Bp [j] ;
        int64_t pB_end   = Bp [j+1] ;
        // NOTE: this test code is NOT doing merge-path. This is just a 
        // single-threaded linear merge for correctness testing.
        k = pA_start;
        int64_t l = pB_start;
        T_Z cij ;
        bool cij_exists = false;
        while( k < pA_end && l < pB_end) {
           if      ( Ai[k] < Bi[l] ) k += 1;
           else if ( Ai[k] > Bi[l] ) l += 1; 
           else {
             if (cij_exists) {
               //std::cout<<" A*B="<< (*MUL_ptr<T_Z>) ( (T_Z)Ax[k] , (T_Z) Bx[l]) <<std::endl ;
               cij = (*ADD_ptr<T_Z>)( cij, (*MUL_ptr<T_Z>)( (T_Z)Ax[k] , (T_Z) Bx[l]) ) ;
             } 
             else {
               cij_exists = true; 
               cij = (*MUL_ptr<T_Z>)( (T_Z)Ax[k], (T_Z)Bx[l] ) ;
             }

             k++;
             l++;
           }
           //std::cout<<"Ak = "<< Ax[k]<< " Bl = "<< Bx[l]<< "sum ="<<sum<<std::endl;
        }
        //std::cout<< " dot  = "<< sum << std::endl;

        // output for this dot product is
        
        if (cij_exists) {
            Cx [pC] = (T_C)cij;
            Ci [pC] = i;
        }
        else {
            C->i [pC] = -1;//GB_FLIP (i)
            C->zombie_count++;
        }
    }
       T_C err = 0;
       for (int j =0 ; j< N; ++j) {
         for ( int l = C->p[j]; l< C->p[j+1]; ++l) {
             
             if (Ci[l] > 0) {
                //std::cout<<j<<","<<l <<" Cx = "<<Cx[l]<<"x_val="<<X_valid[l]<<std::endl;
                err +=  ( X_valid[l] - Cx[l])*(X_valid[l] - Cx[l]);
             }
         }
       }
       std::cout<< " 2-norm of err ="<< err<<std::endl;
       std::cout<< " zombie count CPU = "<<C->get_zombie_count()<<" zGPU ="<<zc_valid<<std::endl;
      
       EXPECT_EQ(err,0);
       EXPECT_EQ( zc_valid, C->get_zombie_count());

       free(X_valid);
       free(i_valid);
     }
    }

G.del();

return result;

}

template <typename T>
bool test_reducefactoryUM( unsigned int N, std::string OP) {

  reduceFactory<T> rF;

  int block(32);
  int nblock= (N + 8*block -1)/(8*block);
  int grid(nblock);
  T* d_data;
  T* output;

  //std::cout<<" alloc'ing data and output"<<std::endl;
  CHECK_CUDA( cudaMallocManaged((void**) &d_data, nblock*sizeof(T)) );
  CHECK_CUDA( cudaMallocManaged((void**) &output, nblock*sizeof(T)) );
  //std::cout<<" alloc done"<<std::endl;
  //std::cout<<" data fill start"<<std::endl;

  fillvector_linear<T> ( N, d_data);

  //std::cout<<" data fill complete"<<std::endl;
  //we will get a triangular sum = N*(N+1)/2 with this input
  //for (unsigned int i =0; i < N; ++i) d_data[i] = i; 

  //std::cout<< " init data done"<<std::endl;
  //for (unsigned int i =0; i < N; ++i) std::cout<< d_data[i] <<" "; 


  T sum;
  std::cout << "Launching reduce"<<OP<<GET_TYPE_NAME(sum)<<" kernel..."<<std::endl;
  rF.jitGridBlockLaunch( grid, block, d_data, output, N, OP );

  for (int i =0; i< nblock; ++i) std::cout<< output[i] <<" "; 

  if (OP == "PLUS"){
      sum = (T) 0;
      myOpPTR<T> = myOP_plus<T>; 
  }
  if (OP == "MIN") {
      sum = (T)std::numeric_limits<T>::max();
      myOpPTR<T> = myOP_min<T>; 
  }
  if (OP == "MAX") {
      sum = (T)std::numeric_limits<T>::min();
      myOpPTR<T> = myOP_max<T>; 
  } 

  for (int i =0; i< nblock; ++i) sum = (*myOpPTR<T>)(sum ,output[i]); 

  T expect;
  bool result = false;
  if (OP == "PLUS") {
     expect  = (T)(N*(N-1)/2);
     T temp = (sum - expect) ;
     if (temp < 0) temp = -temp ;
     //result = (temp < (T)1) ; //adjust formula for leading 0
     EXPECT_LE(temp, 1);
  }
  else if (OP == "MAX") {
     expect = (T)(N-1);
     //result = (sum)== (T)(N-1) ; //max is N-1 
     EXPECT_EQ( sum , (T)(N-1) );

  }
  else if (OP == "MIN") {
     expect = (T)0;
     //result = (sum)== (T)(0) ;   //min is 0
     EXPECT_EQ( sum , (T)(0) );
  }
  else expect = (T) 0;
  std::cout <<std::endl<<"result of test_reducefactoryUM with "<< OP<< " operation ="<< sum 
            <<" expected "<<expect << std::endl;

  cudaFree(d_data);
  cudaFree(output);
  return result; 
}

template <typename T1,typename T2,typename T3>
bool test_dndotfactoryUM( unsigned int N, std::string SEMI_RING) {

  dotFactory<T1,T2,T3> dF;

  int block(512);
  int nblock= (N + 8*block -1)/(8*block);
  int grid(nblock);
  T1* x;
  T2* y;
  T3* output;
  CHECK_CUDA( cudaMallocManaged((void**)&x, N*sizeof(T1)) );
  CHECK_CUDA( cudaMallocManaged((void**)&y, N*sizeof(T2)) );
  CHECK_CUDA( cudaMallocManaged((void**)&output, nblock*sizeof(T3)) );

  //we will get a triangular sum = N*(N+1)/2 with these inputs
  fillvector_linear<T1> (N, x);
  fillvector_constant<T2> (N, y, T2(1));

  dF.jitGridBlockLaunch( grid, block, x, y, output, N, SEMI_RING );

  T3 sum;
  if (SEMI_RING == "PLUS_TIMES")
  {
      myOpPTR<T3> = myOP_plus<T3>; 
      sum = (T3)0; 
  }
  if (SEMI_RING == "MIN_PLUS")
  { 
      sum = std::numeric_limits<T3>::max();
      myOpPTR<T3> = myOP_min<T3>; 
  }

  for (int i =0; i< nblock; ++i) sum = (*myOpPTR<T3>)(sum ,output[i]); 

  bool result = false;
  T3 expect;
  if (SEMI_RING == "PLUS_TIMES") {
     expect = (T3)(N*(N-1)/2);
     T3 temp = (sum -expect) ;
     if (temp < 0) temp = -temp ;
     //result = (temp < (T3)1) ; //adjust formula for leading 0
     EXPECT_LE( temp, (T3)1 );
  }
  else if (SEMI_RING == "MIN_PLUS") {
     expect = (T3) 1;
     //result = (sum == expect) ;   //min is 1 from the (0,1) pair
     EXPECT_EQ( sum, expect);
  }
  else expect = (T3)0;
  std::cout <<"test_dotfactoryUM with "<<SEMI_RING<<" semi-ring="<< sum 
                                       <<" expected "<<expect << std::endl;

  cudaFree(x);
  cudaFree(y);
  cudaFree(output);
  return result; 
}


template <typename T1,typename T2,typename T3>
bool test_spdotfactoryUM( unsigned int N, unsigned int xn, unsigned int yn, std::string SEMI_RING) {

#define INTMIN( A, B) ( (A) < (B) ) ?  (A) : (B)

  // N here is the index space that the sparse vectors are drawn from.
  // Indices in xi and yi are in the range (0,N-1)
  // We will generate a number of random values in this range for test data
  std::cout<< " xn,yn= "<<xn<<','<<yn<<"min = "<< std::min( xn, yn) <<std::endl;
  int n_threads = std::min( xn, yn) / 4;
  std::cout<< "I think we need "<< n_threads<<" threads to do this."<<std::endl;
  int pad_threads = 2;
  while ( pad_threads < n_threads) {
      pad_threads *= 2;
  }
  int block= 32;
  int nblock= ( pad_threads + block -1)/(block);
  int grid(nblock);
  std::cout<<"N="<<N<<" xn ="<<xn<<", yn="<<yn<<" nblock="<<nblock<<" block ="<<block<<std::endl; 
  unsigned int *xi;
  unsigned int *yi;
  T1* x;
  T2* y;
  T3* output;
  unsigned int intersection_size = 0; //will be filled in later if needed and xn != yn 
  unsigned seed = 13372801;
  std::mt19937 r; //random number generator Mersenne Twister
  r.seed(seed);
  cudaMallocManaged((void**)&x, xn*sizeof(T1));
  cudaMallocManaged((void**)&xi, xn*sizeof(int));
  cudaMallocManaged((void**)&y, yn*sizeof(T2));
  cudaMallocManaged((void**)&yi, yn*sizeof(int));
  cudaMallocManaged((void**)&output, nblock*sizeof(T3));

  int inv_sparsity = N/std::max(xn,yn);  //= values not taken per value occupied in index space
  std::cout<<" Using inv_sparsity value of "<< inv_sparsity<<std::endl;
  fillvector_constant<T1> (xn, x, T1(1));
  fillvector_constant<T2> (yn, y, T2(1));

  if( xn == yn){  // test case : all values intersect, generate 1 random number for both
      intersection_size = xn;
      std::cout << " all-intersect case..."<<std::endl;
      for (unsigned int i =0; i < xn; ++i){  
          unsigned int rand_i = inv_sparsity*i+ r() %(inv_sparsity);
          xi[i] = rand_i; //we will get a count of the intersection size  
          yi[i] = rand_i; //we will get a count of the intersection size  
      }
      //std::sort (xi, xi + xn);
      //std::sort (yi, yi + yn);
  }
  else { // generate two different sets of indices, no known intersection pattern
      for (unsigned int i =0; i < xn; ++i){  
          unsigned int rand_i = inv_sparsity*i +r() % (inv_sparsity);
          xi[i] = rand_i; //we will get a count of the intersection size  
      }
      for (unsigned int i =0; i < yn; ++i){  
          unsigned int rand_i = inv_sparsity*i +r() % (inv_sparsity);
          yi[i] = rand_i; //we will get a count of the intersection size  
      }
      //std::sort (xi, xi + xn);
      //std::sort (yi, yi + yn);
      unsigned int xp =0;
      unsigned int yp =0;
      while (1){  //find the intersection size by merge of two sorted lists
          if (xi[xp] < yi[yp]) xp++;
          else if (xi[xp] > yi[yp]) yp++;
          else {
              intersection_size++;
              xp++;
              yp++;
          }
          if ( ( xp == xn ) || ( yp == yn) )  break;
      }
  }
  if( xn < 128 ) {

    std::cout<< " xi = [";
    for (unsigned int i = 0 ; i < xn; ++i) {
        std::cout<< xi[i] << ",";
    }
    std::cout<< " ]" <<std::endl;
     
  }
  std::cout << " Launching sparseDot CUDA kernel xn = "<<xn<<" yn="<<yn<<std::endl;
  spdotFactory<T1,T2,T3> spdF;
  spdF.jitGridBlockLaunch( grid, block, xn, xi, x, yn, yi, y, output, SEMI_RING );
  
  cudaDeviceSynchronize ( ) ;

  T3 sum;
  if (SEMI_RING == "PLUS_TIMES")
  {
      myOpPTR<T3> = myOP_plus<T3>; 
      sum = (T3)0; 
  }
  if (SEMI_RING == "MIN_PLUS")
  { 
      sum = std::numeric_limits<T3>::max();
      myOpPTR<T3> = myOP_min<T3>; 
  }

  for (int i =0; i< nblock; ++i) sum = (*myOpPTR<T3>)(sum ,output[i]); 

  bool result = false;
  T3 expect;
  if (SEMI_RING == "PLUS_TIMES") {
     T3 temp;
     expect = intersection_size;
     temp = (sum - expect);
     if (temp < 0) temp = -temp ;
     result = (temp < (T3)1) ; //adjust formula for leading 0
  }
  else if (SEMI_RING == "MIN_PLUS") {
     expect = 2;
     result = (sum== expect) ;   //min is 2 from the (1,1) pair
  }
  else expect = (T3) 0;

  std::cout <<"test_spdotfactoryUM with "<<SEMI_RING<<" semi-ring= "
            << sum << " expected "<<intersection_size<< std::endl;
  cudaFree(x);
  cudaFree(xi);
  cudaFree(y);
  cudaFree(yi);
  cudaFree(output);
  return result; 
}
