/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "pools.h"
#include <pthread.h>

//------------------------------------------------------------------------------
// Thread pools
//------------------------------------------------------------------------------

static threadpool _readers_thpool = NULL;  // readers
static threadpool _writers_thpool = NULL;  // writers

// set up thread pools  (readers and writers)
// returns 1 if thread pools initialized, 0 otherwise
int ThreadPools_CreatePools
(
	uint reader_count, 
	uint writer_count
) {
	ASSERT(_readers_thpool == NULL);
	ASSERT(_writers_thpool == NULL);

	_readers_thpool = thpool_init(reader_count, "reader");
	if(_readers_thpool == NULL) return 0;

	_writers_thpool = thpool_init(writer_count, "writer");
	if(_writers_thpool == NULL) return 0;

	return 1;
}

// return number of threads in both the readers and writers pools
uint ThreadPools_ThreadCount
(
	void
)
{
	ASSERT(_readers_thpool != NULL);
	ASSERT(_writers_thpool != NULL);

	uint count = 0;
	count +=  thpool_num_threads(_readers_thpool);
	count +=  thpool_num_threads(_writers_thpool);

	return count;
}

// retrieve current thread id
// 0         redis-main
// 1..N + 1  readers
// N + 2..   writers
int ThreadPools_GetThreadID
(
	void
) {
	ASSERT(_readers_thpool != NULL);
	ASSERT(_writers_thpool != NULL);

	// thpool_get_thread_id returns -1 if pthread_self isn't in the thread pool
	// most likely Redis main thread
	int thread_id;
	pthread_t pthread = pthread_self();
	int readers_count = thpool_num_threads(_readers_thpool);

	// search in writers
	thread_id = thpool_get_thread_id(_writers_thpool, pthread);
	// compensate for Redis main thread
	if(thread_id != -1) return readers_count + thread_id + 1; 

	// search in readers pool
	thread_id = thpool_get_thread_id(_readers_thpool, pthread);
	// compensate for Redis main thread
	if(thread_id != -1) return thread_id + 1;

	return 0; // assuming Redis main thread
}

// pause all thread pools
void ThreadPools_Pause
(
	void
) {
	ASSERT(_readers_thpool != NULL);
	ASSERT(_writers_thpool != NULL);

	thpool_pause(_readers_thpool);
	thpool_pause(_writers_thpool);
}

void ThreadPools_Resume
(
	void
) {

	ASSERT(_readers_thpool != NULL);
	ASSERT(_writers_thpool != NULL);

	thpool_resume(_readers_thpool);
	thpool_resume(_writers_thpool);
}

// add task for reader thread
int ThreadPools_AddWorkReader
(
	void (*function_p)(void*), 
	void* arg_p
) {
	ASSERT(_readers_thpool != NULL);

	return thpool_add_work(_readers_thpool, function_p, arg_p); 
}

// add task for writer thread
int ThreadPools_AddWorkWriter
(
	void (*function_p)(void*), 
	void* arg_p
) {
	ASSERT(_writers_thpool != NULL);

	return thpool_add_work(_writers_thpool, function_p, arg_p); 
}

// destroy all thread pools
void ThreadPools_Destroy
(
	void
) {
	ASSERT(_readers_thpool != NULL);
	ASSERT(_writers_thpool != NULL);

	thpool_destroy(_readers_thpool);
	thpool_destroy(_writers_thpool);
}

