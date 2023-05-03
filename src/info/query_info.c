/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <sys/types.h>

#include "qi.h"
#include "../util/simple_timer.h"
#include "../graph/graphcontext.h"
#include "../commands/cmd_context.h"

// resets the stage timer and returns the milliseconds it had recorded before
// having been reset
static void QueryInfo_ResetStageTimer
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    TIMER_RESTART(qi->stage_timer);
}

// returns the number of milliseconds the timer has counted
// this function does not reset the timer
static millis_t QueryInfo_GetCountedMilliseconds
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    millis_t ms = TIMER_GET_ELAPSED_MILLISECONDS(qi->stage_timer);
    QueryInfo_ResetStageTimer(qi);
	return ms;
}

// creates a new, empty query info object
QueryInfo *QueryInfo_New(void) {
    QueryInfo *qi = rm_calloc(1, sizeof(QueryInfo));
    return qi;
}

// returns the date/time when the query was received by the module
// in milliseconds from UNIX epoch
uint64_t QueryInfo_GetReceivedTimestamp
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->received_ts;
}

// returns the total time spent by a query waiting
// executing and reporting
millis_t QueryInfo_GetTotalTimeSpent
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->wait_duration + qi->execution_duration + qi->report_duration;
}

// returns the time the query spent waiting
millis_t QueryInfo_GetWaitingTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->wait_duration;
}

// returns the time the query spent executing
millis_t QueryInfo_GetExecutionTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);
	
	return qi->execution_duration;
}

// returns the time the query spent reporting.
millis_t QueryInfo_GetReportingTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->report_duration;
}

// reads the stage timer and updates the waiting time with it
void QueryInfo_UpdateWaitingTime
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    qi->wait_duration += QueryInfo_GetCountedMilliseconds(qi);
}

// sets the "utilized_cache" flag of a QueryInfo
void QueryInfo_SetUtilizedCache
(
    QueryInfo *qi,  // query info
    bool utilized   // cache utilized
) {
    qi->utilized_cache = utilized;
}

// reads the stage timer and updates the execution time with it.
void QueryInfo_UpdateExecutionTime
(
	QueryInfo *qi
) {
    ASSERT(qi);

    qi->execution_duration += QueryInfo_GetCountedMilliseconds(qi);
}

// reads the stage timer and updates the reporting time with it
void QueryInfo_UpdateReportingTime
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    qi->report_duration += QueryInfo_GetCountedMilliseconds(qi);
}

// clone a QueryInfo
QueryInfo *QueryInfo_Clone
(
    const QueryInfo *qi
) {
    ASSERT(qi != NULL);
    QueryInfo *clone = rm_malloc(sizeof(QueryInfo));
    *clone = *qi;

    if(qi->graph_name != NULL) {
        clone->graph_name = strdup(qi->graph_name);
    }
    if(qi->query_string != NULL) {
        clone->query_string = strdup(qi->query_string);
    }

    return clone;
}

// used as a callback for the circular buffer
void QueryInfo_CloneTo
(
    const void *item_to_clone,
    void *destination_item,
    void *user_data
) {
    ASSERT(item_to_clone);
    ASSERT(destination_item);
    UNUSED(user_data);

    QueryInfo *source = (QueryInfo*)item_to_clone;
    QueryInfo *destination = (QueryInfo*)destination_item;

    // copy the struct (shallow)
    *destination = *source;

    if (source->query_string != NULL) {
        destination->query_string = strdup(source->query_string);
    }

    if (source->graph_name != NULL) {
        destination->graph_name = strdup(source->graph_name);
    }
}

static void _write_query_info_to_stream
(
    RedisModuleCtx *ctx,
    QueryInfo *qi
){
    ASSERT(qi != NULL);

    // TODO: Write the info in pd->qi to the stream
    // use the RedisModuleCall-- function to call "XADD MAXLEN = Config_CMD_INFO_MAX_QUERY_COUNT"
    // so that the length of the stream does not exceed its max.

    // write to stream
        // "Received at"
        // value
        // "Stage"
        // value
        // "Graph name"
        // value
        // "Query"
        // value
        // "Total duration"
        // value
        // "Wait duration"
        // value
        // "Execution duration"
        // value
        // "Report duration"
        // value
        // "Cache utilized"
        // value

    int limit;
    Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &limit);

    // Get the current stream ID for the "append" operation
    RedisModuleCallReply *reply = RedisModule_Call(ctx,
    "XADD",
    "cclcccccclclclclclcl",
    GRAPH_INFO_STREAM_NAME,
    "MAXLEN", limit,
    "*",
    "graph_name", qi->graph_name,
    "query_string", qi->query_string,
    "received_ts", (long long)qi->received_ts,
    "wait_duration", (long long)qi->wait_duration,
    "execution_duration", (long long)qi->execution_duration,
    "report_duration", (long long)qi->report_duration,
    "stage", qi->stage,
    "utilized_cache", qi->utilized_cache ? "true" : "false");

    // Check for errors and clean up
    if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
        RedisModule_Log(ctx, "error", "failed to append to stream: %s", RedisModule_CallReplyStringPtr(reply, NULL));
    }
    RedisModule_FreeCallReply(reply);
}

// write the info stored in a Queryinfo to a stream and free it
// executed by the main-thread when the client is unblocked
void QueryInfo_ReportAndFree
(
    RedisModuleCtx *ctx,  // module context
    void *privdata        // private data
) {
    QueryInfo *qi = (QueryInfo *)privdata;
    _write_query_info_to_stream(ctx, qi);

    // free the QueryInfo
    QueryInfo_Free(qi);
}

// QueryInfo deleter callback
void QueryInfo_Deleter
(
    void *info
) {
    ASSERT(info != NULL);
    QueryInfo *qi = (QueryInfo *)info;
    QueryInfo_Free(qi);
}

// free a QueryInfo
void QueryInfo_Free
(
    QueryInfo *qi
) {
    if (qi->query_string != NULL) {
        free(qi->query_string);
    }
    if (qi->graph_name != NULL) {
        free(qi->graph_name);
    }
}

