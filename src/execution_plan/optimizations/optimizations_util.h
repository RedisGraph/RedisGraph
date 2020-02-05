/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"
#include "../ops/op_filter.h"

// To be used as a possible output of Relate_exp_to_stream.
#define NOT_RESOLVED -1


/**
 * @brief  This function will try to re-position a given filter.
 *         The filter will be placed at the earliest position along
 *         the execution plan where all references are resolved.
 * @param  *plan: Execution plan to modify.
 * @param  *root: Start operation for locating optimal place for the filter op.
 * @param  *filter: Filter operation to place.
 * @retval None
 */
void re_order_filter_op(ExecutionPlan *plan, OpBase *root, OpFilter *filter);

/*  */
/**
 * @brief Given an expression node from a filter tree, returns the stream number
 *        that fully resolves the expression's references.
 * @param  *exp: Filter tree expression node.
 * @param  **stream_entities: Streams to search the expressions referenced entities.
 * @param  stream_count: Amount of stream to search in (Left-to-Right).
 * @retval Stream index if found. NOT_RESOLVED if non of the stream resolve the expression.
 */
int relate_exp_to_stream(AR_ExpNode *exp, rax **stream_entities, int stream_count);

/**
 * @brief  This function generates a rax containing the bound variables of each
 *         child stream of the op.
 * @note   Streams array should be allocated as rax array with
 *         size of at least stream_count.
 * @param  *op: Input operation.
 * @param  **streams: Array of rax (pre-allocated).
 * @param  stream_count: Amount of op children to process (Left-to-Right order).
 * @retval None
 */
void build_streams_from_op(OpBase *op, rax **streams, int stream_count);
