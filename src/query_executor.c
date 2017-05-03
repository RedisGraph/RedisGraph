#include "query_executor.h"
#include "parser/grammar.h"
#include "rmutil/vector.h"
#include "graph/node.h"
#include "parser/parser_common.h"
#include "hexastore/hexastore.h"
#include "hexastore/triplet.h"
#include "aggregate/agg_ctx.h"
#include "aggregate/repository.h"

Graph* BuildGraph(const MatchNode* matchNode) {
    Graph* g = NewGraph();
    Vector* stack = NewVector(void*, 3);
    
    for(int i = 0; i < Vector_Size(matchNode->chainElements); i++) {
        ChainElement* element;
        Vector_Get(matchNode->chainElements, i, &element);

        if(element->t == N_ENTITY) {
            Node *n = NewNode(element->e.alias, NULL);
            Graph_AddNode(g, n);
            Vector_Push(stack, n);
        } else {
            Vector_Push(stack, &element->l);
        }

        if(Vector_Size(stack) == 3) {
            Node* a;
            LinkNode* edge;
            Node* c;

            Vector_Pop(stack, &c);
            Vector_Pop(stack, &edge);
            Vector_Pop(stack, &a);

            if(edge->direction == N_LEFT_TO_RIGHT) {
                Edge *e = NewEdge(NULL, a, c, edge->relationship);
                ConnectNode(a, c,  e);
            } else {
                Edge *e = NewEdge(NULL, c, a, edge->relationship);
                ConnectNode(c, a, e);
            }
            
            // reintroduce last pushed item
            Vector_Push(stack, c);
        } // if ends
    } // For loop ends

    Vector_Free(stack);    
    return g;
}

void GetElementProperyValue(RedisModuleCtx *ctx, const char *elementID, const char *property, RedisModuleString **propValue) {
    RedisModuleString* keyStr =
        RedisModule_CreateString(ctx, elementID, strlen(elementID));

    RedisModuleKey *key = 
        RedisModule_OpenKey(ctx, keyStr, REDISMODULE_READ);

    if(key == NULL) {
        printf("ERROR: trying to get property:[%s] from a missing key:[%s]\n", property, elementID);
        RedisModule_FreeString(ctx, keyStr);
        (*propValue) = NULL;
        return;
    }

    RedisModuleString* elementProp =
        RedisModule_CreateString(ctx, property, strlen(property));

    RedisModule_HashGet(key, REDISMODULE_HASH_NONE, elementProp, propValue, NULL);

    RedisModule_CloseKey(key);
    RedisModule_FreeString(ctx, keyStr);
    RedisModule_FreeString(ctx, elementProp);
}

// type specifies the type of return elements to Retrieve
Vector* _ReturnClause_RetrieveValues(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g, ReturnElementType type) {
    Vector* returnedProps = NewVector(RedisModuleString*, Vector_Size(returnNode->returnElements));

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        ReturnElementNode* retElem;
        Vector_Get(returnNode->returnElements, i, &retElem);

        // Skip elements not of specified type
        if(retElem->type != type) {
            continue;
        }

        Node* n = Graph_GetNodeByAlias(g, retElem->variable->alias);

        if(retElem->variable->property != NULL) {
            RedisModuleString* prop;
            GetElementProperyValue(ctx, n->id, retElem->variable->property, &prop);
            if(prop == NULL) {
                // Couldn't find prop for id.
                // TODO: Free returnedProps.
                return NULL;
            } else {
                Vector_Push(returnedProps, prop);
            }
        } else {
            // Couldn't find an API for HGETALL.
        }
    } // End of for loop

    return returnedProps;
}

// Retrieves all request values specified in return clause
// Returns a vector of RedisModuleString*
Vector* ReturnClause_RetrievePropValues(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g) {
    return _ReturnClause_RetrieveValues(ctx, returnNode, g, N_PROP);
}

// Retrieves "GROUP BY" values.
// Returns a vector of RedisModuleString*
Vector* ReturnClause_RetrieveGroupKeys(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g) {
    return _ReturnClause_RetrieveValues(ctx, returnNode, g, N_PROP);
}

// Retrieves all aggregated properties from graph.
// e.g. SUM(Person.age)
// Returns a vector of RedisModuleString*
Vector* ReturnClause_RetrieveGroupAggVals(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g) {
    return _ReturnClause_RetrieveValues(ctx, returnNode, g, N_AGG_FUNC);
}

int ReturnClause_ContainsCollapsedNodes(const ReturnNode *returnNode) {
    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        ReturnElementNode *returnElementNode;
        Vector_Get(returnNode->returnElements, i, &returnElementNode);
        if(returnElementNode->type == N_NODE) {
            return 1;
        }
    }
    return 0;
}

// Checks if return clause uses aggregation.
int ReturnClause_ContainsAggregation(const ReturnNode* returnNode) {
    int res = 0;

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        ReturnElementNode* retElem;
        Vector_Get(returnNode->returnElements, i, &retElem);
        if(retElem->type == N_AGG_FUNC) {
            res = 1;
            break;
        }
    }

    return res;
}

Vector* ReturnClause_GetAggFuncs(RedisModuleCtx *ctx, const ReturnNode* returnNode) {
    Vector* aggFunctions = NewVector(AggCtx*, 0);

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        ReturnElementNode* retElem;
        Vector_Get(returnNode->returnElements, i, &retElem);

        if(retElem->type == N_AGG_FUNC) {
            AggCtx* func = NULL;
            Agg_GetFunc(retElem->func, &func);
            if(func == NULL) {
                RedisModule_Log(ctx, "error", "aggregation function %s is not supported\n", retElem->func);
                RedisModule_ReplyWithError(ctx, "Invalid aggregation function");
                return NULL;
            }
            Vector_Push(aggFunctions, func);
        }
    }

    return aggFunctions;
}

// void _ExpendReturnClause(RedisModuleCtx *ctx, RedisModuleString *graphName, Graph *g, Node *n, Vector *entryPoints, ReturnNode *returnNode) {
//     Node* src = n;
//     for(int i = 0; i < Vector_Size(src->outgoingEdges); i++) {
//         Edge* edge;
//         Vector_Get(src->outgoingEdges, i, &edge);
//         Node* dest = edge->dest;

//         // Create a triplet out of edge
//         Triplet* triplet = TripletFromEdge(edge);

//         // Query graph using triplet, no filters are applied at this stage
//         HexaStore *hexaStore = GetHexaStore(ctx, graphName);
//         TripletIterator *cursor = HexaStore_QueryTriplet(hexaStore, triplet);

//         FreeTriplet(triplet);

//         // Run through cursor.
//         // Fast skip triplets which do not pass filters
//         triplet = FastSkipTriplets(ctx, NULL, cursor, g, src, dest);
//         if(triplet == NULL) {
//             // Couldn't find a triplet which agrees with filters
//             return;
//         } else {
//             // Set nodes
//             src->id = triplet->subject;
//             edge->relationship = triplet->predicate;
//             dest->id = triplet->object;
//         }
//         // Advance to next node
//         if(Vector_Size(dest->outgoingEdges) > 0) {
//             return _ExpendReturnClause(ctx, graphName, g, dest, entryPoints, returnNode);
//         } else if(Vector_Size(entryPoints) > 0) {
//             // Additional entry point
//             Node* entryPoint = NULL;
//             Vector_Pop(entryPoints, &entryPoint);
//             return _ExpendReturnClause(ctx, graphName, g, entryPoint, entryPoints, returnNode);
//         } else {
//             // We've reach the end of the graph
//             // Expend each collapsed return node.
//             Vector *returnElements = NewVector(ReturnElementNode *, Vector_Size(returnNode->returnElements));

//             for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
//                 ReturnElementNode *returnElementNode;
//                 Vector_Get(returnNode->returnElements, i, &returnElementNode);

//                 if(returnElementNode->type == N_NODE) {
//                     Node* collapsedNode = Graph_GetNodeByAlias(g, returnElementNode->variable->alias);

//                     // Issue HGETALL.
//                     RedisModuleCallReply *reply = RedisModule_Call(ctx, "HGETALL", "c", collapsedNode->id);
//                     if(RedisModule_CallReplyType(reply) != REDISMODULE_REPLY_ARRAY) {
//                         printf("ERROE expecting an array \n");
//                         break;
//                     } else {
//                         // Consume HGETALL.
//                         size_t reply_len = RedisModule_CallReplyLength(reply);
//                         // Foreach element within node's hash
//                         // We're only interested in attribute name.
//                         for(int idx = 0; idx < reply_len; idx+=2) {
//                             RedisModuleCallReply *subreply;
//                             subreply = RedisModule_CallReplyArrayElement(reply, idx);
//                             size_t len;
//                             const char *property = RedisModule_CallReplyStringPtr(subreply, &len);
//                             char* prop = calloc(len+1, sizeof(char));
//                             memcpy(prop, property, len);

//                             // Create a new return element.
//                             Variable* var = NewVariable(collapsedNode->alias, prop);
//                             ReturnElementNode* retElem = NewReturnElementNode(N_PROP, var, NULL, returnElementNode->alias);
//                             Vector_Push(returnElements, retElem);
//                             free(prop);
//                         }
//                     }
//                     RedisModule_FreeCallReply(reply);
//                 } else {
//                     Vector_Push(returnElements, returnElementNode);
//                 }
//             }

//             // Swap collapsed return clause with expended one.
//             // TODO: Free overide vector.
//             returnNode->returnElements = returnElements;
//         }
//         TripletIterator_Free(cursor);
//     }
// }

void ReturnClause_ExpendCollapsedNodes(RedisModuleCtx *ctx, ReturnNode *returnNode, RedisModuleString *graphName, Graph *graph) {
    // TODO: Check if return clause contains a collapsed node before running ExpendReturnClause.
    Vector* entryPoints = Graph_GetNDegreeNodes(graph, 0);
    Node* startNode = NULL;
    Vector_Pop(entryPoints, &startNode);
    // _ExpendReturnClause(ctx, graphName, graph, startNode, entryPoints, returnNode);
}

void nameAnonymousNodes(QueryExpressionNode *ast) {
    Vector *elements = ast->matchNode->chainElements;
    char buff[64];

    // Foreach node
    for(int i = 0; i < Vector_Size(elements); i++) {
        ChainElement *element;
        Vector_Get(elements, i, &element);

        if(element->t != N_ENTITY) {
            continue;
        }
        
        if (element->e.alias == NULL) {
            memset(buff, 0, 64);
            sprintf(buff, "anon_node_%d", i);
            element->e.alias = strdup(buff);
        }
    }
}

void inlineProperties(QueryExpressionNode *ast) {
    // migrate inline filters to WHERE clause
    Vector *elements = ast->matchNode->chainElements;

    // Foreach node
    for(int i = 0; i < Vector_Size(elements); i++) {
        ChainElement *element;
        Vector_Get(elements, i, &element);

        if(element->t != N_ENTITY) {
            continue;
        }

        Vector *properties = element->e.properties;
        if(properties == NULL) {
            continue;
        }

        // Foreach node property
        for(int j = 0; j < Vector_Size(properties); j+=2) {
            SIValue *key;
            SIValue *val;

            Vector_Get(properties, j, &key);
            Vector_Get(properties, j+1, &val);

            const char *alias = element->e.alias;
            const char *property = key->stringval.str;

            FilterNode *filterNode = NewConstantPredicateNode(alias, property, EQ, *val);
            
            // Create WHERE clause if missing.
            if(ast->whereNode == NULL) {
                ast->whereNode = NewWhereNode(filterNode);
            } else {
                // Introduce filter with AND operation
                FilterNode *left = ast->whereNode->filters;
                FilterNode *right = filterNode;
                ast->whereNode->filters = NewConditionNode(left, AND, right);
            }
        }
    }
}

QueryExpressionNode* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    QueryExpressionNode *ast = Query_Parse(query, qLen, errMsg);
    
    if (!ast) {
        return NULL;
    }
    
    // Modify AST
    nameAnonymousNodes(ast);
    inlineProperties(ast);

    return ast;
}