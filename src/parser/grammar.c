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
#define YYNOCODE 82
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_CreateNode* yy4;
  AST_IndexOpType yy15;
  int yy58;
  AST_IndexNode* yy60;
  AST_SetElement* yy61;
  AST_NodeEntity* yy63;
  AST_FilterNode* yy64;
  AST_MatchNode* yy65;
  AST_ReturnNode* yy66;
  AST_ColumnNode* yy82;
  AST_ReturnElementNode* yy84;
  AST_OrderNode* yy88;
  SIValue yy96;
  AST_SetNode* yy98;
  AST_Variable* yy102;
  AST_WhereNode* yy111;
  AST_Query* yy112;
  AST_LinkEntity* yy123;
  AST_LimitNode* yy147;
  AST_ArithmeticExpressionNode* yy154;
  AST_DeleteNode * yy155;
  Vector* yy156;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             107
#define YYNRULE              88
#define YYNTOKEN             46
#define YY_MAX_SHIFT         106
#define YY_MIN_SHIFTREDUCE   167
#define YY_MAX_SHIFTREDUCE   254
#define YY_ERROR_ACTION      255
#define YY_ACCEPT_ACTION     256
#define YY_NO_ACTION         257
#define YY_MIN_REDUCE        258
#define YY_MAX_REDUCE        345
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
#define YY_ACTTAB_COUNT (254)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   302,  106,  256,   52,   62,  264,  276,   79,  319,   54,
 /*    10 */    21,  265,   93,   70,   72,  319,   54,  318,   64,   10,
 /*    20 */   101,  309,   25,   62,  318,  319,   57,   99,  309,   11,
 /*    30 */     9,    8,    7,    2,  318,   83,   31,   64,   10,   63,
 /*    40 */   248,  103,  250,  251,  253,  254,   43,   62,  242,  243,
 /*    50 */   246,  244,  245,   11,    9,    8,    7,   16,  226,  248,
 /*    60 */     6,  250,  251,  253,  254,   15,   62,   36,  319,   54,
 /*    70 */   221,   87,   14,   13,   26,   28,  180,  318,   33,  278,
 /*    80 */    74,  308,  247,  248,   89,  250,  251,  253,  254,   11,
 /*    90 */     9,    8,    7,  212,  319,   55,   80,  275,   79,  319,
 /*   100 */    56,  330,  248,  318,  250,  251,  253,  254,  318,  319,
 /*   110 */   316,   78,  319,  315,   98,  319,   65,  328,  318,    3,
 /*   120 */   100,  318,   30,  278,  318,   71,   85,  319,   53,  319,
 /*   130 */    61,  102,  330,   30,  278,   51,  318,   43,  318,  261,
 /*   140 */    48,   91,   32,   20,   76,   27,   40,   59,  329,   58,
 /*   150 */    97,  304,   43,   60,   26,   28,    8,    7,  235,  236,
 /*   160 */    45,   43,  249,   86,   92,  252,   68,   37,  269,   66,
 /*   170 */    34,   24,  192,   81,   43,   82,   84,   94,  105,   88,
 /*   180 */    46,   90,  279,  263,  298,   95,   96,   47,    1,   49,
 /*   190 */   104,  259,   50,   29,   12,  181,  182,   67,   35,   69,
 /*   200 */    19,   28,  210,   73,   22,   75,  193,    5,  199,   77,
 /*   210 */    38,   18,   39,  102,  258,   23,  197,  202,  203,   41,
 /*   220 */   201,  198,  200,  195,   42,  196,   44,  194,  220,  232,
 /*   230 */   257,    4,  257,  257,  257,  205,  257,  257,  257,  257,
 /*   240 */   257,  257,  257,  257,  257,  257,  257,  257,   17,  257,
 /*   250 */   257,  257,  257,  241,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    73,   47,   48,   49,    4,   51,   63,   64,   64,   65,
 /*    10 */    75,   57,   73,   59,   74,   64,   65,   73,   18,   19,
 /*    20 */    76,   77,   19,    4,   73,   64,   65,   76,   77,    3,
 /*    30 */     4,    5,    6,   33,   73,   17,   18,   18,   19,   78,
 /*    40 */    40,   18,   42,   43,   44,   45,   28,    4,    7,    8,
 /*    50 */     9,   10,   11,    3,    4,    5,    6,   13,   20,   40,
 /*    60 */    22,   42,   43,   44,   45,   21,    4,   23,   64,   65,
 /*    70 */    20,   70,   12,   13,    1,    2,   16,   73,   66,   67,
 /*    80 */    18,   77,   41,   40,   70,   42,   43,   44,   45,    3,
 /*    90 */     4,    5,    6,   20,   64,   65,   62,   63,   64,   64,
 /*   100 */    65,   64,   40,   73,   42,   43,   44,   45,   73,   64,
 /*   110 */    65,   69,   64,   65,   58,   64,   65,   80,   73,   19,
 /*   120 */    34,   73,   66,   67,   73,   58,   17,   64,   65,   64,
 /*   130 */    65,   31,   64,   66,   67,   51,   73,   28,   73,   55,
 /*   140 */    56,   17,   18,   22,   18,   19,    4,   79,   80,   74,
 /*   150 */    17,   74,   28,   74,    1,    2,    5,    6,   37,   38,
 /*   160 */    68,   28,   40,   70,   70,   43,   19,   25,   61,   17,
 /*   170 */    60,   26,   18,   71,   28,   70,   70,   18,   39,   71,
 /*   180 */    53,   70,   67,   54,   72,   72,   70,   52,   32,   53,
 /*   190 */    35,   54,   52,   30,   50,   18,   20,   18,   15,   14,
 /*   200 */    22,    2,   18,   31,   18,   31,   18,    7,    4,   22,
 /*   210 */    18,   22,   18,   31,    0,   17,   20,   27,   27,   18,
 /*   220 */    27,   24,   27,   20,   22,   20,   18,   20,   18,   18,
 /*   230 */    81,   22,   81,   81,   81,   29,   81,   81,   81,   81,
 /*   240 */    81,   81,   81,   81,   81,   81,   81,   81,   36,   81,
 /*   250 */    81,   81,   81,   40,   81,   81,   81,   81,   81,   81,
 /*   260 */    81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
 /*   270 */    81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
 /*   280 */    81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
 /*   290 */    81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
};
#define YY_SHIFT_COUNT    (106)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (214)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    60,    0,   19,   19,   19,   19,   19,   19,   19,   19,
 /*    10 */    19,   19,   44,    3,    3,   23,    3,   23,   23,    3,
 /*    20 */    23,   62,   41,   43,   18,  124,  126,  126,  126,  126,
 /*    30 */   142,  109,  133,  142,  147,  152,  154,  145,  146,  146,
 /*    40 */   145,  146,  159,  159,  146,    3,  139,  155,  156,  139,
 /*    50 */   155,  156,  163,   50,   86,   26,   26,   26,   73,  121,
 /*    60 */   153,  151,  122,   38,  100,  151,  177,  176,  179,  183,
 /*    70 */   185,  178,  199,  184,  172,  186,  174,  188,  187,  200,
 /*    80 */   189,  204,  190,  192,  191,  194,  193,  195,  197,  196,
 /*    90 */   203,  201,  205,  202,  198,  206,  207,  208,  178,  209,
 /*   100 */   210,  209,  211,  182,  212,  213,  214,
};
#define YY_REDUCE_COUNT (52)
#define YY_REDUCE_MIN   (-73)
#define YY_REDUCE_MAX   (144)
static const short yy_reduce_ofst[] = {
 /*     0 */   -46,  -56,  -49,  -39,    4,   30,   35,   45,   48,   51,
 /*    10 */    63,   65,   84,   56,   67,   34,   56,   68,  -57,   12,
 /*    20 */    37,  -73,  -65,  -61,    1,   14,  -60,   75,   77,   79,
 /*    30 */    92,   93,   94,   92,  107,  110,   42,  102,  105,  106,
 /*    40 */   108,  111,  112,  113,  116,  115,  129,  127,  135,  137,
 /*    50 */   136,  140,  144,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   267,  255,  255,  255,  255,  255,  255,  255,  255,  255,
 /*    10 */   255,  255,  267,  270,  255,  255,  255,  255,  255,  255,
 /*    20 */   255,  255,  255,  255,  295,  295,  255,  255,  255,  255,
 /*    30 */   280,  295,  295,  281,  255,  255,  255,  255,  295,  295,
 /*    40 */   255,  295,  255,  255,  295,  255,  331,  324,  262,  331,
 /*    50 */   324,  260,  299,  255,  310,  277,  320,  321,  255,  325,
 /*    60 */   300,  313,  255,  255,  322,  314,  255,  255,  255,  255,
 /*    70 */   255,  266,  305,  255,  255,  255,  255,  255,  282,  255,
 /*    80 */   274,  255,  255,  255,  255,  255,  255,  255,  255,  255,
 /*    90 */   255,  255,  255,  297,  255,  255,  255,  255,  268,  307,
 /*   100 */   255,  306,  255,  322,  255,  255,  255,
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
  /*   21 */ "SET",
  /*   22 */ "COMMA",
  /*   23 */ "DELETE",
  /*   24 */ "RIGHT_ARROW",
  /*   25 */ "LEFT_ARROW",
  /*   26 */ "LEFT_BRACKET",
  /*   27 */ "RIGHT_BRACKET",
  /*   28 */ "LEFT_CURLY_BRACKET",
  /*   29 */ "RIGHT_CURLY_BRACKET",
  /*   30 */ "WHERE",
  /*   31 */ "DOT",
  /*   32 */ "RETURN",
  /*   33 */ "DISTINCT",
  /*   34 */ "AS",
  /*   35 */ "ORDER",
  /*   36 */ "BY",
  /*   37 */ "ASC",
  /*   38 */ "DESC",
  /*   39 */ "LIMIT",
  /*   40 */ "INTEGER",
  /*   41 */ "NE",
  /*   42 */ "STRING",
  /*   43 */ "FLOAT",
  /*   44 */ "TRUE",
  /*   45 */ "FALSE",
  /*   46 */ "error",
  /*   47 */ "expr",
  /*   48 */ "query",
  /*   49 */ "matchClause",
  /*   50 */ "whereClause",
  /*   51 */ "createClause",
  /*   52 */ "returnClause",
  /*   53 */ "orderClause",
  /*   54 */ "limitClause",
  /*   55 */ "deleteClause",
  /*   56 */ "setClause",
  /*   57 */ "indexClause",
  /*   58 */ "chains",
  /*   59 */ "indexOpToken",
  /*   60 */ "indexLabel",
  /*   61 */ "indexProp",
  /*   62 */ "setList",
  /*   63 */ "setElement",
  /*   64 */ "variable",
  /*   65 */ "arithmetic_expression",
  /*   66 */ "chain",
  /*   67 */ "node",
  /*   68 */ "link",
  /*   69 */ "deleteExpression",
  /*   70 */ "properties",
  /*   71 */ "edge",
  /*   72 */ "mapLiteral",
  /*   73 */ "value",
  /*   74 */ "cond",
  /*   75 */ "relation",
  /*   76 */ "returnElements",
  /*   77 */ "returnElement",
  /*   78 */ "arithmetic_expression_list",
  /*   79 */ "columnNameList",
  /*   80 */ "columnName",
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
 /*   8 */ "matchClause ::= MATCH chains",
 /*   9 */ "createClause ::=",
 /*  10 */ "createClause ::= CREATE chains",
 /*  11 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  12 */ "indexOpToken ::= CREATE",
 /*  13 */ "indexOpToken ::= DROP",
 /*  14 */ "indexLabel ::= COLON UQSTRING",
 /*  15 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  16 */ "setClause ::= SET setList",
 /*  17 */ "setList ::= setElement",
 /*  18 */ "setList ::= setList COMMA setElement",
 /*  19 */ "setElement ::= variable EQ arithmetic_expression",
 /*  20 */ "chain ::= node",
 /*  21 */ "chain ::= chain link node",
 /*  22 */ "chains ::= chain",
 /*  23 */ "chains ::= chains COMMA chain",
 /*  24 */ "deleteClause ::= DELETE deleteExpression",
 /*  25 */ "deleteExpression ::= UQSTRING",
 /*  26 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  27 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  28 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  29 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  30 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  31 */ "link ::= DASH edge RIGHT_ARROW",
 /*  32 */ "link ::= LEFT_ARROW edge DASH",
 /*  33 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  34 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  35 */ "edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET",
 /*  36 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  37 */ "properties ::=",
 /*  38 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  39 */ "mapLiteral ::= UQSTRING COLON value",
 /*  40 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  41 */ "whereClause ::=",
 /*  42 */ "whereClause ::= WHERE cond",
 /*  43 */ "cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING",
 /*  44 */ "cond ::= UQSTRING DOT UQSTRING relation value",
 /*  45 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  46 */ "cond ::= cond AND cond",
 /*  47 */ "cond ::= cond OR cond",
 /*  48 */ "returnClause ::= RETURN returnElements",
 /*  49 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  50 */ "returnElements ::= returnElements COMMA returnElement",
 /*  51 */ "returnElements ::= returnElement",
 /*  52 */ "returnElement ::= arithmetic_expression",
 /*  53 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  54 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  55 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  56 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  57 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  58 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  59 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  60 */ "arithmetic_expression ::= value",
 /*  61 */ "arithmetic_expression ::= variable",
 /*  62 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  63 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  64 */ "variable ::= UQSTRING",
 /*  65 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  66 */ "orderClause ::=",
 /*  67 */ "orderClause ::= ORDER BY columnNameList",
 /*  68 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  69 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  70 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  71 */ "columnNameList ::= columnName",
 /*  72 */ "columnName ::= variable",
 /*  73 */ "limitClause ::=",
 /*  74 */ "limitClause ::= LIMIT INTEGER",
 /*  75 */ "relation ::= EQ",
 /*  76 */ "relation ::= GT",
 /*  77 */ "relation ::= LT",
 /*  78 */ "relation ::= LE",
 /*  79 */ "relation ::= GE",
 /*  80 */ "relation ::= NE",
 /*  81 */ "value ::= INTEGER",
 /*  82 */ "value ::= DASH INTEGER",
 /*  83 */ "value ::= STRING",
 /*  84 */ "value ::= FLOAT",
 /*  85 */ "value ::= DASH FLOAT",
 /*  86 */ "value ::= TRUE",
 /*  87 */ "value ::= FALSE",
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
    case 74: /* cond */
{
#line 273 "grammar.y"
 Free_AST_FilterNode((yypminor->yy64)); 
#line 720 "grammar.c"
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
  {   48,   -1 }, /* (0) query ::= expr */
  {   47,   -6 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
  {   47,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   47,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   47,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   47,   -6 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
  {   47,   -1 }, /* (6) expr ::= createClause */
  {   47,   -1 }, /* (7) expr ::= indexClause */
  {   49,   -2 }, /* (8) matchClause ::= MATCH chains */
  {   51,    0 }, /* (9) createClause ::= */
  {   51,   -2 }, /* (10) createClause ::= CREATE chains */
  {   57,   -5 }, /* (11) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   59,   -1 }, /* (12) indexOpToken ::= CREATE */
  {   59,   -1 }, /* (13) indexOpToken ::= DROP */
  {   60,   -2 }, /* (14) indexLabel ::= COLON UQSTRING */
  {   61,   -3 }, /* (15) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   56,   -2 }, /* (16) setClause ::= SET setList */
  {   62,   -1 }, /* (17) setList ::= setElement */
  {   62,   -3 }, /* (18) setList ::= setList COMMA setElement */
  {   63,   -3 }, /* (19) setElement ::= variable EQ arithmetic_expression */
  {   66,   -1 }, /* (20) chain ::= node */
  {   66,   -3 }, /* (21) chain ::= chain link node */
  {   58,   -1 }, /* (22) chains ::= chain */
  {   58,   -3 }, /* (23) chains ::= chains COMMA chain */
  {   55,   -2 }, /* (24) deleteClause ::= DELETE deleteExpression */
  {   69,   -1 }, /* (25) deleteExpression ::= UQSTRING */
  {   69,   -3 }, /* (26) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   67,   -6 }, /* (27) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   67,   -5 }, /* (28) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   67,   -4 }, /* (29) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   67,   -3 }, /* (30) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   68,   -3 }, /* (31) link ::= DASH edge RIGHT_ARROW */
  {   68,   -3 }, /* (32) link ::= LEFT_ARROW edge DASH */
  {   71,   -3 }, /* (33) edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
  {   71,   -4 }, /* (34) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   71,   -5 }, /* (35) edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
  {   71,   -6 }, /* (36) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   70,    0 }, /* (37) properties ::= */
  {   70,   -3 }, /* (38) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   72,   -3 }, /* (39) mapLiteral ::= UQSTRING COLON value */
  {   72,   -5 }, /* (40) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   50,    0 }, /* (41) whereClause ::= */
  {   50,   -2 }, /* (42) whereClause ::= WHERE cond */
  {   74,   -7 }, /* (43) cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
  {   74,   -5 }, /* (44) cond ::= UQSTRING DOT UQSTRING relation value */
  {   74,   -3 }, /* (45) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   74,   -3 }, /* (46) cond ::= cond AND cond */
  {   74,   -3 }, /* (47) cond ::= cond OR cond */
  {   52,   -2 }, /* (48) returnClause ::= RETURN returnElements */
  {   52,   -3 }, /* (49) returnClause ::= RETURN DISTINCT returnElements */
  {   76,   -3 }, /* (50) returnElements ::= returnElements COMMA returnElement */
  {   76,   -1 }, /* (51) returnElements ::= returnElement */
  {   77,   -1 }, /* (52) returnElement ::= arithmetic_expression */
  {   77,   -3 }, /* (53) returnElement ::= arithmetic_expression AS UQSTRING */
  {   65,   -3 }, /* (54) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   65,   -3 }, /* (55) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   65,   -3 }, /* (56) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   65,   -3 }, /* (57) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   65,   -3 }, /* (58) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   65,   -4 }, /* (59) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   65,   -1 }, /* (60) arithmetic_expression ::= value */
  {   65,   -1 }, /* (61) arithmetic_expression ::= variable */
  {   78,   -3 }, /* (62) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   78,   -1 }, /* (63) arithmetic_expression_list ::= arithmetic_expression */
  {   64,   -1 }, /* (64) variable ::= UQSTRING */
  {   64,   -3 }, /* (65) variable ::= UQSTRING DOT UQSTRING */
  {   53,    0 }, /* (66) orderClause ::= */
  {   53,   -3 }, /* (67) orderClause ::= ORDER BY columnNameList */
  {   53,   -4 }, /* (68) orderClause ::= ORDER BY columnNameList ASC */
  {   53,   -4 }, /* (69) orderClause ::= ORDER BY columnNameList DESC */
  {   79,   -3 }, /* (70) columnNameList ::= columnNameList COMMA columnName */
  {   79,   -1 }, /* (71) columnNameList ::= columnName */
  {   80,   -1 }, /* (72) columnName ::= variable */
  {   54,    0 }, /* (73) limitClause ::= */
  {   54,   -2 }, /* (74) limitClause ::= LIMIT INTEGER */
  {   75,   -1 }, /* (75) relation ::= EQ */
  {   75,   -1 }, /* (76) relation ::= GT */
  {   75,   -1 }, /* (77) relation ::= LT */
  {   75,   -1 }, /* (78) relation ::= LE */
  {   75,   -1 }, /* (79) relation ::= GE */
  {   75,   -1 }, /* (80) relation ::= NE */
  {   73,   -1 }, /* (81) value ::= INTEGER */
  {   73,   -2 }, /* (82) value ::= DASH INTEGER */
  {   73,   -1 }, /* (83) value ::= STRING */
  {   73,   -1 }, /* (84) value ::= FLOAT */
  {   73,   -2 }, /* (85) value ::= DASH FLOAT */
  {   73,   -1 }, /* (86) value ::= TRUE */
  {   73,   -1 }, /* (87) value ::= FALSE */
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
{ ctx->root = yymsp[0].minor.yy112; }
#line 1185 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
#line 38 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-5].minor.yy65, yymsp[-4].minor.yy111, yymsp[-3].minor.yy4, NULL, NULL, yymsp[-2].minor.yy66, yymsp[-1].minor.yy88, yymsp[0].minor.yy147, NULL);
}
#line 1192 "grammar.c"
  yymsp[-5].minor.yy112 = yylhsminor.yy112;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 42 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-2].minor.yy65, yymsp[-1].minor.yy111, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1200 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 46 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-2].minor.yy65, yymsp[-1].minor.yy111, NULL, NULL, yymsp[0].minor.yy155, NULL, NULL, NULL, NULL);
}
#line 1208 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 50 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-2].minor.yy65, yymsp[-1].minor.yy111, NULL, yymsp[0].minor.yy98, NULL, NULL, NULL, NULL, NULL);
}
#line 1216 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
#line 54 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-5].minor.yy65, yymsp[-4].minor.yy111, NULL, yymsp[-3].minor.yy98, NULL, yymsp[-2].minor.yy66, yymsp[-1].minor.yy88, yymsp[0].minor.yy147, NULL);
}
#line 1224 "grammar.c"
  yymsp[-5].minor.yy112 = yylhsminor.yy112;
        break;
      case 6: /* expr ::= createClause */
#line 58 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1232 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 7: /* expr ::= indexClause */
#line 62 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy60);
}
#line 1240 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 8: /* matchClause ::= MATCH chains */
#line 68 "grammar.y"
{
	yymsp[-1].minor.yy65 = New_AST_MatchNode(yymsp[0].minor.yy156);
}
#line 1248 "grammar.c"
        break;
      case 9: /* createClause ::= */
#line 75 "grammar.y"
{
	yymsp[1].minor.yy4 = NULL;
}
#line 1255 "grammar.c"
        break;
      case 10: /* createClause ::= CREATE chains */
#line 79 "grammar.y"
{
	yymsp[-1].minor.yy4 = New_AST_CreateNode(yymsp[0].minor.yy156);
}
#line 1262 "grammar.c"
        break;
      case 11: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 85 "grammar.y"
{
  yylhsminor.yy60 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy15);
}
#line 1269 "grammar.c"
  yymsp[-4].minor.yy60 = yylhsminor.yy60;
        break;
      case 12: /* indexOpToken ::= CREATE */
#line 91 "grammar.y"
{ yymsp[0].minor.yy15 = CREATE_INDEX; }
#line 1275 "grammar.c"
        break;
      case 13: /* indexOpToken ::= DROP */
#line 92 "grammar.y"
{ yymsp[0].minor.yy15 = DROP_INDEX; }
#line 1280 "grammar.c"
        break;
      case 14: /* indexLabel ::= COLON UQSTRING */
#line 94 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1287 "grammar.c"
        break;
      case 15: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 98 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1294 "grammar.c"
        break;
      case 16: /* setClause ::= SET setList */
#line 103 "grammar.y"
{
	yymsp[-1].minor.yy98 = New_AST_SetNode(yymsp[0].minor.yy156);
}
#line 1301 "grammar.c"
        break;
      case 17: /* setList ::= setElement */
#line 108 "grammar.y"
{
	yylhsminor.yy156 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy156, yymsp[0].minor.yy61);
}
#line 1309 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 18: /* setList ::= setList COMMA setElement */
#line 112 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy156, yymsp[0].minor.yy61);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1318 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 19: /* setElement ::= variable EQ arithmetic_expression */
#line 118 "grammar.y"
{
	yylhsminor.yy61 = New_AST_SetElement(yymsp[-2].minor.yy102, yymsp[0].minor.yy154);
}
#line 1326 "grammar.c"
  yymsp[-2].minor.yy61 = yylhsminor.yy61;
        break;
      case 20: /* chain ::= node */
#line 124 "grammar.y"
{
	yylhsminor.yy156 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy156, yymsp[0].minor.yy63);
}
#line 1335 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 21: /* chain ::= chain link node */
#line 129 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy156, yymsp[-1].minor.yy123);
	Vector_Push(yymsp[-2].minor.yy156, yymsp[0].minor.yy63);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1345 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 22: /* chains ::= chain */
#line 136 "grammar.y"
{
	yylhsminor.yy156 = yymsp[0].minor.yy156;
}
#line 1353 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 23: /* chains ::= chains COMMA chain */
#line 140 "grammar.y"
{
	for(int i = 0; i < Vector_Size(yymsp[0].minor.yy156); i++) {
		AST_GraphEntity *entity;
		Vector_Get(yymsp[0].minor.yy156, i, &entity);
		Vector_Push(yymsp[-2].minor.yy156, entity);
	}
	Vector_Free(yymsp[0].minor.yy156);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1367 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 24: /* deleteClause ::= DELETE deleteExpression */
#line 153 "grammar.y"
{
	yymsp[-1].minor.yy155 = New_AST_DeleteNode(yymsp[0].minor.yy156);
}
#line 1375 "grammar.c"
        break;
      case 25: /* deleteExpression ::= UQSTRING */
#line 159 "grammar.y"
{
	yylhsminor.yy156 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy156, yymsp[0].minor.yy0.strval);
}
#line 1383 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 26: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 164 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy156, yymsp[0].minor.yy0.strval);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1392 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 27: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 172 "grammar.y"
{
	yymsp[-5].minor.yy63 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy156);
}
#line 1400 "grammar.c"
        break;
      case 28: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 177 "grammar.y"
{
	yymsp[-4].minor.yy63 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy156);
}
#line 1407 "grammar.c"
        break;
      case 29: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 182 "grammar.y"
{
	yymsp[-3].minor.yy63 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy156);
}
#line 1414 "grammar.c"
        break;
      case 30: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 187 "grammar.y"
{
	yymsp[-2].minor.yy63 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy156);
}
#line 1421 "grammar.c"
        break;
      case 31: /* link ::= DASH edge RIGHT_ARROW */
#line 194 "grammar.y"
{
	yymsp[-2].minor.yy123 = yymsp[-1].minor.yy123;
	yymsp[-2].minor.yy123->direction = N_LEFT_TO_RIGHT;
}
#line 1429 "grammar.c"
        break;
      case 32: /* link ::= LEFT_ARROW edge DASH */
#line 200 "grammar.y"
{
	yymsp[-2].minor.yy123 = yymsp[-1].minor.yy123;
	yymsp[-2].minor.yy123->direction = N_RIGHT_TO_LEFT;
}
#line 1437 "grammar.c"
        break;
      case 33: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 207 "grammar.y"
{ 
	yymsp[-2].minor.yy123 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy156, N_DIR_UNKNOWN);
}
#line 1444 "grammar.c"
        break;
      case 34: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 212 "grammar.y"
{ 
	yymsp[-3].minor.yy123 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy156, N_DIR_UNKNOWN);
}
#line 1451 "grammar.c"
        break;
      case 35: /* edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
#line 217 "grammar.y"
{ 
	yymsp[-4].minor.yy123 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy156, N_DIR_UNKNOWN);
}
#line 1458 "grammar.c"
        break;
      case 36: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 222 "grammar.y"
{ 
	yymsp[-5].minor.yy123 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy156, N_DIR_UNKNOWN);
}
#line 1465 "grammar.c"
        break;
      case 37: /* properties ::= */
#line 228 "grammar.y"
{
	yymsp[1].minor.yy156 = NULL;
}
#line 1472 "grammar.c"
        break;
      case 38: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 232 "grammar.y"
{
	yymsp[-2].minor.yy156 = yymsp[-1].minor.yy156;
}
#line 1479 "grammar.c"
        break;
      case 39: /* mapLiteral ::= UQSTRING COLON value */
#line 238 "grammar.y"
{
	yylhsminor.yy156 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy156, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy96;
	Vector_Push(yylhsminor.yy156, val);
}
#line 1494 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 40: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 250 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy156, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy96;
	Vector_Push(yymsp[0].minor.yy156, val);
	
	yylhsminor.yy156 = yymsp[0].minor.yy156;
}
#line 1510 "grammar.c"
  yymsp[-4].minor.yy156 = yylhsminor.yy156;
        break;
      case 41: /* whereClause ::= */
#line 264 "grammar.y"
{ 
	yymsp[1].minor.yy111 = NULL;
}
#line 1518 "grammar.c"
        break;
      case 42: /* whereClause ::= WHERE cond */
#line 267 "grammar.y"
{
	yymsp[-1].minor.yy111 = New_AST_WhereNode(yymsp[0].minor.yy64);
}
#line 1525 "grammar.c"
        break;
      case 43: /* cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
#line 276 "grammar.y"
{ yylhsminor.yy64 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy58, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1530 "grammar.c"
  yymsp[-6].minor.yy64 = yylhsminor.yy64;
        break;
      case 44: /* cond ::= UQSTRING DOT UQSTRING relation value */
#line 279 "grammar.y"
{ yylhsminor.yy64 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy58, yymsp[0].minor.yy96); }
#line 1536 "grammar.c"
  yymsp[-4].minor.yy64 = yylhsminor.yy64;
        break;
      case 45: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 280 "grammar.y"
{ yymsp[-2].minor.yy64 = yymsp[-1].minor.yy64; }
#line 1542 "grammar.c"
        break;
      case 46: /* cond ::= cond AND cond */
#line 281 "grammar.y"
{ yylhsminor.yy64 = New_AST_ConditionNode(yymsp[-2].minor.yy64, AND, yymsp[0].minor.yy64); }
#line 1547 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 47: /* cond ::= cond OR cond */
#line 282 "grammar.y"
{ yylhsminor.yy64 = New_AST_ConditionNode(yymsp[-2].minor.yy64, OR, yymsp[0].minor.yy64); }
#line 1553 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 48: /* returnClause ::= RETURN returnElements */
#line 287 "grammar.y"
{
	yymsp[-1].minor.yy66 = New_AST_ReturnNode(yymsp[0].minor.yy156, 0);
}
#line 1561 "grammar.c"
        break;
      case 49: /* returnClause ::= RETURN DISTINCT returnElements */
#line 290 "grammar.y"
{
	yymsp[-2].minor.yy66 = New_AST_ReturnNode(yymsp[0].minor.yy156, 1);
}
#line 1568 "grammar.c"
        break;
      case 50: /* returnElements ::= returnElements COMMA returnElement */
#line 297 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy156, yymsp[0].minor.yy84);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1576 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 51: /* returnElements ::= returnElement */
#line 302 "grammar.y"
{
	yylhsminor.yy156 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy156, yymsp[0].minor.yy84);
}
#line 1585 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 52: /* returnElement ::= arithmetic_expression */
#line 309 "grammar.y"
{
	yylhsminor.yy84 = New_AST_ReturnElementNode(yymsp[0].minor.yy154, NULL);
}
#line 1593 "grammar.c"
  yymsp[0].minor.yy84 = yylhsminor.yy84;
        break;
      case 53: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 314 "grammar.y"
{
	yylhsminor.yy84 = New_AST_ReturnElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 1601 "grammar.c"
  yymsp[-2].minor.yy84 = yylhsminor.yy84;
        break;
      case 54: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 321 "grammar.y"
{
	yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154;
}
#line 1609 "grammar.c"
        break;
      case 55: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 333 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1619 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 56: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 340 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1630 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 57: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 347 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1641 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 58: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 354 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1652 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 59: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 362 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy156);
}
#line 1660 "grammar.c"
  yymsp[-3].minor.yy154 = yylhsminor.yy154;
        break;
      case 60: /* arithmetic_expression ::= value */
#line 367 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy96);
}
#line 1668 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 61: /* arithmetic_expression ::= variable */
#line 372 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy102->alias, yymsp[0].minor.yy102->property);
}
#line 1676 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 62: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 378 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy156, yymsp[0].minor.yy154);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1685 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 63: /* arithmetic_expression_list ::= arithmetic_expression */
#line 382 "grammar.y"
{
	yylhsminor.yy156 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy156, yymsp[0].minor.yy154);
}
#line 1694 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 64: /* variable ::= UQSTRING */
#line 389 "grammar.y"
{
	yylhsminor.yy102 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1702 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 65: /* variable ::= UQSTRING DOT UQSTRING */
#line 393 "grammar.y"
{
	yylhsminor.yy102 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1710 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 66: /* orderClause ::= */
#line 399 "grammar.y"
{
	yymsp[1].minor.yy88 = NULL;
}
#line 1718 "grammar.c"
        break;
      case 67: /* orderClause ::= ORDER BY columnNameList */
#line 402 "grammar.y"
{
	yymsp[-2].minor.yy88 = New_AST_OrderNode(yymsp[0].minor.yy156, ORDER_DIR_ASC);
}
#line 1725 "grammar.c"
        break;
      case 68: /* orderClause ::= ORDER BY columnNameList ASC */
#line 405 "grammar.y"
{
	yymsp[-3].minor.yy88 = New_AST_OrderNode(yymsp[-1].minor.yy156, ORDER_DIR_ASC);
}
#line 1732 "grammar.c"
        break;
      case 69: /* orderClause ::= ORDER BY columnNameList DESC */
#line 408 "grammar.y"
{
	yymsp[-3].minor.yy88 = New_AST_OrderNode(yymsp[-1].minor.yy156, ORDER_DIR_DESC);
}
#line 1739 "grammar.c"
        break;
      case 70: /* columnNameList ::= columnNameList COMMA columnName */
#line 413 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy156, yymsp[0].minor.yy82);
	yylhsminor.yy156 = yymsp[-2].minor.yy156;
}
#line 1747 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 71: /* columnNameList ::= columnName */
#line 417 "grammar.y"
{
	yylhsminor.yy156 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy156, yymsp[0].minor.yy82);
}
#line 1756 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 72: /* columnName ::= variable */
#line 423 "grammar.y"
{
	if(yymsp[0].minor.yy102->property != NULL) {
		yylhsminor.yy82 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy102);
	} else {
		yylhsminor.yy82 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy102->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy102);
}
#line 1770 "grammar.c"
  yymsp[0].minor.yy82 = yylhsminor.yy82;
        break;
      case 73: /* limitClause ::= */
#line 435 "grammar.y"
{
	yymsp[1].minor.yy147 = NULL;
}
#line 1778 "grammar.c"
        break;
      case 74: /* limitClause ::= LIMIT INTEGER */
#line 438 "grammar.y"
{
	yymsp[-1].minor.yy147 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1785 "grammar.c"
        break;
      case 75: /* relation ::= EQ */
#line 444 "grammar.y"
{ yymsp[0].minor.yy58 = EQ; }
#line 1790 "grammar.c"
        break;
      case 76: /* relation ::= GT */
#line 445 "grammar.y"
{ yymsp[0].minor.yy58 = GT; }
#line 1795 "grammar.c"
        break;
      case 77: /* relation ::= LT */
#line 446 "grammar.y"
{ yymsp[0].minor.yy58 = LT; }
#line 1800 "grammar.c"
        break;
      case 78: /* relation ::= LE */
#line 447 "grammar.y"
{ yymsp[0].minor.yy58 = LE; }
#line 1805 "grammar.c"
        break;
      case 79: /* relation ::= GE */
#line 448 "grammar.y"
{ yymsp[0].minor.yy58 = GE; }
#line 1810 "grammar.c"
        break;
      case 80: /* relation ::= NE */
#line 449 "grammar.y"
{ yymsp[0].minor.yy58 = NE; }
#line 1815 "grammar.c"
        break;
      case 81: /* value ::= INTEGER */
#line 460 "grammar.y"
{  yylhsminor.yy96 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1820 "grammar.c"
  yymsp[0].minor.yy96 = yylhsminor.yy96;
        break;
      case 82: /* value ::= DASH INTEGER */
#line 461 "grammar.y"
{  yymsp[-1].minor.yy96 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1826 "grammar.c"
        break;
      case 83: /* value ::= STRING */
#line 462 "grammar.y"
{  yylhsminor.yy96 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1831 "grammar.c"
  yymsp[0].minor.yy96 = yylhsminor.yy96;
        break;
      case 84: /* value ::= FLOAT */
#line 463 "grammar.y"
{  yylhsminor.yy96 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1837 "grammar.c"
  yymsp[0].minor.yy96 = yylhsminor.yy96;
        break;
      case 85: /* value ::= DASH FLOAT */
#line 464 "grammar.y"
{  yymsp[-1].minor.yy96 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1843 "grammar.c"
        break;
      case 86: /* value ::= TRUE */
#line 465 "grammar.y"
{ yymsp[0].minor.yy96 = SI_BoolVal(1); }
#line 1848 "grammar.c"
        break;
      case 87: /* value ::= FALSE */
#line 466 "grammar.y"
{ yymsp[0].minor.yy96 = SI_BoolVal(0); }
#line 1853 "grammar.c"
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
#line 1918 "grammar.c"
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
#line 468 "grammar.y"


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
#line 2164 "grammar.c"
