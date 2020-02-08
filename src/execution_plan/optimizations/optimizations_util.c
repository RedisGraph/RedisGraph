/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimizations_util.h"
#include "../../util/rax_extensions.h"

void OptimizeUtils_MigrateFilterOp(ExecutionPlan *plan, OpBase *root, OpFilter *filter) {
	rax *references = FilterTree_CollectModified(filter->filterTree);
	OpBase *op = ExecutionPlan_LocateReferences(root, NULL, references);
	if(op != filter->op.children[0]) {
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
		ExecutionPlan_PushBelow(op, (OpBase *)filter);
	}
	raxFree(references);
}

int OptimizeUtils_RelateExpToStream(AR_ExpNode *exp, rax **stream_entities, int stream_count) {
	// Collect the referenced entities in the expression.
	rax *entities = raxNew();
	AR_EXP_CollectEntities(exp, entities);

	int stream_num;
	for(stream_num = 0; stream_num < stream_count; stream_num ++) {
		// See if the stream resolves all of the references.
		if(raxIsSubset(stream_entities[stream_num], entities)) break;
	}
	raxFree(entities);

	if(stream_num == stream_count) return NOT_RESOLVED; // No single stream resolves all references.
	return stream_num;
}

void OptimizeUtils_BuildStreamFromOp(OpBase *op, rax **streams, int stream_count) {
	assert(stream_count <= op->childCount);
	for(int j = 0; j < stream_count; j ++) {
		streams[j] = raxNew();
		ExecutionPlan_BoundVariables(op->children[j], streams[j]);
	}
}
