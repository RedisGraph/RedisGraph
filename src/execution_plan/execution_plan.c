#include "execution_plan.h"
#include "../query_executor.h"

#include "./ops/op_expand_all.h"
#include "./ops/op_expand_into.h"
#include "./ops/op_all_node_scan.h"
#include "./ops/op_node_by_label_scan.h"
#include "./ops/op_produce_results.h"
#include "./ops/op_filter.h"
#include "./ops/op_aggregate.h"

#include "../graph/edge.h"
#include "../rmutil/vector.h"

OpNode* NewOpNode(OpBase *op) {
    OpNode *opNode = malloc(sizeof(OpNode));
    opNode->operation = op;
    opNode->children = NULL;
    opNode->childCount = 0;
    opNode->parents = NULL;
    opNode->parentCount = 0;
    return opNode;
}

/* Checks if parent has given child, if so returns 1
 * otherwise returns 0 */
int _OpNode_ContainsChild(const OpNode *parent, const OpNode *child) {
    for(int i = 0; i < parent->childCount; i++) {
        if(parent->children[i] == child) {
            return 1;
        }
    }
    return 0;
}

void _OpNode_AddChild(OpNode *parent, OpNode *child) {

    // Add child to parent
    if(parent->children == NULL) {
        parent->children = malloc(sizeof(OpNode *));
    }

    parent->children[parent->childCount] = child;
    parent->childCount++;
    parent->children = realloc(parent->children, sizeof(OpNode *) * (parent->childCount+1));

    // Add parent to child
    if(child->parents == NULL) {
        child->parents = malloc(sizeof(OpNode *));
    }

    child->parents[child->parentCount] = parent;
    child->parentCount++;
    child->parents = realloc(child->parents, sizeof(OpNode *) * (child->parentCount+1));
}

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
void _OpNode_RemoveNode(OpNode *a, OpNode *b) {
    // remove child from parent
    for(int i = 0; i < a->childCount; i++) {
        if(a->children[i] == b) {
            // shift left children
            for(int j = i; j < a->childCount-1; j++) {
                a->children[j] = a->children[j+1];
            }
            // uppdate child count
            a->childCount--;
            a->children = realloc(a->children, sizeof(OpNode *) * a->childCount);
            break;
        }
    }

    // remove parent from child
    for(int i = 0; i < b->parentCount; i++) {
        if(b->parents[i] == a) {
            // shift left parents
            for(int j = i; j < b->parentCount-1; j++) {
                b->parents[j] = b->parents[j+1];
            }
            // uppdate parent count
            b->parentCount--;
            b->parents = realloc(b->parents, sizeof(OpNode *) * b->parentCount);
            break;
        }
    }
}

void _OpNode_RemoveChild(OpNode *parent, OpNode *child) {
    _OpNode_RemoveNode(parent, child);
}

void _OpNode_RemoveParent(OpNode *child, OpNode *parent) {
    _OpNode_RemoveNode(parent, child);
}

void _OpNode_PushInBetween(OpNode *parent, OpNode *onlyChild) {
    // Disconnect every parent of parent.
    for(int i = parent->parentCount-1; i > -1; i--) {
        OpNode *grandParent = parent->parents[i];
        _OpNode_AddChild(grandParent, onlyChild);
        _OpNode_RemoveChild(grandParent, parent);
    }
    _OpNode_AddChild(onlyChild, parent);
}

/*
 * Nodes with more than one incoming edge
 * will take part in two expand operations
 * MergeNodes will replace one of the expand operation
 * with an expand_into operation 
 * plan - execution plan to be modified
 * n - node with two incoming edges. */
void _ExecutionPlan_MergeNodes(ExecutionPlan *plan, const Node *n) {
    if(Node_IncomeDegree(n) != 2) {
        return;
    }

    /* locate both expand operations involving 
     * n as the dest node */
    
    OpNode *a = NULL;
    OpNode *b = NULL;

    OpNode *current = NULL;
    Vector *nodesToVisit = NewVector(OpNode*, 3);
    Vector_Push(nodesToVisit, plan->root);
    /* due to the structure of our execution plan
     * there's not need to maintain a visited nodes 
     * as in classic BFS/DFS */
    while(Vector_Size (nodesToVisit) > 0) {
        Vector_Pop(nodesToVisit, &current);
        // TODO: add type to operation.
        if(strcmp(current->operation->name, "Expand All") == 0) {
            ExpandAll *op = current->operation;
            if(Node_Compare(op->destNode, n)) {
                if(a == NULL) {
                    a = current;
                    continue;
                } else {
                    b = current;
                    break;
                }
            }
        }

        for(int i = 0; i < current->childCount; i++) {
            Vector_Push(nodesToVisit, current->children[i]);
        }
    }
    Vector_Free(nodesToVisit);
    
    if(a == NULL || b == NULL) {
        return;
    }

    /* now that both operations involving node n
     * are found we can replace one of them with
     * an expand into operation */
    ExpandAll *op = a->operation;
    OpBase *opExpandInto;
    NewExpandIntoOp(op->ctx, plan->graphName, op->srcNode, op->relation, op->destNode, &opExpandInto);
    
    // free previous operation.
    a->operation->free(a->operation);
    a->operation = opExpandInto;
    OpNode *expandInto = a;

    // link b operation with expand into
    _OpNode_AddChild(expandInto, b);

    // expand into should inherit b's parents
    int bParentCount = b->parentCount;
    for(int i = 0; i < bParentCount; i++) {
        OpNode *bParent = b->parents[i];
        
        if(bParent == expandInto) {
            continue;
        }

        if(!_OpNode_ContainsChild(bParent, expandInto)) {
            _OpNode_AddChild(bParent, expandInto);
        }

        _OpNode_RemoveChild(bParent, b);
    }
}

/* Returns the number of expected IDs given node will generate */
int _ExecutionPlan_EstimateNodeCardinality(RedisModuleCtx *ctx, const RedisModuleString *graph, const Node *n) {
    Store *s = NULL;
    if(n->label != NULL) {
        RedisModuleString* label = RedisModule_CreateString(ctx, n->label, strlen(n->label));
        s = GetStore(ctx, STORE_NODE, graph, label);
        RedisModule_FreeString(ctx, label);
    } else {
        s = GetStore(ctx, STORE_NODE, graph, NULL);
    }
    
    return s->cardinality;
}

void _ExecutionPlan_OptimizeEntryPoints(RedisModuleCtx *ctx, const Graph *g, const RedisModuleString *graphName, FT_FilterNode *filterTree, QueryExpressionNode *ast, OpNode *root) {
    // We've reached a leaf
    if(root->childCount == 0 && strcmp(root->operation->name, "Expand All") == 0) {
        Node *src = ((ExpandAll*)(root->operation))->srcNode;
        Node *dest = ((ExpandAll*)(root->operation))->destNode;
        Node *entryPoint = src;

        // Determin which node should be scaned, based on node cardinality
        int srcCardinality = _ExecutionPlan_EstimateNodeCardinality(ctx, graphName, src);
        int destCardinality = _ExecutionPlan_EstimateNodeCardinality(ctx, graphName, dest);
        
        // if(destCardinality < srcCardinality) {
        //     entryPoint = dest;
        // }
        
        OpBase *scanOp = NULL;
        if(entryPoint->label == NULL) {
            // Node is not labeled, no other option but a full scan.
            scanOp = NewAllNodeScanOp(ctx, entryPoint, graphName);
        } else {
            // TODO: when indexing is enabled, use index when possible.
            scanOp = NewNodeByLabelScanOp(ctx, entryPoint, graphName, entryPoint->label);
        }        
        
        _OpNode_AddChild(root, NewOpNode(scanOp));

    } else {
        // Continue scanning
        for(int i = 0; i < root->childCount; i++) {
            _ExecutionPlan_OptimizeEntryPoints(ctx, g, graphName, filterTree, ast, root->children[i]);
        }
    }
}

Vector* _ExecutionPlan_AddFilters(RedisModuleCtx *ctx, FT_FilterNode **filterTree, OpNode *root) {
    
    // We've reached the end of our execution plan.
    if(root == NULL) {
        return NULL;
    }

    /* List of entities which had their ID resolved
     * at this point of execution, should include all
     * current modified entities plus, all previously
     * modified entities (up the execution plan) */
    Vector *seen;
    char *modifiedEntity;
    
    if(root->operation->modifies) {
        seen = NewVector(char*, Vector_Size(root->operation->modifies));
        
        // Copy current op's modified entities to seen.
        for(int i = 0; i < Vector_Size(root->operation->modifies); i++) {
            Vector_Get(root->operation->modifies, i, &modifiedEntity);
            Vector_Push(seen, modifiedEntity);
        }
    } else {
        seen = NewVector(char*, 0);
    }

    // Traverse execution plan, upwards.
    for(int i = root->childCount-1; i >= 0; i--) {
        Vector *saw = _ExecutionPlan_AddFilters(ctx, filterTree, root->children[i]);
        
        // No need to continue, filter tree is empty.
        if(*filterTree == NULL) {
            if(saw != NULL) {
                Vector_Free(saw);
            }
            Vector_Free(seen);
            return NULL;
        }

        /* Add modified entities from previous operations
         * to current list Update list of entities we've seen. */
        if(saw != NULL) {
            for(int i = 0; i < Vector_Size(saw); i++) {
                Vector_Get(saw, i, &modifiedEntity);
                Vector_Push(seen, modifiedEntity);
            }
            Vector_Free(saw);
        }
    }
    
    // No need to continue, filter tree is empty.
    if(*filterTree == NULL) {
        Vector_Free(seen);
        return NULL;
    }

    // See if filter tree filters any of the current op modified entities
    if(FilterTree_ContainsNode(*filterTree, seen)) {    
        // Create a minimum filter tree for the current execution plan operation
        FT_FilterNode *minTree = FilterTree_MinFilterTree(*filterTree, seen);
        
        // Remove op modified entities from main filter tree.
        FilterTree_RemovePredNodes(filterTree, seen);
        
        OpNode *nodeFilter = NewOpNode(NewFilterOp(ctx, minTree));
        _OpNode_PushInBetween(root, nodeFilter);
    }

    return seen;
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, RedisModuleString *graphName, QueryExpressionNode *ast) {
    Graph *graph = BuildGraph(ast->matchNode);
    ExecutionPlan *executionPlan = malloc(sizeof(ExecutionPlan));
    
    // List of operations
    OpBase *produceResults = NULL;
    FT_FilterNode *filterTree = NULL;

    Vector *Ops = NewVector(OpNode*, 0);
    NewProduceResultsOp(ctx, ast, &produceResults);
    OpNode *opProduceResults = NewOpNode(produceResults);

    executionPlan->root = opProduceResults;
    executionPlan->graph = graph;
    executionPlan->graphName = graphName;

    Vector_Push(Ops, opProduceResults);

    if(ast->whereNode != NULL) {
        filterTree = BuildFiltersTree(ast->whereNode->filters);
    }

    if(ReturnClause_ContainsAggregation(ast->returnNode)) {
        OpNode *opAggregate = NewOpNode(NewAggregateOp(ctx, ast));
        Vector_Push(Ops, opAggregate);
    }

    // Get all nodes without incoming edges
    Vector *entryNodes = Graph_GetNDegreeNodes(graph, 0);

    for(int i = 0; i < Vector_Size(entryNodes); i++) {
        Node *node;
        Vector_Get(entryNodes, i, &node);
        
        // advance if possible
        if(Vector_Size(node->outgoingEdges) > 0) {
            Vector *reversedExpandOps = NewVector(OpNode*, 0);

            // Traverse sub-graph expanded from current node.
            Node *srcNode = node;
            Node *destNode;
            Edge *edge;

            while(Vector_Size(srcNode->outgoingEdges) > 0) {
                Vector_Get(srcNode->outgoingEdges, 0, &edge);
                destNode = edge->dest;
                
                OpNode *opNodeExpandAll = NewOpNode(NewExpandAllOp(ctx, graphName, srcNode, edge, destNode));
                Vector_Push(reversedExpandOps, opNodeExpandAll);
                
                // Advance
                srcNode = destNode;
            }

            // Save expand ops in reverse order.
            while(Vector_Size(reversedExpandOps) > 0) {
                OpNode *opNodeExpandAll;
                Vector_Pop(reversedExpandOps, &opNodeExpandAll);
                Vector_Push(Ops, opNodeExpandAll);
            }
        }

        // Consume Ops in reversed order.
        OpNode* currentOp;
        OpNode* prevOp;
        Vector_Pop(Ops, &prevOp);
        
        // Connect operations in reversed order
        do {
            Vector_Pop(Ops, &currentOp);
            _OpNode_AddChild(currentOp, prevOp);
            prevOp = currentOp;
        } while(Vector_Size(Ops) != 0);

        // Reintroduce Projection.
        Vector_Push(Ops, opProduceResults);
    } // End of entry nodes loop

    Vector_Free(Ops);

    // Optimizations and modifications.
    _ExecutionPlan_OptimizeEntryPoints(ctx, graph, graphName, filterTree, ast, executionPlan->root);
    
    Vector *nodesToMerge = Graph_GetNDegreeNodes(graph, 2);
    for(int i = 0; i < Vector_Size(nodesToMerge); i++) {
        Node *nodeToMerge;
        Vector_Get(nodesToMerge, i, & nodeToMerge);
        _ExecutionPlan_MergeNodes(executionPlan, nodeToMerge);
    }    
    _ExecutionPlan_AddFilters(ctx, &filterTree, executionPlan->root);
    return executionPlan;
}

void _ExecutionPlanPrint(const OpNode *op, char **strPlan, int ident) {
    char strOp[512] = {0};
    sprintf(strOp, "%*s%s\n", ident, "", op->operation->name);
    
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

OpResult _ExecuteOpNode(OpNode *node, Graph *graph) {
    /* Before current node can execute, all of
     * its children must execute */
    OpResult res = node->operation->next(node->operation, graph);
    
    /* Incase we're depleted or require data renewal
     * try to get new data from children */
    if(OP_REQUIRE_NEW_DATA(res)) {
        if(res == OP_DEPLETED) {
            // Op is consumed.
            if(node->childCount == 0) {
                return OP_DEPLETED;
            }
            
            // Reset on depleted
            if (node->operation->reset(node->operation) == OP_ERR) {
                return OP_ERR;
            }

            /* Require each child to return OP_OK before we can
             * continue */
            for(int i = 0; i < node->childCount; i++) {
                OpNode *child = node->children[i];
                if(_ExecuteOpNode(child, graph) != OP_OK) {
                    return res;
                }
            }
            // We're good to go.
            return _ExecuteOpNode(node, graph);
        }
        
        if(res == OP_REFRESH) {
            int i;
            for(i = 0; i < node->childCount; i++) {
                OpNode *child = node->children[i];
                OpResult opResult = _ExecuteOpNode(child, graph);
                if(opResult == OP_OK) {
                    break;
                }
            }

            if(i != node->childCount) {
                i--;
                /* one of our children managed to provide us with new data,
                 * children between 0 to i, did not managed 
                 * to get us new data, we're going to reset them. */
                while(i > -1) {
                    OpNode *child = node->children[i];
                    // TODO: reset child's top node.
                    OpNode *topChild = child;
                    while(topChild->childCount != 0) {
                        // topChild->operation->reset(topChild->operation);
                        topChild = topChild->children[0];
                    }
                    // reset last child.
                    topChild->operation->reset(topChild->operation);
                    OpResult opResult = _ExecuteOpNode(child, graph);
                    if(opResult != OP_OK) {
                        return opResult;
                    }
                    i--;
                }
                
                // We're good to go.
                return _ExecuteOpNode(node, graph);
            }
        }
    }
    return res;
}

ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan) {
    while(_ExecuteOpNode(plan->root, plan->graph) == OP_OK);
    
    // Execution-Plan root node is ProduceResults operation.
    return ((ProduceResults*)plan->root->operation)->resultset;
}

void OpNode_Free(OpNode* op) {
    // Free child operations
    for(int i = 0; i < op->childCount; i++) {
        OpNode_Free(op->children[i]);
    }
    
    // Free internal operation
    op->operation->free(op->operation);
    free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    OpNode_Free(plan->root);
    Graph_Free(plan->graph);
    free(plan);
}