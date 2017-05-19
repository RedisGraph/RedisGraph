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

FT_FilterNode* CreateVaryingFilterNode(const char *LAlias, const char *LProperty, const char *RAlias, const char *RProperty, int op) {
    // At the moment only numerical comparison is supported
    // Assuming compared data is double.
    // TODO: support all types of possible SIValues.
    CmpFunc compareFunc = cmp_double;
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));

    // Create predicate node
    filterNode->t = FT_N_PRED;
    filterNode->pred.t = FT_N_VARYING;

    filterNode->pred.Lop.alias = strdup(LAlias);
    filterNode->pred.Lop.property = strdup(LProperty);
    filterNode->pred.Rop.alias = strdup(RAlias);
    filterNode->pred.Rop.property = strdup(RProperty);

    filterNode->pred.op = op;
    filterNode->pred.cf = compareFunc;
    return filterNode;
}

FT_FilterNode* CreateConstFilterNode(const char *alias, const char *property, int op, SIValue val) {
    CmpFunc compareFunc = NULL;
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));

    // Find out which compare function should we use.
    switch(val.type) {
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

    filterNode->pred.Lop.alias = strdup(alias);
    filterNode->pred.Lop.property = strdup(property);

    filterNode->pred.op = op;
    filterNode->pred.constVal = val; // Not sure about this assignmeant
    filterNode->pred.cf = compareFunc;
    return filterNode;
}

FT_FilterNode* CreateCondFilterNode(int op) {
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));
    filterNode->t = FT_N_COND;
    filterNode->cond.op = op;
    return filterNode;
}

FT_FilterNode *AppendLeftChild(FT_FilterNode *root, FT_FilterNode *child) {
    root->cond.left = child;
    return root->cond.left;
}

FT_FilterNode *AppendRightChild(FT_FilterNode *root, FT_FilterNode *child) {
    root->cond.right = child;
    return root->cond.right;
}

FT_FilterNode* _CreateVaryingFilterNode(PredicateNode n) {
    return CreateVaryingFilterNode(n.alias, n.property, n.nodeVal.alias, n.nodeVal.property, n.op);
}

FT_FilterNode* _CreateConstFilterNode(PredicateNode n) {
    return CreateConstFilterNode(n.alias, n.property, n.op, n.constVal);
}

FT_FilterNode* BuildFiltersTree(const FilterNode* root) {
    if(root->t == N_PRED) {
        if(root->pn.t == N_CONSTANT) {
            return _CreateConstFilterNode(root->pn);
        } else {
            return _CreateVaryingFilterNode(root->pn);
        }
	}

    // root->t == N_COND
    // Create condition node
    FT_FilterNode* filterNode = CreateCondFilterNode(root->cn.op);
    AppendLeftChild(filterNode, BuildFiltersTree(root->cn.left));
    AppendRightChild(filterNode, BuildFiltersTree(root->cn.right));
	return filterNode;
}

int applyFilters(RedisModuleCtx *ctx, Graph* g, FT_FilterNode* root) {
    // Handle predicate node.
    if(root->t == FT_N_PRED) {
        // A op B
        // extract both A and B values
        Node *node = NULL;
        Edge *edge = NULL;        
        char *entityID = NULL;
        SIValue aVal;
        SIValue bVal;

        if(root->pred.t == FT_N_CONSTANT) {
            bVal = root->pred.constVal;
            aVal.type = bVal.type;
        } else {
            aVal.type = bVal.type = T_DOUBLE; // Default to DOUBLE
            node = Graph_GetNodeByAlias(g, root->pred.Rop.alias);
            if(node != NULL) {
                entityID = node->id;
            } else {
                edge = Graph_GetEdgeByAlias(g, root->pred.Rop.alias);
                if(edge != NULL) {
                    entityID = edge->id;
                }
            }
            if(entityID == NULL) {
                // Missing node's ID, assume TRUE.
                return 1;
            }

            RedisModuleString* propValue;
            GetElementProperyValue(ctx, entityID, root->pred.Rop.property, &propValue);
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

        entityID = NULL;
        node = Graph_GetNodeByAlias(g, root->pred.Lop.alias);
        if(node != NULL) {
            entityID = node->id;
        } else {
            edge = Graph_GetEdgeByAlias(g, root->pred.Lop.alias);
            if(edge != NULL) {
                entityID = edge->id;
            }
        }

        if(entityID == NULL) {
            // Missing node's ID, assume TRUE.
            return 1;
        }

        RedisModuleString* propValue;
        GetElementProperyValue(ctx, entityID, root->pred.Lop.property, &propValue);
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

FT_FilterNode* _FilterTree_ClonePredicateNode(const FT_FilterNode *root) {
    if(root->pred.t == FT_N_CONSTANT) {
        return CreateConstFilterNode(root->pred.Lop.alias, root->pred.Lop.property, root->pred.op, SI_Clone(root->pred.constVal));
    }
    if(root->pred.t == FT_N_VARYING) {
        return CreateVaryingFilterNode(root->pred.Lop.alias, root->pred.Lop.property, root->pred.Rop.alias, root->pred.Rop.property, root->pred.op);
    }
}

void FilterTree_Clone(const FT_FilterNode *root, FT_FilterNode **clone) {
    if(root->t == FT_N_PRED) {
        *clone = _FilterTree_ClonePredicateNode(root);
        return;
    }

    *clone = CreateCondFilterNode(root->cond.op);
    FilterTree_Clone(root->cond.left, &(*clone)->cond.left);
    FilterTree_Clone(root->cond.right, &(*clone)->cond.right);
}

void _FilterTree_PredicatePruning(FT_FilterNode **root, const char *alias) {
    if((*root)->t == FT_N_PRED) {
        if(strcmp((*root)->pred.Lop.alias, alias) != 0) {
            FilterTree_Free(*root);
            *root = NULL;
        }
        return;
    }

    _FilterTree_PredicatePruning(&(*root)->cond.left, alias);
    _FilterTree_PredicatePruning(&(*root)->cond.right, alias);
}

void _FilterTree_Squash(FT_FilterNode **root) {
    if(*root == NULL) {
        return;
    }

    // We're only interested in conditional nodes.
    if((*root)->t == FT_N_PRED) {
        return;
    }

    // Left node was removed.
    if((*root)->cond.left == NULL) {
        // TODO: Free current node.
        *root = (*root)->cond.right;
        return _FilterTree_Squash(root);
    }

    if((*root)->cond.right == NULL) {
        // TODO: Free current node.
        *root = (*root)->cond.left;
        return _FilterTree_Squash(root);
    }

    _FilterTree_Squash(&(*root)->cond.left);
    _FilterTree_Squash(&(*root)->cond.right);

    // on return check squashed children
    if((*root)->cond.left == NULL) {
        // TODO: Free current node.
        *root = (*root)->cond.right;
        return;
    }

    if((*root)->cond.right == NULL) {
        // TODO: Free current node.
        *root = (*root)->cond.left;
        return;
    }
}

FT_FilterNode* FilterTree_MinFilterTree(FT_FilterNode *root, const char *alias) {
    FT_FilterNode* minTree;
    FilterTree_Clone(root, &minTree);
    _FilterTree_PredicatePruning(&minTree, alias);
    _FilterTree_Squash(&minTree);
    return minTree;
}

void _FilterTree_Print(const FT_FilterNode *root, int ident) {
    // Ident
    printf("%*s", ident, "");
    
    if(root->t == FT_N_PRED) {
        if(root->pred.t == FT_N_CONSTANT) {
            char value[64] = {0};
            SIValue_ToString(root->pred.constVal, value, 64);
            printf("%s.%s %d %s\n",
                root->pred.Lop.alias,
                root->pred.Lop.property,
                root->pred.op,
                value
            );
        } else {
            printf("%s.%s %d %s.%s\n",
                root->pred.Lop.alias,
                root->pred.Lop.property,
                root->pred.op,
                root->pred.Rop.alias,
                root->pred.Rop.property
            );
        }
        return;
    }

    printf("%d\n", root->cond.op);
    _FilterTree_Print(root->cond.left, ident+4);
    _FilterTree_Print(root->cond.right, ident+4);
}

void FilterTree_Print(const FT_FilterNode *root) {
    _FilterTree_Print(root, 0);
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