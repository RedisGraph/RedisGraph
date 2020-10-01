/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../record.h"
#include "../../util/arr.h"
#include "../../redismodule.h"
#include "../../schema/schema.h"
#include "../../graph/query_graph.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"

#define OP_REQUIRE_NEW_DATA(opRes) (opRes & (OP_DEPLETED | OP_REFRESH)) > 0

typedef enum {
	OPType_ALL_NODE_SCAN,
	OPType_NODE_BY_LABEL_SCAN,
	OPType_INDEX_SCAN,
	OPType_NODE_BY_ID_SEEK,
	OPType_NODE_BY_LABEL_AND_ID_SCAN,
	OPType_EXPAND_INTO,
	OPType_CONDITIONAL_TRAVERSE,
	OPType_CONDITIONAL_VAR_LEN_TRAVERSE,
	OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO,
	OPType_RESULTS,
	OPType_PROJECT,
	OPType_AGGREGATE,
	OPType_SORT,
	OPType_SKIP,
	OPType_LIMIT,
	OPType_DISTINCT,
	OPType_MERGE,
	OPType_MERGE_CREATE,
	OPType_FILTER,
	OPType_CREATE,
	OPType_UPDATE,
	OPType_DELETE,
	OPType_UNWIND,
	OPType_PROC_CALL,
	OPType_ARGUMENT,
	OPType_CARTESIAN_PRODUCT,
	OPType_VALUE_HASH_JOIN,
	OPType_APPLY,
	OPType_JOIN,
	OPType_SEMI_APPLY,
	OPType_ANTI_SEMI_APPLY,
	OPType_OR_APPLY_MULTIPLEXER,
	OPType_AND_APPLY_MULTIPLEXER,
	OPType_OPTIONAL,
} OPType;

typedef enum {
	OP_DEPLETED = 1,
	OP_REFRESH = 2,
	OP_OK = 4,
	OP_ERR = 8,
} OpResult;

// Macro for checking whether an operation is an Apply variant.
#define OP_IS_APPLY(op) ((op)->type == OPType_OR_APPLY_MULTIPLEXER || (op)->type == OPType_AND_APPLY_MULTIPLEXER || (op)->type == OPType_SEMI_APPLY || (op)->type == OPType_ANTI_SEMI_APPLY)

#define PROJECT_OP_COUNT 2
static const OPType PROJECT_OPS[] = {OPType_PROJECT, OPType_AGGREGATE};

#define TRAVERSE_OP_COUNT 2
static const OPType TRAVERSE_OPS[] = {OPType_CONDITIONAL_TRAVERSE, OPType_CONDITIONAL_VAR_LEN_TRAVERSE};

#define SCAN_OP_COUNT 5
static const OPType SCAN_OPS[] = {OPType_ALL_NODE_SCAN, OPType_NODE_BY_LABEL_SCAN, OPType_INDEX_SCAN, OPType_NODE_BY_ID_SEEK, OPType_NODE_BY_LABEL_AND_ID_SCAN};

/* When placing filters, we should not recurse into certain operation's subtrees that would cause
 * logical errors. The cases we currently need to be concerned with are:
 * Merge - the results which should only be filtered after the entity is matched or created - and
 * Apply - which has an Optional child that should project results or NULL before being filtered.
 * The family of SemiApply ops (including the Apply Multiplexers) does not require this restriction
 * since they are always exclusively performing filtering. */
#define FILTER_RECURSE_BLACKLIST_COUNT 2
static const OPType FILTER_RECURSE_BLACKLIST[] = {OPType_APPLY, OPType_MERGE};

struct OpBase;
struct ExecutionPlan;

typedef void (*fpFree)(struct OpBase *);
typedef OpResult(*fpInit)(struct OpBase *);
typedef Record(*fpConsume)(struct OpBase *);
typedef OpResult(*fpReset)(struct OpBase *);
typedef int (*fpToString)(const struct OpBase *, char *, uint);
typedef struct OpBase *(*fpClone)(const struct ExecutionPlan *, const struct OpBase *);

// Execution plan operation statistics.
typedef struct {
	int profileRecordCount;     // Number of records generated.
	double profileExecTime;     // Operation total execution time in ms.
}  OpStats;

struct OpBase {
	OPType type;                // Type of operation.
	fpInit init;                // Called once before execution.
	fpFree free;                // Free operation.
	fpReset reset;              // Reset operation state.
	fpClone clone;              // Operation clone.
	fpConsume consume;          // Produce next record.
	fpConsume profile;          // Profiled version of consume.
	fpToString toString;        // Operation string representation.
	const char *name;           // Operation name.
	int childCount;             // Number of children.
	bool op_initialized;        // True if the operation has already been initialized.
	struct OpBase **children;   // Child operations.
	const char **modifies;      // List of entities this op modifies.
	OpStats *stats;             // Profiling statistics.
	struct OpBase *parent;      // Parent operations.
	const struct ExecutionPlan *plan; // ExecutionPlan this operation is part of.
	bool writer;             // Indicates this is a writer operation.
};
typedef struct OpBase OpBase;

// Initialize op.
void OpBase_Init(OpBase *op, OPType type, const char *name, fpInit init, fpConsume consume,
				 fpReset reset, fpToString toString, fpClone, fpFree free, bool writer,
				 const struct ExecutionPlan *plan);
void OpBase_Free(OpBase *op);       // Free op.
Record OpBase_Consume(OpBase *op);  // Consume op.
Record OpBase_Profile(OpBase *op);  // Profile op.

int OpBase_ToString(const OpBase *op, char *buff, uint buff_len);

OpBase *OpBase_Clone(const struct ExecutionPlan *plan, const OpBase *op);

/* Mark alias as being modified by operation.
 * Returns the ID associated with alias. */
int OpBase_Modifies(OpBase *op, const char *alias);

/* Adds an alias to an existing modifier, such that record[modifier] = record[alias]. */
int OpBase_AliasModifier(OpBase *op, const char *modifier, const char *alias);

/* Returns true if op is aware of alias.
 * an operation is aware of all aliases it modifies and all aliases
 * modified by prior operation within its segment. */
bool OpBase_Aware(OpBase *op, const char *alias, int *idx);

void OpBase_PropagateFree(OpBase *op); // Sends free request to each operation up the chain.
void OpBase_PropagateReset(OpBase *op); // Sends reset request to each operation up the chain.

// Indicates if the operation is a writer operation.
bool OpBase_IsWriter(OpBase *op);

// Update operation consume function.
void OpBase_UpdateConsume(OpBase *op, fpConsume consume);

// Creates a new record that will be populated during execution.
Record OpBase_CreateRecord(const OpBase *op);

// Clones given record.
Record OpBase_CloneRecord(Record r);

// Release record.
void OpBase_DeleteRecord(Record r);

