/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "thpool.h"

#define THPOOL_QUEUE_FULL -2
// create both readers and writers thread pools
int ThreadPools_CreatePools
(
	uint reader_count,
	uint writer_count,
	uint bulk_count
);

// return number of threads in both the readers and writers pools
uint ThreadPools_ThreadCount
(
	void
);

// retrieve current thread id
// 0         redis-main
// 1..N + 1  readers
// N + 2..   writers
int ThreadPools_GetThreadID
(
	void
);

// returns true if caller thread is a READER thread, false otherwise
bool ThreadPools_AmReader
(
	void
);

// returns true if caller thread is a WRITER thread, false otherwise
bool ThreadPools_AmWriter
(
	void
);

// pause all thread pools
void ThreadPools_Pause
(
	void
);

// resume all threads
void ThreadPools_Resume
(
	void
);

// adds a read task
int ThreadPools_AddWorkReader
(
	void (*function_p)(void *),
	void *arg_p
);

// add a write task
int ThreadPools_AddWorkWriter
(
	void (*function_p)(void *),
	void *arg_p
);

// add a bulk laoder task
int ThreadPools_AddWorkBulkLoader
(
	void (*function_p)(void *),
	void *arg_p
);

// Sets the limit on max queued queries in each thread pool.
void ThreadPools_Set_max_pending_work(uint64_t val);

