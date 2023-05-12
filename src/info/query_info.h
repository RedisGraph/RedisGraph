/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include "util/simple_timer.h"

typedef uint32_t millis_t;

// a stage a query may be in
typedef enum QueryStage {
    QueryStage_WAITING   = 1 << 0,
    QueryStage_EXECUTING = 1 << 1,
    QueryStage_REPORTING = 1 << 2,
    QueryStage_FINISHED  = 1 << 3
} QueryStage;

// holds necessary per-query info
typedef struct QueryInfo {
    char *graph_name;            // graph name
    char *query_string;          // query string
    uint64_t received_ts;        // query received timestamp
    millis_t wait_duration;      // waiting time
    millis_t execution_duration; // executing time
    millis_t report_duration;    // reporting time
    QueryStage stage;            // query stage
    simple_timer_t timer;        // timer
    bool utilized_cache;         // utilized cache
} QueryInfo;

// creates a new, empty query info object
QueryInfo *QueryInfo_New(void);

// advance query's stage
// waiting   -> executing
// executing -> reporting
// reporting -> finished
void QueryInfo_AdvanceStage
(
	QueryInfo *qi  // query info
);

// returns the date/time when the query was received by the module
// in milliseconds from UNIX epoch
uint64_t QueryInfo_GetReceivedTimestamp
(
	const QueryInfo *qi
);

// returns the total time spent by a query waiting
// executing and reporting
millis_t QueryInfo_GetTotalTimeSpent
(
	const QueryInfo *qi
);

// returns the time the query spent waiting
millis_t QueryInfo_GetWaitingTime
(
	const QueryInfo *qi
);

// returns the time the query spent executing
millis_t QueryInfo_GetExecutionTime
(
	const QueryInfo *qi
);

// returns the time the query spent reporting.
millis_t QueryInfo_GetReportingTime
(
	const QueryInfo *qi
);

// sets the "utilized_cache" flag of a QueryInfo
void QueryInfo_SetUtilizedCache
(
    QueryInfo *qi,  // query info
    bool utilized   // cache utilized
);

// reads the stage timer and updates the waiting time with it
millis_t QueryInfo_UpdateWaitingTime
(
	QueryInfo *qi
);

// reads the stage timer and updates the execution time with it
millis_t QueryInfo_UpdateExecutionTime
(
	QueryInfo *qi
);

// reads the stage timer and updates the reporting time with it
millis_t QueryInfo_UpdateReportingTime
(
	QueryInfo *qi
);

// clone a QueryInfo
QueryInfo *QueryInfo_Clone
(
    QueryInfo *qi
);

// free a QueryInfo
void QueryInfo_Free
(
    QueryInfo *qi
);

