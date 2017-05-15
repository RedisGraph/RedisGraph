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
#line 7 "grammar.y"

	#include <stdlib.h>
	#include <stdio.h>
	#include <assert.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "parse.h"
	#include "../value.h"

	void yyerror(char *s);
#line 40 "grammar.c"
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
#define YYNOCODE 60
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  SIValue yy18;
  OrderNode* yy29;
  ColumnNode* yy33;
  Variable* yy43;
  Vector* yy54;
  ReturnNode* yy55;
  LimitNode* yy58;
  LinkEntity* yy68;
  QueryExpressionNode* yy71;
  MatchNode* yy78;
  ReturnElementNode* yy79;
  FilterNode* yy99;
  WhereNode* yy110;
  int yy111;
  NodeEntity* yy114;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             72
#define YYNRULE              58
#define YY_MAX_SHIFT         71
#define YY_MIN_SHIFTREDUCE   114
#define YY_MAX_SHIFTREDUCE   171
#define YY_MIN_REDUCE        172
#define YY_MAX_REDUCE        229
#define YY_ERROR_ACTION      230
#define YY_ACCEPT_ACTION     231
#define YY_NO_ACTION         232
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
#define YY_ACTTAB_COUNT (150)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   140,  141,  144,  142,  143,  231,   71,   32,  147,  153,
 /*    10 */    65,  157,   56,   66,  154,   65,  157,   14,  117,   22,
 /*    20 */   145,   19,  146,  148,  149,  150,  146,  148,  149,  150,
 /*    30 */    59,  154,   65,  157,  168,   36,   34,  167,   12,   39,
 /*    40 */    13,   47,   15,   17,    6,   41,   25,  168,   25,   53,
 /*    50 */   166,   28,   25,  137,    2,    5,   25,  164,  165,   16,
 /*    60 */    58,   15,   17,   36,   67,   68,   11,    9,   49,  136,
 /*    70 */    43,   54,   33,   45,   42,   10,   48,   27,   50,   25,
 /*    80 */    63,  118,   38,  138,   35,   37,   40,   70,   44,   69,
 /*    90 */    46,    1,  115,  132,   51,   52,   29,   30,   18,  124,
 /*   100 */   127,   62,   31,   20,  128,   21,  126,  125,  123,  122,
 /*   110 */   120,   23,  121,    8,   26,  119,   24,   17,  130,  135,
 /*   120 */     7,   60,  161,  172,  156,   61,  159,   55,   57,   67,
 /*   130 */   174,    3,  174,  174,  174,  174,  174,  174,   64,  174,
 /*   140 */   174,  174,  171,  174,  174,  174,  174,  174,  174,    4,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,    4,    5,    6,    7,   37,   38,   39,   10,   54,
 /*    10 */    55,   56,   10,   53,   54,   55,   56,   44,   45,   13,
 /*    20 */    23,   15,   24,   25,   26,   27,   24,   25,   26,   27,
 /*    30 */    53,   54,   55,   56,   55,   10,   57,   58,   10,   11,
 /*    40 */    10,   11,    1,    2,   20,   11,   18,   55,   18,   11,
 /*    50 */    58,    9,   18,   12,   29,    8,   18,   33,   34,    9,
 /*    60 */    10,    1,    2,   10,   22,   10,    9,   52,   50,   50,
 /*    70 */    47,   51,   51,   47,   47,   16,   47,   46,   10,   18,
 /*    80 */    10,   45,   47,   51,   51,   48,   47,   35,   48,   31,
 /*    90 */    47,   28,   43,   49,   49,   47,   42,   41,   21,   13,
 /*   100 */    17,   55,   40,   10,   17,   10,   17,   17,   14,   12,
 /*   110 */    12,   10,   12,   11,   10,   12,   20,    2,   19,   10,
 /*   120 */    10,   30,   10,    0,   10,   12,   10,   22,   22,   22,
 /*   130 */    59,   20,   59,   59,   59,   59,   59,   59,   30,   59,
 /*   140 */    59,   59,   24,   59,   59,   59,   59,   59,   59,   32,
};
#define YY_SHIFT_USE_DFLT (150)
#define YY_SHIFT_COUNT    (71)
#define YY_SHIFT_MIN      (-3)
#define YY_SHIFT_MAX      (123)
static const short yy_shift_ofst[] = {
 /*     0 */    47,   25,   53,   53,   55,   57,   55,   -3,   -2,    2,
 /*    10 */    28,   30,   34,   38,    6,   50,   50,   50,   50,   59,
 /*    20 */    61,   61,   59,   61,   68,   68,   61,   57,   70,   52,
 /*    30 */    58,   63,   77,   41,   24,   60,   42,   86,   83,   93,
 /*    40 */    87,   95,   89,   90,   94,   97,   98,  101,  100,   96,
 /*    50 */   102,   99,  103,  104,  115,  109,  105,  110,  106,  111,
 /*    60 */   112,   91,  113,  107,  114,  108,  111,  116,  107,  117,
 /*    70 */   118,  123,
};
#define YY_REDUCE_USE_DFLT (-46)
#define YY_REDUCE_COUNT (32)
#define YY_REDUCE_MIN   (-45)
#define YY_REDUCE_MAX   (62)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -32,  -40,  -23,  -45,  -21,  -27,   -8,   15,   18,   19,
 /*    10 */    23,   26,   27,   29,   31,   20,   21,   32,   33,   37,
 /*    20 */    35,   39,   40,   43,   44,   45,   48,   36,   46,   49,
 /*    30 */    54,   56,   62,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
 /*    10 */   187,  187,  187,  187,  174,  230,  230,  230,  230,  230,
 /*    20 */   187,  187,  230,  187,  230,  230,  187,  230,  230,  228,
 /*    30 */   220,  230,  191,  230,  221,  192,  216,  230,  230,  230,
 /*    40 */   230,  230,  230,  230,  230,  230,  230,  230,  230,  189,
 /*    50 */   230,  230,  230,  230,  197,  230,  205,  230,  230,  210,
 /*    60 */   230,  218,  230,  230,  230,  213,  209,  230,  227,  230,
 /*    70 */   230,  230,
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
  "RIGHT_PARENTHESIS",  "DASH",          "RIGHT_ARROW",   "LEFT_ARROW",  
  "LEFT_BRACKET",  "RIGHT_BRACKET",  "LEFT_CURLY_BRACKET",  "RIGHT_CURLY_BRACKET",
  "COMMA",         "WHERE",         "DOT",           "NE",          
  "INTEGER",       "FLOAT",         "TRUE",          "FALSE",       
  "RETURN",        "DISTINCT",      "AS",            "ORDER",       
  "BY",            "ASC",           "DESC",          "LIMIT",       
  "error",         "query",         "expr",          "matchClause", 
  "whereClause",   "returnClause",  "orderClause",   "limitClause", 
  "chain",         "node",          "link",          "properties",  
  "edge",          "mapLiteral",    "value",         "cond",        
  "op",            "returnElements",  "returnElement",  "variable",    
  "aggFunc",       "columnNameList",  "columnName",  
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
 /*   5 */ "node ::= LEFT_PARENTHESIS STRING COLON STRING properties RIGHT_PARENTHESIS",
 /*   6 */ "node ::= LEFT_PARENTHESIS COLON STRING properties RIGHT_PARENTHESIS",
 /*   7 */ "node ::= LEFT_PARENTHESIS STRING properties RIGHT_PARENTHESIS",
 /*   8 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*   9 */ "link ::= DASH edge RIGHT_ARROW",
 /*  10 */ "link ::= LEFT_ARROW edge DASH",
 /*  11 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  12 */ "edge ::= LEFT_BRACKET STRING properties RIGHT_BRACKET",
 /*  13 */ "edge ::= LEFT_BRACKET COLON STRING properties RIGHT_BRACKET",
 /*  14 */ "edge ::= LEFT_BRACKET STRING COLON STRING properties RIGHT_BRACKET",
 /*  15 */ "properties ::=",
 /*  16 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  17 */ "mapLiteral ::= STRING COLON value",
 /*  18 */ "mapLiteral ::= STRING COLON value COMMA mapLiteral",
 /*  19 */ "whereClause ::=",
 /*  20 */ "whereClause ::= WHERE cond",
 /*  21 */ "cond ::= STRING DOT STRING op STRING DOT STRING",
 /*  22 */ "cond ::= STRING DOT STRING op value",
 /*  23 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  24 */ "cond ::= cond AND cond",
 /*  25 */ "cond ::= cond OR cond",
 /*  26 */ "op ::= EQ",
 /*  27 */ "op ::= GT",
 /*  28 */ "op ::= LT",
 /*  29 */ "op ::= LE",
 /*  30 */ "op ::= GE",
 /*  31 */ "op ::= NE",
 /*  32 */ "value ::= INTEGER",
 /*  33 */ "value ::= STRING",
 /*  34 */ "value ::= FLOAT",
 /*  35 */ "value ::= TRUE",
 /*  36 */ "value ::= FALSE",
 /*  37 */ "returnClause ::= RETURN returnElements",
 /*  38 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  39 */ "returnElements ::= returnElements COMMA returnElement",
 /*  40 */ "returnElements ::= returnElement",
 /*  41 */ "returnElement ::= variable",
 /*  42 */ "returnElement ::= variable AS STRING",
 /*  43 */ "returnElement ::= aggFunc",
 /*  44 */ "returnElement ::= STRING",
 /*  45 */ "variable ::= STRING DOT STRING",
 /*  46 */ "aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS",
 /*  47 */ "aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS AS STRING",
 /*  48 */ "orderClause ::=",
 /*  49 */ "orderClause ::= ORDER BY columnNameList",
 /*  50 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  51 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  52 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  53 */ "columnNameList ::= columnName",
 /*  54 */ "columnName ::= variable",
 /*  55 */ "columnName ::= STRING",
 /*  56 */ "limitClause ::=",
 /*  57 */ "limitClause ::= LIMIT INTEGER",
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
    case 51: /* cond */
{
#line 167 "grammar.y"
 FreeFilterNode((yypminor->yy99)); 
#line 575 "grammar.c"
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
  { 37, 1 },
  { 38, 5 },
  { 39, 2 },
  { 44, 1 },
  { 44, 3 },
  { 45, 6 },
  { 45, 5 },
  { 45, 4 },
  { 45, 3 },
  { 46, 3 },
  { 46, 3 },
  { 48, 3 },
  { 48, 4 },
  { 48, 5 },
  { 48, 6 },
  { 47, 0 },
  { 47, 3 },
  { 49, 3 },
  { 49, 5 },
  { 40, 0 },
  { 40, 2 },
  { 51, 7 },
  { 51, 5 },
  { 51, 3 },
  { 51, 3 },
  { 51, 3 },
  { 52, 1 },
  { 52, 1 },
  { 52, 1 },
  { 52, 1 },
  { 52, 1 },
  { 52, 1 },
  { 50, 1 },
  { 50, 1 },
  { 50, 1 },
  { 50, 1 },
  { 50, 1 },
  { 41, 2 },
  { 41, 3 },
  { 53, 3 },
  { 53, 1 },
  { 54, 1 },
  { 54, 3 },
  { 54, 1 },
  { 54, 1 },
  { 55, 3 },
  { 56, 4 },
  { 56, 6 },
  { 42, 0 },
  { 42, 3 },
  { 42, 4 },
  { 42, 4 },
  { 57, 3 },
  { 57, 1 },
  { 58, 1 },
  { 58, 1 },
  { 43, 0 },
  { 43, 2 },
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
#line 32 "grammar.y"
{ ctx->root = yymsp[0].minor.yy71; }
#line 941 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause returnClause orderClause limitClause */
#line 37 "grammar.y"
{
	yylhsminor.yy71 = NewQueryExpressionNode(yymsp[-4].minor.yy78, yymsp[-3].minor.yy110, yymsp[-2].minor.yy55, yymsp[-1].minor.yy29, yymsp[0].minor.yy58);
}
#line 948 "grammar.c"
  yymsp[-4].minor.yy71 = yylhsminor.yy71;
        break;
      case 2: /* matchClause ::= MATCH chain */
#line 44 "grammar.y"
{
	yymsp[-1].minor.yy78 = NewMatchNode(yymsp[0].minor.yy54);
}
#line 956 "grammar.c"
        break;
      case 3: /* chain ::= node */
#line 51 "grammar.y"
{
	yylhsminor.yy54 = NewVector(GraphEntity*, 1);
	Vector_Push(yylhsminor.yy54, yymsp[0].minor.yy114);
}
#line 964 "grammar.c"
  yymsp[0].minor.yy54 = yylhsminor.yy54;
        break;
      case 4: /* chain ::= chain link node */
#line 56 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy54, yymsp[-1].minor.yy68);
	Vector_Push(yymsp[-2].minor.yy54, yymsp[0].minor.yy114);
	yylhsminor.yy54 = yymsp[-2].minor.yy54;
}
#line 974 "grammar.c"
  yymsp[-2].minor.yy54 = yylhsminor.yy54;
        break;
      case 5: /* node ::= LEFT_PARENTHESIS STRING COLON STRING properties RIGHT_PARENTHESIS */
#line 66 "grammar.y"
{
	yymsp[-5].minor.yy114 = NewNodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy54);
}
#line 982 "grammar.c"
        break;
      case 6: /* node ::= LEFT_PARENTHESIS COLON STRING properties RIGHT_PARENTHESIS */
#line 71 "grammar.y"
{
	yymsp[-4].minor.yy114 = NewNodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy54);
}
#line 989 "grammar.c"
        break;
      case 7: /* node ::= LEFT_PARENTHESIS STRING properties RIGHT_PARENTHESIS */
#line 76 "grammar.y"
{
	yymsp[-3].minor.yy114 = NewNodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy54);
}
#line 996 "grammar.c"
        break;
      case 8: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 81 "grammar.y"
{
	yymsp[-2].minor.yy114 = NewNodeEntity(NULL, NULL, yymsp[-1].minor.yy54);
}
#line 1003 "grammar.c"
        break;
      case 9: /* link ::= DASH edge RIGHT_ARROW */
#line 88 "grammar.y"
{
	yymsp[-2].minor.yy68 = yymsp[-1].minor.yy68;
	yymsp[-2].minor.yy68->direction = N_LEFT_TO_RIGHT;
}
#line 1011 "grammar.c"
        break;
      case 10: /* link ::= LEFT_ARROW edge DASH */
#line 94 "grammar.y"
{
	yymsp[-2].minor.yy68 = yymsp[-1].minor.yy68;
	yymsp[-2].minor.yy68->direction = N_RIGHT_TO_LEFT;
}
#line 1019 "grammar.c"
        break;
      case 11: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 101 "grammar.y"
{ 
	yymsp[-2].minor.yy68 = NewLinkEntity(NULL, NULL, yymsp[-1].minor.yy54, N_DIR_UNKNOWN);
}
#line 1026 "grammar.c"
        break;
      case 12: /* edge ::= LEFT_BRACKET STRING properties RIGHT_BRACKET */
#line 106 "grammar.y"
{ 
	yymsp[-3].minor.yy68 = NewLinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy54, N_DIR_UNKNOWN);
}
#line 1033 "grammar.c"
        break;
      case 13: /* edge ::= LEFT_BRACKET COLON STRING properties RIGHT_BRACKET */
#line 111 "grammar.y"
{ 
	yymsp[-4].minor.yy68 = NewLinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy54, N_DIR_UNKNOWN);
}
#line 1040 "grammar.c"
        break;
      case 14: /* edge ::= LEFT_BRACKET STRING COLON STRING properties RIGHT_BRACKET */
#line 116 "grammar.y"
{ 
	yymsp[-5].minor.yy68 = NewLinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy54, N_DIR_UNKNOWN);
}
#line 1047 "grammar.c"
        break;
      case 15: /* properties ::= */
#line 122 "grammar.y"
{
	yymsp[1].minor.yy54 = NULL;
}
#line 1054 "grammar.c"
        break;
      case 16: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 126 "grammar.y"
{
	yymsp[-2].minor.yy54 = yymsp[-1].minor.yy54;
}
#line 1061 "grammar.c"
        break;
      case 17: /* mapLiteral ::= STRING COLON value */
#line 131 "grammar.y"
{
	yylhsminor.yy54 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(yymsp[-2].minor.yy0.strval));
	Vector_Push(yylhsminor.yy54, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy18;
	Vector_Push(yylhsminor.yy54, val);
}
#line 1076 "grammar.c"
  yymsp[-2].minor.yy54 = yylhsminor.yy54;
        break;
      case 18: /* mapLiteral ::= STRING COLON value COMMA mapLiteral */
#line 143 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(yymsp[-4].minor.yy0.strval));
	Vector_Push(yymsp[0].minor.yy54, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy18;
	Vector_Push(yymsp[0].minor.yy54, val);
	
	yylhsminor.yy54 = yymsp[0].minor.yy54;
}
#line 1092 "grammar.c"
  yymsp[-4].minor.yy54 = yylhsminor.yy54;
        break;
      case 19: /* whereClause ::= */
#line 157 "grammar.y"
{ 
	yymsp[1].minor.yy110 = NULL;
}
#line 1100 "grammar.c"
        break;
      case 20: /* whereClause ::= WHERE cond */
#line 160 "grammar.y"
{
	//printf("where clause\n");
	yymsp[-1].minor.yy110 = NewWhereNode(yymsp[0].minor.yy99);
}
#line 1108 "grammar.c"
        break;
      case 21: /* cond ::= STRING DOT STRING op STRING DOT STRING */
#line 169 "grammar.y"
{ yylhsminor.yy99 = NewVaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy111, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1113 "grammar.c"
  yymsp[-6].minor.yy99 = yylhsminor.yy99;
        break;
      case 22: /* cond ::= STRING DOT STRING op value */
#line 170 "grammar.y"
{ yylhsminor.yy99 = NewConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy111, yymsp[0].minor.yy18); }
#line 1119 "grammar.c"
  yymsp[-4].minor.yy99 = yylhsminor.yy99;
        break;
      case 23: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 171 "grammar.y"
{ yymsp[-2].minor.yy99 = yymsp[-1].minor.yy99; }
#line 1125 "grammar.c"
        break;
      case 24: /* cond ::= cond AND cond */
#line 172 "grammar.y"
{ yylhsminor.yy99 = NewConditionNode(yymsp[-2].minor.yy99, AND, yymsp[0].minor.yy99); }
#line 1130 "grammar.c"
  yymsp[-2].minor.yy99 = yylhsminor.yy99;
        break;
      case 25: /* cond ::= cond OR cond */
#line 173 "grammar.y"
{ yylhsminor.yy99 = NewConditionNode(yymsp[-2].minor.yy99, OR, yymsp[0].minor.yy99); }
#line 1136 "grammar.c"
  yymsp[-2].minor.yy99 = yylhsminor.yy99;
        break;
      case 26: /* op ::= EQ */
#line 177 "grammar.y"
{ yymsp[0].minor.yy111 = EQ; }
#line 1142 "grammar.c"
        break;
      case 27: /* op ::= GT */
#line 178 "grammar.y"
{ yymsp[0].minor.yy111 = GT; }
#line 1147 "grammar.c"
        break;
      case 28: /* op ::= LT */
#line 179 "grammar.y"
{ yymsp[0].minor.yy111 = LT; }
#line 1152 "grammar.c"
        break;
      case 29: /* op ::= LE */
#line 180 "grammar.y"
{ yymsp[0].minor.yy111 = LE; }
#line 1157 "grammar.c"
        break;
      case 30: /* op ::= GE */
#line 181 "grammar.y"
{ yymsp[0].minor.yy111 = GE; }
#line 1162 "grammar.c"
        break;
      case 31: /* op ::= NE */
#line 182 "grammar.y"
{ yymsp[0].minor.yy111 = NE; }
#line 1167 "grammar.c"
        break;
      case 32: /* value ::= INTEGER */
#line 188 "grammar.y"
{  yylhsminor.yy18 = SI_LongVal(yymsp[0].minor.yy0.intval); }
#line 1172 "grammar.c"
  yymsp[0].minor.yy18 = yylhsminor.yy18;
        break;
      case 33: /* value ::= STRING */
#line 189 "grammar.y"
{  yylhsminor.yy18 = SI_StringValC(strdup(yymsp[0].minor.yy0.strval)); }
#line 1178 "grammar.c"
  yymsp[0].minor.yy18 = yylhsminor.yy18;
        break;
      case 34: /* value ::= FLOAT */
#line 190 "grammar.y"
{  yylhsminor.yy18 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1184 "grammar.c"
  yymsp[0].minor.yy18 = yylhsminor.yy18;
        break;
      case 35: /* value ::= TRUE */
#line 191 "grammar.y"
{ yymsp[0].minor.yy18 = SI_BoolVal(1); }
#line 1190 "grammar.c"
        break;
      case 36: /* value ::= FALSE */
#line 192 "grammar.y"
{ yymsp[0].minor.yy18 = SI_BoolVal(0); }
#line 1195 "grammar.c"
        break;
      case 37: /* returnClause ::= RETURN returnElements */
#line 196 "grammar.y"
{
	yymsp[-1].minor.yy55 = NewReturnNode(yymsp[0].minor.yy54, 0);
}
#line 1202 "grammar.c"
        break;
      case 38: /* returnClause ::= RETURN DISTINCT returnElements */
#line 199 "grammar.y"
{
	yymsp[-2].minor.yy55 = NewReturnNode(yymsp[0].minor.yy54, 1);
}
#line 1209 "grammar.c"
        break;
      case 39: /* returnElements ::= returnElements COMMA returnElement */
#line 206 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy54, yymsp[0].minor.yy79);
	yylhsminor.yy54 = yymsp[-2].minor.yy54;
}
#line 1217 "grammar.c"
  yymsp[-2].minor.yy54 = yylhsminor.yy54;
        break;
      case 40: /* returnElements ::= returnElement */
#line 211 "grammar.y"
{
	yylhsminor.yy54 = NewVector(ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy54, yymsp[0].minor.yy79);
}
#line 1226 "grammar.c"
  yymsp[0].minor.yy54 = yylhsminor.yy54;
        break;
      case 41: /* returnElement ::= variable */
#line 218 "grammar.y"
{
	yylhsminor.yy79 = NewReturnElementNode(N_PROP, yymsp[0].minor.yy43, NULL, NULL);
}
#line 1234 "grammar.c"
  yymsp[0].minor.yy79 = yylhsminor.yy79;
        break;
      case 42: /* returnElement ::= variable AS STRING */
#line 221 "grammar.y"
{
	yylhsminor.yy79 = NewReturnElementNode(N_PROP, yymsp[-2].minor.yy43, NULL, yymsp[0].minor.yy0.strval);
}
#line 1242 "grammar.c"
  yymsp[-2].minor.yy79 = yylhsminor.yy79;
        break;
      case 43: /* returnElement ::= aggFunc */
#line 224 "grammar.y"
{
	yylhsminor.yy79 = yymsp[0].minor.yy79;
}
#line 1250 "grammar.c"
  yymsp[0].minor.yy79 = yylhsminor.yy79;
        break;
      case 44: /* returnElement ::= STRING */
#line 227 "grammar.y"
{
	yylhsminor.yy79 = NewReturnElementNode(N_NODE, NewVariable(yymsp[0].minor.yy0.strval, NULL), NULL, NULL);
}
#line 1258 "grammar.c"
  yymsp[0].minor.yy79 = yylhsminor.yy79;
        break;
      case 45: /* variable ::= STRING DOT STRING */
#line 233 "grammar.y"
{
	yylhsminor.yy43 = NewVariable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1266 "grammar.c"
  yymsp[-2].minor.yy43 = yylhsminor.yy43;
        break;
      case 46: /* aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS */
#line 239 "grammar.y"
{
	yylhsminor.yy79 = NewReturnElementNode(N_AGG_FUNC, yymsp[-1].minor.yy43, yymsp[-3].minor.yy0.strval, NULL);
}
#line 1274 "grammar.c"
  yymsp[-3].minor.yy79 = yylhsminor.yy79;
        break;
      case 47: /* aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS AS STRING */
#line 242 "grammar.y"
{
	yylhsminor.yy79 = NewReturnElementNode(N_AGG_FUNC, yymsp[-3].minor.yy43, yymsp[-5].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1282 "grammar.c"
  yymsp[-5].minor.yy79 = yylhsminor.yy79;
        break;
      case 48: /* orderClause ::= */
#line 248 "grammar.y"
{
	yymsp[1].minor.yy29 = NULL;
}
#line 1290 "grammar.c"
        break;
      case 49: /* orderClause ::= ORDER BY columnNameList */
#line 251 "grammar.y"
{
	yymsp[-2].minor.yy29 = NewOrderNode(yymsp[0].minor.yy54, ORDER_DIR_ASC);
}
#line 1297 "grammar.c"
        break;
      case 50: /* orderClause ::= ORDER BY columnNameList ASC */
#line 254 "grammar.y"
{
	yymsp[-3].minor.yy29 = NewOrderNode(yymsp[-1].minor.yy54, ORDER_DIR_ASC);
}
#line 1304 "grammar.c"
        break;
      case 51: /* orderClause ::= ORDER BY columnNameList DESC */
#line 257 "grammar.y"
{
	yymsp[-3].minor.yy29 = NewOrderNode(yymsp[-1].minor.yy54, ORDER_DIR_DESC);
}
#line 1311 "grammar.c"
        break;
      case 52: /* columnNameList ::= columnNameList COMMA columnName */
#line 262 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy54, yymsp[0].minor.yy33);
	yylhsminor.yy54 = yymsp[-2].minor.yy54;
}
#line 1319 "grammar.c"
  yymsp[-2].minor.yy54 = yylhsminor.yy54;
        break;
      case 53: /* columnNameList ::= columnName */
#line 266 "grammar.y"
{
	yylhsminor.yy54 = NewVector(ColumnNode*, 1);
	Vector_Push(yylhsminor.yy54, yymsp[0].minor.yy33);
}
#line 1328 "grammar.c"
  yymsp[0].minor.yy54 = yylhsminor.yy54;
        break;
      case 54: /* columnName ::= variable */
#line 272 "grammar.y"
{
	yylhsminor.yy33 = ColumnNodeFromVariable(yymsp[0].minor.yy43);
	FreeVariable(yymsp[0].minor.yy43);
}
#line 1337 "grammar.c"
  yymsp[0].minor.yy33 = yylhsminor.yy33;
        break;
      case 55: /* columnName ::= STRING */
#line 276 "grammar.y"
{
	yylhsminor.yy33 = ColumnNodeFromAlias(yymsp[0].minor.yy0.strval);
}
#line 1345 "grammar.c"
  yymsp[0].minor.yy33 = yylhsminor.yy33;
        break;
      case 56: /* limitClause ::= */
#line 282 "grammar.y"
{
	yymsp[1].minor.yy58 = NULL;
}
#line 1353 "grammar.c"
        break;
      case 57: /* limitClause ::= LIMIT INTEGER */
#line 285 "grammar.y"
{
	yymsp[-1].minor.yy58 = NewLimitNode(yymsp[0].minor.yy0.intval);
}
#line 1360 "grammar.c"
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
#line 20 "grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'\n", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 1426 "grammar.c"
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
#line 289 "grammar.y"


	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	QueryExpressionNode *Query_Parse(const char *q, size_t len, char **err) {
		yycolumn = 1;	// Reset lexer's token tracking position
		yy_scan_bytes(q, len);
  		void* pParser = ParseAlloc(malloc);
  		int t = 0;

		parseCtx ctx = {.root = NULL, .ok = 1, .errorMsg = NULL};

  		while( (t = yylex()) != 0) {
			Parse(pParser, t, tok, &ctx);
		}
		if (ctx.ok) {
			Parse(pParser, 0, tok, &ctx);
  		}
		ParseFree(pParser, free);
		if (err) {
			*err = ctx.errorMsg;
		}
		return ctx.root;
	}
#line 1661 "grammar.c"
