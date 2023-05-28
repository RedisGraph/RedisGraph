/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../ops/op.h"
typedef struct ExecutionPlan ExecutionPlan;

//------------------------------------------------------------------------------
// Helper functions to move and analyze operations in an ExecutionPlan.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  API for restructuring the op tree.
//------------------------------------------------------------------------------

/* Removes operation from execution plan. */
void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op);

/* Detaches operation from its parent. */
void ExecutionPlan_DetachOp(OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp);

// adds child to be the ind'th child of parent
void ExecutionPlan_AddOpInd
(
	OpBase *parent,  // parent op
	OpBase *child,   // child op
	uint ind         // index of child
);

/* Push b right below a. */
void ExecutionPlan_PushBelow(OpBase *a, OpBase *b);

/* Introduce new_root as the parent of old_root. */
void ExecutionPlan_NewRoot(OpBase *old_root, OpBase *new_root);

/* Update the root op of the execution plan. */
void ExecutionPlan_UpdateRoot(ExecutionPlan *plan, OpBase *new_root);

/* Replace a with b. */
void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b);

//------------------------------------------------------------------------------
// ExecutionPlan_Locate API:
// For performing existence checks and looking up individual operations in tree.
//------------------------------------------------------------------------------

/* Traverse upwards until an operation that resolves the given alias is found.
 * Returns NULL if alias is not resolved. */
OpBase *ExecutionPlan_LocateOpResolvingAlias(OpBase *root, const char *alias);

// Locate the first operation matching one of the given types in the op tree by
// performing DFS. Returns NULL if no matching operation was found
OpBase *ExecutionPlan_LocateOpMatchingType(OpBase *root, const OPType *types, uint type_count);

/* Convenience wrapper around ExecutionPlan_LocateOpMatchingType for lookups of a single type.
 * Locate the first operation of a given type within execution plan by performing DFS.
 * Returns NULL if operation wasn't found. */
OpBase *ExecutionPlan_LocateOp(OpBase *root, OPType type);

// returns all operations of a certain type in a execution plan
void ExecutionPlan_LocateOps
(
	OpBase ***plans,  // array in which ops are stored
	OpBase *root,     // root operation of the plan to traverse
	OPType type       // operation type to search
);

/* Find the earliest operation above the provided recurse_limit, if any,
 * at which all references are resolved. */
OpBase *ExecutionPlan_LocateReferences(OpBase *root, const OpBase *recurse_limit,
									   rax *references_to_resolve);

/* Find the earliest operation at which all references are resolved, if any,
 * both above the provided recurse_limit and without recursing past a blacklisted op. */
OpBase *ExecutionPlan_LocateReferencesExcludingOps(OpBase *root,
		const OpBase *recurse_limit, const OPType *blacklisted_ops,
		int nblacklisted_ops, rax *refs_to_resolve);

//------------------------------------------------------------------------------
// ExecutionPlan_Collect API:
// For collecting all matching operations in tree.
//------------------------------------------------------------------------------

/* Collect all operations matching the given types in the op tree.
 * Returns an array of operations. */
OpBase **ExecutionPlan_CollectOpsMatchingType(OpBase *root, const OPType *types, uint type_count);

/* Convenience wrapper around ExecutionPlan_LocateOpMatchingType for
 * collecting all operations of a given type within the op tree.
 * Returns an array of operations. */
OpBase **ExecutionPlan_CollectOps(OpBase *root, OPType type);

//------------------------------------------------------------------------------
// API for building and relocating operations in transient ExecutionPlans.
//------------------------------------------------------------------------------

/* Populate a rax with all aliases that have been resolved by the given operation
 * and its children. These are the bound variables at this point in execution, and
 * subsequent operations should not introduce them as new entities. For example, in the query:
 * MATCH (a:A) CREATE (a)-[:E]->(b:B)
 * The Create operation should never introduce a new node 'a'. */
void ExecutionPlan_BoundVariables(const OpBase *op, rax *modifiers);

// For all ops in the given tree, associate the provided ExecutionPlan.
// This is for use for updating ops that have been built with a temporary
// ExecutionPlan.
// if qg is set, merge the query graphs of the temporary and main plans
void ExecutionPlan_BindPlanToOps
(
	ExecutionPlan *plan,  // plan to bind the operations to
	OpBase *root,         // root operation
	bool qg               // whether to merge QueryGraphs or not
);

// bind an operation to a plan
void ExecutionPlan_bindOpToPlan
(
	OpBase *op,          // operation to bind
	ExecutionPlan *plan  // plan to bind the op to
);

/* Given an AST path pattern, generate the tree of scan, traverse,
 * and filter operations required to represent it. */
OpBase *ExecutionPlan_BuildOpsFromPath(ExecutionPlan *plan, const char **vars,
									   const cypher_astnode_t *path);

