//------------------------------------------------------------------------------
// rmm_wrap.cpp: C-callable wrapper for an RMM memory resource
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// rmm_wrap.cpp contains a single global object, the RMM_Wrap_Handle that holds
// an RMM (Rapids Memory Manager) memory resource and a hash map (C++
// std:unordered_map).  This allows rmm_wrap to provide 7 functions to a C
// application:

// Create/destroy an RMM resource:
//      rmm_wrap_initialize: create the RMM resource
//      rmm_wrap_finalize: destroy the RMM resource

// C-style malloc/calloc/realloc/free methods:
//      rmm_wrap_malloc:  malloc a block of memory using RMM
//      rmm_wrap_calloc:  calloc a block of memory using RMM
//      rmm_wrap_realloc: realloc a block of allocated by this RMM wrapper
//      rmm_wrap_free:    free a block of memory allocated by this RMM wrapper

// PMR-based allocate/deallocate methods (C-callable):
//      rmm_wrap_allocate (std::size_t *size)
//      rmm_wrap_deallocate (void *p, std::size_t size)

#include "rmm_wrap.hpp"
#include <iostream>

//------------------------------------------------------------------------------
// RMM_Wrap_Handle: a global object containing the RMM context
//------------------------------------------------------------------------------

// rmm_wrap_context is a pointer to a single, global RMM_Wrap_Handle object
// that all methods in this file can access.  The object and its pointer
// cannot be accessed outside this file.

typedef struct
{
    RMM_MODE mode;
    std::shared_ptr<rmm::mr::device_memory_resource>   resource;
    std::shared_ptr<std::pmr::memory_resource>         host_resource;
    std::shared_ptr<alloc_map>                         size_map ;
    std::shared_ptr<cuda_stream_pool>                  stream_pool;
    cudaStream_t                                       main_stream;
}
RMM_Wrap_Handle ;

// rmm_wrap_context: global pointer to the single RMM_Wrap_Handle object
static RMM_Wrap_Handle *rmm_wrap_context = NULL ;

//------------------------------------------------------------------------------
// make a resource pool
//------------------------------------------------------------------------------

#if 0
inline auto make_host()
{
    return std::make_shared<rmm::mr::new_delete_resource>() ;
}

inline auto make_host_pinned()
{
    return std::make_shared<rmm::mr::pinned_memory_resource>() ;
}
#endif

inline auto make_cuda()
{
    return std::make_shared<rmm::mr::cuda_memory_resource>() ;
}

inline auto make_managed()
{
    return std::make_shared<rmm::mr::managed_memory_resource>() ;
}

#if 0
inline auto make_and_set_host_pool
(
    std::size_t initial_size,
    std::size_t maximum_size
)
{
    auto resource = std::pmr::synchronized_pool_resource() ;
    rmm::mr::set_current_device_resource( resource ) ;
    return resource;
}

inline auto make_and_set_host_pinned_pool
(
    std::size_t initial_size,
    std::size_t maximum_size
)
{
    auto resource = rmm::mr::make_owning_wrapper<pool_mr>
        ( make_host_pinned(), initial_size, maximum_size ) ;
    rmm::mr::set_current_device_resource( resource.get()) ;
    return resource;
}
#endif

// size_map is an unordered alloc_map that maps allocation address to the size
// of each allocation

inline auto make_and_set_device_pool
(
    std::size_t initial_size,
    std::size_t maximum_size
)
{
    auto resource = rmm::mr::make_owning_wrapper<rmm::mr::pool_memory_resource>
                    ( make_cuda(), initial_size, maximum_size ) ;
    rmm::mr::set_current_device_resource( resource.get()) ;
    return resource;
}

inline auto make_and_set_managed_pool
(
    std::size_t initial_size,
    std::size_t maximum_size
)
{
    //  std::cout<< " make_managed_pool called with  init_size"
    //  <<initial_size<<" max_size "<<maximum_size<<"\n";

    auto resource = rmm::mr::make_owning_wrapper<rmm::mr::pool_memory_resource>
                        ( make_managed(), initial_size, maximum_size ) ;
    rmm::mr::set_current_device_resource( resource.get()) ;
    return resource;
}

inline std::shared_ptr<rmm::cuda_stream_pool> make_and_set_cuda_stream_pool
(
    std::size_t num_streams
)
{
    return std::make_shared<rmm::cuda_stream_pool>(num_streams);
}

inline cudaStream_t make_cuda_stream()
{
    cudaStream_t user_stream;
    RMM_WRAP_CHECK_CUDA(cudaStreamCreate(&user_stream));
    return user_stream;
}
//------------------------------------------------------------------------------
// rmm_wrap_finalize: destroy the global rmm_wrap_context
//------------------------------------------------------------------------------

// Destroy the rmm_wrap_context.  This method allows destroys the contents of
// the rmm_wrap_context:  the memory resource (host or device) and the
// alloc_map.

void rmm_wrap_finalize (void)
{
    if (rmm_wrap_context != NULL)
    {
        RMM_WRAP_CHECK_CUDA(cudaStreamDestroy(rmm_wrap_context->main_stream));
        delete (rmm_wrap_context) ;
        rmm_wrap_context = NULL ;
    }
}

//------------------------------------------------------------------------------
// rmm_wrap_initialize: initialize the global rmm_wrap_context
//------------------------------------------------------------------------------

int rmm_wrap_initialize             // returns -1 on error, 0 on success
(
    RMM_MODE mode,                  // TODO: describe
    std::size_t init_pool_size,     // TODO: describe
    std::size_t max_pool_size,       // TODO: describe
    std::size_t stream_pool_size
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (rmm_wrap_context != NULL)
    {
        // rmm_wrap_initialize cannot be called twice
        return (-1) ;
    }

    if(stream_pool_size <= 0)
    {
        std::cout << "Stream pool size must be >=0" << std::endl;
        // failed to create the alloc_map
        return (-1) ;
    }

    // create the RMM wrap handle and save it as a global pointer.
    rmm_wrap_context = new RMM_Wrap_Handle() ;

    //  std::cout<< " init called with mode "<<mode<<" init_size "
    // <<init_pool_size<<" max_size "<<max_pool_size<<"\n";

    //--------------------------------------------------------------------------
    // Construct a resource that uses a coalescing best-fit pool allocator
    //--------------------------------------------------------------------------

    // Set CUDA stream pool
    rmm_wrap_context->stream_pool = make_and_set_cuda_stream_pool(stream_pool_size);
    rmm_wrap_context->main_stream = make_cuda_stream();

    if (mode == rmm_wrap_host )
    {
        // rmm_wrap_context->host_resource =
        //  std::pmr::synchronized_pool_resource() ;
        //  // (init_pool_size, max_pool_size) ;
        // rmm_wrap_context->host_resource =  make_and_set_host_pool() ;
        //  // (init_pool_size, max_pool_size) ;
    }
    else if (mode == rmm_wrap_host_pinned )
    {
        // rmm_wrap_context->host_resource =
        //  std::pmr::synchronized_pool_resource() ;
        //  // (init_pool_size, max_pool_size) ;
    }
    else if (mode == rmm_wrap_device )
    {
        rmm_wrap_context->resource =
            make_and_set_device_pool( init_pool_size, max_pool_size) ;
    }
    else if ( mode == rmm_wrap_managed )
    {
        rmm_wrap_context->resource =
            make_and_set_managed_pool( init_pool_size, max_pool_size) ;
    }
    else
    {
        // invalid mode
        return (-1) ;
    }

    // Mark down the mode for reference later
    rmm_wrap_context->mode = mode;

    //--------------------------------------------------------------------------
    // create size map to lookup size of each allocation
    //--------------------------------------------------------------------------

    rmm_wrap_context->size_map = std::make_shared<alloc_map> () ;
    if (rmm_wrap_context->size_map.get() == NULL)
    {
        // failed to create the alloc_map
        return (-1) ;
    }

    return (0) ;
}

cudaStream_t get_next_stream_from_pool() {
    return rmm_wrap_context->stream_pool->get_stream();
}

cudaStream_t get_stream_from_pool(std::size_t stream_id) {
    return rmm_wrap_context->stream_pool->get_stream(stream_id);
}

cudaStream_t get_main_stream() {
    return rmm_wrap_context->main_stream;
}
//------------------------------------------------------------------------------
// rmm_wrap_malloc: malloc-equivalent method using RMM
//------------------------------------------------------------------------------

// rmm_wrap_malloc is identical to the ANSI C11 malloc function, except that
// it uses RMM underneath to allocate the space.

void *rmm_wrap_malloc (std::size_t size)
{
    return (rmm_wrap_allocate (&size)) ;
}

//------------------------------------------------------------------------------
// rmm_wrap_calloc: calloc-equivalent method using RMM
//------------------------------------------------------------------------------

// rmm_wrap_calloc is identical to the ANSI C11 calloc function, except that
// it uses RMM underneath to allocate the space.

void *rmm_wrap_calloc (std::size_t n, std::size_t size)
{
    std::size_t s = n * size ;
    void *p = rmm_wrap_allocate (&s) ;
    // NOTE: this is single-threaded on the CPU.  If you want a faster method,
    // malloc the space and use cudaMemset for the GPU or GB_memset on the CPU.
    // The GraphBLAS GB_calloc_memory method uses malloc and GB_memset.
    memset (p, 0, s) ;
    return (p) ;
}

//------------------------------------------------------------------------------
// rmm_wrap_realloc: realloc-equivalent method using RMM
//------------------------------------------------------------------------------

// rmm_wrap_realloc is identical to the ANSI C11 realloc function, except that
// it uses RMM underneath to allocate the space.

void *rmm_wrap_realloc (void *p, std::size_t newsize)
{
    if (p == NULL)
    {
        // allocate a new block.  This is OK.
        return (rmm_wrap_allocate (&newsize)) ;
    }

    if (newsize == 0)
    {
        // free the block.  This OK.
        rmm_wrap_deallocate (p, 0) ;
        return (NULL) ;
    }

    alloc_map *am = rmm_wrap_context->size_map.get() ;
    std::size_t oldsize = am->at( (std::size_t)(p) ) ;

    if (oldsize == 0)
    {
        // the block is not in the hashmap; cannot realloc it.
        // This is a failure.
        return (NULL) ;
    }

    // check for quick return
    if (newsize >= oldsize/2 && newsize <= oldsize)
    {
        // Be lazy. If the block does not change, or is shrinking but only by a
        // small amount, then leave the block as-is.
        return (p) ;
    }

    // allocate the new space
    void *pnew = rmm_wrap_allocate (&newsize) ;
    if (pnew == NULL)
    {
        // old block is not modified.  This is a failure, but the old block is
        // still in the hashmap.
        return (NULL) ;
    }

    // copy the old space into the new space
    std::size_t s = (oldsize < newsize) ? oldsize : newsize ;
    // FIXME: query the pointer if it's on the GPU.
    memcpy (pnew, p, s) ; // NOTE: single-thread CPU, not GPU.  Slow!

    // free the old space
    rmm_wrap_deallocate (p, oldsize) ;

    // return the new space
    return (pnew) ;
}

//------------------------------------------------------------------------------
// rmm_wrap_free: free a block of memory, size not needed
//------------------------------------------------------------------------------

// rmm_wrap_free is identical to the ANSI C11 free function, except that
// it uses RMM underneath to allocate the space.

void rmm_wrap_free (void *p)
{
    rmm_wrap_deallocate (p, 0) ;
}

//------------------------------------------------------------------------------
// rmm_wrap_allocate: allocate a block of memory using RMM
//------------------------------------------------------------------------------

void *rmm_wrap_allocate( std::size_t *size)
{
    if (rmm_wrap_context == NULL) return (NULL) ;

    alloc_map *am = rmm_wrap_context->size_map.get() ;
    if (am == NULL)
    {
        // PANIC!
        // std::cout<< "Uh oh, can't allocate before initializing RMM"
        // << std::endl;
        return (NULL) ;
    }

    // ensure size is nonzero
    if (*size == 0) *size = 256 ;
    // round-up the allocation to a multiple of 256
    std::size_t aligned = (*size) % 256 ;
    if (aligned > 0)
    {
        *size += (256 - aligned) ;
    }

//  printf(" rmm_wrap_alloc %ld bytes\n",*size) ;


    rmm::mr::device_memory_resource *memoryresource =
        rmm::mr::get_current_device_resource() ;
    void *p = memoryresource->allocate( *size ) ;
    if (p == NULL)
    {
        // out of memory
        *size = 0 ;
        return (NULL) ;
    }

    // insert p into the hashmap
    am->emplace ((std::size_t)p, (std::size_t)(*size)) ;

    // return the allocated block
    return (p) ;
}

//------------------------------------------------------------------------------
// rmm_wrap_deallocate: deallocate a block previously allocated by RMM
//------------------------------------------------------------------------------

void rmm_wrap_deallocate( void *p, std::size_t size)
{
    if (rmm_wrap_context == NULL) return ;

    // Note: there are 3 PANIC cases below.  The API of rmm_wrap_deallocate
    // does not allow an error condition to be returned.  These PANICs could be
    // logged, or they could terminate the program if debug mode enabled, etc.
    // In production, all we can do is ignore the PANIC.

    if (p == NULL)
    {
        // nothing to do; ignore a double-free
        if (size > 0)
        {
            // PANIC!  Why does a NULL pointer have a nonzero size??
        }
        return ;
    }

    // check the size given.  If the input size is zero, then the
    // size is unknown (say rmm_wrap_free(p)).  In that case, just trust the
    // hashmap.  Otherwise, double-check to make sure the size is correct.
    alloc_map *am = rmm_wrap_context->size_map.get() ;
    size_t actual_size = 0 ;
    if (am == NULL)
    {
        // PANIC!
        // std::cout<< "Uh oh, can't deallocate before initializing RMM"
        // << std::endl;
        return ;
    }
    else
    {
       //actual_size = am->at( (std::size_t)(p) )  ;
       auto iter = am->find( (std::size_t)(p) )  ;
       if (iter != am->end() ) actual_size = iter->second;
       else std::cout<< " rmm_wrap:: tried to free unallocated pointer ! " << p ;
    }

    if (actual_size == 0)
    {
        // PANIC!  oops, p is not in the hashmap.  Ignore it.  TODO: could add
        // a printf here, write to a log file, etc.  if debug mode, abort, etc.
        return ;
    }

    if (size > 0 && size != actual_size)
    {
        // PANIC!  oops, invalid old size.  Ignore the input size, and free p
        // anyway.  TODO: could add a printf here, write to a log file, etc.
        // if debug mode, abort, etc.
    }

    // remove p from the hashmap
    am->erase ( (std::size_t)(p) ) ;

    // deallocate the block of memory
    rmm::mr::device_memory_resource *memoryresource =
        rmm::mr::get_current_device_resource() ;
    memoryresource->deallocate( p, actual_size ) ;
}

