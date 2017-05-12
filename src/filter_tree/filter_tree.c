#include "../value.h"
#include "filter_tree.h"
#include "../parser/grammar.h"
#include "../query_executor.h"

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

FT_FilterNode* _CreateVaryingFilterNode(PredicateNode n) {
    // At the moment only numerical comparison is supported
    // Assuming compared data is double.
    // TODO: support all types of possible SIValues.
    CmpFunc compareFunc = cmp_double;
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));

    // Create predicate node
    filterNode->t = FT_N_PRED;
    filterNode->pred.t = FT_N_VARYING;

    filterNode->pred.Lop.alias = strdup(n.alias);
    filterNode->pred.Lop.property = strdup(n.property);
    filterNode->pred.Rop.alias = strdup(n.nodeVal.alias);
    filterNode->pred.Rop.property = strdup(n.nodeVal.property);

    filterNode->pred.op = n.op;
    filterNode->pred.cf = compareFunc;
    return filterNode;
}

FT_FilterNode* _CreateConstFilterNode(PredicateNode n) {
    CmpFunc compareFunc = NULL;
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));

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
        default:
        compareFunc = NULL;
        break;
    }

    // Couldn't figure out which compare function to use.
    if(compareFunc == NULL) {
        // ERROR.
        return NULL;
    }

    // Create predicate node
    filterNode->t = FT_N_PRED;
    filterNode->pred.t = FT_N_CONSTANT;

    filterNode->pred.Lop.alias = strdup(n.alias);
    filterNode->pred.Lop.property = strdup(n.property);

    filterNode->pred.op = n.op;
    filterNode->pred.constVal = n.constVal; // Not sure about this assignmeant
    filterNode->pred.cf = compareFunc;
    return filterNode;
}

FT_FilterNode* BuildFiltersTree(const FilterNode* root) {
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
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));

	filterNode->t = FT_N_COND;
	filterNode->cond.op = root->cn.op;
	
	filterNode->cond.left = BuildFiltersTree(root->cn.left);
	filterNode->cond.right = BuildFiltersTree(root->cn.right);
	return filterNode;
}

int applyFilters(RedisModuleCtx *ctx, Graph* g, FT_FilterNode* root) {
    // Handle predicate node.
    if(root->t == FT_N_PRED) {
        // A op B
        // extract both A and B values
        Node* node;
        SIValue aVal;
        SIValue bVal;

        if(root->pred.t == FT_N_CONSTANT) {
            bVal = root->pred.constVal;
            aVal.type = bVal.type;
        } else {
            aVal.type = bVal.type = T_DOUBLE; // Default to DOUBLE
            node = Graph_GetNodeByAlias(g, root->pred.Rop.alias);
            if(node->id == NULL) {
                // Missing node's ID, assume TRUE.
                return 1;
            }

            RedisModuleString* propValue;
            GetElementProperyValue(ctx, node->id, root->pred.Rop.property, &propValue);
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
        if(node->id == NULL) {
            // Missing node's ID, assume TRUE.
            return 1;
        }

        RedisModuleString* propValue;
        GetElementProperyValue(ctx, node->id, root->pred.Lop.property, &propValue);
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

    // root->t == FT_N_COND

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

int FilterTree_ContainsNode(const FT_FilterNode *root, const char *alias) {
    if(root == NULL) {
        return 0;
    }
    
    if(root->t == FT_N_PRED) {
        if(strcmp(root->pred.Lop.alias, alias) == 0) {
            return 1;
        }

        if(root->pred.t == FT_N_VARYING) {
            if(strcmp(root->pred.Rop.alias, alias) == 0) {
                return 1;
            }
        }

        return 0;
    }

    if(FilterTree_ContainsNode(root->cond.left, alias)) {
        return 1;
    }
    
    if(FilterTree_ContainsNode(root->cond.right, alias)) {
        return 1;
    }
    
    return 0;
}

void _FreeVaryingFilterNode(FT_PredicateNode node) {
    free(node.Lop.alias);
    free(node.Lop.property);
    free(node.Rop.alias);
    free(node.Rop.property);
}

void _FreeConstFilterNode(FT_PredicateNode node) {
    free(node.Lop.alias);
    free(node.Lop.property);
}

void _FilterTree_FreePredNode(FT_PredicateNode node) {
    if(node.t == FT_N_CONSTANT) {
        _FreeConstFilterNode(node);
    } else {
        _FreeVaryingFilterNode(node);
    }
}

void FilterTree_Free(FT_FilterNode *root) {
    if(root->t == FT_N_PRED) {
        _FilterTree_FreePredNode(root->pred);
    } else {
        FilterTree_Free(root->cond.left);
        FilterTree_Free(root->cond.right);
    }

    free(root);
}