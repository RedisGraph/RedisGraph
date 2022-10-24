// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cmath>
#include <cstdint>
#include <random>

#include "../type_convert.hpp"
#include "../GB_Matrix_allocate.h"

static const char *_cudaGetErrorEnum(cudaError_t error) {
  return cudaGetErrorName(error);
}

template <typename T>
void check(T result, char const *const func, const char *const file,
           int const line) {
  if (result) {
    fprintf(stderr, "CUDA error at %s:%d code=%d(%s) \"%s\" \n", file, line,
            static_cast<unsigned int>(result), _cudaGetErrorEnum(result), func);
    exit(EXIT_FAILURE);
  }
}

#define checkCudaErrors(val) check((val), #val, __FILE__, __LINE__)

// This will output the proper error string when calling cudaGetLastError
#define getLastCudaError(msg) __getLastCudaError(msg, __FILE__, __LINE__)

inline void __getLastCudaError(const char *errorMessage, const char *file,
                               const int line) {
  cudaError_t err = cudaGetLastError();

  if (cudaSuccess != err) {
    fprintf(stderr,
            "%s(%i) : getLastCudaError() CUDA error :"
            " %s : (%d) %s.\n",
            file, line, errorMessage, static_cast<int>(err),
            cudaGetErrorString(err));
    exit(EXIT_FAILURE);
  }
}

// This will only print the proper error string when calling cudaGetLastError
// but not exit program incase error detected.
#define printLastCudaError(msg) __printLastCudaError(msg, __FILE__, __LINE__)

inline void __printLastCudaError(const char *errorMessage, const char *file,
                                 const int line) {
  cudaError_t err = cudaGetLastError();

  if (cudaSuccess != err) {
    fprintf(stderr,
            "%s(%i) : getLastCudaError() CUDA error :"
            " %s : (%d) %s.\n",
            file, line, errorMessage, static_cast<int>(err),
            cudaGetErrorString(err));
  }
}
#define CHECK_CUDA(call) checkCudaErrors( call )

//Vector generators
template<typename T>
void fillvector_linear( int N, T *vec) {
   for (int i = 0; i< N; ++i) vec[i] = T(i);
}
template<typename T>
void fillvector_constant( int N, T *vec, T val) {
   for (int i = 0; i< N; ++i) vec[i] = val;
}

// Mix-in class to enable unified memory
class Managed {
public:
  void *operator new(size_t len) {
    void *ptr = nullptr;
    //std::cout<<"in new operator, alloc for "<<len<<" bytes"<<std::endl;
    CHECK_CUDA( cudaMallocManaged( &ptr, len) );
    cudaDeviceSynchronize();
    //std::cout<<"in new operator, sync "<<len<<" bytes"<<std::endl;
    return ptr;
  }

  void operator delete(void *ptr) {
    cudaDeviceSynchronize();
    //std::cout<<"in delete operator, free "<<std::endl;
    CHECK_CUDA( cudaFree(ptr) );
  }
};

// FIXME: We should just be able to get rid of this now.
//Basic matrix container class
template<typename T>
class matrix : public Managed {
    int64_t nrows_;
    int64_t ncols_;

  public:
    GrB_Matrix mat;

    matrix(int64_t nrows, int64_t ncols): nrows_(nrows), ncols_(ncols) {}

     GrB_Matrix get_grb_matrix() {
         return mat;
     }

     uint64_t get_zombie_count() { return mat->nzombies;}

     void clear() {
        GrB_Matrix_clear (mat) ;
     }

     void alloc() {
         GrB_Type type = cuda::to_grb_type<T>();

         GrB_Matrix_new (&mat, type, nrows_, ncols_) ;
         // GxB_Matrix_Option_set (mat, GxB_SPARSITY_CONTROL,
            // GxB_SPARSE) ;
            // or:
            // GxB_HYPERSPARSE, GxB_BITMAP, GxB_FULL

//         mat = GB_Matrix_allocate(
//            type,   /// <<<<<<<BUG HERE, was NULL, which is broken
//            sizeof(T), nrows, ncols, 2, false, false, Nz, -1);
     }

     // FIXME: We probably want this to go away
     void fill_random( int64_t nnz, bool debug_print = false) {


         std::cout << "inside fill" << std::endl;
         alloc();

        int64_t inv_sparsity = (nrows_*ncols_)/nnz;   //= values not taken per value occupied in index space

        std::cout<< "fill_random nrows="<< nrows_<<"ncols=" << ncols_ <<" need "<< nnz<<" values, invsparse = "<<inv_sparsity<<std::endl;
        std::cout<< "fill_random"<<" after alloc values"<<std::endl;
        std::cout<<"vdim ready "<<std::endl;
        std::cout<<"vlen ready "<<std::endl;
        std::cout<<"ready to fill p"<<std::endl;

        bool make_symmetric = false;
        bool no_self_edges = false;

        std::random_device rd;
        std::mt19937 r(rd());
        std::uniform_real_distribution<double> dis(0.0, 1.0);
         for (int64_t k = 0 ; k < nnz ; k++)
         {
             GrB_Index i = ((GrB_Index) (dis(r) * nrows_)) % ((GrB_Index) nrows_) ;
             GrB_Index j = ((GrB_Index) (dis(r) * ncols_)) % ((GrB_Index) ncols_) ;
             if (no_self_edges && (i == j)) continue ;
             double x = dis(r) ;
             // A (i,j) = x
             cuda::set_element<T> (mat, x, i, j) ;
             if (make_symmetric)
             {
                 // A (j,i) = x
                 cuda::set_element<T>(mat, x, j, i) ;
             }
         }

        GrB_Matrix_wait (mat, GrB_MATERIALIZE) ;
        // TODO: Need to specify these
        GxB_Matrix_Option_set (mat, GxB_FORMAT, GxB_BY_ROW) ;
        GxB_Matrix_Option_set (mat, GxB_SPARSITY_CONTROL, GxB_SPARSE) ;
        GrB_Matrix_nvals ((GrB_Index *) &nnz, mat) ;
        GxB_Matrix_fprint (mat, "my mat", GxB_SHORT_VERBOSE, stdout) ;
    
        printf("a_vector = [");
        for (int p = 0;  p < nnz; p++) {
            printf("%ld, ", mat->i [p]);
            if (p > 100) { printf ("...\n") ; break ; }
        }
        printf("]\n");


     }
};



template< typename T_C, typename T_M, typename T_A, typename T_B>
class SpGEMM_problem_generator {

    float Anzpercent,Bnzpercent,Cnzpercent;
    int64_t Cnz;
    int64_t *Bucket = nullptr;

    int64_t BucketStart[13];
    unsigned seed = 13372801;
    bool ready = false;

    int64_t nrows_;
    int64_t ncols_;

  public:

    matrix<T_C> *C= nullptr;
    matrix<T_M> *M= nullptr;
    matrix<T_A> *A= nullptr;
    matrix<T_B> *B= nullptr;

    SpGEMM_problem_generator(int64_t nrows, int64_t ncols): nrows_(nrows), ncols_(ncols) {
    
       // Create sparse matrices
       C = new matrix<T_C>(nrows_, ncols_);
       M = new matrix<T_M>(nrows_, ncols_);
       A = new matrix<T_A>(nrows_, ncols_);
       B = new matrix<T_B>(nrows_, ncols_);
    };

    matrix<T_C>* getCptr(){ return C;}
    matrix<T_M>* getMptr(){ return M;}
    matrix<T_A>* getAptr(){ return A;}
    matrix<T_B>* getBptr(){ return B;}

    int64_t* getBucket() { return Bucket;}
    int64_t* getBucketStart(){ return BucketStart;}

    void loadCj() {

       // Load C_i with column j info to avoid another lookup
       for (int c = 0 ; c< M->mat->vdim; ++c) {
           for ( int r = M->mat->p[c]; r< M->mat->p[c+1]; ++r){
               C->mat->i[r] = c << 4 ; //shift to store bucket info
           }
       }

    }

    void init(int64_t Anz, int64_t Bnz, float Cnzpercent){

       // Get sizes relative to fully dense matrices
       Anzpercent = float(Anz)/float(nrows_*ncols_);
       Bnzpercent = float(Bnz)/float(nrows_*ncols_);
       Cnzpercent = Cnzpercent;
       Cnz = (int64_t)(Cnzpercent * nrows_ * ncols_);
       std::cout<<"Anz% ="<<Anzpercent<<" Bnz% ="<<Bnzpercent<<" Cnz% ="<<Cnzpercent<<std::endl;

       //Seed the generator
       std::cout<<"filling matrices"<<std::endl;

       C->fill_random(Cnz);
       M->fill_random(Cnz);
       A->fill_random(Anz);
       B->fill_random(Bnz);


       std::cout<<"fill complete"<<std::endl;
       C->mat->p = M->mat->p; //same column pointers (assuming CSC here)
       C->mat->p_shallow = true ; // C->mat does not own M->mat->p

       loadCj();

    }

    void del(){
       C->clear();
       M->clear();
       A->clear();
       B->clear();
       if (Bucket != nullptr) CHECK_CUDA( cudaFree(Bucket) );
       delete C;
       delete M;
       delete A;
       delete B;
       CHECK_CUDA( cudaDeviceSynchronize() );
    }

    void fill_buckets( int fill_bucket){

       std::cout<<Cnz<<" slots to fill"<<std::endl;

       if (fill_bucket == -1){  

       // Allocate Bucket space
       CHECK_CUDA( cudaMallocManaged((void**)&Bucket, Cnz*sizeof(int64_t)) );

       //Fill buckets with random extents such that they sum to Cnz, set BucketStart
           BucketStart[0] = 0; 
           BucketStart[12] = Cnz;
           for (int b = 1; b < 12; ++b){
              BucketStart[b] = BucketStart[b-1] + (Cnz / 12);  
              //std::cout<< "bucket "<< b<<" starts at "<<BucketStart[b]<<std::endl;
              for (int j = BucketStart[b-1]; j < BucketStart[b]; ++j) { 
                Bucket[j] = b ; 
              }
           }
           int b = 11;
           for (int j = BucketStart[11]; j < BucketStart[12]; ++j) { 
                Bucket[j] = b ; 
           }
       }
       else {// all in one test bucket
           Bucket = nullptr;
           BucketStart[0] = 0; 
           BucketStart[12] = Cnz;
           for (int b= 0; b<12; ++b){
              if (b <= fill_bucket) BucketStart[b] = 0;
              if (b  > fill_bucket) BucketStart[b] = Cnz;
              //std::cout<< " one  bucket "<< b<<"starts at "<<BucketStart[b]<<std::endl;
           } 
           std::cout<<"all pairs to bucket "<<fill_bucket<<", no filling"<<std::endl;
           std::cout<<"done assigning buckets"<<std::endl;
       }
    }
};


