/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
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
	OPType_AGGREGATE = 1,
	OPType_ALL_NODE_SCAN = (1 << 1),
	OPType_CONDITIONAL_TRAVERSE = (1 << 2),
	OPType_CONDITIONAL_VAR_LEN_TRAVERSE = (1 << 3),
	OPType_FILTER = (1 << 4),
	OPType_NODE_BY_LABEL_SCAN = (1 << 5),
	OPType_INDEX_SCAN = (1 << 6),
	OPType_RESULTS = (1 << 7),
	OPType_CREATE = (1 << 8),
	OPType_UPDATE = (1 << 9),
	OPType_DELETE = (1 << 10),
	OPType_CARTESIAN_PRODUCT = (1 << 11),
	OPType_MERGE = (1 << 12),
	OPType_UNWIND = (1 << 13),
	OPType_SORT = (1 << 14),
	OPType_PROJECT = (1 << 15),
	OPType_SKIP = (1 << 16),
	OPType_LIMIT = (1 << 17),
	OPType_DISTINCT = (1 << 18),
	OPType_EXPAND_INTO = (1 << 19),
	OPType_NODE_BY_ID_SEEK = (1 << 20),
	OPType_PROC_CALL = (1 << 21),
	OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO = (1 << 22),
	OPType_VALUE_HASH_JOIN = (1 << 23),
	OPType_APPLY = (1 << 24),
	OPType_JOIN = (1 << 25),
	OPType_ARGUMENT = (1 << 26),
} OPType;

#define OP_SCAN (OPType_ALL_NODE_SCAN | OPType_NODE_BY_LABEL_SCAN | OPType_INDEX_SCAN | OPType_NODE_BY_ID_SEEK)
#define OP_TAPS (OP_SCAN | OPType_CREATE | OPType_UNWIND | OPType_MERGE)

typedef enum {
	OP_DEPLETED = 1,
	OP_REFRESH = 2,
	OP_OK = 4,
	OP_ERR = 8,
} OpResult;

struct OpBase;

typedef void (*fpFree)(struct OpBase *);
typedef OpResult(*fpInit)(struct OpBase *);
typedef Record(*fpConsume)(struct OpBase *);
typedef OpResult(*fpReset)(struct OpBase *);
typedef int (*fpToString)(const struct OpBase *, char *, uint);

// Execution plan operation statistics.
typedef struct {
	int profileRecordCount;     // Number of records generated.
	double profileExecTime;     // Operation total execution time in ms.
}  OpStats;

struct ExecutionPlan;

struct OpBase {
	OPType type;                // Type of operation.
	fpInit init;                // Called once before execution.
	fpFree free;                // Free operation.
	fpReset reset;              // Reset operation state.
	fpConsume consume;          // Produce next record.
	fpConsume profile;          // Profiled version of consume.
	fpToString toString;        // operation string representation.
	const char *name;           // Operation name.
	int childCount;             // Number of children.
	bool op_initialized;        // True if the operation has already been initialized.
	struct OpBase **children;   // Child operations.
	const char **modifies;      // List of entities this op modifies.
	OpStats *stats;             // Profiling statistics.
	Record *dangling_records;   // Records allocated by this operation that must be freed.
	struct OpBase *parent;      // Parent operations.
	const struct ExecutionPlan *plan; // ExecutionPlan this operation is part of.
};
typedef struct OpBase OpBase;

// Initialize op.
void OpBase_Init(OpBase *op, OPType type, const char *name, fpInit init, fpConsume consume,
				 fpReset reset,
				 fpToString toString, fpFree free, const struct ExecutionPlan *plan);
void OpBase_Free(OpBase *op);       // Free op.
Record OpBase_Consume(OpBase *op);  // Consume op.
Record OpBase_Profile(OpBase *op);  // Profile op.

int OpBase_ToString(const OpBase *op, char *buff, uint buff_len);

/* If an operation holds the sole reference to a Record it is evaluating,
 * that reference should be tracked so that it may be freed in the event of a run-time error. */
void OpBase_AddVolatileRecord(OpBase *op, const Record r);
/* No errors were encountered while processing these Records; the references
 * may be released. */
void OpBase_RemoveVolatileRecords(OpBase *op);

/* Mark alias as being modified by operation.
 * Returns the ID associated with alias. */
int OpBase_Modifies(OpBase *op, const char *alias);

/* Returns true if op is aware of alias.
 * an operation is aware of all aliases it modifies and all aliases
 * modified by prior operation within its segment. */
bool OpBase_Aware(OpBase *op, const char *alias, int *idx);

void OpBase_PropagateFree(OpBase *op); // Sends free request to each operation up the chain.
void OpBase_PropagateReset(OpBase *op); // Sends reset request to each operation up the chain.

// Creates a new record that will be populated during execution.
Record OpBase_CreateRecord(const OpBase *op);

