/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// this file contains all the necessary data structures for the "graph.info"
// command to operate
// in order to show the statistics, it is required to store
// and accumulate the information about different queries, their stages and the
// time those spend in these stages

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/types.h>

#include "query_info.h"
#include "../util/dict.h"
#include "../util/simple_timer.h"
#include "../util/circular_buffer.h"

typedef enum QueryExecutionStatus QueryExecutionStatus;
typedef enum QueryExecutionTypeFlag QueryExecutionTypeFlag;

#define MILLIS_T_MAX UINT32_MAX
typedef struct QueryCtx QueryCtx;

// holds query statistics per graph
typedef struct FinishedQueryCounters {
    atomic_uint_fast64_t ro_succeeded_n;     // # read-only queries succeeded
    atomic_uint_fast64_t write_succeeded_n;  // # write queries succeeded
    atomic_uint_fast64_t ro_failed_n;        // # RO queries failed
    atomic_uint_fast64_t write_failed_n;     // # write queries failed
    atomic_uint_fast64_t ro_timedout_n;      // # RO queries timed out
    atomic_uint_fast64_t write_timedout_n;   // # write queries timed out
} FinishedQueryCounters;

// information about a graph
typedef struct Info {
	dict *waiting_queries;                     // waiting queries
	QueryInfo **working_queries;               // executing & reporting queries
	CircularBuffer finished_queries;           // finished queries
	atomic_uint_fast64_t max_query_time;       // slowest query time
	FinishedQueryCounters counters;            // counters with states
	pthread_mutex_t mutex;                     // info lock
	pthread_rwlock_t finished_queries_rwlock;  // finished queries RWLock
} Info;

// create a new info structure
Info *Info_New(void);

// create the finished queries storage (circular buffer)
void Info_SetFinishedQueriesStorage
(
    const uint n  // size of circular buffer
);

// add a query to the waiting list for the first time (from dispatcher)
// at this stage, no time has been previously accumulated
void Info_AddToWaiting
(
    Info *info,    // info
    QueryInfo *qi  // query info of the query starting to wait
);

// remove 'qi' from the waiting-list and insert it to the executing-queue
void Info_AddToExecuting
(
    Info *info,    // info
    QueryInfo *qi  // query info
);

// indicates that the query has finished the execution and has started
// reporting the results back to the client
void Info_AddToReporting
(
    Info *info  // info
);

// indicates that the query has ran to completion
void Info_AddToFinished
(
    Info *info  // info
);

// transitions a query from executing to waiting
// the queryInfo in the index corresponding to the thread_id
// will be marked as waiting
void Info_ExecutingToWaiting
(
    Info *info  // info
);

//------------------------------------------------------------------------------
// Statistics
//------------------------------------------------------------------------------

// return the number of queries currently waiting for execution
uint64_t Info_GetWaitingCount
(
	Info *info  // info
);

// count the amount of executing and reporting queries
void Info_GetExecutingCount
(
    Info *info,           // info
    uint64_t *executing,  // [OUTPUT] amount of executing queries
    uint64_t *reporting   // [OUTPUT] amount of reporting queries
);

uint64_t Info_GetFinishedCount
(
	Info *info
);

// return the total number of queries currently queued or being executed
uint64_t Info_GetTotalQueriesCount
(
	Info *info  // info
);

// return the maximum registered time a query was spent waiting
millis_t Info_GetMaxWaitTime
(
    Info *info  // info
);

// increments the corresponding query type counter
// the passed parameters define the kind of query [READ/WRITE]
// and its finish status [FAILED/TIMEOUT/SUCCESS]
void Info_IncrementNumberOfQueries
(
    Info *info,
    const QueryExecutionTypeFlag,
    const QueryExecutionStatus
);

// populates 'queries' with clones of queries of given 'stage'
void Info_GetQueries
(
	Info *info,            // info
	QueryStage stage,      // wanted stage
	QueryInfo ***queries,  // queries array
	int cap                // size of array
);

// reset finished queries
// returns old finished queries buffer
CircularBuffer Info_ResetFinishedQueries
(
	Info *info  // info
);

// returns the total number of queries recorded
uint64_t FinishedQueryCounters_GetTotalCount
(
	const FinishedQueryCounters *counters  // counters to sum up
);

// free the info structure's content
void Info_Free
(
	Info *info  // information object to free
);

