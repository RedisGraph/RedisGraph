/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __AGG_CTX_H__
#define __AGG_CTX_H__

#include "../value.h"

typedef char AggError;

struct AggCtx {
	void *fctx;
	AggError *err;
	SIValue result;
	int (*Step)(struct AggCtx *ctx, SIValue *argv, int argc);
	int (*Finalize)(struct AggCtx *ctx);
	void *(*AggCtx_PrivateData_New)();
	void (*AggCtx_PrivateData_Free)(struct AggCtx *ctx);
	bool isDistinct;
};
typedef struct AggCtx AggCtx;

#endif