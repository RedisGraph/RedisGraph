#include "query_executor.h"
#include "parser/grammar.h"
#include "rmutil/vector.h"
#include "graph/node.h"
#include "aggregate/agg_ctx.h"

Graph* BuildGraph(const MatchNode* matchNode) {
    Graph* g = NewGraph();
    Vector* stack = NewVector(ChainElement*, 3);
    
    for(int i = 0; i < Vector_Size(matchNode->chainElements); i++) {
        ChainElement* element;
        Vector_Get(matchNode->chainElements, i, &element);
        Vector_Push(stack, element);

        if(Vector_Size(stack) == 3) {
            ChainElement* a;
            ChainElement* edge;
            ChainElement* c; 
            Vector_Pop(stack, &c);
            Vector_Pop(stack, &edge);
            Vector_Pop(stack, &a);

            // TODO: Validate a, edge, c types
            Node* src;
            Node* dest;

            if(edge->l.direction == N_LEFT_TO_RIGHT) {
                src = Graph_AddNode(g, a->e.alias, a->e.id);
                dest = Graph_AddNode(g, c->e.alias, c->e.id);                
            } else {
                src = Graph_AddNode(g, c->e.alias, c->e.id);
                dest = Graph_AddNode(g, a->e.alias, a->e.id);
            }
            ConnectNode(src, dest, edge->l.relationship);
            
            // reintroduce last pushed item
            Vector_Push(stack, c);
        } // if ends
    } // For loop ends

    Vector_Free(stack);    
    return g;
}

QE_FilterNode* _CreateVaryingFilterNode(PredicateNode n) {
    // At the moment only numerical comparison is supported
    // Assuming compared data is double.
    // TODO: support all types of possible SIValues.
    CmpFunc compareFunc = cmp_double;
    QE_FilterNode* filterNode = (QE_FilterNode*)malloc(sizeof(QE_FilterNode));

    // Create predicate node
    filterNode->t = QE_N_PRED;
    filterNode->pred.t = QE_N_VARYING;

    filterNode->pred.Lop.alias = 
        (char*)malloc(sizeof(char) * (strlen(n.alias) + 1));

    filterNode->pred.Lop.property = 
        (char*)malloc(sizeof(char) * (strlen(n.property) + 1));

    filterNode->pred.Rop.alias = 
        (char*)malloc(sizeof(char) * (strlen(n.nodeVal.alias) + 1));

    filterNode->pred.Rop.property = 
        (char*)malloc(sizeof(char) * (strlen(n.nodeVal.property) + 1));

    strcpy(filterNode->pred.Lop.alias, n.alias);
    strcpy(filterNode->pred.Lop.property, n.property);
    strcpy(filterNode->pred.Rop.alias, n.nodeVal.alias);
    strcpy(filterNode->pred.Rop.property, n.nodeVal.property);

    filterNode->pred.op = n.op;
    filterNode->pred.cf = compareFunc;
    return filterNode;
}

QE_FilterNode* _CreateConstFilterNode(PredicateNode n) {
    CmpFunc compareFunc = NULL;
    QE_FilterNode* filterNode = (QE_FilterNode*)malloc(sizeof(QE_FilterNode));

    // Find out which compare function should we use.
    switch(n.constVal.type) {
        case T_STRING:
        compareFunc = cmp_string;
        break;
        case T_INT32:
        compareFunc = cmp_int;
        break;
        case T_INT64:
        compareFunc = cmp_long;
        break;
        case T_UINT:
        compareFunc = cmp_uint;
        break;
        case T_BOOL:
        compareFunc = cmp_int;
        break;
        case T_FLOAT:
        compareFunc = cmp_float;
        break;
        case T_DOUBLE:
        compareFunc = cmp_double;
        break;
    }

    // Couldn't figure out which compare function to use.
    if(compareFunc == NULL) {
        // ERROR.
        return NULL;
    }

    // Create predicate node
    filterNode->t = QE_N_PRED;
    filterNode->pred.t = QE_N_CONSTANT;

    filterNode->pred.Lop.alias = 
        (char*)malloc(sizeof(char) * (strlen(n.alias) + 1));

    filterNode->pred.Lop.property = 
        (char*)malloc(sizeof(char) * (strlen(n.property) + 1));

    strcpy(filterNode->pred.Lop.alias, n.alias);
    strcpy(filterNode->pred.Lop.property, n.property);

    filterNode->pred.op = n.op;
    filterNode->pred.constVal = n.constVal; // Not sure about this assignmeant
    filterNode->pred.cf = compareFunc;
    return filterNode;
}

QE_FilterNode* BuildFiltersTree(const FilterNode* root) {
    if(root == NULL) {
        return NULL;
    }
    
    if(root->t == N_PRED) {
        if(root->pn.t == N_CONSTANT) {
            return _CreateConstFilterNode(root->pn);
        } else {
            return _CreateVaryingFilterNode(root->pn);
        }
	}

    // root->t == N_COND

    // Create condition node
    QE_FilterNode* filterNode = (QE_FilterNode*)malloc(sizeof(QE_FilterNode));

	filterNode->t = QE_N_COND;
	filterNode->cond.op = root->cn.op;
	
	filterNode->cond.left = BuildFiltersTree(root->cn.left);
	filterNode->cond.right = BuildFiltersTree(root->cn.right);
	return filterNode;
}

RedisModuleString* _GetElementProperyValue(RedisModuleCtx *ctx, const char* elementID, const char* property) {
    RedisModuleString* keyStr =
        RedisModule_CreateString(ctx, elementID, strlen(elementID));

    RedisModuleKey *key = 
        RedisModule_OpenKey(ctx, keyStr, REDISMODULE_READ);

    if(key == NULL) {
        // TODO: RedisModule_FreeString here crashes
        // RedisModule_FreeString(ctx, keyStr);
        return NULL;
    }

    RedisModuleString* propValue = NULL;

    RedisModuleString* elementProp =
        RedisModule_CreateString(ctx, property, strlen(property));

    RedisModule_HashGet(key, REDISMODULE_HASH_NONE, elementProp, &propValue, NULL);

    RedisModule_CloseKey(key);
    RedisModule_FreeString(ctx, keyStr);
    RedisModule_FreeString(ctx, elementProp);

    return propValue;
}

int applyFilters(RedisModuleCtx *ctx, Graph* g, QE_FilterNode* root) {
    // Handle predicate node.
    if(root->t == QE_N_PRED) {
        // A op B
        // extract both A and B values
        Node* node;
        SIValue aVal;
        SIValue bVal;

        if(root->pred.t == QE_N_CONSTANT) {
            bVal = root->pred.constVal;
            aVal.type = bVal.type;
        } else {
            aVal.type = bVal.type = T_DOUBLE; // Default to DOUBLE
            node = Graph_GetNodeByAlias(g, root->pred.Rop.alias);
            RedisModuleString* propValue = _GetElementProperyValue(ctx, node->id, root->pred.Rop.property);
            if(propValue == NULL) {
                // TODO: Log this missing element / property
                return 0;
            }

            // Cast from RedisModuleString to SIValue.
            size_t strProplen = 0;
            const char* strPropValue = RedisModule_StringPtrLen(propValue, &strProplen);
            if (!SI_ParseValue(&bVal, strPropValue, strProplen)) {
                RedisModule_Log(ctx, "error", "Could not parse %.*s\n", (int)strProplen, strPropValue);
                return RedisModule_ReplyWithError(ctx, "Invalid value given");
            }
            RedisModule_FreeString(ctx, propValue);
        }

        node = Graph_GetNodeByAlias(g, root->pred.Lop.alias);
        RedisModuleString* propValue = _GetElementProperyValue(ctx, node->id, root->pred.Lop.property);
        if(propValue == NULL) {
                // TODO: Log this missing element / property
                return 0;
        }
        // Cast from RedisModuleString to SIValue.
        size_t strProplen = 0;
        const char* strPropValue = RedisModule_StringPtrLen(propValue, &strProplen);
        if (!SI_ParseValue(&aVal, strPropValue, strProplen)) {
            RedisModule_Log(ctx, "error", "Could not parse %.*s\n", (int)strProplen, strPropValue);
            return RedisModule_ReplyWithError(ctx, "Invalid value given");
        }
        RedisModule_FreeString(ctx, propValue);

        return _applyFilter(ctx, &aVal, &bVal, root->pred.cf, root->pred.op);
    }

    // root->t == QE_N_COND

    // Visit left subtree
    int pass = applyFilters(ctx, g, root->cond.left);
    
    if(root->cond.op == AND && pass == 1) {
        // Visit right subtree
        pass *= applyFilters(ctx, g, root->cond.right);
    }

    if(root->cond.op == OR && pass == 0) {
        // Visit right subtree
        pass = applyFilters(ctx, g, root->cond.right);
    }

    return pass;
}

// Applies a single filter to a single result.
// Compares given values, tests if values maintain desired relation (op)
int _applyFilter(RedisModuleCtx *ctx, SIValue* aVal, SIValue* bVal, CmpFunc f, int op) {
    // TODO: Make sure values are of the same type
    // TODO: Make sure values type confirms with compare function.
    int rel = f(aVal, bVal);

    switch(op) {
        case EQ:
        return rel == 0;

        case GT:
        return rel > 0;

        case GE:
        return rel >= 0;

        case LT:
        return rel < 0;

        case LE:
        return rel <= 0;

        default:
            RedisModule_Log(ctx, "error", "Unknown comparison operator %d\n", op);
            return RedisModule_ReplyWithError(ctx, "Invalid comparison operator");
    }
}

// type specifies the type of return elements to retrive
Vector* _ReturnClause_RetriveValues(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g, ReturnElementType type) {
    Vector* returnedProps = NewVector(RedisModuleString*, Vector_Size(returnNode->returnElements));

    for(int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
        ReturnElementNode* retElem;
        Vector_Get(returnNode->returnElements, i, &retElem);

        // Skip elements not of specified type
        if(retElem->type != type) {
            continue;
        }

        Node* n = Graph_GetNodeByAlias(g, retElem->alias);

        if(retElem->property != NULL) {
            Vector_Push(returnedProps, _GetElementProperyValue(ctx, n->id, retElem->property));
        } else {
            // Couldn't find an API for HGETALL.
        }
    } // End of for loop

    return returnedProps;
}

// Retrives all request values specified in return clause
// Returns a vector of RedisModuleString*
Vector* ReturnClause_RetrivePropValues(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g) {
    return _ReturnClause_RetriveValues(ctx, returnNode, g, N_PROP);
}

// Retrives "GROUP BY" values.
// Returns a vector of RedisModuleString*
Vector* ReturnClause_RetriveGroupKeys(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g) {
    return _ReturnClause_RetriveValues(ctx, returnNode, g, N_PROP);
}

// Retrives all aggregated properties from graph.
// e.g. SUM(Person.age)
// Returns a vector of RedisModuleString*
Vector* ReturnClause_RetriveGroupAggVals(RedisModuleCtx *ctx, const ReturnNode* returnNode, const Graph* g) {
    return _ReturnClause_RetriveValues(ctx, returnNode, g, N_AGG_FUNC);
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
                return RedisModule_ReplyWithError(ctx, "Invalid aggregation function");
            }
            Vector_Push(aggFunctions, func);
        }
    }

    return aggFunctions;
}