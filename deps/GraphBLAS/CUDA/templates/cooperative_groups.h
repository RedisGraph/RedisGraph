/*
 * Copyright 1993-2016 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
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
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
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

#ifndef _COOPERATIVE_GROUPS_H_
# define _COOPERATIVE_GROUPS_H_

#if defined(__cplusplus) && defined(__CUDACC__)

# include "cooperative_groups_helpers.h"

_CG_BEGIN_NAMESPACE

/**
 * class thread_group;
 *
 * Generic thread group type, into which all groups are convertible.
 * It acts as a container for all storage necessary for the derived groups,
 * and will dispatch the API calls to the correct derived group. This means
 * that all derived groups must implement the same interface as thread_group.
 */
class thread_group
{
    friend _CG_QUALIFIER thread_group this_thread();
    friend _CG_QUALIFIER thread_group tiled_partition(const thread_group& parent, unsigned int tilesz);
    friend class thread_block;

 protected:
    union __align__(8) {
        unsigned int type : 8;
        struct {
            unsigned int type : 8;
            unsigned int size : 24;
            unsigned int mask;
        } coalesced;
        struct {
            void* ptr[2];
        } buffer;
    } _data;

    _CG_QUALIFIER thread_group operator=(const thread_group& src);
    _CG_QUALIFIER thread_group(__internal::groupType type) {
        _data.type = type;
    }

#if __cplusplus >= 201103L
    static_assert(sizeof(_data) == 16, "Failed size check");
#endif

public:
    _CG_QUALIFIER unsigned int size() const;
    _CG_QUALIFIER unsigned int thread_rank() const;
    _CG_QUALIFIER void sync() const;
};

/**
 * thread_group this_thread()
 *
 * Constructs a generic thread_group containing only the calling thread
 */
_CG_QUALIFIER thread_group this_thread()
{
    thread_group g = thread_group(__internal::Coalesced);
    g._data.coalesced.mask = __internal::lanemask32_eq();
    g._data.coalesced.size = 1;
    return (g);
}

#if defined(_CG_HAS_MULTI_GRID_GROUP)

/**
 * class multi_grid_group;
 *
 * Threads within this this group are guaranteed to be co-resident on the
 * same system, on multiple devices within the same launched kernels.
 * To use this group, the kernel must have been launched with
 * cuLaunchCooperativeKernelMultiDevice (or the CUDA Runtime equivalent),
 * and the device must support it (queryable device attribute).
 *
 * Constructed via this_multi_grid();
 */
class multi_grid_group
{
    friend _CG_QUALIFIER multi_grid_group this_multi_grid();

    struct __align__(8) {
        unsigned long long handle;
        unsigned int size;
        unsigned int rank;
    } _data;

#if __cplusplus >= 201103L
    static_assert(sizeof(_data) == 16, "Failed size check");
#endif

public:
    _CG_QUALIFIER multi_grid_group() {
        _data.handle = __internal::multi_grid::get_intrinsic_handle();
        _data.size = __internal::multi_grid::size(_data.handle);
        _data.rank = __internal::multi_grid::thread_rank(_data.handle);
    }

    _CG_QUALIFIER bool is_valid() const {
        return (_data.handle != 0);
    }

    _CG_QUALIFIER void sync() const {
        _CG_ASSERT(is_valid());
        __internal::multi_grid::sync(_data.handle);
    }

    _CG_QUALIFIER unsigned int size() const {
        _CG_ASSERT(is_valid());
        return (_data.size);
    }

    _CG_QUALIFIER unsigned int thread_rank() const {
        _CG_ASSERT(is_valid());
        return (_data.rank);
    }

    _CG_QUALIFIER unsigned int grid_rank() const {
        _CG_ASSERT(is_valid());
        return (__internal::multi_grid::grid_rank(_data.handle));
    }

    _CG_QUALIFIER unsigned int num_grids() const {
        _CG_ASSERT(is_valid());
        return (__internal::multi_grid::num_grids(_data.handle));
    }
};

/**
 * multi_grid_group this_multi_grid()
 *
 * Constructs a multi_grid_group
 */
_CG_QUALIFIER multi_grid_group this_multi_grid()
{
    return (multi_grid_group());
}

#endif

#if defined(_CG_HAS_GRID_GROUP)

/**
 * class grid_group;
 *
 * Threads within this this group are guaranteed to be co-resident on the
 * same device within the same launched kernel. To use this group, the kernel
 * must have been launched with cuLaunchCooperativeKernel (or the CUDA Runtime equivalent),
 * and the device must support it (queryable device attribute).
 *
 * Constructed via this_grid();
 */
class grid_group
{
    friend _CG_QUALIFIER grid_group this_grid();

    struct __align__(8) {
        unsigned long long handle;
        unsigned int size;
        unsigned int rank;
    } _data;

#if __cplusplus >= 201103L
    static_assert(sizeof(_data) == 16, "Failed size check");
#endif

 public:
    _CG_QUALIFIER grid_group() {
        _data.handle = (__internal::grid::get_intrinsic_handle());
        _data.size = __internal::grid::size(_data.handle);
        _data.rank = __internal::grid::thread_rank(_data.handle);
    }

    _CG_QUALIFIER bool is_valid() const {
        return (_data.handle != 0);
    }

    _CG_QUALIFIER void sync() const {
        _CG_ASSERT(is_valid());
        __internal::grid::sync(_data.handle);
    }

    _CG_QUALIFIER unsigned int size() const {
        _CG_ASSERT(is_valid());
        return (_data.size);
    }

    _CG_QUALIFIER unsigned int thread_rank() const {
        _CG_ASSERT(is_valid());
        return (_data.rank);
    }

    _CG_QUALIFIER dim3 group_dim() const {
        _CG_ASSERT(is_valid());
        return (__internal::grid::grid_dim());
    }

};

/**
 * grid_group this_grid()
 *
 * Constructs a grid_group
 */
_CG_QUALIFIER grid_group this_grid()
{
    return (grid_group());
}

#endif

/**
 * class thread_block
 *
 * Every GPU kernel is executed by a grid of thread blocks, and threads within
 * each block are guaranteed to reside on the same streaming multiprocessor.
 * A thread_block represents a thread block whose dimensions are not known until runtime.
 *
 * Constructed via this_thread_block();
 */
class thread_block : public thread_group
{
    friend _CG_QUALIFIER thread_block this_thread_block();
    friend _CG_QUALIFIER thread_group tiled_partition(const thread_group& parent, unsigned int tilesz);
    friend _CG_QUALIFIER thread_group tiled_partition(const thread_block& parent, unsigned int tilesz);

    _CG_QUALIFIER thread_block() : thread_group(__internal::ThreadBlock) {
    }

    // Internal Use
    _CG_QUALIFIER thread_group _get_tiled_threads(unsigned int tilesz) const {
        const bool pow2_tilesz = ((tilesz & (tilesz - 1)) == 0);

        // Invalid, immediately fail
        if (tilesz == 0 || (tilesz > 32) || !pow2_tilesz) {
            __internal::abort();
            return (thread_block());
        }

        unsigned int mask;
        unsigned int base_offset = thread_rank() & (~(tilesz - 1));
        unsigned int masklength = min(size() - base_offset, tilesz);

        mask = (unsigned int)(-1) >> (32 - masklength);
        mask <<= (__internal::laneid() & ~(tilesz - 1));
        thread_group tile = thread_group(__internal::CoalescedTile);
        tile._data.coalesced.mask = mask;
        tile._data.coalesced.size = __popc(mask);
        return (tile);
    }

 public:
    _CG_QUALIFIER void sync() const {
        __internal::cta::sync();
    }

    _CG_QUALIFIER unsigned int size() const {
        return (__internal::cta::size());
    }

    _CG_QUALIFIER unsigned int thread_rank() const {
        return (__internal::cta::thread_rank());
    }

    // Additional functionality exposed by the group
    _CG_QUALIFIER dim3 group_index() const {
        return (__internal::cta::group_index());
    }

    _CG_QUALIFIER dim3 thread_index() const {
        return (__internal::cta::thread_index());
    }

    _CG_QUALIFIER dim3 group_dim() const {
        return (__internal::cta::block_dim());
    }

};

/**
 * thread_block this_thread_block()
 *
 * Constructs a thread_block group
 */
_CG_QUALIFIER thread_block this_thread_block()
{
    return (thread_block());
}

/**
 * class coalesced_group
 *
 * A group representing the current set of converged threads in a warp.
 * The size of the group is not guaranteed and it may return a group of
 * only one thread (itself).
 *
 * This group exposes warp-synchronous builtins.
 * Constructed via coalesced_threads();
 */
class coalesced_group : public thread_group
{
    friend _CG_QUALIFIER coalesced_group coalesced_threads();
    friend _CG_QUALIFIER thread_group tiled_partition(const thread_group& parent, unsigned int tilesz);
    friend _CG_QUALIFIER coalesced_group tiled_partition(const coalesced_group& parent, unsigned int tilesz);

    _CG_QUALIFIER unsigned int _packLanes(unsigned laneMask) const {
        unsigned int member_pack = 0;
        unsigned int member_rank = 0;
        for (int bit_idx = 0; bit_idx < 32; bit_idx++) {
            unsigned int lane_bit = _data.coalesced.mask & (1 << bit_idx);
            if (lane_bit) {
                if (laneMask & lane_bit)
                    member_pack |= 1 << member_rank;
                member_rank++;
            }
        }
        return (member_pack);
    }

    // Internal Use
    _CG_QUALIFIER coalesced_group _get_tiled_threads(unsigned int tilesz) const {
        const bool pow2_tilesz = ((tilesz & (tilesz - 1)) == 0);

        // Invalid, immediately fail
        if (tilesz == 0 || (tilesz > 32) || !pow2_tilesz) {
            __internal::abort();
            return (coalesced_group(0));
        }
        if (size() <= tilesz) {
            return (*this);
        }

        if ((_data.type == __internal::CoalescedTile) && pow2_tilesz) {
            unsigned int base_offset = (thread_rank() & (~(tilesz - 1)));
            unsigned int masklength = min(size() - base_offset, tilesz);
            unsigned int mask = (unsigned int)(-1) >> (32 - masklength);

            mask <<= (__internal::laneid() & ~(tilesz - 1));
            coalesced_group coalesced_tile = coalesced_group(mask);
            coalesced_tile._data.type = __internal::CoalescedTile;
            return (coalesced_tile);
        }
        else if ((_data.type == __internal::Coalesced) && pow2_tilesz) {
            unsigned int mask = 0;
            unsigned int member_rank = 0;
            int seen_lanes = (thread_rank() / tilesz) * tilesz;
            for (unsigned int bit_idx = 0; bit_idx < 32; bit_idx++) {
                unsigned int lane_bit = _data.coalesced.mask & (1 << bit_idx);
                if (lane_bit) {
                    if (seen_lanes <= 0 && member_rank < tilesz) {
                        mask |= lane_bit;
                        member_rank++;
                    }
                    seen_lanes--;
                }
            }
            return (coalesced_group(mask));
        }
        else {
            // None in _CG_VERSION 1000
            __internal::abort();
        }

        return (coalesced_group(0));
    }

 protected:
    // Construct a group from scratch (coalesced_threads)
    _CG_QUALIFIER coalesced_group(unsigned int mask) : thread_group(__internal::Coalesced) {
        _data.coalesced.mask = mask;
        _data.coalesced.size = __popc(mask);
    }

 public:
    _CG_QUALIFIER unsigned int size() const {
        return (_data.coalesced.size);
    }
    _CG_QUALIFIER unsigned int thread_rank() const {
        return (__popc(_data.coalesced.mask & __internal::lanemask32_lt()));
    }
    _CG_QUALIFIER void sync() const {
        __syncwarp(_data.coalesced.mask);
    }

#define COALESCED_SHFL_FUNCTION(type)                                   \
    _CG_QUALIFIER type shfl(type var, unsigned int src_rank) const {    \
        unsigned int lane = (src_rank == 0) ? __ffs(_data.coalesced.mask) - 1 : \
            (size() == 32) ? src_rank : __fns(_data.coalesced.mask, 0, (src_rank + 1)); \
        return (__shfl_sync(_data.coalesced.mask, var, lane, 32));      \
    }

#define COALESCED_SHFL_UP_FUNCTION(type)                                \
    _CG_QUALIFIER type shfl_up(type var, int delta) const {             \
        if (size() == 32) {                                             \
            return (__shfl_up_sync(0xFFFFFFFF, var, delta, 32));        \
        }                                                               \
        unsigned lane = __fns(_data.coalesced.mask, __internal::laneid(), -(delta + 1)); \
        if (lane >= 32) lane = __internal::laneid();                    \
        return (__shfl_sync(_data.coalesced.mask, var, lane, 32));      \
    }

#define COALESCED_SHFL_DOWN_FUNCTION(type)                              \
    _CG_QUALIFIER type shfl_down(type var, int delta) const {           \
        if (size() == 32) {                                             \
            return (__shfl_down_sync(0xFFFFFFFF, var, delta, 32));      \
        }                                                               \
        unsigned int lane = __fns(_data.coalesced.mask, __internal::laneid(), delta + 1); \
        if (lane >= 32) lane = __internal::laneid();                    \
        return (__shfl_sync(_data.coalesced.mask, var, lane, 32));      \
    }

    COALESCED_SHFL_FUNCTION(int);
    COALESCED_SHFL_FUNCTION(unsigned int);
    COALESCED_SHFL_FUNCTION(long);
    COALESCED_SHFL_FUNCTION(unsigned long);
    COALESCED_SHFL_FUNCTION(long long);
    COALESCED_SHFL_FUNCTION(unsigned long long);
    COALESCED_SHFL_FUNCTION(float);
    COALESCED_SHFL_FUNCTION(double);

    COALESCED_SHFL_UP_FUNCTION(int);
    COALESCED_SHFL_UP_FUNCTION(unsigned int);
    COALESCED_SHFL_UP_FUNCTION(long);
    COALESCED_SHFL_UP_FUNCTION(unsigned long);
    COALESCED_SHFL_UP_FUNCTION(long long);
    COALESCED_SHFL_UP_FUNCTION(unsigned long long);
    COALESCED_SHFL_UP_FUNCTION(float);
    COALESCED_SHFL_UP_FUNCTION(double);

    COALESCED_SHFL_DOWN_FUNCTION(int);
    COALESCED_SHFL_DOWN_FUNCTION(unsigned int);
    COALESCED_SHFL_DOWN_FUNCTION(long);
    COALESCED_SHFL_DOWN_FUNCTION(unsigned long);
    COALESCED_SHFL_DOWN_FUNCTION(long long);
    COALESCED_SHFL_DOWN_FUNCTION(unsigned long long);
    COALESCED_SHFL_DOWN_FUNCTION(float);
    COALESCED_SHFL_DOWN_FUNCTION(double);

# ifdef _CG_HAS_FP16_COLLECTIVE
    COALESCED_SHFL_FUNCTION(__half);
    COALESCED_SHFL_UP_FUNCTION(__half);
    COALESCED_SHFL_DOWN_FUNCTION(__half);

    COALESCED_SHFL_FUNCTION(__half2);
    COALESCED_SHFL_UP_FUNCTION(__half2);
    COALESCED_SHFL_DOWN_FUNCTION(__half2);
# endif

#undef COALESCED_SHFL_FUNCTION
#undef COALESCED_SHFL_UP_FUNCTION
#undef COALESCED_SHFL_DOWN_FUNCTION

    _CG_QUALIFIER int any(int predicate) const {
        return (__ballot_sync(_data.coalesced.mask, predicate) != 0);
    }
    _CG_QUALIFIER int all(int predicate) const {
        return (__ballot_sync(_data.coalesced.mask, predicate) == _data.coalesced.mask);
    }
    _CG_QUALIFIER unsigned int ballot(int predicate) const {
        if (size() == 32) {
            return (__ballot_sync(0xFFFFFFFF, predicate));
        }
        unsigned int lane_ballot = __ballot_sync(_data.coalesced.mask, predicate);
        return (_packLanes(lane_ballot));
    }

#ifdef _CG_HAS_MATCH_COLLECTIVE

# define COALESCED_MATCH_ANY_FUNCTION(type)                             \
    _CG_QUALIFIER unsigned int match_any(type val) const {              \
        if (size() == 32) {                                             \
            return (__match_any_sync(0xFFFFFFFF, val));                 \
        }                                                               \
        unsigned int lane_match = __match_any_sync(_data.coalesced.mask, val); \
        return (_packLanes(lane_match));                                \
    }
# define COALESCED_MATCH_ALL_FUNCTION(type)                             \
    _CG_QUALIFIER unsigned int match_all(type val, int &pred) const {   \
        if (size() == 32) {                                             \
            return (__match_all_sync(0xFFFFFFFF, val, &pred));          \
        }                                                               \
        unsigned int lane_match = __match_all_sync(_data.coalesced.mask, val, &pred); \
        return (_packLanes(lane_match));                                \
    }

    COALESCED_MATCH_ANY_FUNCTION(int);
    COALESCED_MATCH_ANY_FUNCTION(unsigned int);
    COALESCED_MATCH_ANY_FUNCTION(long);
    COALESCED_MATCH_ANY_FUNCTION(unsigned long);
    COALESCED_MATCH_ANY_FUNCTION(long long);
    COALESCED_MATCH_ANY_FUNCTION(unsigned long long);
    COALESCED_MATCH_ANY_FUNCTION(float);
    COALESCED_MATCH_ANY_FUNCTION(double);

    COALESCED_MATCH_ALL_FUNCTION(int);
    COALESCED_MATCH_ALL_FUNCTION(unsigned int);
    COALESCED_MATCH_ALL_FUNCTION(long);
    COALESCED_MATCH_ALL_FUNCTION(unsigned long);
    COALESCED_MATCH_ALL_FUNCTION(long long);
    COALESCED_MATCH_ALL_FUNCTION(unsigned long long);
    COALESCED_MATCH_ALL_FUNCTION(float);
    COALESCED_MATCH_ALL_FUNCTION(double);

# undef COALESCED_MATCH_ANY_FUNCTION
# undef COALESCED_MATCH_ALL_FUNCTION

#endif /* !_CG_HAS_MATCH_COLLECTIVE */

};

_CG_QUALIFIER coalesced_group coalesced_threads()
{
    return (coalesced_group(__activemask()));
}

template <unsigned int Size>
class __thread_block_tile_base : public thread_group
{
    static const unsigned int numThreads = Size;

    _CG_QUALIFIER unsigned int build_mask() const {
        unsigned int mask;

        if (numThreads == 32) {
            mask = 0xFFFFFFFF;
        }
        else {
            mask = (unsigned int)(-1) >> (32 - numThreads);
            mask <<= (__internal::laneid() & (~(numThreads - 1)));
        }
        return (mask);
    }

 protected:
    _CG_QUALIFIER __thread_block_tile_base() : thread_group(__internal::CoalescedTile) {
        _data.coalesced.mask = build_mask();
        _data.coalesced.size = numThreads;
    }

 public:
    _CG_QUALIFIER void sync() const {
        __syncwarp(build_mask());
    }
    _CG_QUALIFIER unsigned int thread_rank() const {
        return (__internal::laneid() & (numThreads - 1));
    }
    _CG_QUALIFIER unsigned int size() const {
        return (numThreads);
    }

    // PTX supported collectives
    _CG_QUALIFIER int shfl(int var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER int shfl_down(int var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER int shfl_up(int var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER int shfl_xor(int var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER unsigned int shfl(unsigned int var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER unsigned int shfl_down(unsigned int var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER unsigned int shfl_up(unsigned int var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER unsigned int shfl_xor(unsigned int var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER long shfl(long var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER long shfl_down(long var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER long shfl_up(long var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER long shfl_xor(long var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER unsigned long shfl(unsigned long var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER unsigned long shfl_down(unsigned long var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER unsigned long shfl_up(unsigned long var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER unsigned long shfl_xor(unsigned long var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER long long shfl(long long var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER long long shfl_down(long long var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER long long shfl_up(long long var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER long long shfl_xor(long long var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER unsigned long long shfl(unsigned long long var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER unsigned long long shfl_down(unsigned long long var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER unsigned long long shfl_up(unsigned long long var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER unsigned long long shfl_xor(unsigned long long var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER float shfl(float var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER float shfl_down(float var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER float shfl_up(float var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER float shfl_xor(float var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER double shfl(double var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER double shfl_down(double var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER double shfl_up(double var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER double shfl_xor(double var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER int any(int predicate) const {
        unsigned int lane_ballot = build_mask() & __ballot_sync(build_mask(), predicate);
        return (lane_ballot != 0);
    }
    _CG_QUALIFIER int all(int predicate) const {
        unsigned int lane_ballot = build_mask() & __ballot_sync(build_mask(), predicate);
        return (lane_ballot == build_mask());
    }
    _CG_QUALIFIER unsigned int ballot(int predicate) const {
        unsigned int lane_ballot = build_mask() & __ballot_sync(build_mask(), predicate);
        return (lane_ballot >> (__internal::laneid() & (~(numThreads - 1))));
    }

#ifdef _CG_HAS_FP16_COLLECTIVE
    _CG_QUALIFIER __half shfl(__half var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER __half shfl_down(__half var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER __half shfl_up(__half var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER __half shfl_xor(__half var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
    _CG_QUALIFIER __half2 shfl(__half2 var, int srcRank) const {
        return (__shfl_sync(build_mask(), var, srcRank, numThreads));
    }
    _CG_QUALIFIER __half2 shfl_down(__half2 var, unsigned int delta) const {
        return (__shfl_down_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER __half2 shfl_up(__half2 var, unsigned int delta) const {
        return (__shfl_up_sync(build_mask(), var, delta, numThreads));
    }
    _CG_QUALIFIER __half2 shfl_xor(__half2 var, unsigned int laneMask) const {
        return (__shfl_xor_sync(build_mask(), var, laneMask, numThreads));
    }
#endif

#ifdef _CG_HAS_MATCH_COLLECTIVE
    _CG_QUALIFIER unsigned int match_any(int val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(unsigned int val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(long val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(unsigned long val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(long long val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(unsigned long long val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(float val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_any(double val) const {
        unsigned int lane_match = build_mask() & __match_any_sync(build_mask(), val);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }

    _CG_QUALIFIER unsigned int match_all(int val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(unsigned int val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(long val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(unsigned long val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(long long val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(unsigned long long val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(float val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
    _CG_QUALIFIER unsigned int match_all(double val, int &pred) const {
        unsigned int lane_match = build_mask() & __match_all_sync(build_mask(), val, &pred);
        return (lane_match >> (__internal::laneid() & (~(numThreads - 1))));
    }
#endif

};

/**
 * class thread_block_tile<unsigned int Size>
 *
 * Statically-sized group type, representing one tile of a thread block.
 * The only specializations currently supported are those with native
 * hardware support (1/2/4/8/16/32)
 *
 * This group exposes warp-synchronous builtins.
 * Constructed via tiled_partition<Size>(class thread_block);
 */
template <unsigned int Size>
class thread_block_tile;
template <> class thread_block_tile<32> : public __thread_block_tile_base<32> { };
template <> class thread_block_tile<16> : public __thread_block_tile_base<16> { };
template <> class thread_block_tile<8>  : public __thread_block_tile_base<8> { };
template <> class thread_block_tile<4>  : public __thread_block_tile_base<4> { };
template <> class thread_block_tile<2>  : public __thread_block_tile_base<2> { };
template <> class thread_block_tile<1>  : public __thread_block_tile_base<1> { };

/**
 * Outer level API calls
 * void sync(GroupT) - see <group_type>.sync()
 * void thread_rank(GroupT) - see <group_type>.thread_rank()
 * void group_size(GroupT) - see <group_type>.size()
 */
template <class GroupT> _CG_QUALIFIER void sync(GroupT const &g)
{
    g.sync();
}

template <class GroupT> _CG_QUALIFIER unsigned int thread_rank(GroupT const& g)
{
    return (g.thread_rank());
}

template <class GroupT> _CG_QUALIFIER unsigned int group_size(GroupT const &g)
{
    return (g.size());
}

/**
 * <group_type>.sync()
 *
 * Executes a barrier across the group
 *
 * Implements both a compiler fence and an architectural fence to prevent,
 * memory reordering around the barrier.
 */
_CG_QUALIFIER void thread_group::sync() const
{
    if (_data.type == __internal::Coalesced || _data.type == __internal::CoalescedTile) {
        static_cast<const coalesced_group*>(this)->sync();
    }
    else {
        static_cast<const thread_block*>(this)->sync();
    }
}

/**
 * <group_type>.size()
 *
 * Returns the total number of threads in the group.
 */
_CG_QUALIFIER unsigned int thread_group::size() const
{
    if (_data.type == __internal::Coalesced || _data.type == __internal::CoalescedTile) {
        return (static_cast<const coalesced_group*>(this)->size());
    }
    else {
        return (static_cast<const thread_block*>(this)->size());
    }
}

/**
 * <group_type>.thread_rank()
 *
 * Returns the linearized rank of the calling thread along the interval [0, size()).
 */
_CG_QUALIFIER unsigned int thread_group::thread_rank() const
{
    if (_data.type == __internal::Coalesced || _data.type == __internal::CoalescedTile) {
        return (static_cast<const coalesced_group*>(this)->thread_rank());
    }
    else {
        return (static_cast<const thread_block*>(this)->thread_rank());
    }
}

/**
 * tiled_partition
 *
 * The tiled_partition(parent, tilesz) method is a collective operation that
 * partitions the parent group into a one-dimensional, row-major, tiling of subgroups.
 *
 * A total of ((size(parent)+tilesz-1)/tilesz) subgroups will
 * be created where threads having identical k = (thread_rank(parent)/tilesz)
 * will be members of the same subgroup.
 *
 * The implementation may cause the calling thread to wait until all the members
 * of the parent group have invoked the operation before resuming execution.
 *
 * Functionality is limited to power-of-two sized subgorup instances of at most
 * 32 threads. Only thread_block, thread_block_tile<>, and their subgroups can be
 * tiled_partition() in _CG_VERSION 1000.
 */
_CG_QUALIFIER thread_group tiled_partition(const thread_group& parent, unsigned int tilesz)
{
    if (parent._data.type == __internal::Coalesced || parent._data.type == __internal::CoalescedTile) {
        return (static_cast<const coalesced_group&>(parent)._get_tiled_threads(tilesz));
    }
    else {
        return (static_cast<const thread_block&>(parent)._get_tiled_threads(tilesz));
    }
}
// Thread block type overload: returns a basic thread_group for now (may be specialized later)
_CG_QUALIFIER thread_group tiled_partition(const thread_block& parent, unsigned int tilesz)
{
    return (parent._get_tiled_threads(tilesz));
}
// Coalesced group type overload: retains its ability to stay coalesced
_CG_QUALIFIER coalesced_group tiled_partition(const coalesced_group& parent, unsigned int tilesz)
{
    return (parent._get_tiled_threads(tilesz));
}

namespace __internal {

    // For specializing on different tiled_partition template arguments
    template <unsigned int Size, typename ParentT>
    struct tiled_partition_impl;

    template <unsigned int Size>
    struct tiled_partition_impl<Size, thread_block> : public thread_block_tile<Size> {
        _CG_QUALIFIER tiled_partition_impl(thread_block const &) : thread_block_tile<Size>() {}
    };
    template <unsigned int Size>
    struct tiled_partition_impl<Size, thread_block_tile<32> > : public thread_block_tile<Size> {
        _CG_QUALIFIER tiled_partition_impl(thread_block_tile<32> const&) : thread_block_tile<Size>() {}
    };
    template <unsigned int Size>
    struct tiled_partition_impl<Size, thread_block_tile<16> > : public thread_block_tile<Size> {
        _CG_QUALIFIER tiled_partition_impl(thread_block_tile<16> const&) : thread_block_tile<Size>() {}
    };
    template <unsigned int Size>
    struct tiled_partition_impl<Size, thread_block_tile<8> > : public thread_block_tile<Size> {
        _CG_QUALIFIER tiled_partition_impl(thread_block_tile<8> const&) : thread_block_tile<Size>() {}
    };
    template <unsigned int Size>
    struct tiled_partition_impl<Size, thread_block_tile<4> > : public thread_block_tile<Size> {
        _CG_QUALIFIER tiled_partition_impl(thread_block_tile<4> const&) : thread_block_tile<Size>() {}
    };
    template <unsigned int Size>
    struct tiled_partition_impl<Size, thread_block_tile<2> > : public thread_block_tile<Size> {
        _CG_QUALIFIER tiled_partition_impl(thread_block_tile<2> const&) : thread_block_tile<Size>() {}
    };
    template <>
    struct tiled_partition_impl<1, thread_block_tile<1> > : public thread_block_tile<1> {
        _CG_QUALIFIER tiled_partition_impl(thread_block_tile<1> const&) : thread_block_tile<1>() {}
    };

};

/**
 * tiled_partition<tilesz>
 *
 * The tiled_partition<tilesz>(parent) method is a collective operation that
 * partitions the parent group into a one-dimensional, row-major, tiling of subgroups.
 *
 * A total of ((size(parent)/tilesz) subgroups will be created,
 * therefore the parent group size must be evenly divisible by the tilesz.
 * The allow parent groups are thread_block or thread_block_tile<size>.
 *
 * The implementation may cause the calling thread to wait until all the members
 * of the parent group have invoked the operation before resuming execution.
 *
 * Functionality is limited to native hardware sizes, 1/2/4/8/16/32.
 * The size(parent) must be greater than the template Size parameter
 * otherwise the results are undefined.
 */
template <unsigned int Size, typename ParentT>
_CG_QUALIFIER thread_block_tile<Size> tiled_partition(const ParentT& g)
{
    return (__internal::tiled_partition_impl<Size, ParentT>(g));
}

_CG_END_NAMESPACE

# endif /* ! (__cplusplus, __CUDACC__) */

#endif /* !_COOPERATIVE_GROUPS_H_ */
