/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "cron.h"
#include "util/rmalloc.h"
#include "configuration/config.h"
#include "tasks/stream_finished_queries.h"

// add recurring tasks
void Cron_AddRecurringTasks(void) {
	//--------------------------------------------------------------------------
	// add query streaming task
	//--------------------------------------------------------------------------

	// make sure info tracking is enabled
	bool info_enabled = false;
	if(Config_Option_get(Config_CMD_INFO, &info_enabled) && info_enabled) {
		// create task context
		StreamFinishedQueryCtx *ctx = rm_calloc(1,
				sizeof(StreamFinishedQueryCtx));

		// add recurring task
		Cron_AddTask(0, CronTask_streamFinishedQueries, (void*)ctx);
	}
}

