/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// this file contains the useful statistics and generic information about a
// a graph, this information is used by the "GRAPH.INFO" command

#include "RG.h"
#include "info.h"
#include "util/arr.h"
#include "../util/dict.h"
#include "../query_ctx.h"
#include "util/thpool/pools.h"
#include "configuration/config.h"
#include "util/circular_buffer.h"

#include <string.h>
#include <sys/types.h>

// forward declaration
QueryInfo *QueryInfo_Clone(QueryInfo *qi);

// returns the total number of queries recorded
uint64_t FinishedQueryCounters_GetTotalCount
(
	const FinishedQueryCounters *counters  // counters to sum up
) {
    return 
        counters->write_failed_n     +
        counters->write_timedout_n   +
        counters->ro_failed_n        +
        counters->write_succeeded_n  +
        counters->ro_timedout_n      +
        counters->ro_succeeded_n ;
}

static void _FinishedQueryCounters_Reset
(
    FinishedQueryCounters *counters
) {
    ASSERT(counters != NULL);

	counters->write_failed_n     = 0;
	counters->write_timedout_n   = 0;
	counters->ro_failed_n        = 0;
	counters->write_succeeded_n  = 0;
	counters->ro_timedout_n      = 0;
	counters->ro_succeeded_n     = 0;
}

static void _Info_LockFinished
(
	Info *info,
	bool write
) {
	int res;
	UNUSED(res);

    if(write) {
        res = pthread_rwlock_wrlock(&info->finished_queries_rwlock);
		ASSERT(res == 0);
    } else {
        res = pthread_rwlock_rdlock(&info->finished_queries_rwlock);
		ASSERT(res == 0);
    }
}

static bool _Info_UnlockFinished
(
	Info *info
) {
    return !pthread_rwlock_unlock(&info->finished_queries_rwlock);
}

static void _Info_LockEverything
(
	Info *info
) {
	ASSERT(info != NULL);

    int res = pthread_mutex_lock(&info->mutex);
	UNUSED(res);
	ASSERT(res == 0);
}

static bool _Info_UnlockEverything
(
	Info *info
) {
    ASSERT(info != NULL);

    return !pthread_mutex_unlock(&info->mutex);
}

// create a new info structure
Info *Info_New(void) {
    // HACK: compensate for the main thread
    uint nthreads = ThreadPools_ThreadCount() + 1;

	Info *info = rm_calloc(1, sizeof(Info));

    // initialize hashmap for the waiting queries (keys are QueryInfo *)
    info->waiting_queries = HashTableCreate(&def_dt);

    // initialize working_queries array
    info->working_queries = rm_calloc(nthreads, sizeof(QueryInfo*));

	// initialize finished queries read/write lock
	int res = pthread_rwlock_init(&info->finished_queries_rwlock, NULL);
    ASSERT(res == 0);

	// initialize mutex
    res = pthread_mutex_init(&info->mutex, NULL);
    ASSERT(res == 0);

    // initialize finished_queries
	uint32_t n = 0;
	Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &n);
    info->finished_queries = CircularBuffer_New(sizeof(QueryInfo *), n,
        (void(*)(void*))QueryInfo_Free);

    return info;
}

// add a query to the waiting list for the first time (from dispatcher)
// at this stage, no time has been previously accumulated
void Info_AddToWaiting
(
    Info *info,    // info
    QueryInfo *qi  // query info of the query starting to wait
) {
	ASSERT(qi   != NULL);
	ASSERT(info != NULL);

    // start the stage-timer
    TIMER_RESTART(qi->timer);

    // set the stage to waiting
    qi->stage = QueryStage_WAITING;

    // acquire mutex
    _Info_LockEverything(info);

    // add the query to the waiting dict
    HashTableAdd(info->waiting_queries, qi, qi);

    // release mutex
    _Info_UnlockEverything(info);
}

// remove 'qi' from the waiting-list and insert it to the executing-queue
void Info_AddToExecuting
(
    Info *info,    // info
    QueryInfo *qi  // query info
) {
    ASSERT(qi   != NULL);
    ASSERT(info != NULL);
	ASSERT(qi->stage == QueryStage_WAITING);

    const int tid = ThreadPools_GetThreadID();

    // update waiting time
    QueryInfo_UpdateWaitingTime(qi);

	// acquire mutex
    _Info_LockEverything(info);

	//--------------------------------------------------------------------------
	// remove query info from waiting-list
	//--------------------------------------------------------------------------

    // remove from waiting_queries
    int dict_res = HashTableDelete(info->waiting_queries, (void *)qi);
	ASSERT(dict_res == DICT_OK);

	// release mutex
    _Info_UnlockEverything(info);

    // advance stage from waiting to executing
	QueryInfo_AdvanceStage(qi);

	//--------------------------------------------------------------------------
	// add query info to executing list
	//--------------------------------------------------------------------------

	ASSERT(info->working_queries[tid] == NULL);
	info->working_queries[tid] = qi;
}

// indicates that the query has finished the execution and has started
// reporting the results back to the client
void Info_AddToReporting
(
    Info *info  // info
) {
    ASSERT(info != NULL);

    const int tid = ThreadPools_GetThreadID();
    QueryInfo *qi = info->working_queries[tid];
	ASSERT(qi != NULL);
	ASSERT(qi->stage == QueryStage_EXECUTING);

	QueryInfo_AdvanceStage(qi);
    QueryInfo_UpdateExecutionTime(qi);
}

// indicates that the query has ran to completion
void Info_AddToFinished
(
	Info *info  // info
) {
    ASSERT(info != NULL);

	//--------------------------------------------------------------------------
	// remove query info from working queries
	//--------------------------------------------------------------------------

    const int tid = ThreadPools_GetThreadID();
    QueryInfo *qi = info->working_queries[tid];
	ASSERT(qi != NULL);

    info->working_queries[tid] = NULL;

	// update stage to finished and add to finished buffer
    qi->stage = QueryStage_FINISHED;

    QueryInfo_UpdateReportingTime(qi);

    _Info_LockFinished(info, true);

	// TODO: improve locking!
    CircularBuffer_AddForce(info->finished_queries, (void *)&qi);

    _Info_UnlockFinished(info);
}

// transitions a query from executing to waiting
// the queryInfo in the index corresponding to the thread_id will be removed
// from the `working_queries` array of the Info struct.
void Info_ExecutingToWaiting
(
    Info *info  // info
) {
	ASSERT(info != NULL);

    // remove from working queries array
    const int tid = ThreadPools_GetThreadID();
    QueryInfo *qi = info->working_queries[tid];

	ASSERT(qi != NULL);
	ASSERT(qi->stage == QueryStage_EXECUTING);

    // update the elapsed time in executing state, and restart timer
	QueryInfo_UpdateExecutionTime(qi);

	// clear entry
    info->working_queries[tid] = NULL;

    // set the stage to waiting
    qi->stage = QueryStage_WAITING;

    // acquire mutex
    _Info_LockEverything(info);

    // add the query to the waiting dict
    HashTableAdd(info->waiting_queries, qi, qi);

    // release mutex
    _Info_UnlockEverything(info);
}

//------------------------------------------------------------------------------
// Statistics
//------------------------------------------------------------------------------

// return the number of queries currently waiting for execution
uint64_t Info_GetWaitingCount
(
	Info *info  // info
) {
	ASSERT(info != NULL);

	// TODO: Lock only the waiting_queries mutex
	_Info_LockEverything(info);

	const uint64_t count = HashTableElemCount(info->waiting_queries);

	_Info_UnlockEverything(info);

	return count;
}

// count the amount of executing and reporting queries
void Info_GetExecutingCount
(
    Info *info,           // info
    uint64_t *executing,  // [OUTPUT] amount of executing queries
    uint64_t *reporting   // [OUTPUT] amount of reporting queries
) {
    ASSERT(info != NULL);

    uint n = ThreadPools_ThreadCount() + 1;

	for(uint i = 0; i < n; i++) {
		QueryInfo *qi = info->working_queries[i];
        if(qi != NULL) {
			ASSERT(qi->stage & (QueryStage_EXECUTING | QueryStage_REPORTING));
            if(qi->stage == QueryStage_EXECUTING) {
                (*executing)++;
            } else {
                (*reporting)++;
            }
        }
	}
}

// returns number of finished queries within circular buffer
uint64_t Info_GetFinishedCount
(
	Info *info
) {
	ASSERT(info != NULL);

	_Info_LockFinished(info, false);

	uint64_t n = CircularBuffer_ItemCount(info->finished_queries);

	_Info_UnlockFinished(info);

	return n;
}

// return the total number of queries currently queued or being executed
uint64_t Info_GetTotalQueriesCount
(
	Info *info  // info
) {
    ASSERT(info != NULL);

    uint64_t executing = 0;
    uint64_t reporting = 0;
    uint64_t waiting = Info_GetWaitingCount(info);
    Info_GetExecutingCount(info, &executing, &reporting);

    return waiting + executing + reporting;
}

// return the maximum registered time a query spent waiting
millis_t Info_GetMaxWaitTime
(
    Info *info  // info
) {
    ASSERT(info != NULL);

	QueryInfo *qi = NULL;
	millis_t max_time = 0;

	// TODO: lock waiting only
    _Info_LockEverything(info);

	// scan waiting queries
    dictIterator *it = HashTableGetIterator(info->waiting_queries);
    while((qi = (QueryInfo*)HashTableNext(it)) != NULL) {
		millis_t waiting_time = QueryInfo_UpdateWaitingTime(qi);
        if(waiting_time > max_time) {
            max_time = waiting_time;
        }
    }

	// scan executing queries
	uint n = ThreadPools_ThreadCount() + 1;
	for(uint i = 0; i < n; i++) {
		QueryInfo *qi = info->working_queries[i];
        if(qi != NULL) {
			millis_t waiting_time = QueryInfo_GetWaitingTime(qi);
			if(waiting_time > max_time) {
				max_time = waiting_time;
			}
        }
	}

	_Info_UnlockEverything(info);

	HashTableReleaseIterator(it);

	return max_time;
}

// increments the corresponding query type counter
// the passed parameters define the kind of query and its finish status
void Info_IncrementNumberOfQueries
(
    Info *info,
    const QueryExecutionTypeFlag flags,
    const QueryExecutionStatus status
) {
    ASSERT(info != NULL);

	//--------------------------------------------------------------------------
	// write query
	//--------------------------------------------------------------------------

    if(flags & QueryExecutionTypeFlag_WRITE) {
		// encountered error
        if(status == QueryExecutionStatus_FAILURE) {
            info->counters.write_failed_n++;
		// timedout
        } else if(status == QueryExecutionStatus_TIMEDOUT) {
            info->counters.write_timedout_n++;
        } else {
		// succeeded
            info->counters.write_succeeded_n++;
        }
    }

	//--------------------------------------------------------------------------
	// read query
	//--------------------------------------------------------------------------

	// encountered error
	else if(flags & QueryExecutionTypeFlag_READ) {
		if(status == QueryExecutionStatus_FAILURE) {
			info->counters.ro_failed_n++;
			// timedout
		} else if(status == QueryExecutionStatus_TIMEDOUT) {
			info->counters.ro_timedout_n++;
		} else {
			// succeeded
			info->counters.ro_succeeded_n++;
		}
	}
}

// populates 'queries' with clones of queries of given 'stage'
void Info_GetQueries
(
	Info *info,            // info
	QueryStage stage,      // wanted stage
	QueryInfo ***queries,  // queries array
	int cap                // size of array
) {
	QueryInfo *qi;
    bool waiting   = (stage & QueryStage_WAITING);
	bool executing = (stage & (QueryStage_EXECUTING | QueryStage_REPORTING));

	//--------------------------------------------------------------------------
	// waiting queries
	//--------------------------------------------------------------------------

    if(waiting) {
		ASSERT(info != NULL);
		_Info_LockEverything(info);

        dictIterator *it = HashTableGetIterator(info->waiting_queries);
        while((qi = (QueryInfo*)HashTableNext(it)) != NULL && cap-- > 0) {
            array_append(*queries, QueryInfo_Clone(qi));
        }

		_Info_UnlockEverything(info);
	}

	//--------------------------------------------------------------------------
	// executing queries
	//--------------------------------------------------------------------------

    if(executing) {
		ASSERT(info != NULL);
		uint nthreads = ThreadPools_ThreadCount();

		_Info_LockEverything(info);

        for(uint i = 0; i < nthreads && cap > 0; i++) {
            // append a clone of the current query to st
            if(info->working_queries[i] != NULL) {
				qi = info->working_queries[i];
				if(qi->stage & stage) {
					array_append(*queries, QueryInfo_Clone(qi));
					cap--;
				}
            }
        }

		_Info_UnlockEverything(info);
    }
}

CircularBuffer Info_ResetFinishedQueries
(
	Info *info
) {
	CircularBuffer prev = info->finished_queries;
	int cap = CircularBuffer_Cap(prev);
	CircularBuffer cb = CircularBuffer_New(sizeof(QueryInfo *), cap,
			(void(*)(void*))QueryInfo_Free);

	_Info_LockFinished(info, true);

	info->finished_queries = cb;

	_Info_UnlockFinished(info);

	return prev;
}

// free the info structure's content
void Info_Free
(
	Info *info  // information object to free
) {
    ASSERT(info != NULL);

	// make sure every entry in working_queries is NULL
#ifdef RG_DEBUG
	uint nthreads = ThreadPools_ThreadCount() + 1;
	for(int i = 0; i < nthreads; i++) {
		ASSERT(info->working_queries[i] == NULL);
	}
#endif

	// free working query array
    rm_free(info->working_queries);

	// expecting no waiting queries
	//ASSERT(HashTableElemCount(info->waiting_queries) == 0);

	// free waiting queries container
    HashTableRelease(info->waiting_queries);

    int res = pthread_mutex_destroy(&info->mutex);
    ASSERT(res == 0);

	res = pthread_rwlock_destroy(&info->finished_queries_rwlock);
	ASSERT(res == 0);

	rm_free(info);
}

