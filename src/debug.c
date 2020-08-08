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

static void logCommands(void) {
	int nthreads = thpool_size(_thpool);

	for(int i = 0; i < nthreads; i++) {
		CommandCtx *cmd = command_ctxs[i];
		if(cmd != NULL) {
			printf("%s %s\n", cmd->command_name, cmd->query);
		}
	}
}

void sigsegvHandler(int sig, siginfo_t *info, void *ucontext) {
	// Log currently executing GRAPH commands
	logCommands();

	// Call previous handler
	(*old_act.sa_sigaction)(sig, info, ucontext);
}

void setupSignalHandlers(void) {
	struct sigaction act;

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	act.sa_sigaction = sigsegvHandler;

	sigaction(SIGSEGV, &act, &old_act);
}

