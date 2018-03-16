#ifndef __PARSER_COMMON_H__
#define __PARSER_COMMON_H__
#include <stdlib.h>
#include "ast.h"

AST_QueryExpressionNode *Query_Parse(const char *q, size_t len, char **err);
#endif