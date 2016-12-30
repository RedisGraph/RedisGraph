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
#define YYNOCODE 44
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  ReturnNode* yy11;
  SIValue yy22;
  Vector* yy24;
  WhereNode* yy34;
  ChainElement* yy44;
  MatchNode* yy46;
  FilterNode* yy55;
  QueryExpressionNode* yy59;
  ReturnElementNode* yy60;
  int yy64;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  QueryExpressionNode **root ;
#define ParseARG_PDECL , QueryExpressionNode **root 
#define ParseARG_FETCH  QueryExpressionNode **root  = yypParser->root 
#define ParseARG_STORE yypParser->root  = root 
#define YYNSTATE             44
#define YYNRULE              38
#define YY_MAX_SHIFT         43
#define YY_MIN_SHIFTREDUCE   75
#define YY_MAX_SHIFTREDUCE   112
#define YY_MIN_REDUCE        113
#define YY_MAX_REDUCE        150
#define YY_ERROR_ACTION      151
#define YY_ACCEPT_ACTION     152
#define YY_NO_ACTION         153
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
#define YY_ACTTAB_COUNT (86)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    95,   96,   99,   97,   98,  152,   43,   14,   36,   42,
 /*    10 */   108,  109,  110,  107,  109,  110,    3,  100,   15,  101,
 /*    20 */   103,  104,  105,   19,   31,   83,    7,    9,    6,   78,
 /*    30 */    12,   29,    8,   38,   23,   25,   27,   92,   21,   24,
 /*    40 */    41,   28,    5,   33,   82,    7,    9,   11,   91,   40,
 /*    50 */    34,   16,   93,   20,   79,    1,   10,   22,   87,   76,
 /*    60 */    39,   86,   13,   17,   30,   18,   85,   26,   81,   84,
 /*    70 */    80,   32,    9,   90,   35,    4,   37,  111,  112,   41,
 /*    80 */   113,  115,  115,  115,  115,    2,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,    4,    5,    6,    7,   28,   29,   30,   10,   39,
 /*    10 */    40,   41,   42,   40,   41,   42,    8,   20,    9,   21,
 /*    20 */    22,   23,   24,   10,   11,   12,    1,    2,   33,   34,
 /*    30 */     9,   13,    9,   10,   10,   17,   10,   12,   10,   15,
 /*    40 */    19,   15,   37,   11,   12,    1,    2,   35,   38,   10,
 /*    50 */    36,   36,   36,   36,   34,   25,   18,   15,   13,   32,
 /*    60 */    41,   13,   31,   14,   10,   14,   16,   15,   12,   16,
 /*    70 */    12,   10,    2,   10,   19,   10,   19,   10,   12,   19,
 /*    80 */     0,   43,   43,   43,   43,   26,
};
#define YY_SHIFT_USE_DFLT (86)
#define YY_SHIFT_COUNT    (43)
#define YY_SHIFT_MIN      (-3)
#define YY_SHIFT_MAX      (80)
static const signed char yy_shift_ofst[] = {
 /*     0 */     8,   28,   28,    9,   -3,   -2,   18,   23,   23,   23,
 /*    10 */    23,    9,   39,   30,   38,   13,   25,   24,   26,   32,
 /*    20 */    44,   21,   45,   42,   48,   49,   50,   52,   53,   51,
 /*    30 */    56,   54,   58,   61,   70,   63,   55,   65,   57,   66,
 /*    40 */    60,   67,   59,   80,
};
#define YY_REDUCE_USE_DFLT (-31)
#define YY_REDUCE_COUNT (14)
#define YY_REDUCE_MIN   (-30)
#define YY_REDUCE_MAX   (31)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -23,  -30,  -27,   -5,    5,   10,   12,   14,   15,   16,
 /*    10 */    17,   20,   19,   27,   31,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   151,  151,  151,  151,  151,  151,  115,  151,  151,  151,
 /*    10 */   151,  151,  151,  151,  126,  151,  151,  151,  151,  151,
 /*    20 */   127,  151,  151,  151,  151,  151,  151,  151,  151,  151,
 /*    30 */   151,  151,  151,  151,  132,  151,  140,  151,  151,  151,
 /*    40 */   151,  151,  144,  151,
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
  "FALSE",         "RETURN",        "COMMA",         "error",       
  "query",         "expr",          "matchClause",   "whereClause", 
  "returnClause",  "chain",         "node",          "link",        
  "cond",          "op",            "value",         "returnElements",
  "returnElement",  "variable",      "aggFunc",     
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause returnClause",
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
 /*  36 */ "variable ::= STRING DOT STRING",
 /*  37 */ "aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS",
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
    case 36: /* cond */
{
#line 100 "grammar.y"
 FreeFilterNode((yypminor->yy55)); 
#line 524 "grammar.c"
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
  { 28, 1 },
  { 29, 3 },
  { 30, 2 },
  { 33, 1 },
  { 33, 3 },
  { 34, 5 },
  { 34, 4 },
  { 34, 3 },
  { 34, 2 },
  { 35, 4 },
  { 35, 5 },
  { 35, 4 },
  { 35, 5 },
  { 31, 0 },
  { 31, 2 },
  { 36, 7 },
  { 36, 5 },
  { 36, 3 },
  { 36, 3 },
  { 36, 3 },
  { 37, 1 },
  { 37, 1 },
  { 37, 1 },
  { 37, 1 },
  { 37, 1 },
  { 37, 1 },
  { 38, 1 },
  { 38, 1 },
  { 38, 1 },
  { 38, 1 },
  { 38, 1 },
  { 32, 2 },
  { 39, 3 },
  { 39, 1 },
  { 40, 1 },
  { 40, 1 },
  { 41, 3 },
  { 42, 4 },
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
{ *root = yymsp[0].minor.yy59; }
#line 870 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause returnClause */
#line 29 "grammar.y"
{ 
	//printf("Query expression\n");
	yylhsminor.yy59 = NewQueryExpressionNode(yymsp[-2].minor.yy46, yymsp[-1].minor.yy34, yymsp[0].minor.yy11); 
}
#line 878 "grammar.c"
  yymsp[-2].minor.yy59 = yylhsminor.yy59;
        break;
      case 2: /* matchClause ::= MATCH chain */
#line 37 "grammar.y"
{
	yymsp[-1].minor.yy46 = NewMatchNode(yymsp[0].minor.yy24);
}
#line 886 "grammar.c"
        break;
      case 3: /* chain ::= node */
#line 44 "grammar.y"
{
	yylhsminor.yy24 = NewVector(ChainElement*, 1);
	Vector_Push(yylhsminor.yy24, yymsp[0].minor.yy44);
}
#line 894 "grammar.c"
  yymsp[0].minor.yy24 = yylhsminor.yy24;
        break;
      case 4: /* chain ::= chain link node */
#line 49 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy24, yymsp[-1].minor.yy44);
	Vector_Push(yymsp[-2].minor.yy24, yymsp[0].minor.yy44);
	yylhsminor.yy24 = yymsp[-2].minor.yy24;
}
#line 904 "grammar.c"
  yymsp[-2].minor.yy24 = yylhsminor.yy24;
        break;
      case 5: /* node ::= LEFT_PARENTHESIS STRING COLON STRING RIGHT_PARENTHESIS */
#line 58 "grammar.y"
{
	yymsp[-4].minor.yy44 = NewChainEntity(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy0.strval);
}
#line 912 "grammar.c"
        break;
      case 6: /* node ::= LEFT_PARENTHESIS COLON STRING RIGHT_PARENTHESIS */
#line 61 "grammar.y"
{
	yymsp[-3].minor.yy44 = NewChainEntity("", yymsp[-1].minor.yy0.strval);
}
#line 919 "grammar.c"
        break;
      case 7: /* node ::= LEFT_PARENTHESIS STRING RIGHT_PARENTHESIS */
#line 64 "grammar.y"
{
	yymsp[-2].minor.yy44 = NewChainEntity(yymsp[-1].minor.yy0.strval, "");
}
#line 926 "grammar.c"
        break;
      case 8: /* node ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS */
#line 67 "grammar.y"
{
	yymsp[-1].minor.yy44 = NewChainEntity("", "");
}
#line 933 "grammar.c"
        break;
      case 9: /* link ::= DASH LEFT_BRACKET RIGHT_BRACKET RIGHT_ARROW */
#line 74 "grammar.y"
{ 
	yymsp[-3].minor.yy44 =  NewChainLink("", N_LEFT_TO_RIGHT); 
}
#line 940 "grammar.c"
        break;
      case 10: /* link ::= DASH LEFT_BRACKET STRING RIGHT_BRACKET RIGHT_ARROW */
#line 77 "grammar.y"
{ 
	yymsp[-4].minor.yy44 = NewChainLink(yymsp[-2].minor.yy0.strval, N_LEFT_TO_RIGHT);
}
#line 947 "grammar.c"
        break;
      case 11: /* link ::= LEFT_ARROW LEFT_BRACKET RIGHT_BRACKET DASH */
#line 80 "grammar.y"
{ 
	yymsp[-3].minor.yy44 = NewChainLink("", N_RIGHT_TO_LEFT); 
}
#line 954 "grammar.c"
        break;
      case 12: /* link ::= LEFT_ARROW LEFT_BRACKET STRING RIGHT_BRACKET DASH */
#line 83 "grammar.y"
{ 
	yymsp[-4].minor.yy44 = NewChainLink(yymsp[-2].minor.yy0.strval, N_RIGHT_TO_LEFT);
}
#line 961 "grammar.c"
        break;
      case 13: /* whereClause ::= */
#line 89 "grammar.y"
{ 
	//printf("no where clause\n");
	yymsp[1].minor.yy34 = NULL;
}
#line 969 "grammar.c"
        break;
      case 14: /* whereClause ::= WHERE cond */
#line 93 "grammar.y"
{
	//printf("where clause\n");
	yymsp[-1].minor.yy34 = NewWhereNode(yymsp[0].minor.yy55);
}
#line 977 "grammar.c"
        break;
      case 15: /* cond ::= STRING DOT STRING op STRING DOT STRING */
#line 102 "grammar.y"
{ yylhsminor.yy55 = NewVaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy64, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 982 "grammar.c"
  yymsp[-6].minor.yy55 = yylhsminor.yy55;
        break;
      case 16: /* cond ::= STRING DOT STRING op value */
#line 103 "grammar.y"
{ yylhsminor.yy55 = NewConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy64, yymsp[0].minor.yy22); }
#line 988 "grammar.c"
  yymsp[-4].minor.yy55 = yylhsminor.yy55;
        break;
      case 17: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 104 "grammar.y"
{ yymsp[-2].minor.yy55 = yymsp[-1].minor.yy55; }
#line 994 "grammar.c"
        break;
      case 18: /* cond ::= cond AND cond */
#line 105 "grammar.y"
{ yylhsminor.yy55 = NewConditionNode(yymsp[-2].minor.yy55, AND, yymsp[0].minor.yy55); }
#line 999 "grammar.c"
  yymsp[-2].minor.yy55 = yylhsminor.yy55;
        break;
      case 19: /* cond ::= cond OR cond */
#line 106 "grammar.y"
{ yylhsminor.yy55 = NewConditionNode(yymsp[-2].minor.yy55, OR, yymsp[0].minor.yy55); }
#line 1005 "grammar.c"
  yymsp[-2].minor.yy55 = yylhsminor.yy55;
        break;
      case 20: /* op ::= EQ */
#line 110 "grammar.y"
{ yymsp[0].minor.yy64 = EQ; }
#line 1011 "grammar.c"
        break;
      case 21: /* op ::= GT */
#line 111 "grammar.y"
{ yymsp[0].minor.yy64 = GT; }
#line 1016 "grammar.c"
        break;
      case 22: /* op ::= LT */
#line 112 "grammar.y"
{ yymsp[0].minor.yy64 = LT; }
#line 1021 "grammar.c"
        break;
      case 23: /* op ::= LE */
#line 113 "grammar.y"
{ yymsp[0].minor.yy64 = LE; }
#line 1026 "grammar.c"
        break;
      case 24: /* op ::= GE */
#line 114 "grammar.y"
{ yymsp[0].minor.yy64 = GE; }
#line 1031 "grammar.c"
        break;
      case 25: /* op ::= NE */
#line 115 "grammar.y"
{ yymsp[0].minor.yy64 = NE; }
#line 1036 "grammar.c"
        break;
      case 26: /* value ::= INTEGER */
#line 121 "grammar.y"
{  yylhsminor.yy22 = SI_LongVal(yymsp[0].minor.yy0.intval); }
#line 1041 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 27: /* value ::= STRING */
#line 122 "grammar.y"
{  yylhsminor.yy22 = SI_StringValC(strdup(yymsp[0].minor.yy0.strval)); }
#line 1047 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 28: /* value ::= FLOAT */
#line 123 "grammar.y"
{  yylhsminor.yy22 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1053 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 29: /* value ::= TRUE */
#line 124 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(1); }
#line 1059 "grammar.c"
        break;
      case 30: /* value ::= FALSE */
#line 125 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(0); }
#line 1064 "grammar.c"
        break;
      case 31: /* returnClause ::= RETURN returnElements */
#line 129 "grammar.y"
{
	yymsp[-1].minor.yy11 = NewReturnNode(yymsp[0].minor.yy24);
}
#line 1071 "grammar.c"
        break;
      case 32: /* returnElements ::= returnElements COMMA returnElement */
#line 136 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy24, yymsp[0].minor.yy60);
	yylhsminor.yy24 = yymsp[-2].minor.yy24;
}
#line 1079 "grammar.c"
  yymsp[-2].minor.yy24 = yylhsminor.yy24;
        break;
      case 33: /* returnElements ::= returnElement */
#line 141 "grammar.y"
{
	yylhsminor.yy24 = NewVector(ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy24, yymsp[0].minor.yy60); 
}
#line 1088 "grammar.c"
  yymsp[0].minor.yy24 = yylhsminor.yy24;
        break;
      case 34: /* returnElement ::= variable */
      case 35: /* returnElement ::= aggFunc */ yytestcase(yyruleno==35);
#line 148 "grammar.y"
{
	yylhsminor.yy60 = yymsp[0].minor.yy60;
}
#line 1097 "grammar.c"
  yymsp[0].minor.yy60 = yylhsminor.yy60;
        break;
      case 36: /* variable ::= STRING DOT STRING */
#line 158 "grammar.y"
{
	yylhsminor.yy60 = NewReturnElementNode(N_PROP, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval, NULL);
}
#line 1105 "grammar.c"
  yymsp[-2].minor.yy60 = yylhsminor.yy60;
        break;
      case 37: /* aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS */
#line 164 "grammar.y"
{
	yylhsminor.yy60 = NewReturnElementNode(N_AGG_FUNC, yymsp[-1].minor.yy60->alias, yymsp[-1].minor.yy60->property, yymsp[-3].minor.yy0.strval);
}
#line 1113 "grammar.c"
  yymsp[-3].minor.yy60 = yylhsminor.yy60;
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
#line 1176 "grammar.c"
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
#line 169 "grammar.y"


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
	
#line 1408 "grammar.c"
