/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimizations_util.h"

void re_order_filter_op(ExecutionPlan *plan, OpBase *root, OpFilter *filter) {
	FT_FilterNode *additional_filter_tree = filter->filterTree;

	rax *references = FilterTree_CollectModified(additional_filter_tree);
	OpBase *op = ExecutionPlan_LocateReferences(root, NULL, references);
	ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
	ExecutionPlan_PushBelow(op, (OpBase *)filter);
	raxFree(references);
}

// Returns true if the stream resolves all required entities.
static bool _stream_resolves_entities(rax *stream_resolves, rax *entities_to_resolve) {
	bool resolved_all = true;
	raxIterator it;
	raxStart(&it, entities_to_resolve);

	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		if(raxFind(stream_resolves, it.key, it.key_len) == raxNotFound) {
			resolved_all = false;
			break;
		}
	}

	raxStop(&it);
	return resolved_all;
}

int relate_exp_to_stream(AR_ExpNode *exp, rax **stream_entities, int stream_count) {
	// Collect the referenced entities in the expression.
	rax *entities = raxNew();
	AR_EXP_CollectEntities(exp, entities);

	int stream_num;
	for(stream_num = 0; stream_num < stream_count; stream_num ++) {
		// See if the stream resolves all of the references.
		if(_stream_resolves_entities(stream_entities[stream_num], entities)) break;
	}
	raxFree(entities);

	if(stream_num == stream_count) return NOT_RESOLVED; // No single stream resolves all references.
	return stream_num;
}

void build_streams_from_op(OpBase *op, rax **streams, int stream_count) {
	assert(stream_count <= op->childCount);
	for(int j = 0; j < stream_count; j ++) {
		streams[j] = raxNew();
		ExecutionPlan_BoundVariables(op->children[j], streams[j]);
	}
}
