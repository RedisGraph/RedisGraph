/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include "RG.h"
#include "util/thpool/pools.h"
#include "commands/cmd_context.h"

extern CommandCtx **command_ctxs;

static struct sigaction old_act;

static void startCrashReport(void) {
	RedisModule_Log(NULL, "warning", "=== REDISGRAPH BUG REPORT START: ===");
}

static void endCrashReport(void) {
	RedisModule_Log(NULL, "warning", "=== REDISGRAPH BUG REPORT END. ===");
}

static void logCommands(void) {
	// #readers + #writers + Redis main thread
	int nthreads = ThreadPools_ThreadCount() + 1;

	for(int i = 0; i < nthreads; i++) {
		CommandCtx *cmd = command_ctxs[i];
		if(cmd != NULL) {
			RedisModule_Log(NULL, "warning", "%s %s", cmd->command_name,
					cmd->query);
		}
	}
}

void InfoFunc(RedisModuleInfoCtx *ctx, int for_crash_report) {
	// make sure information is requested for crash report
	if(!for_crash_report) return;

	// pause all working threads
	// NOTE: pausing is not an atomic action;
	// other threads can potentially change states before being interrupted.
	ThreadPools_Pause();

	char *command_desc = NULL;
	// #readers + #writers + Redis main thread
	int nthreads = ThreadPools_ThreadCount() + 1;

	RedisModule_InfoAddSection(ctx, "executing commands");

	for(int i = 0; i < nthreads; i++) {
		CommandCtx *cmd = command_ctxs[i];
		if(cmd != NULL) {
			asprintf(&command_desc, "%s %s", cmd->command_name, cmd->query);
			RedisModule_InfoAddFieldCString(ctx, "command", command_desc);
			free(command_desc);
		}
	}
}

void crashHandler(int sig, siginfo_t *info, void *ucontext) {
	// pause all working threads
	// NOTE: pausing is an async operation
	ThreadPools_Pause();

	startCrashReport();

	// log currently executing GRAPH commands
	logCommands();

	endCrashReport();

	// call previous (Redis original) handler
	(*old_act.sa_sigaction)(sig, info, ucontext);
}

void setupCrashHandlers(RedisModuleCtx *ctx) {
	// if RedisModule_RegisterInfoFunc is available use it
	// to report RedisGraph additional information in case of a crash
	// otherwise overwrite Redis signal handler

	if(RedisModule_RegisterInfoFunc) {
		int registered = RedisModule_RegisterInfoFunc(ctx, InfoFunc);
		ASSERT(registered == REDISMODULE_OK);
	} else {
		// RegisterInfoFunc is not available, replace redis
		// SIGSEGV signal handler

		struct sigaction act;

		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
		act.sa_sigaction = crashHandler;

		sigaction(SIGSEGV, &act, &old_act);
	}
}

