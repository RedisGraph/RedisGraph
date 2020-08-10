/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include "util/thpool/thpool.h"
#include "commands/cmd_context.h"

extern threadpool _thpool;
extern CommandCtx **command_ctxs;

static struct sigaction old_act;

static void startCrashReport(void) {
	RedisModule_Log(NULL, "warning", "=== REDISGRAPH BUG REPORT START: ===");
}

static void endCrashReport(void) {
	RedisModule_Log(NULL, "warning", "=== REDISGRAPH BUG REPORT END. ===");
}

static void logCommands(void) {
	int nthreads = thpool_num_threads(_thpool);

	for(int i = 0; i < nthreads; i++) {
		CommandCtx *cmd = command_ctxs[i];
		if(cmd != NULL) {
			RedisModule_Log(NULL, "warning", "%s %s", cmd->command_name,
					cmd->query);
		}
	}
}

void sigsegvHandler(int sig, siginfo_t *info, void *ucontext) {
	// pause all working threads
	// NOTE: pausing is an async operation
	thpool_pause(_thpool);

	startCrashReport();

	// log currently executing GRAPH commands
	logCommands();

	endCrashReport();

	// call previous (Redis original) signal handler
	(*old_act.sa_sigaction)(sig, info, ucontext);
}

void setupSignalHandlers(void) {
	struct sigaction act;

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	act.sa_sigaction = sigsegvHandler;

	sigaction(SIGSEGV, &act, &old_act);
}

