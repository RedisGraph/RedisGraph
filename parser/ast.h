#ifndef AST_H
#define AST_H

#include "../rmutil/vector.h"
#include "../value.h"

typedef enum {
  N_PRED,
  N_COND,
} FilterNodeType;

struct filterNode;

typedef struct {
	char* id;
	char* alias;
} ItemNode;

typedef struct {
	char* relationship;
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

typedef struct {
	ItemNode* src;
	LinkNode* relation;
	ItemNode* dest;
} RelationshipNode;

typedef struct {
	RelationshipNode* relationshipNode;
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


MatchNode* NewMatchNode(RelationshipNode* relationshipNode);

ItemNode* NewItemNode(const char* alias, const char* id);
LinkNode* NewLinkNode(const char* relationship);

FilterNode* NewPredicateNode(const char* alias, const char* property, int op, SIValue v);
FilterNode* NewConditionNode(FilterNode *left, int op, FilterNode *right);

WhereNode* NewWhereNode(FilterNode* filters);

ReturnNode* NewReturnNode(Vector* variables);

RelationshipNode* NewRelationshipNode(ItemNode* src, LinkNode* relation, ItemNode* dest);

QueryExpressionNode* NewQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode);

void FreeItemNode(ItemNode* itemNode);
void FreeLinkNode(LinkNode* linkNode);
void FreeMatchNode(MatchNode* matchNode);
void FreeWhereNode(WhereNode* whereNode);
void FreeFilterNode(FilterNode* filterNode);
void FreeReturnNode(ReturnNode* returnNode);
void FreeVariableNode(VariableNode* variableNode);
void FreeRelationshipNode(RelationshipNode* relationshipNode);
void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode);
#endif