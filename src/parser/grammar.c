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
#define YYNOCODE 85
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  int yy4;
  AST_MatchNode* yy5;
  AST_SetNode* yy20;
  AST_MergeNode* yy21;
  AST_ColumnNode* yy22;
  AST_CreateNode* yy28;
  AST_OrderNode* yy29;
  AST_Variable* yy36;
  AST_IndexNode* yy48;
  AST_IndexOpType yy57;
  AST_ArithmeticExpressionNode* yy58;
  AST_NodeEntity* yy69;
  SIValue yy78;
  AST_LinkEntity* yy93;
  AST_WhereNode* yy99;
  AST_ReturnNode* yy120;
  AST_FilterNode* yy130;
  AST_LimitNode* yy135;
  AST_DeleteNode * yy143;
  AST_Query* yy160;
  Vector* yy162;
  AST_ReturnElementNode* yy163;
  AST_SetElement* yy168;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             109
#define YYNRULE              92
#define YYNTOKEN             48
#define YY_MAX_SHIFT         108
#define YY_MIN_SHIFTREDUCE   172
#define YY_MAX_SHIFTREDUCE   263
#define YY_ERROR_ACTION      264
#define YY_ACCEPT_ACTION     265
#define YY_NO_ACTION         266
#define YY_MIN_REDUCE        267
#define YY_MAX_REDUCE        358
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
#define YY_ACTTAB_COUNT (260)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   314,  108,  265,   54,   64,  273,  276,  331,   57,  331,
 /*    10 */    56,  274,  275,   95,   72,  330,   64,  330,   66,   10,
 /*    20 */   103,  321,   26,   64,  331,   58,  234,   14,   13,    6,
 /*    30 */    76,  187,  330,   22,    2,   64,   18,   66,   10,  288,
 /*    40 */    81,  256,   87,  258,  259,  261,  262,  263,    1,   11,
 /*    50 */     9,    8,    7,  256,   45,  258,  259,  261,  262,  263,
 /*    60 */   256,  105,  258,  259,  261,  262,  263,  250,  251,  254,
 /*    70 */   252,  253,  256,   21,  258,  259,  261,  262,  263,  331,
 /*    80 */    56,   11,    9,    8,    7,   47,  342,  330,  243,  244,
 /*    90 */   101,  321,  331,   59,   11,    9,    8,    7,  229,   89,
 /*   100 */   330,  340,  255,  331,   56,   65,  331,  328,   82,  287,
 /*   110 */    81,  330,  331,  327,  330,  320,  331,   67,   85,   33,
 /*   120 */   330,  331,   55,   42,  330,   16,  102,  331,   63,  330,
 /*   130 */    45,   93,   34,   53,   15,  330,   38,  270,   50,  342,
 /*   140 */   100,   91,   73,   45,   32,   39,   32,   27,   99,  290,
 /*   150 */    74,  290,  290,   61,  341,   28,   30,   35,   78,   29,
 /*   160 */    45,   60,  290,    3,   28,   30,    8,    7,  257,   88,
 /*   170 */   316,  260,   62,   94,  220,   70,  104,  280,   36,   68,
 /*   180 */    80,   83,  200,   84,   25,   90,   96,   45,  107,   86,
 /*   190 */    92,  272,  291,  310,   98,   97,  106,   48,    1,   49,
 /*   200 */    31,   52,  268,   12,   51,  188,  189,   69,   37,   71,
 /*   210 */    30,   20,  218,   75,   24,   77,  201,    5,  207,  210,
 /*   220 */    79,   40,   19,   41,  205,  249,   23,  203,  211,  209,
 /*   230 */   208,  206,  204,   43,   44,   46,  202,  228,  240,  267,
 /*   240 */   266,    4,  266,  213,  266,  266,  266,  266,  266,  266,
 /*   250 */   266,  266,  266,  104,  266,  266,  266,  266,  266,   17,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    76,   49,   50,   51,    4,   53,   54,   68,   69,   68,
 /*    10 */    69,   59,   60,   76,   62,   76,    4,   76,   18,   19,
 /*    20 */    79,   80,   19,    4,   68,   69,   20,   12,   13,   23,
 /*    30 */    18,   16,   76,   78,   34,    4,   21,   18,   19,   67,
 /*    40 */    68,   41,   17,   43,   44,   45,   46,   47,   33,    3,
 /*    50 */     4,    5,    6,   41,   29,   43,   44,   45,   46,   47,
 /*    60 */    41,   18,   43,   44,   45,   46,   47,    7,    8,    9,
 /*    70 */    10,   11,   41,   23,   43,   44,   45,   46,   47,   68,
 /*    80 */    69,    3,    4,    5,    6,   71,   68,   76,   38,   39,
 /*    90 */    79,   80,   68,   69,    3,    4,    5,    6,   20,   73,
 /*   100 */    76,   83,   42,   68,   69,   81,   68,   69,   66,   67,
 /*   110 */    68,   76,   68,   69,   76,   80,   68,   69,   17,   18,
 /*   120 */    76,   68,   69,    4,   76,   13,   35,   68,   69,   76,
 /*   130 */    29,   17,   18,   53,   22,   76,   24,   57,   58,   68,
 /*   140 */    61,   73,   61,   29,   65,   26,   65,   65,   17,   70,
 /*   150 */    77,   70,   70,   82,   83,    1,    2,   65,   18,   19,
 /*   160 */    29,   77,   70,   19,    1,    2,    5,    6,   41,   73,
 /*   170 */    77,   44,   77,   73,   20,   19,   32,   64,   63,   17,
 /*   180 */    72,   74,   18,   73,   27,   74,   18,   29,   40,   73,
 /*   190 */    73,   56,   70,   75,   73,   75,   36,   55,   33,   54,
 /*   200 */    31,   54,   56,   52,   55,   18,   20,   18,   15,   14,
 /*   210 */     2,   23,   18,   32,   18,   32,   18,    7,    4,   28,
 /*   220 */    23,   18,   23,   18,   20,   41,   17,   20,   28,   28,
 /*   230 */    28,   25,   20,   18,   23,   18,   20,   18,   18,    0,
 /*   240 */    84,   23,   84,   30,   84,   84,   84,   84,   84,   84,
 /*   250 */    84,   84,   84,   32,   84,   84,   84,   84,   84,   37,
 /*   260 */    84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
 /*   270 */    84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
 /*   280 */    84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
 /*   290 */    84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
 /*   300 */    84,   84,   84,   84,   84,   84,   84,   84,
};
#define YY_SHIFT_COUNT    (108)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (239)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    15,    0,   19,   19,   19,   19,   19,   19,   19,   19,
 /*    10 */    19,   19,  112,    3,    3,   43,    3,   43,    3,   43,
 /*    20 */     3,   43,   12,   31,   60,  101,  114,  119,  140,  140,
 /*    30 */   140,  140,  119,   25,  131,  119,  156,  162,  164,  157,
 /*    40 */   158,  158,  157,  158,  168,  168,  158,    3,  148,  160,
 /*    50 */   165,  148,  160,  165,  169,   78,   91,   46,   46,   46,
 /*    60 */   154,   50,  163,  161,  127,    6,  144,  161,  187,  186,
 /*    70 */   189,  193,  195,  188,  208,  194,  181,  196,  183,  198,
 /*    80 */   197,  210,  199,  214,  191,  203,  200,  205,  201,  202,
 /*    90 */   206,  204,  207,  215,  212,  211,  209,  213,  216,  217,
 /*   100 */   188,  218,  219,  218,  220,  221,  222,  184,  239,
};
#define YY_REDUCE_COUNT (54)
#define YY_REDUCE_MIN   (-76)
#define YY_REDUCE_MAX   (151)
static const short yy_reduce_ofst[] = {
 /*     0 */   -48,  -59,   11,   24,   35,  -61,  -44,   38,   44,   48,
 /*    10 */    53,   59,   80,   79,   81,   42,   79,   71,   82,  -28,
 /*    20 */    92,   18,  -76,  -63,  -45,   26,   68,   14,   73,   84,
 /*    30 */    93,   95,   14,   96,  100,   14,  113,  115,  108,  107,
 /*    40 */   110,  116,  111,  117,  118,  120,  121,  122,  135,  142,
 /*    50 */   145,  146,  149,  147,  151,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   278,  264,  264,  264,  264,  264,  264,  264,  264,  264,
 /*    10 */   264,  264,  278,  281,  264,  264,  264,  264,  264,  264,
 /*    20 */   264,  264,  264,  264,  264,  307,  307,  285,  264,  264,
 /*    30 */   264,  264,  292,  307,  307,  293,  264,  264,  264,  264,
 /*    40 */   307,  307,  264,  307,  264,  264,  307,  264,  343,  336,
 /*    50 */   271,  343,  336,  269,  311,  264,  322,  289,  332,  333,
 /*    60 */   264,  337,  312,  325,  264,  264,  334,  326,  264,  264,
 /*    70 */   264,  264,  264,  277,  317,  264,  264,  264,  264,  264,
 /*    80 */   294,  264,  286,  264,  264,  264,  264,  264,  264,  264,
 /*    90 */   264,  264,  264,  264,  264,  309,  264,  264,  264,  264,
 /*   100 */   279,  319,  264,  318,  264,  334,  264,  264,  264,
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
  /*   14 */ "INDEX",
  /*   15 */ "ON",
  /*   16 */ "DROP",
  /*   17 */ "COLON",
  /*   18 */ "UQSTRING",
  /*   19 */ "LEFT_PARENTHESIS",
  /*   20 */ "RIGHT_PARENTHESIS",
  /*   21 */ "MERGE",
  /*   22 */ "SET",
  /*   23 */ "COMMA",
  /*   24 */ "DELETE",
  /*   25 */ "RIGHT_ARROW",
  /*   26 */ "LEFT_ARROW",
  /*   27 */ "LEFT_BRACKET",
  /*   28 */ "RIGHT_BRACKET",
  /*   29 */ "LEFT_CURLY_BRACKET",
  /*   30 */ "RIGHT_CURLY_BRACKET",
  /*   31 */ "WHERE",
  /*   32 */ "DOT",
  /*   33 */ "RETURN",
  /*   34 */ "DISTINCT",
  /*   35 */ "AS",
  /*   36 */ "ORDER",
  /*   37 */ "BY",
  /*   38 */ "ASC",
  /*   39 */ "DESC",
  /*   40 */ "LIMIT",
  /*   41 */ "INTEGER",
  /*   42 */ "NE",
  /*   43 */ "STRING",
  /*   44 */ "FLOAT",
  /*   45 */ "TRUE",
  /*   46 */ "FALSE",
  /*   47 */ "NULLVAL",
  /*   48 */ "error",
  /*   49 */ "expr",
  /*   50 */ "query",
  /*   51 */ "matchClause",
  /*   52 */ "whereClause",
  /*   53 */ "createClause",
  /*   54 */ "returnClause",
  /*   55 */ "orderClause",
  /*   56 */ "limitClause",
  /*   57 */ "deleteClause",
  /*   58 */ "setClause",
  /*   59 */ "indexClause",
  /*   60 */ "mergeClause",
  /*   61 */ "chains",
  /*   62 */ "indexOpToken",
  /*   63 */ "indexLabel",
  /*   64 */ "indexProp",
  /*   65 */ "chain",
  /*   66 */ "setList",
  /*   67 */ "setElement",
  /*   68 */ "variable",
  /*   69 */ "arithmetic_expression",
  /*   70 */ "node",
  /*   71 */ "link",
  /*   72 */ "deleteExpression",
  /*   73 */ "properties",
  /*   74 */ "edge",
  /*   75 */ "mapLiteral",
  /*   76 */ "value",
  /*   77 */ "cond",
  /*   78 */ "relation",
  /*   79 */ "returnElements",
  /*   80 */ "returnElement",
  /*   81 */ "arithmetic_expression_list",
  /*   82 */ "columnNameList",
  /*   83 */ "columnName",
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
 /*   7 */ "expr ::= indexClause",
 /*   8 */ "expr ::= mergeClause",
 /*   9 */ "expr ::= returnClause",
 /*  10 */ "matchClause ::= MATCH chains",
 /*  11 */ "createClause ::=",
 /*  12 */ "createClause ::= CREATE chains",
 /*  13 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  14 */ "indexOpToken ::= CREATE",
 /*  15 */ "indexOpToken ::= DROP",
 /*  16 */ "indexLabel ::= COLON UQSTRING",
 /*  17 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  18 */ "mergeClause ::= MERGE chain",
 /*  19 */ "setClause ::= SET setList",
 /*  20 */ "setList ::= setElement",
 /*  21 */ "setList ::= setList COMMA setElement",
 /*  22 */ "setElement ::= variable EQ arithmetic_expression",
 /*  23 */ "chain ::= node",
 /*  24 */ "chain ::= chain link node",
 /*  25 */ "chains ::= chain",
 /*  26 */ "chains ::= chains COMMA chain",
 /*  27 */ "deleteClause ::= DELETE deleteExpression",
 /*  28 */ "deleteExpression ::= UQSTRING",
 /*  29 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  30 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  31 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  32 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  33 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  34 */ "link ::= DASH edge RIGHT_ARROW",
 /*  35 */ "link ::= LEFT_ARROW edge DASH",
 /*  36 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  37 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  38 */ "edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET",
 /*  39 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  40 */ "properties ::=",
 /*  41 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  42 */ "mapLiteral ::= UQSTRING COLON value",
 /*  43 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  44 */ "whereClause ::=",
 /*  45 */ "whereClause ::= WHERE cond",
 /*  46 */ "cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING",
 /*  47 */ "cond ::= UQSTRING DOT UQSTRING relation value",
 /*  48 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  49 */ "cond ::= cond AND cond",
 /*  50 */ "cond ::= cond OR cond",
 /*  51 */ "returnClause ::= RETURN returnElements",
 /*  52 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  53 */ "returnElements ::= returnElements COMMA returnElement",
 /*  54 */ "returnElements ::= returnElement",
 /*  55 */ "returnElement ::= arithmetic_expression",
 /*  56 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  57 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  58 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  59 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  60 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  61 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  62 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  63 */ "arithmetic_expression ::= value",
 /*  64 */ "arithmetic_expression ::= variable",
 /*  65 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  66 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  67 */ "variable ::= UQSTRING",
 /*  68 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  69 */ "orderClause ::=",
 /*  70 */ "orderClause ::= ORDER BY columnNameList",
 /*  71 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  72 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  73 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  74 */ "columnNameList ::= columnName",
 /*  75 */ "columnName ::= variable",
 /*  76 */ "limitClause ::=",
 /*  77 */ "limitClause ::= LIMIT INTEGER",
 /*  78 */ "relation ::= EQ",
 /*  79 */ "relation ::= GT",
 /*  80 */ "relation ::= LT",
 /*  81 */ "relation ::= LE",
 /*  82 */ "relation ::= GE",
 /*  83 */ "relation ::= NE",
 /*  84 */ "value ::= INTEGER",
 /*  85 */ "value ::= DASH INTEGER",
 /*  86 */ "value ::= STRING",
 /*  87 */ "value ::= FLOAT",
 /*  88 */ "value ::= DASH FLOAT",
 /*  89 */ "value ::= TRUE",
 /*  90 */ "value ::= FALSE",
 /*  91 */ "value ::= NULLVAL",
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
    case 77: /* cond */
{
#line 284 "grammar.y"
 Free_AST_FilterNode((yypminor->yy130)); 
#line 729 "grammar.c"
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
  {   50,   -1 }, /* (0) query ::= expr */
  {   49,   -6 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
  {   49,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   49,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   49,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   49,   -6 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
  {   49,   -1 }, /* (6) expr ::= createClause */
  {   49,   -1 }, /* (7) expr ::= indexClause */
  {   49,   -1 }, /* (8) expr ::= mergeClause */
  {   49,   -1 }, /* (9) expr ::= returnClause */
  {   51,   -2 }, /* (10) matchClause ::= MATCH chains */
  {   53,    0 }, /* (11) createClause ::= */
  {   53,   -2 }, /* (12) createClause ::= CREATE chains */
  {   59,   -5 }, /* (13) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   62,   -1 }, /* (14) indexOpToken ::= CREATE */
  {   62,   -1 }, /* (15) indexOpToken ::= DROP */
  {   63,   -2 }, /* (16) indexLabel ::= COLON UQSTRING */
  {   64,   -3 }, /* (17) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   60,   -2 }, /* (18) mergeClause ::= MERGE chain */
  {   58,   -2 }, /* (19) setClause ::= SET setList */
  {   66,   -1 }, /* (20) setList ::= setElement */
  {   66,   -3 }, /* (21) setList ::= setList COMMA setElement */
  {   67,   -3 }, /* (22) setElement ::= variable EQ arithmetic_expression */
  {   65,   -1 }, /* (23) chain ::= node */
  {   65,   -3 }, /* (24) chain ::= chain link node */
  {   61,   -1 }, /* (25) chains ::= chain */
  {   61,   -3 }, /* (26) chains ::= chains COMMA chain */
  {   57,   -2 }, /* (27) deleteClause ::= DELETE deleteExpression */
  {   72,   -1 }, /* (28) deleteExpression ::= UQSTRING */
  {   72,   -3 }, /* (29) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   70,   -6 }, /* (30) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   70,   -5 }, /* (31) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   70,   -4 }, /* (32) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   70,   -3 }, /* (33) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   71,   -3 }, /* (34) link ::= DASH edge RIGHT_ARROW */
  {   71,   -3 }, /* (35) link ::= LEFT_ARROW edge DASH */
  {   74,   -3 }, /* (36) edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
  {   74,   -4 }, /* (37) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   74,   -5 }, /* (38) edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
  {   74,   -6 }, /* (39) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   73,    0 }, /* (40) properties ::= */
  {   73,   -3 }, /* (41) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   75,   -3 }, /* (42) mapLiteral ::= UQSTRING COLON value */
  {   75,   -5 }, /* (43) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   52,    0 }, /* (44) whereClause ::= */
  {   52,   -2 }, /* (45) whereClause ::= WHERE cond */
  {   77,   -7 }, /* (46) cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
  {   77,   -5 }, /* (47) cond ::= UQSTRING DOT UQSTRING relation value */
  {   77,   -3 }, /* (48) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   77,   -3 }, /* (49) cond ::= cond AND cond */
  {   77,   -3 }, /* (50) cond ::= cond OR cond */
  {   54,   -2 }, /* (51) returnClause ::= RETURN returnElements */
  {   54,   -3 }, /* (52) returnClause ::= RETURN DISTINCT returnElements */
  {   79,   -3 }, /* (53) returnElements ::= returnElements COMMA returnElement */
  {   79,   -1 }, /* (54) returnElements ::= returnElement */
  {   80,   -1 }, /* (55) returnElement ::= arithmetic_expression */
  {   80,   -3 }, /* (56) returnElement ::= arithmetic_expression AS UQSTRING */
  {   69,   -3 }, /* (57) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   69,   -3 }, /* (58) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   69,   -3 }, /* (59) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   69,   -3 }, /* (60) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   69,   -3 }, /* (61) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   69,   -4 }, /* (62) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   69,   -1 }, /* (63) arithmetic_expression ::= value */
  {   69,   -1 }, /* (64) arithmetic_expression ::= variable */
  {   81,   -3 }, /* (65) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   81,   -1 }, /* (66) arithmetic_expression_list ::= arithmetic_expression */
  {   68,   -1 }, /* (67) variable ::= UQSTRING */
  {   68,   -3 }, /* (68) variable ::= UQSTRING DOT UQSTRING */
  {   55,    0 }, /* (69) orderClause ::= */
  {   55,   -3 }, /* (70) orderClause ::= ORDER BY columnNameList */
  {   55,   -4 }, /* (71) orderClause ::= ORDER BY columnNameList ASC */
  {   55,   -4 }, /* (72) orderClause ::= ORDER BY columnNameList DESC */
  {   82,   -3 }, /* (73) columnNameList ::= columnNameList COMMA columnName */
  {   82,   -1 }, /* (74) columnNameList ::= columnName */
  {   83,   -1 }, /* (75) columnName ::= variable */
  {   56,    0 }, /* (76) limitClause ::= */
  {   56,   -2 }, /* (77) limitClause ::= LIMIT INTEGER */
  {   78,   -1 }, /* (78) relation ::= EQ */
  {   78,   -1 }, /* (79) relation ::= GT */
  {   78,   -1 }, /* (80) relation ::= LT */
  {   78,   -1 }, /* (81) relation ::= LE */
  {   78,   -1 }, /* (82) relation ::= GE */
  {   78,   -1 }, /* (83) relation ::= NE */
  {   76,   -1 }, /* (84) value ::= INTEGER */
  {   76,   -2 }, /* (85) value ::= DASH INTEGER */
  {   76,   -1 }, /* (86) value ::= STRING */
  {   76,   -1 }, /* (87) value ::= FLOAT */
  {   76,   -2 }, /* (88) value ::= DASH FLOAT */
  {   76,   -1 }, /* (89) value ::= TRUE */
  {   76,   -1 }, /* (90) value ::= FALSE */
  {   76,   -1 }, /* (91) value ::= NULLVAL */
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
{ ctx->root = yymsp[0].minor.yy160; }
#line 1198 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
#line 38 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(yymsp[-5].minor.yy5, yymsp[-4].minor.yy99, yymsp[-3].minor.yy28, NULL, NULL, NULL, yymsp[-2].minor.yy120, yymsp[-1].minor.yy29, yymsp[0].minor.yy135, NULL);
}
#line 1205 "grammar.c"
  yymsp[-5].minor.yy160 = yylhsminor.yy160;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 42 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(yymsp[-2].minor.yy5, yymsp[-1].minor.yy99, yymsp[0].minor.yy28, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1213 "grammar.c"
  yymsp[-2].minor.yy160 = yylhsminor.yy160;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 46 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(yymsp[-2].minor.yy5, yymsp[-1].minor.yy99, NULL, NULL, NULL, yymsp[0].minor.yy143, NULL, NULL, NULL, NULL);
}
#line 1221 "grammar.c"
  yymsp[-2].minor.yy160 = yylhsminor.yy160;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 50 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(yymsp[-2].minor.yy5, yymsp[-1].minor.yy99, NULL, NULL, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL);
}
#line 1229 "grammar.c"
  yymsp[-2].minor.yy160 = yylhsminor.yy160;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
#line 54 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(yymsp[-5].minor.yy5, yymsp[-4].minor.yy99, NULL, NULL, yymsp[-3].minor.yy20, NULL, yymsp[-2].minor.yy120, yymsp[-1].minor.yy29, yymsp[0].minor.yy135, NULL);
}
#line 1237 "grammar.c"
  yymsp[-5].minor.yy160 = yylhsminor.yy160;
        break;
      case 6: /* expr ::= createClause */
#line 58 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy28, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1245 "grammar.c"
  yymsp[0].minor.yy160 = yylhsminor.yy160;
        break;
      case 7: /* expr ::= indexClause */
#line 62 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy48);
}
#line 1253 "grammar.c"
  yymsp[0].minor.yy160 = yylhsminor.yy160;
        break;
      case 8: /* expr ::= mergeClause */
#line 66 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy21, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1261 "grammar.c"
  yymsp[0].minor.yy160 = yylhsminor.yy160;
        break;
      case 9: /* expr ::= returnClause */
#line 70 "grammar.y"
{
	yylhsminor.yy160 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, NULL, NULL, NULL);
}
#line 1269 "grammar.c"
  yymsp[0].minor.yy160 = yylhsminor.yy160;
        break;
      case 10: /* matchClause ::= MATCH chains */
#line 76 "grammar.y"
{
	yymsp[-1].minor.yy5 = New_AST_MatchNode(yymsp[0].minor.yy162);
}
#line 1277 "grammar.c"
        break;
      case 11: /* createClause ::= */
#line 83 "grammar.y"
{
	yymsp[1].minor.yy28 = NULL;
}
#line 1284 "grammar.c"
        break;
      case 12: /* createClause ::= CREATE chains */
#line 87 "grammar.y"
{
	yymsp[-1].minor.yy28 = New_AST_CreateNode(yymsp[0].minor.yy162);
}
#line 1291 "grammar.c"
        break;
      case 13: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 93 "grammar.y"
{
  yylhsminor.yy48 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy57);
}
#line 1298 "grammar.c"
  yymsp[-4].minor.yy48 = yylhsminor.yy48;
        break;
      case 14: /* indexOpToken ::= CREATE */
#line 99 "grammar.y"
{ yymsp[0].minor.yy57 = CREATE_INDEX; }
#line 1304 "grammar.c"
        break;
      case 15: /* indexOpToken ::= DROP */
#line 100 "grammar.y"
{ yymsp[0].minor.yy57 = DROP_INDEX; }
#line 1309 "grammar.c"
        break;
      case 16: /* indexLabel ::= COLON UQSTRING */
#line 102 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1316 "grammar.c"
        break;
      case 17: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 106 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1323 "grammar.c"
        break;
      case 18: /* mergeClause ::= MERGE chain */
#line 112 "grammar.y"
{
	yymsp[-1].minor.yy21 = New_AST_MergeNode(yymsp[0].minor.yy162);
}
#line 1330 "grammar.c"
        break;
      case 19: /* setClause ::= SET setList */
#line 117 "grammar.y"
{
	yymsp[-1].minor.yy20 = New_AST_SetNode(yymsp[0].minor.yy162);
}
#line 1337 "grammar.c"
        break;
      case 20: /* setList ::= setElement */
#line 122 "grammar.y"
{
	yylhsminor.yy162 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy168);
}
#line 1345 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 21: /* setList ::= setList COMMA setElement */
#line 126 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy168);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1354 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 22: /* setElement ::= variable EQ arithmetic_expression */
#line 132 "grammar.y"
{
	yylhsminor.yy168 = New_AST_SetElement(yymsp[-2].minor.yy36, yymsp[0].minor.yy58);
}
#line 1362 "grammar.c"
  yymsp[-2].minor.yy168 = yylhsminor.yy168;
        break;
      case 23: /* chain ::= node */
#line 138 "grammar.y"
{
	yylhsminor.yy162 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy69);
}
#line 1371 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 24: /* chain ::= chain link node */
#line 143 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[-1].minor.yy93);
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy69);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1381 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 25: /* chains ::= chain */
#line 151 "grammar.y"
{
	yylhsminor.yy162 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy162);
}
#line 1390 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 26: /* chains ::= chains COMMA chain */
#line 156 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy162);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1399 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 27: /* deleteClause ::= DELETE deleteExpression */
#line 164 "grammar.y"
{
	yymsp[-1].minor.yy143 = New_AST_DeleteNode(yymsp[0].minor.yy162);
}
#line 1407 "grammar.c"
        break;
      case 28: /* deleteExpression ::= UQSTRING */
#line 170 "grammar.y"
{
	yylhsminor.yy162 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy0.strval);
}
#line 1415 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 29: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 175 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy0.strval);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1424 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 30: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 183 "grammar.y"
{
	yymsp[-5].minor.yy69 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy162);
}
#line 1432 "grammar.c"
        break;
      case 31: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 188 "grammar.y"
{
	yymsp[-4].minor.yy69 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy162);
}
#line 1439 "grammar.c"
        break;
      case 32: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 193 "grammar.y"
{
	yymsp[-3].minor.yy69 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy162);
}
#line 1446 "grammar.c"
        break;
      case 33: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 198 "grammar.y"
{
	yymsp[-2].minor.yy69 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy162);
}
#line 1453 "grammar.c"
        break;
      case 34: /* link ::= DASH edge RIGHT_ARROW */
#line 205 "grammar.y"
{
	yymsp[-2].minor.yy93 = yymsp[-1].minor.yy93;
	yymsp[-2].minor.yy93->direction = N_LEFT_TO_RIGHT;
}
#line 1461 "grammar.c"
        break;
      case 35: /* link ::= LEFT_ARROW edge DASH */
#line 211 "grammar.y"
{
	yymsp[-2].minor.yy93 = yymsp[-1].minor.yy93;
	yymsp[-2].minor.yy93->direction = N_RIGHT_TO_LEFT;
}
#line 1469 "grammar.c"
        break;
      case 36: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 218 "grammar.y"
{ 
	yymsp[-2].minor.yy93 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy162, N_DIR_UNKNOWN);
}
#line 1476 "grammar.c"
        break;
      case 37: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 223 "grammar.y"
{ 
	yymsp[-3].minor.yy93 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy162, N_DIR_UNKNOWN);
}
#line 1483 "grammar.c"
        break;
      case 38: /* edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
#line 228 "grammar.y"
{ 
	yymsp[-4].minor.yy93 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy162, N_DIR_UNKNOWN);
}
#line 1490 "grammar.c"
        break;
      case 39: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 233 "grammar.y"
{ 
	yymsp[-5].minor.yy93 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy162, N_DIR_UNKNOWN);
}
#line 1497 "grammar.c"
        break;
      case 40: /* properties ::= */
#line 239 "grammar.y"
{
	yymsp[1].minor.yy162 = NULL;
}
#line 1504 "grammar.c"
        break;
      case 41: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 243 "grammar.y"
{
	yymsp[-2].minor.yy162 = yymsp[-1].minor.yy162;
}
#line 1511 "grammar.c"
        break;
      case 42: /* mapLiteral ::= UQSTRING COLON value */
#line 249 "grammar.y"
{
	yylhsminor.yy162 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy162, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy78;
	Vector_Push(yylhsminor.yy162, val);
}
#line 1526 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 43: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 261 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy162, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy78;
	Vector_Push(yymsp[0].minor.yy162, val);
	
	yylhsminor.yy162 = yymsp[0].minor.yy162;
}
#line 1542 "grammar.c"
  yymsp[-4].minor.yy162 = yylhsminor.yy162;
        break;
      case 44: /* whereClause ::= */
#line 275 "grammar.y"
{ 
	yymsp[1].minor.yy99 = NULL;
}
#line 1550 "grammar.c"
        break;
      case 45: /* whereClause ::= WHERE cond */
#line 278 "grammar.y"
{
	yymsp[-1].minor.yy99 = New_AST_WhereNode(yymsp[0].minor.yy130);
}
#line 1557 "grammar.c"
        break;
      case 46: /* cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
#line 287 "grammar.y"
{ yylhsminor.yy130 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy4, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1562 "grammar.c"
  yymsp[-6].minor.yy130 = yylhsminor.yy130;
        break;
      case 47: /* cond ::= UQSTRING DOT UQSTRING relation value */
#line 290 "grammar.y"
{ yylhsminor.yy130 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy4, yymsp[0].minor.yy78); }
#line 1568 "grammar.c"
  yymsp[-4].minor.yy130 = yylhsminor.yy130;
        break;
      case 48: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 291 "grammar.y"
{ yymsp[-2].minor.yy130 = yymsp[-1].minor.yy130; }
#line 1574 "grammar.c"
        break;
      case 49: /* cond ::= cond AND cond */
#line 292 "grammar.y"
{ yylhsminor.yy130 = New_AST_ConditionNode(yymsp[-2].minor.yy130, AND, yymsp[0].minor.yy130); }
#line 1579 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 50: /* cond ::= cond OR cond */
#line 293 "grammar.y"
{ yylhsminor.yy130 = New_AST_ConditionNode(yymsp[-2].minor.yy130, OR, yymsp[0].minor.yy130); }
#line 1585 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 51: /* returnClause ::= RETURN returnElements */
#line 298 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy162, 0);
}
#line 1593 "grammar.c"
        break;
      case 52: /* returnClause ::= RETURN DISTINCT returnElements */
#line 301 "grammar.y"
{
	yymsp[-2].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy162, 1);
}
#line 1600 "grammar.c"
        break;
      case 53: /* returnElements ::= returnElements COMMA returnElement */
#line 308 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy163);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1608 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 54: /* returnElements ::= returnElement */
#line 313 "grammar.y"
{
	yylhsminor.yy162 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy163);
}
#line 1617 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 55: /* returnElement ::= arithmetic_expression */
#line 320 "grammar.y"
{
	yylhsminor.yy163 = New_AST_ReturnElementNode(yymsp[0].minor.yy58, NULL);
}
#line 1625 "grammar.c"
  yymsp[0].minor.yy163 = yylhsminor.yy163;
        break;
      case 56: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 325 "grammar.y"
{
	yylhsminor.yy163 = New_AST_ReturnElementNode(yymsp[-2].minor.yy58, yymsp[0].minor.yy0.strval);
}
#line 1633 "grammar.c"
  yymsp[-2].minor.yy163 = yylhsminor.yy163;
        break;
      case 57: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 332 "grammar.y"
{
	yymsp[-2].minor.yy58 = yymsp[-1].minor.yy58;
}
#line 1641 "grammar.c"
        break;
      case 58: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 344 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy58);
	Vector_Push(args, yymsp[0].minor.yy58);
	yylhsminor.yy58 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1651 "grammar.c"
  yymsp[-2].minor.yy58 = yylhsminor.yy58;
        break;
      case 59: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 351 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy58);
	Vector_Push(args, yymsp[0].minor.yy58);
	yylhsminor.yy58 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1662 "grammar.c"
  yymsp[-2].minor.yy58 = yylhsminor.yy58;
        break;
      case 60: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 358 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy58);
	Vector_Push(args, yymsp[0].minor.yy58);
	yylhsminor.yy58 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1673 "grammar.c"
  yymsp[-2].minor.yy58 = yylhsminor.yy58;
        break;
      case 61: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 365 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy58);
	Vector_Push(args, yymsp[0].minor.yy58);
	yylhsminor.yy58 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1684 "grammar.c"
  yymsp[-2].minor.yy58 = yylhsminor.yy58;
        break;
      case 62: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 373 "grammar.y"
{
	yylhsminor.yy58 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy162);
}
#line 1692 "grammar.c"
  yymsp[-3].minor.yy58 = yylhsminor.yy58;
        break;
      case 63: /* arithmetic_expression ::= value */
#line 378 "grammar.y"
{
	yylhsminor.yy58 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy78);
}
#line 1700 "grammar.c"
  yymsp[0].minor.yy58 = yylhsminor.yy58;
        break;
      case 64: /* arithmetic_expression ::= variable */
#line 383 "grammar.y"
{
	yylhsminor.yy58 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy36->alias, yymsp[0].minor.yy36->property);
}
#line 1708 "grammar.c"
  yymsp[0].minor.yy58 = yylhsminor.yy58;
        break;
      case 65: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 389 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy58);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1717 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 66: /* arithmetic_expression_list ::= arithmetic_expression */
#line 393 "grammar.y"
{
	yylhsminor.yy162 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy58);
}
#line 1726 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 67: /* variable ::= UQSTRING */
#line 400 "grammar.y"
{
	yylhsminor.yy36 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1734 "grammar.c"
  yymsp[0].minor.yy36 = yylhsminor.yy36;
        break;
      case 68: /* variable ::= UQSTRING DOT UQSTRING */
#line 404 "grammar.y"
{
	yylhsminor.yy36 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1742 "grammar.c"
  yymsp[-2].minor.yy36 = yylhsminor.yy36;
        break;
      case 69: /* orderClause ::= */
#line 410 "grammar.y"
{
	yymsp[1].minor.yy29 = NULL;
}
#line 1750 "grammar.c"
        break;
      case 70: /* orderClause ::= ORDER BY columnNameList */
#line 413 "grammar.y"
{
	yymsp[-2].minor.yy29 = New_AST_OrderNode(yymsp[0].minor.yy162, ORDER_DIR_ASC);
}
#line 1757 "grammar.c"
        break;
      case 71: /* orderClause ::= ORDER BY columnNameList ASC */
#line 416 "grammar.y"
{
	yymsp[-3].minor.yy29 = New_AST_OrderNode(yymsp[-1].minor.yy162, ORDER_DIR_ASC);
}
#line 1764 "grammar.c"
        break;
      case 72: /* orderClause ::= ORDER BY columnNameList DESC */
#line 419 "grammar.y"
{
	yymsp[-3].minor.yy29 = New_AST_OrderNode(yymsp[-1].minor.yy162, ORDER_DIR_DESC);
}
#line 1771 "grammar.c"
        break;
      case 73: /* columnNameList ::= columnNameList COMMA columnName */
#line 424 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy162, yymsp[0].minor.yy22);
	yylhsminor.yy162 = yymsp[-2].minor.yy162;
}
#line 1779 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 74: /* columnNameList ::= columnName */
#line 428 "grammar.y"
{
	yylhsminor.yy162 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy162, yymsp[0].minor.yy22);
}
#line 1788 "grammar.c"
  yymsp[0].minor.yy162 = yylhsminor.yy162;
        break;
      case 75: /* columnName ::= variable */
#line 434 "grammar.y"
{
	if(yymsp[0].minor.yy36->property != NULL) {
		yylhsminor.yy22 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy36);
	} else {
		yylhsminor.yy22 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy36->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy36);
}
#line 1802 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 76: /* limitClause ::= */
#line 446 "grammar.y"
{
	yymsp[1].minor.yy135 = NULL;
}
#line 1810 "grammar.c"
        break;
      case 77: /* limitClause ::= LIMIT INTEGER */
#line 449 "grammar.y"
{
	yymsp[-1].minor.yy135 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1817 "grammar.c"
        break;
      case 78: /* relation ::= EQ */
#line 455 "grammar.y"
{ yymsp[0].minor.yy4 = EQ; }
#line 1822 "grammar.c"
        break;
      case 79: /* relation ::= GT */
#line 456 "grammar.y"
{ yymsp[0].minor.yy4 = GT; }
#line 1827 "grammar.c"
        break;
      case 80: /* relation ::= LT */
#line 457 "grammar.y"
{ yymsp[0].minor.yy4 = LT; }
#line 1832 "grammar.c"
        break;
      case 81: /* relation ::= LE */
#line 458 "grammar.y"
{ yymsp[0].minor.yy4 = LE; }
#line 1837 "grammar.c"
        break;
      case 82: /* relation ::= GE */
#line 459 "grammar.y"
{ yymsp[0].minor.yy4 = GE; }
#line 1842 "grammar.c"
        break;
      case 83: /* relation ::= NE */
#line 460 "grammar.y"
{ yymsp[0].minor.yy4 = NE; }
#line 1847 "grammar.c"
        break;
      case 84: /* value ::= INTEGER */
#line 471 "grammar.y"
{  yylhsminor.yy78 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1852 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 85: /* value ::= DASH INTEGER */
#line 472 "grammar.y"
{  yymsp[-1].minor.yy78 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1858 "grammar.c"
        break;
      case 86: /* value ::= STRING */
#line 473 "grammar.y"
{  yylhsminor.yy78 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1863 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 87: /* value ::= FLOAT */
#line 474 "grammar.y"
{  yylhsminor.yy78 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1869 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 88: /* value ::= DASH FLOAT */
#line 475 "grammar.y"
{  yymsp[-1].minor.yy78 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1875 "grammar.c"
        break;
      case 89: /* value ::= TRUE */
#line 476 "grammar.y"
{ yymsp[0].minor.yy78 = SI_BoolVal(1); }
#line 1880 "grammar.c"
        break;
      case 90: /* value ::= FALSE */
#line 477 "grammar.y"
{ yymsp[0].minor.yy78 = SI_BoolVal(0); }
#line 1885 "grammar.c"
        break;
      case 91: /* value ::= NULLVAL */
#line 478 "grammar.y"
{ yymsp[0].minor.yy78 = SI_NullVal(); }
#line 1890 "grammar.c"
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
#line 1955 "grammar.c"
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
#line 480 "grammar.y"


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
#line 2201 "grammar.c"
