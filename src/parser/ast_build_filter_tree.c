#include "ast_build_filter_tree.h"
#include "ast_shared.h"

FT_FilterNode* CreateCondFilterNode(AST_Operator op) {
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

FT_FilterNode* _CreatePredicateFilterNode(const AST *ast, AST_Operator op, const cypher_astnode_t *lhs, const cypher_astnode_t *rhs) {
    FT_FilterNode *filterNode = malloc(sizeof(FT_FilterNode));
    filterNode->t = FT_N_PRED;
    filterNode->pred.op = op;
    filterNode->pred.lhs = AR_EXP_FromExpression(ast, lhs);
    filterNode->pred.rhs = AR_EXP_FromExpression(ast, rhs);
    return filterNode;
}

// Only used for inlined property filters
FT_FilterNode* _CreatePredicateFilterNodeFromExps(AST_Operator op, AR_ExpNode *lhs, AR_ExpNode *rhs) {
    FT_FilterNode *filterNode = malloc(sizeof(FT_FilterNode));
    filterNode->t = FT_N_PRED;
    filterNode->pred.op = op;
    filterNode->pred.lhs = lhs;
    filterNode->pred.rhs = rhs;
    return filterNode;
}


FT_FilterNode* _CreateFilterNode(const AST *ast, AST_Operator op, const cypher_astnode_t *lhs, const cypher_astnode_t *rhs) {
    FT_FilterNode *filter = NULL;
    switch (op) {
        case OP_OR:
        case OP_AND:
            filter = CreateCondFilterNode(op);
            AppendLeftChild(filter, FilterNode_FromAST(ast, lhs));
            AppendRightChild(filter, FilterNode_FromAST(ast, rhs));
            return filter;
        case OP_EQUAL:
        case OP_NEQUAL:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            return _CreatePredicateFilterNode(ast, op, lhs, rhs);
        default:
            assert("attempted to convert unhandled type into filter" && false);
    }
}

void FT_Append(FT_FilterNode **root_ptr, FT_FilterNode *child) {
    assert(child);

    FT_FilterNode *root = *root_ptr;
    // If the tree is uninitialized, its root is the child
    if (root == NULL) {
        *root_ptr = child;
        return;
    }

    // Promote predicate node to AND condition filter
    if (root->t == FT_N_PRED) {
        FT_FilterNode *new_root = CreateCondFilterNode(OP_AND);
        AppendLeftChild(new_root, root);
        AppendRightChild(new_root, child);
        *root_ptr = new_root;
        return;
    }

    if (root->cond.left == NULL) {
        AppendLeftChild(root, child);
    } else if (root->cond.right == NULL) {
        AppendRightChild(root, child);
    } else {
        FT_FilterNode *new_cond = CreateCondFilterNode(OP_AND);
        AppendLeftChild(new_cond, root);
        AppendRightChild(new_cond, child);
        *root_ptr = new_cond;
    }
}

// AND, OR, XOR (others?)
FT_FilterNode* _convertBinaryOperator(const AST *ast, const cypher_astnode_t *op_node) {
    const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(op_node);
    // Arguments are of type CYPHER_AST_EXPRESSION
    const cypher_astnode_t *lhs = cypher_ast_binary_operator_get_argument1(op_node);
    const cypher_astnode_t *rhs = cypher_ast_binary_operator_get_argument2(op_node);

    AST_Operator op = AST_ConvertOperatorNode(operator);

    return _CreateFilterNode(ast, op, lhs, rhs);
}

/* A comparison node contains two arrays - one of operators, and one of expressions.
 * Most comparisons will only have one operator and two expressions, but Cypher
 * allows more complex formulations like "x < y <= z". */
FT_FilterNode* _convertComparison(const AST *ast, const cypher_astnode_t *comparison_node) {
    unsigned int nelems = cypher_ast_comparison_get_length(comparison_node);
    assert(nelems == 1); // TODO tmp, but may require modifying tree formation.

    const cypher_operator_t *operator = cypher_ast_comparison_get_operator(comparison_node, 0);
    AST_Operator op = AST_ConvertOperatorNode(operator);

    // All arguments are of type CYPHER_AST_EXPRESSION
    const cypher_astnode_t *lhs = cypher_ast_comparison_get_argument(comparison_node, 0);
    const cypher_astnode_t *rhs = cypher_ast_comparison_get_argument(comparison_node, 1);

    return _CreatePredicateFilterNode(ast, op, lhs, rhs);
}

FT_FilterNode* _convertInlinedProperties(AST *ast, const cypher_astnode_t *entity, SchemaType type) {
    const cypher_astnode_t *props = NULL;

    if (type == SCHEMA_NODE) {
        props = cypher_ast_node_pattern_get_properties(entity);
    } else { // relation
        props = cypher_ast_rel_pattern_get_properties(entity);
    }

    if (!props) return NULL;

    AR_ExpNode *exp = AST_GetEntity(ast, entity);
    if (exp->record_idx == NOT_IN_RECORD) {
        exp->record_idx = AST_AddAnonymousRecordEntry(ast);
    }
    unsigned int record_idx = exp->record_idx;

    FT_FilterNode *root = NULL;
    unsigned int nelems = cypher_ast_map_nentries(props);
    for (unsigned int i = 0; i < nelems; i ++) {
        // key is of type CYPHER_AST_PROP_NAME
        const char *prop = cypher_ast_prop_name_get_value(cypher_ast_map_get_key(props, i));
        AR_ExpNode *lhs = AR_EXP_FromInlinedFilter(type, record_idx, prop);
        // val is of type CYPHER_AST_EXPRESSION
        const cypher_astnode_t *val = cypher_ast_map_get_value(props, i);
        AR_ExpNode *rhs = AR_EXP_FromExpression(ast, val);
        /* TODO In a query like:
         * "MATCH (r:person {name:"Roi"}) RETURN r"
         * (note the repeated double quotes) - this creates a variable rather than a scalar.
         * Can we use this to handle escape characters or something? How does it work? */
        // Inlined properties can only be scalars right now
        assert(rhs->operand.type == AR_EXP_CONSTANT && "non-scalar inlined property - add handling for this?");
        // FT_FilterNode *t = _CreatePredicateFilterNode(ast, OP_EQUAL, lhs, rhs);
        FT_FilterNode *t = _CreatePredicateFilterNodeFromExps(OP_EQUAL, lhs, rhs);
        FT_Append(&root, t);
    }
    return root;
}

void _collectFilters(AST *ast, FT_FilterNode **root, const cypher_astnode_t *entity) {
    if (!entity) return;

    cypher_astnode_type_t type = cypher_astnode_type(entity);

    FT_FilterNode *node = NULL;
    // If the current entity is a node or edge pattern, capture its properties map (if any)
    if (type == CYPHER_AST_NODE_PATTERN) {
        node = _convertInlinedProperties(ast, entity, SCHEMA_NODE);
    } else if (type == CYPHER_AST_REL_PATTERN) {
        node = _convertInlinedProperties(ast, entity, SCHEMA_EDGE);
    } else if (type == CYPHER_AST_COMPARISON) {
        node = _convertComparison(ast, entity);
    } else if (type == CYPHER_AST_BINARY_OPERATOR) {
        node = _convertBinaryOperator(ast, entity);
    } else if (type == CYPHER_AST_UNARY_OPERATOR) {
        // TODO, also n-ary maybe
        assert(false);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(entity);
        for(unsigned int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
            // Recursively continue searching
            _collectFilters(ast, root, child);
        }
    }
    if (node) FT_Append(root, node);
}

// TODO use more widely or delete
FT_FilterNode* FilterNode_FromAST(const AST *ast, const cypher_astnode_t *expr) {
    assert(expr);
    cypher_astnode_type_t type = cypher_astnode_type(expr);
    if (type == CYPHER_AST_BINARY_OPERATOR) {
        return _convertBinaryOperator(ast, expr);
    } else if (type == CYPHER_AST_COMPARISON) {
        return _convertComparison(ast, expr);
    } else {
        // AR_ExpNode *lhs = AR_EXP_FromExpression(ast, expr);
    }
    assert(false);
    return NULL;
}

FT_FilterNode* AST_BuildFilterTree(AST *ast) {

    FT_FilterNode *filter_tree = NULL; 
    unsigned int clause_count = cypher_astnode_nchildren(ast->root);
    const cypher_astnode_t *match_clauses[clause_count];
    unsigned int match_count = AST_GetTopLevelClauses(ast, CYPHER_AST_MATCH, match_clauses);
    for (unsigned int i = 0; i < match_count; i ++) {
        _collectFilters(ast, &filter_tree, match_clauses[i]);
    }

    const cypher_astnode_t *merge_clauses[clause_count];
    unsigned int merge_count = AST_GetTopLevelClauses(ast, CYPHER_AST_MERGE, merge_clauses);
    for (unsigned int i = 0; i < merge_count; i ++) {
        _collectFilters(ast, &filter_tree, merge_clauses[i]);
    }
    return filter_tree;
}
