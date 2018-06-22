#include <assert.h>
#include "../value.h"
#include "filter_tree.h"
#include "../parser/grammar.h"
#include "../query_executor.h"
#include "../rmutil/vector.h"

FT_FilterNode* LeftChild(const FT_FilterNode *node) { return node->cond.left; }
FT_FilterNode* RightChild(const FT_FilterNode *node) { return node->cond.right; }

int static inline IsNodeConstantPredicate(const FT_FilterNode *node) {
    return (node->t == FT_N_PRED && node->pred.t == FT_N_CONSTANT);
}

int static inline IsNodeVaryingPredicate(const FT_FilterNode *node) {
    return (node->t == FT_N_PRED && node->pred.t == FT_N_VARYING);
}

int static inline IsNodePredicate(const FT_FilterNode *node) {
    return node->t == FT_N_PRED;
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
    filterNode->pred.Lop.e = NULL;
    filterNode->pred.Rop.alias = strdup(RAlias);
    filterNode->pred.Rop.property = strdup(RProperty);
    filterNode->pred.Rop.e = NULL;

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
    filterNode->pred.Lop.e = NULL;

    filterNode->pred.op = op;
    filterNode->pred.constVal = val;
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

FT_FilterNode* _CreateVaryingFilterNode(AST_PredicateNode n) {
    return CreateVaryingFilterNode(n.alias, n.property, n.nodeVal.alias, n.nodeVal.property, n.op);
}

FT_FilterNode* _CreateConstFilterNode(AST_PredicateNode n) {
    return CreateConstFilterNode(n.alias, n.property, n.op, n.constVal);
}

void _FilterTree_SubTrees(const FT_FilterNode *root, Vector *sub_trees) {
    if (root == NULL) return;

    switch(root->t) {
        case FT_N_PRED:
            /* This is a simple predicate tree, can not traverse further. */
            Vector_Push(sub_trees, root);
            break;
        case FT_N_COND:
            switch(root->cond.op) {
                case AND:
                    /* Break AND down to its components. */
                    _FilterTree_SubTrees(root->cond.left, sub_trees);
                    _FilterTree_SubTrees(root->cond.right, sub_trees);
                    break;
                case OR:
                    /* OR tree must be return as is. */
                    Vector_Push(sub_trees, root);
                    break;
                default:
                    assert(0);
            }
            break;
        default:
            assert(0);
            break;
    }
}

Vector* FilterTree_SubTrees(const FT_FilterNode *root) {
    Vector *sub_trees = NewVector(FT_FilterNode *, 1);
    _FilterTree_SubTrees(root, sub_trees);
    return sub_trees;
}

FT_FilterNode* BuildFiltersTree(const AST_FilterNode *root) {
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

/* Applies a single filter to a single result.
 * Compares given values, tests if values maintain desired relation (op) */
int _applyFilter(SIValue* aVal, SIValue* bVal, CmpFunc f, int op) {
    /* TODO: Make sure values are of the same type
     * TODO: Make sure values type confirms with compare function. */
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

        case NE:
        return rel != 0;

        default:
        /* Op should be enforced by AST. */
        assert(0);
    }
    /* We shouldn't reach this point. */
    return 0;
}

int _applyPredicateFilters(const FT_FilterNode* root) {
    /* A op B
     * Extract both A and B values. */
    GraphEntity *entity;
    SIValue *aVal;
    SIValue *bVal;

    if(IsNodeConstantPredicate(root)) {
        bVal = (SIValue *)&root->pred.constVal;
    } else {
        entity = *root->pred.Rop.e;
        assert (entity && entity->id != INVALID_ENTITY_ID);
        bVal = GraphEntity_Get_Property(entity, root->pred.Rop.property);
    }

    entity = *root->pred.Lop.e;
    assert(entity && entity->id != INVALID_ENTITY_ID);
    aVal = GraphEntity_Get_Property(entity, root->pred.Lop.property);

    return _applyFilter(aVal, bVal, root->pred.cf, root->pred.op);
}

int FilterTree_applyFilters(const FT_FilterNode* root) {
    /* Handle predicate node. */
    if(IsNodePredicate(root)) {
        return _applyPredicateFilters(root);
    }

    /* root->t == FT_N_COND, visit left subtree. */
    int pass = FilterTree_applyFilters(LeftChild(root));
    
    if(root->cond.op == AND && pass == 1) {
        /* Visit right subtree. */
        pass *= FilterTree_applyFilters(RightChild(root));
    }

    if(root->cond.op == OR && pass == 0) {
        /* Visit right subtree. */
        pass = FilterTree_applyFilters(RightChild(root));
    }

    return pass;
}

void FilterTree_bindEntities(FT_FilterNode* root, const QueryGraph *g) {
    switch(root->t) {
        case FT_N_COND:
            FilterTree_bindEntities(root->cond.left, g);
            FilterTree_bindEntities(root->cond.right, g);
            break;
        case FT_N_PRED:
            root->pred.Lop.e = QueryGraph_GetEntityRef(g, root->pred.Lop.alias);
            if(root->pred.t == FT_N_VARYING) {
                root->pred.Rop.e = QueryGraph_GetEntityRef(g, root->pred.Rop.alias);
            }
            break;
        default:
            assert(0);
            break;
    }
}

void _FilterTree_CollectAliases(const FT_FilterNode *root, TrieMap *aliases) {
    if(root == NULL) return;

    switch(root->t) {
        case FT_N_COND:
        {
            _FilterTree_CollectAliases(root->cond.left, aliases);
            _FilterTree_CollectAliases(root->cond.right, aliases);
            break;
        }
        case FT_N_PRED:
        {
            // Add alias to triemap.
            char *alias = root->pred.Lop.alias;
            TrieMap_Add(aliases, alias, strlen(alias), NULL, NULL);
            if(root->pred.t == FT_N_VARYING) {
                alias = root->pred.Rop.alias;
                TrieMap_Add(aliases, alias, strlen(alias), NULL, NULL);
            }
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }
}

Vector *FilterTree_CollectAliases(const FT_FilterNode *root) {
    TrieMap *t = NewTrieMap();
    _FilterTree_CollectAliases(root, t);

    Vector *aliases = NewVector(char*, t->cardinality);
    TrieMapIterator *it = TrieMap_Iterate(t, "", 0);
    
    char *ptr;
    tm_len_t len;
    void *value;

    while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
        Vector_Push(aliases, strdup(ptr));
    }

    TrieMap_Free(t, NULL);
    return aliases;
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
