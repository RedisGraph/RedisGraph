#ifndef _CLAUSE_ORDER_H
#define _CLAUSE_ORDER_H

#include "../../rmutil/vector.h"
#include "../ast_common.h"

typedef enum {
	N_VARIABLE,
	N_ALIAS
} AST_ColumnNodeType;

typedef struct {
	char *alias;
	char *property;
	AST_ColumnNodeType type;
} AST_ColumnNode;

typedef enum {
	ORDER_DIR_ASC,
	ORDER_DIR_DESC
} AST_OrderByDirection;

typedef struct {
	Vector *columns;	// Vector of ColumnNodes
	AST_OrderByDirection direction;
} AST_OrderNode;

AST_OrderNode* New_AST_OrderNode(Vector* columns, AST_OrderByDirection direction);
AST_ColumnNode* New_AST_ColumnNode(const char *alias, const char *prop, AST_ColumnNodeType type);
AST_ColumnNode* AST_ColumnNodeFromVariable(const AST_Variable *variable);
AST_ColumnNode* AST_ColumnNodeFromAlias(const char *alias);
void Free_AST_OrderNode(AST_OrderNode *orderNode);
void Free_AST_ColumnNode(AST_ColumnNode *node);

#endif
