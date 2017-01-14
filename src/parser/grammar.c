/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/
#line 11 "grammar.y"

	#include <stdlib.h>
	#include <stdio.h>
	#include <assert.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"

	void yyerror(char *s);
#line 38 "grammar.c"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    ParseTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 50
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  MatchNode* yy2;
  LimitNode* yy18;
  SIValue yy22;
  WhereNode* yy26;
  FilterNode* yy31;
  int yy32;
  QueryExpressionNode* yy35;
  Vector* yy64;
  Variable* yy65;
  ReturnElementNode* yy71;
  ChainElement* yy76;
  OrderNode* yy79;
  ReturnNode* yy93;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  QueryExpressionNode **root ;
#define ParseARG_PDECL , QueryExpressionNode **root 
#define ParseARG_FETCH  QueryExpressionNode **root  = yypParser->root 
#define ParseARG_STORE yypParser->root  = root 
#define YYNSTATE             51
#define YYNRULE              44
#define YY_MAX_SHIFT         50
#define YY_MIN_SHIFTREDUCE   85
#define YY_MAX_SHIFTREDUCE   128
#define YY_MIN_REDUCE        129
#define YY_MAX_REDUCE        172
#define YY_ERROR_ACTION      173
#define YY_ACCEPT_ACTION     174
#define YY_NO_ACTION         175
/************* End control #defines *******************************************/

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if:
**    (1)  The yy_shift_ofst[S]+X value is out of range, or
**    (2)  yy_lookahead[yy_shift_ofst[S]+X] is not equal to X, or
**    (3)  yy_shift_ofst[S] equal YY_SHIFT_USE_DFLT.
** (Implementation note: YY_SHIFT_USE_DFLT is chosen so that
** YY_SHIFT_USE_DFLT+X will be out of range for all possible lookaheads X.
** Hence only tests (1) and (2) need to be evaluated.)
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (101)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   105,  106,  109,  107,  108,  174,   50,   18,   40,   44,
 /*    10 */   118,  119,  120,  117,  119,  120,  122,  110,   47,  111,
 /*    20 */   113,  114,  115,   23,   35,   93,    8,   10,    7,   88,
 /*    30 */    33,    9,   42,   27,   29,    3,   31,  102,   28,   37,
 /*    40 */    92,   32,    8,   10,   13,    6,   25,   19,  101,   46,
 /*    50 */    12,  129,   38,   20,   45,  103,   24,   89,   49,   48,
 /*    60 */    97,   86,   43,   11,   26,  121,   15,    1,   16,   96,
 /*    70 */    95,   17,   21,   94,   30,   22,   91,   34,   90,   36,
 /*    80 */    10,  100,   39,    5,   41,  123,  124,   45,    2,  131,
 /*    90 */   131,  131,  131,  131,  131,  131,   14,  128,  131,  131,
 /*   100 */     4,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,    4,    5,    6,    7,   31,   32,   33,   10,   44,
 /*    10 */    45,   46,   47,   45,   46,   47,   46,   20,   48,   21,
 /*    20 */    22,   23,   24,   10,   11,   12,    1,    2,   38,   39,
 /*    30 */    13,    9,   10,   10,   17,    8,   10,   12,   15,   11,
 /*    40 */    12,   15,    1,    2,    9,   42,   10,    9,   43,   10,
 /*    50 */    40,    0,   41,   41,   19,   41,   41,   39,   29,   27,
 /*    60 */    13,   37,   46,   18,   15,   46,   36,   25,   35,   13,
 /*    70 */    16,   34,   14,   16,   15,   14,   12,   10,   12,   10,
 /*    80 */     2,   10,   19,   10,   19,   10,   12,   19,   26,   49,
 /*    90 */    49,   49,   49,   49,   49,   49,   26,   21,   49,   49,
 /*   100 */    28,
};
#define YY_SHIFT_USE_DFLT (101)
#define YY_SHIFT_COUNT    (50)
#define YY_SHIFT_MIN      (-3)
#define YY_SHIFT_MAX      (78)
static const short yy_shift_ofst[] = {
 /*     0 */    27,   36,   36,   38,   39,   -3,   -2,   17,   22,   22,
 /*    10 */    22,   22,   38,   39,   39,   29,   32,   42,   45,   13,
 /*    20 */    25,   23,   26,   28,   41,   35,   47,   49,   56,   58,
 /*    30 */    54,   59,   57,   61,   64,   67,   66,   69,   78,   71,
 /*    40 */    63,   73,   65,   74,   62,   75,   68,   70,   72,   76,
 /*    50 */    51,
};
#define YY_REDUCE_USE_DFLT (-36)
#define YY_REDUCE_COUNT (18)
#define YY_REDUCE_MIN   (-35)
#define YY_REDUCE_MAX   (37)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -26,  -35,  -32,  -10,  -30,    3,    5,   10,   11,   12,
 /*    10 */    14,   15,   18,   16,   19,   24,   30,   33,   37,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   173,  173,  173,  173,  173,  173,  173,  131,  173,  173,
 /*    10 */   173,  173,  173,  173,  173,  171,  169,  173,  142,  173,
 /*    20 */   173,  173,  173,  173,  143,  173,  173,  173,  173,  173,
 /*    30 */   173,  173,  173,  173,  173,  173,  173,  173,  148,  173,
 /*    40 */   156,  173,  173,  173,  160,  173,  173,  170,  173,  173,
 /*    50 */   173,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  yyStackEntry *yytos;          /* Pointer to top element of the stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyhwm;                    /* High-water mark of the stack */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
  yyStackEntry yystk0;          /* First stack entry */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "OR",            "AND",           "EQ",          
  "GT",            "GE",            "LT",            "LE",          
  "MATCH",         "LEFT_PARENTHESIS",  "STRING",        "COLON",       
  "RIGHT_PARENTHESIS",  "DASH",          "LEFT_BRACKET",  "RIGHT_BRACKET",
  "RIGHT_ARROW",   "LEFT_ARROW",    "WHERE",         "DOT",         
  "NE",            "INTEGER",       "FLOAT",         "TRUE",        
  "FALSE",         "RETURN",        "COMMA",         "ORDER",       
  "BY",            "LIMIT",         "error",         "query",       
  "expr",          "matchClause",   "whereClause",   "returnClause",
  "orderClause",   "limitClause",   "chain",         "node",        
  "link",          "cond",          "op",            "value",       
  "returnElements",  "returnElement",  "variable",      "aggFunc",     
  "variableList",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause returnClause orderClause limitClause",
 /*   2 */ "matchClause ::= MATCH chain",
 /*   3 */ "chain ::= node",
 /*   4 */ "chain ::= chain link node",
 /*   5 */ "node ::= LEFT_PARENTHESIS STRING COLON STRING RIGHT_PARENTHESIS",
 /*   6 */ "node ::= LEFT_PARENTHESIS COLON STRING RIGHT_PARENTHESIS",
 /*   7 */ "node ::= LEFT_PARENTHESIS STRING RIGHT_PARENTHESIS",
 /*   8 */ "node ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS",
 /*   9 */ "link ::= DASH LEFT_BRACKET RIGHT_BRACKET RIGHT_ARROW",
 /*  10 */ "link ::= DASH LEFT_BRACKET STRING RIGHT_BRACKET RIGHT_ARROW",
 /*  11 */ "link ::= LEFT_ARROW LEFT_BRACKET RIGHT_BRACKET DASH",
 /*  12 */ "link ::= LEFT_ARROW LEFT_BRACKET STRING RIGHT_BRACKET DASH",
 /*  13 */ "whereClause ::=",
 /*  14 */ "whereClause ::= WHERE cond",
 /*  15 */ "cond ::= STRING DOT STRING op STRING DOT STRING",
 /*  16 */ "cond ::= STRING DOT STRING op value",
 /*  17 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  18 */ "cond ::= cond AND cond",
 /*  19 */ "cond ::= cond OR cond",
 /*  20 */ "op ::= EQ",
 /*  21 */ "op ::= GT",
 /*  22 */ "op ::= LT",
 /*  23 */ "op ::= LE",
 /*  24 */ "op ::= GE",
 /*  25 */ "op ::= NE",
 /*  26 */ "value ::= INTEGER",
 /*  27 */ "value ::= STRING",
 /*  28 */ "value ::= FLOAT",
 /*  29 */ "value ::= TRUE",
 /*  30 */ "value ::= FALSE",
 /*  31 */ "returnClause ::= RETURN returnElements",
 /*  32 */ "returnElements ::= returnElements COMMA returnElement",
 /*  33 */ "returnElements ::= returnElement",
 /*  34 */ "returnElement ::= variable",
 /*  35 */ "returnElement ::= aggFunc",
 /*  36 */ "variableList ::= variableList COMMA variable",
 /*  37 */ "variableList ::= variable",
 /*  38 */ "variable ::= STRING DOT STRING",
 /*  39 */ "aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS",
 /*  40 */ "orderClause ::=",
 /*  41 */ "orderClause ::= ORDER BY variableList",
 /*  42 */ "limitClause ::=",
 /*  43 */ "limitClause ::= LIMIT INTEGER",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.  Return the number
** of errors.  Return 0 on success.
*/
static int yyGrowStack(yyParser *p){
  int newSize;
  int idx;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  idx = p->yytos ? (int)(p->yytos - p->yystack) : 0;
  if( p->yystack==&p->yystk0 ){
    pNew = malloc(newSize*sizeof(pNew[0]));
    if( pNew ) pNew[0] = p->yystk0;
  }else{
    pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  }
  if( pNew ){
    p->yystack = pNew;
    p->yytos = &p->yystack[idx];
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows from %d to %d entries.\n",
              yyTracePrompt, p->yystksz, newSize);
    }
#endif
    p->yystksz = newSize;
  }
  return pNew==0; 
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to ParseAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ){
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyhwm = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yytos = NULL;
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    if( yyGrowStack(pParser) ){
      pParser->yystack = &pParser->yystk0;
      pParser->yystksz = 1;
    }
#endif
#ifndef YYNOERRORRECOVERY
    pParser->yyerrcnt = -1;
#endif
    pParser->yytos = pParser->yystack;
    pParser->yystack[0].stateno = 0;
    pParser->yystack[0].major = 0;
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
    case 41: /* cond */
{
#line 98 "grammar.y"
 FreeFilterNode((yypminor->yy31)); 
#line 541 "grammar.c"
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yytos!=0 );
  assert( pParser->yytos > pParser->yystack );
  yytos = pParser->yytos--;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static unsigned int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yytos->stateno;
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
        iLookAhead = iFallback;
        continue;
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD && iLookAhead>0
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead],
               yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser){
   ParseARG_FETCH;
   yypParser->yytos--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  ParseTOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yytos++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
    yypParser->yyhwm++;
    assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack) );
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH] ){
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  if( yyNewState > YY_MAX_SHIFT ){
    yyNewState += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
  }
  yytos = yypParser->yytos;
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 31, 1 },
  { 32, 5 },
  { 33, 2 },
  { 38, 1 },
  { 38, 3 },
  { 39, 5 },
  { 39, 4 },
  { 39, 3 },
  { 39, 2 },
  { 40, 4 },
  { 40, 5 },
  { 40, 4 },
  { 40, 5 },
  { 34, 0 },
  { 34, 2 },
  { 41, 7 },
  { 41, 5 },
  { 41, 3 },
  { 41, 3 },
  { 41, 3 },
  { 42, 1 },
  { 42, 1 },
  { 42, 1 },
  { 42, 1 },
  { 42, 1 },
  { 42, 1 },
  { 43, 1 },
  { 43, 1 },
  { 43, 1 },
  { 43, 1 },
  { 43, 1 },
  { 35, 2 },
  { 44, 3 },
  { 44, 1 },
  { 45, 1 },
  { 45, 1 },
  { 48, 3 },
  { 48, 1 },
  { 46, 3 },
  { 47, 4 },
  { 36, 0 },
  { 36, 3 },
  { 37, 0 },
  { 37, 2 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno        /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
      yypParser->yyhwm++;
      assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack));
    }
#endif
#if YYSTACKDEPTH>0 
    if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH-1] ){
      yyStackOverflow(yypParser);
      return;
    }
#else
    if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz-1] ){
      if( yyGrowStack(yypParser) ){
        yyStackOverflow(yypParser);
        return;
      }
      yymsp = yypParser->yytos;
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* query ::= expr */
#line 24 "grammar.y"
{ *root = yymsp[0].minor.yy35; }
#line 893 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause returnClause orderClause limitClause */
#line 29 "grammar.y"
{
	yylhsminor.yy35 = NewQueryExpressionNode(yymsp[-4].minor.yy2, yymsp[-3].minor.yy26, yymsp[-2].minor.yy93, yymsp[-1].minor.yy79, yymsp[0].minor.yy18);
}
#line 900 "grammar.c"
  yymsp[-4].minor.yy35 = yylhsminor.yy35;
        break;
      case 2: /* matchClause ::= MATCH chain */
#line 36 "grammar.y"
{
	yymsp[-1].minor.yy2 = NewMatchNode(yymsp[0].minor.yy64);
}
#line 908 "grammar.c"
        break;
      case 3: /* chain ::= node */
#line 43 "grammar.y"
{
	yylhsminor.yy64 = NewVector(ChainElement*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy76);
}
#line 916 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 4: /* chain ::= chain link node */
#line 48 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[-1].minor.yy76);
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy76);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 926 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 5: /* node ::= LEFT_PARENTHESIS STRING COLON STRING RIGHT_PARENTHESIS */
#line 57 "grammar.y"
{
	yymsp[-4].minor.yy76 = NewChainEntity(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy0.strval);
}
#line 934 "grammar.c"
        break;
      case 6: /* node ::= LEFT_PARENTHESIS COLON STRING RIGHT_PARENTHESIS */
#line 60 "grammar.y"
{
	yymsp[-3].minor.yy76 = NewChainEntity("", yymsp[-1].minor.yy0.strval);
}
#line 941 "grammar.c"
        break;
      case 7: /* node ::= LEFT_PARENTHESIS STRING RIGHT_PARENTHESIS */
#line 63 "grammar.y"
{
	yymsp[-2].minor.yy76 = NewChainEntity(yymsp[-1].minor.yy0.strval, "");
}
#line 948 "grammar.c"
        break;
      case 8: /* node ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS */
#line 66 "grammar.y"
{
	yymsp[-1].minor.yy76 = NewChainEntity("", "");
}
#line 955 "grammar.c"
        break;
      case 9: /* link ::= DASH LEFT_BRACKET RIGHT_BRACKET RIGHT_ARROW */
#line 73 "grammar.y"
{ 
	yymsp[-3].minor.yy76 =  NewChainLink("", N_LEFT_TO_RIGHT); 
}
#line 962 "grammar.c"
        break;
      case 10: /* link ::= DASH LEFT_BRACKET STRING RIGHT_BRACKET RIGHT_ARROW */
#line 76 "grammar.y"
{ 
	yymsp[-4].minor.yy76 = NewChainLink(yymsp[-2].minor.yy0.strval, N_LEFT_TO_RIGHT);
}
#line 969 "grammar.c"
        break;
      case 11: /* link ::= LEFT_ARROW LEFT_BRACKET RIGHT_BRACKET DASH */
#line 79 "grammar.y"
{ 
	yymsp[-3].minor.yy76 = NewChainLink("", N_RIGHT_TO_LEFT); 
}
#line 976 "grammar.c"
        break;
      case 12: /* link ::= LEFT_ARROW LEFT_BRACKET STRING RIGHT_BRACKET DASH */
#line 82 "grammar.y"
{ 
	yymsp[-4].minor.yy76 = NewChainLink(yymsp[-2].minor.yy0.strval, N_RIGHT_TO_LEFT);
}
#line 983 "grammar.c"
        break;
      case 13: /* whereClause ::= */
#line 88 "grammar.y"
{ 
	yymsp[1].minor.yy26 = NULL;
}
#line 990 "grammar.c"
        break;
      case 14: /* whereClause ::= WHERE cond */
#line 91 "grammar.y"
{
	//printf("where clause\n");
	yymsp[-1].minor.yy26 = NewWhereNode(yymsp[0].minor.yy31);
}
#line 998 "grammar.c"
        break;
      case 15: /* cond ::= STRING DOT STRING op STRING DOT STRING */
#line 100 "grammar.y"
{ yylhsminor.yy31 = NewVaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy32, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1003 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 16: /* cond ::= STRING DOT STRING op value */
#line 101 "grammar.y"
{ yylhsminor.yy31 = NewConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy32, yymsp[0].minor.yy22); }
#line 1009 "grammar.c"
  yymsp[-4].minor.yy31 = yylhsminor.yy31;
        break;
      case 17: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 102 "grammar.y"
{ yymsp[-2].minor.yy31 = yymsp[-1].minor.yy31; }
#line 1015 "grammar.c"
        break;
      case 18: /* cond ::= cond AND cond */
#line 103 "grammar.y"
{ yylhsminor.yy31 = NewConditionNode(yymsp[-2].minor.yy31, AND, yymsp[0].minor.yy31); }
#line 1020 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 19: /* cond ::= cond OR cond */
#line 104 "grammar.y"
{ yylhsminor.yy31 = NewConditionNode(yymsp[-2].minor.yy31, OR, yymsp[0].minor.yy31); }
#line 1026 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 20: /* op ::= EQ */
#line 108 "grammar.y"
{ yymsp[0].minor.yy32 = EQ; }
#line 1032 "grammar.c"
        break;
      case 21: /* op ::= GT */
#line 109 "grammar.y"
{ yymsp[0].minor.yy32 = GT; }
#line 1037 "grammar.c"
        break;
      case 22: /* op ::= LT */
#line 110 "grammar.y"
{ yymsp[0].minor.yy32 = LT; }
#line 1042 "grammar.c"
        break;
      case 23: /* op ::= LE */
#line 111 "grammar.y"
{ yymsp[0].minor.yy32 = LE; }
#line 1047 "grammar.c"
        break;
      case 24: /* op ::= GE */
#line 112 "grammar.y"
{ yymsp[0].minor.yy32 = GE; }
#line 1052 "grammar.c"
        break;
      case 25: /* op ::= NE */
#line 113 "grammar.y"
{ yymsp[0].minor.yy32 = NE; }
#line 1057 "grammar.c"
        break;
      case 26: /* value ::= INTEGER */
#line 119 "grammar.y"
{  yylhsminor.yy22 = SI_LongVal(yymsp[0].minor.yy0.intval); }
#line 1062 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 27: /* value ::= STRING */
#line 120 "grammar.y"
{  yylhsminor.yy22 = SI_StringValC(strdup(yymsp[0].minor.yy0.strval)); }
#line 1068 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 28: /* value ::= FLOAT */
#line 121 "grammar.y"
{  yylhsminor.yy22 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1074 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 29: /* value ::= TRUE */
#line 122 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(1); }
#line 1080 "grammar.c"
        break;
      case 30: /* value ::= FALSE */
#line 123 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(0); }
#line 1085 "grammar.c"
        break;
      case 31: /* returnClause ::= RETURN returnElements */
#line 127 "grammar.y"
{
	yymsp[-1].minor.yy93 = NewReturnNode(yymsp[0].minor.yy64);
}
#line 1092 "grammar.c"
        break;
      case 32: /* returnElements ::= returnElements COMMA returnElement */
#line 134 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy71);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1100 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 33: /* returnElements ::= returnElement */
#line 139 "grammar.y"
{
	yylhsminor.yy64 = NewVector(ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy71);
}
#line 1109 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 34: /* returnElement ::= variable */
#line 146 "grammar.y"
{
	yylhsminor.yy71 = NewReturnElementNode(N_PROP, yymsp[0].minor.yy65->alias, yymsp[0].minor.yy65->property, NULL);
	FreeVariable(yymsp[0].minor.yy65);
}
#line 1118 "grammar.c"
  yymsp[0].minor.yy71 = yylhsminor.yy71;
        break;
      case 35: /* returnElement ::= aggFunc */
#line 151 "grammar.y"
{
	yylhsminor.yy71 = yymsp[0].minor.yy71;
}
#line 1126 "grammar.c"
  yymsp[0].minor.yy71 = yylhsminor.yy71;
        break;
      case 36: /* variableList ::= variableList COMMA variable */
#line 156 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy65);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1135 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 37: /* variableList ::= variable */
#line 161 "grammar.y"
{
	yylhsminor.yy64 = NewVector(Variable*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy65);
}
#line 1144 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 38: /* variable ::= STRING DOT STRING */
#line 168 "grammar.y"
{
	yylhsminor.yy65 = NewVariable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1152 "grammar.c"
  yymsp[-2].minor.yy65 = yylhsminor.yy65;
        break;
      case 39: /* aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS */
#line 174 "grammar.y"
{
	yylhsminor.yy71 = NewReturnElementNode(N_AGG_FUNC, yymsp[-1].minor.yy65->alias, yymsp[-1].minor.yy65->property, yymsp[-3].minor.yy0.strval);
}
#line 1160 "grammar.c"
  yymsp[-3].minor.yy71 = yylhsminor.yy71;
        break;
      case 40: /* orderClause ::= */
#line 180 "grammar.y"
{
	yymsp[1].minor.yy79 = NULL;
}
#line 1168 "grammar.c"
        break;
      case 41: /* orderClause ::= ORDER BY variableList */
#line 183 "grammar.y"
{
	yymsp[-2].minor.yy79 = NewOrderNode(yymsp[0].minor.yy64);
}
#line 1175 "grammar.c"
        break;
      case 42: /* limitClause ::= */
#line 189 "grammar.y"
{
	yymsp[1].minor.yy18 = NULL;
}
#line 1182 "grammar.c"
        break;
      case 43: /* limitClause ::= LIMIT INTEGER */
#line 192 "grammar.y"
{
	yymsp[-1].minor.yy18 = NewLimitNode(yymsp[0].minor.yy0.intval);
}
#line 1189 "grammar.c"
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ){
      yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    }
    yymsp -= yysize-1;
    yypParser->yytos = yymsp;
    yymsp->stateno = (YYACTIONTYPE)yyact;
    yymsp->major = (YYCODETYPE)yygoto;
    yyTraceShift(yypParser, yyact);
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yypParser->yytos -= yysize;
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  ParseTOKENTYPE yyminor         /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/
#line 7 "grammar.y"

	//printf("Syntax error!\n");
#line 1251 "grammar.c"
/************ End %syntax_error code ******************************************/
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  assert( yypParser->yytos==yypParser->yystack );
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  unsigned int yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  yypParser = (yyParser*)yyp;
  assert( yypParser->yytos!=0 );
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yytos->major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while( yypParser->yytos >= &yypParser->yystack
            && yymx != YYERRORSYMBOL
            && (yyact = yy_find_reduce_action(
                        yypParser->yytos->stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yytos < yypParser->yystack || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
          yypParser->yyerrcnt = -1;
#endif
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
        yypParser->yyerrcnt = -1;
#endif
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yytos>yypParser->yystack );
#ifndef NDEBUG
  if( yyTraceFILE ){
    yyStackEntry *i;
    char cDiv = '[';
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=&yypParser->yystack[1]; i<=yypParser->yytos; i++){
      fprintf(yyTraceFILE,"%c%s", cDiv, yyTokenName[i->major]);
      cDiv = ' ';
    }
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
#line 196 "grammar.y"


	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;

	QueryExpressionNode *ParseQuery(const char *c, size_t len) {

		yy_scan_bytes(c, len);
  		void* pParser = ParseAlloc(malloc);
  		int t = 0;

  		QueryExpressionNode *ret = NULL;

  		while( (t = yylex()) != 0) {
  			////printf("Token %d\n", t);
    		Parse(pParser, t, tok, &ret);
  		}
  		Parse(pParser, 0, tok, &ret);
  		ParseFree(pParser, free);

  		return ret;
	}
	
#line 1483 "grammar.c"
