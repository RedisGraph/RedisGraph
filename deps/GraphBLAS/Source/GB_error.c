//------------------------------------------------------------------------------
// GB_error: log an error string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_error logs the details of an error to the error string in thread-local
// storage so that it is accessible to GrB_error.  A GrB_PANIC is not logged
// to the error string since the panic may mean the string is not available.

// SuiteSparse:GraphBLAS can generate a GrB_PANIC in the following ways:

//  (1) a failure to create the critical section or the POSIX thread-local
//      storage key in GrB_init.
//  (2) a failure in the critical section (see GB_CRITICAL, GB_queue_*, and
//      Template/GB_critical_section).
//  (3) a failure to allocate thread-local storage for GrB_error
//      (see GB_thread_local_access).
//  (4) a failure to destroy the critical section in GrB_finalize.

#include "GB.h"

GrB_Info GB_error           // log an error in thread-local-storage
(
    GrB_Info info,          // error return code from a GraphBLAS function
    GB_Context Context      // pointer to a Context struct, on the stack
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // GrB_SUCCESS and GrB_NO_VALUE are not errors.

    // GrB_PANIC cannot use this error reporting mechanism because the error
    // string requires thread-local storage.

    ASSERT (info != GrB_SUCCESS) ;
    ASSERT (info > GrB_NO_VALUE) ;
    ASSERT (info < GrB_PANIC) ;

    //--------------------------------------------------------------------------
    // get pointer to thread-local-storage
    //--------------------------------------------------------------------------

    char *p = GB_thread_local_access ( ) ;

    //--------------------------------------------------------------------------
    // write the error to the string p
    //--------------------------------------------------------------------------

    if (p != NULL)
    {
        // p now points to thread-local storage (char array of size GB_RLEN+1)
        snprintf (p, GB_RLEN, "GraphBLAS error: %s\nfunction: %s\n%s\n",
            GB_status_code (info), Context->where, Context->details) ;
        return (info) ;
    }
    else
    {
        // If a failure occured or p is NULL then do not write to the string.
        // A GrB_PANIC will be returned.
        return (GrB_PANIC) ;
    }
}

