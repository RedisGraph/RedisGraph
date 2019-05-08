/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./arithmetic_expression.h"
#include "./aggregate.h"
#include "./repository.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../util/triemap/triemap.h"

#include "assert.h"
#include <math.h>
#include <ctype.h>

/* Arithmetic function repository. */
static TrieMap *__aeRegisteredFuncs = NULL;

/* Returns 1 if the operand is a numeric type, and 0 otherwise.
 * This also rejects NULL values. */
static inline int _validate_numeric(const SIValue v) {
    return SI_TYPE(v) & SI_NUMERIC;
}

bool _AR_SetFunction(AR_ExpNode *exp, AST_Operator op) {
    switch(op) {
        case OP_PLUS:
            exp->op.f = AR_ADD;
            exp->op.func_name = "ADD";
            return true;
        case OP_MINUS:
            exp->op.f = AR_SUB;
            exp->op.func_name = "SUB";
            return true;
        case OP_MULT:
            exp->op.f = AR_MUL;
            exp->op.func_name = "MUL";
            return true;
        case OP_DIV:
            exp->op.f = AR_DIV;
            exp->op.func_name = "DIV";
            return true;
        case OP_MOD: // TODO implement
        case OP_POW: // TODO implement
        // Includes operators like <, AND, etc
        default:
            assert(false);
            return NULL;
    }
}

static AR_ExpNode* _AR_EXP_NewOpNodeFromAST(AST_Operator op, int child_count) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OP;
    node->record_idx = NOT_IN_RECORD;
    node->op.func_name = NULL; // TODO needed?
    node->op.child_count = child_count;
    node->op.children = (AR_ExpNode **)malloc(child_count * sizeof(AR_ExpNode*));

    /* Determine function type. */
    node->op.type = AR_OP_FUNC;
    _AR_SetFunction(node, op);

    return node;

}

// TODO we don't really have as many func_name strings as before (they were generated in grammar.y)
// maybe replace with above
static AR_ExpNode* _AR_EXP_NewOpNode(char *func_name, int child_count) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OP;
    node->record_idx = NOT_IN_RECORD;
    node->op.func_name = func_name;
    node->op.child_count = child_count;
    node->op.children = rm_malloc(child_count * sizeof(AR_ExpNode*));

    /* Determine function type. */
    AR_Func func = AR_GetFunc(func_name);
    if(func != NULL) {
        node->op.f = func;
        node->op.type = AR_OP_FUNC;
    } else {
        /* Either this is an aggregation function
         * or the requested function does not exists. */
        AggCtx* agg_func;
        Agg_GetFunc(func_name, &agg_func);

        /* TODO: handle Unknown function. */
        assert(agg_func != NULL);
        node->op.agg_func = agg_func;
        node->op.type = AR_OP_AGGREGATE;
    }

    return node;
}

AR_ExpNode* AR_EXP_NewVariableOperandNode(const AST *ast, const cypher_astnode_t *entity, const char *alias, const char *prop) {
    AR_ExpNode *node = malloc(sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->record_idx = NOT_IN_RECORD;
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity_alias = strdup(alias);
    unsigned int id = AST_GetAliasID(ast, (char*)alias); // TODO still sensible?
    node->operand.variadic.entity_alias_idx = id;
    node->operand.variadic.entity_prop = (prop) ? strdup(prop) : NULL;
    node->operand.variadic.ast_ref = entity;

    if(prop) {
        // Retrieve the property from the schema of the base entity (not this entity,
        // which is just a PROPERTY_OPERATOR)
        AR_ExpNode *base_entity = ast->defined_entities[id];
        cypher_astnode_type_t type = cypher_astnode_type(base_entity->operand.variadic.ast_ref);
        if (type == CYPHER_AST_NODE_PATTERN) {
            node->operand.variadic.entity_prop_idx = Attribute_GetID(SCHEMA_NODE, prop);
        } else { // (type == CYPHER_AST_REL_PATTERN)
            node->operand.variadic.entity_prop_idx = Attribute_GetID(SCHEMA_EDGE, prop);
        }
    }
    return node;
}

AR_ExpNode* AR_EXP_FromInlinedFilter(SchemaType base_type, unsigned int record_idx, const char *prop) {
    AR_ExpNode *node = malloc(sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->record_idx = NOT_IN_RECORD; // TODO right?
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity_alias = NULL;
    node->operand.variadic.entity_alias_idx = record_idx;
    node->operand.variadic.entity_prop = (prop) ? strdup(prop) : NULL;
    // node->operand.variadic.ast_ref = entity;

    if(prop) { // TODO necessary?
        // Retrieve the property from the schema of the base entity (not this entity,
        // which is just a PROPERTY_OPERATOR)
        node->operand.variadic.entity_prop_idx = Attribute_GetID(base_type, prop);
    }
    return node;

}

AR_ExpNode* AR_EXP_NewConstOperandNode(SIValue constant) {
    AR_ExpNode *node = malloc(sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->record_idx = NOT_IN_RECORD;
    node->operand.type = AR_EXP_CONSTANT;
    node->operand.constant = constant;
    return node;
}

AR_ExpNode* AR_EXP_DuplicateAggFunc(const AR_ExpNode *expr) {
    assert(expr->type == AR_EXP_OP && expr->op.type == AR_OP_AGGREGATE);
    AR_ExpNode *clone = malloc(sizeof(AR_ExpNode));
    clone->type = AR_EXP_OP;
    clone->op.type = AR_OP_AGGREGATE;
    clone->record_idx = expr->record_idx;
    char *func_name = expr->op.func_name;
    clone->op.func_name = func_name;

    AggCtx* agg_func;
    Agg_GetFunc(func_name, &agg_func);
    clone->op.agg_func = agg_func;
    clone->op.child_count = expr->op.child_count;
    clone->op.children = expr->op.children; //  TODO sufficient?

    return clone;
}

void AR_EXP_AssignRecordIndex(AR_ExpNode *exp, unsigned int idx) {
    exp->record_idx = idx;
}

AR_ExpNode* AR_EXP_FromExpression(const AST *ast, const cypher_astnode_t *expr) {
    const cypher_astnode_type_t type = cypher_astnode_type(expr);

    /* Function invocations*/
    if (type == CYPHER_AST_APPLY_OPERATOR || type == CYPHER_AST_APPLY_ALL_OPERATOR) {
        // TODO handle CYPHER_AST_APPLY_ALL_OPERATOR
        const cypher_astnode_t *func_node = cypher_ast_apply_operator_get_func_name(expr);
        const char *func_name = cypher_ast_function_name_get_value(func_node);
        // TODO When implementing calls like COUNT(DISTINCT), use cypher_ast_apply_operator_get_distinct()
        unsigned int arg_count = cypher_ast_apply_operator_narguments(expr);
        AR_ExpNode *op = _AR_EXP_NewOpNode((char*)func_name, arg_count);
        for (unsigned int i = 0; i < arg_count; i ++) {
            const cypher_astnode_t *arg = cypher_ast_apply_operator_get_argument(expr, i);
            // Recursively convert arguments
            op->op.children[i] = AR_EXP_FromExpression(ast, arg);
        }
        return op;

    /* Variables (full nodes and edges, UNWIND artifacts */
    } else if (type == CYPHER_AST_IDENTIFIER) {
        // Identifier referencing another AST entity
        const char *alias = cypher_ast_identifier_get_name(expr);
        return AR_EXP_NewVariableOperandNode(ast, expr, alias, NULL);

    /* Entity-property pair */
    } else if (type == CYPHER_AST_PROPERTY_OPERATOR) {
        // Identifier and property pair
        // Extract the entity alias from the property. Currently, the embedded
        // expression should only refer to the IDENTIFIER type.
        const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(expr);
        assert(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
        const char *alias = cypher_ast_identifier_get_name(prop_expr);
        // Extract the property name
        const cypher_astnode_t *prop_name_node = cypher_ast_property_operator_get_prop_name(expr);
        const char *prop_name = cypher_ast_prop_name_get_value(prop_name_node);
        return AR_EXP_NewVariableOperandNode(ast, expr, alias, prop_name);

    /* SIValue constant types */
    } else if (type == CYPHER_AST_INTEGER) {
        const char *value_str = cypher_ast_integer_get_valuestr(expr);
        char *endptr = NULL;
        int64_t l = strtol(value_str, &endptr, 0);
        assert(endptr[0] == 0);
        SIValue converted = SI_LongVal(l);
        return AR_EXP_NewConstOperandNode(converted);
    } else if (type == CYPHER_AST_FLOAT) {
        const char *value_str = cypher_ast_float_get_valuestr(expr);
        char *endptr = NULL;
        double d = strtod(value_str, &endptr);
        assert(endptr[0] == 0);
        SIValue converted = SI_DoubleVal(d);
        return AR_EXP_NewConstOperandNode(converted);
    } else if (type == CYPHER_AST_STRING) {
        const char *value_str = cypher_ast_string_get_value(expr);
        SIValue converted = SI_DuplicateStringVal(value_str); // TODO use const strings instead?
        return AR_EXP_NewConstOperandNode(converted);
    } else if (type == CYPHER_AST_TRUE) {
        SIValue converted = SI_BoolVal(true);
        return AR_EXP_NewConstOperandNode(converted);
    } else if (type == CYPHER_AST_FALSE) {
        SIValue converted = SI_BoolVal(false);
        return AR_EXP_NewConstOperandNode(converted);
    } else if (type == CYPHER_AST_NULL) {
        SIValue converted = SI_NullVal();
        return AR_EXP_NewConstOperandNode(converted);

    /* Comparison/filter types */
    // TODO Possibly implement these, replacing AST_Filter types
    } else if (type == CYPHER_AST_UNARY_OPERATOR) {
        // unary
        const cypher_astnode_t *arg = cypher_ast_unary_operator_get_argument(expr); // CYPHER_AST_EXPRESSION
        const cypher_operator_t *operator = cypher_ast_unary_operator_get_operator(expr);
        if (operator == CYPHER_OP_UNARY_MINUS) {
            // -3 is a unary minus operation on the integer 3
            // TODO I feel like this is really over-elaborate - handle it in parser?
            // TODO this can obviously be improved greatly
            AR_ExpNode *ar_exp = AR_EXP_FromExpression(ast, arg);
            SIValue exp = AR_EXP_Evaluate(ar_exp, NULL);
            exp = SIValue_Subtract(SI_LongVal(0), exp);
            return AR_EXP_NewConstOperandNode(exp);
        } else if (operator == CYPHER_OP_UNARY_PLUS) {
            assert(false); // TODO
        } else if (operator == CYPHER_OP_NOT) {
            assert(false); // TODO
        }
    } else if (type == CYPHER_AST_BINARY_OPERATOR) {
        const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(expr);
        AST_Operator operator_enum = AST_ConvertOperatorNode(operator);
        // Arguments are of type CYPHER_AST_EXPRESSION
        AR_ExpNode *op = _AR_EXP_NewOpNodeFromAST(operator_enum, 2);
        const cypher_astnode_t *lhs_node = cypher_ast_binary_operator_get_argument1(expr);
        op->op.children[0] = AR_EXP_FromExpression(ast, lhs_node);
        const cypher_astnode_t *rhs_node = cypher_ast_binary_operator_get_argument2(expr);
        op->op.children[1] = AR_EXP_FromExpression(ast, rhs_node);
        return op;

        // binary comparison
        // TODO handle here?
        // CYPHER_OP_OR;
        // CYPHER_OP_XOR;
        // CYPHER_OP_AND;
        // CYPHER_OP_EQUAL;
        // CYPHER_OP_NEQUAL;
        // CYPHER_OP_LT;
        // CYPHER_OP_GT;
        // CYPHER_OP_LTE;
        // CYPHER_OP_GTE;
        // CYPHER_OP_PLUS;
        // CYPHER_OP_MINUS;
        // CYPHER_OP_MULT;
        // CYPHER_OP_DIV;
        // CYPHER_OP_MOD;
        // CYPHER_OP_POW;
        // More, STARTS_WITH and stuff?
    } else if (type == CYPHER_AST_COMPARISON) {
        // generic comparison

        printf("\ngot binary\n");

    } else {
    /*
       Unhandled types:
       CYPHER_AST_COLLECTION
       CYPHER_AST_CASE
       CYPHER_AST_LABELS_OPERATOR
       CYPHER_AST_LIST_COMPREHENSION
       CYPHER_AST_MAP
       CYPHER_AST_MAP_PROJECTION
       CYPHER_AST_PARAMETER
       CYPHER_AST_PATTERN_COMPREHENSION
       CYPHER_AST_SLICE_OPERATOR
       CYPHER_AST_REDUCE // TODO what is this?
       CYPHER_AST_SUBSCRIPT_OPERATOR
    */
        printf("Encountered unhandled type '%s'\n", cypher_astnode_typestr(type));
        assert(false);
    }

    assert(false);
    return NULL;
}

int AR_EXP_GetOperandType(AR_ExpNode *exp) {
    if (exp->type == AR_EXP_OPERAND) return exp->operand.type;
    return -1;
}

/* Update arithmetic expression variable node by setting node's property index.
 * when constructing an arithmetic expression we'll delay setting graph entity 
 * attribute index to the first execution of the expression, this is due to 
 * entity aliasing where we lose track over which entity is aliased, consider
 * MATCH (n:User) WITH n AS x RETURN x.name 
 * When constructing the arithmetic expression x.name, we don't know
 * who X is referring to. */
void _AR_EXP_UpdatePropIdx(AR_ExpNode *root, const Record r) {
    RecordEntryType t = Record_GetType(r, root->operand.variadic.entity_alias_idx);
    SchemaType st = (t == REC_TYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
    root->operand.variadic.entity_prop_idx = Attribute_GetID(st, root->operand.variadic.entity_prop);
}

SIValue AR_EXP_Evaluate(AR_ExpNode *root, const Record r) {
    SIValue result;
    /* Deal with Operation node. */
    if(root->type == AR_EXP_OP) {
        /* Aggregation function should be reduced by now.
         * TODO: verify above statement. */
        if(root->op.type == AR_OP_AGGREGATE) {
            AggCtx *agg = root->op.agg_func;
            result = agg->result;
        } else {
            /* Evaluate each child before evaluating current node. */
            SIValue sub_trees[root->op.child_count];
            for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
                sub_trees[child_idx] = AR_EXP_Evaluate(root->op.children[child_idx], r);
            }
            /* Evaluate self. */
            result = root->op.f(sub_trees, root->op.child_count);
        }
    } else {
        /* Deal with a constant node. */
        if(root->operand.type == AR_EXP_CONSTANT) {
            result = root->operand.constant;
        } else {
            // Fetch entity property value.
            if (root->operand.variadic.entity_prop != NULL) {
                GraphEntity *ge = Record_GetGraphEntity(r, root->operand.variadic.entity_alias_idx);
                if(root->operand.variadic.entity_prop_idx == ATTRIBUTE_NOTFOUND) {
                    _AR_EXP_UpdatePropIdx(root, r);
                }
                SIValue *property = GraphEntity_GetProperty(ge, root->operand.variadic.entity_prop_idx);
                if(property == PROPERTY_NOTFOUND) result = SI_NullVal();
                else result = SI_ShallowCopy(*property);
            } else {
                // Alias doesn't necessarily refers to a graph entity,
                // it could also be a constant.
                // TODO: consider moving this logic to a generic method of Record.
                int aliasIdx = root->operand.variadic.entity_alias_idx;
                // int aliasIdx = (root->record_idx == NOT_IN_RECORD) ? root->operand.variadic.entity_alias_idx : root->record_idx;
                RecordEntryType t = Record_GetType(r, aliasIdx);
                switch(t) {
                    case REC_TYPE_SCALAR:
                        result = Record_GetScalar(r, aliasIdx);
                        break;
                    case REC_TYPE_NODE:
                        result = SI_Node(Record_GetGraphEntity(r, aliasIdx));
                        break;
                    case REC_TYPE_EDGE:
                        result = SI_Edge(Record_GetGraphEntity(r, aliasIdx));
                        break;
                    default:
                        assert(false);
                }
            }
        }
    }
    return result;
}

void AR_EXP_Aggregate(const AR_ExpNode *root, const Record r) {
    if(root->type == AR_EXP_OP) {
        if(root->op.type == AR_OP_AGGREGATE) {
            /* Process child nodes before aggregating. */
            SIValue sub_trees[root->op.child_count];
            int i = 0;
            for(; i < root->op.child_count; i++) {
                AR_ExpNode *child = root->op.children[i];
                sub_trees[i] = AR_EXP_Evaluate(child, r);
            }

            /* Aggregate. */
            AggCtx *agg = root->op.agg_func;
            agg->Step(agg, sub_trees, root->op.child_count);
        } else {
            /* Keep searching for aggregation nodes. */
            for(int i = 0; i < root->op.child_count; i++) {
                AR_ExpNode *child = root->op.children[i];
                AR_EXP_Aggregate(child, r);
            }
        }
    }
}

void AR_EXP_Reduce(const AR_ExpNode *root) {
    if(root->type == AR_EXP_OP) {
        if(root->op.type == AR_OP_AGGREGATE) {
            /* Reduce. */
            AggCtx *agg = root->op.agg_func;
            Agg_Finalize(agg);
        } else {
            /* Keep searching for aggregation nodes. */
            for(int i = 0; i < root->op.child_count; i++) {
                AR_ExpNode *child = root->op.children[i];
                AR_EXP_Reduce(child);
            }
        }
    }
}

void AR_EXP_CollectEntityIDs(AR_ExpNode *root, TrieMap *record_ids) {
    if (root->type == AR_EXP_OP) {
        for (int i = 0; i < root->op.child_count; i ++) {
            AR_EXP_CollectEntityIDs(root->op.children[i], record_ids);
        }
    } else { // type == AR_EXP_OPERAND
        if (root->operand.type == AR_EXP_VARIADIC) {
            int record_idx = root->operand.variadic.entity_alias_idx;
            assert(record_idx != NOT_IN_RECORD);
            TrieMap_Add(record_ids, (char*)&record_idx, sizeof(record_idx), NULL, NULL);
        }
    }
}

int AR_EXP_ContainsAggregation(AR_ExpNode *root, AR_ExpNode **agg_node) {
    if(root->type == AR_EXP_OP && root->op.type == AR_OP_AGGREGATE) {
        if(agg_node != NULL) *agg_node = root;
        return 1;
    }

    if(root->type == AR_EXP_OP) {
        for(int i = 0; i < root->op.child_count; i++) {
            AR_ExpNode *child = root->op.children[i];
            if(AR_EXP_ContainsAggregation(child, agg_node)) {
                return 1;
            }
        }
    }

    return 0;
}

void _AR_EXP_ToString(const AR_ExpNode *root, char **str, size_t *str_size, size_t *bytes_written) {
    /* Make sure there are at least 64 bytes in str. */
    if(*str == NULL) {
        *bytes_written = 0;
        *str_size = 128;
        *str = rm_calloc(*str_size, sizeof(char));
    }

    if((*str_size - strlen(*str)) < 64) {
        *str_size += 128;
        *str = rm_realloc(*str, sizeof(char) * *str_size);
    }

    /* Concat Op. */
    if(root->type == AR_EXP_OP) {
        /* Binary operation? */
        char binary_op = 0;

        if (strcmp(root->op.func_name, "ADD") == 0) binary_op = '+';
        else if (strcmp(root->op.func_name, "SUB") == 0) binary_op = '-';
        else if (strcmp(root->op.func_name, "MUL") == 0) binary_op = '*';
        else if (strcmp(root->op.func_name, "DIV") == 0)  binary_op = '/';

        if(binary_op) {
            _AR_EXP_ToString(root->op.children[0], str, str_size, bytes_written);

            /* Make sure there are at least 64 bytes in str. */
            if((*str_size - strlen(*str)) < 64) {
                *str_size += 128;
                *str = rm_realloc(*str, sizeof(char) * *str_size);
            }

            *bytes_written += sprintf((*str + *bytes_written), " %c ", binary_op);

            _AR_EXP_ToString(root->op.children[1], str, str_size, bytes_written);
        } else {
            /* Operation isn't necessarily a binary operation, use function call representation. */
            *bytes_written += sprintf((*str + *bytes_written), "%s(", root->op.func_name);

            for(int i = 0; i < root->op.child_count ; i++) {
                _AR_EXP_ToString(root->op.children[i], str, str_size, bytes_written);

                /* Make sure there are at least 64 bytes in str. */
                if((*str_size - strlen(*str)) < 64) {
                    *str_size += 128;
                    *str = rm_realloc(*str, sizeof(char) * *str_size);
                }
                if(i < (root->op.child_count-1)) {
                    *bytes_written += sprintf((*str + *bytes_written), ",");
                }
            }
            *bytes_written += sprintf((*str + *bytes_written), ")");
        }
    } else {
        // Concat Operand node.
        if (root->operand.type == AR_EXP_CONSTANT) {
            size_t len = SIValue_ToString(root->operand.constant, (*str + *bytes_written), 64);
            *bytes_written += len;
        } else {
            if (root->operand.variadic.entity_prop != NULL) {
                *bytes_written += sprintf(
                    (*str + *bytes_written), "%s.%s",
                    root->operand.variadic.entity_alias, root->operand.variadic.entity_prop);
            } else {
                *bytes_written += sprintf((*str + *bytes_written), "%s", root->operand.variadic.entity_alias);
            }
        }
    }
}

void AR_EXP_ToString(const AR_ExpNode *root, char **str) {
    size_t str_size = 0;
    size_t bytes_written = 0;
    *str = NULL;
    _AR_EXP_ToString(root, str, &str_size, &bytes_written);
}

static AR_ExpNode* _AR_EXP_CloneOperand(AR_ExpNode* exp) {
    AR_ExpNode *clone = rm_calloc(1, sizeof(AR_ExpNode));
    clone->type = AR_EXP_OPERAND;
    clone->record_idx = NOT_IN_RECORD;
    switch(exp->operand.type) {
        case AR_EXP_CONSTANT:
            clone->operand.type = AR_EXP_CONSTANT;
            clone->operand.constant = exp->operand.constant;
            break;
        case AR_EXP_VARIADIC:
            clone->operand.type = AR_EXP_VARIADIC;
            clone->operand.variadic.entity_alias = rm_strdup(exp->operand.variadic.entity_alias);
            clone->operand.variadic.entity_alias_idx = exp->operand.variadic.entity_alias_idx;
            if(exp->operand.variadic.entity_prop) {
                clone->operand.variadic.entity_prop = rm_strdup(exp->operand.variadic.entity_prop);
            }
            clone->operand.variadic.entity_prop_idx = exp->operand.variadic.entity_prop_idx;
            break;
        default:
            assert(false);
            break;
    }
    return clone;
}

static AR_ExpNode* _AR_EXP_CloneOp(AR_ExpNode* exp) {
    AR_ExpNode *clone = _AR_EXP_NewOpNode(exp->op.func_name, exp->op.child_count);
    for(uint i = 0; i < exp->op.child_count; i++) {
        AR_ExpNode *child = AR_EXP_Clone(exp->op.children[i]);
        clone->op.children[i] = child;
    }
    return clone;
}

AR_ExpNode* AR_EXP_Clone(AR_ExpNode* exp) {
    AR_ExpNode *clone;
    switch(exp->type) {
        case AR_EXP_OPERAND:
            clone = _AR_EXP_CloneOperand(exp);
            break;
        case AR_EXP_OP:
            clone = _AR_EXP_CloneOp(exp);
            break;
        default:
            assert(false);
            break;
    }
    return clone;
}

void AR_EXP_Free(AR_ExpNode *root) {
    // if(root->type == AR_EXP_OP) {
        // for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
            // AR_EXP_Free(root->op.children[child_idx]);
        // }
        // rm_free(root->op.children);
        // if (root->op.type == AR_OP_AGGREGATE) {
            // AggCtx_Free(root->op.agg_func);
        // }
    // } else {
        // if (root->operand.type == AR_EXP_CONSTANT) {
            // SIValue_Free(&root->operand.constant);
        // } else {
            // if (root->operand.variadic.entity_alias) rm_free(root->operand.variadic.entity_alias);
            // if (root->operand.variadic.entity_prop) rm_free(root->operand.variadic.entity_prop);
        // }
    // }
    // rm_free(root);
}

/* Mathematical functions - numeric */

/* The '+' operator is overloaded to perform string concatenation
 * as well as arithmetic addition. */
SIValue AR_ADD(SIValue *argv, int argc) {
    SIValue result = argv[0];
    char buffer[512];
    char *string_arg = NULL;

    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    for(int i = 1; i < argc; i++) {
        if(SIValue_IsNull(argv[i])) return SI_NullVal();

        /* Perform numeric addition only if both result and current argument
         * are numeric. */
        if(_validate_numeric(result) && _validate_numeric(argv[i])) {
            result = SIValue_Add(result, argv[i]);
        } else {
            /* String concatenation.
             * Make sure result is a String. */
            if(SI_TYPE(result) & SI_NUMERIC) {
                /* Result is numeric, convert to string. */
                SIValue_ToString(result, buffer, 512);
                result = SI_DuplicateStringVal(buffer);
            } else {
                /* Result is already a string,
                 * Make sure result owns the string. */
                if(SI_TYPE(result) & T_CONSTSTRING) {
                    result = SI_DuplicateStringVal(result.stringval);
                }
            }

            /* Get a string representation of argument. */
            unsigned int argument_len = 0;
            if(!(SI_TYPE(argv[i]) & SI_STRING)) {
                /* Argument is not a string, get a string representation. */
                argument_len = SIValue_ToString(argv[i], buffer, 512);
                string_arg = buffer;
            } else {
                string_arg = argv[i].stringval;
                argument_len = strlen(string_arg);
            }

            /* Concat, make sure result has enough space to hold new string. */
            unsigned int required_size = strlen(result.stringval) + argument_len + 1;
            result.stringval = rm_realloc(result.stringval, required_size);
            strcat(result.stringval, string_arg);
        }
    }

    return result;
}

SIValue AR_SUB(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();

    for(int i = 1; i < argc; i++) {
        if (!_validate_numeric(argv[i])) return SI_NullVal();
        result = SIValue_Subtract(result, argv[i]);
    }
    return result;
}

SIValue AR_MUL(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();

    for(int i = 1; i < argc; i++) {
        if (!_validate_numeric(argv[i])) return SI_NullVal();
        result = SIValue_Multiply(result, argv[i]);
    }
    return result;
}

SIValue AR_DIV(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();

    for(int i = 1; i < argc; i++) {
        if (!_validate_numeric(argv[i])) return SI_NullVal();
        result = SIValue_Divide(result, argv[i]);
    }
    return result;
}

/* TODO All AR_* functions need to emit appropriate failures when provided
 * with arguments of invalid types and handle multiple arguments. */

SIValue AR_ABS(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();
    if(SI_GET_NUMERIC(argv[0]) < 0) return SIValue_Multiply(argv[0], SI_LongVal(-1));
    return argv[0];
}

SIValue AR_CEIL(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();
    // No modification is required for non-decimal values
    if (SI_TYPE(result) == T_DOUBLE) result.doubleval = ceil(result.doubleval);

    return result;
}

SIValue AR_FLOOR(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();
    // No modification is required for non-decimal values
    if (SI_TYPE(result) == T_DOUBLE) result.doubleval = floor(result.doubleval);

    return result;
}

SIValue AR_RAND(SIValue *argv, int argc) {
    return SI_DoubleVal((double)rand() / (double)RAND_MAX);
}

SIValue AR_ROUND(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if (!_validate_numeric(result)) return SI_NullVal();
    // No modification is required for non-decimal values
    if (SI_TYPE(result) == T_DOUBLE) result.doubleval = round(result.doubleval);

    return result;
}

SIValue AR_SIGN(SIValue *argv, int argc) {
    if (!_validate_numeric(argv[0])) return SI_NullVal();
    int64_t sign = SIGN(SI_GET_NUMERIC(argv[0]));
    return SI_LongVal(sign);
}

/* String functions */

SIValue AR_LEFT(SIValue *argv, int argc) {
    assert(argc == 2);
    if(SIValue_IsNull(argv[0])) return SI_NullVal();

    assert(SI_TYPE(argv[0]) & SI_STRING);
    assert(SI_TYPE(argv[1]) == T_INT64);

    int64_t newlen = argv[1].longval;
    if (strlen(argv[0].stringval) <= newlen) {
      // No need to truncate this string based on the requested length
      return SI_DuplicateStringVal(argv[0].stringval);
    }
    char *left_str = rm_malloc((newlen + 1) * sizeof(char));
    strncpy(left_str, argv[0].stringval, newlen * sizeof(char));
    left_str[newlen] = '\0';
    return SI_TransferStringVal(left_str);
}

SIValue AR_LTRIM(SIValue *argv, int argc) {
    if(SIValue_IsNull(argv[0])) return SI_NullVal();

    assert(argc == 1 && SI_TYPE(argv[0]) & SI_STRING);

    char *trimmed = argv[0].stringval;

    while(*trimmed == ' ') {
      trimmed ++;
    }

    return SI_DuplicateStringVal(trimmed);
}

SIValue AR_RIGHT(SIValue *argv, int argc) {
    assert(argc == 2);
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    assert(SI_TYPE(argv[0]) & SI_STRING);
    assert(SI_TYPE(argv[1]) == T_INT64);

    int64_t newlen = argv[1].longval;
    int64_t start = strlen(argv[0].stringval) - newlen;

    if (start <= 0) {
      // No need to truncate this string based on the requested length
      return SI_DuplicateStringVal(argv[0].stringval);
    }
    return SI_DuplicateStringVal(argv[0].stringval + start);
}

SIValue AR_RTRIM(SIValue *argv, int argc) {
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    assert(argc == 1 && SI_TYPE(argv[0]) & SI_STRING);

    char *str = argv[0].stringval;

    size_t i = strlen(str);
    while(i > 0 && str[i - 1] == ' ') {
      i --;
    }

    char *trimmed = rm_malloc((i + 1) * sizeof(char));
    strncpy(trimmed, str, i);
    trimmed[i] = '\0';

    return SI_TransferStringVal(trimmed);
}

SIValue AR_REVERSE(SIValue *argv, int argc) {
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    assert(SI_TYPE(argv[0]) & SI_STRING);
    char *str = argv[0].stringval;
    size_t str_len = strlen(str);
    char *reverse = rm_malloc((str_len + 1) * sizeof(char));

    int i = str_len-1;
    int j = 0;
    while(i >= 0) { reverse[j++] = str[i--]; }
    reverse[j] = '\0';
    return SI_TransferStringVal(reverse);
}

SIValue AR_SUBSTRING(SIValue *argv, int argc) {
    /*
        argv[0] - original string
        argv[1] - start position
        argv[2] - length
        If length is omitted, the function returns the substring starting at the position given by start and extending to the end of original.
        If either start or length is null or a negative integer, an error is raised.
        If start is 0, the substring will start at the beginning of original.
        If length is 0, the empty string will be returned.
    */
    assert(argc > 1);
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    assert(SI_TYPE(argv[1]) == T_INT64);

    char *original = argv[0].stringval;
    int64_t original_len = strlen(original);
    int64_t start = argv[1].longval;
    int64_t length;

    /* Make sure start doesn't overreach. */
    assert(start < original_len && start >= 0);

    if(argc == 2) {
        length = original_len - start;
    } else {
        assert(SI_TYPE(argv[2]) == T_INT64);
        length = argv[2].longval;
        assert(length >= 0);

        /* Make sure length does not overreach. */
        if(start + length > original_len) {
            length = original_len - start;
        }
    }

    char *substring = rm_malloc((length + 1) * sizeof(char));
    strncpy(substring, original + start, length);
    substring[length] = '\0';

    return SI_TransferStringVal(substring);
}

void _toLower(const char *str, char *lower, size_t *lower_len) {
    size_t str_len = strlen(str);
    /* Avoid overflow. */
    assert(*lower_len > str_len);

    /* Update lower len*/
    *lower_len = str_len;

    int i = 0;
    for(; i < str_len; i++) lower[i] = tolower(str[i]);
    lower[i] = 0;
}

SIValue AR_TOLOWER(SIValue *argv, int argc) {
    assert(argc == 1);

    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    char *original = argv[0].stringval;
    size_t lower_len = strlen(original) + 1;
    char *lower = rm_malloc((lower_len + 1) * sizeof(char));
    _toLower(original, lower, &lower_len);
    return SI_TransferStringVal(lower);
}

void _toUpper(const char *str, char *upper, size_t *upper_len) {
    size_t str_len = strlen(str);
    /* Avoid overflow. */
    assert(*upper_len > str_len);

    /* Update upper len*/
    *upper_len = str_len;

    int i = 0;
    for(; i < str_len; i++) upper[i] = toupper(str[i]);
    upper[i] = 0;
}

SIValue AR_TOUPPER(SIValue *argv, int argc) {
    assert(argc == 1);

    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    char *original = argv[0].stringval;
    size_t upper_len = strlen(original) + 1;
    char *upper = rm_malloc((upper_len + 1) * sizeof(char));
    _toUpper(original, upper, &upper_len);
    return SI_TransferStringVal(upper);
}

SIValue AR_TOSTRING(SIValue *argv, int argc) {
    assert(argc == 1);

    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    size_t len = SIValue_StringConcatLen(argv, 1);
    char *str = rm_malloc(len * sizeof(char));
    SIValue_ToString(argv[0], str, len);
    return SI_TransferStringVal(str);
}

SIValue AR_TRIM(SIValue *argv, int argc) {
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    SIValue ltrim = AR_LTRIM(argv, argc);
    SIValue trimmed = AR_RTRIM(&ltrim, 1);
    return trimmed;
}

SIValue AR_ID(SIValue *argv, int argc) {
    assert(argc == 1);
    assert(SI_TYPE(argv[0]) & (T_NODE | T_EDGE));
    GraphEntity *graph_entity = (GraphEntity*)argv[0].ptrval;
    return SI_LongVal(ENTITY_GET_ID(graph_entity));
}

SIValue AR_LABELS(SIValue *argv, int argc) {
    assert(argc == 1);
    // TODO Introduce AST validations to prevent non-nodes
    // from reaching here
    if (SI_TYPE(argv[0]) != T_NODE) return SI_NullVal();

    char *label = "";
    Node *node = argv[0].ptrval;
    GraphContext *gc = GraphContext_GetFromTLS();
    Graph *g = gc->g;
    int labelID = Graph_GetNodeLabel(g, ENTITY_GET_ID(node));
    if(labelID != GRAPH_NO_LABEL) label = gc->node_schemas[labelID]->name;
    return SI_ConstStringVal(label);
}

SIValue AR_TYPE(SIValue *argv, int argc) {
    assert(argc == 1);
    // TODO Introduce AST validations to prevent non-edges
    // from reaching here
    if (SI_TYPE(argv[0]) != T_EDGE) return SI_NullVal();

    char *type = "";
    Edge *e = argv[0].ptrval;
    GraphContext *gc = GraphContext_GetFromTLS();
    int id = Graph_GetEdgeRelation(gc->g, e);
    if(id != GRAPH_NO_RELATION) type = gc->relation_schemas[id]->name;
    return SI_ConstStringVal(type);
}

SIValue AR_EXISTS(SIValue *argv, int argc) {
    assert(argc == 1);
    /* MATCH (n) WHERE EXISTS(n.name) RETURN n
     * In case n.name does not exists
     * SIValue representing NULL is returned.
     * if n.name exists its value can not be NULL. */
    if(SIValue_IsNull(argv[0])) return SI_BoolVal(0);
    return SI_BoolVal(1);
}

void AR_RegFunc(char *func_name, size_t func_name_len, AR_Func func) {
    if (__aeRegisteredFuncs == NULL) {
        __aeRegisteredFuncs = NewTrieMap();
    }

    TrieMap_Add(__aeRegisteredFuncs, func_name, func_name_len, func, NULL);
}

AR_Func AR_GetFunc(char *func_name) {
    char lower_func_name[32] = {0};
    size_t lower_func_name_len = 32;
    _toLower(func_name, &lower_func_name[0], &lower_func_name_len);
    void *f = TrieMap_Find(__aeRegisteredFuncs, lower_func_name, lower_func_name_len);
    if(f != TRIEMAP_NOTFOUND) {
        return f;
    }
    return NULL;
}

bool AR_FuncExists(const char *func_name) {
    char lower_func_name[32] = {0};
    size_t lower_func_name_len = 32;
    _toLower(func_name, &lower_func_name[0], &lower_func_name_len);
    void *f = TrieMap_Find(__aeRegisteredFuncs, lower_func_name, lower_func_name_len);
    return (f != TRIEMAP_NOTFOUND);
}

void AR_RegisterFuncs() {
    char lower_func_name[32] = {0};
    size_t lower_func_name_len = 32;

    _toLower("add", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_ADD);
    lower_func_name_len = 32;

    _toLower("sub", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_SUB);
    lower_func_name_len = 32;

    _toLower("mul", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_MUL);
    lower_func_name_len = 32;

    _toLower("div", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_DIV);
    lower_func_name_len = 32;

    _toLower("abs", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_ABS);
    lower_func_name_len = 32;

    _toLower("ceil", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_CEIL);
    lower_func_name_len = 32;

    _toLower("floor", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_FLOOR);
    lower_func_name_len = 32;

    _toLower("rand", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_RAND);
    lower_func_name_len = 32;

    _toLower("round", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_ROUND);
    lower_func_name_len = 32;

    _toLower("sign", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_SIGN);
    lower_func_name_len = 32;


    /* String operations. */
    _toLower("left", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_LEFT);
    lower_func_name_len = 32;

    _toLower("reverse", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_REVERSE);
    lower_func_name_len = 32;

    _toLower("right", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_RIGHT);
    lower_func_name_len = 32;

    _toLower("ltrim", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_LTRIM);
    lower_func_name_len = 32;

    _toLower("rtrim", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_RTRIM);
    lower_func_name_len = 32;

    _toLower("substring", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_SUBSTRING);
    lower_func_name_len = 32;

    _toLower("tolower", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_TOLOWER);
    lower_func_name_len = 32;

    _toLower("toupper", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_TOUPPER);
    lower_func_name_len = 32;

    _toLower("tostring", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_TOSTRING);
    lower_func_name_len = 32;

    _toLower("trim", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_TRIM);
    lower_func_name_len = 32;

    _toLower("id", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_ID);
    lower_func_name_len = 32;

    _toLower("labels", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_LABELS);
    lower_func_name_len = 32;

    _toLower("type", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_TYPE);
    lower_func_name_len = 32;

    _toLower("exists", &lower_func_name[0], &lower_func_name_len);
    AR_RegFunc(lower_func_name, lower_func_name_len, AR_EXISTS);
    lower_func_name_len = 32;    
}
