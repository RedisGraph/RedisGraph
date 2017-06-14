#include "graph/node.h"
#include "stores/store.h"
#include "rmutil/vector.h"
#include "rmutil/util.h"
#include "query_executor.h"
#include "parser/grammar.h"
#include "aggregate/agg_ctx.h"
#include "hexastore/triplet.h"
#include "hexastore/hexastore.h"
#include "parser/parser_common.h"
#include "aggregate/repository.h"

Graph* BuildGraph(const MatchNode* matchNode) {
    Graph* g = NewGraph();
    Vector* stack = NewVector(void*, 3);
    
    for(int i = 0; i < Vector_Size(matchNode->graphEntities); i++) {
        GraphEntity *entity;
        Vector_Get(matchNode->graphEntities, i, &entity);

        if(entity->t == N_ENTITY) {
            Node *n = NewNode(entity->alias, NULL, entity->label);
            Graph_AddNode(g, n);
            Vector_Push(stack, n);
        } else {
            Vector_Push(stack, entity);
        }

        if(Vector_Size(stack) == 3) {
            Node* a;
            LinkEntity* edge;
            Node* c;

            Vector_Pop(stack, &c);
            Vector_Pop(stack, &edge);
            Vector_Pop(stack, &a);

            if(edge->direction == N_LEFT_TO_RIGHT) {
                Edge *e = NewEdge(NULL, edge->ge.alias, a, c, edge->ge.label);
                ConnectNode(a, c, e);
            } else {
                Edge *e = NewEdge(NULL, edge->ge.alias, c, a, edge->ge.label);
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

        char *entityID = NULL;
        Node* n = Graph_GetNodeByAlias(g, retElem->variable->alias);
        if(n != NULL) {
            entityID = n->id;
        } else {
            Edge *e = Graph_GetEdgeByAlias(g, retElem->variable->alias);
            if(e != NULL) {
                entityID = e->id;
            }
        }

        if(retElem->variable->property != NULL) {
            RedisModuleString* prop;
            GetElementProperyValue(ctx, entityID, retElem->variable->property, &prop);
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

void ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, QueryExpressionNode *ast, RedisModuleString *graphName) {
    /* Assumption, each collapsed node is tagged with a label
     * TODO: maintain a label schema, this way we won't have
     * to call HGETALL each time to discover label attributes */
    Vector *expandReturnElements = NewVector(ReturnElementNode *, Vector_Size(ast->returnNode->returnElements));

    for(int i = 0; i < Vector_Size(ast->returnNode->returnElements); i++) {
        ReturnElementNode *node;
        Vector_Get(ast->returnNode->returnElements, i, &node);
        
        if(node->type != N_NODE) {
            Vector_Push(expandReturnElements, node);
            continue;
        }
        
        // Find collapsed node's label
        GraphEntity *collapsedNode = NULL;
        for(int j = 0; j < Vector_Size(ast->matchNode->graphEntities); j++) {
            GraphEntity *ge;
            Vector_Get(ast->matchNode->graphEntities, j, &ge);
            if(ge->t != N_ENTITY) {
                continue;
            }
            if(strcmp(ge->alias, node->variable->alias) == 0) {
                collapsedNode = ge;
                break;
            }
        }

        if(collapsedNode == NULL) {
            // Invalud query, return clause refers to none existing node.
            // TODO: Validate query.
            printf("Error, could not find collapsed node\n");
            return;
        }

        // Discard collapsed node.
        FreeReturnElementNode(node);
        
        // Find an id, for node label
        RedisModuleString *label =
            RedisModule_CreateString(ctx, collapsedNode->label, strlen(collapsedNode->label));        
        
        Store *s = GetStore(ctx, STORE_NODE, graphName, label);
        
        RedisModule_FreeString(ctx, label);

        StoreIterator *it = Store_Search(s, "");
        char *id;
        tm_len_t idLen;
        Node *retrievedNode;
        StoreIterator_Next(it, &id, &idLen, &retrievedNode);
        
        RedisModuleString *rmId = RedisModule_CreateString(ctx, id, idLen);
        RMUtilInfo *attributes = RMUtil_HGetAll(ctx, rmId);
        RedisModule_FreeString(ctx, rmId);
        StoreIterator_Free(it);
        
        for(int j = 0; j < attributes->numEntries; j++) {
            RMUtilInfoEntry entry = attributes->entries[j];
            // Create a new return element.
            Variable* var = NewVariable(collapsedNode->alias, entry.key);
            free(entry.key);
            free(entry.val);
            ReturnElementNode* retElem = NewReturnElementNode(N_PROP, var, NULL, NULL);
            Vector_Push(expandReturnElements, retElem);
        }
        RMUtilRedisInfo_Free(attributes);
    }
    // Override previous return clause
    Vector_Free(ast->returnNode->returnElements);
    ast->returnNode->returnElements = expandReturnElements;
}

void nameAnonymousNodes(QueryExpressionNode *ast) {
    Vector *entities = ast->matchNode->graphEntities;
    
    // Foreach graph entity: node/edge.
    for(int i = 0; i < Vector_Size(entities); i++) {
        GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        
        if (entity->alias == NULL) {
            asprintf(&entity->alias, "anon_%d", i);
        }
    }
}

void inlineProperties(QueryExpressionNode *ast) {
    // migrate inline filters to WHERE clause
    Vector *entities = ast->matchNode->graphEntities;

    // Foreach entity
    for(int i = 0; i < Vector_Size(entities); i++) {
        GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        Vector *properties = entity->properties;
        if(properties == NULL) {
            continue;
        }

        // Foreach property
        for(int j = 0; j < Vector_Size(properties); j+=2) {
            SIValue *key;
            SIValue *val;

            Vector_Get(properties, j, &key);
            Vector_Get(properties, j+1, &val);

            const char *alias = entity->alias;
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