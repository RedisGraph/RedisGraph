/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../../ops/op.h"
#include "../../../record.h"

typedef enum {
	OP_DEPLETED = 1,
	OP_REFRESH = 2,
	OP_OK = 4,
	OP_ERR = 8,
} RT_OpResult;

struct RT_OpBase;
struct RT_ExecutionPlan;

typedef void (*RT_fpFree)(struct RT_OpBase *);
typedef RT_OpResult(*RT_fpInit)(struct RT_OpBase *);
typedef Record(*RT_fpConsume)(struct RT_OpBase *);
typedef RT_OpResult(*RT_fpReset)(struct RT_OpBase *);
typedef void (*RT_fpToString)(const struct RT_OpBase *, sds *);
typedef struct RT_OpBase *(*RT_fpClone)(const struct RT_ExecutionPlan *, const struct RT_OpBase *);

// Execution plan operation statistics.
typedef struct {
	int profileRecordCount;     // Number of records generated.
	double profileExecTime;     // Operation total execution time in ms.
}  OpStats;

struct RT_OpBase {
	OPType type;                 // Type of operation
	RT_fpInit init;              // Called once before execution
	RT_fpFree free;              // Free operation
	RT_fpReset reset;            // Reset operation state
	RT_fpClone clone;            // Operation clone
	RT_fpConsume consume;        // Produce next record
	RT_fpConsume profile;        // Profiled version of consume
	int childCount;              // Number of children
	bool op_initialized;         // True if the operation has already been initialized
	struct RT_OpBase **children; // Child operations
	const char **modifies;       // List of entities this op modifies
	OpStats *stats;              // Profiling statistics
	struct RT_OpBase *parent;    // Parent operations
	const struct RT_ExecutionPlan *plan; // ExecutionPlan this operation is part of
	bool writer;                 // Indicates this is a writer operation
};
typedef struct RT_OpBase RT_OpBase;

// Initialize op.
void RT_OpBase_Init(RT_OpBase *op, OPType type, RT_fpInit init, RT_fpConsume consume,
				 RT_fpReset reset, RT_fpClone, RT_fpFree free, bool writer,
				 const struct RT_ExecutionPlan *plan);
void RT_OpBase_Free(RT_OpBase *op);       // Free op
Record RT_OpBase_Consume(RT_OpBase *op);  // Consume op
Record RT_OpBase_Profile(RT_OpBase *op);  // Profile op

RT_OpBase *RT_OpBase_Clone(const struct RT_ExecutionPlan *plan, const RT_OpBase *op);

// returns operation type
OPType RT_OpBase_Type(const RT_OpBase *op);

// returns true if op is aware of alias
// an operation is aware of all aliases it modifies and all aliases
// modified by prior operation within its segment
bool RT_OpBase_Aware(RT_OpBase *op, const char *alias, uint *idx);

// sends free request to each operation up the chain
void RT_OpBase_PropagateFree(RT_OpBase *op);

// sends reset request to each operation up the chain
void RT_OpBase_PropagateReset(RT_OpBase *op);

// indicates if the operation is a writer operation
bool RT_OpBase_IsWriter(RT_OpBase *op);

// update operation consume function
void RT_OpBase_UpdateConsume(RT_OpBase *op, RT_fpConsume consume);

// creates a new record that will be populated during execution
Record RT_OpBase_CreateRecord(const RT_OpBase *op);

// clones given record
Record RT_OpBase_CloneRecord(Record r);

// release record
void RT_OpBase_DeleteRecord(Record r);
