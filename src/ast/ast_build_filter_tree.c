#include "ast_build_filter_tree.h"
#include "ast_shared.h"
#include "../execution_plan/record_map.h"
#include "../util/arr.h"

// Forward declaration
FT_FilterNode* _FilterNode_FromAST(RecordMap *record_map, const cypher_astnode_t *expr);

FT_FilterNode* _CreatePredicateFilterNode(RecordMap *record_map, AST_Operator op, const cypher_astnode_t *lhs, const cypher_astnode_t *rhs) {
    return FilterTree_CreatePredicateFilter(op, AR_EXP_FromExpression(record_map, lhs), AR_EXP_FromExpression(record_map, rhs));
}

void _FT_Append(FT_FilterNode **root_ptr, FT_FilterNode *child) {
    assert(child);

    FT_FilterNode *root = *root_ptr;
    // If the tree is uninitialized, its root is the child
    if (root == NULL) {
        *root_ptr = child;
        return;
    }

    if (root->t == FT_N_COND) {
        if (root->cond.left == NULL) {
            AppendLeftChild(root, child);
            return;
        }
        if (root->cond.right == NULL) {
            AppendRightChild(root, child);
            return;
        }
    }

    FT_FilterNode *new_root = FilterTree_CreateConditionFilter(OP_AND);
    AppendLeftChild(new_root, root);
    AppendRightChild(new_root, child);
    *root_ptr = new_root;
}

FT_FilterNode* _CreateFilterSubtree(RecordMap *record_map, AST_Operator op, const cypher_astnode_t *lhs, const cypher_astnode_t *rhs) {
    FT_FilterNode *filter = NULL;
    switch (op) {
        case OP_OR:
        case OP_AND:
            filter = FilterTree_CreateConditionFilter(op);
            AppendLeftChild(filter, _FilterNode_FromAST(record_map, lhs));
            AppendRightChild(filter, _FilterNode_FromAST(record_map, rhs));
            return filter;
        case OP_EQUAL:
        case OP_NEQUAL:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            return _CreatePredicateFilterNode(record_map, op, lhs, rhs);
        default:
            assert("attempted to convert unhandled type into filter" && false);
    }
}

// AND, OR, XOR (others?)
/* WHERE (condition) AND (condition),
 * WHERE a.val = b.val */
FT_FilterNode* _convertBinaryOperator(RecordMap *record_map, const cypher_astnode_t *op_node) {
    const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(op_node);
    // Arguments are of type CYPHER_AST_EXPRESSION
    const cypher_astnode_t *lhs = cypher_ast_binary_operator_get_argument1(op_node);
    const cypher_astnode_t *rhs = cypher_ast_binary_operator_get_argument2(op_node);

    AST_Operator op = AST_ConvertOperatorNode(operator);

    return _CreateFilterSubtree(record_map, op, lhs, rhs);
}

/* A comparison node contains two arrays - one of operators, and one of expressions.
 * Most comparisons will only have one operator and two expressions, but Cypher
 * allows more complex formulations like "x < y <= z".
 * A comparison takes a form such as "WHERE a.val < y.val". */
FT_FilterNode* _convertComparison(RecordMap *record_map, const cypher_astnode_t *comparison_node) {
    unsigned int nelems = cypher_ast_comparison_get_length(comparison_node);
    assert(nelems == 1); // TODO tmp, but may require modifying tree formation.

    const cypher_operator_t *operator = cypher_ast_comparison_get_operator(comparison_node, 0);
    AST_Operator op = AST_ConvertOperatorNode(operator);

    // All arguments are of type CYPHER_AST_EXPRESSION
    const cypher_astnode_t *lhs = cypher_ast_comparison_get_argument(comparison_node, 0);
    const cypher_astnode_t *rhs = cypher_ast_comparison_get_argument(comparison_node, 1);

    return _CreatePredicateFilterNode(record_map, op, lhs, rhs);
}

static FT_FilterNode* _convertInlinedProperties(RecordMap *record_map, const AST *ast,
                                         const cypher_astnode_t *entity, EntityType type) {
    const cypher_astnode_t *props = NULL;

    if (type == ENTITY_NODE) {
        props = cypher_ast_node_pattern_get_properties(entity);
    } else { // relation
        props = cypher_ast_rel_pattern_get_properties(entity);
    }

    if (!props) return NULL;

    // Retrieve the AST ID of the entity.
    uint ast_id = AST_GetEntityIDFromReference(ast, entity);

    // Add the AST ID to the record map.
    uint record_id = RecordMap_FindOrAddID(record_map, ast_id);

    FT_FilterNode *root = NULL;
    unsigned int nelems = cypher_ast_map_nentries(props);
    for (unsigned int i = 0; i < nelems; i ++) {
        // key is of type CYPHER_AST_PROP_NAME
        const char *prop = cypher_ast_prop_name_get_value(cypher_ast_map_get_key(props, i));
        AR_ExpNode *lhs = AR_EXP_NewVariableOperandNode(record_map, NULL, prop);
        lhs->operand.variadic.entity_alias_idx = record_id;
        // val is of type CYPHER_AST_EXPRESSION
        const cypher_astnode_t *val = cypher_ast_map_get_value(props, i);
        AR_ExpNode *rhs = AR_EXP_FromExpression(record_map, val);
        /* TODO In a query like:
         * "MATCH (r:person {name:"Roi"}) RETURN r"
         * (note the repeated double quotes) - this creates a variable rather than a scalar.
         * Can we use this to handle escape characters or something? How does it work? */
        // Inlined properties can only be scalars right now
        assert(rhs->operand.type == AR_EXP_CONSTANT && "non-scalar inlined property are not currently supported.");
        FT_FilterNode *t = FilterTree_CreatePredicateFilter(OP_EQUAL, lhs, rhs);
        _FT_Append(&root, t);
    }
    return root;
}

FT_FilterNode* _FilterNode_FromAST(RecordMap *record_map, const cypher_astnode_t *expr) {
    assert(expr);
    cypher_astnode_type_t type = cypher_astnode_type(expr);
    if (type == CYPHER_AST_BINARY_OPERATOR) {
        return _convertBinaryOperator(record_map, expr);
    } else if (type == CYPHER_AST_COMPARISON) {
        return _convertComparison(record_map, expr);
    }
    assert(false);
    return NULL;
}

void _AST_ConvertFilters(RecordMap *record_map, const AST *ast,
                         FT_FilterNode **root, const cypher_astnode_t *entity) {
    if (!entity) return;

    cypher_astnode_type_t type = cypher_astnode_type(entity);

    FT_FilterNode *node = NULL;
    // If the current entity is a node or edge pattern, capture its properties map (if any)
    if (type == CYPHER_AST_NODE_PATTERN) {
        node = _convertInlinedProperties(record_map, ast, entity, ENTITY_NODE);
    } else if (type == CYPHER_AST_REL_PATTERN) {
        node = _convertInlinedProperties(record_map, ast, entity, ENTITY_EDGE);
    } else if (type == CYPHER_AST_COMPARISON) {
        node = _convertComparison(record_map, entity);
    } else if (type == CYPHER_AST_BINARY_OPERATOR) {
        node = _convertBinaryOperator(record_map, entity);
    } else if (type == CYPHER_AST_UNARY_OPERATOR) {
        // TODO, also n-ary maybe
        assert(false && "unary filters are not currently supported.");
    } else {
        unsigned int child_count = cypher_astnode_nchildren(entity);
        for(unsigned int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
            // Recursively continue searching
            _AST_ConvertFilters(record_map, ast, root, child);
        }
    }
    if (node) _FT_Append(root, node);
}

FT_FilterNode* AST_BuildFilterTree(AST *ast, RecordMap *record_map) {
    FT_FilterNode *filter_tree = NULL;
    const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
    if (match_clauses) {
        uint match_count = array_len(match_clauses);
        for (unsigned int i = 0; i < match_count; i ++) {
            _AST_ConvertFilters(record_map, ast, &filter_tree, match_clauses[i]);
        }
        array_free(match_clauses);
    }

    const cypher_astnode_t **merge_clauses = AST_GetClauses(ast, CYPHER_AST_MERGE);
    if (merge_clauses) {
        uint merge_count = array_len(merge_clauses);
        for (unsigned int i = 0; i < merge_count; i ++) {
            _AST_ConvertFilters(record_map, ast, &filter_tree, merge_clauses[i]);
        }
        array_free(merge_clauses);
    }

    return filter_tree;
}
