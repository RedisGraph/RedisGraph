#ifndef AST_H
#define AST_H

#include "../rmutil/vector.h"
#include "../value.h"

typedef enum {
  N_PRED,
  N_COND,
} AST_FilterNodeType;

typedef enum {
	N_LEFT_TO_RIGHT,
	N_RIGHT_TO_LEFT,
	N_DIR_UNKNOWN,
} AST_LinkDirection;

typedef enum {
	N_ENTITY,
	N_LINK,
} AST_GraphEntityType;

typedef enum {
	N_CONSTANT,
	N_VARYING,
} AST_CompareValueType;

typedef enum {
	ORDER_DIR_ASC,
	ORDER_DIR_DESC
} AST_OrderByDirection;

typedef enum {
	N_VARIABLE,
	N_ALIAS
} AST_ColumnNodeType;

struct filterNode;

typedef struct {
	char *alias;
	char *label;
	Vector *properties;
	AST_GraphEntityType t;
} AST_GraphEntity;

typedef AST_GraphEntity AST_NodeEntity;

typedef struct {
	AST_GraphEntity ge;
	AST_LinkDirection direction;
} AST_LinkEntity;

typedef struct {
	union {
		SIValue constVal;
		struct {
			char *alias;
			char *property;
		} nodeVal;
	};
	AST_CompareValueType t; // Comapred value type, constant/node
	char *alias;		// Node alias
	char *property; 	// Node property
	int op;				// Type of comparison
} AST_PredicateNode;

typedef struct conditionNode {
  struct filterNode *left;
  struct filterNode *right;
  int op;
} AST_ConditionNode;

typedef struct filterNode {
  union {
    AST_PredicateNode pn;
    AST_ConditionNode cn;
  };
  AST_FilterNodeType t;
} AST_FilterNode;

/* Arithmetic expression structs */

/* ArExpNodeType lists the type of nodes within
 * an arithmeric expression tree. */
typedef enum {
    AST_AR_EXP_OP,
    AST_AR_EXP_OPERAND,
} AST_ArithmeticExpression_NodeType;

typedef enum {
    AST_AR_EXP_CONSTANT,
    AST_AR_EXP_VARIADIC,
} AST_ArithmeticExpression_OperandNodeType;

typedef struct {
    char *function; /* Name of operation. */
	Vector *args;	/* Vector of AST_ArithmeticExpressionNode pointers. */
} AST_ArithmeticExpressionOP;

/* OperandNode represents either a constant numeric value, 
 * or a graph entity property. */
typedef struct {
    union {
        SIValue constant;
        struct {
			char *alias;
			char *property;
		} variadic;
    };
    AST_ArithmeticExpression_OperandNodeType type;
} AST_ArithmeticExpressionOperand;

typedef struct {
	union {
		AST_ArithmeticExpressionOperand operand;
        AST_ArithmeticExpressionOP op;
    };
    AST_ArithmeticExpression_NodeType type;
} AST_ArithmeticExpressionNode;

typedef struct {
	Vector *graphEntities;
} AST_MatchNode;

typedef struct {	
	Vector *graphEntities; /* Vector of Vectors of AST_GraphEntity pointers. */
} AST_CreateNode;

typedef struct {	
	Vector *graphEntities; /* Vector of Vectors of char pointers. */
} AST_DeleteNode;

typedef struct {
	AST_FilterNode *filters;
} AST_WhereNode;

typedef struct {
	Vector *returnElements; // Vector of ReturnElementNode pointers
	int distinct;
} AST_ReturnNode;

typedef struct {
	int limit;
} AST_LimitNode;

typedef struct {
	char *alias;
	char *property;
} AST_Variable;

typedef struct {
	char *alias; 		// Alias given to this return element (using the AS keyword)
	AST_ArithmeticExpressionNode *exp;
} AST_ReturnElementNode;

typedef struct {
	Vector *columns;	// Vector of ColumnNodes
	AST_OrderByDirection direction;
} AST_OrderNode;

typedef struct {
	char *alias;
	char *property;
	AST_ColumnNodeType type;
} AST_ColumnNode;

typedef struct {
	AST_MatchNode *matchNode;
	AST_CreateNode *createNode;
	AST_DeleteNode *deleteNode;
	AST_WhereNode *whereNode;
	AST_ReturnNode *returnNode;
	AST_OrderNode *orderNode;
	AST_LimitNode *limitNode;
} AST_QueryExpressionNode;

AST_NodeEntity* New_AST_NodeEntity(char *alias, char *label, Vector *properties);
AST_LinkEntity* New_AST_LinkEntity(char *alias, char *relationship, Vector *properties, AST_LinkDirection dir);
AST_MatchNode* New_AST_MatchNode(Vector *elements);
AST_CreateNode* New_AST_CreateNode(Vector *elements);
AST_DeleteNode* New_AST_DeleteNode(Vector *elements);
AST_FilterNode* New_AST_ConstantPredicateNode(const char *alias, const char *property, int op, SIValue value);
AST_FilterNode* New_AST_VaryingPredicateNode(const char *lAlias, const char *lProperty, int op, const char *rAlias, const char *rProperty);
AST_FilterNode* New_AST_ConditionNode(AST_FilterNode *left, int op, AST_FilterNode *right);
/* Arithmetic expression */
AST_ArithmeticExpressionNode* New_AST_AR_EXP_VariableOperandNode(char* alias, char *property);
AST_ArithmeticExpressionNode* NEW_AST_AR_EXP_ConstOperandNode(SIValue constant);
AST_ArithmeticExpressionNode* NEW_AST_AR_EXP_OpNode(char *func, Vector *args);

AST_WhereNode* New_AST_WhereNode(AST_FilterNode *filters);
AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ArithmeticExpressionNode *exp, const char *alias);
AST_ReturnNode* New_AST_ReturnNode(Vector* returnElements, int distinct);
AST_OrderNode* New_AST_OrderNode(Vector* columns, AST_OrderByDirection direction);
AST_ColumnNode* New_AST_ColumnNode(const char *alias, const char *prop, AST_ColumnNodeType type);
AST_ColumnNode* AST_ColumnNodeFromVariable(const AST_Variable *variable);
AST_ColumnNode* AST_ColumnNodeFromAlias(const char *alias);
AST_Variable* New_AST_Variable(const char *alias, const char *property);
AST_LimitNode* New_AST_LimitNode(int limit);
AST_QueryExpressionNode* New_AST_QueryExpressionNode(AST_MatchNode *matchNode, AST_WhereNode *whereNode, AST_CreateNode *createNode, AST_DeleteNode *deleteNode, AST_ReturnNode *returnNode, AST_OrderNode *orderNode, AST_LimitNode *limitNode);

void Free_AST_Variable(AST_Variable *v);
void Free_AST_ColumnNode(AST_ColumnNode *node);
void Free_AST_MatchNode(AST_MatchNode *matchNode);
void Free_AST_CreateNode(AST_CreateNode *createNode);
void Free_AST_DeleteNode(AST_DeleteNode *deleteNode);
void Free_AST_WhereNode(AST_WhereNode *whereNode);
void Free_AST_FilterNode(AST_FilterNode *filterNode);
void Free_AST_ReturnNode(AST_ReturnNode *returnNode);
void Free_AST_OrderNode(AST_OrderNode *orderNode);
void Free_AST_LimitNode(AST_LimitNode *limitNode);
void Free_AST_ReturnElementNode(AST_ReturnElementNode *returnElementNode);
void Free_AST_ArithmeticExpressionNode(AST_ArithmeticExpressionNode *arExpNode);
void Free_AST_GraphEntity(AST_GraphEntity *entity);
void Free_AST_QueryExpressionNode(AST_QueryExpressionNode *queryExpressionNode);
#endif