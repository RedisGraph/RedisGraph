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
#define YYNOCODE 89
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_ArithmeticExpressionNode* yy2;
  AST_SetNode* yy12;
  SIValue yy22;
  AST_DeleteNode * yy31;
  AST_IndexNode* yy32;
  AST_Query* yy40;
  AST_ReturnNode* yy48;
  AST_OrderNode* yy52;
  AST_ColumnNode* yy54;
  AST_LinkEntity* yy61;
  AST_NodeEntity* yy69;
  AST_FilterNode* yy82;
  AST_WhereNode* yy83;
  AST_SetElement* yy88;
  AST_IndexOpType yy89;
  int yy108;
  AST_SkipNode* yy115;
  AST_Variable* yy116;
  AST_LimitNode* yy127;
  Vector* yy130;
  AST_MergeNode* yy132;
  AST_MatchNode* yy133;
  AST_LinkLength* yy138;
  AST_CreateNode* yy140;
  AST_ReturnElementNode* yy170;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             116
#define YYNRULE              100
#define YYNTOKEN             50
#define YY_MAX_SHIFT         115
#define YY_MIN_SHIFTREDUCE   181
#define YY_MAX_SHIFTREDUCE   280
#define YY_ERROR_ACTION      281
#define YY_ACCEPT_ACTION     282
#define YY_NO_ACTION         283
#define YY_MIN_REDUCE        284
#define YY_MAX_REDUCE        383
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
#define YY_ACTTAB_COUNT (302)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */     9,   58,  101,   32,   71,  241,  287,   54,  307,   16,
 /*    10 */    14,   13,   12,  267,  268,  271,  269,  270,   73,   15,
 /*    20 */   115,  282,   59,  365,  290,  293,  244,  354,   28,  273,
 /*    30 */    31,  291,  292,   21,   79,    2,  353,   81,   67,  364,
 /*    40 */    71,  241,   20,  111,   39,  275,  276,  278,  279,  280,
 /*    50 */   272,  354,   61,   71,   73,   15,   16,   14,   13,   12,
 /*    60 */   353,    4,    6,  109,  343,  273,   44,   73,    5,   16,
 /*    70 */    14,   13,   12,  267,  268,  271,  269,  270,  273,   71,
 /*    80 */   234,  275,  276,  278,  279,  280,   71,  305,   84,  108,
 /*    90 */   354,   61,   97,  365,  275,  276,  278,  279,  280,  353,
 /*   100 */    73,   15,  107,  343,  273,   16,   14,   13,   12,  363,
 /*   110 */   272,  273,   85,  304,   84,   16,   14,   13,   12,   50,
 /*   120 */   275,  276,  278,  279,  280,    4,    6,  275,  276,  278,
 /*   130 */   279,  280,  244,   19,   18,  354,   61,  196,   36,   91,
 /*   140 */   354,   27,   23,  307,  353,  354,   65,  100,  342,  353,
 /*   150 */    66,  354,   28,   77,  353,    1,  354,   28,   75,   72,
 /*   160 */   353,  338,  354,   62,   45,  353,   68,  354,   63,  354,
 /*   170 */    64,  353,  354,  351,  354,  350,  353,  297,  353,  354,
 /*   180 */    74,  353,   37,  353,   94,   92,   40,  106,  353,  354,
 /*   190 */    60,   33,   26,  354,   70,   80,  307,   90,  353,   33,
 /*   200 */   274,  105,  353,  209,  307,   88,   34,   99,   35,  258,
 /*   210 */   259,   48,   13,   12,    8,   48,   30,  277,   83,   48,
 /*   220 */   249,   48,   86,   11,   48,   87,   69,   89,   41,   98,
 /*   230 */    95,   96,  110,  102,  104,  333,  103,  308,  289,  114,
 /*   240 */   113,   51,  112,   52,    1,    7,  285,   53,  197,  198,
 /*   250 */    55,   56,   76,   57,   38,   17,   78,    6,   25,  210,
 /*   260 */    82,   10,  216,   24,  219,   42,  220,   43,  214,   46,
 /*   270 */    29,  218,   22,  224,  222,   49,   93,  215,  217,  212,
 /*   280 */   213,  284,  211,  243,   47,  255,    3,  283,  228,  283,
 /*   290 */   283,  283,  283,  283,  283,  264,  283,  266,  283,  283,
 /*   300 */   283,  110,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    82,   55,   80,   68,    4,    5,   60,   61,   73,    3,
 /*    10 */     4,    5,    6,    7,    8,    9,   10,   11,   18,   19,
 /*    20 */    51,   52,   53,   71,   55,   56,   20,   71,   72,   29,
 /*    30 */    19,   62,   63,   13,   65,   35,   80,   81,   86,   87,
 /*    40 */     4,    5,   22,   18,   24,   45,   46,   47,   48,   49,
 /*    50 */    44,   71,   72,    4,   18,   19,    3,    4,    5,    6,
 /*    60 */    80,    1,    2,   83,   84,   29,   76,   18,   19,    3,
 /*    70 */     4,    5,    6,    7,    8,    9,   10,   11,   29,    4,
 /*    80 */    20,   45,   46,   47,   48,   49,    4,   70,   71,   36,
 /*    90 */    71,   72,   76,   71,   45,   46,   47,   48,   49,   80,
 /*   100 */    18,   19,   83,   84,   29,    3,    4,    5,    6,   87,
 /*   110 */    44,   29,   69,   70,   71,    3,    4,    5,    6,   74,
 /*   120 */    45,   46,   47,   48,   49,    1,    2,   45,   46,   47,
 /*   130 */    48,   49,   20,   12,   13,   71,   72,   16,   68,   76,
 /*   140 */    71,   72,   21,   73,   80,   71,   72,   76,   84,   80,
 /*   150 */    81,   71,   72,   19,   80,   34,   71,   72,   17,   85,
 /*   160 */    80,   81,   71,   72,    4,   80,   81,   71,   72,   71,
 /*   170 */    72,   80,   71,   72,   71,   72,   80,   67,   80,   71,
 /*   180 */    72,   80,   66,   80,   29,   30,   26,   64,   80,   71,
 /*   190 */    72,   68,   23,   71,   72,   64,   73,   17,   80,   68,
 /*   200 */    29,   17,   80,   18,   73,   17,   18,   17,   18,   40,
 /*   210 */    41,   31,    5,    6,   19,   31,   27,   46,   75,   31,
 /*   220 */    20,   31,   77,   23,   31,   76,    5,   76,   78,   76,
 /*   230 */    78,   77,   37,   18,   76,   79,   79,   73,   59,   43,
 /*   240 */    42,   58,   38,   57,   34,   33,   59,   56,   18,   20,
 /*   250 */    58,   57,   18,   56,   15,   54,   14,    2,   23,   18,
 /*   260 */    23,    7,    4,   23,   28,   18,   28,   18,   20,   18,
 /*   270 */    17,   28,   39,   29,   29,   18,   30,   25,   28,   20,
 /*   280 */    20,    0,   20,   18,   23,   18,   23,   88,   32,   88,
 /*   290 */    88,   88,   88,   88,   88,   29,   88,   29,   88,   88,
 /*   300 */    88,   37,   88,   88,   88,   88,   88,   88,   88,   88,
 /*   310 */    88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
 /*   320 */    88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
 /*   330 */    88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
 /*   340 */    88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
 /*   350 */    88,   88,
};
#define YY_SHIFT_COUNT    (115)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (281)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */   121,    0,   36,   36,   49,   49,   49,   49,   82,   82,
 /*    10 */    82,   82,   82,   82,   82,   82,   82,   20,   11,   11,
 /*    20 */    25,   11,   25,   11,   25,   11,   25,    6,   66,   75,
 /*    30 */   188,  190,  160,  160,  180,  184,  160,  134,  141,  185,
 /*    40 */   189,  193,  221,  193,  221,  189,  193,  215,  215,  193,
 /*    50 */    11,  196,  198,  204,  210,  196,  198,  204,  210,  212,
 /*    60 */   112,   53,  102,  102,  102,  102,   60,  169,  124,  155,
 /*    70 */   207,  171,  200,  195,  207,  230,  229,  234,  239,  242,
 /*    80 */   235,  255,  241,  237,  254,  240,  258,  236,  247,  238,
 /*    90 */   249,  243,  244,  245,  246,  250,  252,  248,  259,  251,
 /*   100 */   260,  261,  253,  256,  262,  257,  235,  263,  265,  263,
 /*   110 */   267,  264,  233,  266,  268,  281,
};
#define YY_REDUCE_COUNT (59)
#define YY_REDUCE_MIN   (-82)
#define YY_REDUCE_MAX   (201)
static const short yy_reduce_ofst[] = {
 /*     0 */   -31,  -20,   19,   64,  -44,   69,   80,   85,   74,   91,
 /*    10 */    96,   98,  101,  103,  108,  118,  122,  -54,  123,  131,
 /*    20 */    43,  123,  -48,  -65,   17,   70,   22,  -82,  -82,  -78,
 /*    30 */   -10,   16,   45,   45,   63,   71,   45,  110,  116,  143,
 /*    40 */   145,  149,  150,  151,  152,  154,  153,  156,  157,  158,
 /*    50 */   164,  179,  183,  186,  191,  187,  192,  194,  197,  201,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   295,  281,  281,  281,  281,  281,  281,  281,  281,  281,
 /*    10 */   281,  281,  281,  281,  281,  281,  281,  295,  298,  281,
 /*    20 */   281,  281,  281,  281,  281,  281,  281,  281,  281,  281,
 /*    30 */   330,  330,  302,  309,  330,  330,  310,  281,  281,  281,
 /*    40 */   281,  330,  324,  330,  324,  281,  330,  281,  281,  330,
 /*    50 */   281,  368,  366,  359,  288,  368,  366,  359,  286,  334,
 /*    60 */   281,  345,  336,  306,  355,  356,  281,  360,  335,  329,
 /*    70 */   348,  281,  281,  357,  349,  281,  281,  281,  281,  281,
 /*    80 */   294,  339,  281,  311,  281,  303,  281,  281,  281,  281,
 /*    90 */   281,  281,  281,  326,  328,  281,  281,  281,  281,  281,
 /*   100 */   281,  332,  281,  281,  281,  281,  296,  341,  281,  340,
 /*   110 */   281,  357,  281,  281,  281,  281,
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
  /*   44 */ "NE",
  /*   45 */ "STRING",
  /*   46 */ "FLOAT",
  /*   47 */ "TRUE",
  /*   48 */ "FALSE",
  /*   49 */ "NULLVAL",
  /*   50 */ "error",
  /*   51 */ "expr",
  /*   52 */ "query",
  /*   53 */ "matchClause",
  /*   54 */ "whereClause",
  /*   55 */ "createClause",
  /*   56 */ "returnClause",
  /*   57 */ "orderClause",
  /*   58 */ "skipClause",
  /*   59 */ "limitClause",
  /*   60 */ "deleteClause",
  /*   61 */ "setClause",
  /*   62 */ "indexClause",
  /*   63 */ "mergeClause",
  /*   64 */ "chains",
  /*   65 */ "indexOpToken",
  /*   66 */ "indexLabel",
  /*   67 */ "indexProp",
  /*   68 */ "chain",
  /*   69 */ "setList",
  /*   70 */ "setElement",
  /*   71 */ "variable",
  /*   72 */ "arithmetic_expression",
  /*   73 */ "node",
  /*   74 */ "link",
  /*   75 */ "deleteExpression",
  /*   76 */ "properties",
  /*   77 */ "edge",
  /*   78 */ "edgeLength",
  /*   79 */ "mapLiteral",
  /*   80 */ "value",
  /*   81 */ "cond",
  /*   82 */ "relation",
  /*   83 */ "returnElements",
  /*   84 */ "returnElement",
  /*   85 */ "arithmetic_expression_list",
  /*   86 */ "columnNameList",
  /*   87 */ "columnName",
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
 /*  36 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  37 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  38 */ "edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET",
 /*  39 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  40 */ "edgeLength ::=",
 /*  41 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  42 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  43 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  44 */ "edgeLength ::= MUL INTEGER",
 /*  45 */ "edgeLength ::= MUL",
 /*  46 */ "properties ::=",
 /*  47 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  48 */ "mapLiteral ::= UQSTRING COLON value",
 /*  49 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  50 */ "whereClause ::=",
 /*  51 */ "whereClause ::= WHERE cond",
 /*  52 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  53 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  54 */ "cond ::= cond AND cond",
 /*  55 */ "cond ::= cond OR cond",
 /*  56 */ "returnClause ::= RETURN returnElements",
 /*  57 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  58 */ "returnElements ::= returnElements COMMA returnElement",
 /*  59 */ "returnElements ::= returnElement",
 /*  60 */ "returnElement ::= MUL",
 /*  61 */ "returnElement ::= arithmetic_expression",
 /*  62 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  63 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  64 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  65 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  66 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  67 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  68 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  69 */ "arithmetic_expression ::= value",
 /*  70 */ "arithmetic_expression ::= variable",
 /*  71 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  72 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  73 */ "variable ::= UQSTRING",
 /*  74 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  75 */ "orderClause ::=",
 /*  76 */ "orderClause ::= ORDER BY columnNameList",
 /*  77 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  78 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  79 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  80 */ "columnNameList ::= columnName",
 /*  81 */ "columnName ::= variable",
 /*  82 */ "skipClause ::=",
 /*  83 */ "skipClause ::= SKIP INTEGER",
 /*  84 */ "limitClause ::=",
 /*  85 */ "limitClause ::= LIMIT INTEGER",
 /*  86 */ "relation ::= EQ",
 /*  87 */ "relation ::= GT",
 /*  88 */ "relation ::= LT",
 /*  89 */ "relation ::= LE",
 /*  90 */ "relation ::= GE",
 /*  91 */ "relation ::= NE",
 /*  92 */ "value ::= INTEGER",
 /*  93 */ "value ::= DASH INTEGER",
 /*  94 */ "value ::= STRING",
 /*  95 */ "value ::= FLOAT",
 /*  96 */ "value ::= DASH FLOAT",
 /*  97 */ "value ::= TRUE",
 /*  98 */ "value ::= FALSE",
 /*  99 */ "value ::= NULLVAL",
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
    case 81: /* cond */
{
#line 324 "grammar.y"
 Free_AST_FilterNode((yypminor->yy82)); 
#line 763 "grammar.c"
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
  {   52,   -1 }, /* (0) query ::= expr */
  {   51,   -7 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
  {   51,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   51,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   51,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   51,   -7 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   51,   -1 }, /* (6) expr ::= createClause */
  {   51,   -1 }, /* (7) expr ::= indexClause */
  {   51,   -1 }, /* (8) expr ::= mergeClause */
  {   51,   -1 }, /* (9) expr ::= returnClause */
  {   53,   -2 }, /* (10) matchClause ::= MATCH chains */
  {   55,    0 }, /* (11) createClause ::= */
  {   55,   -2 }, /* (12) createClause ::= CREATE chains */
  {   62,   -5 }, /* (13) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   65,   -1 }, /* (14) indexOpToken ::= CREATE */
  {   65,   -1 }, /* (15) indexOpToken ::= DROP */
  {   66,   -2 }, /* (16) indexLabel ::= COLON UQSTRING */
  {   67,   -3 }, /* (17) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   63,   -2 }, /* (18) mergeClause ::= MERGE chain */
  {   61,   -2 }, /* (19) setClause ::= SET setList */
  {   69,   -1 }, /* (20) setList ::= setElement */
  {   69,   -3 }, /* (21) setList ::= setList COMMA setElement */
  {   70,   -3 }, /* (22) setElement ::= variable EQ arithmetic_expression */
  {   68,   -1 }, /* (23) chain ::= node */
  {   68,   -3 }, /* (24) chain ::= chain link node */
  {   64,   -1 }, /* (25) chains ::= chain */
  {   64,   -3 }, /* (26) chains ::= chains COMMA chain */
  {   60,   -2 }, /* (27) deleteClause ::= DELETE deleteExpression */
  {   75,   -1 }, /* (28) deleteExpression ::= UQSTRING */
  {   75,   -3 }, /* (29) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   73,   -6 }, /* (30) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   73,   -5 }, /* (31) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   73,   -4 }, /* (32) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   73,   -3 }, /* (33) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   74,   -3 }, /* (34) link ::= DASH edge RIGHT_ARROW */
  {   74,   -3 }, /* (35) link ::= LEFT_ARROW edge DASH */
  {   77,   -4 }, /* (36) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   77,   -4 }, /* (37) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   77,   -6 }, /* (38) edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
  {   77,   -6 }, /* (39) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   78,    0 }, /* (40) edgeLength ::= */
  {   78,   -4 }, /* (41) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   78,   -3 }, /* (42) edgeLength ::= MUL INTEGER DOTDOT */
  {   78,   -3 }, /* (43) edgeLength ::= MUL DOTDOT INTEGER */
  {   78,   -2 }, /* (44) edgeLength ::= MUL INTEGER */
  {   78,   -1 }, /* (45) edgeLength ::= MUL */
  {   76,    0 }, /* (46) properties ::= */
  {   76,   -3 }, /* (47) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   79,   -3 }, /* (48) mapLiteral ::= UQSTRING COLON value */
  {   79,   -5 }, /* (49) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   54,    0 }, /* (50) whereClause ::= */
  {   54,   -2 }, /* (51) whereClause ::= WHERE cond */
  {   81,   -3 }, /* (52) cond ::= arithmetic_expression relation arithmetic_expression */
  {   81,   -3 }, /* (53) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   81,   -3 }, /* (54) cond ::= cond AND cond */
  {   81,   -3 }, /* (55) cond ::= cond OR cond */
  {   56,   -2 }, /* (56) returnClause ::= RETURN returnElements */
  {   56,   -3 }, /* (57) returnClause ::= RETURN DISTINCT returnElements */
  {   83,   -3 }, /* (58) returnElements ::= returnElements COMMA returnElement */
  {   83,   -1 }, /* (59) returnElements ::= returnElement */
  {   84,   -1 }, /* (60) returnElement ::= MUL */
  {   84,   -1 }, /* (61) returnElement ::= arithmetic_expression */
  {   84,   -3 }, /* (62) returnElement ::= arithmetic_expression AS UQSTRING */
  {   72,   -3 }, /* (63) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   72,   -3 }, /* (64) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   72,   -3 }, /* (65) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   72,   -3 }, /* (66) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   72,   -3 }, /* (67) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   72,   -4 }, /* (68) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   72,   -1 }, /* (69) arithmetic_expression ::= value */
  {   72,   -1 }, /* (70) arithmetic_expression ::= variable */
  {   85,   -3 }, /* (71) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   85,   -1 }, /* (72) arithmetic_expression_list ::= arithmetic_expression */
  {   71,   -1 }, /* (73) variable ::= UQSTRING */
  {   71,   -3 }, /* (74) variable ::= UQSTRING DOT UQSTRING */
  {   57,    0 }, /* (75) orderClause ::= */
  {   57,   -3 }, /* (76) orderClause ::= ORDER BY columnNameList */
  {   57,   -4 }, /* (77) orderClause ::= ORDER BY columnNameList ASC */
  {   57,   -4 }, /* (78) orderClause ::= ORDER BY columnNameList DESC */
  {   86,   -3 }, /* (79) columnNameList ::= columnNameList COMMA columnName */
  {   86,   -1 }, /* (80) columnNameList ::= columnName */
  {   87,   -1 }, /* (81) columnName ::= variable */
  {   58,    0 }, /* (82) skipClause ::= */
  {   58,   -2 }, /* (83) skipClause ::= SKIP INTEGER */
  {   59,    0 }, /* (84) limitClause ::= */
  {   59,   -2 }, /* (85) limitClause ::= LIMIT INTEGER */
  {   82,   -1 }, /* (86) relation ::= EQ */
  {   82,   -1 }, /* (87) relation ::= GT */
  {   82,   -1 }, /* (88) relation ::= LT */
  {   82,   -1 }, /* (89) relation ::= LE */
  {   82,   -1 }, /* (90) relation ::= GE */
  {   82,   -1 }, /* (91) relation ::= NE */
  {   80,   -1 }, /* (92) value ::= INTEGER */
  {   80,   -2 }, /* (93) value ::= DASH INTEGER */
  {   80,   -1 }, /* (94) value ::= STRING */
  {   80,   -1 }, /* (95) value ::= FLOAT */
  {   80,   -2 }, /* (96) value ::= DASH FLOAT */
  {   80,   -1 }, /* (97) value ::= TRUE */
  {   80,   -1 }, /* (98) value ::= FALSE */
  {   80,   -1 }, /* (99) value ::= NULLVAL */
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
#line 1240 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
#line 46 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-6].minor.yy133, yymsp[-5].minor.yy83, yymsp[-4].minor.yy140, NULL, NULL, NULL, yymsp[-3].minor.yy48, yymsp[-2].minor.yy52, yymsp[-1].minor.yy115, yymsp[0].minor.yy127, NULL);
}
#line 1247 "grammar.c"
  yymsp[-6].minor.yy40 = yylhsminor.yy40;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 50 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-2].minor.yy133, yymsp[-1].minor.yy83, yymsp[0].minor.yy140, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1255 "grammar.c"
  yymsp[-2].minor.yy40 = yylhsminor.yy40;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 54 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-2].minor.yy133, yymsp[-1].minor.yy83, NULL, NULL, NULL, yymsp[0].minor.yy31, NULL, NULL, NULL, NULL, NULL);
}
#line 1263 "grammar.c"
  yymsp[-2].minor.yy40 = yylhsminor.yy40;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 58 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-2].minor.yy133, yymsp[-1].minor.yy83, NULL, NULL, yymsp[0].minor.yy12, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1271 "grammar.c"
  yymsp[-2].minor.yy40 = yylhsminor.yy40;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 62 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(yymsp[-6].minor.yy133, yymsp[-5].minor.yy83, NULL, NULL, yymsp[-4].minor.yy12, NULL, yymsp[-3].minor.yy48, yymsp[-2].minor.yy52, yymsp[-1].minor.yy115, yymsp[0].minor.yy127, NULL);
}
#line 1279 "grammar.c"
  yymsp[-6].minor.yy40 = yylhsminor.yy40;
        break;
      case 6: /* expr ::= createClause */
#line 66 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy140, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1287 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 7: /* expr ::= indexClause */
#line 70 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy32);
}
#line 1295 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 8: /* expr ::= mergeClause */
#line 74 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy132, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1303 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 9: /* expr ::= returnClause */
#line 78 "grammar.y"
{
	yylhsminor.yy40 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy48, NULL, NULL, NULL, NULL);
}
#line 1311 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 10: /* matchClause ::= MATCH chains */
#line 84 "grammar.y"
{
	yymsp[-1].minor.yy133 = New_AST_MatchNode(yymsp[0].minor.yy130);
}
#line 1319 "grammar.c"
        break;
      case 11: /* createClause ::= */
#line 91 "grammar.y"
{
	yymsp[1].minor.yy140 = NULL;
}
#line 1326 "grammar.c"
        break;
      case 12: /* createClause ::= CREATE chains */
#line 95 "grammar.y"
{
	yymsp[-1].minor.yy140 = New_AST_CreateNode(yymsp[0].minor.yy130);
}
#line 1333 "grammar.c"
        break;
      case 13: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 101 "grammar.y"
{
  yylhsminor.yy32 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy89);
}
#line 1340 "grammar.c"
  yymsp[-4].minor.yy32 = yylhsminor.yy32;
        break;
      case 14: /* indexOpToken ::= CREATE */
#line 107 "grammar.y"
{ yymsp[0].minor.yy89 = CREATE_INDEX; }
#line 1346 "grammar.c"
        break;
      case 15: /* indexOpToken ::= DROP */
#line 108 "grammar.y"
{ yymsp[0].minor.yy89 = DROP_INDEX; }
#line 1351 "grammar.c"
        break;
      case 16: /* indexLabel ::= COLON UQSTRING */
#line 110 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1358 "grammar.c"
        break;
      case 17: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 114 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1365 "grammar.c"
        break;
      case 18: /* mergeClause ::= MERGE chain */
#line 120 "grammar.y"
{
	yymsp[-1].minor.yy132 = New_AST_MergeNode(yymsp[0].minor.yy130);
}
#line 1372 "grammar.c"
        break;
      case 19: /* setClause ::= SET setList */
#line 125 "grammar.y"
{
	yymsp[-1].minor.yy12 = New_AST_SetNode(yymsp[0].minor.yy130);
}
#line 1379 "grammar.c"
        break;
      case 20: /* setList ::= setElement */
#line 130 "grammar.y"
{
	yylhsminor.yy130 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy88);
}
#line 1387 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 21: /* setList ::= setList COMMA setElement */
#line 134 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy88);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1396 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 22: /* setElement ::= variable EQ arithmetic_expression */
#line 140 "grammar.y"
{
	yylhsminor.yy88 = New_AST_SetElement(yymsp[-2].minor.yy116, yymsp[0].minor.yy2);
}
#line 1404 "grammar.c"
  yymsp[-2].minor.yy88 = yylhsminor.yy88;
        break;
      case 23: /* chain ::= node */
#line 146 "grammar.y"
{
	yylhsminor.yy130 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy69);
}
#line 1413 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 24: /* chain ::= chain link node */
#line 151 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[-1].minor.yy61);
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy69);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1423 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 25: /* chains ::= chain */
#line 159 "grammar.y"
{
	yylhsminor.yy130 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy130);
}
#line 1432 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 26: /* chains ::= chains COMMA chain */
#line 164 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy130);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1441 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 27: /* deleteClause ::= DELETE deleteExpression */
#line 172 "grammar.y"
{
	yymsp[-1].minor.yy31 = New_AST_DeleteNode(yymsp[0].minor.yy130);
}
#line 1449 "grammar.c"
        break;
      case 28: /* deleteExpression ::= UQSTRING */
#line 178 "grammar.y"
{
	yylhsminor.yy130 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy0.strval);
}
#line 1457 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 29: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 183 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy0.strval);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1466 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 30: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 191 "grammar.y"
{
	yymsp[-5].minor.yy69 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy130);
}
#line 1474 "grammar.c"
        break;
      case 31: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 196 "grammar.y"
{
	yymsp[-4].minor.yy69 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy130);
}
#line 1481 "grammar.c"
        break;
      case 32: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 201 "grammar.y"
{
	yymsp[-3].minor.yy69 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy130);
}
#line 1488 "grammar.c"
        break;
      case 33: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 206 "grammar.y"
{
	yymsp[-2].minor.yy69 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy130);
}
#line 1495 "grammar.c"
        break;
      case 34: /* link ::= DASH edge RIGHT_ARROW */
#line 213 "grammar.y"
{
	yymsp[-2].minor.yy61 = yymsp[-1].minor.yy61;
	yymsp[-2].minor.yy61->direction = N_LEFT_TO_RIGHT;
}
#line 1503 "grammar.c"
        break;
      case 35: /* link ::= LEFT_ARROW edge DASH */
#line 219 "grammar.y"
{
	yymsp[-2].minor.yy61 = yymsp[-1].minor.yy61;
	yymsp[-2].minor.yy61->direction = N_RIGHT_TO_LEFT;
}
#line 1511 "grammar.c"
        break;
      case 36: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 226 "grammar.y"
{ 
	yymsp[-3].minor.yy61 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy130, N_DIR_UNKNOWN, yymsp[-1].minor.yy138);
}
#line 1518 "grammar.c"
        break;
      case 37: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 231 "grammar.y"
{ 
	yymsp[-3].minor.yy61 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy130, N_DIR_UNKNOWN, NULL);
}
#line 1525 "grammar.c"
        break;
      case 38: /* edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
#line 236 "grammar.y"
{ 
	yymsp[-5].minor.yy61 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy130, N_DIR_UNKNOWN, yymsp[-2].minor.yy138);
}
#line 1532 "grammar.c"
        break;
      case 39: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 241 "grammar.y"
{ 
	yymsp[-5].minor.yy61 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy130, N_DIR_UNKNOWN, NULL);
}
#line 1539 "grammar.c"
        break;
      case 40: /* edgeLength ::= */
#line 248 "grammar.y"
{
	yymsp[1].minor.yy138 = NULL;
}
#line 1546 "grammar.c"
        break;
      case 41: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 253 "grammar.y"
{
	yymsp[-3].minor.yy138 = New_AST_LinkLength(yymsp[-2].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1553 "grammar.c"
        break;
      case 42: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 258 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(yymsp[-1].minor.yy0.intval, UINT_MAX-2);
}
#line 1560 "grammar.c"
        break;
      case 43: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 263 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(1, yymsp[0].minor.yy0.intval);
}
#line 1567 "grammar.c"
        break;
      case 44: /* edgeLength ::= MUL INTEGER */
#line 268 "grammar.y"
{
	yymsp[-1].minor.yy138 = New_AST_LinkLength(yymsp[0].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1574 "grammar.c"
        break;
      case 45: /* edgeLength ::= MUL */
#line 273 "grammar.y"
{
	yymsp[0].minor.yy138 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1581 "grammar.c"
        break;
      case 46: /* properties ::= */
#line 279 "grammar.y"
{
	yymsp[1].minor.yy130 = NULL;
}
#line 1588 "grammar.c"
        break;
      case 47: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 283 "grammar.y"
{
	yymsp[-2].minor.yy130 = yymsp[-1].minor.yy130;
}
#line 1595 "grammar.c"
        break;
      case 48: /* mapLiteral ::= UQSTRING COLON value */
#line 289 "grammar.y"
{
	yylhsminor.yy130 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy130, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy22;
	Vector_Push(yylhsminor.yy130, val);
}
#line 1610 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 49: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 301 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy130, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy22;
	Vector_Push(yymsp[0].minor.yy130, val);
	
	yylhsminor.yy130 = yymsp[0].minor.yy130;
}
#line 1626 "grammar.c"
  yymsp[-4].minor.yy130 = yylhsminor.yy130;
        break;
      case 50: /* whereClause ::= */
#line 315 "grammar.y"
{ 
	yymsp[1].minor.yy83 = NULL;
}
#line 1634 "grammar.c"
        break;
      case 51: /* whereClause ::= WHERE cond */
#line 318 "grammar.y"
{
	yymsp[-1].minor.yy83 = New_AST_WhereNode(yymsp[0].minor.yy82);
}
#line 1641 "grammar.c"
        break;
      case 52: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 327 "grammar.y"
{ yylhsminor.yy82 = New_AST_PredicateNode(yymsp[-2].minor.yy2, yymsp[-1].minor.yy108, yymsp[0].minor.yy2); }
#line 1646 "grammar.c"
  yymsp[-2].minor.yy82 = yylhsminor.yy82;
        break;
      case 53: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 329 "grammar.y"
{ yymsp[-2].minor.yy82 = yymsp[-1].minor.yy82; }
#line 1652 "grammar.c"
        break;
      case 54: /* cond ::= cond AND cond */
#line 330 "grammar.y"
{ yylhsminor.yy82 = New_AST_ConditionNode(yymsp[-2].minor.yy82, AND, yymsp[0].minor.yy82); }
#line 1657 "grammar.c"
  yymsp[-2].minor.yy82 = yylhsminor.yy82;
        break;
      case 55: /* cond ::= cond OR cond */
#line 331 "grammar.y"
{ yylhsminor.yy82 = New_AST_ConditionNode(yymsp[-2].minor.yy82, OR, yymsp[0].minor.yy82); }
#line 1663 "grammar.c"
  yymsp[-2].minor.yy82 = yylhsminor.yy82;
        break;
      case 56: /* returnClause ::= RETURN returnElements */
#line 335 "grammar.y"
{
	yymsp[-1].minor.yy48 = New_AST_ReturnNode(yymsp[0].minor.yy130, 0);
}
#line 1671 "grammar.c"
        break;
      case 57: /* returnClause ::= RETURN DISTINCT returnElements */
#line 338 "grammar.y"
{
	yymsp[-2].minor.yy48 = New_AST_ReturnNode(yymsp[0].minor.yy130, 1);
}
#line 1678 "grammar.c"
        break;
      case 58: /* returnElements ::= returnElements COMMA returnElement */
#line 344 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy170);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1686 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 59: /* returnElements ::= returnElement */
#line 349 "grammar.y"
{
	yylhsminor.yy130 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy170);
}
#line 1695 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 60: /* returnElement ::= MUL */
#line 357 "grammar.y"
{
	yymsp[0].minor.yy170 = New_AST_ReturnElementExpandALL();
}
#line 1703 "grammar.c"
        break;
      case 61: /* returnElement ::= arithmetic_expression */
#line 360 "grammar.y"
{
	yylhsminor.yy170 = New_AST_ReturnElementNode(yymsp[0].minor.yy2, NULL);
}
#line 1710 "grammar.c"
  yymsp[0].minor.yy170 = yylhsminor.yy170;
        break;
      case 62: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 364 "grammar.y"
{
	yylhsminor.yy170 = New_AST_ReturnElementNode(yymsp[-2].minor.yy2, yymsp[0].minor.yy0.strval);
}
#line 1718 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 63: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 371 "grammar.y"
{
	yymsp[-2].minor.yy2 = yymsp[-1].minor.yy2;
}
#line 1726 "grammar.c"
        break;
      case 64: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 383 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1736 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 65: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 390 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1747 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 66: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 397 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1758 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 67: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 404 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1769 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 68: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 412 "grammar.y"
{
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy130);
}
#line 1777 "grammar.c"
  yymsp[-3].minor.yy2 = yylhsminor.yy2;
        break;
      case 69: /* arithmetic_expression ::= value */
#line 417 "grammar.y"
{
	yylhsminor.yy2 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy22);
}
#line 1785 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 70: /* arithmetic_expression ::= variable */
#line 422 "grammar.y"
{
	yylhsminor.yy2 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy116->alias, yymsp[0].minor.yy116->property);
	free(yymsp[0].minor.yy116);
}
#line 1794 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 71: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 429 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy2);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1803 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 72: /* arithmetic_expression_list ::= arithmetic_expression */
#line 433 "grammar.y"
{
	yylhsminor.yy130 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy2);
}
#line 1812 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 73: /* variable ::= UQSTRING */
#line 440 "grammar.y"
{
	yylhsminor.yy116 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1820 "grammar.c"
  yymsp[0].minor.yy116 = yylhsminor.yy116;
        break;
      case 74: /* variable ::= UQSTRING DOT UQSTRING */
#line 444 "grammar.y"
{
	yylhsminor.yy116 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1828 "grammar.c"
  yymsp[-2].minor.yy116 = yylhsminor.yy116;
        break;
      case 75: /* orderClause ::= */
#line 450 "grammar.y"
{
	yymsp[1].minor.yy52 = NULL;
}
#line 1836 "grammar.c"
        break;
      case 76: /* orderClause ::= ORDER BY columnNameList */
#line 453 "grammar.y"
{
	yymsp[-2].minor.yy52 = New_AST_OrderNode(yymsp[0].minor.yy130, ORDER_DIR_ASC);
}
#line 1843 "grammar.c"
        break;
      case 77: /* orderClause ::= ORDER BY columnNameList ASC */
#line 456 "grammar.y"
{
	yymsp[-3].minor.yy52 = New_AST_OrderNode(yymsp[-1].minor.yy130, ORDER_DIR_ASC);
}
#line 1850 "grammar.c"
        break;
      case 78: /* orderClause ::= ORDER BY columnNameList DESC */
#line 459 "grammar.y"
{
	yymsp[-3].minor.yy52 = New_AST_OrderNode(yymsp[-1].minor.yy130, ORDER_DIR_DESC);
}
#line 1857 "grammar.c"
        break;
      case 79: /* columnNameList ::= columnNameList COMMA columnName */
#line 464 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy130, yymsp[0].minor.yy54);
	yylhsminor.yy130 = yymsp[-2].minor.yy130;
}
#line 1865 "grammar.c"
  yymsp[-2].minor.yy130 = yylhsminor.yy130;
        break;
      case 80: /* columnNameList ::= columnName */
#line 468 "grammar.y"
{
	yylhsminor.yy130 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy130, yymsp[0].minor.yy54);
}
#line 1874 "grammar.c"
  yymsp[0].minor.yy130 = yylhsminor.yy130;
        break;
      case 81: /* columnName ::= variable */
#line 474 "grammar.y"
{
	if(yymsp[0].minor.yy116->property != NULL) {
		yylhsminor.yy54 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy116);
	} else {
		yylhsminor.yy54 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy116->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy116);
}
#line 1888 "grammar.c"
  yymsp[0].minor.yy54 = yylhsminor.yy54;
        break;
      case 82: /* skipClause ::= */
#line 486 "grammar.y"
{
	yymsp[1].minor.yy115 = NULL;
}
#line 1896 "grammar.c"
        break;
      case 83: /* skipClause ::= SKIP INTEGER */
#line 489 "grammar.y"
{
	yymsp[-1].minor.yy115 = New_AST_SkipNode(yymsp[0].minor.yy0.intval);
}
#line 1903 "grammar.c"
        break;
      case 84: /* limitClause ::= */
#line 495 "grammar.y"
{
	yymsp[1].minor.yy127 = NULL;
}
#line 1910 "grammar.c"
        break;
      case 85: /* limitClause ::= LIMIT INTEGER */
#line 498 "grammar.y"
{
	yymsp[-1].minor.yy127 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1917 "grammar.c"
        break;
      case 86: /* relation ::= EQ */
#line 504 "grammar.y"
{ yymsp[0].minor.yy108 = EQ; }
#line 1922 "grammar.c"
        break;
      case 87: /* relation ::= GT */
#line 505 "grammar.y"
{ yymsp[0].minor.yy108 = GT; }
#line 1927 "grammar.c"
        break;
      case 88: /* relation ::= LT */
#line 506 "grammar.y"
{ yymsp[0].minor.yy108 = LT; }
#line 1932 "grammar.c"
        break;
      case 89: /* relation ::= LE */
#line 507 "grammar.y"
{ yymsp[0].minor.yy108 = LE; }
#line 1937 "grammar.c"
        break;
      case 90: /* relation ::= GE */
#line 508 "grammar.y"
{ yymsp[0].minor.yy108 = GE; }
#line 1942 "grammar.c"
        break;
      case 91: /* relation ::= NE */
#line 509 "grammar.y"
{ yymsp[0].minor.yy108 = NE; }
#line 1947 "grammar.c"
        break;
      case 92: /* value ::= INTEGER */
#line 520 "grammar.y"
{  yylhsminor.yy22 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1952 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 93: /* value ::= DASH INTEGER */
#line 521 "grammar.y"
{  yymsp[-1].minor.yy22 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1958 "grammar.c"
        break;
      case 94: /* value ::= STRING */
#line 522 "grammar.y"
{  yylhsminor.yy22 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1963 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 95: /* value ::= FLOAT */
#line 523 "grammar.y"
{  yylhsminor.yy22 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1969 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 96: /* value ::= DASH FLOAT */
#line 524 "grammar.y"
{  yymsp[-1].minor.yy22 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1975 "grammar.c"
        break;
      case 97: /* value ::= TRUE */
#line 525 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(1); }
#line 1980 "grammar.c"
        break;
      case 98: /* value ::= FALSE */
#line 526 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(0); }
#line 1985 "grammar.c"
        break;
      case 99: /* value ::= NULLVAL */
#line 527 "grammar.y"
{ yymsp[0].minor.yy22 = SI_NullVal(); }
#line 1990 "grammar.c"
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
#line 2055 "grammar.c"
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
#line 529 "grammar.y"


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
#line 2303 "grammar.c"
