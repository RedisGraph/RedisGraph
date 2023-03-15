/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// a stage a query may be in
typedef enum QueryStage {
    QueryStage_WAITING = 0,
    QueryStage_EXECUTING,
    QueryStage_REPORTING,
    QueryStage_FINISHED,
} QueryStage;

// holds necessary per-query info
typedef struct QueryInfo {
    uint64_t received_ts;        // query received timestamp
    millis_t wait_duration;      // waiting time
    millis_t execution_duration; // executing time
    millis_t report_duration;    // reporting time
    const QueryCtx *ctx;         // query context
    QueryStage stage;            // query stage
    simple_timer_t stage_timer;  // timer
} QueryInfo;

// creates a new, empty query info object
QueryInfo *QueryInfo_New(void);

// assigns the query context to the query info
void QueryInfo_SetQueryContext
(
	QueryInfo *qi,
	const QueryCtx *ctx
);

// returns the query context associated with the query info
const QueryCtx* QueryInfo_GetQueryContext
(
	const QueryInfo *qi
);

// returns true if the query info object is valid and can be worked with
bool QueryInfo_IsValid
(
	const QueryInfo *qi
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

// returns the number of milliseconds the timer has counted
// this function does not reset the timer
millis_t QueryInfo_GetCountedMilliseconds
(
	QueryInfo *qi
);

// resets the stage timer and returns the milliseconds it had recorded before
// having been reset
millis_t QueryInfo_ResetStageTimer
(
	QueryInfo *qi
);

