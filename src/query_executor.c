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

Graph* BuildGraph(const AST_MatchNode *matchNode) {
    Graph* g = NewGraph();
    Vector* stack = NewVector(void*, 3);
    
    for(int i = 0; i < Vector_Size(matchNode->graphEntities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(matchNode->graphEntities, i, &entity);

        if(entity->t == N_ENTITY) {
            Node *n = NewNode(INVALID_ENTITY_ID, entity->label);
            Graph_AddNode(g, n, entity->alias);
            Vector_Push(stack, n);
        } else {
            Vector_Push(stack, entity);
        }

        if(Vector_Size(stack) == 3) {
            Node* a;
            AST_LinkEntity* edge;
            Node* c;

            Vector_Pop(stack, &c);
            Vector_Pop(stack, &edge);
            Vector_Pop(stack, &a);

            if(edge->direction == N_LEFT_TO_RIGHT) {
                Edge *e = NewEdge(INVALID_ENTITY_ID, a, c, edge->ge.label);
                Graph_ConnectNodes(g, a, c, e, edge->ge.alias);
            } else {
                Edge *e = NewEdge(INVALID_ENTITY_ID, c, a, edge->ge.label);
                Graph_ConnectNodes(g, c, a, e, edge->ge.alias);
            }
            
            /* Reintroduce last pushed item. */
            Vector_Push(stack, c);
        } /* If ends. */
    } /* For loop ends. */

    Vector_Free(stack);    
    return g;
}

/* Type specifies the type of return elements to Retrieve. */
Vector* _ReturnClause_RetrieveValues(const AST_ReturnNode *returnNode, const Graph *g, AST_ReturnElementType type) {
    Vector* returned_props = NewVector(SIValue*, Vector_Size(returnNode->returnElements));

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        AST_ReturnElementNode *retElem;
        Vector_Get(returnNode->returnElements, i, &retElem);

        /* Skip elements not of specified type. */
        if(retElem->type != type) {
            continue;
        }

        GraphEntity *e = Graph_GetEntityByAlias(g, retElem->variable->alias);
        SIValue *property = GraphEntity_Get_Property(e, retElem->variable->property);
        if(property == PROPERTY_NOTFOUND) {
            /* Couldn't find prop for id.
             * TODO: Free returned_props. */
            Vector_Free(returned_props);
            return NULL;
        }
        Vector_Push(returned_props, property);
    }

    return returned_props;
}

/* Retrieves all request values specified in return clause
 * Returns a vector of SIValue* */
Vector* ReturnClause_RetrievePropValues(const AST_ReturnNode *returnNode, const Graph *g) {
    return _ReturnClause_RetrieveValues(returnNode, g, N_PROP);
}

/* Retrieves "GROUP BY" values.
 * Returns a vector of SIValue* */
Vector* ReturnClause_RetrieveGroupKeys(const AST_ReturnNode *returnNode, const Graph *g) {
    return _ReturnClause_RetrieveValues(returnNode, g, N_PROP);
}

/* Retrieves all aggregated properties from graph.
 * e.g. SUM(Person.age)
 * Returns a vector of SIValue* */
Vector* ReturnClause_RetrieveGroupAggVals(const AST_ReturnNode *returnNode, const Graph *g) {
    return _ReturnClause_RetrieveValues(returnNode, g, N_AGG_FUNC);
}

int ReturnClause_ContainsCollapsedNodes(const AST_ReturnNode *returnNode) {
    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        AST_ReturnElementNode *returnElementNode;
        Vector_Get(returnNode->returnElements, i, &returnElementNode);
        if(returnElementNode->type == N_NODE) {
            return 1;
        }
    }
    return 0;
}

/* Checks if return clause uses aggregation. */
int ReturnClause_ContainsAggregation(const AST_ReturnNode *returnNode) {
    int res = 0;

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        AST_ReturnElementNode *retElem;
        Vector_Get(returnNode->returnElements, i, &retElem);
        if(retElem->type == N_AGG_FUNC) {
            res = 1;
            break;
        }
    }

    return res;
}

Vector* ReturnClause_GetAggFuncs(RedisModuleCtx *ctx, const AST_ReturnNode *returnNode) {
    Vector* aggFunctions = NewVector(AggCtx*, 0);

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        AST_ReturnElementNode *retElem;
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

void ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, const char *graphName) {
    /* Assumption, each collapsed node is tagged with a label
     * TODO: maintain a label schema, this way we won't have
     * to call HGETALL each time to discover label attributes */
    Vector *expandReturnElements = NewVector(AST_ReturnElementNode*, Vector_Size(ast->returnNode->returnElements));

    for(int i = 0; i < Vector_Size(ast->returnNode->returnElements); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(ast->returnNode->returnElements, i, &ret_elem);
        
        if(ret_elem->type != N_NODE) {
            Vector_Push(expandReturnElements, ret_elem);
            continue;
        }
        
        /* Find collapsed node's label. */
        AST_GraphEntity *collapsed_entity = NULL;
        for(int j = 0; j < Vector_Size(ast->matchNode->graphEntities); j++) {
            AST_GraphEntity *ge;
            Vector_Get(ast->matchNode->graphEntities, j, &ge);
            if(strcmp(ge->alias, ret_elem->variable->alias) == 0) {
                collapsed_entity = ge;
                break;
            }
        }

        if(collapsed_entity == NULL) {
            /* Invalud query, return clause refers to none existing entity. */
            /* TODO: Validate query. */
            printf("Error, could not find collapsed entity\n");
            return;
        }

        /* Discard collapsed node. */
        Free_AST_ReturnElementNode(ret_elem);
        
        /* Find an id, for entity. */
        StoreType store_type = (collapsed_entity->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
        Store *s = GetStore(ctx, store_type, graphName, collapsed_entity->label);
        StoreIterator *it = Store_Search(s, "");
        char *id;
        tm_len_t id_len;
        GraphEntity *entity;
        StoreIterator_Next(it, &id, &id_len, (void**)&entity);
        StoreIterator_Free(it);
        
        for(int j = 0; j < entity->prop_count; j++) {
            /* Create a new return element. */
            AST_Variable *var = New_AST_Variable(collapsed_entity->alias,
                                                 entity->properties[j].name);
            AST_ReturnElementNode *retElem = New_AST_ReturnElementNode(N_PROP, var, NULL, NULL);
            Vector_Push(expandReturnElements, retElem);
        }
    }
    /* Override previous return clause. */
    Vector_Free(ast->returnNode->returnElements);
    ast->returnNode->returnElements = expandReturnElements;
}

void nameAnonymousNodes(AST_QueryExpressionNode *ast) {
    Vector *entities = ast->matchNode->graphEntities;
    
    /* Foreach graph entity: node/edge. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        
        if (entity->alias == NULL) {
            asprintf(&entity->alias, "anon_%d", i);
        }
    }
}

void inlineProperties(AST_QueryExpressionNode *ast) {
    /* Migrate inline filters to WHERE clause. */
    Vector *entities = ast->matchNode->graphEntities;

    /* Foreach entity. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        Vector *properties = entity->properties;
        if(properties == NULL) {
            continue;
        }

        /* Foreach property. */
        for(int j = 0; j < Vector_Size(properties); j+=2) {
            SIValue *key;
            SIValue *val;

            Vector_Get(properties, j, &key);
            Vector_Get(properties, j+1, &val);

            const char *alias = entity->alias;
            const char *property = key->stringval.str;

            AST_FilterNode *filterNode = New_AST_ConstantPredicateNode(alias, property, EQ, *val);
            
            /* Create WHERE clause if missing. */
            if(ast->whereNode == NULL) {
                ast->whereNode = New_AST_WhereNode(filterNode);
            } else {
                /* Introduce filter with AND operation. */
                AST_FilterNode *left = ast->whereNode->filters;
                AST_FilterNode *right = filterNode;
                ast->whereNode->filters = New_AST_ConditionNode(left, AND, right);
            }
        }
    }
}

AST_QueryExpressionNode* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    AST_QueryExpressionNode *ast = Query_Parse(query, qLen, errMsg);
    
    if (!ast) {
        return NULL;
    }
    
    /* Modify AST. */
    nameAnonymousNodes(ast);
    inlineProperties(ast);

    return ast;
}