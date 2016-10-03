#ifndef __PARSER_COMMON_H__
#define __PARSER_COMMON_H__
#include <stdlib.h>
#include "ast.h"

QueryExpressionNode *ParseQuery(const char *c, size_t len);
#endif