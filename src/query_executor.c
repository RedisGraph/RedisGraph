#include "query_executor.h"
#include "parser/grammar.h"
#include "rmutil/vector.h"
#include "graph/node.h"

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
            if(!_GetElementProperyValue(ctx, node->id, root->pred.Rop.property, &bVal)) {
                // TODO: Log this missing element / property
                return 0;
            }
        }
        node = Graph_GetNodeByAlias(g, root->pred.Lop.alias);
        if(!_GetElementProperyValue(ctx, node->id, root->pred.Lop.property, &aVal)) {
            // TODO: Log this missing element / property
            return 0;
        }

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

int _GetElementProperyValue(RedisModuleCtx *ctx, const char* elementID, const char* property, SIType* propVal) {
    RedisModuleString* keyStr =
        RedisModule_CreateString(ctx, elementID, strlen(elementID));
    
    RedisModuleKey *key = 
        RedisModule_OpenKey(ctx, keyStr, REDISMODULE_READ);

    if(key == NULL) {
        // TODO: RedisModule_FreeString here crashes
        // RedisModule_FreeString(ctx, keyStr);
        return 0;
    }

    RedisModuleString* propValue = NULL;

    RedisModuleString* elementProp =
        RedisModule_CreateString(ctx, property, strlen(property));

    RedisModule_HashGet(key, REDISMODULE_HASH_NONE, elementProp, &propValue, NULL);

    RedisModule_CloseKey(key);
    RedisModule_FreeString(ctx, keyStr);
    RedisModule_FreeString(ctx, elementProp);

    size_t strProplen;
    const char* strPropValue = RedisModule_StringPtrLen(propValue, &strProplen);
    RedisModule_FreeString(ctx, propValue);

    if (!SI_ParseValue(propVal, strPropValue, strProplen)) {
        RedisModule_Log(ctx, "error", "Could not parse %.*s\n", (int)strProplen, strPropValue);
        return RedisModule_ReplyWithError(ctx, "Invalid value given");
    }

    return 1;
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
