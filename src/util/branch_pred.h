#pragma once
/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

/* Branch prediction functionality            */
/* Give the compiler hints whether            */
/* the condition is going to be true or false */

#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)   // 
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif
