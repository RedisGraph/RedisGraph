//******************************************************************************
//  Sparse dot products in batch form, sparse - dense case. 
//  Each thread in this kernel is responsible for m vector-pairs(x,y), 
//  m = 256/sz, where sz is in {4, 16, 64, 256}
//  We know each non-zero on the sparse side will hit a dense value.
//  Template on <T_C, T_A, T_B, T_X, T_Y, T_Z >
//  Parameters:

//  int64_t start          <- beginning of bucket  
//  int64_t end            <- end of bucket
//  int64_t *Bucket        <- index of each pair in this bucket
//  matrix<T_C> *C         <- C result matrix 
//  matrix<T_C> *M         <- Mask matrix 
//  matrix<T_A> *A         <- A matrix to multiply, sparse 
//  matrix<T_B> *B         <- B matrix to multiply, dense in sparse format? 
//  int sz                 <- size hint for smaller vector
//******************************************************************************
#include <limits>
#include <cstdint>
#include <stdio.h>
#include "mySemiRing.h"
#include "matrix.h"

template< typename T_C, typename T_A, typename T_B, typename T_X, typename T_Y, typename T_Z>
__global__ void AxB_dot3_phase3_spdn
( 
  int64_t start, 
  int64_t end,
  int64_t *Bucket, 
  GrB_Matrix C, 
  GrB_Matrix M, 
  GrB_Matrix A, 
  GrB_Matrix B,
  int sz 
)
{

   T_A *Ax = (T_A*)A->x;
   T_B *Bx = (T_B*)B->x;
   T_C *Cx = (T_C*)C->x;
   int64_t *Ci = C->i;
   int64_t *Mi = M->i;
   int64_t *Ai = A->i;
   int64_t *Bi = B->i;
   int64_t *Ap = A->p;
   int64_t *Bp = B->p;

   // sz = expected non-zeros per dot 
   int m = 256/sz;
   int nvecs = end - start;
   int dpt = nvecs/32;
   m = dpt < m ? dpt : m;
   //if( threadIdx.x ==0)
   //   printf("thd:%d %d dots/thrd, nvecs = %d blockDim=%d\n",threadIdx.x, sz, nvecs, blockDim.x);
   //__syncthreads();
   int dots = (nvecs +m -1)/m; 
   int zc = 0;
     
   for ( int tid= threadIdx.x +blockDim.x*blockIdx.x;
             tid < dots;
             tid += blockDim.x * gridDim.x) {
      int pair_id, im; 
       //if (threadIdx.x ==0)
       //  printf("thd%u pi=%lld\n",tid, start+threadIdx.x); 
       //  __syncthreads();

      for (pair_id = start+tid, im = 0; 
           im < m && pair_id < end;  
           ++im,     pair_id += dots ){

         int64_t i = Mi[pair_id];
         int64_t j = Ci[pair_id] >> 4;
      //if (threadIdx.x ==0)
      //   printf("thd%u i,j=%lld,%lld\n",tid, i,j); 
      //   __syncthreads();
         
     //  printf("thd%d pi=%d xn=%lld yn=%lld\n",tid, pair_id, 
     //                 A->p[i+1]- A->p[i],
     //                 B->p[j+1]- B->p[j]);

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
            GB_GETA ( aki=(T_Z)Ax[pA+k] ) ;           // aki = A(k,i)
            GB_GETB ( bkj=(T_Z)Bx[pB] ) ;           // bkj = B(k,j)
            GB_C_MULT ( cij, aki, bkj ) ;           // cij = aki * bkj

            for (int64_t p = pB+1 ; p < pB_end ; p++)
            { 
                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
                int64_t k = Bi [p] ;                // next row index of B(:,j)
                // cij += A(k,i) * B(k,j)
                GB_GETA ( aki=(T_Z)Ax[pA+k] ) ;           // aki = A(k,i)
                GB_GETB ( bkj=(T_Z)Bx[p] ) ;           // bkj = B(k,j)
                GB_MULTADD ( cij, aki, bkj ) ;        // cij += aki * bkj
            }

         }
         if( nnzB == B->vlen) // B is dense
         {
            int64_t k = Ai [pA] ;               // first row index of A(:,i)
            // cij = A(k,i) * B(k,j)
            GB_GETA ( aki=(T_Z)Ax[ pA ] ) ;           // aki = A(k,i)
            GB_GETB ( bkj=(T_Z)Bx[ pB+k ] ) ;           // bkj = B(k,j)
            GB_C_MULT ( cij, aki, bkj) ;           // cij = aki * bkj

            for (int64_t p = pA+1 ; p < pA_end ; p++)
            { 
                //GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
                int64_t k = Ai [p] ;                // next row index of A(:,i)
                // cij += A(k,i) * B(k,j)
                GB_GETA ( aki=(T_Z)Ax[ p ] ) ;           // aki = A(k,i)
                GB_GETB ( bkj=(T_Z)Bx[ pB+k] ) ;           // bkj = B(k,j)
                GB_MULTADD ( cij, aki, bkj) ;        // cij += aki * bkj
            }
         }

         GB_PUTC( Ci[pair_id]=i ) ;
         GB_PUTC( Cx[pair_id]=cij ) ;
        
      }
  
   }
   
}
