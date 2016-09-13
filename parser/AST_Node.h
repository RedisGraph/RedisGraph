#ifndef ASTNODE_H_
#define ASTNODE_H_

#include "../rmutil/vector.h"

typedef enum {EQ, GT, GE, LT, LE} Operator;
typedef enum {NUMERIC, STRING} ValueType;

typedef struct {
	char* id;
	char* alias;
} ItemNode;

typedef struct {
	char* id;
	char* alias;
} LinkNode;

typedef struct {
	union{
		char* strValue;
		int nValue;
	};
	ValueType t;
	Operator op;
	char* alias;
	char* property;	
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
	Vector* filters;
} WhereNode;

typedef struct {
	char* alias;
	char* property;
} VariableNode;

typedef struct {
	Vector* variables;
} ReturnNode;

typedef struct {
	MatchNode* matchNode;
	WhereNode* whereNode;
	ReturnNode* returnNode;
}QueryExpressionNode;


MatchNode* CreateMatchNode(RelationshipNode* relationshipNode);

ItemNode* CreateItemNode(const char* alias, const char* id);
LinkNode* CreateLinkNode(const char* alias, const char* id);

FilterNode* CreateStringFilterNode(const char* alias, const char* property, Operator op, const char* value);
FilterNode* CreateNumericFilterNode(const char* alias, const char* property, Operator op, int value);

WhereNode* CreateWhereNode(Vector* filters);

VariableNode* CreateVariableNode(const char* alias, const char* property);

ReturnNode* CreateReturnNode(Vector* variables);

RelationshipNode* CreateRelationshipNode(ItemNode* src, LinkNode* relation, ItemNode* dest);

QueryExpressionNode* CreateQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode);

void FreeItemNode(ItemNode* itemNode);
void FreeLinkNode(LinkNode* linkNode);
void FreeFilterNode(FilterNode* filterNode);
void FreeMatchNode(MatchNode* matchNode);
void FreeWhereNode(WhereNode* whereNode);
void FreeVariableNode(VariableNode* variableNode);
void FreeReturnNode(ReturnNode* returnNode);
void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode);
#endif