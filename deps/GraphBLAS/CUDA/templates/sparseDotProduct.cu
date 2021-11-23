//------------------------------------------------------------------------------
// sparseDotProduct_merge_path.cu 
//------------------------------------------------------------------------------

// The sparseDotProduct CUDA kernel produces the semi-ring dot product of two
// sparse vectors of types T1 and T2 and common index space size n, to a scalar 
// odata of type T3. The vectors are sparse, with different numbers of non-zeros.
// ie. we want to produce dot(x,y) in the sense of the given semi-ring.

// This version uses a merge-path algorithm, when the sizes g_xnz and g_ynz are 
// relatively close in size, but for any size of N.

// Both the grid and block are 1D, so blockDim.x is the # threads in a
// threadblock, and the # of threadblocks is grid.x

// Let b = blockIdx.x, and let s be blockDim.x. s= 32 with a variable number
// of active threads = min( min(g_xnz, g_ynz), 32) 

// Thus, threadblock b owns a part of the index set spanned by g_xi and g_yi.  Its job
// is to find the intersection of the index sets g_xi and g_yi, perform the semi-ring dot
// product on those items in the intersection, and finally reduce this data to a scalar, 
// on exit write it to g_odata [b].

#include <limits>
#include <cooperative_groups.h>

using namespace cooperative_groups;

template< typename T, int tile_sz>
__device__ T reduce_sum(thread_block_tile<tile_sz> g, T val)
{
    // Each iteration halves the number of active threads
    // Each thread adds its partial sum[i] to sum[lane+i]
    for (int i = g.size() / 2; i > 0; i /= 2)
    {
        val = ADD( val, g.shfl_down(val,i) );
        //if (g.thread_rank() ==0)
        //    printf("in reduce_sum i=%i val = %f\n", i, val);
    }
    return val; // note: only thread 0 will return full sum
}

#define INTMIN( A, B) ( (A) < (B) ) ?  (A) : (B)
#define INTMAX( A, B) ( (A) > (B) ) ?  (A) : (B)
#define intersects_per_thread 4

template< typename T1, typename T2, typename T3>
__global__ void sparseDotProduct
(
    unsigned int g_xnz,       // Number of non-zeros in x
    unsigned int *g_xi,       // Non-zero indices in x, size xnz
    T1 *g_xdata,              // array of size xnz, type T1
    unsigned int g_ynz,       // Number of non-zeros in y
    unsigned int *g_yi,       // Non-zero indices in y, size ynz
    T2 *g_ydata,              // array of size ynz, type T2
    T3 *g_odata               // array of size grid.x, type T3
)
{
    // set thread ID
    unsigned int tid_global = threadIdx.x+ blockDim.x* blockIdx.x;
    unsigned int tid = threadIdx.x;

    unsigned long int b = blockIdx.x ;

    // total items to be inspected
    unsigned int nxy = (g_xnz + g_ynz);

    //largest possible number of intersections is the smaller nz
    unsigned int n_intersect = INTMIN( g_xnz, g_ynz); 

    //we want more than one intersection per thread
    unsigned int parts = (n_intersect+ intersects_per_thread -1)/ intersects_per_thread; 

    unsigned int work_per_thread = (nxy +parts -1)/parts;
    unsigned int diag = INTMIN( work_per_thread*tid_global, nxy);
    unsigned int diag_end = INTMIN( diag + work_per_thread, nxy);
    //printf(" thd%d parts = %u wpt = %u diag, diag_end  = %u,%u\n",tid, parts, work_per_thread, diag, diag_end); 

   unsigned int x_min = INTMAX( (int)(diag - g_ynz), 0);
   unsigned int x_max = INTMIN( diag, g_xnz);

   //printf("start thd%u x_min = %u x_max = %u\n", tid_global, x_min,x_max);
   while ( x_min < x_max) { //binary search for correct diag break
      unsigned int pivot = (x_min +x_max)/2;
      if ( g_xi[pivot] < g_yi[ diag -pivot -1]) {
         x_min = pivot +1;
      }
      else {
         x_max = pivot;
      }
   }
   int xcoord = x_min;
   int ycoord = diag -x_min -1;
   if (( diag > 0) &&(diag < (g_xnz+g_ynz)) && (g_xi[xcoord] == g_yi[ycoord]) ) { 
       diag--; //adjust for intersection incrementing both pointers 
   }
   // two start points are known now
   int x_start = xcoord;
   int y_start = diag -xcoord; 

   //if (x_start != y_start)
   //   printf("start thd%u  xs,ys = %i,%i\n", tid_global, x_start, y_start);

   x_min = INTMAX( (int)(diag_end - g_ynz), 0);
   x_max = INTMIN( diag_end, g_xnz);

   while ( x_min < x_max) {
      unsigned int pivot = (x_min +x_max)/2;
      //printf("thd%u pre_sw piv=%u diag_e = %u  xmin,xmax=%u,%u\n", tid_global, pivot, diag_end,x_min, x_max);
      if ( g_xi[pivot] < g_yi[ diag_end -pivot -1]) {
         x_min = pivot +1;
      }
      else {
         x_max = pivot;
      }
      //printf("thd%u piv=%u xmin,xmax = %u,%u\n", tid_global, pivot, x_min, x_max);
   }
   xcoord = x_min;
   ycoord = diag_end -x_min -1;
   if ( (diag_end < (g_xnz+g_ynz)) && (g_xi[xcoord] == g_yi[ycoord]) ) { 
       diag--; //adjust for intersection incrementing both pointers  
   }
   // two end points are known now
   int x_end = xcoord; 
   int y_end = diag_end - xcoord; 

   /* 
   if (tid == 0 && b == 0) {
        printf ("type1 is size %d\n", sizeof (T1)) ;
        for (int k = 0 ; k < g_xnz ; k++) printf ("%4d: %g,", k, (T1) g_xdata [k]) ;
        printf ("\n") ;
        printf ("type2 is size %d\n", sizeof (T2)) ;
        for (int k = 0 ; k < g_ynz ; k++) printf ("%4d: %g,", k, (T2) g_ydata [k]) ;
        printf ("\n") ;
    }
    __syncthreads();
    */

    T3 sum = (T3) 0;
    //printf(" thd%u has init value %f\n",tid, sum);

    // nothing to do
    if ( (x_start >= x_end) || (y_start >= y_end) ) { return ; }

    //merge-path dot product
    int k = x_start;
    int l = y_start;
    while ( k < x_end && l < y_end )
    {
       if      ( g_xi[k] < g_yi[l] ) k += 1;
       else if ( g_xi[k] > g_yi[l] ) l += 1; 
       else {
          //printf("  thd%d ix at %u \n",tid_global,g_xi[k]);
          //printf("   sum += %f * %f \n",tid,g_xdata[k],g_ydata[l]);
          //sum = ADD( sum, MUL( g_xdata[k], g_ydata[l]));
          MULADD( sum, g_xdata[k], g_ydata[l]);
          //printf(" thd%u work value = %f\n",tid_global, sum);
          k+= 1;
          l+= 1;
       }

    }

    __syncthreads ( ) ;
    /*
    if (1)
    {
        printf ("thd%u done with intersect and multiply, val = %f\n",tid_global, sum) ;
    }
    __syncthreads ( ) ;
    */

    //--------------------------------------------------------------------------
    // reduce sum per-thread values to a single scalar
    //--------------------------------------------------------------------------
    // Using tile size fixed at compile time, we don't need shared memory
    #define tile_sz 32 
    thread_block_tile<tile_sz> tile = tiled_partition<tile_sz>( this_thread_block());
    T3 block_sum = reduce_sum<T3,tile_sz>(tile, sum);

    // write result for this block to global mem
    if (tid == 0)
    {
        printf ("final %d : %g\n", b,  block_sum) ;
        g_odata [b] = block_sum ;
    }
}

