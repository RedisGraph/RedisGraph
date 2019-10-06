/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __SI_AGREGATE_H__
#define __SI_AGREGATE_H__

#include <stdlib.h>

#include "agg_ctx.h"
#include "../value.h"

#define AGG_OK 1
#define AGG_SKIP 2
#define AGG_EOF 0
#define AGG_ERR -1

#define AGG_STATE_INIT 0
#define AGG_STATE_DONE 1
#define AGG_STATE_ERR 2
#define AGG_STATE_EOF 3

typedef int (*StepFunc)(AggCtx *ctx, SIValue *argv, int argc);  // Aggregatoin step function.
typedef int (*FinalizeFunc)(AggCtx *ctx);                       // Aggregation finalize function.
// Aggregation context inner data creation function.
typedef void *(*AggCtx_PrivateData_New)(AggCtx *ctx);
// Aggregation context inner data free function.
typedef void (*AggCtx_PrivateData_Free)(AggCtx *ctx);

/**
 * @brief  Creates a new aggregation context, with the given step and finalize functions.
 *         The aggregation context creates and free its own private data, with the supplied functions.
 * @param  step: Aggregation step function.
 * @param  finalize: Aggregation finalize function.
 * @param  privateDataNew: New private data creation function.
 * @param  privateDataFree: Private data free function.
 * @param  isDistinct: Indicates if the aggregation is done over distinct values or not.
 * @retval New aggregation context.
 */
AggCtx *Agg_NewCtx(StepFunc step, FinalizeFunc finalize, AggCtx_PrivateData_New privateDataNew,
				   AggCtx_PrivateData_Free privateDataFree, bool isDistinct);

/**
 * @brief  Clones an aggregation context. This will duplicate the original context methods
 *         and isDistinct indicator, and will create a new inner data.
 * @param  *ctx: Original aggregation context.
 * @retval New aggregation context.
 */
AggCtx *Agg_CloneCtx(AggCtx *ctx);

/**
 * @brief  Frees an aggregation context.
 * @param  *ctx: Aggregation context.
 * @retval None
 */
void AggCtx_Free(AggCtx *ctx);

/**
 * @brief  Sets an aggregation error in case of an exception and returns AGG_ERR.
 * @param  *ctx: Aggregation context
 * @param  *err: Aggregation error value.
 * @retval AGG_ERR error code.
 */
int Agg_SetError(AggCtx *ctx, AggError *err);

/**
 * @brief  Returns the inner data member of the aggregation context.
 * @param  *ctx: Aggregation context.
 * @retval Inner data.
 */
void *Agg_FuncCtx(AggCtx *ctx);

/**
 * @brief  Set the result of the aggregation in the aggregation context.
 * @param  *ctx: Aggregation context.
 * @param  v: Aggregation result.
 * @retval None
 */
void Agg_SetResult(AggCtx *ctx, SIValue v);

/**
 * @brief  Executes a calculation step over a given SIValue tuple.
 * @param  *ctx: Aggregation context.
 * @param  *argv: SIValue tuple.
 * @param  argc: Tuple size.
 * @retval AGG_OK in case of succsess, AGG_ERR in case of an exception.
 */
int Agg_Step(AggCtx *ctx, SIValue *argv, int argc);

/**
 * @brief  Finalize the aggregation calculation and sets the finale result.
 * @param  *ctx: Aggregation context.
 * @retval AGG_OK
 */
int Agg_Finalize(AggCtx *ctx);
#endif
