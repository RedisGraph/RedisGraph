/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../graph/edge.h"
#include "../rmutil/vector.h"
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

/* Push b right above a. */
void _OpBase_PushAbove(OpBase *a, OpBase *b) {
    /* B is a new operation. */
    assert(!(b->parent || b->children));
    assert(a->children);

    /* Remove each child of A and add it as a child of B. */
    while(a->childCount) {
        OpBase* child = a->children[0];
        _OpBase_RemoveChild(a, child);
        _OpBase_AddChild(b, child);
    }

    /* B is the only child of A. */
    _OpBase_AddChild(a, b);
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

void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp) {
    _OpBase_AddChild(parent, newOp);
}

void ExecutionPlan_ReplaceOp(OpBase *a, OpBase *b) {
    _OpBase_PushAbove(a->parent, b);
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

void _Determine_Graph_Size(const AST_Query *ast, size_t *node_count, size_t *edge_count) {
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

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx, Graph *g,
                                const char *graph_name,
                                AST_Query *ast,
                                bool explain) {

    ExecutionPlan *execution_plan = (ExecutionPlan*)calloc(1, sizeof(ExecutionPlan));
    // execution_plan->root = NewOpNode(NULL);    
    execution_plan->graph_name = graph_name;
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
    QueryGraph *q = NewQueryGraph_WithCapacity(node_count, edge_count);
    execution_plan->graph = q;
    FT_FilterNode *filter_tree = NULL;
    if(ast->whereNode != NULL)
        filter_tree = BuildFiltersTree(ast->whereNode->filters);
        execution_plan->filter_tree = filter_tree;

    if(ast->matchNode) {
        BuildQueryGraph(ctx, g, graph_name, q, ast->matchNode->_mergedPatterns);

        // For every pattern in match clause.
        size_t patternCount = Vector_Size(ast->matchNode->patterns);
        
        /* Incase we're dealing with multiple patterns
         * we'll simply join them all together with a join operation. */
        bool multiPattern = patternCount > 1;
        OpBase *cartesianProduct = NULL;
        if(multiPattern) {
            cartesianProduct = NewCartesianProductOp();
            Vector_Push(ops, cartesianProduct);
        }
        
        // Keep track after all traversal operations along a pattern.
        Vector *traversals = NewVector(OpBase*, 1);

        for(int i = 0; i < patternCount; i++) {
            Vector *pattern;
            Vector_Get(ast->matchNode->patterns, i, &pattern);

            if(Vector_Size(pattern) > 1) {
                size_t expCount = 0;
                AlgebraicExpression **exps = AlgebraicExpression_From_Query(ast, pattern, q, &expCount);
                
                TRAVERSE_ORDER order = determineTraverseOrder(q, filter_tree, exps, expCount);
                if(order == TRAVERSE_ORDER_FIRST) {
                    AlgebraicExpression *exp = exps[0];
                    selectEntryPoint(exp, q, filter_tree);

                    // Create SCAN operation.
                    if((*exp->src_node)->label) {
                        op = NewNodeByLabelScanOp(ctx, q, g, graph_name, exp->src_node, (*exp->src_node)->label);
                        Vector_Push(traversals, op);
                    } else {
                        op = NewAllNodeScanOp(q, g, exp->src_node);
                        Vector_Push(traversals, op);
                    }
                    for(int i = 0; i < expCount; i++) {
                        op = NewCondTraverseOp(g, q, exps[i]);
                        Vector_Push(traversals, op);
                    }
                } else {
                    AlgebraicExpression *exp = exps[expCount-1];
                    selectEntryPoint(exp, q, filter_tree);
                    // Create SCAN operation.
                    if((*exp->dest_node)->label) {
                        op = NewNodeByLabelScanOp(ctx, q, g, graph_name, exp->dest_node, (*exp->dest_node)->label);
                        Vector_Push(traversals, op);
                    } else {
                        op = NewAllNodeScanOp(q, g, exp->dest_node);
                        Vector_Push(traversals, op);
                    }

                    for(int i = expCount-1; i >= 0; i--) {
                        AlgebraicExpression_Transpose(exps[i]);
                        op = NewCondTraverseOp(g, q, exps[i]);
                        Vector_Push(traversals, op);
                    }
                }
            } else {
                /* Node scan. */
                AST_GraphEntity *ge;
                Vector_Get(pattern, 0, &ge);
                Node **n = QueryGraph_GetNodeRef(q, QueryGraph_GetNodeByAlias(q, ge->alias));
                if(ge->label)
                    op = NewNodeByLabelScanOp(ctx, q, g, graph_name, n, ge->label);
                else
                    op = NewAllNodeScanOp(q, g, n);
                Vector_Push(traversals, op);
            }
            
            if(multiPattern) {
                // Connect traversal operations.
                OpBase *childOp;
                OpBase *parentOp;
                Vector_Pop(traversals, &parentOp);
                // Connect cartesian product to the root of traversal.
                _OpBase_AddChild(cartesianProduct, parentOp);
                while(Vector_Pop(traversals, &childOp)) {
                    _OpBase_AddChild(parentOp, childOp);
                    parentOp = childOp;
                }
            } else {
                for(int traversalIdx = 0; traversalIdx < Vector_Size(traversals); traversalIdx++) {
                    Vector_Get(traversals, traversalIdx, &op);
                    Vector_Push(ops, op);
                }
            }
            Vector_Clear(traversals);
        }
        Vector_Free(traversals);
    }

    /* Set root operation */
    if(ast->createNode) {
        BuildQueryGraph(ctx, g, graph_name, q, ast->createNode->graphEntities);

        int request_refresh = (ast->matchNode != NULL);
        OpBase *opCreate = NewCreateOp(ctx, ast, g, q, graph_name, request_refresh, execution_plan->result_set);

        Vector_Push(ops, opCreate);
    }

    if(ast->mergeNode) {
        OpBase *opMerge = NewMergeOp(ctx, ast, g, q, graph_name, execution_plan->result_set);
        Vector_Push(ops, opMerge);
    }

    if(ast->deleteNode) {
        OpBase *opDelete = NewDeleteOp(ast->deleteNode, q, g, execution_plan->result_set);
        Vector_Push(ops, opDelete);
    }

    if(ast->setNode) {
        OpBase *op_update = NewUpdateOp(ctx, ast, q, execution_plan->result_set, graph_name);
        Vector_Push(ops, op_update);
    }

    if(ast->returnNode) {
        if(ReturnClause_ContainsAggregation(ast->returnNode)) {
           op = NewAggregateOp(ctx, ast);
        } else {
            op = NewProduceResultsOp(ast, execution_plan->result_set, q);
        }
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
        FilterTree_bindEntities(execution_plan->filter_tree, execution_plan->graph);
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
            OpBase *filter_op = NewFilterOp(tree, q);
            _OpBase_PushBelow(op, filter_op);
            for(int j = 0; j < Vector_Size(references); j++) {
                char *ref;
                Vector_Get(references, j, &ref);
                free(ref);
            }
            Vector_Free(references);

            ExecutionPlanPrint(execution_plan);
        }
        Vector_Free(sub_trees);
    }
    
    optimizePlan(ctx, graph_name, execution_plan);

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
    while(op->consume(op, plan->graph) == OP_OK);
    return plan->result_set;
}

void _ExecutionPlanFreeRecursive(OpBase* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanFreeRecursive(op->children[i]);
    }
    OpBase_Free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    _ExecutionPlanFreeRecursive(plan->root);
    // QueryGraph_Free(plan->graph);
    if(plan->filter_tree)
        FilterTree_Free(plan->filter_tree);
    free(plan);
}
