/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../ops/op.h"
#include "rax.h"
#include "cypher-parser.h"

typedef struct RT_ExecutionPlan RT_ExecutionPlan;

//------------------------------------------------------------------------------
// Helper functions to move and analyze operations in an RT_ExecutionPlan.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  API for restructuring the op tree.
//------------------------------------------------------------------------------

/* Removes operation from execution plan. */
void RT_ExecutionPlan_RemoveOp(RT_ExecutionPlan *plan, RT_OpBase *op);

/* Detaches operation from its parent. */
void RT_ExecutionPlan_DetachOp(RT_OpBase *op);

/* Adds operation to execution plan as a child of parent. */
void RT_ExecutionPlan_AddOp(RT_OpBase *parent, RT_OpBase *newOp);

/* Push b right below a. */
void RT_ExecutionPlan_PushBelow(RT_OpBase *a, RT_OpBase *b);

/* Introduce new_root as the parent of old_root. */
void RT_ExecutionPlan_NewRoot(RT_OpBase *old_root, RT_OpBase *new_root);

/* Update the root op of the execution plan. */
void RT_ExecutionPlan_UpdateRoot(RT_ExecutionPlan *plan, RT_OpBase *new_root);

/* Replace a with b. */
void RT_ExecutionPlan_ReplaceOp(RT_ExecutionPlan *plan, RT_OpBase *a, RT_OpBase *b);

//------------------------------------------------------------------------------
// RT_ExecutionPlan_Locate API:
// For performing existence checks and looking up individual operations in tree.
//------------------------------------------------------------------------------

/* Traverse upwards until an operation that resolves the given alias is found.
 * Returns NULL if alias is not resolved. */
RT_OpBase *RT_ExecutionPlan_LocateOpResolvingAlias(RT_OpBase *root, const char *alias);

/* Locate the first operation matching one of the given types in the op tree by performing DFS.
 * Returns NULL if no matching operation was found. */
RT_OpBase *RT_ExecutionPlan_LocateOpMatchingType(RT_OpBase *root, const OPType *types, uint type_count);

/* Convenience wrapper around RT_ExecutionPlan_LocateOpMatchingType for lookups of a single type.
 * Locate the first operation of a given type within execution plan by performing DFS.
 * Returns NULL if operation wasn't found. */
RT_OpBase *RT_ExecutionPlan_LocateOp(RT_OpBase *root, OPType type);

/* Find the earliest operation above the provided recurse_limit, if any,
 * at which all references are resolved. */
RT_OpBase *RT_ExecutionPlan_LocateReferences(RT_OpBase *root, const RT_OpBase *recurse_limit,
									   rax *references_to_resolve);

// Returns all tap operations for given execution plan.
RT_OpBase **RT_ExecutionPlan_LocateTaps(const RT_ExecutionPlan *plan);

/* Find the earliest operation at which all references are resolved, if any,
 * both above the provided recurse_limit and without recursing past a blacklisted op. */
RT_OpBase *RT_ExecutionPlan_LocateReferencesExcludingOps(RT_OpBase *root,
		const RT_OpBase *recurse_limit, const OPType *blacklisted_ops,
		int nblacklisted_ops, rax *refs_to_resolve);

//------------------------------------------------------------------------------
// RT_ExecutionPlan_Collect API:
// For collecting all matching operations in tree.
//------------------------------------------------------------------------------

/* Collect all operations matching the given types in the op tree.
 * Returns an array of operations. */
RT_OpBase **RT_ExecutionPlan_CollectOpsMatchingType(RT_OpBase *root, const OPType *types, uint type_count);

/* Convenience wrapper around RT_ExecutionPlan_LocateOpMatchingType for
 * collecting all operations of a given type within the op tree.
 * Returns an array of operations. */
RT_OpBase **RT_ExecutionPlan_CollectOps(RT_OpBase *root, OPType type);

//------------------------------------------------------------------------------
// API for building and relocating operations in transient RT_ExecutionPlans.
//------------------------------------------------------------------------------

/* Populate a rax with all aliases that have been resolved by the given operation
 * and its children. These are the bound variables at this point in execution, and
 * subsequent operations should not introduce them as new entities. For example, in the query:
 * MATCH (a:A) CREATE (a)-[:E]->(b:B)
 * The Create operation should never introduce a new node 'a'. */
void RT_ExecutionPlan_BoundVariables(const RT_OpBase *op, rax *modifiers);

/* For all ops in the given tree, assocate the provided RT_ExecutionPlan.
 * This is for use for updating ops that have been built with a temporary RT_ExecutionPlan. */
void RT_ExecutionPlan_BindPlanToOps(RT_ExecutionPlan *plan, RT_OpBase *root);

/* Given an AST path pattern, generate the tree of scan, traverse,
 * and filter operations required to represent it. */
RT_OpBase *RT_ExecutionPlan_BuildOpsFromPath(RT_ExecutionPlan *plan, const char **vars,
									   const cypher_astnode_t *path);

