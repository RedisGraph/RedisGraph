#include <stdlib.h>
#include "lexglobal.h"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string( const char * );
extern int yylex( void );
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

  return 0;
}