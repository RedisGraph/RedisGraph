 /* Copyright 1993-2016 NVIDIA Corporation.  All rights reserved.
  *
  * NOTICE TO LICENSEE:
  *
  * The source code and/or documentation ("Licensed Deliverables") are
  * subject to NVIDIA intellectual property rights under U.S. and
  * international Copyright laws.
  *
  * The Licensed Deliverables contained herein are PROPRIETARY and
  * CONFIDENTIAL to NVIDIA and are being provided under the terms and
  * conditions of a form of NVIDIA software license agreement by and
  * between NVIDIA and Licensee ("License Agreement") or electronically
  * accepted by Licensee.  Notwithstanding any terms or conditions to
  * the contrary in the License Agreement, reproduction or disclosure
  * of the Licensed Deliverables to any third party without the express
  * written consent of NVIDIA is prohibited.
  *
  * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
  * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
  * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  THEY ARE
  * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
  * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
  * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
  * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
  * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
  * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
  * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
  * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
  * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
  * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
  * OF THESE LICENSED DELIVERABLES.
  *
  * U.S. Government End Users.  These Licensed Deliverables are a
  * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
  * 1995), consisting of "commercial computer software" and "commercial
  * computer software documentation" as such terms are used in 48
  * C.F.R. 12.212 (SEPT 1995) and are provided to the U.S. Government
  * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
  * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
  * U.S. Government End Users acquire the Licensed Deliverables with
  * only those rights set forth herein.
  *
  * Any use of the Licensed Deliverables in individual and commercial
  * software must include, in the user documentation and internal
  * comments to the code, the above Disclaimer and U.S. Government End
  * Users Notice.
  */

/*
** Define: _CG_VERSION
*/
# define _CG_VERSION 1000

/*
** Define: _CG_ABI_VERSION
*/
# ifndef _CG_ABI_VERSION
#  define _CG_ABI_VERSION 1
# endif

/*
** Define: _CG_ABI_EXPERIMENTAL
** Desc: If enabled, sets all features enabled (ABI-breaking or experimental)
*/
# if defined(_CG_ABI_EXPERIMENTAL)
# endif

# define _CG_CONCAT_INNER(x, y) x ## y
# define _CG_CONCAT_OUTER(x, y) _CG_CONCAT_INNER(x, y)
# define _CG_NAMESPACE _CG_CONCAT_OUTER(__v, _CG_ABI_VERSION)

# define _CG_BEGIN_NAMESPACE \
    namespace cooperative_groups { namespace _CG_NAMESPACE {
# define _CG_END_NAMESPACE \
    }; using namespace _CG_NAMESPACE; };

# if !defined(_CG_STATIC_QUALIFIER)
#  define _CG_STATIC_QUALIFIER static __forceinline__ __device__
# endif
# if !defined(_CG_QUALIFIER)
#  define _CG_QUALIFIER __forceinline__ __device__
# endif

# if (__CUDA_ARCH__ >= 600) || !defined(__CUDA_ARCH__)
#  define _CG_HAS_GRID_GROUP
# endif
# if (__CUDA_ARCH__ >= 600) || !defined(__CUDA_ARCH__)
#  define _CG_HAS_MULTI_GRID_GROUP
# endif
# if (__CUDA_ARCH__ >= 700) || !defined(__CUDA_ARCH__)
#  define _CG_HAS_MATCH_COLLECTIVE
# endif
// Has __half and __half2
// Only usable if you include the cuda_fp16.h extension, and
// _before_ including cooperative_groups.h
# ifdef __CUDA_FP16_TYPES_EXIST__
#  define _CG_HAS_FP16_COLLECTIVE
# endif

/*
** Define: CG_DEBUG
** What: Enables various runtime safety checks
*/
#if defined(__CUDACC_DEBUG__) && !defined(_CG_DEBUG)
# define _CG_DEBUG 1
#endif

#if defined(_CG_DEBUG) && (_CG_DEBUG == 1) && !defined(NDEBUG)
# include <assert.h>
# define _CG_ASSERT(x) assert((x));
# define _CG_ABORT() assert(0);
#else
# define _CG_ASSERT(x)
# define _CG_ABORT() __trap();
#endif

_CG_BEGIN_NAMESPACE

namespace __internal {

    enum groupType {
        CoalescedTile,
        Coalesced,
        ThreadBlock,
        Grid,
        MultiGrid,
    };

#if defined(_CG_HAS_GRID_GROUP)

    namespace grid {

        _CG_STATIC_QUALIFIER unsigned long long get_intrinsic_handle()
        {
            return (cudaCGGetIntrinsicHandle(cudaCGScopeGrid));
        }

        _CG_STATIC_QUALIFIER void sync(const unsigned long long handle)
        {
            cudaCGSynchronizeGrid(handle, 0);
        }

        _CG_STATIC_QUALIFIER unsigned int size(const unsigned long long handle)
        {
            return (blockDim.z * gridDim.z) *
                (blockDim.y * gridDim.y) *
                (blockDim.x * gridDim.x);
        }

        _CG_STATIC_QUALIFIER unsigned int thread_rank(const unsigned long long handle)
        {
            unsigned int blkIdx = ((blockIdx.z * gridDim.y * gridDim.x) +
                               (blockIdx.y * gridDim.x) +
                               blockIdx.x);
            return (blkIdx * (blockDim.x * blockDim.y * blockDim.z) +
                    ((threadIdx.z * blockDim.y * blockDim.x) +
                     (threadIdx.y * blockDim.x) +
                     threadIdx.x));
        }

        _CG_STATIC_QUALIFIER dim3 grid_dim()
        {
            return (dim3(gridDim.x, gridDim.y, gridDim.z));
        }
    };

#endif

#if defined(_CG_HAS_MULTI_GRID_GROUP)

    namespace multi_grid {

        _CG_STATIC_QUALIFIER unsigned long long get_intrinsic_handle()
        {
            return (cudaCGGetIntrinsicHandle(cudaCGScopeMultiGrid));
        }

        _CG_STATIC_QUALIFIER void sync(const unsigned long long handle)
        {
            cudaError_t err = cudaCGSynchronize(handle, 0);
        }

        _CG_STATIC_QUALIFIER unsigned int size(const unsigned long long handle)
        {
            unsigned int numThreads = 0;
            cudaCGGetSize(&numThreads, NULL, handle);
            return numThreads;
        }

        _CG_STATIC_QUALIFIER unsigned int thread_rank(const unsigned long long handle)
        {
            unsigned int threadRank = 0;
            cudaCGGetRank(&threadRank, NULL, handle);
            return threadRank;
        }

        _CG_STATIC_QUALIFIER unsigned int grid_rank(const unsigned long long handle)
        {
            unsigned int gridRank = 0;
            cudaCGGetRank(NULL, &gridRank, handle);
            return gridRank;
        }

        _CG_STATIC_QUALIFIER unsigned int num_grids(const unsigned long long handle)
        {
            unsigned int numGrids = 0;
            cudaCGGetSize(NULL, &numGrids, handle);
            return numGrids;
        }

    };

#endif

    namespace cta {

        _CG_STATIC_QUALIFIER void sync()
        {
            __barrier_sync(0);
        }

        _CG_STATIC_QUALIFIER unsigned int size()
        {
            return (blockDim.x * blockDim.y * blockDim.z);
        }

        _CG_STATIC_QUALIFIER unsigned int thread_rank()
        {
            return ((threadIdx.z * blockDim.y * blockDim.x) +
                    (threadIdx.y * blockDim.x) +
                    threadIdx.x);
        }

        _CG_STATIC_QUALIFIER dim3 group_index()
        {
            return (dim3(blockIdx.x, blockIdx.y, blockIdx.z));
        }

        _CG_STATIC_QUALIFIER dim3 thread_index()
        {
            return (dim3(threadIdx.x, threadIdx.y, threadIdx.z));
        }

        _CG_STATIC_QUALIFIER dim3 block_dim()
        {
            return (dim3(blockDim.x, blockDim.y, blockDim.z));
        }

    };

    _CG_STATIC_QUALIFIER unsigned int laneid()
    {
        unsigned int laneid;
        asm volatile("mov.u32 %0, %%laneid;" : "=r"(laneid));
        return laneid;
    }

    _CG_STATIC_QUALIFIER unsigned int warpsz()
    {
        unsigned int warpSize;
        asm volatile("mov.u32 %0, WARP_SZ;" : "=r"(warpSize));
        return warpSize;
    }

    _CG_STATIC_QUALIFIER unsigned int lanemask32_eq()
    {
        unsigned int lanemask32_eq;
        asm volatile("mov.u32 %0, %%lanemask_eq;" : "=r"(lanemask32_eq));
        return (lanemask32_eq);
    }

    _CG_STATIC_QUALIFIER unsigned int lanemask32_lt()
    {
        unsigned int lanemask32_lt;
        asm volatile("mov.u32 %0, %%lanemask_lt;" : "=r"(lanemask32_lt));
        return (lanemask32_lt);
    }

    _CG_STATIC_QUALIFIER void abort()
    {
        _CG_ABORT();
    }

}; // !Namespace internal

_CG_END_NAMESPACE
