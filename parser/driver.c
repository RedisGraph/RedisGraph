#include <stdlib.h>
#include "lexglobal.h"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string( const char * );
extern int yylex( void );
extern QueryExpressionNode* queryExpressionNode;
// extern void yy_delete_buffer( YY_BUFFER_STATE );

int main(int argc, char const *argv[]) {
  void* pParser = ParseAlloc(malloc);
  int yv;

  yy_scan_string("MATCH (me:Roi)-[]->() where me.lastname = lipman RETURN me.age");

  while( (yv=yylex()) != 0) {
    Parse(pParser, yv, yylval.str);
  }

  Parse (pParser, 0, 0);  
  ParseFree(pParser, free);

  // Validate AST.
  assert(queryExpressionNode != NULL);
  assert(queryExpressionNode->matchNode != NULL);
  assert(queryExpressionNode->whereNode != NULL);
  assert(queryExpressionNode->returnNode != NULL);

  // Validate match clause
  MatchNode* matchNode = queryExpressionNode->matchNode;
  assert(matchNode->relationshipNode != NULL);
  
  RelationshipNode* relationshipNode = matchNode->relationshipNode;
  assert(relationshipNode->src != NULL);
  assert(relationshipNode->relation != NULL);
  assert(relationshipNode->dest != NULL);

  ItemNode* src = relationshipNode->src;
  printf("src->id: %s src->alias: %s\n", src->id, src->alias);
  assert(strcmp(src->id, "Roi") == 0);
  assert(strcmp(src->alias, "me") == 0);

  ItemNode* dest = relationshipNode->dest;
  assert(strcmp(dest->id, "") == 0);
  assert(strcmp(dest->alias, "") == 0);

  LinkNode* link = relationshipNode->relation;
  assert(strcmp(link->id, "") == 0);
  assert(strcmp(link->alias, "") == 0);

  // Validate where clause
  WhereNode* whereNode = queryExpressionNode->whereNode;
  assert(Vector_Size(whereNode->filters) == 1);

  FilterNode *filterNode;
  Vector_Get(whereNode->filters, 0, &filterNode);
  assert(strcmp(filterNode->alias, "me") == 0);
  assert(strcmp(filterNode->property, "lastname") == 0);
  assert(filterNode->op == EQ);
  // assert(filterNode->t == STRING);
  assert(strcmp(filterNode->strValue, "lipman") == 0);

  // Validate return clause RETURN me.age
  ReturnNode* returnNode = queryExpressionNode->returnNode;
  assert(Vector_Size(returnNode->variables) == 1);

  VariableNode *variableNode;
  Vector_Get(returnNode->variables, 0, &variableNode);

  assert(strcmp(variableNode->alias, "me") == 0);
  assert(strcmp(variableNode->property, "age") == 0);

  FreeQueryExpressionNode(queryExpressionNode);
  
  printf("PASS\n");
  return 0;
}