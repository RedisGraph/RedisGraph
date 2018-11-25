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
#define YYNOCODE 95
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_IndexOpType yy1;
  AST_MatchNode* yy5;
  AST_WhereNode* yy7;
  AST_IndexNode* yy8;
  AST_DeleteNode * yy15;
  AST_SetElement* yy16;
  AST_CreateNode* yy20;
  AST_Query* yy24;
  AST_ArithmeticExpressionNode* yy34;
  AST_UnwindNode* yy37;
  Vector* yy42;
  SIValue yy46;
  AST_LinkEntity* yy53;
  AST_SkipNode* yy59;
  AST_NodeEntity* yy81;
  AST_ReturnElementNode* yy86;
  int yy92;
  AST_ColumnNode* yy98;
  AST_SetNode* yy120;
  AST_MergeNode* yy132;
  AST_ReturnNode* yy140;
  AST_LimitNode* yy147;
  AST_FilterNode* yy170;
  AST_OrderNode* yy176;
  AST_LinkLength* yy178;
  AST_Variable* yy188;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             126
#define YYNRULE              109
#define YYNTOKEN             51
#define YY_MAX_SHIFT         125
#define YY_MIN_SHIFTREDUCE   198
#define YY_MAX_SHIFTREDUCE   306
#define YY_ERROR_ACTION      307
#define YY_ACCEPT_ACTION     308
#define YY_NO_ACTION         309
#define YY_MIN_REDUCE        310
#define YY_MAX_REDUCE        418
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
#define YY_ACTTAB_COUNT (324)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   125,  308,   65,   12,  316,  320,   78,  266,   95,  338,
 /*    10 */    94,   11,  318,  319,   41,  323,   78,   56,  328,   89,
 /*    20 */    80,   18,   19,   17,   16,   15,  293,  294,  297,  295,
 /*    30 */   296,  299,   78,  266,   21,   20,   33,    2,  221,  269,
 /*    40 */   111,  299,  121,   25,  388,   68,   80,   18,  301,  302,
 /*    50 */   304,  305,  306,  387,  388,   67,    1,  299,  301,  302,
 /*    60 */   304,  305,  306,  387,  298,   49,   84,  376,   19,   17,
 /*    70 */    16,   15,  107,   23,  301,  302,  304,  305,  306,   19,
 /*    80 */    17,   16,   15,  293,  294,  297,  295,  296,   78,  116,
 /*    90 */   388,   71,  101,   78,    1,   35,   55,  388,   67,  387,
 /*   100 */   341,  118,   80,   18,   74,  110,  387,   80,    7,  119,
 /*   110 */   377,  388,   67,  299,   19,   17,   16,   15,  299,  274,
 /*   120 */   387,  298,   14,  117,  377,   19,   17,   16,   15,  331,
 /*   130 */   301,  302,  304,  305,  306,  301,  302,  304,  305,  306,
 /*   140 */    87,   64,  269,  339,   94,   90,  313,   60,  388,   30,
 /*   150 */    85,   35,  388,   29,   56,  328,  341,  387,   91,  388,
 /*   160 */    30,  387,   72,  388,   30,  388,   71,   23,  387,  372,
 /*   170 */   317,   43,  387,   75,  387,   21,   22,  300,   44,   79,
 /*   180 */   388,   69,  399,   56,  328,  388,   70,  388,  385,  387,
 /*   190 */   388,  384,  388,   81,  387,  303,  387,   73,  398,  387,
 /*   200 */    28,  387,  388,   66,  388,   77,   98,   36,  109,   37,
 /*   210 */    50,  387,  399,  387,   34,    6,    8,  283,  284,  341,
 /*   220 */    53,   38,   53,   10,  100,  115,  341,   39,  397,  324,
 /*   230 */     6,    8,   45,  124,  259,   14,   16,   15,   53,   53,
 /*   240 */    83,  120,  104,  102,  321,  123,   42,   93,  234,   32,
 /*   250 */    96,   53,   97,   76,   99,   46,  108,  105,  106,  112,
 /*   260 */   342,  367,  114,  113,   23,  329,  122,    1,   58,  315,
 /*   270 */    57,    9,   59,  292,    3,   62,  311,   61,   63,   82,
 /*   280 */     5,  222,  223,   86,   40,   88,    8,   27,  235,   92,
 /*   290 */    13,   26,  241,   47,   48,  244,  120,   24,  310,   51,
 /*   300 */   245,  243,  239,  249,   31,  240,  247,  242,  103,  237,
 /*   310 */   238,   54,  236,  309,   52,  289,  309,  253,    4,  268,
 /*   320 */   280,  309,  309,  291,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    52,   53,   54,   88,   56,   57,    4,    5,   75,   76,
 /*    10 */    77,   63,   64,   65,   66,   67,    4,   69,   70,   71,
 /*    20 */    18,   19,    3,    4,    5,    6,    7,    8,    9,   10,
 /*    30 */    11,   29,    4,    5,   12,   13,   19,   35,   16,   20,
 /*    40 */    86,   29,   18,   21,   77,   78,   18,   19,   46,   47,
 /*    50 */    48,   49,   50,   86,   77,   78,   34,   29,   46,   47,
 /*    60 */    48,   49,   50,   86,   45,   82,   44,   90,    3,    4,
 /*    70 */     5,    6,   82,   13,   46,   47,   48,   49,   50,    3,
 /*    80 */     4,    5,    6,    7,    8,    9,   10,   11,    4,   68,
 /*    90 */    77,   78,   82,    4,   34,   74,   80,   77,   78,   86,
 /*   100 */    79,   36,   18,   19,   91,   82,   86,   18,   19,   89,
 /*   110 */    90,   77,   78,   29,    3,    4,    5,    6,   29,   20,
 /*   120 */    86,   45,   23,   89,   90,    3,    4,    5,    6,   73,
 /*   130 */    46,   47,   48,   49,   50,   46,   47,   48,   49,   50,
 /*   140 */    19,   56,   20,   76,   77,   68,   61,   62,   77,   78,
 /*   150 */    17,   74,   77,   78,   69,   70,   79,   86,   87,   77,
 /*   160 */    78,   86,   87,   77,   78,   77,   78,   13,   86,   87,
 /*   170 */    56,   57,   86,   87,   86,   12,   22,   29,   24,   91,
 /*   180 */    77,   78,   77,   69,   70,   77,   78,   77,   78,   86,
 /*   190 */    77,   78,   77,   78,   86,   47,   86,   92,   93,   86,
 /*   200 */    23,   86,   77,   78,   77,   78,   17,   18,   17,   18,
 /*   210 */     4,   86,   77,   86,   74,    1,    2,   40,   41,   79,
 /*   220 */    31,   74,   31,   19,   17,   17,   79,   72,   93,   67,
 /*   230 */     1,    2,   26,   43,   20,   23,    5,    6,   31,   31,
 /*   240 */    28,   37,   29,   30,   60,   42,   59,   81,   18,   27,
 /*   250 */    83,   31,   82,    5,   82,   84,   82,   84,   83,   18,
 /*   260 */    79,   85,   82,   85,   13,   70,   38,   34,   58,   60,
 /*   270 */    59,   33,   57,   18,   55,   58,   60,   59,   57,   36,
 /*   280 */    27,   18,   20,   18,   15,   14,    2,   23,   18,   23,
 /*   290 */     7,   23,    4,   18,   18,   28,   37,   39,    0,   18,
 /*   300 */    28,   28,   20,   29,   17,   25,   29,   28,   30,   20,
 /*   310 */    20,   18,   20,   94,   23,   29,   94,   32,   23,   18,
 /*   320 */    18,   94,   94,   29,   94,   94,   94,   94,   94,   94,
 /*   330 */    94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
 /*   340 */    94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
 /*   350 */    94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
 /*   360 */    94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
 /*   370 */    94,   94,   94,   94,   94,
};
#define YY_SHIFT_COUNT    (125)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (302)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    22,    2,   28,  154,   28,   84,   89,   89,   89,   89,
 /*    10 */    84,   60,   84,   84,   84,   84,   84,   84,   84,   84,
 /*    20 */    17,   17,   24,   17,   24,   17,   24,   17,   24,   19,
 /*    30 */    76,   12,  189,  191,  206,  206,  207,  208,  206,  121,
 /*    40 */   133,  163,  190,  203,  230,  222,  220,  248,  220,  248,
 /*    50 */   222,  220,  241,  241,  220,   17,  251,  190,  203,  228,
 /*    60 */   233,  190,  203,  228,  233,  238,  122,   65,  111,  111,
 /*    70 */   111,  111,  214,  177,  212,  229,  213,  231,  148,   99,
 /*    80 */   204,  231,  255,  243,  253,  263,  262,  265,  269,  271,
 /*    90 */   264,  284,  270,  266,  283,  268,  288,  267,  275,  272,
 /*   100 */   276,  273,  274,  277,  278,  279,  280,  282,  289,  281,
 /*   110 */   290,  291,  287,  285,  292,  293,  264,  295,  301,  295,
 /*   120 */   302,  259,  258,  286,  294,  298,
};
#define YY_REDUCE_COUNT (65)
#define YY_REDUCE_MIN   (-85)
#define YY_REDUCE_MAX   (221)
static const short yy_reduce_ofst[] = {
 /*     0 */   -52,   20,   34,   85,  -23,   13,   71,   75,   82,   86,
 /*    10 */    88,  114,  -33,  103,  108,  110,  113,  115,  125,  127,
 /*    20 */    21,   77,  -67,   21,  105,  140,   67,  147,  135,  -85,
 /*    30 */   -85,  -46,  -17,  -10,   16,   16,   10,   23,   16,   56,
 /*    40 */   155,  162,  184,  187,  166,  167,  170,  171,  172,  173,
 /*    50 */   175,  174,  176,  178,  180,  181,  195,  209,  211,  210,
 /*    60 */   215,  216,  218,  217,  221,  219,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   326,  307,  307,  326,  307,  307,  307,  307,  307,  307,
 /*    10 */   307,  326,  307,  307,  307,  307,  307,  307,  307,  307,
 /*    20 */   332,  307,  307,  307,  307,  307,  307,  307,  307,  307,
 /*    30 */   307,  307,  364,  364,  336,  343,  364,  364,  344,  307,
 /*    40 */   307,  322,  402,  400,  307,  307,  364,  358,  364,  358,
 /*    50 */   307,  364,  307,  307,  364,  307,  327,  402,  400,  393,
 /*    60 */   314,  402,  400,  393,  312,  368,  307,  379,  370,  340,
 /*    70 */   389,  390,  307,  394,  307,  369,  363,  382,  307,  307,
 /*    80 */   391,  383,  307,  307,  307,  307,  307,  307,  307,  307,
 /*    90 */   325,  373,  307,  345,  307,  337,  307,  307,  307,  307,
 /*   100 */   307,  307,  307,  360,  362,  307,  307,  307,  307,  307,
 /*   110 */   307,  366,  307,  307,  307,  307,  330,  375,  307,  374,
 /*   120 */   307,  391,  307,  307,  307,  307,
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
  /*   54 */ "multipleMatchClause",
  /*   55 */ "whereClause",
  /*   56 */ "multipleCreateClause",
  /*   57 */ "returnClause",
  /*   58 */ "orderClause",
  /*   59 */ "skipClause",
  /*   60 */ "limitClause",
  /*   61 */ "deleteClause",
  /*   62 */ "setClause",
  /*   63 */ "unwindClause",
  /*   64 */ "indexClause",
  /*   65 */ "mergeClause",
  /*   66 */ "matchClauses",
  /*   67 */ "matchClause",
  /*   68 */ "chains",
  /*   69 */ "createClauses",
  /*   70 */ "createClause",
  /*   71 */ "indexOpToken",
  /*   72 */ "indexLabel",
  /*   73 */ "indexProp",
  /*   74 */ "chain",
  /*   75 */ "setList",
  /*   76 */ "setElement",
  /*   77 */ "variable",
  /*   78 */ "arithmetic_expression",
  /*   79 */ "node",
  /*   80 */ "link",
  /*   81 */ "deleteExpression",
  /*   82 */ "properties",
  /*   83 */ "edge",
  /*   84 */ "edgeLength",
  /*   85 */ "mapLiteral",
  /*   86 */ "value",
  /*   87 */ "cond",
  /*   88 */ "relation",
  /*   89 */ "returnElements",
  /*   90 */ "returnElement",
  /*   91 */ "arithmetic_expression_list",
  /*   92 */ "columnNameList",
  /*   93 */ "columnName",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause",
 /*   2 */ "expr ::= multipleMatchClause whereClause multipleCreateClause",
 /*   3 */ "expr ::= multipleMatchClause whereClause deleteClause",
 /*   4 */ "expr ::= multipleMatchClause whereClause setClause",
 /*   5 */ "expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause",
 /*   6 */ "expr ::= multipleCreateClause",
 /*   7 */ "expr ::= unwindClause multipleCreateClause",
 /*   8 */ "expr ::= indexClause",
 /*   9 */ "expr ::= mergeClause",
 /*  10 */ "expr ::= returnClause",
 /*  11 */ "expr ::= unwindClause returnClause skipClause limitClause",
 /*  12 */ "multipleMatchClause ::= matchClauses",
 /*  13 */ "matchClauses ::= matchClause",
 /*  14 */ "matchClauses ::= matchClauses matchClause",
 /*  15 */ "matchClause ::= MATCH chains",
 /*  16 */ "multipleCreateClause ::=",
 /*  17 */ "multipleCreateClause ::= createClauses",
 /*  18 */ "createClauses ::= createClause",
 /*  19 */ "createClauses ::= createClauses createClause",
 /*  20 */ "createClause ::= CREATE chains",
 /*  21 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  22 */ "indexOpToken ::= CREATE",
 /*  23 */ "indexOpToken ::= DROP",
 /*  24 */ "indexLabel ::= COLON UQSTRING",
 /*  25 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  26 */ "mergeClause ::= MERGE chain",
 /*  27 */ "setClause ::= SET setList",
 /*  28 */ "setList ::= setElement",
 /*  29 */ "setList ::= setList COMMA setElement",
 /*  30 */ "setElement ::= variable EQ arithmetic_expression",
 /*  31 */ "chain ::= node",
 /*  32 */ "chain ::= chain link node",
 /*  33 */ "chains ::= chain",
 /*  34 */ "chains ::= chains COMMA chain",
 /*  35 */ "deleteClause ::= DELETE deleteExpression",
 /*  36 */ "deleteExpression ::= UQSTRING",
 /*  37 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  38 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  39 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  40 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  41 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  42 */ "link ::= DASH edge RIGHT_ARROW",
 /*  43 */ "link ::= LEFT_ARROW edge DASH",
 /*  44 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  45 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  46 */ "edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET",
 /*  47 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  48 */ "edgeLength ::=",
 /*  49 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  50 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  51 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  52 */ "edgeLength ::= MUL INTEGER",
 /*  53 */ "edgeLength ::= MUL",
 /*  54 */ "properties ::=",
 /*  55 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  56 */ "mapLiteral ::= UQSTRING COLON value",
 /*  57 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  58 */ "whereClause ::=",
 /*  59 */ "whereClause ::= WHERE cond",
 /*  60 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  61 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  62 */ "cond ::= cond AND cond",
 /*  63 */ "cond ::= cond OR cond",
 /*  64 */ "returnClause ::= RETURN returnElements",
 /*  65 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  66 */ "returnElements ::= returnElements COMMA returnElement",
 /*  67 */ "returnElements ::= returnElement",
 /*  68 */ "returnElement ::= MUL",
 /*  69 */ "returnElement ::= arithmetic_expression",
 /*  70 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  71 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  72 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  73 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  74 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  75 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  76 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  77 */ "arithmetic_expression ::= value",
 /*  78 */ "arithmetic_expression ::= variable",
 /*  79 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  80 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  81 */ "variable ::= UQSTRING",
 /*  82 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  83 */ "orderClause ::=",
 /*  84 */ "orderClause ::= ORDER BY columnNameList",
 /*  85 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  86 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  87 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  88 */ "columnNameList ::= columnName",
 /*  89 */ "columnName ::= variable",
 /*  90 */ "skipClause ::=",
 /*  91 */ "skipClause ::= SKIP INTEGER",
 /*  92 */ "limitClause ::=",
 /*  93 */ "limitClause ::= LIMIT INTEGER",
 /*  94 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /*  95 */ "relation ::= EQ",
 /*  96 */ "relation ::= GT",
 /*  97 */ "relation ::= LT",
 /*  98 */ "relation ::= LE",
 /*  99 */ "relation ::= GE",
 /* 100 */ "relation ::= NE",
 /* 101 */ "value ::= INTEGER",
 /* 102 */ "value ::= DASH INTEGER",
 /* 103 */ "value ::= STRING",
 /* 104 */ "value ::= FLOAT",
 /* 105 */ "value ::= DASH FLOAT",
 /* 106 */ "value ::= TRUE",
 /* 107 */ "value ::= FALSE",
 /* 108 */ "value ::= NULLVAL",
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
    case 87: /* cond */
{
#line 368 "grammar.y"
 Free_AST_FilterNode((yypminor->yy170)); 
#line 786 "grammar.c"
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
  {   52,   -7 }, /* (1) expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
  {   52,   -3 }, /* (2) expr ::= multipleMatchClause whereClause multipleCreateClause */
  {   52,   -3 }, /* (3) expr ::= multipleMatchClause whereClause deleteClause */
  {   52,   -3 }, /* (4) expr ::= multipleMatchClause whereClause setClause */
  {   52,   -7 }, /* (5) expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   52,   -1 }, /* (6) expr ::= multipleCreateClause */
  {   52,   -2 }, /* (7) expr ::= unwindClause multipleCreateClause */
  {   52,   -1 }, /* (8) expr ::= indexClause */
  {   52,   -1 }, /* (9) expr ::= mergeClause */
  {   52,   -1 }, /* (10) expr ::= returnClause */
  {   52,   -4 }, /* (11) expr ::= unwindClause returnClause skipClause limitClause */
  {   54,   -1 }, /* (12) multipleMatchClause ::= matchClauses */
  {   66,   -1 }, /* (13) matchClauses ::= matchClause */
  {   66,   -2 }, /* (14) matchClauses ::= matchClauses matchClause */
  {   67,   -2 }, /* (15) matchClause ::= MATCH chains */
  {   56,    0 }, /* (16) multipleCreateClause ::= */
  {   56,   -1 }, /* (17) multipleCreateClause ::= createClauses */
  {   69,   -1 }, /* (18) createClauses ::= createClause */
  {   69,   -2 }, /* (19) createClauses ::= createClauses createClause */
  {   70,   -2 }, /* (20) createClause ::= CREATE chains */
  {   64,   -5 }, /* (21) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   71,   -1 }, /* (22) indexOpToken ::= CREATE */
  {   71,   -1 }, /* (23) indexOpToken ::= DROP */
  {   72,   -2 }, /* (24) indexLabel ::= COLON UQSTRING */
  {   73,   -3 }, /* (25) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   65,   -2 }, /* (26) mergeClause ::= MERGE chain */
  {   62,   -2 }, /* (27) setClause ::= SET setList */
  {   75,   -1 }, /* (28) setList ::= setElement */
  {   75,   -3 }, /* (29) setList ::= setList COMMA setElement */
  {   76,   -3 }, /* (30) setElement ::= variable EQ arithmetic_expression */
  {   74,   -1 }, /* (31) chain ::= node */
  {   74,   -3 }, /* (32) chain ::= chain link node */
  {   68,   -1 }, /* (33) chains ::= chain */
  {   68,   -3 }, /* (34) chains ::= chains COMMA chain */
  {   61,   -2 }, /* (35) deleteClause ::= DELETE deleteExpression */
  {   81,   -1 }, /* (36) deleteExpression ::= UQSTRING */
  {   81,   -3 }, /* (37) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   79,   -6 }, /* (38) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   79,   -5 }, /* (39) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   79,   -4 }, /* (40) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   79,   -3 }, /* (41) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   80,   -3 }, /* (42) link ::= DASH edge RIGHT_ARROW */
  {   80,   -3 }, /* (43) link ::= LEFT_ARROW edge DASH */
  {   83,   -4 }, /* (44) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   83,   -4 }, /* (45) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   83,   -6 }, /* (46) edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
  {   83,   -6 }, /* (47) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   84,    0 }, /* (48) edgeLength ::= */
  {   84,   -4 }, /* (49) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   84,   -3 }, /* (50) edgeLength ::= MUL INTEGER DOTDOT */
  {   84,   -3 }, /* (51) edgeLength ::= MUL DOTDOT INTEGER */
  {   84,   -2 }, /* (52) edgeLength ::= MUL INTEGER */
  {   84,   -1 }, /* (53) edgeLength ::= MUL */
  {   82,    0 }, /* (54) properties ::= */
  {   82,   -3 }, /* (55) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   85,   -3 }, /* (56) mapLiteral ::= UQSTRING COLON value */
  {   85,   -5 }, /* (57) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   55,    0 }, /* (58) whereClause ::= */
  {   55,   -2 }, /* (59) whereClause ::= WHERE cond */
  {   87,   -3 }, /* (60) cond ::= arithmetic_expression relation arithmetic_expression */
  {   87,   -3 }, /* (61) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   87,   -3 }, /* (62) cond ::= cond AND cond */
  {   87,   -3 }, /* (63) cond ::= cond OR cond */
  {   57,   -2 }, /* (64) returnClause ::= RETURN returnElements */
  {   57,   -3 }, /* (65) returnClause ::= RETURN DISTINCT returnElements */
  {   89,   -3 }, /* (66) returnElements ::= returnElements COMMA returnElement */
  {   89,   -1 }, /* (67) returnElements ::= returnElement */
  {   90,   -1 }, /* (68) returnElement ::= MUL */
  {   90,   -1 }, /* (69) returnElement ::= arithmetic_expression */
  {   90,   -3 }, /* (70) returnElement ::= arithmetic_expression AS UQSTRING */
  {   78,   -3 }, /* (71) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   78,   -3 }, /* (72) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   78,   -3 }, /* (73) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   78,   -3 }, /* (74) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   78,   -3 }, /* (75) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   78,   -4 }, /* (76) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   78,   -1 }, /* (77) arithmetic_expression ::= value */
  {   78,   -1 }, /* (78) arithmetic_expression ::= variable */
  {   91,   -3 }, /* (79) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   91,   -1 }, /* (80) arithmetic_expression_list ::= arithmetic_expression */
  {   77,   -1 }, /* (81) variable ::= UQSTRING */
  {   77,   -3 }, /* (82) variable ::= UQSTRING DOT UQSTRING */
  {   58,    0 }, /* (83) orderClause ::= */
  {   58,   -3 }, /* (84) orderClause ::= ORDER BY columnNameList */
  {   58,   -4 }, /* (85) orderClause ::= ORDER BY columnNameList ASC */
  {   58,   -4 }, /* (86) orderClause ::= ORDER BY columnNameList DESC */
  {   92,   -3 }, /* (87) columnNameList ::= columnNameList COMMA columnName */
  {   92,   -1 }, /* (88) columnNameList ::= columnName */
  {   93,   -1 }, /* (89) columnName ::= variable */
  {   59,    0 }, /* (90) skipClause ::= */
  {   59,   -2 }, /* (91) skipClause ::= SKIP INTEGER */
  {   60,    0 }, /* (92) limitClause ::= */
  {   60,   -2 }, /* (93) limitClause ::= LIMIT INTEGER */
  {   63,   -6 }, /* (94) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {   88,   -1 }, /* (95) relation ::= EQ */
  {   88,   -1 }, /* (96) relation ::= GT */
  {   88,   -1 }, /* (97) relation ::= LT */
  {   88,   -1 }, /* (98) relation ::= LE */
  {   88,   -1 }, /* (99) relation ::= GE */
  {   88,   -1 }, /* (100) relation ::= NE */
  {   86,   -1 }, /* (101) value ::= INTEGER */
  {   86,   -2 }, /* (102) value ::= DASH INTEGER */
  {   86,   -1 }, /* (103) value ::= STRING */
  {   86,   -1 }, /* (104) value ::= FLOAT */
  {   86,   -2 }, /* (105) value ::= DASH FLOAT */
  {   86,   -1 }, /* (106) value ::= TRUE */
  {   86,   -1 }, /* (107) value ::= FALSE */
  {   86,   -1 }, /* (108) value ::= NULLVAL */
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
{ ctx->root = yymsp[0].minor.yy24; }
#line 1272 "grammar.c"
        break;
      case 1: /* expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
#line 46 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(yymsp[-6].minor.yy5, yymsp[-5].minor.yy7, yymsp[-4].minor.yy20, NULL, NULL, NULL, yymsp[-3].minor.yy140, yymsp[-2].minor.yy176, yymsp[-1].minor.yy59, yymsp[0].minor.yy147, NULL, NULL);
}
#line 1279 "grammar.c"
  yymsp[-6].minor.yy24 = yylhsminor.yy24;
        break;
      case 2: /* expr ::= multipleMatchClause whereClause multipleCreateClause */
#line 50 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(yymsp[-2].minor.yy5, yymsp[-1].minor.yy7, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1287 "grammar.c"
  yymsp[-2].minor.yy24 = yylhsminor.yy24;
        break;
      case 3: /* expr ::= multipleMatchClause whereClause deleteClause */
#line 54 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(yymsp[-2].minor.yy5, yymsp[-1].minor.yy7, NULL, NULL, NULL, yymsp[0].minor.yy15, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1295 "grammar.c"
  yymsp[-2].minor.yy24 = yylhsminor.yy24;
        break;
      case 4: /* expr ::= multipleMatchClause whereClause setClause */
#line 58 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(yymsp[-2].minor.yy5, yymsp[-1].minor.yy7, NULL, NULL, yymsp[0].minor.yy120, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1303 "grammar.c"
  yymsp[-2].minor.yy24 = yylhsminor.yy24;
        break;
      case 5: /* expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 62 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(yymsp[-6].minor.yy5, yymsp[-5].minor.yy7, NULL, NULL, yymsp[-4].minor.yy120, NULL, yymsp[-3].minor.yy140, yymsp[-2].minor.yy176, yymsp[-1].minor.yy59, yymsp[0].minor.yy147, NULL, NULL);
}
#line 1311 "grammar.c"
  yymsp[-6].minor.yy24 = yylhsminor.yy24;
        break;
      case 6: /* expr ::= multipleCreateClause */
#line 66 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1319 "grammar.c"
  yymsp[0].minor.yy24 = yylhsminor.yy24;
        break;
      case 7: /* expr ::= unwindClause multipleCreateClause */
#line 70 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy37);
}
#line 1327 "grammar.c"
  yymsp[-1].minor.yy24 = yylhsminor.yy24;
        break;
      case 8: /* expr ::= indexClause */
#line 74 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy8, NULL);
}
#line 1335 "grammar.c"
  yymsp[0].minor.yy24 = yylhsminor.yy24;
        break;
      case 9: /* expr ::= mergeClause */
#line 78 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy132, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1343 "grammar.c"
  yymsp[0].minor.yy24 = yylhsminor.yy24;
        break;
      case 10: /* expr ::= returnClause */
#line 82 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy140, NULL, NULL, NULL, NULL, NULL);
}
#line 1351 "grammar.c"
  yymsp[0].minor.yy24 = yylhsminor.yy24;
        break;
      case 11: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 86 "grammar.y"
{
	yylhsminor.yy24 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy140, NULL, yymsp[-1].minor.yy59, yymsp[0].minor.yy147, NULL, yymsp[-3].minor.yy37);
}
#line 1359 "grammar.c"
  yymsp[-3].minor.yy24 = yylhsminor.yy24;
        break;
      case 12: /* multipleMatchClause ::= matchClauses */
#line 91 "grammar.y"
{
	yylhsminor.yy5 = New_AST_MatchNode(yymsp[0].minor.yy42);
}
#line 1367 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 13: /* matchClauses ::= matchClause */
      case 18: /* createClauses ::= createClause */ yytestcase(yyruleno==18);
#line 97 "grammar.y"
{
	yylhsminor.yy42 = yymsp[0].minor.yy42;
}
#line 1376 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 14: /* matchClauses ::= matchClauses matchClause */
      case 19: /* createClauses ::= createClauses createClause */ yytestcase(yyruleno==19);
#line 101 "grammar.y"
{
	Vector *v;
	while(Vector_Pop(yymsp[0].minor.yy42, &v)) Vector_Push(yymsp[-1].minor.yy42, v);
	Vector_Free(yymsp[0].minor.yy42);
	yylhsminor.yy42 = yymsp[-1].minor.yy42;
}
#line 1388 "grammar.c"
  yymsp[-1].minor.yy42 = yylhsminor.yy42;
        break;
      case 15: /* matchClause ::= MATCH chains */
      case 20: /* createClause ::= CREATE chains */ yytestcase(yyruleno==20);
#line 110 "grammar.y"
{
	yymsp[-1].minor.yy42 = yymsp[0].minor.yy42;
}
#line 1397 "grammar.c"
        break;
      case 16: /* multipleCreateClause ::= */
#line 115 "grammar.y"
{
	yymsp[1].minor.yy20 = NULL;
}
#line 1404 "grammar.c"
        break;
      case 17: /* multipleCreateClause ::= createClauses */
#line 119 "grammar.y"
{
	yylhsminor.yy20 = New_AST_CreateNode(yymsp[0].minor.yy42);
}
#line 1411 "grammar.c"
  yymsp[0].minor.yy20 = yylhsminor.yy20;
        break;
      case 21: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 145 "grammar.y"
{
  yylhsminor.yy8 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy1);
}
#line 1419 "grammar.c"
  yymsp[-4].minor.yy8 = yylhsminor.yy8;
        break;
      case 22: /* indexOpToken ::= CREATE */
#line 151 "grammar.y"
{ yymsp[0].minor.yy1 = CREATE_INDEX; }
#line 1425 "grammar.c"
        break;
      case 23: /* indexOpToken ::= DROP */
#line 152 "grammar.y"
{ yymsp[0].minor.yy1 = DROP_INDEX; }
#line 1430 "grammar.c"
        break;
      case 24: /* indexLabel ::= COLON UQSTRING */
#line 154 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1437 "grammar.c"
        break;
      case 25: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 158 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1444 "grammar.c"
        break;
      case 26: /* mergeClause ::= MERGE chain */
#line 164 "grammar.y"
{
	yymsp[-1].minor.yy132 = New_AST_MergeNode(yymsp[0].minor.yy42);
}
#line 1451 "grammar.c"
        break;
      case 27: /* setClause ::= SET setList */
#line 169 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_SetNode(yymsp[0].minor.yy42);
}
#line 1458 "grammar.c"
        break;
      case 28: /* setList ::= setElement */
#line 174 "grammar.y"
{
	yylhsminor.yy42 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy16);
}
#line 1466 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 29: /* setList ::= setList COMMA setElement */
#line 178 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy16);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1475 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 30: /* setElement ::= variable EQ arithmetic_expression */
#line 184 "grammar.y"
{
	yylhsminor.yy16 = New_AST_SetElement(yymsp[-2].minor.yy188, yymsp[0].minor.yy34);
}
#line 1483 "grammar.c"
  yymsp[-2].minor.yy16 = yylhsminor.yy16;
        break;
      case 31: /* chain ::= node */
#line 190 "grammar.y"
{
	yylhsminor.yy42 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy81);
}
#line 1492 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 32: /* chain ::= chain link node */
#line 195 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[-1].minor.yy53);
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy81);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1502 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 33: /* chains ::= chain */
#line 203 "grammar.y"
{
	yylhsminor.yy42 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy42);
}
#line 1511 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 34: /* chains ::= chains COMMA chain */
#line 208 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy42);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1520 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 35: /* deleteClause ::= DELETE deleteExpression */
#line 216 "grammar.y"
{
	yymsp[-1].minor.yy15 = New_AST_DeleteNode(yymsp[0].minor.yy42);
}
#line 1528 "grammar.c"
        break;
      case 36: /* deleteExpression ::= UQSTRING */
#line 222 "grammar.y"
{
	yylhsminor.yy42 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy0.strval);
}
#line 1536 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 37: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 227 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy0.strval);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1545 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 38: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 235 "grammar.y"
{
	yymsp[-5].minor.yy81 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy42);
}
#line 1553 "grammar.c"
        break;
      case 39: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 240 "grammar.y"
{
	yymsp[-4].minor.yy81 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy42);
}
#line 1560 "grammar.c"
        break;
      case 40: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 245 "grammar.y"
{
	yymsp[-3].minor.yy81 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy42);
}
#line 1567 "grammar.c"
        break;
      case 41: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 250 "grammar.y"
{
	yymsp[-2].minor.yy81 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy42);
}
#line 1574 "grammar.c"
        break;
      case 42: /* link ::= DASH edge RIGHT_ARROW */
#line 257 "grammar.y"
{
	yymsp[-2].minor.yy53 = yymsp[-1].minor.yy53;
	yymsp[-2].minor.yy53->direction = N_LEFT_TO_RIGHT;
}
#line 1582 "grammar.c"
        break;
      case 43: /* link ::= LEFT_ARROW edge DASH */
#line 263 "grammar.y"
{
	yymsp[-2].minor.yy53 = yymsp[-1].minor.yy53;
	yymsp[-2].minor.yy53->direction = N_RIGHT_TO_LEFT;
}
#line 1590 "grammar.c"
        break;
      case 44: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 270 "grammar.y"
{ 
	yymsp[-3].minor.yy53 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy42, N_DIR_UNKNOWN, yymsp[-1].minor.yy178);
}
#line 1597 "grammar.c"
        break;
      case 45: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 275 "grammar.y"
{ 
	yymsp[-3].minor.yy53 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy42, N_DIR_UNKNOWN, NULL);
}
#line 1604 "grammar.c"
        break;
      case 46: /* edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
#line 280 "grammar.y"
{ 
	yymsp[-5].minor.yy53 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy42, N_DIR_UNKNOWN, yymsp[-2].minor.yy178);
}
#line 1611 "grammar.c"
        break;
      case 47: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 285 "grammar.y"
{ 
	yymsp[-5].minor.yy53 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy42, N_DIR_UNKNOWN, NULL);
}
#line 1618 "grammar.c"
        break;
      case 48: /* edgeLength ::= */
#line 292 "grammar.y"
{
	yymsp[1].minor.yy178 = NULL;
}
#line 1625 "grammar.c"
        break;
      case 49: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 297 "grammar.y"
{
	yymsp[-3].minor.yy178 = New_AST_LinkLength(yymsp[-2].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1632 "grammar.c"
        break;
      case 50: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 302 "grammar.y"
{
	yymsp[-2].minor.yy178 = New_AST_LinkLength(yymsp[-1].minor.yy0.intval, UINT_MAX-2);
}
#line 1639 "grammar.c"
        break;
      case 51: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 307 "grammar.y"
{
	yymsp[-2].minor.yy178 = New_AST_LinkLength(1, yymsp[0].minor.yy0.intval);
}
#line 1646 "grammar.c"
        break;
      case 52: /* edgeLength ::= MUL INTEGER */
#line 312 "grammar.y"
{
	yymsp[-1].minor.yy178 = New_AST_LinkLength(yymsp[0].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1653 "grammar.c"
        break;
      case 53: /* edgeLength ::= MUL */
#line 317 "grammar.y"
{
	yymsp[0].minor.yy178 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1660 "grammar.c"
        break;
      case 54: /* properties ::= */
#line 323 "grammar.y"
{
	yymsp[1].minor.yy42 = NULL;
}
#line 1667 "grammar.c"
        break;
      case 55: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 327 "grammar.y"
{
	yymsp[-2].minor.yy42 = yymsp[-1].minor.yy42;
}
#line 1674 "grammar.c"
        break;
      case 56: /* mapLiteral ::= UQSTRING COLON value */
#line 333 "grammar.y"
{
	yylhsminor.yy42 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy42, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy46;
	Vector_Push(yylhsminor.yy42, val);
}
#line 1689 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 57: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 345 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy42, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy46;
	Vector_Push(yymsp[0].minor.yy42, val);
	
	yylhsminor.yy42 = yymsp[0].minor.yy42;
}
#line 1705 "grammar.c"
  yymsp[-4].minor.yy42 = yylhsminor.yy42;
        break;
      case 58: /* whereClause ::= */
#line 359 "grammar.y"
{ 
	yymsp[1].minor.yy7 = NULL;
}
#line 1713 "grammar.c"
        break;
      case 59: /* whereClause ::= WHERE cond */
#line 362 "grammar.y"
{
	yymsp[-1].minor.yy7 = New_AST_WhereNode(yymsp[0].minor.yy170);
}
#line 1720 "grammar.c"
        break;
      case 60: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 371 "grammar.y"
{ yylhsminor.yy170 = New_AST_PredicateNode(yymsp[-2].minor.yy34, yymsp[-1].minor.yy92, yymsp[0].minor.yy34); }
#line 1725 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 61: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 373 "grammar.y"
{ yymsp[-2].minor.yy170 = yymsp[-1].minor.yy170; }
#line 1731 "grammar.c"
        break;
      case 62: /* cond ::= cond AND cond */
#line 374 "grammar.y"
{ yylhsminor.yy170 = New_AST_ConditionNode(yymsp[-2].minor.yy170, AND, yymsp[0].minor.yy170); }
#line 1736 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 63: /* cond ::= cond OR cond */
#line 375 "grammar.y"
{ yylhsminor.yy170 = New_AST_ConditionNode(yymsp[-2].minor.yy170, OR, yymsp[0].minor.yy170); }
#line 1742 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 64: /* returnClause ::= RETURN returnElements */
#line 379 "grammar.y"
{
	yymsp[-1].minor.yy140 = New_AST_ReturnNode(yymsp[0].minor.yy42, 0);
}
#line 1750 "grammar.c"
        break;
      case 65: /* returnClause ::= RETURN DISTINCT returnElements */
#line 382 "grammar.y"
{
	yymsp[-2].minor.yy140 = New_AST_ReturnNode(yymsp[0].minor.yy42, 1);
}
#line 1757 "grammar.c"
        break;
      case 66: /* returnElements ::= returnElements COMMA returnElement */
#line 388 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy86);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1765 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 67: /* returnElements ::= returnElement */
#line 393 "grammar.y"
{
	yylhsminor.yy42 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy86);
}
#line 1774 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 68: /* returnElement ::= MUL */
#line 401 "grammar.y"
{
	yymsp[0].minor.yy86 = New_AST_ReturnElementExpandALL();
}
#line 1782 "grammar.c"
        break;
      case 69: /* returnElement ::= arithmetic_expression */
#line 404 "grammar.y"
{
	yylhsminor.yy86 = New_AST_ReturnElementNode(yymsp[0].minor.yy34, NULL);
}
#line 1789 "grammar.c"
  yymsp[0].minor.yy86 = yylhsminor.yy86;
        break;
      case 70: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 408 "grammar.y"
{
	yylhsminor.yy86 = New_AST_ReturnElementNode(yymsp[-2].minor.yy34, yymsp[0].minor.yy0.strval);
}
#line 1797 "grammar.c"
  yymsp[-2].minor.yy86 = yylhsminor.yy86;
        break;
      case 71: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 415 "grammar.y"
{
	yymsp[-2].minor.yy34 = yymsp[-1].minor.yy34;
}
#line 1805 "grammar.c"
        break;
      case 72: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 427 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy34);
	Vector_Push(args, yymsp[0].minor.yy34);
	yylhsminor.yy34 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1815 "grammar.c"
  yymsp[-2].minor.yy34 = yylhsminor.yy34;
        break;
      case 73: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 434 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy34);
	Vector_Push(args, yymsp[0].minor.yy34);
	yylhsminor.yy34 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1826 "grammar.c"
  yymsp[-2].minor.yy34 = yylhsminor.yy34;
        break;
      case 74: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 441 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy34);
	Vector_Push(args, yymsp[0].minor.yy34);
	yylhsminor.yy34 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1837 "grammar.c"
  yymsp[-2].minor.yy34 = yylhsminor.yy34;
        break;
      case 75: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 448 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy34);
	Vector_Push(args, yymsp[0].minor.yy34);
	yylhsminor.yy34 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1848 "grammar.c"
  yymsp[-2].minor.yy34 = yylhsminor.yy34;
        break;
      case 76: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 456 "grammar.y"
{
	yylhsminor.yy34 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy42);
}
#line 1856 "grammar.c"
  yymsp[-3].minor.yy34 = yylhsminor.yy34;
        break;
      case 77: /* arithmetic_expression ::= value */
#line 461 "grammar.y"
{
	yylhsminor.yy34 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy46);
}
#line 1864 "grammar.c"
  yymsp[0].minor.yy34 = yylhsminor.yy34;
        break;
      case 78: /* arithmetic_expression ::= variable */
#line 466 "grammar.y"
{
	yylhsminor.yy34 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy188->alias, yymsp[0].minor.yy188->property);
	free(yymsp[0].minor.yy188);
}
#line 1873 "grammar.c"
  yymsp[0].minor.yy34 = yylhsminor.yy34;
        break;
      case 79: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 473 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy34);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1882 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 80: /* arithmetic_expression_list ::= arithmetic_expression */
#line 477 "grammar.y"
{
	yylhsminor.yy42 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy34);
}
#line 1891 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 81: /* variable ::= UQSTRING */
#line 484 "grammar.y"
{
	yylhsminor.yy188 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1899 "grammar.c"
  yymsp[0].minor.yy188 = yylhsminor.yy188;
        break;
      case 82: /* variable ::= UQSTRING DOT UQSTRING */
#line 488 "grammar.y"
{
	yylhsminor.yy188 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1907 "grammar.c"
  yymsp[-2].minor.yy188 = yylhsminor.yy188;
        break;
      case 83: /* orderClause ::= */
#line 494 "grammar.y"
{
	yymsp[1].minor.yy176 = NULL;
}
#line 1915 "grammar.c"
        break;
      case 84: /* orderClause ::= ORDER BY columnNameList */
#line 497 "grammar.y"
{
	yymsp[-2].minor.yy176 = New_AST_OrderNode(yymsp[0].minor.yy42, ORDER_DIR_ASC);
}
#line 1922 "grammar.c"
        break;
      case 85: /* orderClause ::= ORDER BY columnNameList ASC */
#line 500 "grammar.y"
{
	yymsp[-3].minor.yy176 = New_AST_OrderNode(yymsp[-1].minor.yy42, ORDER_DIR_ASC);
}
#line 1929 "grammar.c"
        break;
      case 86: /* orderClause ::= ORDER BY columnNameList DESC */
#line 503 "grammar.y"
{
	yymsp[-3].minor.yy176 = New_AST_OrderNode(yymsp[-1].minor.yy42, ORDER_DIR_DESC);
}
#line 1936 "grammar.c"
        break;
      case 87: /* columnNameList ::= columnNameList COMMA columnName */
#line 508 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy42, yymsp[0].minor.yy98);
	yylhsminor.yy42 = yymsp[-2].minor.yy42;
}
#line 1944 "grammar.c"
  yymsp[-2].minor.yy42 = yylhsminor.yy42;
        break;
      case 88: /* columnNameList ::= columnName */
#line 512 "grammar.y"
{
	yylhsminor.yy42 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy42, yymsp[0].minor.yy98);
}
#line 1953 "grammar.c"
  yymsp[0].minor.yy42 = yylhsminor.yy42;
        break;
      case 89: /* columnName ::= variable */
#line 518 "grammar.y"
{
	if(yymsp[0].minor.yy188->property != NULL) {
		yylhsminor.yy98 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy188);
	} else {
		yylhsminor.yy98 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy188->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy188);
}
#line 1967 "grammar.c"
  yymsp[0].minor.yy98 = yylhsminor.yy98;
        break;
      case 90: /* skipClause ::= */
#line 530 "grammar.y"
{
	yymsp[1].minor.yy59 = NULL;
}
#line 1975 "grammar.c"
        break;
      case 91: /* skipClause ::= SKIP INTEGER */
#line 533 "grammar.y"
{
	yymsp[-1].minor.yy59 = New_AST_SkipNode(yymsp[0].minor.yy0.intval);
}
#line 1982 "grammar.c"
        break;
      case 92: /* limitClause ::= */
#line 539 "grammar.y"
{
	yymsp[1].minor.yy147 = NULL;
}
#line 1989 "grammar.c"
        break;
      case 93: /* limitClause ::= LIMIT INTEGER */
#line 542 "grammar.y"
{
	yymsp[-1].minor.yy147 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1996 "grammar.c"
        break;
      case 94: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 548 "grammar.y"
{
	yymsp[-5].minor.yy37 = New_AST_UnwindNode(yymsp[-3].minor.yy42, yymsp[0].minor.yy0.strval);
}
#line 2003 "grammar.c"
        break;
      case 95: /* relation ::= EQ */
#line 553 "grammar.y"
{ yymsp[0].minor.yy92 = EQ; }
#line 2008 "grammar.c"
        break;
      case 96: /* relation ::= GT */
#line 554 "grammar.y"
{ yymsp[0].minor.yy92 = GT; }
#line 2013 "grammar.c"
        break;
      case 97: /* relation ::= LT */
#line 555 "grammar.y"
{ yymsp[0].minor.yy92 = LT; }
#line 2018 "grammar.c"
        break;
      case 98: /* relation ::= LE */
#line 556 "grammar.y"
{ yymsp[0].minor.yy92 = LE; }
#line 2023 "grammar.c"
        break;
      case 99: /* relation ::= GE */
#line 557 "grammar.y"
{ yymsp[0].minor.yy92 = GE; }
#line 2028 "grammar.c"
        break;
      case 100: /* relation ::= NE */
#line 558 "grammar.y"
{ yymsp[0].minor.yy92 = NE; }
#line 2033 "grammar.c"
        break;
      case 101: /* value ::= INTEGER */
#line 569 "grammar.y"
{  yylhsminor.yy46 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 2038 "grammar.c"
  yymsp[0].minor.yy46 = yylhsminor.yy46;
        break;
      case 102: /* value ::= DASH INTEGER */
#line 570 "grammar.y"
{  yymsp[-1].minor.yy46 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 2044 "grammar.c"
        break;
      case 103: /* value ::= STRING */
#line 571 "grammar.y"
{  yylhsminor.yy46 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2049 "grammar.c"
  yymsp[0].minor.yy46 = yylhsminor.yy46;
        break;
      case 104: /* value ::= FLOAT */
#line 572 "grammar.y"
{  yylhsminor.yy46 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2055 "grammar.c"
  yymsp[0].minor.yy46 = yylhsminor.yy46;
        break;
      case 105: /* value ::= DASH FLOAT */
#line 573 "grammar.y"
{  yymsp[-1].minor.yy46 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2061 "grammar.c"
        break;
      case 106: /* value ::= TRUE */
#line 574 "grammar.y"
{ yymsp[0].minor.yy46 = SI_BoolVal(1); }
#line 2066 "grammar.c"
        break;
      case 107: /* value ::= FALSE */
#line 575 "grammar.y"
{ yymsp[0].minor.yy46 = SI_BoolVal(0); }
#line 2071 "grammar.c"
        break;
      case 108: /* value ::= NULLVAL */
#line 576 "grammar.y"
{ yymsp[0].minor.yy46 = SI_NullVal(); }
#line 2076 "grammar.c"
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
#line 2141 "grammar.c"
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
#line 578 "grammar.y"


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
#line 2389 "grammar.c"
