#include "execution_plan.h"
#include "../query_executor.h"

#include "./ops/expand_all.h"
#include "./ops/all_node_scan.h"
#include "./ops/node_by_label_scan.h"
#include "./ops/produce_results.h"
#include "./ops/filter.h"
#include "./ops/op_aggregate.h"

#include "../graph/edge.h"
#include "../rmutil/vector.h"

OpNode* NewOpNode(OpBase *op) {
    OpNode *opNode = malloc(sizeof(OpNode));
    opNode->operation = op;
    opNode->children = NULL;
    opNode->childCount = 0;
    return opNode;
}

void OpNode_AddChild(OpNode* parent, OpNode* child) {
    if(parent->children == NULL) {
        parent->children = malloc(sizeof(OpNode *));
    }

    parent->children[parent->childCount] = child;
    parent->childCount++;
    parent->children = realloc(parent->children, sizeof(OpNode *) * (parent->childCount+1));
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, RedisModuleString *graphName, QueryExpressionNode *ast) {
    Graph *graph = BuildGraph(ast->matchNode);
    ExecutionPlan *executionPlan = malloc(sizeof(ExecutionPlan));
    
    // List of operations
    Vector *Ops = NewVector(OpNode*, 0);

    OpBase *opFilter = NULL;
    OpBase *produceResults = NULL;
    NewProduceResultsOp(ctx, ast, &produceResults);

    OpNode *opProduceResults = NewOpNode(produceResults);
    executionPlan->root = opProduceResults;
    executionPlan->graph = graph;

    Vector_Push(Ops, opProduceResults);
    
    if(ReturnClause_ContainsAggregation(ast->returnNode)) {
        OpNode *opAggregate = NewOpNode(NewAggregateOp(ctx, ast));
        Vector_Push(Ops, opAggregate);
    }

    // if(ast->whereNode != NULL) {
    //     opFilter = NewFilterOp(ctx, ast);
    //     Vector_Push(Ops,  NewOpNode(opFilter));
    // }

    // Get all nodes without incoming edges
    Vector *entryNodes = Graph_GetNDegreeNodes(graph, 0);

    for(int i = 0; i < Vector_Size(entryNodes); i++) {
        Node *node;
        Vector_Get(entryNodes, i, &node);
        
        // Expand if possible
        if(Vector_Size(node->outgoingEdges) > 0) {
            Vector *reversedExpandOps = NewVector(OpNode*, 0);

            // Traverse sub-graph expanded from current node.
            // Assuming only one edge per node.
            // TODO: this assumption doesn't hold, support multi outgoing edges.
            Node *srcNode = node;
            Node *destNode;
            Edge *edge;

            do {
                Vector_Get(srcNode->outgoingEdges, 0, &edge);
                destNode = edge->dest;

                OpNode *opNodeExpandAll = NewOpNode(NewExpandAllOp(ctx, graphName, srcNode, edge, destNode));
                Vector_Push(reversedExpandOps, opNodeExpandAll);
                
                // can we advance?
                if(Vector_Size(destNode->outgoingEdges) == 0) {
                    break;
                }

                // Advance
                srcNode = destNode;
            } while(destNode != NULL);

            // Save expand ops in reverse order.
            while(Vector_Size(reversedExpandOps) > 0) {
                OpNode *opNodeExpandAll;
                Vector_Pop(reversedExpandOps, &opNodeExpandAll);
                Vector_Push(Ops, opNodeExpandAll);
            }
        }

        // Locate node within MATCH clause.
        ChainElement *chainElement;
        for(int j = 0; j < Vector_Size(ast->matchNode->chainElements); j++) {
            Vector_Get(ast->matchNode->chainElements, j, &chainElement);
            if(chainElement->t == N_ENTITY && strcmp(node->alias, chainElement->e.alias) == 0) { break; }
        }

        OpBase* scanOp;
        // Determin operation type
        if(chainElement->e.label == NULL) {
            // Node is not labeled, no other option but a full scan.
            // TODO: if this node is a part of a pattern (A)-[]->(B)
            // then it might be possible to scan B more efficently
            // if B is labeled.
            scanOp = NewAllNodeScanOp(ctx, node, graphName);
        } else {
            // TODO: when indexing is enabled, use index when possible.
            scanOp = NewNodeByLabelScanOp(ctx, node, graphName, chainElement->e.label);
        }
        Vector_Push(Ops, NewOpNode(scanOp));

        // Consume Ops in reversed order.
        OpNode* currentOp;
        OpNode* prevOp;
        Vector_Pop(Ops, &prevOp);
        
        // Connect operations in reversed order
        do {
            Vector_Pop(Ops, &currentOp);

            // Inster filter after every step.
            if(ast->whereNode != NULL) {
                opFilter = NewFilterOp(ctx, ast);
                OpNode *nodeFilter = NewOpNode(opFilter);
                OpNode_AddChild(currentOp, nodeFilter);
                OpNode_AddChild(nodeFilter, prevOp);
            } else {
                OpNode_AddChild(currentOp, prevOp);
            }
            prevOp = currentOp;
        } while(Vector_Size(Ops) != 0);

        // Reintroduce Projection.
        Vector_Push(Ops, opProduceResults);
    } // End of entry nodes loop

    Vector_Free(Ops);

    return executionPlan;
}

char* _ExecutionPlanPrint(const OpNode *op, char **strPlan, int ident) {
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
        }

        /* Try to get new data into the graph
         * TODO: find a better soulution for this if condition.
         * Produce Results requires all of its children to return OP_OK
         * before consuming. */
        if(strcmp(node->operation->name, "Produce Results") != 0) {
            for(int i = 0; i < node->childCount; i++) {
                OpNode *child = node->children[i];
                switch(_ExecuteOpNode(child, graph)) {
                    case OP_ERR:
                        return OP_ERR;
                    case OP_OK:
                        // Managed to get new data into the graph,
                        // Recall ourself.
                        return _ExecuteOpNode(node, graph);
                }
            }
        } else {
            for(int i = 0; i < node->childCount; i++) {
                OpNode *child = node->children[i];
                if (_ExecuteOpNode(child, graph) != OP_OK) {
                    return res;
                }
            }
            return _ExecuteOpNode(node, graph);
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