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
#define YYNOCODE 93
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_LinkLength* yy18;
  AST_OrderNode* yy20;
  AST_ReturnNode* yy24;
  AST_IndexNode* yy25;
  AST_Query* yy40;
  AST_UnwindNode* yy41;
  int yy52;
  AST_DeleteNode * yy55;
  AST_SkipNode* yy59;
  AST_MergeNode* yy76;
  AST_ColumnNode* yy78;
  AST_CreateNode* yy100;
  AST_IndexOpType yy105;
  AST_MatchNode* yy109;
  AST_NodeEntity* yy110;
  AST_SetNode* yy132;
  AST_ArithmeticExpressionNode* yy138;
  AST_SetElement* yy144;
  SIValue yy150;
  Vector* yy154;
  AST_ReturnElementNode* yy155;
  AST_Variable* yy156;
  AST_LinkEntity* yy157;
  AST_LimitNode* yy159;
  AST_FilterNode* yy162;
  AST_WhereNode* yy171;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             125
#define YYNRULE              106
#define YYNTOKEN             51
#define YY_MAX_SHIFT         124
#define YY_MIN_SHIFTREDUCE   195
#define YY_MAX_SHIFTREDUCE   300
#define YY_ERROR_ACTION      301
#define YY_ACCEPT_ACTION     302
#define YY_NO_ACTION         303
#define YY_MIN_REDUCE        304
#define YY_MAX_REDUCE        409
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
#define YY_ACTTAB_COUNT (320)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   124,  302,   64,  115,  310,  314,   77,  260,   12,   35,
 /*    10 */   110,   11,  312,  313,  332,   55,  319,   88,   63,   23,
 /*    20 */    79,   18,   77,  307,   59,   23,  379,   66,  120,   55,
 /*    30 */   319,  293,  379,   66,   22,  378,   43,    2,  118,  368,
 /*    40 */     1,  378,  330,   93,  116,  368,   33,  293,  295,  296,
 /*    50 */   298,  299,  300,   19,   17,   16,   15,  287,  288,  291,
 /*    60 */   289,  290,   77,  260,  295,  296,  298,  299,  300,   34,
 /*    70 */   263,   21,   20,   48,  332,  215,   79,   18,   97,   36,
 /*    80 */    25,   19,   17,   16,   15,    6,    8,  293,   19,   17,
 /*    90 */    16,   15,   52,    1,  106,  292,  100,   38,  263,   94,
 /*   100 */   329,   93,  332,   83,  295,  296,  298,  299,  300,   19,
 /*   110 */    17,   16,   15,  287,  288,  291,  289,  290,   77,  311,
 /*   120 */    42,  117,   99,   77,   54,  109,  379,   66,  108,   37,
 /*   130 */    55,  319,   79,   18,   39,  378,   52,   79,    7,  367,
 /*   140 */   379,   70,   52,  293,  379,   70,  379,   30,  293,  378,
 /*   150 */    86,  292,  294,  378,   73,  378,   90,  390,   78,  322,
 /*   160 */   295,  296,  298,  299,  300,  295,  296,  298,  299,  300,
 /*   170 */   297,  379,   29,  388,  379,   30,   19,   17,   16,   15,
 /*   180 */   378,   71,   10,  378,  363,  379,   30,  379,   67,   84,
 /*   190 */   379,   68,  379,   69,  378,   74,  378,  379,  376,  378,
 /*   200 */   119,  378,  379,  375,    6,    8,  378,  379,   80,  379,
 /*   210 */    65,  378,   28,  379,   76,   89,  378,  390,  378,   49,
 /*   220 */    14,   35,  378,  253,  114,   82,  332,  103,  101,  277,
 /*   230 */   278,  123,   72,  389,   16,   15,  268,  315,   52,   14,
 /*   240 */    41,   44,  122,  228,   92,   32,   75,   95,   52,   96,
 /*   250 */    45,   98,  104,  111,  320,  105,  107,  333,  358,  113,
 /*   260 */    23,  112,  309,   56,  121,    1,   57,   58,  286,  305,
 /*   270 */    60,   61,    9,   81,  216,    3,   62,  217,    5,   85,
 /*   280 */    40,   87,   27,    8,  229,   91,   13,   26,  235,   46,
 /*   290 */    47,  238,  247,   24,  304,   50,  239,  237,  233,  243,
 /*   300 */   231,  234,  241,  236,  102,  232,   51,   31,  230,   53,
 /*   310 */     4,  262,  303,  303,  274,  283,  303,  303,  285,  119,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    52,   53,   54,   66,   56,   57,    4,    5,   86,   72,
 /*    10 */    84,   63,   64,   65,   77,   67,   68,   69,   56,   13,
 /*    20 */    18,   19,    4,   61,   62,   13,   75,   76,   18,   67,
 /*    30 */    68,   29,   75,   76,   22,   84,   24,   35,   87,   88,
 /*    40 */    34,   84,   74,   75,   87,   88,   19,   29,   46,   47,
 /*    50 */    48,   49,   50,    3,    4,    5,    6,    7,    8,    9,
 /*    60 */    10,   11,    4,    5,   46,   47,   48,   49,   50,   72,
 /*    70 */    20,   12,   13,   80,   77,   16,   18,   19,   17,   18,
 /*    80 */    21,    3,    4,    5,    6,    1,    2,   29,    3,    4,
 /*    90 */     5,    6,   31,   34,   80,   45,   80,   72,   20,   73,
 /*   100 */    74,   75,   77,   44,   46,   47,   48,   49,   50,    3,
 /*   110 */     4,    5,    6,    7,    8,    9,   10,   11,    4,   56,
 /*   120 */    57,   36,   17,    4,   78,   80,   75,   76,   17,   18,
 /*   130 */    67,   68,   18,   19,   70,   84,   31,   18,   19,   88,
 /*   140 */    75,   76,   31,   29,   75,   76,   75,   76,   29,   84,
 /*   150 */    19,   45,   29,   84,   89,   84,   85,   75,   89,   71,
 /*   160 */    46,   47,   48,   49,   50,   46,   47,   48,   49,   50,
 /*   170 */    47,   75,   76,   91,   75,   76,    3,    4,    5,    6,
 /*   180 */    84,   85,   19,   84,   85,   75,   76,   75,   76,   17,
 /*   190 */    75,   76,   75,   76,   84,   85,   84,   75,   76,   84,
 /*   200 */    37,   84,   75,   76,    1,    2,   84,   75,   76,   75,
 /*   210 */    76,   84,   23,   75,   76,   66,   84,   75,   84,    4,
 /*   220 */    23,   72,   84,   20,   17,   28,   77,   29,   30,   40,
 /*   230 */    41,   43,   90,   91,    5,    6,   20,   60,   31,   23,
 /*   240 */    59,   26,   42,   18,   79,   27,    5,   81,   31,   80,
 /*   250 */    82,   80,   82,   18,   68,   81,   80,   77,   83,   80,
 /*   260 */    13,   83,   60,   59,   38,   34,   58,   57,   18,   60,
 /*   270 */    59,   58,   33,   36,   18,   55,   57,   20,   27,   18,
 /*   280 */    15,   14,   23,    2,   18,   23,    7,   23,    4,   18,
 /*   290 */    18,   28,   32,   39,    0,   18,   28,   28,   20,   29,
 /*   300 */    20,   25,   29,   28,   30,   20,   23,   17,   20,   18,
 /*   310 */    23,   18,   92,   92,   18,   29,   92,   92,   29,   37,
 /*   320 */    92,   92,   92,   92,   92,   92,   92,   92,   92,   92,
 /*   330 */    92,   92,   92,   92,   92,   92,   92,   92,   92,   92,
 /*   340 */    92,   92,   92,   92,   92,   92,   92,   92,   92,   92,
 /*   350 */    92,   92,   92,   92,   92,   92,   92,   92,   92,   92,
 /*   360 */    92,   92,   92,   92,   92,   92,   92,   92,   92,   92,
 /*   370 */    92,
};
#define YY_SHIFT_COUNT    (124)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (296)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    59,    2,   58,   12,   58,  114,  119,  119,  119,  119,
 /*    10 */   114,    6,  114,  114,  114,  114,  114,  114,  114,  114,
 /*    20 */    27,   27,   10,   27,   10,   27,   10,   27,   10,   50,
 /*    30 */   106,   18,   61,  111,  215,  215,  105,  207,  215,  131,
 /*    40 */   172,  188,  200,  225,  218,  217,  241,  217,  241,  218,
 /*    50 */   217,  235,  235,  217,   27,  247,  188,  200,  226,  231,
 /*    60 */   188,  200,  226,  231,  239,   78,   85,  173,  173,  173,
 /*    70 */   173,  203,  189,  197,   84,  198,  229,  123,  216,  163,
 /*    80 */   229,  250,  237,  251,  256,  257,  261,  265,  267,  259,
 /*    90 */   281,  266,  262,  279,  264,  284,  263,  271,  268,  272,
 /*   100 */   269,  270,  273,  274,  275,  276,  278,  280,  277,  285,
 /*   110 */   283,  290,  260,  288,  291,  259,  287,  293,  287,  296,
 /*   120 */   282,  254,  286,  289,  294,
};
#define YY_REDUCE_COUNT (64)
#define YY_REDUCE_MIN   (-78)
#define YY_REDUCE_MAX   (220)
static const short yy_reduce_ofst[] = {
 /*     0 */   -52,  -49,  -43,  -38,   51,   65,   71,   96,   99,  110,
 /*    10 */    69,   63,  112,  115,  117,  122,  127,  132,  134,  138,
 /*    20 */   -63,  149,   26,  -63,  142,   -3,  -32,   25,   82,  -78,
 /*    30 */   -78,  -74,   -7,   14,   46,   46,   16,   45,   46,   88,
 /*    40 */    64,  177,  181,  165,  166,  169,  168,  171,  170,  174,
 /*    50 */   176,  175,  178,  179,  180,  186,  202,  204,  208,  210,
 /*    60 */   209,  211,  213,  219,  220,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   317,  301,  301,  317,  301,  301,  301,  301,  301,  301,
 /*    10 */   301,  317,  301,  301,  301,  301,  301,  301,  301,  301,
 /*    20 */   323,  301,  301,  301,  301,  301,  301,  301,  301,  301,
 /*    30 */   301,  301,  355,  355,  327,  334,  355,  355,  335,  301,
 /*    40 */   301,  393,  391,  301,  301,  355,  349,  355,  349,  301,
 /*    50 */   355,  301,  301,  355,  301,  318,  393,  391,  384,  308,
 /*    60 */   393,  391,  384,  306,  359,  301,  370,  361,  331,  380,
 /*    70 */   381,  301,  385,  301,  360,  354,  373,  301,  301,  382,
 /*    80 */   374,  301,  301,  301,  301,  301,  301,  301,  301,  316,
 /*    90 */   364,  301,  336,  301,  328,  301,  301,  301,  301,  301,
 /*   100 */   301,  301,  351,  353,  301,  301,  301,  301,  301,  301,
 /*   110 */   357,  301,  301,  301,  301,  321,  366,  301,  365,  301,
 /*   120 */   382,  301,  301,  301,  301,
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
  /*   66 */ "chains",
  /*   67 */ "createClauses",
  /*   68 */ "createClause",
  /*   69 */ "indexOpToken",
  /*   70 */ "indexLabel",
  /*   71 */ "indexProp",
  /*   72 */ "chain",
  /*   73 */ "setList",
  /*   74 */ "setElement",
  /*   75 */ "variable",
  /*   76 */ "arithmetic_expression",
  /*   77 */ "node",
  /*   78 */ "link",
  /*   79 */ "deleteExpression",
  /*   80 */ "properties",
  /*   81 */ "edge",
  /*   82 */ "edgeLength",
  /*   83 */ "mapLiteral",
  /*   84 */ "value",
  /*   85 */ "cond",
  /*   86 */ "relation",
  /*   87 */ "returnElements",
  /*   88 */ "returnElement",
  /*   89 */ "arithmetic_expression_list",
  /*   90 */ "columnNameList",
  /*   91 */ "columnName",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause",
 /*   2 */ "expr ::= matchClause whereClause multipleCreateClause",
 /*   3 */ "expr ::= matchClause whereClause deleteClause",
 /*   4 */ "expr ::= matchClause whereClause setClause",
 /*   5 */ "expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause",
 /*   6 */ "expr ::= multipleCreateClause",
 /*   7 */ "expr ::= unwindClause multipleCreateClause",
 /*   8 */ "expr ::= indexClause",
 /*   9 */ "expr ::= mergeClause",
 /*  10 */ "expr ::= returnClause",
 /*  11 */ "expr ::= unwindClause returnClause skipClause limitClause",
 /*  12 */ "matchClause ::= MATCH chains",
 /*  13 */ "multipleCreateClause ::=",
 /*  14 */ "multipleCreateClause ::= createClauses",
 /*  15 */ "createClauses ::= createClause",
 /*  16 */ "createClauses ::= createClauses createClause",
 /*  17 */ "createClause ::= CREATE chains",
 /*  18 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  19 */ "indexOpToken ::= CREATE",
 /*  20 */ "indexOpToken ::= DROP",
 /*  21 */ "indexLabel ::= COLON UQSTRING",
 /*  22 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  23 */ "mergeClause ::= MERGE chain",
 /*  24 */ "setClause ::= SET setList",
 /*  25 */ "setList ::= setElement",
 /*  26 */ "setList ::= setList COMMA setElement",
 /*  27 */ "setElement ::= variable EQ arithmetic_expression",
 /*  28 */ "chain ::= node",
 /*  29 */ "chain ::= chain link node",
 /*  30 */ "chains ::= chain",
 /*  31 */ "chains ::= chains COMMA chain",
 /*  32 */ "deleteClause ::= DELETE deleteExpression",
 /*  33 */ "deleteExpression ::= UQSTRING",
 /*  34 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  35 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  36 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  37 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  38 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  39 */ "link ::= DASH edge RIGHT_ARROW",
 /*  40 */ "link ::= LEFT_ARROW edge DASH",
 /*  41 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  42 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  43 */ "edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET",
 /*  44 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  45 */ "edgeLength ::=",
 /*  46 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  47 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  48 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  49 */ "edgeLength ::= MUL INTEGER",
 /*  50 */ "edgeLength ::= MUL",
 /*  51 */ "properties ::=",
 /*  52 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  53 */ "mapLiteral ::= UQSTRING COLON value",
 /*  54 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  55 */ "whereClause ::=",
 /*  56 */ "whereClause ::= WHERE cond",
 /*  57 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  58 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  59 */ "cond ::= cond AND cond",
 /*  60 */ "cond ::= cond OR cond",
 /*  61 */ "returnClause ::= RETURN returnElements",
 /*  62 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  63 */ "returnElements ::= returnElements COMMA returnElement",
 /*  64 */ "returnElements ::= returnElement",
 /*  65 */ "returnElement ::= MUL",
 /*  66 */ "returnElement ::= arithmetic_expression",
 /*  67 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  68 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  69 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  70 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  71 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  72 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  73 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  74 */ "arithmetic_expression ::= value",
 /*  75 */ "arithmetic_expression ::= variable",
 /*  76 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  77 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  78 */ "variable ::= UQSTRING",
 /*  79 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  80 */ "orderClause ::=",
 /*  81 */ "orderClause ::= ORDER BY columnNameList",
 /*  82 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  83 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  84 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  85 */ "columnNameList ::= columnName",
 /*  86 */ "columnName ::= variable",
 /*  87 */ "skipClause ::=",
 /*  88 */ "skipClause ::= SKIP INTEGER",
 /*  89 */ "limitClause ::=",
 /*  90 */ "limitClause ::= LIMIT INTEGER",
 /*  91 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /*  92 */ "relation ::= EQ",
 /*  93 */ "relation ::= GT",
 /*  94 */ "relation ::= LT",
 /*  95 */ "relation ::= LE",
 /*  96 */ "relation ::= GE",
 /*  97 */ "relation ::= NE",
 /*  98 */ "value ::= INTEGER",
 /*  99 */ "value ::= DASH INTEGER",
 /* 100 */ "value ::= STRING",
 /* 101 */ "value ::= FLOAT",
 /* 102 */ "value ::= DASH FLOAT",
 /* 103 */ "value ::= TRUE",
 /* 104 */ "value ::= FALSE",
 /* 105 */ "value ::= NULLVAL",
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
    case 85: /* cond */
{
#line 350 "grammar.y"
 Free_AST_FilterNode((yypminor->yy162)); 
#line 780 "grammar.c"
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
  {   52,   -7 }, /* (1) expr ::= matchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
  {   52,   -3 }, /* (2) expr ::= matchClause whereClause multipleCreateClause */
  {   52,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   52,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   52,   -7 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   52,   -1 }, /* (6) expr ::= multipleCreateClause */
  {   52,   -2 }, /* (7) expr ::= unwindClause multipleCreateClause */
  {   52,   -1 }, /* (8) expr ::= indexClause */
  {   52,   -1 }, /* (9) expr ::= mergeClause */
  {   52,   -1 }, /* (10) expr ::= returnClause */
  {   52,   -4 }, /* (11) expr ::= unwindClause returnClause skipClause limitClause */
  {   54,   -2 }, /* (12) matchClause ::= MATCH chains */
  {   56,    0 }, /* (13) multipleCreateClause ::= */
  {   56,   -1 }, /* (14) multipleCreateClause ::= createClauses */
  {   67,   -1 }, /* (15) createClauses ::= createClause */
  {   67,   -2 }, /* (16) createClauses ::= createClauses createClause */
  {   68,   -2 }, /* (17) createClause ::= CREATE chains */
  {   64,   -5 }, /* (18) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   69,   -1 }, /* (19) indexOpToken ::= CREATE */
  {   69,   -1 }, /* (20) indexOpToken ::= DROP */
  {   70,   -2 }, /* (21) indexLabel ::= COLON UQSTRING */
  {   71,   -3 }, /* (22) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   65,   -2 }, /* (23) mergeClause ::= MERGE chain */
  {   62,   -2 }, /* (24) setClause ::= SET setList */
  {   73,   -1 }, /* (25) setList ::= setElement */
  {   73,   -3 }, /* (26) setList ::= setList COMMA setElement */
  {   74,   -3 }, /* (27) setElement ::= variable EQ arithmetic_expression */
  {   72,   -1 }, /* (28) chain ::= node */
  {   72,   -3 }, /* (29) chain ::= chain link node */
  {   66,   -1 }, /* (30) chains ::= chain */
  {   66,   -3 }, /* (31) chains ::= chains COMMA chain */
  {   61,   -2 }, /* (32) deleteClause ::= DELETE deleteExpression */
  {   79,   -1 }, /* (33) deleteExpression ::= UQSTRING */
  {   79,   -3 }, /* (34) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   77,   -6 }, /* (35) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   77,   -5 }, /* (36) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   77,   -4 }, /* (37) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   77,   -3 }, /* (38) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   78,   -3 }, /* (39) link ::= DASH edge RIGHT_ARROW */
  {   78,   -3 }, /* (40) link ::= LEFT_ARROW edge DASH */
  {   81,   -4 }, /* (41) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   81,   -4 }, /* (42) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   81,   -6 }, /* (43) edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
  {   81,   -6 }, /* (44) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   82,    0 }, /* (45) edgeLength ::= */
  {   82,   -4 }, /* (46) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   82,   -3 }, /* (47) edgeLength ::= MUL INTEGER DOTDOT */
  {   82,   -3 }, /* (48) edgeLength ::= MUL DOTDOT INTEGER */
  {   82,   -2 }, /* (49) edgeLength ::= MUL INTEGER */
  {   82,   -1 }, /* (50) edgeLength ::= MUL */
  {   80,    0 }, /* (51) properties ::= */
  {   80,   -3 }, /* (52) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   83,   -3 }, /* (53) mapLiteral ::= UQSTRING COLON value */
  {   83,   -5 }, /* (54) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   55,    0 }, /* (55) whereClause ::= */
  {   55,   -2 }, /* (56) whereClause ::= WHERE cond */
  {   85,   -3 }, /* (57) cond ::= arithmetic_expression relation arithmetic_expression */
  {   85,   -3 }, /* (58) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   85,   -3 }, /* (59) cond ::= cond AND cond */
  {   85,   -3 }, /* (60) cond ::= cond OR cond */
  {   57,   -2 }, /* (61) returnClause ::= RETURN returnElements */
  {   57,   -3 }, /* (62) returnClause ::= RETURN DISTINCT returnElements */
  {   87,   -3 }, /* (63) returnElements ::= returnElements COMMA returnElement */
  {   87,   -1 }, /* (64) returnElements ::= returnElement */
  {   88,   -1 }, /* (65) returnElement ::= MUL */
  {   88,   -1 }, /* (66) returnElement ::= arithmetic_expression */
  {   88,   -3 }, /* (67) returnElement ::= arithmetic_expression AS UQSTRING */
  {   76,   -3 }, /* (68) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   76,   -3 }, /* (69) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   76,   -3 }, /* (70) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   76,   -3 }, /* (71) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   76,   -3 }, /* (72) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   76,   -4 }, /* (73) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   76,   -1 }, /* (74) arithmetic_expression ::= value */
  {   76,   -1 }, /* (75) arithmetic_expression ::= variable */
  {   89,   -3 }, /* (76) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   89,   -1 }, /* (77) arithmetic_expression_list ::= arithmetic_expression */
  {   75,   -1 }, /* (78) variable ::= UQSTRING */
  {   75,   -3 }, /* (79) variable ::= UQSTRING DOT UQSTRING */
  {   58,    0 }, /* (80) orderClause ::= */
  {   58,   -3 }, /* (81) orderClause ::= ORDER BY columnNameList */
  {   58,   -4 }, /* (82) orderClause ::= ORDER BY columnNameList ASC */
  {   58,   -4 }, /* (83) orderClause ::= ORDER BY columnNameList DESC */
  {   90,   -3 }, /* (84) columnNameList ::= columnNameList COMMA columnName */
  {   90,   -1 }, /* (85) columnNameList ::= columnName */
  {   91,   -1 }, /* (86) columnName ::= variable */
  {   59,    0 }, /* (87) skipClause ::= */
  {   59,   -2 }, /* (88) skipClause ::= SKIP INTEGER */
  {   60,    0 }, /* (89) limitClause ::= */
  {   60,   -2 }, /* (90) limitClause ::= LIMIT INTEGER */
  {   63,   -6 }, /* (91) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {   86,   -1 }, /* (92) relation ::= EQ */
  {   86,   -1 }, /* (93) relation ::= GT */
  {   86,   -1 }, /* (94) relation ::= LT */
  {   86,   -1 }, /* (95) relation ::= LE */
  {   86,   -1 }, /* (96) relation ::= GE */
  {   86,   -1 }, /* (97) relation ::= NE */
  {   84,   -1 }, /* (98) value ::= INTEGER */
  {   84,   -2 }, /* (99) value ::= DASH INTEGER */
  {   84,   -1 }, /* (100) value ::= STRING */
  {   84,   -1 }, /* (101) value ::= FLOAT */
  {   84,   -2 }, /* (102) value ::= DASH FLOAT */
  {   84,   -1 }, /* (103) value ::= TRUE */
  {   84,   -1 }, /* (104) value ::= FALSE */
  {   84,   -1 }, /* (105) value ::= NULLVAL */
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
{ ctx->root = yymsp[0].minor.yy40; }
#line 1263 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
#line 46 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-6].minor.yy109, yymsp[-5].minor.yy171, yymsp[-4].minor.yy100, NULL, NULL, NULL, yymsp[-3].minor.yy24, yymsp[-2].minor.yy20, yymsp[-1].minor.yy59, yymsp[0].minor.yy159, NULL, NULL);
}
#line 1270 "grammar.c"
  yymsp[-6].minor.yy40 = yylhsminor.yy40;
        break;
      case 2: /* expr ::= matchClause whereClause multipleCreateClause */
#line 50 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-2].minor.yy109, yymsp[-1].minor.yy171, yymsp[0].minor.yy100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1278 "grammar.c"
  yymsp[-2].minor.yy40 = yylhsminor.yy40;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 54 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-2].minor.yy109, yymsp[-1].minor.yy171, NULL, NULL, NULL, yymsp[0].minor.yy55, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1286 "grammar.c"
  yymsp[-2].minor.yy40 = yylhsminor.yy40;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 58 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-2].minor.yy109, yymsp[-1].minor.yy171, NULL, NULL, yymsp[0].minor.yy132, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1294 "grammar.c"
  yymsp[-2].minor.yy40 = yylhsminor.yy40;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 62 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-6].minor.yy109, yymsp[-5].minor.yy171, NULL, NULL, yymsp[-4].minor.yy132, NULL, yymsp[-3].minor.yy24, yymsp[-2].minor.yy20, yymsp[-1].minor.yy59, yymsp[0].minor.yy159, NULL, NULL);
}
#line 1302 "grammar.c"
  yymsp[-6].minor.yy40 = yylhsminor.yy40;
        break;
      case 6: /* expr ::= multipleCreateClause */
#line 66 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1310 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 7: /* expr ::= unwindClause multipleCreateClause */
#line 70 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy41);
}
#line 1318 "grammar.c"
  yymsp[-1].minor.yy40 = yylhsminor.yy40;
        break;
      case 8: /* expr ::= indexClause */
#line 74 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy25, NULL);
}
#line 1326 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 9: /* expr ::= mergeClause */
#line 78 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy76, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1334 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 10: /* expr ::= returnClause */
#line 82 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy24, NULL, NULL, NULL, NULL, NULL);
}
#line 1342 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 11: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 86 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy24, NULL, yymsp[-1].minor.yy59, yymsp[0].minor.yy159, NULL, yymsp[-3].minor.yy41);
}
#line 1350 "grammar.c"
  yymsp[-3].minor.yy40 = yylhsminor.yy40;
        break;
      case 12: /* matchClause ::= MATCH chains */
#line 92 "grammar.y"
{
	yymsp[-1].minor.yy109 = New_AST_MatchNode(yymsp[0].minor.yy154);
}
#line 1358 "grammar.c"
        break;
      case 13: /* multipleCreateClause ::= */
#line 97 "grammar.y"
{
	yymsp[1].minor.yy100 = NULL;
}
#line 1365 "grammar.c"
        break;
      case 14: /* multipleCreateClause ::= createClauses */
#line 101 "grammar.y"
{
	yylhsminor.yy100 = New_AST_CreateNode(yymsp[0].minor.yy154);
}
#line 1372 "grammar.c"
  yymsp[0].minor.yy100 = yylhsminor.yy100;
        break;
      case 15: /* createClauses ::= createClause */
#line 108 "grammar.y"
{
	yylhsminor.yy154 = yymsp[0].minor.yy154;
}
#line 1380 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 16: /* createClauses ::= createClauses createClause */
#line 111 "grammar.y"
{
	Vector *v;
	while(Vector_Pop(yymsp[0].minor.yy154, &v)) Vector_Push(yymsp[-1].minor.yy154, v);
	Vector_Free(yymsp[0].minor.yy154);
	yylhsminor.yy154 = yymsp[-1].minor.yy154;
}
#line 1391 "grammar.c"
  yymsp[-1].minor.yy154 = yylhsminor.yy154;
        break;
      case 17: /* createClause ::= CREATE chains */
#line 121 "grammar.y"
{
	yymsp[-1].minor.yy154 = yymsp[0].minor.yy154;
}
#line 1399 "grammar.c"
        break;
      case 18: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 127 "grammar.y"
{
  yylhsminor.yy25 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy105);
}
#line 1406 "grammar.c"
  yymsp[-4].minor.yy25 = yylhsminor.yy25;
        break;
      case 19: /* indexOpToken ::= CREATE */
#line 133 "grammar.y"
{ yymsp[0].minor.yy105 = CREATE_INDEX; }
#line 1412 "grammar.c"
        break;
      case 20: /* indexOpToken ::= DROP */
#line 134 "grammar.y"
{ yymsp[0].minor.yy105 = DROP_INDEX; }
#line 1417 "grammar.c"
        break;
      case 21: /* indexLabel ::= COLON UQSTRING */
#line 136 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1424 "grammar.c"
        break;
      case 22: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 140 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1431 "grammar.c"
        break;
      case 23: /* mergeClause ::= MERGE chain */
#line 146 "grammar.y"
{
	yymsp[-1].minor.yy76 = New_AST_MergeNode(yymsp[0].minor.yy154);
}
#line 1438 "grammar.c"
        break;
      case 24: /* setClause ::= SET setList */
#line 151 "grammar.y"
{
	yymsp[-1].minor.yy132 = New_AST_SetNode(yymsp[0].minor.yy154);
}
#line 1445 "grammar.c"
        break;
      case 25: /* setList ::= setElement */
#line 156 "grammar.y"
{
	yylhsminor.yy154 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy144);
}
#line 1453 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 26: /* setList ::= setList COMMA setElement */
#line 160 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy144);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1462 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 27: /* setElement ::= variable EQ arithmetic_expression */
#line 166 "grammar.y"
{
	yylhsminor.yy144 = New_AST_SetElement(yymsp[-2].minor.yy156, yymsp[0].minor.yy138);
}
#line 1470 "grammar.c"
  yymsp[-2].minor.yy144 = yylhsminor.yy144;
        break;
      case 28: /* chain ::= node */
#line 172 "grammar.y"
{
	yylhsminor.yy154 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy110);
}
#line 1479 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 29: /* chain ::= chain link node */
#line 177 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[-1].minor.yy157);
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy110);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1489 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 30: /* chains ::= chain */
#line 185 "grammar.y"
{
	yylhsminor.yy154 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy154);
}
#line 1498 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 31: /* chains ::= chains COMMA chain */
#line 190 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy154);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1507 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 32: /* deleteClause ::= DELETE deleteExpression */
#line 198 "grammar.y"
{
	yymsp[-1].minor.yy55 = New_AST_DeleteNode(yymsp[0].minor.yy154);
}
#line 1515 "grammar.c"
        break;
      case 33: /* deleteExpression ::= UQSTRING */
#line 204 "grammar.y"
{
	yylhsminor.yy154 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy0.strval);
}
#line 1523 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 34: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 209 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1532 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 35: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 217 "grammar.y"
{
	yymsp[-5].minor.yy110 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy154);
}
#line 1540 "grammar.c"
        break;
      case 36: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 222 "grammar.y"
{
	yymsp[-4].minor.yy110 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy154);
}
#line 1547 "grammar.c"
        break;
      case 37: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 227 "grammar.y"
{
	yymsp[-3].minor.yy110 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy154);
}
#line 1554 "grammar.c"
        break;
      case 38: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 232 "grammar.y"
{
	yymsp[-2].minor.yy110 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy154);
}
#line 1561 "grammar.c"
        break;
      case 39: /* link ::= DASH edge RIGHT_ARROW */
#line 239 "grammar.y"
{
	yymsp[-2].minor.yy157 = yymsp[-1].minor.yy157;
	yymsp[-2].minor.yy157->direction = N_LEFT_TO_RIGHT;
}
#line 1569 "grammar.c"
        break;
      case 40: /* link ::= LEFT_ARROW edge DASH */
#line 245 "grammar.y"
{
	yymsp[-2].minor.yy157 = yymsp[-1].minor.yy157;
	yymsp[-2].minor.yy157->direction = N_RIGHT_TO_LEFT;
}
#line 1577 "grammar.c"
        break;
      case 41: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 252 "grammar.y"
{ 
	yymsp[-3].minor.yy157 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy154, N_DIR_UNKNOWN, yymsp[-1].minor.yy18);
}
#line 1584 "grammar.c"
        break;
      case 42: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 257 "grammar.y"
{ 
	yymsp[-3].minor.yy157 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy154, N_DIR_UNKNOWN, NULL);
}
#line 1591 "grammar.c"
        break;
      case 43: /* edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
#line 262 "grammar.y"
{ 
	yymsp[-5].minor.yy157 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy154, N_DIR_UNKNOWN, yymsp[-2].minor.yy18);
}
#line 1598 "grammar.c"
        break;
      case 44: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 267 "grammar.y"
{ 
	yymsp[-5].minor.yy157 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy154, N_DIR_UNKNOWN, NULL);
}
#line 1605 "grammar.c"
        break;
      case 45: /* edgeLength ::= */
#line 274 "grammar.y"
{
	yymsp[1].minor.yy18 = NULL;
}
#line 1612 "grammar.c"
        break;
      case 46: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 279 "grammar.y"
{
	yymsp[-3].minor.yy18 = New_AST_LinkLength(yymsp[-2].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1619 "grammar.c"
        break;
      case 47: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 284 "grammar.y"
{
	yymsp[-2].minor.yy18 = New_AST_LinkLength(yymsp[-1].minor.yy0.intval, UINT_MAX-2);
}
#line 1626 "grammar.c"
        break;
      case 48: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 289 "grammar.y"
{
	yymsp[-2].minor.yy18 = New_AST_LinkLength(1, yymsp[0].minor.yy0.intval);
}
#line 1633 "grammar.c"
        break;
      case 49: /* edgeLength ::= MUL INTEGER */
#line 294 "grammar.y"
{
	yymsp[-1].minor.yy18 = New_AST_LinkLength(yymsp[0].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1640 "grammar.c"
        break;
      case 50: /* edgeLength ::= MUL */
#line 299 "grammar.y"
{
	yymsp[0].minor.yy18 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1647 "grammar.c"
        break;
      case 51: /* properties ::= */
#line 305 "grammar.y"
{
	yymsp[1].minor.yy154 = NULL;
}
#line 1654 "grammar.c"
        break;
      case 52: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 309 "grammar.y"
{
	yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154;
}
#line 1661 "grammar.c"
        break;
      case 53: /* mapLiteral ::= UQSTRING COLON value */
#line 315 "grammar.y"
{
	yylhsminor.yy154 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy154, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy150;
	Vector_Push(yylhsminor.yy154, val);
}
#line 1676 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 54: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 327 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy154, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy150;
	Vector_Push(yymsp[0].minor.yy154, val);
	
	yylhsminor.yy154 = yymsp[0].minor.yy154;
}
#line 1692 "grammar.c"
  yymsp[-4].minor.yy154 = yylhsminor.yy154;
        break;
      case 55: /* whereClause ::= */
#line 341 "grammar.y"
{ 
	yymsp[1].minor.yy171 = NULL;
}
#line 1700 "grammar.c"
        break;
      case 56: /* whereClause ::= WHERE cond */
#line 344 "grammar.y"
{
	yymsp[-1].minor.yy171 = New_AST_WhereNode(yymsp[0].minor.yy162);
}
#line 1707 "grammar.c"
        break;
      case 57: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 353 "grammar.y"
{ yylhsminor.yy162 = New_AST_PredicateNode(yymsp[-2].minor.yy138, yymsp[-1].minor.yy52, yymsp[0].minor.yy138); }
#line 1712 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 58: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 355 "grammar.y"
{ yymsp[-2].minor.yy162 = yymsp[-1].minor.yy162; }
#line 1718 "grammar.c"
        break;
      case 59: /* cond ::= cond AND cond */
#line 356 "grammar.y"
{ yylhsminor.yy162 = New_AST_ConditionNode(yymsp[-2].minor.yy162, AND, yymsp[0].minor.yy162); }
#line 1723 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 60: /* cond ::= cond OR cond */
#line 357 "grammar.y"
{ yylhsminor.yy162 = New_AST_ConditionNode(yymsp[-2].minor.yy162, OR, yymsp[0].minor.yy162); }
#line 1729 "grammar.c"
  yymsp[-2].minor.yy162 = yylhsminor.yy162;
        break;
      case 61: /* returnClause ::= RETURN returnElements */
#line 361 "grammar.y"
{
	yymsp[-1].minor.yy24 = New_AST_ReturnNode(yymsp[0].minor.yy154, 0);
}
#line 1737 "grammar.c"
        break;
      case 62: /* returnClause ::= RETURN DISTINCT returnElements */
#line 364 "grammar.y"
{
	yymsp[-2].minor.yy24 = New_AST_ReturnNode(yymsp[0].minor.yy154, 1);
}
#line 1744 "grammar.c"
        break;
      case 63: /* returnElements ::= returnElements COMMA returnElement */
#line 370 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy155);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1752 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 64: /* returnElements ::= returnElement */
#line 375 "grammar.y"
{
	yylhsminor.yy154 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy155);
}
#line 1761 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 65: /* returnElement ::= MUL */
#line 383 "grammar.y"
{
	yymsp[0].minor.yy155 = New_AST_ReturnElementExpandALL();
}
#line 1769 "grammar.c"
        break;
      case 66: /* returnElement ::= arithmetic_expression */
#line 386 "grammar.y"
{
	yylhsminor.yy155 = New_AST_ReturnElementNode(yymsp[0].minor.yy138, NULL);
}
#line 1776 "grammar.c"
  yymsp[0].minor.yy155 = yylhsminor.yy155;
        break;
      case 67: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 390 "grammar.y"
{
	yylhsminor.yy155 = New_AST_ReturnElementNode(yymsp[-2].minor.yy138, yymsp[0].minor.yy0.strval);
}
#line 1784 "grammar.c"
  yymsp[-2].minor.yy155 = yylhsminor.yy155;
        break;
      case 68: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 397 "grammar.y"
{
	yymsp[-2].minor.yy138 = yymsp[-1].minor.yy138;
}
#line 1792 "grammar.c"
        break;
      case 69: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 409 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy138);
	Vector_Push(args, yymsp[0].minor.yy138);
	yylhsminor.yy138 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1802 "grammar.c"
  yymsp[-2].minor.yy138 = yylhsminor.yy138;
        break;
      case 70: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 416 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy138);
	Vector_Push(args, yymsp[0].minor.yy138);
	yylhsminor.yy138 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1813 "grammar.c"
  yymsp[-2].minor.yy138 = yylhsminor.yy138;
        break;
      case 71: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 423 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy138);
	Vector_Push(args, yymsp[0].minor.yy138);
	yylhsminor.yy138 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1824 "grammar.c"
  yymsp[-2].minor.yy138 = yylhsminor.yy138;
        break;
      case 72: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 430 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy138);
	Vector_Push(args, yymsp[0].minor.yy138);
	yylhsminor.yy138 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1835 "grammar.c"
  yymsp[-2].minor.yy138 = yylhsminor.yy138;
        break;
      case 73: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 438 "grammar.y"
{
	yylhsminor.yy138 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy154);
}
#line 1843 "grammar.c"
  yymsp[-3].minor.yy138 = yylhsminor.yy138;
        break;
      case 74: /* arithmetic_expression ::= value */
#line 443 "grammar.y"
{
	yylhsminor.yy138 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy150);
}
#line 1851 "grammar.c"
  yymsp[0].minor.yy138 = yylhsminor.yy138;
        break;
      case 75: /* arithmetic_expression ::= variable */
#line 448 "grammar.y"
{
	yylhsminor.yy138 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy156->alias, yymsp[0].minor.yy156->property);
	free(yymsp[0].minor.yy156);
}
#line 1860 "grammar.c"
  yymsp[0].minor.yy138 = yylhsminor.yy138;
        break;
      case 76: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 455 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy138);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1869 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 77: /* arithmetic_expression_list ::= arithmetic_expression */
#line 459 "grammar.y"
{
	yylhsminor.yy154 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy138);
}
#line 1878 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 78: /* variable ::= UQSTRING */
#line 466 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1886 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 79: /* variable ::= UQSTRING DOT UQSTRING */
#line 470 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1894 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 80: /* orderClause ::= */
#line 476 "grammar.y"
{
	yymsp[1].minor.yy20 = NULL;
}
#line 1902 "grammar.c"
        break;
      case 81: /* orderClause ::= ORDER BY columnNameList */
#line 479 "grammar.y"
{
	yymsp[-2].minor.yy20 = New_AST_OrderNode(yymsp[0].minor.yy154, ORDER_DIR_ASC);
}
#line 1909 "grammar.c"
        break;
      case 82: /* orderClause ::= ORDER BY columnNameList ASC */
#line 482 "grammar.y"
{
	yymsp[-3].minor.yy20 = New_AST_OrderNode(yymsp[-1].minor.yy154, ORDER_DIR_ASC);
}
#line 1916 "grammar.c"
        break;
      case 83: /* orderClause ::= ORDER BY columnNameList DESC */
#line 485 "grammar.y"
{
	yymsp[-3].minor.yy20 = New_AST_OrderNode(yymsp[-1].minor.yy154, ORDER_DIR_DESC);
}
#line 1923 "grammar.c"
        break;
      case 84: /* columnNameList ::= columnNameList COMMA columnName */
#line 490 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy154, yymsp[0].minor.yy78);
	yylhsminor.yy154 = yymsp[-2].minor.yy154;
}
#line 1931 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 85: /* columnNameList ::= columnName */
#line 494 "grammar.y"
{
	yylhsminor.yy154 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy154, yymsp[0].minor.yy78);
}
#line 1940 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 86: /* columnName ::= variable */
#line 500 "grammar.y"
{
	if(yymsp[0].minor.yy156->property != NULL) {
		yylhsminor.yy78 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy156);
	} else {
		yylhsminor.yy78 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy156->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy156);
}
#line 1954 "grammar.c"
  yymsp[0].minor.yy78 = yylhsminor.yy78;
        break;
      case 87: /* skipClause ::= */
#line 512 "grammar.y"
{
	yymsp[1].minor.yy59 = NULL;
}
#line 1962 "grammar.c"
        break;
      case 88: /* skipClause ::= SKIP INTEGER */
#line 515 "grammar.y"
{
	yymsp[-1].minor.yy59 = New_AST_SkipNode(yymsp[0].minor.yy0.intval);
}
#line 1969 "grammar.c"
        break;
      case 89: /* limitClause ::= */
#line 521 "grammar.y"
{
	yymsp[1].minor.yy159 = NULL;
}
#line 1976 "grammar.c"
        break;
      case 90: /* limitClause ::= LIMIT INTEGER */
#line 524 "grammar.y"
{
	yymsp[-1].minor.yy159 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1983 "grammar.c"
        break;
      case 91: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 530 "grammar.y"
{
	yymsp[-5].minor.yy41 = New_AST_UnwindNode(yymsp[-3].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 1990 "grammar.c"
        break;
      case 92: /* relation ::= EQ */
#line 535 "grammar.y"
{ yymsp[0].minor.yy52 = EQ; }
#line 1995 "grammar.c"
        break;
      case 93: /* relation ::= GT */
#line 536 "grammar.y"
{ yymsp[0].minor.yy52 = GT; }
#line 2000 "grammar.c"
        break;
      case 94: /* relation ::= LT */
#line 537 "grammar.y"
{ yymsp[0].minor.yy52 = LT; }
#line 2005 "grammar.c"
        break;
      case 95: /* relation ::= LE */
#line 538 "grammar.y"
{ yymsp[0].minor.yy52 = LE; }
#line 2010 "grammar.c"
        break;
      case 96: /* relation ::= GE */
#line 539 "grammar.y"
{ yymsp[0].minor.yy52 = GE; }
#line 2015 "grammar.c"
        break;
      case 97: /* relation ::= NE */
#line 540 "grammar.y"
{ yymsp[0].minor.yy52 = NE; }
#line 2020 "grammar.c"
        break;
      case 98: /* value ::= INTEGER */
#line 551 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 2025 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 99: /* value ::= DASH INTEGER */
#line 552 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 2031 "grammar.c"
        break;
      case 100: /* value ::= STRING */
#line 553 "grammar.y"
{  yylhsminor.yy150 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2036 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 101: /* value ::= FLOAT */
#line 554 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2042 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 102: /* value ::= DASH FLOAT */
#line 555 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2048 "grammar.c"
        break;
      case 103: /* value ::= TRUE */
#line 556 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(1); }
#line 2053 "grammar.c"
        break;
      case 104: /* value ::= FALSE */
#line 557 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(0); }
#line 2058 "grammar.c"
        break;
      case 105: /* value ::= NULLVAL */
#line 558 "grammar.y"
{ yymsp[0].minor.yy150 = SI_NullVal(); }
#line 2063 "grammar.c"
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
#line 2128 "grammar.c"
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
#line 560 "grammar.y"


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
#line 2376 "grammar.c"
