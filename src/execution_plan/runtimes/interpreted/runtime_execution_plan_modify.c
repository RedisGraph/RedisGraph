#include "runtime_execution_plan_modify.h"
#include "../../../util/rmalloc.h"

static void _OpBase_AddChild(RT_OpBase *parent, RT_OpBase *child) {
	// Add child to parent
	if(parent->children == NULL) {
		parent->children = rm_malloc(sizeof(RT_OpBase *));
	} else {
		parent->children = rm_realloc(parent->children, sizeof(RT_OpBase *) * (parent->childCount + 1));
	}
	parent->children[parent->childCount++] = child;

	// Add parent to child
	child->parent = parent;
}

inline void RT_ExecutionPlan_AddOp(RT_OpBase *parent, RT_OpBase *newOp) {
	_OpBase_AddChild(parent, newOp);
}
