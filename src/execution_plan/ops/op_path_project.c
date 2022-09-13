/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "op_path_project.h"
#include "../../datatypes/path/sipath_builder.h"

// Forward declarations
static Record PathProjectConsume(OpBase *opBase);
static OpBase *PathProjectClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewPathProjectOp(const ExecutionPlan *plan, QGPath **paths) {
	OpPathProject *op = rm_malloc(sizeof(OpPathProject));
	op->paths = paths;
	uint count = array_len(paths);
	op->record_idx = array_new(int, count);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PATH_PROJECT, "Path Project", NULL,
				PathProjectConsume, NULL, NULL, PathProjectClone, NULL, false, plan);

	for(uint i = 0; i < count; i ++) {
		op->record_idx[i] = OpBase_Modifies((OpBase *)op, paths[i]->alias);
	}

	return (OpBase *)op;
}

static Record PathProjectConsume(OpBase *opBase) {
	OpPathProject *op = (OpPathProject *)opBase;
	OpBase *child = op->op.children[0];
	Record r = OpBase_Consume(child);
	if(!r) return NULL;

	uint count = array_len(op->paths);
	for(uint i = 0; i < count; i ++) {
		uint nodes_count = array_len(op->paths[i]->nodes);
		uint edges_count = array_len(op->paths[i]->edges);
		uint nelements = nodes_count + edges_count;
		SIValue path = SIPathBuilder_New(nelements);
		int idx = 0;
		OpBase_Aware(opBase, op->paths[i]->nodes[0]->alias, &idx);
		SIValue element = Record_Get(r, idx);
		if(SI_TYPE(element) == T_NULL) {
			/* If any element of the path does not exist, the entire path is invalid.
			* Free it and return a null value. */
			SIValue_Free(path);
			path = SI_NullVal();
		}
		if(SI_TYPE(path) != T_NULL) {
			SIPathBuilder_AppendNode(path, element);
			for(uint j = 1; j < nodes_count; j++) {
				OpBase_Aware(opBase, op->paths[i]->edges[j - 1]->alias, &idx);
				element = Record_Get(r, idx);
				if(SI_TYPE(element) == T_NULL) {
					/* If any element of the path does not exist, the entire path is invalid.
					* Free it and return a null value. */
					SIValue_Free(path);
					path = SI_NullVal();
					break;
				}

				if(SI_TYPE(element) == T_PATH) {
					if(SIPath_Length(element) == 0) {
						continue;
					}
					SIPathBuilder_AppendPath(path, element, false);
				} else {
					SIPathBuilder_AppendEdge(path, element, false);
				}

				OpBase_Aware(opBase, op->paths[i]->nodes[j]->alias, &idx);
				element = Record_Get(r, idx);
				if(SI_TYPE(element) == T_NULL) {
					/* If any element of the path does not exist, the entire path is invalid.
					* Free it and return a null value. */
					SIValue_Free(path);
					path = SI_NullVal();
					break;
				}

				SIPathBuilder_AppendNode(path, element);
			}
		}
		Record_Add(r, op->record_idx[i], path);
	}
	
	
	return r;
}

static inline OpBase *PathProjectClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_PATH_PROJECT);
	const OpPathProject *op = (const OpPathProject *)opBase;
	return NewPathProjectOp(plan, op->paths);
}

