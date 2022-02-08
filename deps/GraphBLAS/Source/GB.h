//------------------------------------------------------------------------------
// GB.h: definitions visible only inside GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_H
#define GB_H

//------------------------------------------------------------------------------
// definitions that modify GraphBLAS.h
//------------------------------------------------------------------------------

#include "GB_dev.h"
#include "GB_compiler.h"
#include "GB_cpu_features.h"
#include "GB_warnings.h"
#include "GB_coverage.h"
#define GB_LIBRARY

//------------------------------------------------------------------------------
// user-visible GraphBLAS.h
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// internal #include files
//------------------------------------------------------------------------------

#define restrict GB_restrict

#include "GB_prefix.h"
#include "GB_bytes.h"
#include "GB_defaults.h"
#include "GB_index.h"
#include "GB_cplusplus.h"
#include "GB_Global.h"
#include "GB_printf.h"
#include "GB_assert.h"
#include "GB_opaque.h"
#include "GB_static_header.h"
#include "GB_casting.h"
#include "GB_math.h"
#include "GB_bitwise.h"
#include "GB_binary_search.h"
#include "GB_check.h"
#include "GB_nnz.h"
#include "GB_zombie.h"
#include "GB_partition.h"
#include "GB_omp.h"
#include "GB_context.h"
#include "GB_nthreads.h"
#include "GB_memory.h"
#include "GB_werk.h"
#include "GB_log2.h"
#include "GB_iso.h"
#include "GB_Pending_n.h"
#include "GB_nvals.h"
#include "GB_aliased.h"
#include "GB_init.h"
#include "GB_new.h"
#include "GB_clear.h"
#include "GB_resize.h"
#include "GB_dup.h"
#include "GB_code_compatible.h"
#include "GB_compatible.h"
#include "GB_task_struct.h"
#include "GB_transplant.h"
#include "GB_type.h"
#include "GB_slice.h"
#include "GB_multiply.h"
#include "GB_extractTuples.h"
#include "GB_cumsum.h"
#include "GB_Descriptor_get.h"
#include "GB_Element.h"
#include "GB_op.h"
#include "GB_hyper.h"
#include "GB_ok.h"
#include "GB_cast.h"
#include "GB_wait.h"
#include "GB_convert.h"
#include "GB_ops.h"
#include "GB_cuda_gateway.h"

#endif

