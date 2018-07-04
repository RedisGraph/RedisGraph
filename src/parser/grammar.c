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
#line 9 "grammar.y"

	#include <stdlib.h>
	#include <stdio.h>
	#include <assert.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "./clauses/clauses.h"
	#include "parse.h"
	#include "../value.h"

	void yyerror(char *s);
#line 41 "grammar.c"
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
**    YYNTOKEN           Number of terminal symbols
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
**    YY_MIN_REDUCE      Minimum value for reduce actions
**    YY_MAX_REDUCE      Maximum value for reduce actions
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 75
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  int yy8;
  AST_OrderNode* yy12;
  AST_MatchNode* yy17;
  AST_NodeEntity* yy21;
  AST_ReturnElementNode* yy26;
  AST_DeleteNode * yy31;
  AST_ArithmeticExpressionNode* yy42;
  AST_WhereNode* yy59;
  AST_SetElement* yy68;
  AST_ReturnNode* yy72;
  AST_Variable* yy76;
  SIValue yy86;
  AST_ColumnNode* yy90;
  AST_FilterNode* yy102;
  AST_CreateNode* yy104;
  Vector* yy114;
  AST_LinkEntity* yy117;
  AST_Query* yy124;
  AST_SetNode* yy132;
  AST_LimitNode* yy139;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             99
#define YYNRULE              82
#define YYNTOKEN             43
#define YY_MAX_SHIFT         98
#define YY_MIN_SHIFTREDUCE   154
#define YY_MAX_SHIFTREDUCE   235
#define YY_ERROR_ACTION      236
#define YY_ACCEPT_ACTION     237
#define YY_NO_ACTION         238
#define YY_MIN_REDUCE        239
#define YY_MAX_REDUCE        320
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
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X.
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array.
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
#define YY_ACTTAB_COUNT (240)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   277,   98,  237,   49,   59,  245,   11,    9,    8,    7,
 /*    10 */   251,   71,  223,  224,  227,  225,  226,   61,   10,  294,
 /*    20 */    51,   95,   59,   11,    9,    8,    7,   37,  293,   48,
 /*    30 */     2,   93,  284,  242,   45,   61,   10,  229,   20,  231,
 /*    40 */   232,  234,  235,  228,   59,   34,   11,    9,    8,    7,
 /*    50 */   305,   92,  294,   51,   85,  229,   24,  231,  232,  234,
 /*    60 */   235,  293,   59,  202,   91,  284,  303,  294,   52,   15,
 /*    70 */    14,   79,   33,   13,   15,   66,  293,  229,   81,  231,
 /*    80 */   232,  234,  235,  294,   51,   63,   64,   72,  250,   71,
 /*    90 */    29,  253,  293,  294,   54,  229,  283,  231,  232,  234,
 /*   100 */   235,   55,  293,   42,  294,   53,   30,   60,   75,  294,
 /*   110 */   291,  294,  290,  293,   40,  279,  294,   62,  293,  305,
 /*   120 */   293,  294,   50,  294,   58,  293,   25,   27,   19,  173,
 /*   130 */   293,   90,  293,   77,   56,  304,   29,  253,   31,   40,
 /*   140 */    83,   32,  253,   68,   26,  193,   40,  216,  217,   78,
 /*   150 */    89,    3,   25,   27,    8,    7,   40,  230,    6,   57,
 /*   160 */   233,   94,   84,  207,   70,   23,   40,   73,   74,   76,
 /*   170 */    86,   80,   82,  273,   88,   87,  254,   96,  244,   43,
 /*   180 */    97,    1,   46,   44,   28,   47,  240,   12,   18,   27,
 /*   190 */    65,  191,   21,   67,  174,   69,    5,   17,  180,  178,
 /*   200 */   183,   35,  184,   36,   16,  182,  179,  181,  176,   38,
 /*   210 */   177,   39,   22,    4,   41,  175,  201,  186,  213,  239,
 /*   220 */   238,  238,  238,  238,  238,  238,  238,  238,   94,  238,
 /*   230 */   238,  238,  238,  238,  238,  238,  238,  238,  238,  222,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    66,   44,   45,   46,    4,   48,    3,    4,    5,    6,
 /*    10 */    56,   57,    7,    8,    9,   10,   11,   17,   18,   57,
 /*    20 */    58,   17,    4,    3,    4,    5,    6,    4,   66,   48,
 /*    30 */    30,   69,   70,   52,   53,   17,   18,   37,   68,   39,
 /*    40 */    40,   41,   42,   38,    4,   22,    3,    4,    5,    6,
 /*    50 */    57,   31,   57,   58,   66,   37,   18,   39,   40,   41,
 /*    60 */    42,   66,    4,   20,   69,   70,   73,   57,   58,   13,
 /*    70 */    14,   63,   16,   12,   13,   17,   66,   37,   63,   39,
 /*    80 */    40,   41,   42,   57,   58,   54,   67,   55,   56,   57,
 /*    90 */    59,   60,   66,   57,   58,   37,   70,   39,   40,   41,
 /*   100 */    42,   67,   66,   61,   57,   58,   17,   71,   19,   57,
 /*   110 */    58,   57,   58,   66,   25,   67,   57,   58,   66,   57,
 /*   120 */    66,   57,   58,   57,   58,   66,    1,    2,   15,   17,
 /*   130 */    66,   54,   66,   19,   72,   73,   59,   60,   17,   25,
 /*   140 */    19,   59,   60,   17,   18,   20,   25,   34,   35,   63,
 /*   150 */    19,   18,    1,    2,    5,    6,   25,   37,   15,   67,
 /*   160 */    40,   28,   63,   20,   62,   23,   25,   64,   63,   63,
 /*   170 */    17,   64,   63,   65,   63,   65,   60,   32,   51,   50,
 /*   180 */    36,   29,   50,   49,   27,   49,   51,   47,   15,    2,
 /*   190 */    28,   17,   17,   28,   17,   15,    7,   15,    4,   20,
 /*   200 */    24,   17,   24,   17,   33,   24,   21,   24,   20,   17,
 /*   210 */    20,   15,   19,   15,   17,   20,   17,   26,   17,    0,
 /*   220 */    74,   74,   74,   74,   74,   74,   74,   74,   28,   74,
 /*   230 */    74,   74,   74,   74,   74,   74,   74,   74,   74,   37,
 /*   240 */    74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
 /*   250 */    74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
 /*   260 */    74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
 /*   270 */    74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
 /*   280 */    74,   74,   74,
};
#define YY_SHIFT_COUNT    (98)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (219)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    61,    0,   18,   18,   18,   18,   18,   18,   18,   18,
 /*    10 */    18,   18,   56,   38,    4,   38,    4,    4,   38,    4,
 /*    20 */    58,    5,   40,   89,  121,  126,  126,  126,  126,   23,
 /*    30 */   114,  131,   23,  112,  142,  141,  141,  142,  141,  153,
 /*    40 */   153,  141,   38,  144,  145,  152,  144,  145,  152,  157,
 /*    50 */    43,   20,    3,    3,    3,  125,  113,  151,  149,  120,
 /*    60 */   143,  133,  149,  173,  187,  174,  162,  175,  165,  177,
 /*    70 */   180,  189,  182,  194,  176,  184,  178,  186,  181,  183,
 /*    80 */   185,  179,  188,  192,  190,  196,  193,  191,  195,  197,
 /*    90 */   173,  198,  199,  198,  201,  200,  171,  202,  219,
};
#define YY_REDUCE_COUNT (49)
#define YY_REDUCE_MIN   (-66)
#define YY_REDUCE_MAX   (140)
static const short yy_reduce_ofst[] = {
 /*     0 */   -43,  -38,   -5,   36,   26,   10,   47,   52,   54,   59,
 /*    10 */    64,   66,  -19,   31,   32,   77,   62,  -46,   82,   -7,
 /*    20 */   -66,  -30,  -12,    8,   15,   19,   34,   48,   92,   42,
 /*    30 */    86,   99,   42,  102,  103,  105,  106,  107,  109,  108,
 /*    40 */   110,  111,  116,  127,  129,  134,  135,  132,  136,  140,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   247,  236,  236,  236,  236,  236,  236,  236,  236,  236,
 /*    10 */   236,  236,  247,  236,  236,  236,  236,  236,  236,  236,
 /*    20 */   236,  236,  236,  270,  270,  236,  236,  236,  236,  255,
 /*    30 */   270,  270,  256,  236,  236,  270,  270,  236,  270,  236,
 /*    40 */   236,  270,  236,  306,  299,  243,  306,  299,  241,  274,
 /*    50 */   236,  285,  252,  295,  296,  236,  300,  275,  288,  236,
 /*    60 */   236,  297,  289,  246,  280,  236,  236,  236,  236,  236,
 /*    70 */   257,  236,  249,  236,  236,  236,  236,  236,  236,  236,
 /*    80 */   236,  236,  236,  236,  236,  272,  236,  236,  236,  236,
 /*    90 */   248,  282,  236,  281,  236,  297,  236,  236,  236,
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
  yyStackEntry *yystackEnd;            /* Last entry in the stack */
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

#if defined(YYCOVERAGE) || !defined(NDEBUG)
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  /*    0 */ "$",
  /*    1 */ "OR",
  /*    2 */ "AND",
  /*    3 */ "ADD",
  /*    4 */ "DASH",
  /*    5 */ "MUL",
  /*    6 */ "DIV",
  /*    7 */ "EQ",
  /*    8 */ "GT",
  /*    9 */ "GE",
  /*   10 */ "LT",
  /*   11 */ "LE",
  /*   12 */ "MATCH",
  /*   13 */ "CREATE",
  /*   14 */ "SET",
  /*   15 */ "COMMA",
  /*   16 */ "DELETE",
  /*   17 */ "UQSTRING",
  /*   18 */ "LEFT_PARENTHESIS",
  /*   19 */ "COLON",
  /*   20 */ "RIGHT_PARENTHESIS",
  /*   21 */ "RIGHT_ARROW",
  /*   22 */ "LEFT_ARROW",
  /*   23 */ "LEFT_BRACKET",
  /*   24 */ "RIGHT_BRACKET",
  /*   25 */ "LEFT_CURLY_BRACKET",
  /*   26 */ "RIGHT_CURLY_BRACKET",
  /*   27 */ "WHERE",
  /*   28 */ "DOT",
  /*   29 */ "RETURN",
  /*   30 */ "DISTINCT",
  /*   31 */ "AS",
  /*   32 */ "ORDER",
  /*   33 */ "BY",
  /*   34 */ "ASC",
  /*   35 */ "DESC",
  /*   36 */ "LIMIT",
  /*   37 */ "INTEGER",
  /*   38 */ "NE",
  /*   39 */ "STRING",
  /*   40 */ "FLOAT",
  /*   41 */ "TRUE",
  /*   42 */ "FALSE",
  /*   43 */ "error",
  /*   44 */ "expr",
  /*   45 */ "query",
  /*   46 */ "matchClause",
  /*   47 */ "whereClause",
  /*   48 */ "createClause",
  /*   49 */ "returnClause",
  /*   50 */ "orderClause",
  /*   51 */ "limitClause",
  /*   52 */ "deleteClause",
  /*   53 */ "setClause",
  /*   54 */ "chains",
  /*   55 */ "setList",
  /*   56 */ "setElement",
  /*   57 */ "variable",
  /*   58 */ "arithmetic_expression",
  /*   59 */ "chain",
  /*   60 */ "node",
  /*   61 */ "link",
  /*   62 */ "deleteExpression",
  /*   63 */ "properties",
  /*   64 */ "edge",
  /*   65 */ "mapLiteral",
  /*   66 */ "value",
  /*   67 */ "cond",
  /*   68 */ "relation",
  /*   69 */ "returnElements",
  /*   70 */ "returnElement",
  /*   71 */ "arithmetic_expression_list",
  /*   72 */ "columnNameList",
  /*   73 */ "columnName",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause createClause returnClause orderClause limitClause",
 /*   2 */ "expr ::= matchClause whereClause createClause",
 /*   3 */ "expr ::= matchClause whereClause deleteClause",
 /*   4 */ "expr ::= matchClause whereClause setClause",
 /*   5 */ "expr ::= matchClause whereClause setClause returnClause orderClause limitClause",
 /*   6 */ "expr ::= createClause",
 /*   7 */ "matchClause ::= MATCH chains",
 /*   8 */ "createClause ::=",
 /*   9 */ "createClause ::= CREATE chains",
 /*  10 */ "setClause ::= SET setList",
 /*  11 */ "setList ::= setElement",
 /*  12 */ "setList ::= setList COMMA setElement",
 /*  13 */ "setElement ::= variable EQ arithmetic_expression",
 /*  14 */ "chain ::= node",
 /*  15 */ "chain ::= chain link node",
 /*  16 */ "chains ::= chain",
 /*  17 */ "chains ::= chains COMMA chain",
 /*  18 */ "deleteClause ::= DELETE deleteExpression",
 /*  19 */ "deleteExpression ::= UQSTRING",
 /*  20 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  21 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  22 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  23 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  24 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  25 */ "link ::= DASH edge RIGHT_ARROW",
 /*  26 */ "link ::= LEFT_ARROW edge DASH",
 /*  27 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  28 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  29 */ "edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET",
 /*  30 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  31 */ "properties ::=",
 /*  32 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  33 */ "mapLiteral ::= UQSTRING COLON value",
 /*  34 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  35 */ "whereClause ::=",
 /*  36 */ "whereClause ::= WHERE cond",
 /*  37 */ "cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING",
 /*  38 */ "cond ::= UQSTRING DOT UQSTRING relation value",
 /*  39 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  40 */ "cond ::= cond AND cond",
 /*  41 */ "cond ::= cond OR cond",
 /*  42 */ "returnClause ::= RETURN returnElements",
 /*  43 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  44 */ "returnElements ::= returnElements COMMA returnElement",
 /*  45 */ "returnElements ::= returnElement",
 /*  46 */ "returnElement ::= arithmetic_expression",
 /*  47 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  48 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  49 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  50 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  51 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  52 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  53 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  54 */ "arithmetic_expression ::= value",
 /*  55 */ "arithmetic_expression ::= variable",
 /*  56 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  57 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  58 */ "variable ::= UQSTRING",
 /*  59 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  60 */ "orderClause ::=",
 /*  61 */ "orderClause ::= ORDER BY columnNameList",
 /*  62 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  63 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  64 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  65 */ "columnNameList ::= columnName",
 /*  66 */ "columnName ::= variable",
 /*  67 */ "limitClause ::=",
 /*  68 */ "limitClause ::= LIMIT INTEGER",
 /*  69 */ "relation ::= EQ",
 /*  70 */ "relation ::= GT",
 /*  71 */ "relation ::= LT",
 /*  72 */ "relation ::= LE",
 /*  73 */ "relation ::= GE",
 /*  74 */ "relation ::= NE",
 /*  75 */ "value ::= INTEGER",
 /*  76 */ "value ::= DASH INTEGER",
 /*  77 */ "value ::= STRING",
 /*  78 */ "value ::= FLOAT",
 /*  79 */ "value ::= DASH FLOAT",
 /*  80 */ "value ::= TRUE",
 /*  81 */ "value ::= FALSE",
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

/* Initialize a new parser that has already been allocated.
*/
void ParseInit(void *yypParser){
  yyParser *pParser = (yyParser*)yypParser;
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
#if YYSTACKDEPTH>0
  pParser->yystackEnd = &pParser->yystack[YYSTACKDEPTH-1];
#endif
}

#ifndef Parse_ENGINEALWAYSONSTACK
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
  if( pParser ) ParseInit(pParser);
  return pParser;
}
#endif /* Parse_ENGINEALWAYSONSTACK */


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
    case 67: /* cond */
{
#line 250 "grammar.y"
 Free_AST_FilterNode((yypminor->yy102)); 
#line 699 "grammar.c"
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
** Clear all secondary memory allocations from the parser
*/
void ParseFinalize(void *p){
  yyParser *pParser = (yyParser*)p;
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
}

#ifndef Parse_ENGINEALWAYSONSTACK
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
#ifndef YYPARSEFREENEVERNULL
  if( p==0 ) return;
#endif
  ParseFinalize(p);
  (*freeProc)(p);
}
#endif /* Parse_ENGINEALWAYSONSTACK */

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/* This array of booleans keeps track of the parser statement
** coverage.  The element yycoverage[X][Y] is set when the parser
** is in state X and has a lookahead token Y.  In a well-tested
** systems, every element of this matrix should end up being set.
*/
#if defined(YYCOVERAGE)
static unsigned char yycoverage[YYNSTATE][YYNTOKEN];
#endif

/*
** Write into out a description of every state/lookahead combination that
**
**   (1)  has not been used by the parser, and
**   (2)  is not a syntax error.
**
** Return the number of missed state/lookahead combinations.
*/
#if defined(YYCOVERAGE)
int ParseCoverage(FILE *out){
  int stateno, iLookAhead, i;
  int nMissed = 0;
  for(stateno=0; stateno<YYNSTATE; stateno++){
    i = yy_shift_ofst[stateno];
    for(iLookAhead=0; iLookAhead<YYNTOKEN; iLookAhead++){
      if( yy_lookahead[i+iLookAhead]!=iLookAhead ) continue;
      if( yycoverage[stateno][iLookAhead]==0 ) nMissed++;
      if( out ){
        fprintf(out,"State %d lookahead %s %s\n", stateno,
                yyTokenName[iLookAhead],
                yycoverage[stateno][iLookAhead] ? "ok" : "missed");
      }
    }
  }
  return nMissed;
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
 
  if( stateno>YY_MAX_SHIFT ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
#if defined(YYCOVERAGE)
  yycoverage[stateno][iLookAhead] = 1;
#endif
  do{
    i = yy_shift_ofst[stateno];
    assert( i>=0 );
    assert( i+YYNTOKEN<=(int)sizeof(yy_lookahead)/sizeof(yy_lookahead[0]) );
    assert( iLookAhead!=YYNOCODE );
    assert( iLookAhead < YYNTOKEN );
    i += iLookAhead;
    if( yy_lookahead[i]!=iLookAhead ){
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
static void yyTraceShift(yyParser *yypParser, int yyNewState, const char *zTag){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%s%s '%s', go to state %d\n",
         yyTracePrompt, zTag, yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%s%s '%s', pending reduce %d\n",
         yyTracePrompt, zTag, yyTokenName[yypParser->yytos->major],
         yyNewState - YY_MIN_REDUCE);
    }
  }
}
#else
# define yyTraceShift(X,Y,Z)
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
  if( yypParser->yytos>yypParser->yystackEnd ){
    yypParser->yytos--;
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yypParser->yytos--;
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
  yyTraceShift(yypParser, yyNewState, "Shift");
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;       /* Symbol on the left-hand side of the rule */
  signed char nrhs;     /* Negative of the number of RHS symbols in the rule */
} yyRuleInfo[] = {
  {   45,   -1 }, /* (0) query ::= expr */
  {   44,   -6 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
  {   44,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   44,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   44,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   44,   -6 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
  {   44,   -1 }, /* (6) expr ::= createClause */
  {   46,   -2 }, /* (7) matchClause ::= MATCH chains */
  {   48,    0 }, /* (8) createClause ::= */
  {   48,   -2 }, /* (9) createClause ::= CREATE chains */
  {   53,   -2 }, /* (10) setClause ::= SET setList */
  {   55,   -1 }, /* (11) setList ::= setElement */
  {   55,   -3 }, /* (12) setList ::= setList COMMA setElement */
  {   56,   -3 }, /* (13) setElement ::= variable EQ arithmetic_expression */
  {   59,   -1 }, /* (14) chain ::= node */
  {   59,   -3 }, /* (15) chain ::= chain link node */
  {   54,   -1 }, /* (16) chains ::= chain */
  {   54,   -3 }, /* (17) chains ::= chains COMMA chain */
  {   52,   -2 }, /* (18) deleteClause ::= DELETE deleteExpression */
  {   62,   -1 }, /* (19) deleteExpression ::= UQSTRING */
  {   62,   -3 }, /* (20) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   60,   -6 }, /* (21) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   60,   -5 }, /* (22) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   60,   -4 }, /* (23) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   60,   -3 }, /* (24) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   61,   -3 }, /* (25) link ::= DASH edge RIGHT_ARROW */
  {   61,   -3 }, /* (26) link ::= LEFT_ARROW edge DASH */
  {   64,   -3 }, /* (27) edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
  {   64,   -4 }, /* (28) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   64,   -5 }, /* (29) edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
  {   64,   -6 }, /* (30) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   63,    0 }, /* (31) properties ::= */
  {   63,   -3 }, /* (32) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   65,   -3 }, /* (33) mapLiteral ::= UQSTRING COLON value */
  {   65,   -5 }, /* (34) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   47,    0 }, /* (35) whereClause ::= */
  {   47,   -2 }, /* (36) whereClause ::= WHERE cond */
  {   67,   -7 }, /* (37) cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
  {   67,   -5 }, /* (38) cond ::= UQSTRING DOT UQSTRING relation value */
  {   67,   -3 }, /* (39) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   67,   -3 }, /* (40) cond ::= cond AND cond */
  {   67,   -3 }, /* (41) cond ::= cond OR cond */
  {   49,   -2 }, /* (42) returnClause ::= RETURN returnElements */
  {   49,   -3 }, /* (43) returnClause ::= RETURN DISTINCT returnElements */
  {   69,   -3 }, /* (44) returnElements ::= returnElements COMMA returnElement */
  {   69,   -1 }, /* (45) returnElements ::= returnElement */
  {   70,   -1 }, /* (46) returnElement ::= arithmetic_expression */
  {   70,   -3 }, /* (47) returnElement ::= arithmetic_expression AS UQSTRING */
  {   58,   -3 }, /* (48) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   58,   -3 }, /* (49) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   58,   -3 }, /* (50) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   58,   -3 }, /* (51) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   58,   -3 }, /* (52) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   58,   -4 }, /* (53) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   58,   -1 }, /* (54) arithmetic_expression ::= value */
  {   58,   -1 }, /* (55) arithmetic_expression ::= variable */
  {   71,   -3 }, /* (56) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   71,   -1 }, /* (57) arithmetic_expression_list ::= arithmetic_expression */
  {   57,   -1 }, /* (58) variable ::= UQSTRING */
  {   57,   -3 }, /* (59) variable ::= UQSTRING DOT UQSTRING */
  {   50,    0 }, /* (60) orderClause ::= */
  {   50,   -3 }, /* (61) orderClause ::= ORDER BY columnNameList */
  {   50,   -4 }, /* (62) orderClause ::= ORDER BY columnNameList ASC */
  {   50,   -4 }, /* (63) orderClause ::= ORDER BY columnNameList DESC */
  {   72,   -3 }, /* (64) columnNameList ::= columnNameList COMMA columnName */
  {   72,   -1 }, /* (65) columnNameList ::= columnName */
  {   73,   -1 }, /* (66) columnName ::= variable */
  {   51,    0 }, /* (67) limitClause ::= */
  {   51,   -2 }, /* (68) limitClause ::= LIMIT INTEGER */
  {   68,   -1 }, /* (69) relation ::= EQ */
  {   68,   -1 }, /* (70) relation ::= GT */
  {   68,   -1 }, /* (71) relation ::= LT */
  {   68,   -1 }, /* (72) relation ::= LE */
  {   68,   -1 }, /* (73) relation ::= GE */
  {   68,   -1 }, /* (74) relation ::= NE */
  {   66,   -1 }, /* (75) value ::= INTEGER */
  {   66,   -2 }, /* (76) value ::= DASH INTEGER */
  {   66,   -1 }, /* (77) value ::= STRING */
  {   66,   -1 }, /* (78) value ::= FLOAT */
  {   66,   -2 }, /* (79) value ::= DASH FLOAT */
  {   66,   -1 }, /* (80) value ::= TRUE */
  {   66,   -1 }, /* (81) value ::= FALSE */
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
**
** The yyLookahead and yyLookaheadToken parameters provide reduce actions
** access to the lookahead token (if any).  The yyLookahead will be YYNOCODE
** if the lookahead token has already been consumed.  As this procedure is
** only called from one place, optimizing compilers will in-line it, which
** means that the extra parameters have no performance impact.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno,       /* Number of the rule by which to reduce */
  int yyLookahead,             /* Lookahead token, or YYNOCODE if none */
  ParseTOKENTYPE yyLookaheadToken  /* Value of the lookahead token */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  (void)yyLookahead;
  (void)yyLookaheadToken;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    if( yysize ){
      fprintf(yyTraceFILE, "%sReduce %d [%s], go to state %d.\n",
        yyTracePrompt,
        yyruleno, yyRuleName[yyruleno], yymsp[yysize].stateno);
    }else{
      fprintf(yyTraceFILE, "%sReduce %d [%s].\n",
        yyTracePrompt, yyruleno, yyRuleName[yyruleno]);
    }
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
    if( yypParser->yytos>=yypParser->yystackEnd ){
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
#line 36 "grammar.y"
{ ctx->root = yymsp[0].minor.yy124; }
#line 1158 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
#line 38 "grammar.y"
{
	yylhsminor.yy124 = New_AST_Query(yymsp[-5].minor.yy17, yymsp[-4].minor.yy59, yymsp[-3].minor.yy104, NULL, NULL, yymsp[-2].minor.yy72, yymsp[-1].minor.yy12, yymsp[0].minor.yy139);
}
#line 1165 "grammar.c"
  yymsp[-5].minor.yy124 = yylhsminor.yy124;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 42 "grammar.y"
{
	yylhsminor.yy124 = New_AST_Query(yymsp[-2].minor.yy17, yymsp[-1].minor.yy59, yymsp[0].minor.yy104, NULL, NULL, NULL, NULL, NULL);
}
#line 1173 "grammar.c"
  yymsp[-2].minor.yy124 = yylhsminor.yy124;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 46 "grammar.y"
{
	yylhsminor.yy124 = New_AST_Query(yymsp[-2].minor.yy17, yymsp[-1].minor.yy59, NULL, NULL, yymsp[0].minor.yy31, NULL, NULL, NULL);
}
#line 1181 "grammar.c"
  yymsp[-2].minor.yy124 = yylhsminor.yy124;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 50 "grammar.y"
{
	yylhsminor.yy124 = New_AST_Query(yymsp[-2].minor.yy17, yymsp[-1].minor.yy59, NULL, yymsp[0].minor.yy132, NULL, NULL, NULL, NULL);
}
#line 1189 "grammar.c"
  yymsp[-2].minor.yy124 = yylhsminor.yy124;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
#line 54 "grammar.y"
{
	yylhsminor.yy124 = New_AST_Query(yymsp[-5].minor.yy17, yymsp[-4].minor.yy59, NULL, yymsp[-3].minor.yy132, NULL, yymsp[-2].minor.yy72, yymsp[-1].minor.yy12, yymsp[0].minor.yy139);
}
#line 1197 "grammar.c"
  yymsp[-5].minor.yy124 = yylhsminor.yy124;
        break;
      case 6: /* expr ::= createClause */
#line 58 "grammar.y"
{
	yylhsminor.yy124 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy104, NULL, NULL, NULL, NULL, NULL);
}
#line 1205 "grammar.c"
  yymsp[0].minor.yy124 = yylhsminor.yy124;
        break;
      case 7: /* matchClause ::= MATCH chains */
#line 64 "grammar.y"
{
	yymsp[-1].minor.yy17 = New_AST_MatchNode(yymsp[0].minor.yy114);
}
#line 1213 "grammar.c"
        break;
      case 8: /* createClause ::= */
#line 71 "grammar.y"
{
	yymsp[1].minor.yy104 = NULL;
}
#line 1220 "grammar.c"
        break;
      case 9: /* createClause ::= CREATE chains */
#line 75 "grammar.y"
{
	yymsp[-1].minor.yy104 = New_AST_CreateNode(yymsp[0].minor.yy114);
}
#line 1227 "grammar.c"
        break;
      case 10: /* setClause ::= SET setList */
#line 80 "grammar.y"
{
	yymsp[-1].minor.yy132 = New_AST_SetNode(yymsp[0].minor.yy114);
}
#line 1234 "grammar.c"
        break;
      case 11: /* setList ::= setElement */
#line 85 "grammar.y"
{
	yylhsminor.yy114 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy114, yymsp[0].minor.yy68);
}
#line 1242 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 12: /* setList ::= setList COMMA setElement */
#line 89 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy114, yymsp[0].minor.yy68);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1251 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 13: /* setElement ::= variable EQ arithmetic_expression */
#line 95 "grammar.y"
{
	yylhsminor.yy68 = New_AST_SetElement(yymsp[-2].minor.yy76, yymsp[0].minor.yy42);
}
#line 1259 "grammar.c"
  yymsp[-2].minor.yy68 = yylhsminor.yy68;
        break;
      case 14: /* chain ::= node */
#line 101 "grammar.y"
{
	yylhsminor.yy114 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy114, yymsp[0].minor.yy21);
}
#line 1268 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 15: /* chain ::= chain link node */
#line 106 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy114, yymsp[-1].minor.yy117);
	Vector_Push(yymsp[-2].minor.yy114, yymsp[0].minor.yy21);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1278 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 16: /* chains ::= chain */
#line 113 "grammar.y"
{
	yylhsminor.yy114 = yymsp[0].minor.yy114;
}
#line 1286 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 17: /* chains ::= chains COMMA chain */
#line 117 "grammar.y"
{
	for(int i = 0; i < Vector_Size(yymsp[0].minor.yy114); i++) {
		AST_GraphEntity *entity;
		Vector_Get(yymsp[0].minor.yy114, i, &entity);
		Vector_Push(yymsp[-2].minor.yy114, entity);
	}
	Vector_Free(yymsp[0].minor.yy114);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1300 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 18: /* deleteClause ::= DELETE deleteExpression */
#line 130 "grammar.y"
{
	yymsp[-1].minor.yy31 = New_AST_DeleteNode(yymsp[0].minor.yy114);
}
#line 1308 "grammar.c"
        break;
      case 19: /* deleteExpression ::= UQSTRING */
#line 136 "grammar.y"
{
	yylhsminor.yy114 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy114, yymsp[0].minor.yy0.strval);
}
#line 1316 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 20: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 141 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy114, yymsp[0].minor.yy0.strval);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1325 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 21: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 149 "grammar.y"
{
	yymsp[-5].minor.yy21 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy114);
}
#line 1333 "grammar.c"
        break;
      case 22: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 154 "grammar.y"
{
	yymsp[-4].minor.yy21 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy114);
}
#line 1340 "grammar.c"
        break;
      case 23: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 159 "grammar.y"
{
	yymsp[-3].minor.yy21 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy114);
}
#line 1347 "grammar.c"
        break;
      case 24: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 164 "grammar.y"
{
	yymsp[-2].minor.yy21 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy114);
}
#line 1354 "grammar.c"
        break;
      case 25: /* link ::= DASH edge RIGHT_ARROW */
#line 171 "grammar.y"
{
	yymsp[-2].minor.yy117 = yymsp[-1].minor.yy117;
	yymsp[-2].minor.yy117->direction = N_LEFT_TO_RIGHT;
}
#line 1362 "grammar.c"
        break;
      case 26: /* link ::= LEFT_ARROW edge DASH */
#line 177 "grammar.y"
{
	yymsp[-2].minor.yy117 = yymsp[-1].minor.yy117;
	yymsp[-2].minor.yy117->direction = N_RIGHT_TO_LEFT;
}
#line 1370 "grammar.c"
        break;
      case 27: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 184 "grammar.y"
{ 
	yymsp[-2].minor.yy117 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy114, N_DIR_UNKNOWN);
}
#line 1377 "grammar.c"
        break;
      case 28: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 189 "grammar.y"
{ 
	yymsp[-3].minor.yy117 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy114, N_DIR_UNKNOWN);
}
#line 1384 "grammar.c"
        break;
      case 29: /* edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
#line 194 "grammar.y"
{ 
	yymsp[-4].minor.yy117 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy114, N_DIR_UNKNOWN);
}
#line 1391 "grammar.c"
        break;
      case 30: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 199 "grammar.y"
{ 
	yymsp[-5].minor.yy117 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy114, N_DIR_UNKNOWN);
}
#line 1398 "grammar.c"
        break;
      case 31: /* properties ::= */
#line 205 "grammar.y"
{
	yymsp[1].minor.yy114 = NULL;
}
#line 1405 "grammar.c"
        break;
      case 32: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 209 "grammar.y"
{
	yymsp[-2].minor.yy114 = yymsp[-1].minor.yy114;
}
#line 1412 "grammar.c"
        break;
      case 33: /* mapLiteral ::= UQSTRING COLON value */
#line 215 "grammar.y"
{
	yylhsminor.yy114 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy114, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy86;
	Vector_Push(yylhsminor.yy114, val);
}
#line 1427 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 34: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 227 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy114, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy86;
	Vector_Push(yymsp[0].minor.yy114, val);
	
	yylhsminor.yy114 = yymsp[0].minor.yy114;
}
#line 1443 "grammar.c"
  yymsp[-4].minor.yy114 = yylhsminor.yy114;
        break;
      case 35: /* whereClause ::= */
#line 241 "grammar.y"
{ 
	yymsp[1].minor.yy59 = NULL;
}
#line 1451 "grammar.c"
        break;
      case 36: /* whereClause ::= WHERE cond */
#line 244 "grammar.y"
{
	yymsp[-1].minor.yy59 = New_AST_WhereNode(yymsp[0].minor.yy102);
}
#line 1458 "grammar.c"
        break;
      case 37: /* cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
#line 253 "grammar.y"
{ yylhsminor.yy102 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy8, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1463 "grammar.c"
  yymsp[-6].minor.yy102 = yylhsminor.yy102;
        break;
      case 38: /* cond ::= UQSTRING DOT UQSTRING relation value */
#line 256 "grammar.y"
{ yylhsminor.yy102 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy8, yymsp[0].minor.yy86); }
#line 1469 "grammar.c"
  yymsp[-4].minor.yy102 = yylhsminor.yy102;
        break;
      case 39: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 257 "grammar.y"
{ yymsp[-2].minor.yy102 = yymsp[-1].minor.yy102; }
#line 1475 "grammar.c"
        break;
      case 40: /* cond ::= cond AND cond */
#line 258 "grammar.y"
{ yylhsminor.yy102 = New_AST_ConditionNode(yymsp[-2].minor.yy102, AND, yymsp[0].minor.yy102); }
#line 1480 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 41: /* cond ::= cond OR cond */
#line 259 "grammar.y"
{ yylhsminor.yy102 = New_AST_ConditionNode(yymsp[-2].minor.yy102, OR, yymsp[0].minor.yy102); }
#line 1486 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 42: /* returnClause ::= RETURN returnElements */
#line 264 "grammar.y"
{
	yymsp[-1].minor.yy72 = New_AST_ReturnNode(yymsp[0].minor.yy114, 0);
}
#line 1494 "grammar.c"
        break;
      case 43: /* returnClause ::= RETURN DISTINCT returnElements */
#line 267 "grammar.y"
{
	yymsp[-2].minor.yy72 = New_AST_ReturnNode(yymsp[0].minor.yy114, 1);
}
#line 1501 "grammar.c"
        break;
      case 44: /* returnElements ::= returnElements COMMA returnElement */
#line 274 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy114, yymsp[0].minor.yy26);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1509 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 45: /* returnElements ::= returnElement */
#line 279 "grammar.y"
{
	yylhsminor.yy114 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy114, yymsp[0].minor.yy26);
}
#line 1518 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 46: /* returnElement ::= arithmetic_expression */
#line 286 "grammar.y"
{
	yylhsminor.yy26 = New_AST_ReturnElementNode(yymsp[0].minor.yy42, NULL);
}
#line 1526 "grammar.c"
  yymsp[0].minor.yy26 = yylhsminor.yy26;
        break;
      case 47: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 291 "grammar.y"
{
	yylhsminor.yy26 = New_AST_ReturnElementNode(yymsp[-2].minor.yy42, yymsp[0].minor.yy0.strval);
}
#line 1534 "grammar.c"
  yymsp[-2].minor.yy26 = yylhsminor.yy26;
        break;
      case 48: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 298 "grammar.y"
{
	yymsp[-2].minor.yy42 = yymsp[-1].minor.yy42;
}
#line 1542 "grammar.c"
        break;
      case 49: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 310 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy42);
	Vector_Push(args, yymsp[0].minor.yy42);
	yylhsminor.yy42 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1552 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 50: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 317 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy42);
	Vector_Push(args, yymsp[0].minor.yy42);
	yylhsminor.yy42 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1563 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 51: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 324 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy42);
	Vector_Push(args, yymsp[0].minor.yy42);
	yylhsminor.yy42 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1574 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 52: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 331 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy42);
	Vector_Push(args, yymsp[0].minor.yy42);
	yylhsminor.yy42 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1585 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 53: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 339 "grammar.y"
{
	yylhsminor.yy42 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy114);
}
#line 1593 "grammar.c"
  yymsp[-3].minor.yy42 = yylhsminor.yy42;
        break;
      case 54: /* arithmetic_expression ::= value */
#line 344 "grammar.y"
{
	yylhsminor.yy42 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy86);
}
#line 1601 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 55: /* arithmetic_expression ::= variable */
#line 349 "grammar.y"
{
	yylhsminor.yy42 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy76->alias, yymsp[0].minor.yy76->property);
}
#line 1609 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 56: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 355 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy114, yymsp[0].minor.yy42);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1618 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 57: /* arithmetic_expression_list ::= arithmetic_expression */
#line 359 "grammar.y"
{
	yylhsminor.yy114 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy114, yymsp[0].minor.yy42);
}
#line 1627 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 58: /* variable ::= UQSTRING */
#line 366 "grammar.y"
{
	yylhsminor.yy76 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1635 "grammar.c"
  yymsp[0].minor.yy76 = yylhsminor.yy76;
        break;
      case 59: /* variable ::= UQSTRING DOT UQSTRING */
#line 370 "grammar.y"
{
	yylhsminor.yy76 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1643 "grammar.c"
  yymsp[-2].minor.yy76 = yylhsminor.yy76;
        break;
      case 60: /* orderClause ::= */
#line 376 "grammar.y"
{
	yymsp[1].minor.yy12 = NULL;
}
#line 1651 "grammar.c"
        break;
      case 61: /* orderClause ::= ORDER BY columnNameList */
#line 379 "grammar.y"
{
	yymsp[-2].minor.yy12 = New_AST_OrderNode(yymsp[0].minor.yy114, ORDER_DIR_ASC);
}
#line 1658 "grammar.c"
        break;
      case 62: /* orderClause ::= ORDER BY columnNameList ASC */
#line 382 "grammar.y"
{
	yymsp[-3].minor.yy12 = New_AST_OrderNode(yymsp[-1].minor.yy114, ORDER_DIR_ASC);
}
#line 1665 "grammar.c"
        break;
      case 63: /* orderClause ::= ORDER BY columnNameList DESC */
#line 385 "grammar.y"
{
	yymsp[-3].minor.yy12 = New_AST_OrderNode(yymsp[-1].minor.yy114, ORDER_DIR_DESC);
}
#line 1672 "grammar.c"
        break;
      case 64: /* columnNameList ::= columnNameList COMMA columnName */
#line 390 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy114, yymsp[0].minor.yy90);
	yylhsminor.yy114 = yymsp[-2].minor.yy114;
}
#line 1680 "grammar.c"
  yymsp[-2].minor.yy114 = yylhsminor.yy114;
        break;
      case 65: /* columnNameList ::= columnName */
#line 394 "grammar.y"
{
	yylhsminor.yy114 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy114, yymsp[0].minor.yy90);
}
#line 1689 "grammar.c"
  yymsp[0].minor.yy114 = yylhsminor.yy114;
        break;
      case 66: /* columnName ::= variable */
#line 400 "grammar.y"
{
	if(yymsp[0].minor.yy76->property != NULL) {
		yylhsminor.yy90 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy76);
	} else {
		yylhsminor.yy90 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy76->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy76);
}
#line 1703 "grammar.c"
  yymsp[0].minor.yy90 = yylhsminor.yy90;
        break;
      case 67: /* limitClause ::= */
#line 412 "grammar.y"
{
	yymsp[1].minor.yy139 = NULL;
}
#line 1711 "grammar.c"
        break;
      case 68: /* limitClause ::= LIMIT INTEGER */
#line 415 "grammar.y"
{
	yymsp[-1].minor.yy139 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1718 "grammar.c"
        break;
      case 69: /* relation ::= EQ */
#line 421 "grammar.y"
{ yymsp[0].minor.yy8 = EQ; }
#line 1723 "grammar.c"
        break;
      case 70: /* relation ::= GT */
#line 422 "grammar.y"
{ yymsp[0].minor.yy8 = GT; }
#line 1728 "grammar.c"
        break;
      case 71: /* relation ::= LT */
#line 423 "grammar.y"
{ yymsp[0].minor.yy8 = LT; }
#line 1733 "grammar.c"
        break;
      case 72: /* relation ::= LE */
#line 424 "grammar.y"
{ yymsp[0].minor.yy8 = LE; }
#line 1738 "grammar.c"
        break;
      case 73: /* relation ::= GE */
#line 425 "grammar.y"
{ yymsp[0].minor.yy8 = GE; }
#line 1743 "grammar.c"
        break;
      case 74: /* relation ::= NE */
#line 426 "grammar.y"
{ yymsp[0].minor.yy8 = NE; }
#line 1748 "grammar.c"
        break;
      case 75: /* value ::= INTEGER */
#line 437 "grammar.y"
{  yylhsminor.yy86 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1753 "grammar.c"
  yymsp[0].minor.yy86 = yylhsminor.yy86;
        break;
      case 76: /* value ::= DASH INTEGER */
#line 438 "grammar.y"
{  yymsp[-1].minor.yy86 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1759 "grammar.c"
        break;
      case 77: /* value ::= STRING */
#line 439 "grammar.y"
{  yylhsminor.yy86 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1764 "grammar.c"
  yymsp[0].minor.yy86 = yylhsminor.yy86;
        break;
      case 78: /* value ::= FLOAT */
#line 440 "grammar.y"
{  yylhsminor.yy86 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1770 "grammar.c"
  yymsp[0].minor.yy86 = yylhsminor.yy86;
        break;
      case 79: /* value ::= DASH FLOAT */
#line 441 "grammar.y"
{  yymsp[-1].minor.yy86 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1776 "grammar.c"
        break;
      case 80: /* value ::= TRUE */
#line 442 "grammar.y"
{ yymsp[0].minor.yy86 = SI_BoolVal(1); }
#line 1781 "grammar.c"
        break;
      case 81: /* value ::= FALSE */
#line 443 "grammar.y"
{ yymsp[0].minor.yy86 = SI_BoolVal(0); }
#line 1786 "grammar.c"
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[yysize].stateno,(YYCODETYPE)yygoto);

  /* There are no SHIFTREDUCE actions on nonterminals because the table
  ** generator has simplified them to pure REDUCE actions. */
  assert( !(yyact>YY_MAX_SHIFT && yyact<=YY_MAX_SHIFTREDUCE) );

  /* It is not possible for a REDUCE to be followed by an error */
  assert( yyact!=YY_ERROR_ACTION );

  yymsp += yysize+1;
  yypParser->yytos = yymsp;
  yymsp->stateno = (YYACTIONTYPE)yyact;
  yymsp->major = (YYCODETYPE)yygoto;
  yyTraceShift(yypParser, yyact, "... then shift");
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
#line 23 "grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'\n", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 1851 "grammar.c"
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
    int stateno = yypParser->yytos->stateno;
    if( stateno < YY_MIN_REDUCE ){
      fprintf(yyTraceFILE,"%sInput '%s' in state %d\n",
              yyTracePrompt,yyTokenName[yymajor],stateno);
    }else{
      fprintf(yyTraceFILE,"%sInput '%s' with pending reduce %d\n",
              yyTracePrompt,yyTokenName[yymajor],stateno-YY_MIN_REDUCE);
    }
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact >= YY_MIN_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE,yymajor,yyminor);
    }else if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact==YY_ACCEPT_ACTION ){
      yypParser->yytos--;
      yy_accept(yypParser);
      return;
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
        while( yypParser->yytos >= yypParser->yystack
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
#line 445 "grammar.y"


	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	AST_Query *Query_Parse(const char *q, size_t len, char **err) {
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
#line 2097 "grammar.c"
