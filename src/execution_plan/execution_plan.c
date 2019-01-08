/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../util/vector.h"
#include "../util/arr.h"
#include "../graph/entities/edge.h"
#include "../query_executor.h"
#include "../arithmetic/algebraic_expression.h"
#include "./optimizations/optimizer.h"
#include "./optimizations/optimizations.h"

/* Checks if parent has given child, if so returns 1
 * otherwise returns 0 */
int _OpBase_ContainsChild(const OpBase *parent, const OpBase *child) {
    for(int i = 0; i < parent->childCount; i++) {
        if(parent->children[i] == child) {
            return 1;
        }
    }
    return 0;
}

void _OpBase_AddChild(OpBase *parent, OpBase *child) {
    // Add child to parent
    if(parent->children == NULL) {
        parent->children = malloc(sizeof(OpBase *));
    } else {
        parent->children = realloc(parent->children, sizeof(OpBase *) * (parent->childCount+1));
    }
    parent->children[parent->childCount++] = child;

    // Add parent to child
    child->parent = parent;
}

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
void _OpBase_RemoveNode(OpBase *parent, OpBase *child) {
    // Remove child from parent.
    int i = 0;
    for(; i < parent->childCount; i++) {
        if(parent->children[i] == child) break;
    }
    
    assert(i != parent->childCount);    

    // Uppdate child count.
    parent->childCount--;
    if(parent->childCount == 0) {
        free(parent->children);
        parent->children = NULL;
    } else {
        // Shift left children.
        for(int j = i; j < parent->childCount; j++) {
            parent->children[j] = parent->children[j+1];
        }
        parent->children = realloc(parent->children, sizeof(OpBase *) * parent->childCount);
    }

    // Remove parent from child.
    child->parent = NULL;
}

void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
    _OpBase_RemoveNode(parent, child);
}

/* Push b right below a. */
void _OpBase_PushBelow(OpBase *a, OpBase *b) {
    /* B is a new operation. */
    assert(!(b->parent || b->children));
    assert(a->parent);

    /* Replace A's former parent. */
    OpBase *a_former_parent = a->parent;

    /* Disconnect A from its former parent. */
    _OpBase_RemoveChild(a_former_parent, a);

    /* Add A's former parent as parent of B. */
    _OpBase_AddChild(a_former_parent, b);

    /* Add A as a child of B. */
    _OpBase_AddChild(b, a);
}

Vector* _ExecutionPlan_Locate_References(OpBase *root, OpBase **op, Vector *references) {
    /* List of entities which had their ID resolved
     * at this point of execution, should include all
     * previously modified entities (up the execution plan). */
    Vector *seen = NewVector(char*, 0);
    char *modifiedEntity;

    /* Append current op modified entities. */
    if(root->modifies) {
        for(int i = 0; i < Vector_Size(root->modifies); i++) {
            Vector_Get(root->modifies, i, &modifiedEntity);
            Vector_Push(seen, modifiedEntity);
        }
    }

     /* Traverse execution plan, upwards. */
    for(int i = 0; i < root->childCount; i++) {
        Vector *saw = _ExecutionPlan_Locate_References(root->children[i], op, references);

        /* Quick return if op was located. */
        if(*op) {
            Vector_Free(saw);
            return seen;
        }

        /* Add modified entities from previous operation. */
        for(int i = 0; i < Vector_Size(saw); i++) {
            Vector_Get(saw, i, &modifiedEntity);
            Vector_Push(seen, modifiedEntity);
        }
        Vector_Free(saw);
    }

    /* See if all references been resolved.
     * TODO: create a vector compare function
     * which checks if the content of one is in the other. */
    int match = Vector_Size(references);
    size_t ref_count = Vector_Size(references);
    size_t seen_count = Vector_Size(seen);

    for(int i = 0; i < ref_count; i++) {
        char *reference;        
        Vector_Get(references, i, &reference);

        int j = 0;
        for(; j < seen_count; j++) {
            char *resolved;
            Vector_Get(seen, j, &resolved);
            if(!strcmp(reference, resolved)) {
                /* Match! */
                break;
            }
        }
        
        /* no match, quick break, */
        if (j == seen_count) break;
        else match--;
    }

    if(!match) *op = root;
    return seen;
}

void _Count_Graph_Entities(const Vector *entities, size_t *node_count, size_t *edge_count) {
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        if(entity->t == N_ENTITY) {
            (*node_count)++;
        } else if(entity->t == N_LINK) {
            (*edge_count)++;
        }
    }
}

void _Determine_Graph_Size(const AST *ast, size_t *node_count, size_t *edge_count) {
    *edge_count = 0;
    *node_count = 0;
    Vector *entities;
    
    if(ast->matchNode) {
        entities = ast->matchNode->_mergedPatterns;
        _Count_Graph_Entities(entities, node_count, edge_count);
    }

    if(ast->createNode) {
        entities = ast->createNode->graphEntities;
        _Count_Graph_Entities(entities, node_count, edge_count);
    }
}

void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp) {
    _OpBase_AddChild(parent, newOp);
}

void ExecutionPlan_ReplaceOp(OpBase *a, OpBase *b) {
    // Insert the new operation between the original and its parent.
    _OpBase_PushBelow(a, b);
    // Delete the original operation.
    ExecutionPlan_RemoveOp(a);
}

void ExecutionPlan_RemoveOp(OpBase *op) {
    assert(op->parent != NULL);
    
    // Remove op from its parent.
    OpBase* parent = op->parent;
    _OpBase_RemoveChild(op->parent, op);

    // Add each of op's children as a child of op's parent.
    for(int i = 0; i < op->childCount; i++) {
        _OpBase_AddChild(parent, op->children[i]);
    }
}

OpBase* ExecutionPlan_Locate_References(OpBase *root, Vector *references) {
    OpBase *op = NULL;    
    Vector *temp = _ExecutionPlan_Locate_References(root, &op, references);
    Vector_Free(temp);
    return op;
}

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx,
                                GraphContext *gc,
                                AST *ast,
                                bool explain) {

    Graph *g = gc->g;
    ExecutionPlan *execution_plan = (ExecutionPlan*)calloc(1, sizeof(ExecutionPlan));    
    execution_plan->result_set = (explain) ? NULL: NewResultSet(ast, ctx);
    execution_plan->filter_tree = NULL;
    Vector *ops = NewVector(OpBase*, 1);
    OpBase *op;

    /* Predetermin graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old references in ExpandAll. */
    size_t node_count;
    size_t edge_count;
    _Determine_Graph_Size(ast, &node_count, &edge_count);
    QueryGraph *q = QueryGraph_New(node_count, edge_count);
    execution_plan->query_graph = q;

    FT_FilterNode *filter_tree = NULL;
    if(ast->whereNode != NULL) {
        filter_tree = BuildFiltersTree(ast, ast->whereNode->filters);
        execution_plan->filter_tree = filter_tree;
    }

    if(ast->matchNode) {
        BuildQueryGraph(gc, q, ast->matchNode->_mergedPatterns);

        int component_count;
        Node **starting_points = QueryGraph_ConnectedComponents(q, &component_count);


        AE_Unit ***components = AlgebraicExpression_BuildExps(ast, q, starting_points, component_count);

        if (array_len(components) > 1) {
            // Insert a Cartesian Product root if we're traversing multiple independent patterns
            OpBase *cartesianProduct = NewCartesianProductOp();
            Vector_Push(ops, cartesianProduct);
        }

        // For every connected component
        for (int i = 0; i < array_len(components); i ++) {
            AE_Unit **component_exps = components[i];

            // Add a scan operation
            if (component_exps[0]->src->mat) { // Node is labeled
              op = NewNodeByLabelScanOp(gc, component_exps[0]->src);
            } else {
              op = NewAllNodeScanOp(g, component_exps[0]->src);
            }
            Vector_Push(ops, op);

            for (int j = 0; j < array_len(component_exps); j ++) {
              AE_Unit *expression = component_exps[j];
              if (j == 0 && !expression->dest && !expression->edge) {
                // This was just a scan, no traversal required
                continue;
              }
              // TODO choose an ideal starting place
              // TODO Replace the first operand matrix with a node/label scan
              // (optimize later for filters, nodes over labels, etc)

              // Push traversal operation onto stack
              OpBase *cond_traverse = NewCondTraverseOp(g, expression);
              Vector_Push(ops, cond_traverse);
            }
        }
    }

    if(ast->unwindNode) {
        OpBase *opUnwind = NewUnwindOp(ast->unwindNode);
        Vector_Push(ops, opUnwind);
    }

    /* Set root operation */
    if(ast->createNode) {
        BuildQueryGraph(gc, q, ast->createNode->graphEntities);
        OpBase *opCreate = NewCreateOp(ctx, gc, ast, q, execution_plan->result_set);

        Vector_Push(ops, opCreate);
    }

    if(ast->mergeNode) {
        OpBase *opMerge = NewMergeOp(gc, ast, q, execution_plan->result_set);
        Vector_Push(ops, opMerge);
    }

    if(ast->deleteNode) {
        OpBase *opDelete = NewDeleteOp(ast->deleteNode, q, gc, execution_plan->result_set);
        Vector_Push(ops, opDelete);
    }

    if(ast->setNode) {
        OpBase *op_update = NewUpdateOp(gc, ast, execution_plan->result_set);
        Vector_Push(ops, op_update);
    }

    if(ast->returnNode) {
        // Projection before sort and produce result-set
        // Both Aggregate and Projection perform projection.
        if(ReturnClause_ContainsAggregation(ast->returnNode)) {
            op = NewAggregateOp(ast);
            Vector_Push(ops, op);
        } else {            
            op = NewProjectOp(ast);
            Vector_Push(ops, op);
        }

        if(ast->orderNode) {
            op = NewSortOp(ast);
            Vector_Push(ops, op);
        }

        op = NewProduceResultsOp(execution_plan->result_set, q);
        Vector_Push(ops, op);
    }

    OpBase *parent_op;
    OpBase *child_op;
    Vector_Pop(ops, &parent_op);
    execution_plan->root = parent_op;

    while(Vector_Pop(ops, &child_op)) {
        _OpBase_AddChild(parent_op, child_op);
        parent_op = child_op;
    }

    if(ast->whereNode != NULL) {
        Vector *sub_trees = FilterTree_SubTrees(execution_plan->filter_tree);

        /* For each filter tree find the earliest position along the execution 
         * after which the filter tree can be applied. */
        for(int i = 0; i < Vector_Size(sub_trees); i++) {
            FT_FilterNode *tree;
            Vector_Get(sub_trees, i, &tree);

            Vector *references = FilterTree_CollectAliases(tree);

            /* Scan execution plan, locate the earliest position where all 
             * references been resolved. */
            OpBase *op = ExecutionPlan_Locate_References(execution_plan->root, references);
            assert(op);

            /* Create filter node.
             * Introduce filter op right below located op. */
            OpBase *filter_op = NewFilterOp(tree);
            _OpBase_PushBelow(op, filter_op);
            for(int j = 0; j < Vector_Size(references); j++) {
                char *ref;
                Vector_Get(references, j, &ref);
                free(ref);
            }
            Vector_Free(references);
        }
        Vector_Free(sub_trees);
    }
    
    Vector_Free(ops);
    optimizePlan(gc, execution_plan);

    return execution_plan;
}

void _ExecutionPlanPrint(const OpBase *op, char **strPlan, int ident) {
    char strOp[512] = {0};
    sprintf(strOp, "%*s%s\n", ident, "", op->name);
    
    if(*strPlan == NULL) {
        *strPlan = calloc(strlen(strOp) + 1, sizeof(char));
    } else {
        *strPlan = realloc(*strPlan, sizeof(char) * (strlen(*strPlan) + strlen(strOp) + 2));
    }
    strcat(*strPlan, strOp);

    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanPrint(op->children[i], strPlan, ident + 4);
    }
}

char* ExecutionPlanPrint(const ExecutionPlan *plan) {
    char *strPlan = NULL;
    _ExecutionPlanPrint(plan->root, &strPlan, 0);
    return strPlan;
}

ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan) {
    OpBase *op = plan->root;
    Record r;
    while((r = op->consume(op)) != NULL) Record_Free(r);
    return plan->result_set;
}

void _ExecutionPlanFreeRecursive(OpBase* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanFreeRecursive(op->children[i]);
    }
    OpBase_Free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    if (plan == NULL) return;

    _ExecutionPlanFreeRecursive(plan->root);
    if(plan->filter_tree) FilterTree_Free(plan->filter_tree);
    if(plan->query_graph) QueryGraph_Free(plan->query_graph);
    free(plan);
}
