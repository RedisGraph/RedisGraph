/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "heap.h"
#include <stdbool.h>
#include <sys/types.h>

/* CRON is a task scheduler
 * a task is defined by:
 * when it should run; delta in ms from the time it's introduced
 * a callback to call when it is time to execute the task
 * and an optional private data passed to the callback */

// Cron task typedef
typedef void *CronTask;

// task callback function
typedef void (*CronTaskCB)(void *pdata);

// Start CRON, should be called once
void Cron_Start(void);

// Stop CRON
void Cron_Stop(void);

// Create a new CRON task and return it
CronTask Cron_AddTask
(
	uint when,      // number of milliseconds until task invocation
	CronTaskCB cb,  // callback to call when task is due
	void *pdata     // private data to pass to callback, owned by task
);

/* Attempt to remove the given task from the CRON queue,
 * return true if successful. */
bool Cron_RemoveTask
(
	CronTask task   // task to be removed
);

