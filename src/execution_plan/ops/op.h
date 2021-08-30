/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include "../../util/sds/sds.h"

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

// Macro for checking whether an operation is an Apply variant.
#define OP_IS_APPLY(op) ((op)->type == OPType_OR_APPLY_MULTIPLEXER || (op)->type == OPType_AND_APPLY_MULTIPLEXER || (op)->type == OPType_SEMI_APPLY || (op)->type == OPType_ANTI_SEMI_APPLY)
#define RT_OP_IS_APPLY(op) ((op)->op_desc->type == OPType_OR_APPLY_MULTIPLEXER || (op)->op_desc->type == OPType_AND_APPLY_MULTIPLEXER || (op)->op_desc->type == OPType_SEMI_APPLY || (op)->op_desc->type == OPType_ANTI_SEMI_APPLY)

#define PROJECT_OP_COUNT 2
static const OPType PROJECT_OPS[] = {OPType_PROJECT, OPType_AGGREGATE};

#define TRAVERSE_OP_COUNT 2
static const OPType TRAVERSE_OPS[] = {OPType_CONDITIONAL_TRAVERSE, OPType_CONDITIONAL_VAR_LEN_TRAVERSE};

#define SCAN_OP_COUNT 5
static const OPType SCAN_OPS[] = {OPType_ALL_NODE_SCAN, OPType_NODE_BY_LABEL_SCAN, OPType_INDEX_SCAN, OPType_NODE_BY_ID_SEEK, OPType_NODE_BY_LABEL_AND_ID_SCAN};

#define BLACKLIST_OP_COUNT 2
static const OPType FILTER_RECURSE_BLACKLIST[] = {OPType_APPLY, OPType_MERGE};

#define EAGER_OP_COUNT 5
static const OPType EAGER_OPERATIONS[] = {OPType_AGGREGATE, OPType_CREATE, OPType_UPDATE, OPType_DELETE, OPType_MERGE};

struct OpBase;
struct ExecutionPlan;

typedef void (*fpFree)(struct OpBase *);


struct OpBase {
	OPType type;                // Type of operation
	fpFree free;                // Free operation
	const char *name;           // Operation name
	int childCount;             // Number of children
	struct OpBase **children;   // Child operations
	const char **modifies;      // List of entities this op modifies
	struct OpBase *parent;      // Parent operations
	const struct ExecutionPlan *plan; // ExecutionPlan this operation is part of
	bool writer;                // Indicates this is a writer operation
};
typedef struct OpBase OpBase;

// Initialize op
void OpBase_Init(OpBase *op, OPType type, const char *name,
	fpFree free, bool writer, const struct ExecutionPlan *plan);
void OpBase_Free(OpBase *op);       // Free op

// returns operation type
OPType OpBase_Type(const OpBase *op);

// Mark alias as being modified by operation
// Returns the ID associated with alias
int OpBase_Modifies(OpBase *op, const char *alias);

// Adds an alias to an existing modifier, such that record[modifier] = record[alias]
int OpBase_AliasModifier(OpBase *op, const char *modifier, const char *alias);

// Returns true if op is aware of alias
// an operation is aware of all aliases it modifies and all aliases
// modified by prior operation within its segment
bool OpBase_Aware(OpBase *op, const char *alias, uint *idx);

// Indicates if the operation is a writer operation
bool OpBase_IsWriter(OpBase *op);
