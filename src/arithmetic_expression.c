#include "arithmetic_expression.h"
#include "./util/triemap/triemap.h"
#include "./aggregate/aggregate.h"
#include "./aggregate/repository.h"

#include "assert.h"
#include <math.h>
#include <ctype.h>

/* Arithmetic function repository. */
static TrieMap *__aeRegisteredFuncs = NULL;

SIValue AR_EXP_Evaluate(const AR_ExpNode *root) {
    SIValue result;
    SIValue sub_trees[32];
    
    /* Deal with Operation node. */
    if(root->type == AR_EXP_OP) {
        /* Aggregation function should be reduced by now. 
         * TODO: verify above statement. */
        if(root->op.type == AR_OP_AGGREGATE) {
            AggCtx *agg = root->op.agg_func;
            result = agg->result;
        } else {
            /* Evaluate each child before evaluating current node. */
            /* TODO: deal with root->child_count > sizeof(sub_trees). */
            for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
                sub_trees[child_idx] = AR_EXP_Evaluate(root->op.children[child_idx]);
            }
            /* Evaluate self. */
            result = root->op.f(sub_trees, root->op.child_count);
        }
    } else {
        /* Deal with a constant node. */
        if(root->operand.type == AR_EXP_CONSTANT) {
            result = root->operand.constant;
        } else {
            /* Fetch entity property value. */
            SIValue *property = GraphEntity_Get_Property(*root->operand.variadic.entity, root->operand.variadic.entity_prop);
            /* TODO: Handle PROPERTY_NOTFOUND. */
            result = *property;
        }
    }

    return result;
}

void AR_EXP_Aggregate(const AR_ExpNode *root) {
    SIValue sub_trees[32];

    if(root->type == AR_EXP_OP) {
        if(root->op.type == AR_OP_AGGREGATE) {
            /* Process child nodes before aggregating. */
            int i = 0;
            for(; i < root->op.child_count; i++) {
                AR_ExpNode *child = root->op.children[i];
                sub_trees[i] = AR_EXP_Evaluate(child);
            }

            /* Aggregate. */
            AggCtx *agg = root->op.agg_func;
            agg->Step(agg, sub_trees, i+1);
        } else {
            /* Keep searching for aggregation nodes. */
            for(int i = 0; i < root->op.child_count; i++) {
                AR_ExpNode *child = root->op.children[i];
                AR_EXP_Aggregate(child);
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

AR_ExpNode* AR_EXP_NewConstOperandNode(SIValue constant) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->operand.type = AR_EXP_CONSTANT;
    node->operand.constant = constant;
    return node;
}

AR_ExpNode* AR_EXP_NewVariableOperandNode(GraphEntity **entity, const char *entity_prop, const char *entity_alias) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity = entity;
    node->operand.variadic.entity_prop = strdup(entity_prop);
    node->operand.variadic.entity_alias = strdup(entity_alias);
    return node;
}

AR_ExpNode* AR_EXP_NewOpNode(char *func_name, int child_count) {
    AR_ExpNode *node = calloc(1, sizeof(AR_ExpNode));
    node->type = AR_EXP_OP;    
    node->op.func_name = strdup(func_name);
    node->op.child_count = child_count;
    node->op.children = (AR_ExpNode **)malloc(child_count * sizeof(AR_ExpNode*));

    /* Determin function type. */
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
        *str = calloc(*str_size, sizeof(char));
    }

    if((*str_size - strlen(*str)) < 64) {
        *str_size += 128;
        *str = realloc(*str, sizeof(char) * *str_size);
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
                *str = realloc(*str, sizeof(char) * *str_size);
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
                    *str = realloc(*str, sizeof(char) * *str_size);
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
            *bytes_written += sprintf((*str + *bytes_written),
                                      "%s.%s",
                                      root->operand.variadic.entity_alias,
                                      root->operand.variadic.entity_prop);
        }
    }
}

void AR_EXP_ToString(const AR_ExpNode *root, char **str) {
    size_t str_size = 0;
    size_t bytes_written = 0;
    *str = NULL;
    _AR_EXP_ToString(root, str, &str_size, &bytes_written);
}

AR_ExpNode* AR_EXP_BuildFromAST(const AST_ArithmeticExpressionNode *exp, const Graph *g) {
    AR_ExpNode *root;

    if(exp->type == AST_AR_EXP_OP) {
        root = AR_EXP_NewOpNode(exp->op.function, Vector_Size(exp->op.args));
        /* Process operands. */
        for(int i = 0; i < root->op.child_count; i++) {
            AST_ArithmeticExpressionNode *child;
            Vector_Get(exp->op.args, i, &child);
            root->op.children[i] = AR_EXP_BuildFromAST(child, g);
        }
    } else {
        if(exp->operand.type == AST_AR_EXP_CONSTANT) {
            root = AR_EXP_NewConstOperandNode(exp->operand.constant);
        } else {
            GraphEntity *e = Graph_GetEntityByAlias(g, exp->operand.variadic.alias);
            GraphEntity **entity = Graph_GetEntityRef(g, e);
            root = AR_EXP_NewVariableOperandNode(entity,
                                                 exp->operand.variadic.property,
                                                 exp->operand.variadic.alias);
        }
    }

    return root;
}

void AR_EXP_Free(AR_ExpNode *root) {
    if(root->type == AR_EXP_OP) {
        for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
            AR_EXP_Free(root->op.children[child_idx]);
        }
        free(root->op.children);
    }
    free(root);
}

/* Mathematical functions - numeric */

SIValue AR_ADD(SIValue *argv, int argc) {
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
        result.doubleval += argv[i].doubleval;
    }
    return result;
}

SIValue AR_SUB(SIValue *argv, int argc) {
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
        result.doubleval -= argv[i].doubleval;
    }
    return result;
}

SIValue AR_MUL(SIValue *argv, int argc) {
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
        /* Assuming every argv is of type double. */
        result.doubleval *= argv[i].doubleval;
    }
    return result;
}

SIValue AR_DIV(SIValue *argv, int argc) {
    SIValue result;
    result = argv[0];
    for(int i = 1; i < argc; i++) {
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
    assert(argv[0].type == T_STRING);
    assert(argv[1].type == T_DOUBLE);
    
    char* left_str = strdup(argv[0].stringval.str);
    size_t str_len = argv[0].stringval.len;
    size_t left = (size_t)argv[1].doubleval;

    /* Boundry check. */
    if(left < str_len) {
        left_str[left] = 0;
    }

    return SI_StringValC(left_str);
}

SIValue AR_LTRIM(SIValue *argv, int argc) {
    assert(argc == 1 && argv[0].type == T_STRING);
    
    char *str = argv[0].stringval.str;
    int i = 0;

    while(str[i++] == ' ');
    if(i>0) i--;

    size_t trimmed_len = argv[0].stringval.len - i;
    char *trimmed = malloc(sizeof(char) * trimmed_len);

    memcpy(trimmed, str+i, trimmed_len);
    return SI_StringValC(trimmed);
}

SIValue AR_RIGHT(SIValue *argv, int argc) {
    assert(argc == 2);
    assert(argv[0].type == T_STRING);
    assert(argv[1].type == T_DOUBLE);
    
    char* right_str = strdup(argv[0].stringval.str);
    size_t str_len = argv[0].stringval.len;
    size_t right = (size_t)argv[1].doubleval;

    /* Boundry check. */
    if(right < str_len) {
        /* TODO: free has to be called on original pointer position. */
        right_str += str_len - right;
    }

    return SI_StringValC(right_str);
}

SIValue AR_RTRIM(SIValue *argv, int argc) {
    assert(argc == 1 && argv[0].type == T_STRING);
    
    char *str = argv[0].stringval.str;
    int i = argv[0].stringval.len-1;
    
    while(i >= 0 && str[i] == ' ') {
         i--;
    }
    
    size_t trimmed_len = i+1; /* NULL terminator.*/
    char *trimmed = malloc(sizeof(char) * (trimmed_len));

    memcpy(trimmed, str, trimmed_len);
    return SI_StringValC(trimmed);
}

SIValue AR_REVERSE(SIValue *argv, int argc) {
    assert(argv[0].type == T_STRING);
    char *str = argv[0].stringval.str;
    size_t str_len = argv[0].stringval.len;
    char* reverse = malloc(sizeof(char) * (str_len+1));
    
    int i = str_len-1;
    int j = 0;
    while(i >= 0) { reverse[j++] = str[i--]; }
    reverse[j] = 0;
    return SI_StringValC(reverse);
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
    char *original = argv[0].stringval.str;
    size_t original_len = argv[0].stringval.len;
    uint start = (uint)argv[1].doubleval;
    size_t length;
    char *substring;

    /* Make sure start doesn't overreach. */
    assert(start < original_len);

    if(argc == 2) {
        length = original_len - start;
    } else {
        length = (size_t)argv[2].doubleval;
        
        /* Make sure length does not overreach. */
        if(start + length > original_len) {
            length = original_len - start;
        }
    }

    assert(start >=0 && length >=0);
    
    substring = calloc(length+1, sizeof(char));
    memcpy(substring, original + start, length);
    return SI_StringValC(substring);
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
    char *original = argv[0].stringval.str;
    size_t lower_len = argv[0].stringval.len + 1;
    char *lower = calloc(lower_len, sizeof(char));
    _toLower(original, lower, &lower_len);
    return SI_StringValC(lower);
}

SIValue AR_TOUPPER(SIValue *argv, int argc) {
    assert(argc == 1);

    char *original = argv[0].stringval.str;
    size_t original_len = argv[0].stringval.len;
    char *upper = calloc(original_len+1, sizeof(char));
    
    for(int i = 0; i < original_len; i++) {
        upper[i] = toupper(original[i]);
    }

    return SI_StringValC(upper);
}

SIValue AR_TOSTRING(SIValue *argv, int argc) {
    assert(argc == 1);

    size_t len = 128;
    char *str = calloc(len, sizeof(char));
    SIValue_ToString(argv[0], str, len);
    return SI_StringValC(str);
}

SIValue AR_TRIM(SIValue *argv, int argc) {
    SIValue ltrim = AR_LTRIM(argv, argc);
    SIValue trimmed = AR_RTRIM(&ltrim, 1);
    return trimmed;
}

// SIValue AR_REPLACE(SIValue *argv, int argc) {

// }

// SIValue AR_SPLIT(SIValue *argv, int argc) {

// }

// SIValue AR_CONCAT(SIValue *argv, int argc) {

// }

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
}