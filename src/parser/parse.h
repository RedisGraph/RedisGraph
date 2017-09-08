#ifndef __QUERY_PARSER_PARSE_H__
#define __QUERY_PARSER_PARSE_H__

#include "ast.h"
//#include "../rmutil/alloc.h"

typedef struct {
    AST_QueryExpressionNode *root;
    int ok;
    char *errorMsg;
} parseCtx;

#endif // !__QUERY_PARSER_PARSE_H__