/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
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

static AR_ExpNode* _AR_EXP_NewConstOperandNode(SIValue constant) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->operand.type = AR_EXP_CONSTANT;
    node->operand.constant = constant;
    return node;
}

static AR_ExpNode* _AR_EXP_NewVariableOperandNode(const AST *ast, char *entity_prop, char *entity_alias) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity_alias = strdup(entity_alias);
    node->operand.variadic.entity_alias_idx = AST_GetAliasID(ast, entity_alias);
    node->operand.variadic.entity_prop = entity_prop != NULL ? strdup(entity_prop) : NULL;
    return node;
}

static AR_ExpNode* _AR_EXP_NewOpNode(char *func_name, int child_count) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OP;    
    node->op.func_name = func_name;
    node->op.child_count = child_count;
    node->op.children = (AR_ExpNode **)malloc(child_count * sizeof(AR_ExpNode*));

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

AR_ExpNode* AR_EXP_BuildFromAST(const AST *ast, const AST_ArithmeticExpressionNode *exp) {
    AR_ExpNode *root;

    if(exp->type == AST_AR_EXP_OP) {
        root = _AR_EXP_NewOpNode(exp->op.function, Vector_Size(exp->op.args));
        /* Process operands. */
        for(int i = 0; i < root->op.child_count; i++) {
            AST_ArithmeticExpressionNode *child;
            Vector_Get(exp->op.args, i, &child);
            root->op.children[i] = AR_EXP_BuildFromAST(ast, child);
        }
    } else {
        if(exp->operand.type == AST_AR_EXP_CONSTANT) {
            root = _AR_EXP_NewConstOperandNode(exp->operand.constant);
        } else {
            root = _AR_EXP_NewVariableOperandNode(ast,
                                                  exp->operand.variadic.property,
                                                  exp->operand.variadic.alias);
        }
    }

    return root;
}

int AR_EXP_GetOperandType(AR_ExpNode *exp) {
    if (exp->type == AR_EXP_OPERAND) return exp->operand.type;
    return -1;
}

SIValue AR_EXP_Evaluate(const AR_ExpNode *root, const Record r) {
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
                SIValue *property = GraphEntity_Get_Property(ge, root->operand.variadic.entity_prop);
                /* TODO: Handle PROPERTY_NOTFOUND. */
                result = SI_ShallowCopy(*property);
            } else {
                // Alias doesn't necessarily refers to a graph entity,
                // it could also be a constant.
                // TODO: consider moving this logic to a generic method of Record.
                int aliasIdx = root->operand.variadic.entity_alias_idx;
                RecordEntryType t = Record_GetType(r, aliasIdx);
                switch(t) {
                    case REC_TYPE_SCALAR:
                        result = Record_GetScalar(r, aliasIdx);
                        break;
                    case REC_TYPE_NODE:
                    case REC_TYPE_EDGE:
                        result = SI_PtrVal(Record_GetGraphEntity(r, aliasIdx));
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

void AR_EXP_CollectAliases(AR_ExpNode *root, TrieMap *aliases) {
    if (root->type == AR_EXP_OP) {
        for (int i = 0; i < root->op.child_count; i ++) {
            AR_EXP_CollectAliases(root->op.children[i], aliases);
        }
    } else { // type == AR_EXP_OPERAND
        if (root->operand.type == AR_EXP_VARIADIC) {
            char *alias = root->operand.variadic.entity_alias;
            if (alias) TrieMap_Add(aliases, alias, strlen(alias), NULL, NULL);
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

void AR_EXP_Free(AR_ExpNode *root) {
    // TODO: I believe we don't handle freeing aggregated functions correctly
    if(root->type == AR_EXP_OP) {
        for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
            AR_EXP_Free(root->op.children[child_idx]);
        }
        free(root->op.children);
    } else {
        if (root->operand.type == AR_EXP_CONSTANT) {
            SIValue_Free(&root->operand.constant);
        } else {
            if (root->operand.variadic.entity_alias) free(root->operand.variadic.entity_alias);
            if (root->operand.variadic.entity_prop) free(root->operand.variadic.entity_prop);
        }
    }
    free(root);
}

/* Mathematical functions - numeric */

SIValue AR_ADD(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if(SI_TYPE(result) & SI_NUMERIC) {
        /* If the result is numeric, ensure that it is represented as a double. */
        SIValue_ConvertToDouble(&result);
    }
    char buffer[512];
    char *string_arg = NULL;
    double numeric_arg;

    for(int i = 1; i < argc; i++) {
        if(SIValue_IsNull(argv[i])) return SI_NullVal();

        /* Perform numeric addition only if both result and current argument
         * are numeric. */
        if(SI_TYPE(result) & SI_NUMERIC && SI_TYPE(argv[i]) & SI_NUMERIC) {
            SIValue_ToDouble(&argv[i], &numeric_arg);
            /* Numeric addition. */
            result.doubleval += numeric_arg;
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
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
        if(SIValue_IsNull(argv[i])) return SI_NullVal();
        result.doubleval -= argv[i].doubleval;
    }
    return result;
}

SIValue AR_MUL(SIValue *argv, int argc) {
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
        if(SIValue_IsNull(argv[i])) return SI_NullVal();
        /* Assuming every argv is of type double. */
        result.doubleval *= argv[i].doubleval;
    }
    return result;
}

SIValue AR_DIV(SIValue *argv, int argc) {
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
        if(SIValue_IsNull(argv[i])) return SI_NullVal();
        /* Assuming every argv is of type double. */
        /* TODO: division by zero. */
        result.doubleval /= argv[i].doubleval;
    }
    return result;
}

SIValue AR_ABS(SIValue *argv, int argc) {
    SIValue result = argv[0];
    if(result.doubleval < 0) {
        result.doubleval *= -1;
    }
    return result;
}

SIValue AR_CEIL(SIValue *argv, int argc) {
    SIValue result = argv[0];
    result.doubleval = ceil(result.doubleval);
    return result;
}

SIValue AR_FLOOR(SIValue *argv, int argc) {
    SIValue result = argv[0];
    result.doubleval = floor(result.doubleval);
    return result;
}

SIValue AR_RAND(SIValue *argv, int argc) {
    SIValue result = SI_DoubleVal((double)rand() / (double)RAND_MAX);
    return result;
}

SIValue AR_ROUND(SIValue *argv, int argc) {
    SIValue result = argv[0];
    result.doubleval = round(result.doubleval);
    return result;
}

SIValue AR_SIGN(SIValue *argv, int argc) {
    assert(argc == 1);
    if(SIValue_IsNull(argv[0])) return SI_NullVal();

    if(argv[0].doubleval == 0) {
        return SI_DoubleVal(0);
    } else if(argv[0].doubleval < 0) {
        return SI_DoubleVal(-1);
    } else {
        return SI_DoubleVal(1);
    }
}

/* String functions */

SIValue AR_LEFT(SIValue *argv, int argc) {
    assert(argc == 2);
    if(SIValue_IsNull(argv[0])) return SI_NullVal();

    assert(SI_TYPE(argv[0]) & SI_STRING);
    assert(SI_TYPE(argv[1]) == T_DOUBLE);

    size_t newlen = (size_t)argv[1].doubleval;
    if (strlen(argv[0].stringval) <= newlen) {
      // No need to truncate this string based on the requested length
      return SI_DuplicateStringVal(argv[0].stringval);
    }
    char left_str[newlen + 1];
    strncpy(left_str, argv[0].stringval, newlen * sizeof(char));
    left_str[newlen] = '\0';
    return SI_DuplicateStringVal(left_str);
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
    assert(SI_TYPE(argv[1]) == T_DOUBLE);

    int newlen = (int)argv[1].doubleval;
    int start = strlen(argv[0].stringval) - newlen;

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

    int i = strlen(str);
    while(i > 0 && str[i - 1] == ' ') {
      i --;
    }

    char trimmed[i + 1];

    strncpy(trimmed, str, i);
    trimmed[i] = '\0';

    return SI_DuplicateStringVal(trimmed);
}

SIValue AR_REVERSE(SIValue *argv, int argc) {
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    assert(SI_TYPE(argv[0]) & SI_STRING);
    char *str = argv[0].stringval;
    size_t str_len = strlen(str);
    char reverse[str_len + 1];
    
    int i = str_len-1;
    int j = 0;
    while(i >= 0) { reverse[j++] = str[i--]; }
    reverse[j] = '\0';
    return SI_DuplicateStringVal(reverse);
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
    char *original = argv[0].stringval;
    size_t original_len = strlen(original);
    int start = (int)argv[1].doubleval;
    size_t length;

    /* Make sure start doesn't overreach. */
    assert(start < original_len && start >= 0);

    if(argc == 2) {
        length = original_len - start;
    } else {
        assert(argv[2].doubleval >= 0);
        length = (size_t)argv[2].doubleval;
        
        /* Make sure length does not overreach. */
        if(start + length > original_len) {
            length = original_len - start;
        }
    }
    
    char substring[length + 1];
    strncpy(substring, original + start, length);
    substring[length] = '\0';

    return SI_DuplicateStringVal(substring);
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
    char lower[lower_len];
    _toLower(original, lower, &lower_len);
    return SI_DuplicateStringVal(lower);
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
    char upper[upper_len];
    _toUpper(original, upper, &upper_len);
    return SI_DuplicateStringVal(upper);
}

SIValue AR_TOSTRING(SIValue *argv, int argc) {
    assert(argc == 1);

    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    size_t len = 128;
    char str[128] = {0};
    SIValue_ToString(argv[0], str, len);
    return SI_DuplicateStringVal(str);
}

SIValue AR_TRIM(SIValue *argv, int argc) {
    if(SIValue_IsNull(argv[0])) return SI_NullVal();
    SIValue ltrim = AR_LTRIM(argv, argc);
    SIValue trimmed = AR_RTRIM(&ltrim, 1);
    return trimmed;
}

SIValue AR_ID(SIValue *argv, int argc) {
    assert(argc == 1);
    assert(SI_TYPE(argv[0]) == T_PTR);
    GraphEntity *graph_entity = (GraphEntity*)argv[0].ptrval;
    return SI_LongVal(ENTITY_GET_ID(graph_entity));
}

SIValue AR_LABELS(SIValue *argv, int argc) {    
    assert(argc == 1);
    assert(SI_TYPE(argv[0]) == T_PTR);

    char *label = "";
    Node *node = argv[0].ptrval;
    GraphContext *gc = GraphContext_GetFromLTS();
    Graph *g = gc->g;
    int labelID = Graph_GetNodeLabel(g, ENTITY_GET_ID(node));
    if(labelID != GRAPH_NO_LABEL) label = gc->node_stores[labelID]->label;
    return SI_ConstStringVal(label);
}

SIValue AR_TYPE(SIValue *argv, int argc) {
    assert(argc == 1);
    assert(SI_TYPE(argv[0]) == T_PTR);

    char *type = "";
    Edge *e = argv[0].ptrval;
    GraphContext *gc = GraphContext_GetFromLTS();
    Graph *g = gc->g;
    int id = Graph_GetEdgeRelation(gc->g, e);
    if(id != GRAPH_NO_RELATION) type = gc->relation_stores[id]->label;
    return SI_ConstStringVal(type);
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
}
