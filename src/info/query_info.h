/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <sys/types.h>

#include <stdbool.h>
#include "util/simple_timer.h"

typedef uint32_t millis_t;

// a stage a query may be in
typedef enum QueryStage {
    QueryStage_WAITING = 0,
    QueryStage_EXECUTING,
    QueryStage_REPORTING,
    QueryStage_FINISHED,
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
    simple_timer_t stage_timer;  // timer
    bool utilized_cache;         // utilized cache
} QueryInfo;

// creates a new, empty query info object
QueryInfo *QueryInfo_New(void);

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
void QueryInfo_UpdateWaitingTime
(
	QueryInfo *qi
);

// reads the stage timer and updates the execution time with it
void QueryInfo_UpdateExecutionTime
(
	QueryInfo *qi
);

// reads the stage timer and updates the reporting time with it
void QueryInfo_UpdateReportingTime
(
	QueryInfo *qi
);

// clone a QueryInfo
QueryInfo *QueryInfo_Clone
(
    QueryInfo *qi
);

// used as a callback for the circular buffer
void QueryInfo_CloneTo
(
    const void *item_to_clone,
    void *destination_item,
    void *user_data
);

// QueryInfo deleter callback
void QueryInfo_Deleter
(
    void *info
);

// free a QueryInfo
void QueryInfo_Free
(
    QueryInfo *qi
);
