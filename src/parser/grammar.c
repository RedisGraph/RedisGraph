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
#define YYNOCODE 63
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_WhereNode* yy3;
  AST_QueryExpressionNode* yy6;
  AST_FilterNode* yy10;
  AST_NodeEntity* yy17;
  AST_ColumnNode* yy22;
  AST_LimitNode* yy63;
  int yy68;
  AST_Variable* yy69;
  AST_CreateNode * yy72;
  AST_LinkEntity* yy89;
  AST_ReturnElementNode* yy94;
  SIValue yy102;
  AST_OrderNode* yy108;
  AST_MatchNode* yy109;
  Vector* yy110;
  AST_ReturnNode* yy124;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             78
#define YYNRULE              64
#define YY_MAX_SHIFT         77
#define YY_MIN_SHIFTREDUCE   121
#define YY_MAX_SHIFTREDUCE   184
#define YY_MIN_REDUCE        185
#define YY_MAX_REDUCE        248
#define YY_ERROR_ACTION      249
#define YY_ACCEPT_ACTION     250
#define YY_NO_ACTION         251
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
#define YY_ACTTAB_COUNT (156)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   153,  154,  157,  155,  156,   77,  250,   37,   10,  124,
 /*    10 */    44,  166,   71,  170,  160,   72,  167,   71,  170,   22,
 /*    20 */   126,  158,   47,  159,  161,  162,  163,  159,  161,  162,
 /*    30 */   163,   65,  167,   71,  170,  181,    8,   39,  180,   20,
 /*    40 */    50,   21,   58,   15,   17,    6,    3,   29,   41,   29,
 /*    50 */    14,  126,   19,  126,  181,   32,  150,  179,   52,   13,
 /*    60 */   177,  178,   26,   64,   23,   29,    2,   73,   16,   46,
 /*    70 */    29,   15,   17,   41,   74,  149,   60,   12,   54,   31,
 /*    80 */    56,   61,   68,   29,   69,  127,   42,   38,  151,   40,
 /*    90 */    53,   59,   76,    3,   48,   17,   49,   75,   18,   51,
 /*   100 */     1,   34,   55,   57,  145,   63,   62,  148,   36,  122,
 /*   110 */    33,   43,    9,   45,    7,   35,  137,  140,   24,   25,
 /*   120 */    27,  141,  139,   11,  138,  136,  135,  133,  134,   28,
 /*   130 */   132,   67,  143,   30,    4,  174,  185,  169,  172,  187,
 /*   140 */   187,   73,  187,  187,  187,  187,   66,  187,  187,  187,
 /*   150 */   187,   70,  184,  187,  187,    5,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,    4,    5,    6,    7,   38,   39,   40,   55,   42,
 /*    10 */    12,   57,   58,   59,   12,   56,   57,   58,   59,   46,
 /*    20 */    47,   24,   49,   25,   26,   27,   28,   25,   26,   27,
 /*    30 */    28,   56,   57,   58,   59,   58,   10,   60,   61,   12,
 /*    40 */    13,   12,   13,    1,    2,    8,    9,   20,   12,   20,
 /*    50 */    46,   47,   46,   47,   58,   11,   14,   61,   13,   11,
 /*    60 */    34,   35,   15,   13,   17,   20,   30,   23,   11,   12,
 /*    70 */    20,    1,    2,   12,   12,   53,   53,   18,   50,   48,
 /*    80 */    50,   12,   58,   20,   12,   47,   54,   54,   54,   54,
 /*    90 */    50,   50,   36,    9,   51,    2,   50,   32,   22,   50,
 /*   100 */    29,   43,   51,   50,   52,   50,   52,   12,   41,   45,
 /*   110 */    44,   23,   12,   23,   10,   42,   15,   19,   12,   12,
 /*   120 */    12,   19,   19,   13,   19,   16,   14,   14,   14,   10,
 /*   130 */    14,   14,   21,   12,   10,   12,    0,   12,   12,   62,
 /*   140 */    62,   23,   62,   62,   62,   62,   31,   62,   62,   62,
 /*   150 */    62,   31,   25,   62,   62,   33,
};
#define YY_SHIFT_USE_DFLT (156)
#define YY_SHIFT_COUNT    (77)
#define YY_SHIFT_MIN      (-3)
#define YY_SHIFT_MAX      (136)
static const short yy_shift_ofst[] = {
 /*     0 */    37,   36,   61,   48,   61,   62,   48,   48,   62,   -3,
 /*    10 */    -2,    2,   27,   29,   47,   57,   57,   57,   57,   47,
 /*    20 */    45,   50,   47,   59,   63,   63,   59,   63,   69,   69,
 /*    30 */    63,   48,   72,   56,   65,   71,   84,   76,   42,   26,
 /*    40 */    70,   44,   93,   95,   88,  100,   90,  104,  101,   98,
 /*    50 */   106,  102,  107,  103,  105,  109,  112,  113,  108,  114,
 /*    60 */   119,  110,  111,  116,  121,  124,  123,  115,  117,  118,
 /*    70 */   125,  120,  124,  126,  118,  122,  127,  136,
};
#define YY_REDUCE_USE_DFLT (-48)
#define YY_REDUCE_COUNT (37)
#define YY_REDUCE_MIN   (-47)
#define YY_REDUCE_MAX   (73)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -33,  -41,  -25,  -27,  -46,  -23,    4,    6,   -4,  -47,
 /*    10 */    22,   23,   28,   30,   31,   32,   33,   34,   35,   31,
 /*    20 */    40,   41,   31,   43,   46,   49,   51,   53,   52,   54,
 /*    30 */    55,   38,   24,   64,   66,   58,   73,   67,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   192,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    10 */   249,  249,  206,  206,  189,  249,  249,  249,  249,  195,
 /*    20 */   206,  206,  194,  249,  206,  206,  249,  206,  249,  249,
 /*    30 */   206,  249,  249,  247,  239,  187,  192,  210,  249,  240,
 /*    40 */   211,  235,  216,  249,  224,  249,  249,  193,  249,  249,
 /*    50 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    60 */   208,  249,  249,  249,  249,  229,  249,  237,  249,  249,
 /*    70 */   249,  232,  228,  249,  246,  249,  249,  249,
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
  "MATCH",         "CREATE",        "COMMA",         "LEFT_PARENTHESIS",
  "STRING",        "COLON",         "RIGHT_PARENTHESIS",  "DASH",        
  "RIGHT_ARROW",   "LEFT_ARROW",    "LEFT_BRACKET",  "RIGHT_BRACKET",
  "LEFT_CURLY_BRACKET",  "RIGHT_CURLY_BRACKET",  "WHERE",         "DOT",         
  "NE",            "INTEGER",       "FLOAT",         "TRUE",        
  "FALSE",         "RETURN",        "DISTINCT",      "AS",          
  "ORDER",         "BY",            "ASC",           "DESC",        
  "LIMIT",         "error",         "expr",          "query",       
  "matchClause",   "whereClause",   "creatClause",   "returnClause",
  "orderClause",   "limitClause",   "chain",         "node",        
  "link",          "createExpression",  "properties",    "edge",        
  "mapLiteral",    "value",         "cond",          "op",          
  "returnElements",  "returnElement",  "variable",      "aggFunc",     
  "columnNameList",  "columnName",  
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause creatClause returnClause orderClause limitClause",
 /*   2 */ "expr ::= matchClause whereClause creatClause",
 /*   3 */ "expr ::= creatClause",
 /*   4 */ "matchClause ::= MATCH chain",
 /*   5 */ "chain ::= node",
 /*   6 */ "chain ::= chain link node",
 /*   7 */ "creatClause ::=",
 /*   8 */ "creatClause ::= CREATE createExpression",
 /*   9 */ "createExpression ::= chain",
 /*  10 */ "createExpression ::= createExpression COMMA chain",
 /*  11 */ "node ::= LEFT_PARENTHESIS STRING COLON STRING properties RIGHT_PARENTHESIS",
 /*  12 */ "node ::= LEFT_PARENTHESIS COLON STRING properties RIGHT_PARENTHESIS",
 /*  13 */ "node ::= LEFT_PARENTHESIS STRING properties RIGHT_PARENTHESIS",
 /*  14 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  15 */ "link ::= DASH edge RIGHT_ARROW",
 /*  16 */ "link ::= LEFT_ARROW edge DASH",
 /*  17 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  18 */ "edge ::= LEFT_BRACKET STRING properties RIGHT_BRACKET",
 /*  19 */ "edge ::= LEFT_BRACKET COLON STRING properties RIGHT_BRACKET",
 /*  20 */ "edge ::= LEFT_BRACKET STRING COLON STRING properties RIGHT_BRACKET",
 /*  21 */ "properties ::=",
 /*  22 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  23 */ "mapLiteral ::= STRING COLON value",
 /*  24 */ "mapLiteral ::= STRING COLON value COMMA mapLiteral",
 /*  25 */ "whereClause ::=",
 /*  26 */ "whereClause ::= WHERE cond",
 /*  27 */ "cond ::= STRING DOT STRING op STRING DOT STRING",
 /*  28 */ "cond ::= STRING DOT STRING op value",
 /*  29 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  30 */ "cond ::= cond AND cond",
 /*  31 */ "cond ::= cond OR cond",
 /*  32 */ "op ::= EQ",
 /*  33 */ "op ::= GT",
 /*  34 */ "op ::= LT",
 /*  35 */ "op ::= LE",
 /*  36 */ "op ::= GE",
 /*  37 */ "op ::= NE",
 /*  38 */ "value ::= INTEGER",
 /*  39 */ "value ::= STRING",
 /*  40 */ "value ::= FLOAT",
 /*  41 */ "value ::= TRUE",
 /*  42 */ "value ::= FALSE",
 /*  43 */ "returnClause ::= RETURN returnElements",
 /*  44 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  45 */ "returnElements ::= returnElements COMMA returnElement",
 /*  46 */ "returnElements ::= returnElement",
 /*  47 */ "returnElement ::= variable",
 /*  48 */ "returnElement ::= variable AS STRING",
 /*  49 */ "returnElement ::= aggFunc",
 /*  50 */ "returnElement ::= STRING",
 /*  51 */ "variable ::= STRING DOT STRING",
 /*  52 */ "aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS",
 /*  53 */ "aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS AS STRING",
 /*  54 */ "orderClause ::=",
 /*  55 */ "orderClause ::= ORDER BY columnNameList",
 /*  56 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  57 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  58 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  59 */ "columnNameList ::= columnName",
 /*  60 */ "columnName ::= variable",
 /*  61 */ "columnName ::= STRING",
 /*  62 */ "limitClause ::=",
 /*  63 */ "limitClause ::= LIMIT INTEGER",
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
    case 54: /* cond */
{
#line 194 "grammar.y"
 Free_AST_FilterNode((yypminor->yy10)); 
#line 585 "grammar.c"
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
  { 39, 1 },
  { 38, 6 },
  { 38, 3 },
  { 38, 1 },
  { 40, 2 },
  { 46, 1 },
  { 46, 3 },
  { 42, 0 },
  { 42, 2 },
  { 49, 1 },
  { 49, 3 },
  { 47, 6 },
  { 47, 5 },
  { 47, 4 },
  { 47, 3 },
  { 48, 3 },
  { 48, 3 },
  { 51, 3 },
  { 51, 4 },
  { 51, 5 },
  { 51, 6 },
  { 50, 0 },
  { 50, 3 },
  { 52, 3 },
  { 52, 5 },
  { 41, 0 },
  { 41, 2 },
  { 54, 7 },
  { 54, 5 },
  { 54, 3 },
  { 54, 3 },
  { 54, 3 },
  { 55, 1 },
  { 55, 1 },
  { 55, 1 },
  { 55, 1 },
  { 55, 1 },
  { 55, 1 },
  { 53, 1 },
  { 53, 1 },
  { 53, 1 },
  { 53, 1 },
  { 53, 1 },
  { 43, 2 },
  { 43, 3 },
  { 56, 3 },
  { 56, 1 },
  { 57, 1 },
  { 57, 3 },
  { 57, 1 },
  { 57, 1 },
  { 58, 3 },
  { 59, 4 },
  { 59, 6 },
  { 44, 0 },
  { 44, 3 },
  { 44, 4 },
  { 44, 4 },
  { 60, 3 },
  { 60, 1 },
  { 61, 1 },
  { 61, 1 },
  { 45, 0 },
  { 45, 2 },
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
#line 33 "grammar.y"
{ ctx->root = yymsp[0].minor.yy6; }
#line 957 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause creatClause returnClause orderClause limitClause */
#line 35 "grammar.y"
{
	yylhsminor.yy6 = New_AST_QueryExpressionNode(yymsp[-5].minor.yy109, yymsp[-4].minor.yy3, yymsp[-3].minor.yy72, yymsp[-2].minor.yy124, yymsp[-1].minor.yy108, yymsp[0].minor.yy63);
}
#line 964 "grammar.c"
  yymsp[-5].minor.yy6 = yylhsminor.yy6;
        break;
      case 2: /* expr ::= matchClause whereClause creatClause */
#line 39 "grammar.y"
{
	yylhsminor.yy6 = New_AST_QueryExpressionNode(yymsp[-2].minor.yy109, yymsp[-1].minor.yy3, yymsp[0].minor.yy72, NULL, NULL, NULL);
}
#line 972 "grammar.c"
  yymsp[-2].minor.yy6 = yylhsminor.yy6;
        break;
      case 3: /* expr ::= creatClause */
#line 43 "grammar.y"
{
	yylhsminor.yy6 = New_AST_QueryExpressionNode(NULL, NULL, yymsp[0].minor.yy72, NULL, NULL, NULL);
}
#line 980 "grammar.c"
  yymsp[0].minor.yy6 = yylhsminor.yy6;
        break;
      case 4: /* matchClause ::= MATCH chain */
#line 49 "grammar.y"
{
	yymsp[-1].minor.yy109 = New_AST_MatchNode(yymsp[0].minor.yy110);
}
#line 988 "grammar.c"
        break;
      case 5: /* chain ::= node */
#line 56 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy17);
}
#line 996 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 6: /* chain ::= chain link node */
#line 61 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[-1].minor.yy89);
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy17);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1006 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 7: /* creatClause ::= */
#line 70 "grammar.y"
{ 
	yymsp[1].minor.yy72 = NULL;
}
#line 1014 "grammar.c"
        break;
      case 8: /* creatClause ::= CREATE createExpression */
#line 74 "grammar.y"
{
	yymsp[-1].minor.yy72 = New_AST_CreateNode(yymsp[0].minor.yy110);
}
#line 1021 "grammar.c"
        break;
      case 9: /* createExpression ::= chain */
#line 80 "grammar.y"
{
	yylhsminor.yy110 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy110);
}
#line 1029 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 10: /* createExpression ::= createExpression COMMA chain */
#line 85 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy110);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1038 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 11: /* node ::= LEFT_PARENTHESIS STRING COLON STRING properties RIGHT_PARENTHESIS */
#line 94 "grammar.y"
{
	yymsp[-5].minor.yy17 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110);
}
#line 1046 "grammar.c"
        break;
      case 12: /* node ::= LEFT_PARENTHESIS COLON STRING properties RIGHT_PARENTHESIS */
#line 99 "grammar.y"
{
	yymsp[-4].minor.yy17 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110);
}
#line 1053 "grammar.c"
        break;
      case 13: /* node ::= LEFT_PARENTHESIS STRING properties RIGHT_PARENTHESIS */
#line 104 "grammar.y"
{
	yymsp[-3].minor.yy17 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy110);
}
#line 1060 "grammar.c"
        break;
      case 14: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 109 "grammar.y"
{
	yymsp[-2].minor.yy17 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy110);
}
#line 1067 "grammar.c"
        break;
      case 15: /* link ::= DASH edge RIGHT_ARROW */
#line 116 "grammar.y"
{
	yymsp[-2].minor.yy89 = yymsp[-1].minor.yy89;
	yymsp[-2].minor.yy89->direction = N_LEFT_TO_RIGHT;
}
#line 1075 "grammar.c"
        break;
      case 16: /* link ::= LEFT_ARROW edge DASH */
#line 122 "grammar.y"
{
	yymsp[-2].minor.yy89 = yymsp[-1].minor.yy89;
	yymsp[-2].minor.yy89->direction = N_RIGHT_TO_LEFT;
}
#line 1083 "grammar.c"
        break;
      case 17: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 129 "grammar.y"
{ 
	yymsp[-2].minor.yy89 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1090 "grammar.c"
        break;
      case 18: /* edge ::= LEFT_BRACKET STRING properties RIGHT_BRACKET */
#line 134 "grammar.y"
{ 
	yymsp[-3].minor.yy89 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1097 "grammar.c"
        break;
      case 19: /* edge ::= LEFT_BRACKET COLON STRING properties RIGHT_BRACKET */
#line 139 "grammar.y"
{ 
	yymsp[-4].minor.yy89 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1104 "grammar.c"
        break;
      case 20: /* edge ::= LEFT_BRACKET STRING COLON STRING properties RIGHT_BRACKET */
#line 144 "grammar.y"
{ 
	yymsp[-5].minor.yy89 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1111 "grammar.c"
        break;
      case 21: /* properties ::= */
#line 150 "grammar.y"
{
	yymsp[1].minor.yy110 = NULL;
}
#line 1118 "grammar.c"
        break;
      case 22: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 154 "grammar.y"
{
	yymsp[-2].minor.yy110 = yymsp[-1].minor.yy110;
}
#line 1125 "grammar.c"
        break;
      case 23: /* mapLiteral ::= STRING COLON value */
#line 159 "grammar.y"
{
	yylhsminor.yy110 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(yymsp[-2].minor.yy0.strval));
	Vector_Push(yylhsminor.yy110, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy102;
	Vector_Push(yylhsminor.yy110, val);
}
#line 1140 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 24: /* mapLiteral ::= STRING COLON value COMMA mapLiteral */
#line 171 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(yymsp[-4].minor.yy0.strval));
	Vector_Push(yymsp[0].minor.yy110, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy102;
	Vector_Push(yymsp[0].minor.yy110, val);
	
	yylhsminor.yy110 = yymsp[0].minor.yy110;
}
#line 1156 "grammar.c"
  yymsp[-4].minor.yy110 = yylhsminor.yy110;
        break;
      case 25: /* whereClause ::= */
#line 185 "grammar.y"
{ 
	yymsp[1].minor.yy3 = NULL;
}
#line 1164 "grammar.c"
        break;
      case 26: /* whereClause ::= WHERE cond */
#line 188 "grammar.y"
{
	yymsp[-1].minor.yy3 = New_AST_WhereNode(yymsp[0].minor.yy10);
}
#line 1171 "grammar.c"
        break;
      case 27: /* cond ::= STRING DOT STRING op STRING DOT STRING */
#line 196 "grammar.y"
{ yylhsminor.yy10 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy68, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1176 "grammar.c"
  yymsp[-6].minor.yy10 = yylhsminor.yy10;
        break;
      case 28: /* cond ::= STRING DOT STRING op value */
#line 197 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy68, yymsp[0].minor.yy102); }
#line 1182 "grammar.c"
  yymsp[-4].minor.yy10 = yylhsminor.yy10;
        break;
      case 29: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 198 "grammar.y"
{ yymsp[-2].minor.yy10 = yymsp[-1].minor.yy10; }
#line 1188 "grammar.c"
        break;
      case 30: /* cond ::= cond AND cond */
#line 199 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, AND, yymsp[0].minor.yy10); }
#line 1193 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 31: /* cond ::= cond OR cond */
#line 200 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, OR, yymsp[0].minor.yy10); }
#line 1199 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 32: /* op ::= EQ */
#line 204 "grammar.y"
{ yymsp[0].minor.yy68 = EQ; }
#line 1205 "grammar.c"
        break;
      case 33: /* op ::= GT */
#line 205 "grammar.y"
{ yymsp[0].minor.yy68 = GT; }
#line 1210 "grammar.c"
        break;
      case 34: /* op ::= LT */
#line 206 "grammar.y"
{ yymsp[0].minor.yy68 = LT; }
#line 1215 "grammar.c"
        break;
      case 35: /* op ::= LE */
#line 207 "grammar.y"
{ yymsp[0].minor.yy68 = LE; }
#line 1220 "grammar.c"
        break;
      case 36: /* op ::= GE */
#line 208 "grammar.y"
{ yymsp[0].minor.yy68 = GE; }
#line 1225 "grammar.c"
        break;
      case 37: /* op ::= NE */
#line 209 "grammar.y"
{ yymsp[0].minor.yy68 = NE; }
#line 1230 "grammar.c"
        break;
      case 38: /* value ::= INTEGER */
#line 215 "grammar.y"
{  yylhsminor.yy102 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1235 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 39: /* value ::= STRING */
#line 216 "grammar.y"
{  yylhsminor.yy102 = SI_StringValC(strdup(yymsp[0].minor.yy0.strval)); }
#line 1241 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 40: /* value ::= FLOAT */
#line 217 "grammar.y"
{  yylhsminor.yy102 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1247 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 41: /* value ::= TRUE */
#line 218 "grammar.y"
{ yymsp[0].minor.yy102 = SI_BoolVal(1); }
#line 1253 "grammar.c"
        break;
      case 42: /* value ::= FALSE */
#line 219 "grammar.y"
{ yymsp[0].minor.yy102 = SI_BoolVal(0); }
#line 1258 "grammar.c"
        break;
      case 43: /* returnClause ::= RETURN returnElements */
#line 223 "grammar.y"
{
	yymsp[-1].minor.yy124 = New_AST_ReturnNode(yymsp[0].minor.yy110, 0);
}
#line 1265 "grammar.c"
        break;
      case 44: /* returnClause ::= RETURN DISTINCT returnElements */
#line 226 "grammar.y"
{
	yymsp[-2].minor.yy124 = New_AST_ReturnNode(yymsp[0].minor.yy110, 1);
}
#line 1272 "grammar.c"
        break;
      case 45: /* returnElements ::= returnElements COMMA returnElement */
#line 233 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy94);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1280 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 46: /* returnElements ::= returnElement */
#line 238 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy94);
}
#line 1289 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 47: /* returnElement ::= variable */
#line 245 "grammar.y"
{
	yylhsminor.yy94 = New_AST_ReturnElementNode(N_PROP, yymsp[0].minor.yy69, NULL, NULL);
}
#line 1297 "grammar.c"
  yymsp[0].minor.yy94 = yylhsminor.yy94;
        break;
      case 48: /* returnElement ::= variable AS STRING */
#line 248 "grammar.y"
{
	yylhsminor.yy94 = New_AST_ReturnElementNode(N_PROP, yymsp[-2].minor.yy69, NULL, yymsp[0].minor.yy0.strval);
}
#line 1305 "grammar.c"
  yymsp[-2].minor.yy94 = yylhsminor.yy94;
        break;
      case 49: /* returnElement ::= aggFunc */
#line 251 "grammar.y"
{
	yylhsminor.yy94 = yymsp[0].minor.yy94;
}
#line 1313 "grammar.c"
  yymsp[0].minor.yy94 = yylhsminor.yy94;
        break;
      case 50: /* returnElement ::= STRING */
#line 254 "grammar.y"
{
	yylhsminor.yy94 = New_AST_ReturnElementNode(N_NODE, New_AST_Variable(yymsp[0].minor.yy0.strval, NULL), NULL, NULL);
}
#line 1321 "grammar.c"
  yymsp[0].minor.yy94 = yylhsminor.yy94;
        break;
      case 51: /* variable ::= STRING DOT STRING */
#line 260 "grammar.y"
{
	yylhsminor.yy69 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1329 "grammar.c"
  yymsp[-2].minor.yy69 = yylhsminor.yy69;
        break;
      case 52: /* aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS */
#line 266 "grammar.y"
{
	yylhsminor.yy94 = New_AST_ReturnElementNode(N_AGG_FUNC, yymsp[-1].minor.yy69, yymsp[-3].minor.yy0.strval, NULL);
}
#line 1337 "grammar.c"
  yymsp[-3].minor.yy94 = yylhsminor.yy94;
        break;
      case 53: /* aggFunc ::= STRING LEFT_PARENTHESIS variable RIGHT_PARENTHESIS AS STRING */
#line 269 "grammar.y"
{
	yylhsminor.yy94 = New_AST_ReturnElementNode(N_AGG_FUNC, yymsp[-3].minor.yy69, yymsp[-5].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1345 "grammar.c"
  yymsp[-5].minor.yy94 = yylhsminor.yy94;
        break;
      case 54: /* orderClause ::= */
#line 275 "grammar.y"
{
	yymsp[1].minor.yy108 = NULL;
}
#line 1353 "grammar.c"
        break;
      case 55: /* orderClause ::= ORDER BY columnNameList */
#line 278 "grammar.y"
{
	yymsp[-2].minor.yy108 = New_AST_OrderNode(yymsp[0].minor.yy110, ORDER_DIR_ASC);
}
#line 1360 "grammar.c"
        break;
      case 56: /* orderClause ::= ORDER BY columnNameList ASC */
#line 281 "grammar.y"
{
	yymsp[-3].minor.yy108 = New_AST_OrderNode(yymsp[-1].minor.yy110, ORDER_DIR_ASC);
}
#line 1367 "grammar.c"
        break;
      case 57: /* orderClause ::= ORDER BY columnNameList DESC */
#line 284 "grammar.y"
{
	yymsp[-3].minor.yy108 = New_AST_OrderNode(yymsp[-1].minor.yy110, ORDER_DIR_DESC);
}
#line 1374 "grammar.c"
        break;
      case 58: /* columnNameList ::= columnNameList COMMA columnName */
#line 289 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy22);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1382 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 59: /* columnNameList ::= columnName */
#line 293 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy22);
}
#line 1391 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 60: /* columnName ::= variable */
#line 299 "grammar.y"
{
	yylhsminor.yy22 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy69);
	Free_AST_Variable(yymsp[0].minor.yy69);
}
#line 1400 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 61: /* columnName ::= STRING */
#line 303 "grammar.y"
{
	yylhsminor.yy22 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy0.strval);
}
#line 1408 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 62: /* limitClause ::= */
#line 309 "grammar.y"
{
	yymsp[1].minor.yy63 = NULL;
}
#line 1416 "grammar.c"
        break;
      case 63: /* limitClause ::= LIMIT INTEGER */
#line 312 "grammar.y"
{
	yymsp[-1].minor.yy63 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1423 "grammar.c"
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
#line 1489 "grammar.c"
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
#line 316 "grammar.y"


	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	AST_QueryExpressionNode *Query_Parse(const char *q, size_t len, char **err) {
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
#line 1724 "grammar.c"
