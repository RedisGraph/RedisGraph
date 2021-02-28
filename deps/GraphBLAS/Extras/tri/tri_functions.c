//------------------------------------------------------------------------------
// tri_functions: four variants of outer-product triangle counting
//------------------------------------------------------------------------------

#include "tri_def.h"

//------------------------------------------------------------------------------
// outer-product methods, no binary search
//------------------------------------------------------------------------------

// tri_mark
#include "tri_template.c"

// tri_bit
#define BIT
#include "tri_template.c"

// tri_mark_parallel
#define PARALLEL
#include "tri_template.c"

// tri_bit_parallel
#define PARALLEL
#define BIT
#include "tri_template.c"

//------------------------------------------------------------------------------
// dot product methods
//------------------------------------------------------------------------------

// tri_dot
#include "tri_dot_template.c"

// tri_dot_parallel
#define PARALLEL
#include "tri_dot_template.c"

//------------------------------------------------------------------------------
// outer-product methods, with binary search
//------------------------------------------------------------------------------

// tri_logmark
#define LOGSEARCH
#include "tri_template.c"

// tri_logbit
#define LOGSEARCH
#define BIT
#include "tri_template.c"

// tri_logmark_parallel
#define LOGSEARCH
#define PARALLEL
#include "tri_template.c"

// tri_logbit_parallel
#define LOGSEARCH
#define PARALLEL
#define BIT
#include "tri_template.c"
