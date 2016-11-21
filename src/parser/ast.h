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
	char* alias;
	char* property;
	int op;
	SIValue val;
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

// typedef struct {
// 	ItemNode* src;
// 	LinkNode* relation;
// 	ItemNode* dest;
// } RelationshipNode;

typedef struct {
	union {
		EntityNode e;
		LinkNode l;
	};
	ChainElementType t;
} ChainElement;

typedef struct {
	// RelationshipNode* relationshipNode;
	Vector* chainElements;
} MatchNode;

typedef struct {
	FilterNode* filters;
} WhereNode;

typedef struct {
	Vector* variables;
} ReturnNode;

typedef struct {
	char* alias;
	char* property;
} VariableNode;

typedef struct {
	MatchNode* matchNode;
	WhereNode* whereNode;
	ReturnNode* returnNode;
}QueryExpressionNode;


ChainElement* NewChainEntity(char* alias, char* id);
ChainElement* NewChainLink(char* relationship, LinkDirection dir);

MatchNode* NewMatchNode(Vector* elements);


FilterNode* NewPredicateNode(const char* alias, const char* property, int op, SIValue v);
FilterNode* NewConditionNode(FilterNode *left, int op, FilterNode *right);

WhereNode* NewWhereNode(FilterNode* filters);

VariableNode* NewVariableNode(const char* alias, const char* property);

ReturnNode* NewReturnNode(Vector* variables);

QueryExpressionNode* NewQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode);

void FreeMatchNode(MatchNode* matchNode);
void FreeWhereNode(WhereNode* whereNode);
void FreeFilterNode(FilterNode* filterNode);
void FreeReturnNode(ReturnNode* returnNode);
void FreeVariableNode(VariableNode* variableNode);
void FreeChainElement(ChainElement* chainElement);
void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode);
#endif