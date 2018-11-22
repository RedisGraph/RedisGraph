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
	#include <limits.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "./clauses/clauses.h"
	#include "parse.h"
	#include "../value.h"

	void yyerror(char *s);

	/*
	**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
	**                       zero the stack is dynamically sized using realloc()
	*/
	// Increase depth from 100 to 1000 to handel deep recursion.
	#define YYSTACKDEPTH 1000
#line 49 "grammar.c"
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
#define YYNOCODE 91
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_SkipNode* yy3;
  AST_NodeEntity* yy9;
  AST_ColumnNode* yy10;
  AST_MergeNode* yy20;
  AST_IndexNode* yy24;
  AST_SetElement* yy25;
  AST_LinkLength* yy30;
  AST_FilterNode* yy46;
  AST_ReturnNode* yy48;
  AST_MatchNode* yy65;
  Vector* yy66;
  AST_CreateNode* yy76;
  SIValue yy78;
  AST_SetNode* yy80;
  AST_OrderNode* yy88;
  AST_UnwindNode* yy97;
  AST_IndexOpType yy105;
  AST_LinkEntity* yy106;
  AST_WhereNode* yy111;
  AST_Query* yy112;
  int yy113;
  AST_Variable* yy120;
  AST_LimitNode* yy147;
  AST_ArithmeticExpressionNode* yy154;
  AST_DeleteNode * yy155;
  AST_ReturnElementNode* yy174;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             124
#define YYNRULE              103
#define YYNTOKEN             51
#define YY_MAX_SHIFT         123
#define YY_MIN_SHIFTREDUCE   192
#define YY_MAX_SHIFTREDUCE   294
#define YY_ERROR_ACTION      295
#define YY_ACCEPT_ACTION     296
#define YY_NO_ACTION         297
#define YY_MIN_REDUCE        298
#define YY_MAX_REDUCE        400
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
#define YY_ACTTAB_COUNT (312)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    10,   62,  305,   42,   76,  254,  301,   58,  109,   17,
 /*    10 */    15,   14,   13,  281,  282,  285,  283,  284,   78,   16,
 /*    20 */    17,   15,   14,   13,  370,   65,  257,  381,  119,  287,
 /*    30 */    20,   19,   33,  369,  209,    2,  117,  359,   48,   25,
 /*    40 */    76,  254,   71,  380,  370,   65,  289,  290,  292,  293,
 /*    50 */   294,  286,    1,  369,   78,   16,  115,  359,   17,   15,
 /*    60 */    14,   13,   82,   28,  105,  287,   93,  320,   92,   17,
 /*    70 */    15,   14,   13,  281,  282,  285,  283,  284,   49,   54,
 /*    80 */   271,  272,  289,  290,  292,  293,  294,   76,   85,   96,
 /*    90 */    36,  116,   76,  123,  296,   63,  313,  304,  308,   76,
 /*   100 */    44,   78,   16,   52,   24,  306,  307,   99,   87,  107,
 /*   110 */    37,  286,  287,   78,    6,  108,  114,  287,  370,   30,
 /*   120 */    35,    5,    7,   52,  287,  323,   83,  369,   89,  289,
 /*   130 */   290,  292,  293,  294,  289,  290,  292,  293,  294,   39,
 /*   140 */   247,  289,  290,  292,  293,  294,   17,   15,   14,   13,
 /*   150 */    22,  370,   65,  122,   22,  102,  100,  370,   69,   21,
 /*   160 */   369,   43,  288,  257,  358,  309,  369,  370,   29,  370,
 /*   170 */    30,   72,  370,   30,   41,    1,  369,   70,  369,  354,
 /*   180 */   291,  369,   73,  321,   92,  370,   69,  370,   66,    5,
 /*   190 */     7,  370,   67,   34,  369,  381,  369,    9,  323,   77,
 /*   200 */   369,  370,   68,  370,  367,  370,  366,  370,   79,   98,
 /*   210 */   369,  379,  369,  121,  369,  118,  369,  370,   64,  370,
 /*   220 */    75,  113,   38,   52,  222,   88,  369,  323,  369,   35,
 /*   230 */    91,   12,   14,   13,  323,   52,   81,  262,   32,   94,
 /*   240 */    12,   52,   95,   74,   97,   45,  106,  103,  104,  110,
 /*   250 */   324,  349,  112,  111,  303,   55,  120,   56,    1,   57,
 /*   260 */   299,    8,   59,   60,   61,  280,   80,   18,    4,  210,
 /*   270 */   211,   84,   40,   86,    7,   27,  223,   90,   11,   26,
 /*   280 */   229,   46,   47,  232,   23,  118,  298,   50,  233,  231,
 /*   290 */   227,  237,   31,  228,  235,  230,  101,  225,  226,   53,
 /*   300 */   224,  297,   51,  277,  297,  241,    3,  256,  268,  297,
 /*   310 */   297,  279,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    84,   56,   56,   57,    4,    5,   61,   62,   82,    3,
 /*    10 */     4,    5,    6,    7,    8,    9,   10,   11,   18,   19,
 /*    20 */     3,    4,    5,    6,   73,   74,   20,   73,   18,   29,
 /*    30 */    12,   13,   19,   82,   16,   35,   85,   86,   78,   21,
 /*    40 */     4,    5,   88,   89,   73,   74,   46,   47,   48,   49,
 /*    50 */    50,   45,   34,   82,   18,   19,   85,   86,    3,    4,
 /*    60 */     5,    6,   44,   23,   78,   29,   71,   72,   73,    3,
 /*    70 */     4,    5,    6,    7,    8,    9,   10,   11,    4,   76,
 /*    80 */    40,   41,   46,   47,   48,   49,   50,    4,   19,   17,
 /*    90 */    18,   36,    4,   52,   53,   54,   69,   56,   57,    4,
 /*   100 */    26,   18,   19,   31,   63,   64,   65,   78,   67,   17,
 /*   110 */    18,   45,   29,   18,   19,   78,   66,   29,   73,   74,
 /*   120 */    70,    1,    2,   31,   29,   75,   17,   82,   83,   46,
 /*   130 */    47,   48,   49,   50,   46,   47,   48,   49,   50,   68,
 /*   140 */    20,   46,   47,   48,   49,   50,    3,    4,    5,    6,
 /*   150 */    13,   73,   74,   43,   13,   29,   30,   73,   74,   22,
 /*   160 */    82,   24,   29,   20,   86,   60,   82,   73,   74,   73,
 /*   170 */    74,   87,   73,   74,   59,   34,   82,   83,   82,   83,
 /*   180 */    47,   82,   83,   72,   73,   73,   74,   73,   74,    1,
 /*   190 */     2,   73,   74,   70,   82,   73,   82,   19,   75,   87,
 /*   200 */    82,   73,   74,   73,   74,   73,   74,   73,   74,   17,
 /*   210 */    82,   89,   82,   42,   82,   37,   82,   73,   74,   73,
 /*   220 */    74,   17,   70,   31,   18,   66,   82,   75,   82,   70,
 /*   230 */    77,   23,    5,    6,   75,   31,   28,   20,   27,   79,
 /*   240 */    23,   31,   78,    5,   78,   80,   78,   80,   79,   18,
 /*   250 */    75,   81,   78,   81,   60,   59,   38,   58,   34,   57,
 /*   260 */    60,   33,   59,   58,   57,   18,   36,   55,   27,   18,
 /*   270 */    20,   18,   15,   14,    2,   23,   18,   23,    7,   23,
 /*   280 */     4,   18,   18,   28,   39,   37,    0,   18,   28,   28,
 /*   290 */    20,   29,   17,   25,   29,   28,   30,   20,   20,   18,
 /*   300 */    20,   90,   23,   29,   90,   32,   23,   18,   18,   90,
 /*   310 */    90,   29,   90,   90,   90,   90,   90,   90,   90,   90,
 /*   320 */    90,   90,   90,   90,   90,   90,   90,   90,   90,   90,
 /*   330 */    90,   90,   90,   90,   90,   90,   90,   90,   90,   90,
 /*   340 */    90,   90,   90,   90,   90,   90,   90,   90,   90,   90,
 /*   350 */    90,   90,   90,   90,   90,   90,   90,   90,   90,   90,
 /*   360 */    90,   90,   90,
};
#define YY_SHIFT_COUNT    (123)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (290)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    18,    0,   36,   36,   83,   95,   95,   95,   95,   83,
 /*    10 */    83,   83,   83,   83,   83,   83,   83,   83,  137,   13,
 /*    20 */    13,   10,   13,   10,  141,   13,   10,   13,   10,    6,
 /*    30 */    66,   88,   72,   92,   74,   74,  192,  204,   74,   69,
 /*    40 */   109,  110,  171,  206,  211,  210,  238,  210,  238,  211,
 /*    50 */   210,  231,  231,  210,   13,  110,  171,  218,  224,  110,
 /*    60 */   171,  218,  224,  228,  143,   55,   17,   17,   17,   17,
 /*    70 */   120,   40,  208,  188,  126,  227,  133,  217,  178,  227,
 /*    80 */   247,  230,  241,  251,  250,  253,  257,  259,  252,  272,
 /*    90 */   258,  254,  271,  256,  276,  255,  263,  260,  264,  261,
 /*   100 */   262,  265,  266,  267,  268,  270,  277,  269,  278,  279,
 /*   110 */   275,  273,  280,  281,  252,  283,  289,  283,  290,  248,
 /*   120 */   245,  274,  282,  286,
};
#define YY_REDUCE_COUNT (63)
#define YY_REDUCE_MIN   (-84)
#define YY_REDUCE_MAX   (212)
static const short yy_reduce_ofst[] = {
 /*     0 */    41,  -49,  -29,   78,   84,   45,   94,   96,   99,  112,
 /*    10 */   114,  118,  128,  130,  132,  134,  144,  146,  -55,   50,
 /*    20 */   159,   -5,   50,  -46,  -54,  123,  111,  152,  122,  -84,
 /*    30 */   -84,  -74,  -40,  -14,    3,    3,   29,   37,    3,   27,
 /*    40 */    71,  105,  115,  153,  160,  164,  165,  166,  167,  169,
 /*    50 */   168,  170,  172,  174,  175,  194,  196,  199,  202,  200,
 /*    60 */   203,  205,  207,  212,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   311,  295,  295,  295,  295,  295,  295,  295,  295,  295,
 /*    10 */   295,  295,  295,  295,  295,  295,  295,  295,  311,  314,
 /*    20 */   295,  295,  295,  295,  311,  295,  295,  295,  295,  295,
 /*    30 */   295,  295,  346,  346,  318,  325,  346,  346,  326,  295,
 /*    40 */   295,  384,  382,  295,  295,  346,  340,  346,  340,  295,
 /*    50 */   346,  295,  295,  346,  295,  384,  382,  375,  302,  384,
 /*    60 */   382,  375,  300,  350,  295,  361,  352,  322,  371,  372,
 /*    70 */   295,  376,  295,  351,  345,  364,  295,  295,  373,  365,
 /*    80 */   295,  295,  295,  295,  295,  295,  295,  295,  310,  355,
 /*    90 */   295,  327,  295,  319,  295,  295,  295,  295,  295,  295,
 /*   100 */   295,  342,  344,  295,  295,  295,  295,  295,  295,  348,
 /*   110 */   295,  295,  295,  295,  312,  357,  295,  356,  295,  373,
 /*   120 */   295,  295,  295,  295,
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
  /*   29 */ "INTEGER",
  /*   30 */ "DOTDOT",
  /*   31 */ "LEFT_CURLY_BRACKET",
  /*   32 */ "RIGHT_CURLY_BRACKET",
  /*   33 */ "WHERE",
  /*   34 */ "RETURN",
  /*   35 */ "DISTINCT",
  /*   36 */ "AS",
  /*   37 */ "DOT",
  /*   38 */ "ORDER",
  /*   39 */ "BY",
  /*   40 */ "ASC",
  /*   41 */ "DESC",
  /*   42 */ "SKIP",
  /*   43 */ "LIMIT",
  /*   44 */ "UNWIND",
  /*   45 */ "NE",
  /*   46 */ "STRING",
  /*   47 */ "FLOAT",
  /*   48 */ "TRUE",
  /*   49 */ "FALSE",
  /*   50 */ "NULLVAL",
  /*   51 */ "error",
  /*   52 */ "expr",
  /*   53 */ "query",
  /*   54 */ "matchClause",
  /*   55 */ "whereClause",
  /*   56 */ "createClause",
  /*   57 */ "returnClause",
  /*   58 */ "orderClause",
  /*   59 */ "skipClause",
  /*   60 */ "limitClause",
  /*   61 */ "deleteClause",
  /*   62 */ "setClause",
  /*   63 */ "unwindClause",
  /*   64 */ "indexClause",
  /*   65 */ "mergeClause",
  /*   66 */ "chains",
  /*   67 */ "indexOpToken",
  /*   68 */ "indexLabel",
  /*   69 */ "indexProp",
  /*   70 */ "chain",
  /*   71 */ "setList",
  /*   72 */ "setElement",
  /*   73 */ "variable",
  /*   74 */ "arithmetic_expression",
  /*   75 */ "node",
  /*   76 */ "link",
  /*   77 */ "deleteExpression",
  /*   78 */ "properties",
  /*   79 */ "edge",
  /*   80 */ "edgeLength",
  /*   81 */ "mapLiteral",
  /*   82 */ "value",
  /*   83 */ "cond",
  /*   84 */ "relation",
  /*   85 */ "returnElements",
  /*   86 */ "returnElement",
  /*   87 */ "arithmetic_expression_list",
  /*   88 */ "columnNameList",
  /*   89 */ "columnName",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause",
 /*   2 */ "expr ::= matchClause whereClause createClause",
 /*   3 */ "expr ::= matchClause whereClause deleteClause",
 /*   4 */ "expr ::= matchClause whereClause setClause",
 /*   5 */ "expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause",
 /*   6 */ "expr ::= createClause",
 /*   7 */ "expr ::= unwindClause createClause",
 /*   8 */ "expr ::= indexClause",
 /*   9 */ "expr ::= mergeClause",
 /*  10 */ "expr ::= returnClause",
 /*  11 */ "expr ::= unwindClause returnClause skipClause limitClause",
 /*  12 */ "matchClause ::= MATCH chains",
 /*  13 */ "createClause ::=",
 /*  14 */ "createClause ::= CREATE chains",
 /*  15 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  16 */ "indexOpToken ::= CREATE",
 /*  17 */ "indexOpToken ::= DROP",
 /*  18 */ "indexLabel ::= COLON UQSTRING",
 /*  19 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  20 */ "mergeClause ::= MERGE chain",
 /*  21 */ "setClause ::= SET setList",
 /*  22 */ "setList ::= setElement",
 /*  23 */ "setList ::= setList COMMA setElement",
 /*  24 */ "setElement ::= variable EQ arithmetic_expression",
 /*  25 */ "chain ::= node",
 /*  26 */ "chain ::= chain link node",
 /*  27 */ "chains ::= chain",
 /*  28 */ "chains ::= chains COMMA chain",
 /*  29 */ "deleteClause ::= DELETE deleteExpression",
 /*  30 */ "deleteExpression ::= UQSTRING",
 /*  31 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  32 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  33 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  34 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  35 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  36 */ "link ::= DASH edge RIGHT_ARROW",
 /*  37 */ "link ::= LEFT_ARROW edge DASH",
 /*  38 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  39 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  40 */ "edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET",
 /*  41 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  42 */ "edgeLength ::=",
 /*  43 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  44 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  45 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  46 */ "edgeLength ::= MUL INTEGER",
 /*  47 */ "edgeLength ::= MUL",
 /*  48 */ "properties ::=",
 /*  49 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  50 */ "mapLiteral ::= UQSTRING COLON value",
 /*  51 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  52 */ "whereClause ::=",
 /*  53 */ "whereClause ::= WHERE cond",
 /*  54 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  55 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  56 */ "cond ::= cond AND cond",
 /*  57 */ "cond ::= cond OR cond",
 /*  58 */ "returnClause ::= RETURN returnElements",
 /*  59 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  60 */ "returnElements ::= returnElements COMMA returnElement",
 /*  61 */ "returnElements ::= returnElement",
 /*  62 */ "returnElement ::= MUL",
 /*  63 */ "returnElement ::= arithmetic_expression",
 /*  64 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  65 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  66 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  67 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  68 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  69 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  70 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  71 */ "arithmetic_expression ::= value",
 /*  72 */ "arithmetic_expression ::= variable",
 /*  73 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  74 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  75 */ "variable ::= UQSTRING",
 /*  76 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  77 */ "orderClause ::=",
 /*  78 */ "orderClause ::= ORDER BY columnNameList",
 /*  79 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  80 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  81 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  82 */ "columnNameList ::= columnName",
 /*  83 */ "columnName ::= variable",
 /*  84 */ "skipClause ::=",
 /*  85 */ "skipClause ::= SKIP INTEGER",
 /*  86 */ "limitClause ::=",
 /*  87 */ "limitClause ::= LIMIT INTEGER",
 /*  88 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /*  89 */ "relation ::= EQ",
 /*  90 */ "relation ::= GT",
 /*  91 */ "relation ::= LT",
 /*  92 */ "relation ::= LE",
 /*  93 */ "relation ::= GE",
 /*  94 */ "relation ::= NE",
 /*  95 */ "value ::= INTEGER",
 /*  96 */ "value ::= DASH INTEGER",
 /*  97 */ "value ::= STRING",
 /*  98 */ "value ::= FLOAT",
 /*  99 */ "value ::= DASH FLOAT",
 /* 100 */ "value ::= TRUE",
 /* 101 */ "value ::= FALSE",
 /* 102 */ "value ::= NULLVAL",
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
    case 83: /* cond */
{
#line 332 "grammar.y"
 Free_AST_FilterNode((yypminor->yy46)); 
#line 774 "grammar.c"
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
  {   53,   -1 }, /* (0) query ::= expr */
  {   52,   -7 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
  {   52,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   52,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   52,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   52,   -7 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   52,   -1 }, /* (6) expr ::= createClause */
  {   52,   -2 }, /* (7) expr ::= unwindClause createClause */
  {   52,   -1 }, /* (8) expr ::= indexClause */
  {   52,   -1 }, /* (9) expr ::= mergeClause */
  {   52,   -1 }, /* (10) expr ::= returnClause */
  {   52,   -4 }, /* (11) expr ::= unwindClause returnClause skipClause limitClause */
  {   54,   -2 }, /* (12) matchClause ::= MATCH chains */
  {   56,    0 }, /* (13) createClause ::= */
  {   56,   -2 }, /* (14) createClause ::= CREATE chains */
  {   64,   -5 }, /* (15) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   67,   -1 }, /* (16) indexOpToken ::= CREATE */
  {   67,   -1 }, /* (17) indexOpToken ::= DROP */
  {   68,   -2 }, /* (18) indexLabel ::= COLON UQSTRING */
  {   69,   -3 }, /* (19) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   65,   -2 }, /* (20) mergeClause ::= MERGE chain */
  {   62,   -2 }, /* (21) setClause ::= SET setList */
  {   71,   -1 }, /* (22) setList ::= setElement */
  {   71,   -3 }, /* (23) setList ::= setList COMMA setElement */
  {   72,   -3 }, /* (24) setElement ::= variable EQ arithmetic_expression */
  {   70,   -1 }, /* (25) chain ::= node */
  {   70,   -3 }, /* (26) chain ::= chain link node */
  {   66,   -1 }, /* (27) chains ::= chain */
  {   66,   -3 }, /* (28) chains ::= chains COMMA chain */
  {   61,   -2 }, /* (29) deleteClause ::= DELETE deleteExpression */
  {   77,   -1 }, /* (30) deleteExpression ::= UQSTRING */
  {   77,   -3 }, /* (31) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   75,   -6 }, /* (32) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   75,   -5 }, /* (33) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   75,   -4 }, /* (34) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   75,   -3 }, /* (35) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   76,   -3 }, /* (36) link ::= DASH edge RIGHT_ARROW */
  {   76,   -3 }, /* (37) link ::= LEFT_ARROW edge DASH */
  {   79,   -4 }, /* (38) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   79,   -4 }, /* (39) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   79,   -6 }, /* (40) edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
  {   79,   -6 }, /* (41) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   80,    0 }, /* (42) edgeLength ::= */
  {   80,   -4 }, /* (43) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   80,   -3 }, /* (44) edgeLength ::= MUL INTEGER DOTDOT */
  {   80,   -3 }, /* (45) edgeLength ::= MUL DOTDOT INTEGER */
  {   80,   -2 }, /* (46) edgeLength ::= MUL INTEGER */
  {   80,   -1 }, /* (47) edgeLength ::= MUL */
  {   78,    0 }, /* (48) properties ::= */
  {   78,   -3 }, /* (49) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   81,   -3 }, /* (50) mapLiteral ::= UQSTRING COLON value */
  {   81,   -5 }, /* (51) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   55,    0 }, /* (52) whereClause ::= */
  {   55,   -2 }, /* (53) whereClause ::= WHERE cond */
  {   83,   -3 }, /* (54) cond ::= arithmetic_expression relation arithmetic_expression */
  {   83,   -3 }, /* (55) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   83,   -3 }, /* (56) cond ::= cond AND cond */
  {   83,   -3 }, /* (57) cond ::= cond OR cond */
  {   57,   -2 }, /* (58) returnClause ::= RETURN returnElements */
  {   57,   -3 }, /* (59) returnClause ::= RETURN DISTINCT returnElements */
  {   85,   -3 }, /* (60) returnElements ::= returnElements COMMA returnElement */
  {   85,   -1 }, /* (61) returnElements ::= returnElement */
  {   86,   -1 }, /* (62) returnElement ::= MUL */
  {   86,   -1 }, /* (63) returnElement ::= arithmetic_expression */
  {   86,   -3 }, /* (64) returnElement ::= arithmetic_expression AS UQSTRING */
  {   74,   -3 }, /* (65) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   74,   -3 }, /* (66) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   74,   -3 }, /* (67) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   74,   -3 }, /* (68) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   74,   -3 }, /* (69) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   74,   -4 }, /* (70) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   74,   -1 }, /* (71) arithmetic_expression ::= value */
  {   74,   -1 }, /* (72) arithmetic_expression ::= variable */
  {   87,   -3 }, /* (73) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   87,   -1 }, /* (74) arithmetic_expression_list ::= arithmetic_expression */
  {   73,   -1 }, /* (75) variable ::= UQSTRING */
  {   73,   -3 }, /* (76) variable ::= UQSTRING DOT UQSTRING */
  {   58,    0 }, /* (77) orderClause ::= */
  {   58,   -3 }, /* (78) orderClause ::= ORDER BY columnNameList */
  {   58,   -4 }, /* (79) orderClause ::= ORDER BY columnNameList ASC */
  {   58,   -4 }, /* (80) orderClause ::= ORDER BY columnNameList DESC */
  {   88,   -3 }, /* (81) columnNameList ::= columnNameList COMMA columnName */
  {   88,   -1 }, /* (82) columnNameList ::= columnName */
  {   89,   -1 }, /* (83) columnName ::= variable */
  {   59,    0 }, /* (84) skipClause ::= */
  {   59,   -2 }, /* (85) skipClause ::= SKIP INTEGER */
  {   60,    0 }, /* (86) limitClause ::= */
  {   60,   -2 }, /* (87) limitClause ::= LIMIT INTEGER */
  {   63,   -6 }, /* (88) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {   84,   -1 }, /* (89) relation ::= EQ */
  {   84,   -1 }, /* (90) relation ::= GT */
  {   84,   -1 }, /* (91) relation ::= LT */
  {   84,   -1 }, /* (92) relation ::= LE */
  {   84,   -1 }, /* (93) relation ::= GE */
  {   84,   -1 }, /* (94) relation ::= NE */
  {   82,   -1 }, /* (95) value ::= INTEGER */
  {   82,   -2 }, /* (96) value ::= DASH INTEGER */
  {   82,   -1 }, /* (97) value ::= STRING */
  {   82,   -1 }, /* (98) value ::= FLOAT */
  {   82,   -2 }, /* (99) value ::= DASH FLOAT */
  {   82,   -1 }, /* (100) value ::= TRUE */
  {   82,   -1 }, /* (101) value ::= FALSE */
  {   82,   -1 }, /* (102) value ::= NULLVAL */
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
#line 44 "grammar.y"
{ ctx->root = yymsp[0].minor.yy112; }
#line 1254 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
#line 46 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-6].minor.yy65, yymsp[-5].minor.yy111, yymsp[-4].minor.yy76, NULL, NULL, NULL, yymsp[-3].minor.yy48, yymsp[-2].minor.yy88, yymsp[-1].minor.yy3, yymsp[0].minor.yy147, NULL, NULL);
}
#line 1261 "grammar.c"
  yymsp[-6].minor.yy112 = yylhsminor.yy112;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 50 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-2].minor.yy65, yymsp[-1].minor.yy111, yymsp[0].minor.yy76, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1269 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 54 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-2].minor.yy65, yymsp[-1].minor.yy111, NULL, NULL, NULL, yymsp[0].minor.yy155, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1277 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 58 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-2].minor.yy65, yymsp[-1].minor.yy111, NULL, NULL, yymsp[0].minor.yy80, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1285 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 62 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(yymsp[-6].minor.yy65, yymsp[-5].minor.yy111, NULL, NULL, yymsp[-4].minor.yy80, NULL, yymsp[-3].minor.yy48, yymsp[-2].minor.yy88, yymsp[-1].minor.yy3, yymsp[0].minor.yy147, NULL, NULL);
}
#line 1293 "grammar.c"
  yymsp[-6].minor.yy112 = yylhsminor.yy112;
        break;
      case 6: /* expr ::= createClause */
#line 66 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy76, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1301 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 7: /* expr ::= unwindClause createClause */
#line 70 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy76, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy97);
}
#line 1309 "grammar.c"
  yymsp[-1].minor.yy112 = yylhsminor.yy112;
        break;
      case 8: /* expr ::= indexClause */
#line 74 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy24, NULL);
}
#line 1317 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 9: /* expr ::= mergeClause */
#line 78 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1325 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 10: /* expr ::= returnClause */
#line 82 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy48, NULL, NULL, NULL, NULL, NULL);
}
#line 1333 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 11: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 86 "grammar.y"
{
	yylhsminor.yy112 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy48, NULL, yymsp[-1].minor.yy3, yymsp[0].minor.yy147, NULL, yymsp[-3].minor.yy97);
}
#line 1341 "grammar.c"
  yymsp[-3].minor.yy112 = yylhsminor.yy112;
        break;
      case 12: /* matchClause ::= MATCH chains */
#line 92 "grammar.y"
{
	yymsp[-1].minor.yy65 = New_AST_MatchNode(yymsp[0].minor.yy66);
}
#line 1349 "grammar.c"
        break;
      case 13: /* createClause ::= */
#line 99 "grammar.y"
{
	yymsp[1].minor.yy76 = NULL;
}
#line 1356 "grammar.c"
        break;
      case 14: /* createClause ::= CREATE chains */
#line 103 "grammar.y"
{
	yymsp[-1].minor.yy76 = New_AST_CreateNode(yymsp[0].minor.yy66);
}
#line 1363 "grammar.c"
        break;
      case 15: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 109 "grammar.y"
{
  yylhsminor.yy24 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy105);
}
#line 1370 "grammar.c"
  yymsp[-4].minor.yy24 = yylhsminor.yy24;
        break;
      case 16: /* indexOpToken ::= CREATE */
#line 115 "grammar.y"
{ yymsp[0].minor.yy105 = CREATE_INDEX; }
#line 1376 "grammar.c"
        break;
      case 17: /* indexOpToken ::= DROP */
#line 116 "grammar.y"
{ yymsp[0].minor.yy105 = DROP_INDEX; }
#line 1381 "grammar.c"
        break;
      case 18: /* indexLabel ::= COLON UQSTRING */
#line 118 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1388 "grammar.c"
        break;
      case 19: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 122 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1395 "grammar.c"
        break;
      case 20: /* mergeClause ::= MERGE chain */
#line 128 "grammar.y"
{
	yymsp[-1].minor.yy20 = New_AST_MergeNode(yymsp[0].minor.yy66);
}
#line 1402 "grammar.c"
        break;
      case 21: /* setClause ::= SET setList */
#line 133 "grammar.y"
{
	yymsp[-1].minor.yy80 = New_AST_SetNode(yymsp[0].minor.yy66);
}
#line 1409 "grammar.c"
        break;
      case 22: /* setList ::= setElement */
#line 138 "grammar.y"
{
	yylhsminor.yy66 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy25);
}
#line 1417 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 23: /* setList ::= setList COMMA setElement */
#line 142 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy25);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1426 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 24: /* setElement ::= variable EQ arithmetic_expression */
#line 148 "grammar.y"
{
	yylhsminor.yy25 = New_AST_SetElement(yymsp[-2].minor.yy120, yymsp[0].minor.yy154);
}
#line 1434 "grammar.c"
  yymsp[-2].minor.yy25 = yylhsminor.yy25;
        break;
      case 25: /* chain ::= node */
#line 154 "grammar.y"
{
	yylhsminor.yy66 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy9);
}
#line 1443 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 26: /* chain ::= chain link node */
#line 159 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[-1].minor.yy106);
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy9);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1453 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 27: /* chains ::= chain */
#line 167 "grammar.y"
{
	yylhsminor.yy66 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy66);
}
#line 1462 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 28: /* chains ::= chains COMMA chain */
#line 172 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy66);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1471 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 29: /* deleteClause ::= DELETE deleteExpression */
#line 180 "grammar.y"
{
	yymsp[-1].minor.yy155 = New_AST_DeleteNode(yymsp[0].minor.yy66);
}
#line 1479 "grammar.c"
        break;
      case 30: /* deleteExpression ::= UQSTRING */
#line 186 "grammar.y"
{
	yylhsminor.yy66 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy0.strval);
}
#line 1487 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 31: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 191 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy0.strval);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1496 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 32: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 199 "grammar.y"
{
	yymsp[-5].minor.yy9 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy66);
}
#line 1504 "grammar.c"
        break;
      case 33: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 204 "grammar.y"
{
	yymsp[-4].minor.yy9 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy66);
}
#line 1511 "grammar.c"
        break;
      case 34: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 209 "grammar.y"
{
	yymsp[-3].minor.yy9 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy66);
}
#line 1518 "grammar.c"
        break;
      case 35: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 214 "grammar.y"
{
	yymsp[-2].minor.yy9 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy66);
}
#line 1525 "grammar.c"
        break;
      case 36: /* link ::= DASH edge RIGHT_ARROW */
#line 221 "grammar.y"
{
	yymsp[-2].minor.yy106 = yymsp[-1].minor.yy106;
	yymsp[-2].minor.yy106->direction = N_LEFT_TO_RIGHT;
}
#line 1533 "grammar.c"
        break;
      case 37: /* link ::= LEFT_ARROW edge DASH */
#line 227 "grammar.y"
{
	yymsp[-2].minor.yy106 = yymsp[-1].minor.yy106;
	yymsp[-2].minor.yy106->direction = N_RIGHT_TO_LEFT;
}
#line 1541 "grammar.c"
        break;
      case 38: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 234 "grammar.y"
{ 
	yymsp[-3].minor.yy106 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy66, N_DIR_UNKNOWN, yymsp[-1].minor.yy30);
}
#line 1548 "grammar.c"
        break;
      case 39: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 239 "grammar.y"
{ 
	yymsp[-3].minor.yy106 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy66, N_DIR_UNKNOWN, NULL);
}
#line 1555 "grammar.c"
        break;
      case 40: /* edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
#line 244 "grammar.y"
{ 
	yymsp[-5].minor.yy106 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy66, N_DIR_UNKNOWN, yymsp[-2].minor.yy30);
}
#line 1562 "grammar.c"
        break;
      case 41: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 249 "grammar.y"
{ 
	yymsp[-5].minor.yy106 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy66, N_DIR_UNKNOWN, NULL);
}
#line 1569 "grammar.c"
        break;
      case 42: /* edgeLength ::= */
#line 256 "grammar.y"
{
	yymsp[1].minor.yy30 = NULL;
}
#line 1576 "grammar.c"
        break;
      case 43: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 261 "grammar.y"
{
	yymsp[-3].minor.yy30 = New_AST_LinkLength(yymsp[-2].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1583 "grammar.c"
        break;
      case 44: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 266 "grammar.y"
{
	yymsp[-2].minor.yy30 = New_AST_LinkLength(yymsp[-1].minor.yy0.intval, UINT_MAX-2);
}
#line 1590 "grammar.c"
        break;
      case 45: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 271 "grammar.y"
{
	yymsp[-2].minor.yy30 = New_AST_LinkLength(1, yymsp[0].minor.yy0.intval);
}
#line 1597 "grammar.c"
        break;
      case 46: /* edgeLength ::= MUL INTEGER */
#line 276 "grammar.y"
{
	yymsp[-1].minor.yy30 = New_AST_LinkLength(yymsp[0].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1604 "grammar.c"
        break;
      case 47: /* edgeLength ::= MUL */
#line 281 "grammar.y"
{
	yymsp[0].minor.yy30 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1611 "grammar.c"
        break;
      case 48: /* properties ::= */
#line 287 "grammar.y"
{
	yymsp[1].minor.yy66 = NULL;
}
#line 1618 "grammar.c"
        break;
      case 49: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 291 "grammar.y"
{
	yymsp[-2].minor.yy66 = yymsp[-1].minor.yy66;
}
#line 1625 "grammar.c"
        break;
      case 50: /* mapLiteral ::= UQSTRING COLON value */
#line 297 "grammar.y"
{
	yylhsminor.yy66 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy66, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy78;
	Vector_Push(yylhsminor.yy66, val);
}
#line 1640 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 51: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 309 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy66, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy78;
	Vector_Push(yymsp[0].minor.yy66, val);
	
	yylhsminor.yy66 = yymsp[0].minor.yy66;
}
#line 1656 "grammar.c"
  yymsp[-4].minor.yy66 = yylhsminor.yy66;
        break;
      case 52: /* whereClause ::= */
#line 323 "grammar.y"
{ 
	yymsp[1].minor.yy111 = NULL;
}
#line 1664 "grammar.c"
        break;
      case 53: /* whereClause ::= WHERE cond */
#line 326 "grammar.y"
{
	yymsp[-1].minor.yy111 = New_AST_WhereNode(yymsp[0].minor.yy46);
}
#line 1671 "grammar.c"
        break;
      case 54: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 335 "grammar.y"
{ yylhsminor.yy46 = New_AST_PredicateNode(yymsp[-2].minor.yy154, yymsp[-1].minor.yy113, yymsp[0].minor.yy154); }
#line 1676 "grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 55: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 337 "grammar.y"
{ yymsp[-2].minor.yy46 = yymsp[-1].minor.yy46; }
#line 1682 "grammar.c"
        break;
      case 56: /* cond ::= cond AND cond */
#line 338 "grammar.y"
{ yylhsminor.yy46 = New_AST_ConditionNode(yymsp[-2].minor.yy46, AND, yymsp[0].minor.yy46); }
#line 1687 "grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 57: /* cond ::= cond OR cond */
#line 339 "grammar.y"
{ yylhsminor.yy46 = New_AST_ConditionNode(yymsp[-2].minor.yy46, OR, yymsp[0].minor.yy46); }
#line 1693 "grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 58: /* returnClause ::= RETURN returnElements */
#line 343 "grammar.y"
{
	yymsp[-1].minor.yy48 = New_AST_ReturnNode(yymsp[0].minor.yy66, 0);
}
#line 1701 "grammar.c"
        break;
      case 59: /* returnClause ::= RETURN DISTINCT returnElements */
#line 346 "grammar.y"
{
	yymsp[-2].minor.yy48 = New_AST_ReturnNode(yymsp[0].minor.yy66, 1);
}
#line 1708 "grammar.c"
        break;
      case 60: /* returnElements ::= returnElements COMMA returnElement */
#line 352 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy174);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1716 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 61: /* returnElements ::= returnElement */
#line 357 "grammar.y"
{
	yylhsminor.yy66 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy174);
}
#line 1725 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 62: /* returnElement ::= MUL */
#line 365 "grammar.y"
{
	yymsp[0].minor.yy174 = New_AST_ReturnElementExpandALL();
}
#line 1733 "grammar.c"
        break;
      case 63: /* returnElement ::= arithmetic_expression */
#line 368 "grammar.y"
{
	yylhsminor.yy174 = New_AST_ReturnElementNode(yymsp[0].minor.yy154, NULL);
}
#line 1740 "grammar.c"
  yymsp[0].minor.yy174 = yylhsminor.yy174;
        break;
      case 64: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 372 "grammar.y"
{
	yylhsminor.yy174 = New_AST_ReturnElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 1748 "grammar.c"
  yymsp[-2].minor.yy174 = yylhsminor.yy174;
        break;
      case 65: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 379 "grammar.y"
{
	yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154;
}
#line 1756 "grammar.c"
        break;
      case 66: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 391 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1766 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 67: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 398 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1777 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 68: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 405 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1788 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 69: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 412 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1799 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 70: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 420 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy66);
}
#line 1807 "grammar.c"
  yymsp[-3].minor.yy154 = yylhsminor.yy154;
        break;
      case 71: /* arithmetic_expression ::= value */
#line 425 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy78);
}
#line 1815 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 72: /* arithmetic_expression ::= variable */
#line 430 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy120->alias, yymsp[0].minor.yy120->property);
	free(yymsp[0].minor.yy120);
}
#line 1824 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 73: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 437 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy154);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1833 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 74: /* arithmetic_expression_list ::= arithmetic_expression */
#line 441 "grammar.y"
{
	yylhsminor.yy66 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy154);
}
#line 1842 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 75: /* variable ::= UQSTRING */
#line 448 "grammar.y"
{
	yylhsminor.yy120 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1850 "grammar.c"
  yymsp[0].minor.yy120 = yylhsminor.yy120;
        break;
      case 76: /* variable ::= UQSTRING DOT UQSTRING */
#line 452 "grammar.y"
{
	yylhsminor.yy120 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1858 "grammar.c"
  yymsp[-2].minor.yy120 = yylhsminor.yy120;
        break;
      case 77: /* orderClause ::= */
#line 458 "grammar.y"
{
	yymsp[1].minor.yy88 = NULL;
}
#line 1866 "grammar.c"
        break;
      case 78: /* orderClause ::= ORDER BY columnNameList */
#line 461 "grammar.y"
{
	yymsp[-2].minor.yy88 = New_AST_OrderNode(yymsp[0].minor.yy66, ORDER_DIR_ASC);
}
#line 1873 "grammar.c"
        break;
      case 79: /* orderClause ::= ORDER BY columnNameList ASC */
#line 464 "grammar.y"
{
	yymsp[-3].minor.yy88 = New_AST_OrderNode(yymsp[-1].minor.yy66, ORDER_DIR_ASC);
}
#line 1880 "grammar.c"
        break;
      case 80: /* orderClause ::= ORDER BY columnNameList DESC */
#line 467 "grammar.y"
{
	yymsp[-3].minor.yy88 = New_AST_OrderNode(yymsp[-1].minor.yy66, ORDER_DIR_DESC);
}
#line 1887 "grammar.c"
        break;
      case 81: /* columnNameList ::= columnNameList COMMA columnName */
#line 472 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy66, yymsp[0].minor.yy10);
	yylhsminor.yy66 = yymsp[-2].minor.yy66;
}
#line 1895 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 82: /* columnNameList ::= columnName */
#line 476 "grammar.y"
{
	yylhsminor.yy66 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy66, yymsp[0].minor.yy10);
}
#line 1904 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 83: /* columnName ::= variable */
#line 482 "grammar.y"
{
	if(yymsp[0].minor.yy120->property != NULL) {
		yylhsminor.yy10 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy120);
	} else {
		yylhsminor.yy10 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy120->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy120);
}
#line 1918 "grammar.c"
  yymsp[0].minor.yy10 = yylhsminor.yy10;
        break;
      case 84: /* skipClause ::= */
#line 494 "grammar.y"
{
	yymsp[1].minor.yy3 = NULL;
}
#line 1926 "grammar.c"
        break;
      case 85: /* skipClause ::= SKIP INTEGER */
#line 497 "grammar.y"
{
	yymsp[-1].minor.yy3 = New_AST_SkipNode(yymsp[0].minor.yy0.intval);
}
#line 1933 "grammar.c"
        break;
      case 86: /* limitClause ::= */
#line 503 "grammar.y"
{
	yymsp[1].minor.yy147 = NULL;
}
#line 1940 "grammar.c"
        break;
      case 87: /* limitClause ::= LIMIT INTEGER */
#line 506 "grammar.y"
{
	yymsp[-1].minor.yy147 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1947 "grammar.c"
        break;
      case 88: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 512 "grammar.y"
{
	yymsp[-5].minor.yy97 = New_AST_UnwindNode(yymsp[-3].minor.yy66, yymsp[0].minor.yy0.strval);
}
#line 1954 "grammar.c"
        break;
      case 89: /* relation ::= EQ */
#line 517 "grammar.y"
{ yymsp[0].minor.yy113 = EQ; }
#line 1959 "grammar.c"
        break;
      case 90: /* relation ::= GT */
#line 518 "grammar.y"
{ yymsp[0].minor.yy113 = GT; }
#line 1964 "grammar.c"
        break;
      case 91: /* relation ::= LT */
#line 519 "grammar.y"
{ yymsp[0].minor.yy113 = LT; }
#line 1969 "grammar.c"
        break;
      case 92: /* relation ::= LE */
#line 520 "grammar.y"
{ yymsp[0].minor.yy113 = LE; }
#line 1974 "grammar.c"
        break;
      case 93: /* relation ::= GE */
#line 521 "grammar.y"
{ yymsp[0].minor.yy113 = GE; }
#line 1979 "grammar.c"
        break;
      case 94: /* relation ::= NE */
#line 522 "grammar.y"
{ yymsp[0].minor.yy113 = NE; }
#line 1984 "grammar.c"
        break;
      case 95: /* value ::= INTEGER */
#line 533 "grammar.y"
{  yylhsminor.yy78 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1989 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 96: /* value ::= DASH INTEGER */
#line 534 "grammar.y"
{  yymsp[-1].minor.yy78 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1995 "grammar.c"
        break;
      case 97: /* value ::= STRING */
#line 535 "grammar.y"
{  yylhsminor.yy78 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2000 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 98: /* value ::= FLOAT */
#line 536 "grammar.y"
{  yylhsminor.yy78 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2006 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 99: /* value ::= DASH FLOAT */
#line 537 "grammar.y"
{  yymsp[-1].minor.yy78 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2012 "grammar.c"
        break;
      case 100: /* value ::= TRUE */
#line 538 "grammar.y"
{ yymsp[0].minor.yy78 = SI_BoolVal(1); }
#line 2017 "grammar.c"
        break;
      case 101: /* value ::= FALSE */
#line 539 "grammar.y"
{ yymsp[0].minor.yy78 = SI_BoolVal(0); }
#line 2022 "grammar.c"
        break;
      case 102: /* value ::= NULLVAL */
#line 540 "grammar.y"
{ yymsp[0].minor.yy78 = SI_NullVal(); }
#line 2027 "grammar.c"
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
#line 31 "grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 2092 "grammar.c"
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
#line 542 "grammar.y"


	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	extern int             yylex_destroy (void);
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
		yylex_destroy();
		return ctx.root;
	}
#line 2340 "grammar.c"
