#include "../value.h"
#include "filter_tree.h"
#include "../parser/grammar.h"
#include "../query_executor.h"
#include "../rmutil/vector.h"

FT_FilterNode* LeftChild(const FT_FilterNode *node) { return node->cond.left; }
FT_FilterNode* RightChild(const FT_FilterNode *node) { return node->cond.right; }

int static inline IsNodeConstantPredicate(FT_FilterNode *node) {
    return (node->t == FT_N_PRED && node->pred.t == FT_N_CONSTANT);
}

int static inline IsNodeVaryingPredicate(FT_FilterNode *node) {
    return (node->t == FT_N_PRED && node->pred.t == FT_N_VARYING);
}

int static inline IsNodePredicate(FT_FilterNode *node) {
    return node->t == FT_N_PRED;
}

int static inline IsNodeCondition(FT_FilterNode *node) {
    return node->t == FT_N_COND;
}

int _vectorContains(const Vector *elements, const char *item) {
    int i = 0;
    for(; i < Vector_Size(elements); i++) {
        char *current;
        Vector_Get(elements, i, &current);
        if(strcmp(current, item) == 0) {
            return 1;
        }
    }
    return 0;
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
    if(IsNodePredicate(root)) {
        // A op B
        // extract both A and B values
        Node *node = NULL;
        Edge *edge = NULL;        
        char *entityID = NULL;
        SIValue aVal;
        SIValue bVal;

        if(IsNodeConstantPredicate(root)) {
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
        char* strPropValue = RedisModule_StringPtrLen(propValue, &strProplen);
        // TODO: Free strPropValue.
        strPropValue = strndup(strPropValue, strProplen);
        if (!SI_ParseValue(&aVal, strPropValue, strProplen)) {
            RedisModule_Log(ctx, "error", "Could not parse %.*s\n", (int)strProplen, strPropValue);
            return RedisModule_ReplyWithError(ctx, "Invalid value given");
        }
        RedisModule_FreeString(ctx, propValue);

        return _applyFilter(ctx, &aVal, &bVal, root->pred.cf, root->pred.op);
    }

    // root->t == FT_N_COND

    // Visit left subtree
    int pass = applyFilters(ctx, g, LeftChild(root));
    
    if(root->cond.op == AND && pass == 1) {
        // Visit right subtree
        pass *= applyFilters(ctx, g, RightChild(root));
    }

    if(root->cond.op == OR && pass == 0) {
        // Visit right subtree
        pass = applyFilters(ctx, g, RightChild(root));
    }

    return pass;
}

int FilterTree_ContainsNode(const FT_FilterNode *root, const Vector *aliases) {
    if(root == NULL) {
        return 0;
    }
    
    // Is this a predicate node?
    if(IsNodePredicate(root)) {

        // For const predicate nodes, check only left predicate
        if(IsNodeConstantPredicate(root)) {
            return _vectorContains(aliases, root->pred.Lop.alias);
        }
        
        // For varying predicate nodes, check both left and right predicate
        if(IsNodeVaryingPredicate(root)) {
            return (_vectorContains(aliases, root->pred.Lop.alias) && 
                    _vectorContains(aliases, root->pred.Rop.alias));
        }
    }

    return (FilterTree_ContainsNode(LeftChild(root), aliases) ||
            FilterTree_ContainsNode(RightChild(root), aliases));
}

FT_FilterNode* _FilterTree_ClonePredicateNode(const FT_FilterNode *root) {
    if(IsNodeConstantPredicate(root)) {
        return CreateConstFilterNode(root->pred.Lop.alias, root->pred.Lop.property, root->pred.op, SI_Clone(root->pred.constVal));
    }
    if(IsNodeVaryingPredicate(root) == FT_N_VARYING) {
        return CreateVaryingFilterNode(root->pred.Lop.alias, root->pred.Lop.property, root->pred.Rop.alias, root->pred.Rop.property, root->pred.op);
    }
}

void FilterTree_Clone(const FT_FilterNode *root, FT_FilterNode **clone) {
    if(IsNodePredicate(root)) {
        *clone = _FilterTree_ClonePredicateNode(root);
        return;
    }

    *clone = CreateCondFilterNode(root->cond.op);
    FilterTree_Clone(LeftChild(root), &((*clone)->cond.left));
    FilterTree_Clone(RightChild(root), &((*clone)->cond.right));
}

void _FilterTree_RemovePredNodes(FT_FilterNode **root, const Vector *aliases) {
    if(IsNodeVaryingPredicate(*root)) {
        if(_vectorContains(aliases, (*root)->pred.Lop.alias) &&
        _vectorContains(aliases, (*root)->pred.Rop.alias)) {
            FilterTree_Free(*root);
            *root = NULL;
        }
        return;
    }

    if(IsNodeConstantPredicate(*root)) {
        if(_vectorContains(aliases, (*root)->pred.Lop.alias)) {
            FilterTree_Free(*root);
            *root = NULL;
        }
        return;
    }

    _FilterTree_RemovePredNodes(&((*root)->cond.left), aliases);
    _FilterTree_RemovePredNodes(&((*root)->cond.right), aliases);
}

void FilterTree_RemovePredNodes(FT_FilterNode **root, const Vector *aliases) {
    if(*root == NULL) { return; }
    _FilterTree_RemovePredNodes(root, aliases);
    FilterTree_Squash(root);
}

void FilterTree_RemoveAllNodesExcept(FT_FilterNode **root, Vector *aliases) {
    /* For varying predicate nodes we can discard nodes 
     * either left or right predicate is missing from aliases */
    if(IsNodeVaryingPredicate(*root)) {
        if(!_vectorContains(aliases, (*root)->pred.Lop.alias) ||
           !_vectorContains(aliases, (*root)->pred.Rop.alias)) {
            FilterTree_Free(*root);
            *root = NULL;
        }
        return;
    }

    if(IsNodeConstantPredicate(*root)) {
        // Check if current node is not in aliases.
        if(!_vectorContains(aliases, (*root)->pred.Lop.alias)) {
            FilterTree_Free(*root);
            *root = NULL;
        }
        return;
    }
    
    FilterTree_RemoveAllNodesExcept(&(*root)->cond.left, aliases);
    FilterTree_RemoveAllNodesExcept(&(*root)->cond.right, aliases);
}

void FilterTree_Squash(FT_FilterNode **root) {
    if(*root == NULL) {
        return;
    }

    // We're only interested in conditional nodes.
    if(IsNodePredicate(*root)) {
        return;
    }

    // Left node was removed.
    if((*root)->cond.left == NULL) {
        // TODO: Free current node.
        *root = (*root)->cond.right;
        return FilterTree_Squash(root);
    }

    // Right node was removed.
    if((*root)->cond.right == NULL) {
        // TODO: Free current node.
        *root = (*root)->cond.left;
        return FilterTree_Squash(root);
    }

    FilterTree_Squash(&(*root)->cond.left);
    FilterTree_Squash(&(*root)->cond.right);

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

FT_FilterNode* FilterTree_MinFilterTree(FT_FilterNode *root, Vector *aliases) {
    FT_FilterNode* minTree;
    FilterTree_Clone(root, &minTree);
    FilterTree_RemoveAllNodesExcept(&minTree, aliases);
    FilterTree_Squash(&minTree);
    return minTree;
}

void _FilterTree_Print(const FT_FilterNode *root, int ident) {
    // Ident
    printf("%*s", ident, "");
    
    if(IsNodeConstantPredicate(root)) {
        char value[64] = {0};
        SIValue_ToString(root->pred.constVal, value, 64);
        printf("%s.%s %d %s\n",
            root->pred.Lop.alias,
            root->pred.Lop.property,
            root->pred.op,
            value
        );
        return;
    }
    if(IsNodeVaryingPredicate(root)) {
        printf("%s.%s %d %s.%s\n",
            root->pred.Lop.alias,
            root->pred.Lop.property,
            root->pred.op,
            root->pred.Rop.alias,
            root->pred.Rop.property
        );
        return;
    }

    printf("%d\n", root->cond.op);
    _FilterTree_Print(LeftChild(root), ident+4);
    _FilterTree_Print(RightChild(root), ident+4);
}

void FilterTree_Print(const FT_FilterNode *root) {
    if(root == NULL) {
        printf("empty filter tree\n");
        return; 
    }
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
    if(root == NULL) { return; }
    if(IsNodePredicate(root)) {
        _FilterTree_FreePredNode(root->pred);
    } else {
        FilterTree_Free(root->cond.left);
        FilterTree_Free(root->cond.right);
    }

    free(root);
}