#ifndef AST_H
#define AST_H

#include "../rmutil/vector.h"
#include "../value.h"

typedef enum {
  N_PRED,
  N_COND,
} FilterNodeType;

typedef enum {
	N_LEFT_TO_RIGHT,
	N_RIGHT_TO_LEFT,
} LinkDirection;

typedef enum {
	N_ENTITY,
	N_LINK,
} ChainElementType;

typedef enum {
	N_CONSTANT,
	N_VARYING,
} CompareValueType;

typedef enum {
	N_PROP,
	N_AGG_FUNC
} ReturnElementType;

struct filterNode;

typedef struct {
	char* id;
	char* alias;
} EntityNode;

typedef struct {
	char* relationship;
	LinkDirection direction;

} LinkNode;

typedef struct {
	union {
		SIValue constVal;
		struct {
			char* alias;
			char* property;
		} nodeVal;
	};
	CompareValueType t; // Comapred value type, constant/node
	char* alias;		// Node alias
	char* property; 	// Node property
	int op;				// Type of comparison
} PredicateNode;

typedef struct conditionNode {
  struct filterNode *left;
  struct filterNode *right;
  int op;
} ConditionNode;

typedef struct filterNode {
  union {
    PredicateNode pn;
    ConditionNode cn;
  };
  FilterNodeType t;
} FilterNode;

typedef struct {
	union {
		EntityNode e;
		LinkNode l;
	};
	ChainElementType t;
} ChainElement;

typedef struct {
	Vector* chainElements;
} MatchNode;

typedef struct {
	FilterNode* filters;
} WhereNode;

typedef struct {
	Vector* returnElements;
} ReturnNode;

typedef struct {
	int limit;
} LimitNode;

// TODO: consider using union
typedef struct {
	char* alias;
	char* property;
	char* func;
	ReturnElementType type;
} ReturnElementNode;

typedef struct {
	char* alias;
	char* property;
} Variable;

typedef struct {
	Vector* variables;
} OrderNode;

typedef struct {
	MatchNode* matchNode;
	WhereNode* whereNode;
	ReturnNode* returnNode;
	OrderNode* orderNode;
	LimitNode* limitNode;
} QueryExpressionNode;

ChainElement* NewChainEntity(char* alias, char* id);
ChainElement* NewChainLink(char* relationship, LinkDirection dir);

MatchNode* NewMatchNode(Vector* elements);

FilterNode* NewConstantPredicateNode(const char* alias, const char* property, int op, SIValue value);
FilterNode* NewVaryingPredicateNode(const char* lAlias, const char* lProperty, int op, const char* rAlias, const char* rProperty);
FilterNode* NewConditionNode(FilterNode *left, int op, FilterNode *right);

WhereNode* NewWhereNode(FilterNode* filters);

ReturnElementNode* NewReturnElementNode(ReturnElementType type, const char* alias, const char* property, const char* aggFunc);

ReturnNode* NewReturnNode(Vector* returnElements);

OrderNode* NewOrderNode(Vector* variables);

Variable* NewVariable(const char* alias, const char* property);

LimitNode* NewLimitNode(int limit);

QueryExpressionNode* NewQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode, OrderNode* orderNode, LimitNode* limitNode);

void FreeVariable(Variable* v);
void FreeMatchNode(MatchNode* matchNode);
void FreeWhereNode(WhereNode* whereNode);
void FreeFilterNode(FilterNode* filterNode);
void FreeReturnNode(ReturnNode* returnNode);
void FreeOrderNode(OrderNode* orderNode);
void FreeLimitNode(LimitNode* limitNode);
void FreeReturnElementNode(ReturnElementNode* returnElementNode);
void FreeChainElement(ChainElement* chainElement);
void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode);
#endif