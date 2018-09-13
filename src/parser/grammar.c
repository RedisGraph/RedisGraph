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
	#include <math.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "./clauses/clauses.h"
	#include "parse.h"
	#include "../value.h"

	void yyerror(char *s);
#line 42 "grammar.c"
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
#define YYNOCODE 88
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_MatchNode* yy11;
  AST_Query* yy16;
  AST_SetNode* yy20;
  AST_CreateNode* yy28;
  AST_NodeEntity* yy33;
  int yy46;
  AST_SetElement* yy54;
  AST_ArithmeticExpressionNode* yy64;
  AST_Variable* yy66;
  AST_SkipNode* yy81;
  AST_OrderNode* yy82;
  AST_IndexOpType yy83;
  AST_LinkEntity* yy87;
  AST_LinkLength* yy96;
  Vector* yy102;
  AST_ColumnNode* yy106;
  AST_ReturnElementNode* yy108;
  AST_LimitNode* yy117;
  AST_DeleteNode * yy119;
  AST_IndexNode* yy132;
  SIValue yy144;
  AST_WhereNode* yy153;
  AST_FilterNode* yy154;
  AST_ReturnNode* yy156;
  AST_MergeNode* yy164;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             120
#define YYNRULE              99
#define YYNTOKEN             49
#define YY_MAX_SHIFT         119
#define YY_MIN_SHIFTREDUCE   185
#define YY_MAX_SHIFTREDUCE   283
#define YY_ERROR_ACTION      284
#define YY_ACCEPT_ACTION     285
#define YY_NO_ACTION         286
#define YY_MIN_REDUCE        287
#define YY_MAX_REDUCE        385
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
#define YY_ACTTAB_COUNT (291)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    88,  307,   87,   16,   14,   13,   12,  270,  271,  274,
 /*    10 */   272,  273,    9,   74,   16,   14,   13,   12,  308,   87,
 /*    20 */   247,  119,  285,   63,   62,  293,  296,   76,   15,  290,
 /*    30 */    58,  247,  294,  295,  367,   82,  356,   65,  276,   16,
 /*    40 */    14,   13,   12,  275,    2,  355,   32,   38,  113,  346,
 /*    50 */   365,  310,  310,  278,  279,  281,  282,  283,   16,   14,
 /*    60 */    13,   12,  270,  271,  274,  272,  273,   74,   31,   19,
 /*    70 */    18,   49,   74,  200,  356,   28,   96,   74,   23,   21,
 /*    80 */   115,   76,   15,  355,   84,  277,   76,    4,   20,   45,
 /*    90 */    41,    1,  276,   42,  356,   27,  110,  276,  275,  105,
 /*   100 */    33,  280,  276,  355,   70,  310,   99,  278,  279,  281,
 /*   110 */   282,  283,  278,  279,  281,  282,  283,  278,  279,  281,
 /*   120 */   282,  283,  101,   83,  356,   65,   54,   33,   16,   14,
 /*   130 */    13,   12,  310,  355,  356,   28,  111,  346,   34,   36,
 /*   140 */   356,   28,   43,  355,  341,  356,   69,   96,   80,  355,
 /*   150 */    72,   35,   52,  109,  355,  356,   65,  356,   66,   75,
 /*   160 */    48,  112,  356,   67,  355,   52,  355,   52,  345,  356,
 /*   170 */    68,  355,    7,  356,  353,  356,  352,   46,  355,  356,
 /*   180 */    77,  367,  355,  114,  355,  356,   64,   26,  355,  356,
 /*   190 */    73,  103,   37,   98,  355,  104,   71,  366,  355,    3,
 /*   200 */     5,    3,    5,  261,  262,   52,   13,   12,  252,  300,
 /*   210 */    78,   11,   39,  213,   86,   89,   30,   52,  238,   90,
 /*   220 */    91,   96,  106,   44,   92,   97,  336,  102,   47,  311,
 /*   230 */   100,  108,  118,  107,  292,  117,   55,   56,   57,  116,
 /*   240 */     1,   60,  288,    6,   59,   17,   61,  201,  202,   79,
 /*   250 */    40,   81,    5,   25,  214,   85,   10,   24,  220,  232,
 /*   260 */    22,  226,  287,  286,  286,  218,  223,  225,  219,  228,
 /*   270 */    50,   93,   94,   95,  224,  222,  221,  216,  217,   51,
 /*   280 */   215,  114,    8,   29,  267,   53,  246,  258,  286,  286,
 /*   290 */   269,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    68,   69,   70,    3,    4,    5,    6,    7,    8,    9,
 /*    10 */    10,   11,   81,    4,    3,    4,    5,    6,   69,   70,
 /*    20 */    20,   50,   51,   52,   54,   54,   55,   18,   19,   59,
 /*    30 */    60,   20,   61,   62,   70,   64,   70,   71,   29,    3,
 /*    40 */     4,    5,    6,   43,   35,   79,   67,   67,   82,   83,
 /*    50 */    86,   72,   72,   44,   45,   46,   47,   48,    3,    4,
 /*    60 */     5,    6,    7,    8,    9,   10,   11,    4,   19,   12,
 /*    70 */    13,    4,    4,   16,   70,   71,    5,    4,   21,   13,
 /*    80 */    18,   18,   19,   79,   80,   29,   18,   19,   22,   18,
 /*    90 */    24,   34,   29,   26,   70,   71,   63,   29,   43,   79,
 /*   100 */    67,   45,   29,   79,   80,   72,   75,   44,   45,   46,
 /*   110 */    47,   48,   44,   45,   46,   47,   48,   44,   45,   46,
 /*   120 */    47,   48,   75,   63,   70,   71,   73,   67,    3,    4,
 /*   130 */     5,    6,   72,   79,   70,   71,   82,   83,   17,   18,
 /*   140 */    70,   71,   77,   79,   80,   70,   71,    5,   19,   79,
 /*   150 */    80,   17,   31,   17,   79,   70,   71,   70,   71,   84,
 /*   160 */    18,   36,   70,   71,   79,   31,   79,   31,   83,   70,
 /*   170 */    71,   79,   19,   70,   71,   70,   71,   77,   79,   70,
 /*   180 */    71,   70,   79,   30,   79,   70,   71,   23,   79,   70,
 /*   190 */    71,   17,   18,   75,   79,   75,   85,   86,   79,    1,
 /*   200 */     2,    1,    2,   39,   40,   31,    5,    6,   20,   66,
 /*   210 */    17,   23,   65,   18,   74,   76,   27,   31,   20,   75,
 /*   220 */    75,    5,   18,   77,   75,   75,   78,   75,   77,   72,
 /*   230 */    76,   75,   42,   78,   58,   41,   57,   56,   55,   37,
 /*   240 */    34,   56,   58,   33,   57,   53,   55,   18,   20,   18,
 /*   250 */    15,   14,    2,   23,   18,   23,    7,   23,    4,   32,
 /*   260 */    38,   28,    0,   87,   87,   20,   28,   28,   25,   29,
 /*   270 */    18,   30,   30,   29,   28,   28,   28,   20,   20,   23,
 /*   280 */    20,   30,   23,   17,   29,   18,   18,   18,   87,   87,
 /*   290 */    29,   87,   87,   87,   87,   87,   87,   87,   87,   87,
 /*   300 */    87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
 /*   310 */    87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
 /*   320 */    87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
 /*   330 */    87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
};
#define YY_SHIFT_COUNT    (119)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (269)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    57,    9,   63,   68,   68,   68,   68,   63,   63,   63,
 /*    10 */    63,   63,   63,   63,   63,   63,   63,   66,   49,   49,
 /*    20 */    62,   49,   62,   49,   62,   49,   62,    0,   55,   73,
 /*    30 */   121,  174,   67,   67,   71,  142,  134,  136,   67,  129,
 /*    40 */   193,  195,  189,  186,  186,  216,  186,  186,  216,  189,
 /*    50 */   186,  204,  204,  186,   49,  190,  194,  202,  206,  190,
 /*    60 */   194,  202,  206,  210,   11,  125,   36,   36,   36,   36,
 /*    70 */   198,  164,  200,  201,   56,  188,  153,  201,  229,  228,
 /*    80 */   231,  235,  237,  230,  250,  236,  232,  249,  234,  254,
 /*    90 */   233,  238,  239,  240,  241,  242,  244,  246,  247,  248,
 /*   100 */   243,  245,  257,  252,  258,  256,  266,  227,  260,  267,
 /*   110 */   230,  259,  268,  259,  269,  251,  222,  255,  261,  262,
};
#define YY_REDUCE_COUNT (63)
#define YY_REDUCE_MIN   (-69)
#define YY_REDUCE_MAX   (192)
static const short yy_reduce_ofst[] = {
 /*     0 */   -29,  -34,   54,    4,   24,   64,   70,   75,   85,   87,
 /*    10 */    92,   99,  103,  105,  109,  115,  119,  -30,   33,   60,
 /*    20 */   -68,   33,  111,  -21,  -51,  -20,  -36,  -69,  -69,   20,
 /*    30 */    31,   47,   53,   53,   65,  100,  118,  120,   53,  143,
 /*    40 */   147,  140,  139,  144,  145,  146,  149,  150,  151,  154,
 /*    50 */   152,  148,  155,  156,  157,  176,  179,  181,  183,  184,
 /*    60 */   187,  185,  191,  192,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   298,  284,  284,  284,  284,  284,  284,  284,  284,  284,
 /*    10 */   284,  284,  284,  284,  284,  284,  284,  298,  301,  284,
 /*    20 */   284,  284,  284,  284,  284,  284,  284,  284,  284,  284,
 /*    30 */   333,  333,  305,  312,  329,  329,  333,  333,  313,  284,
 /*    40 */   284,  284,  284,  333,  333,  329,  333,  333,  329,  284,
 /*    50 */   333,  284,  284,  333,  284,  370,  368,  361,  291,  370,
 /*    60 */   368,  361,  289,  337,  284,  347,  339,  309,  357,  358,
 /*    70 */   284,  362,  338,  350,  284,  284,  359,  351,  284,  284,
 /*    80 */   284,  284,  284,  297,  342,  284,  314,  284,  306,  284,
 /*    90 */   284,  284,  284,  284,  284,  331,  332,  284,  284,  284,
 /*   100 */   284,  284,  284,  284,  284,  335,  284,  284,  284,  284,
 /*   110 */   299,  344,  284,  343,  284,  359,  284,  284,  284,  284,
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
  /*   30 */ "DOT",
  /*   31 */ "LEFT_CURLY_BRACKET",
  /*   32 */ "RIGHT_CURLY_BRACKET",
  /*   33 */ "WHERE",
  /*   34 */ "RETURN",
  /*   35 */ "DISTINCT",
  /*   36 */ "AS",
  /*   37 */ "ORDER",
  /*   38 */ "BY",
  /*   39 */ "ASC",
  /*   40 */ "DESC",
  /*   41 */ "SKIP",
  /*   42 */ "LIMIT",
  /*   43 */ "NE",
  /*   44 */ "STRING",
  /*   45 */ "FLOAT",
  /*   46 */ "TRUE",
  /*   47 */ "FALSE",
  /*   48 */ "NULLVAL",
  /*   49 */ "error",
  /*   50 */ "expr",
  /*   51 */ "query",
  /*   52 */ "matchClause",
  /*   53 */ "whereClause",
  /*   54 */ "createClause",
  /*   55 */ "returnClause",
  /*   56 */ "orderClause",
  /*   57 */ "skipClause",
  /*   58 */ "limitClause",
  /*   59 */ "deleteClause",
  /*   60 */ "setClause",
  /*   61 */ "indexClause",
  /*   62 */ "mergeClause",
  /*   63 */ "chains",
  /*   64 */ "indexOpToken",
  /*   65 */ "indexLabel",
  /*   66 */ "indexProp",
  /*   67 */ "chain",
  /*   68 */ "setList",
  /*   69 */ "setElement",
  /*   70 */ "variable",
  /*   71 */ "arithmetic_expression",
  /*   72 */ "node",
  /*   73 */ "link",
  /*   74 */ "deleteExpression",
  /*   75 */ "properties",
  /*   76 */ "edge",
  /*   77 */ "edgeLength",
  /*   78 */ "mapLiteral",
  /*   79 */ "value",
  /*   80 */ "cond",
  /*   81 */ "relation",
  /*   82 */ "returnElements",
  /*   83 */ "returnElement",
  /*   84 */ "arithmetic_expression_list",
  /*   85 */ "columnNameList",
  /*   86 */ "columnName",
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
 /*  36 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  37 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  38 */ "edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET",
 /*  39 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING edgeLength properties RIGHT_BRACKET",
 /*  40 */ "edge ::= LEFT_BRACKET UQSTRING COLON edgeLength properties RIGHT_BRACKET",
 /*  41 */ "edge ::= LEFT_BRACKET COLON edgeLength properties RIGHT_BRACKET",
 /*  42 */ "edgeLength ::=",
 /*  43 */ "edgeLength ::= MUL INTEGER DOT DOT INTEGER",
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
 /*  60 */ "returnElement ::= arithmetic_expression",
 /*  61 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  62 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  63 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  64 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  65 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  66 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  67 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  68 */ "arithmetic_expression ::= value",
 /*  69 */ "arithmetic_expression ::= variable",
 /*  70 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  71 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  72 */ "variable ::= UQSTRING",
 /*  73 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  74 */ "orderClause ::=",
 /*  75 */ "orderClause ::= ORDER BY columnNameList",
 /*  76 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  77 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  78 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  79 */ "columnNameList ::= columnName",
 /*  80 */ "columnName ::= variable",
 /*  81 */ "skipClause ::=",
 /*  82 */ "skipClause ::= SKIP INTEGER",
 /*  83 */ "limitClause ::=",
 /*  84 */ "limitClause ::= LIMIT INTEGER",
 /*  85 */ "relation ::= EQ",
 /*  86 */ "relation ::= GT",
 /*  87 */ "relation ::= LT",
 /*  88 */ "relation ::= LE",
 /*  89 */ "relation ::= GE",
 /*  90 */ "relation ::= NE",
 /*  91 */ "value ::= INTEGER",
 /*  92 */ "value ::= DASH INTEGER",
 /*  93 */ "value ::= STRING",
 /*  94 */ "value ::= FLOAT",
 /*  95 */ "value ::= DASH FLOAT",
 /*  96 */ "value ::= TRUE",
 /*  97 */ "value ::= FALSE",
 /*  98 */ "value ::= NULLVAL",
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
    case 80: /* cond */
{
#line 317 "grammar.y"
 Free_AST_FilterNode((yypminor->yy154)); 
#line 752 "grammar.c"
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
  {   51,   -1 }, /* (0) query ::= expr */
  {   50,   -7 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
  {   50,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   50,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   50,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   50,   -7 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   50,   -1 }, /* (6) expr ::= createClause */
  {   50,   -1 }, /* (7) expr ::= indexClause */
  {   50,   -1 }, /* (8) expr ::= mergeClause */
  {   50,   -1 }, /* (9) expr ::= returnClause */
  {   52,   -2 }, /* (10) matchClause ::= MATCH chains */
  {   54,    0 }, /* (11) createClause ::= */
  {   54,   -2 }, /* (12) createClause ::= CREATE chains */
  {   61,   -5 }, /* (13) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   64,   -1 }, /* (14) indexOpToken ::= CREATE */
  {   64,   -1 }, /* (15) indexOpToken ::= DROP */
  {   65,   -2 }, /* (16) indexLabel ::= COLON UQSTRING */
  {   66,   -3 }, /* (17) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   62,   -2 }, /* (18) mergeClause ::= MERGE chain */
  {   60,   -2 }, /* (19) setClause ::= SET setList */
  {   68,   -1 }, /* (20) setList ::= setElement */
  {   68,   -3 }, /* (21) setList ::= setList COMMA setElement */
  {   69,   -3 }, /* (22) setElement ::= variable EQ arithmetic_expression */
  {   67,   -1 }, /* (23) chain ::= node */
  {   67,   -3 }, /* (24) chain ::= chain link node */
  {   63,   -1 }, /* (25) chains ::= chain */
  {   63,   -3 }, /* (26) chains ::= chains COMMA chain */
  {   59,   -2 }, /* (27) deleteClause ::= DELETE deleteExpression */
  {   74,   -1 }, /* (28) deleteExpression ::= UQSTRING */
  {   74,   -3 }, /* (29) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   72,   -6 }, /* (30) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   72,   -5 }, /* (31) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   72,   -4 }, /* (32) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   72,   -3 }, /* (33) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   73,   -3 }, /* (34) link ::= DASH edge RIGHT_ARROW */
  {   73,   -3 }, /* (35) link ::= LEFT_ARROW edge DASH */
  {   76,   -3 }, /* (36) edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
  {   76,   -4 }, /* (37) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   76,   -6 }, /* (38) edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
  {   76,   -7 }, /* (39) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING edgeLength properties RIGHT_BRACKET */
  {   76,   -6 }, /* (40) edge ::= LEFT_BRACKET UQSTRING COLON edgeLength properties RIGHT_BRACKET */
  {   76,   -5 }, /* (41) edge ::= LEFT_BRACKET COLON edgeLength properties RIGHT_BRACKET */
  {   77,    0 }, /* (42) edgeLength ::= */
  {   77,   -5 }, /* (43) edgeLength ::= MUL INTEGER DOT DOT INTEGER */
  {   77,   -2 }, /* (44) edgeLength ::= MUL INTEGER */
  {   77,   -1 }, /* (45) edgeLength ::= MUL */
  {   75,    0 }, /* (46) properties ::= */
  {   75,   -3 }, /* (47) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   78,   -3 }, /* (48) mapLiteral ::= UQSTRING COLON value */
  {   78,   -5 }, /* (49) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   53,    0 }, /* (50) whereClause ::= */
  {   53,   -2 }, /* (51) whereClause ::= WHERE cond */
  {   80,   -3 }, /* (52) cond ::= arithmetic_expression relation arithmetic_expression */
  {   80,   -3 }, /* (53) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   80,   -3 }, /* (54) cond ::= cond AND cond */
  {   80,   -3 }, /* (55) cond ::= cond OR cond */
  {   55,   -2 }, /* (56) returnClause ::= RETURN returnElements */
  {   55,   -3 }, /* (57) returnClause ::= RETURN DISTINCT returnElements */
  {   82,   -3 }, /* (58) returnElements ::= returnElements COMMA returnElement */
  {   82,   -1 }, /* (59) returnElements ::= returnElement */
  {   83,   -1 }, /* (60) returnElement ::= arithmetic_expression */
  {   83,   -3 }, /* (61) returnElement ::= arithmetic_expression AS UQSTRING */
  {   71,   -3 }, /* (62) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   71,   -3 }, /* (63) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   71,   -3 }, /* (64) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   71,   -3 }, /* (65) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   71,   -3 }, /* (66) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   71,   -4 }, /* (67) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   71,   -1 }, /* (68) arithmetic_expression ::= value */
  {   71,   -1 }, /* (69) arithmetic_expression ::= variable */
  {   84,   -3 }, /* (70) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   84,   -1 }, /* (71) arithmetic_expression_list ::= arithmetic_expression */
  {   70,   -1 }, /* (72) variable ::= UQSTRING */
  {   70,   -3 }, /* (73) variable ::= UQSTRING DOT UQSTRING */
  {   56,    0 }, /* (74) orderClause ::= */
  {   56,   -3 }, /* (75) orderClause ::= ORDER BY columnNameList */
  {   56,   -4 }, /* (76) orderClause ::= ORDER BY columnNameList ASC */
  {   56,   -4 }, /* (77) orderClause ::= ORDER BY columnNameList DESC */
  {   85,   -3 }, /* (78) columnNameList ::= columnNameList COMMA columnName */
  {   85,   -1 }, /* (79) columnNameList ::= columnName */
  {   86,   -1 }, /* (80) columnName ::= variable */
  {   57,    0 }, /* (81) skipClause ::= */
  {   57,   -2 }, /* (82) skipClause ::= SKIP INTEGER */
  {   58,    0 }, /* (83) limitClause ::= */
  {   58,   -2 }, /* (84) limitClause ::= LIMIT INTEGER */
  {   81,   -1 }, /* (85) relation ::= EQ */
  {   81,   -1 }, /* (86) relation ::= GT */
  {   81,   -1 }, /* (87) relation ::= LT */
  {   81,   -1 }, /* (88) relation ::= LE */
  {   81,   -1 }, /* (89) relation ::= GE */
  {   81,   -1 }, /* (90) relation ::= NE */
  {   79,   -1 }, /* (91) value ::= INTEGER */
  {   79,   -2 }, /* (92) value ::= DASH INTEGER */
  {   79,   -1 }, /* (93) value ::= STRING */
  {   79,   -1 }, /* (94) value ::= FLOAT */
  {   79,   -2 }, /* (95) value ::= DASH FLOAT */
  {   79,   -1 }, /* (96) value ::= TRUE */
  {   79,   -1 }, /* (97) value ::= FALSE */
  {   79,   -1 }, /* (98) value ::= NULLVAL */
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
#line 37 "grammar.y"
{ ctx->root = yymsp[0].minor.yy16; }
#line 1228 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
#line 39 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(yymsp[-6].minor.yy11, yymsp[-5].minor.yy153, yymsp[-4].minor.yy28, NULL, NULL, NULL, yymsp[-3].minor.yy156, yymsp[-2].minor.yy82, yymsp[-1].minor.yy81, yymsp[0].minor.yy117, NULL);
}
#line 1235 "grammar.c"
  yymsp[-6].minor.yy16 = yylhsminor.yy16;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 43 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(yymsp[-2].minor.yy11, yymsp[-1].minor.yy153, yymsp[0].minor.yy28, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1243 "grammar.c"
  yymsp[-2].minor.yy16 = yylhsminor.yy16;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 47 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(yymsp[-2].minor.yy11, yymsp[-1].minor.yy153, NULL, NULL, NULL, yymsp[0].minor.yy119, NULL, NULL, NULL, NULL, NULL);
}
#line 1251 "grammar.c"
  yymsp[-2].minor.yy16 = yylhsminor.yy16;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 51 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(yymsp[-2].minor.yy11, yymsp[-1].minor.yy153, NULL, NULL, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1259 "grammar.c"
  yymsp[-2].minor.yy16 = yylhsminor.yy16;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 55 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(yymsp[-6].minor.yy11, yymsp[-5].minor.yy153, NULL, NULL, yymsp[-4].minor.yy20, NULL, yymsp[-3].minor.yy156, yymsp[-2].minor.yy82, yymsp[-1].minor.yy81, yymsp[0].minor.yy117, NULL);
}
#line 1267 "grammar.c"
  yymsp[-6].minor.yy16 = yylhsminor.yy16;
        break;
      case 6: /* expr ::= createClause */
#line 59 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy28, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1275 "grammar.c"
  yymsp[0].minor.yy16 = yylhsminor.yy16;
        break;
      case 7: /* expr ::= indexClause */
#line 63 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy132);
}
#line 1283 "grammar.c"
  yymsp[0].minor.yy16 = yylhsminor.yy16;
        break;
      case 8: /* expr ::= mergeClause */
#line 67 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy164, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1291 "grammar.c"
  yymsp[0].minor.yy16 = yylhsminor.yy16;
        break;
      case 9: /* expr ::= returnClause */
#line 71 "grammar.y"
{
	yylhsminor.yy16 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy156, NULL, NULL, NULL, NULL);
}
#line 1299 "grammar.c"
  yymsp[0].minor.yy16 = yylhsminor.yy16;
        break;
      case 10: /* matchClause ::= MATCH chains */
#line 77 "grammar.y"
{
	yymsp[-1].minor.yy11 = New_AST_MatchNode(yymsp[0].minor.yy102);
}
#line 1307 "grammar.c"
        break;
      case 11: /* createClause ::= */
#line 84 "grammar.y"
{
	yymsp[1].minor.yy28 = NULL;
}
#line 1314 "grammar.c"
        break;
      case 12: /* createClause ::= CREATE chains */
#line 88 "grammar.y"
{
	yymsp[-1].minor.yy28 = New_AST_CreateNode(yymsp[0].minor.yy102);
}
#line 1321 "grammar.c"
        break;
      case 13: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 94 "grammar.y"
{
  yylhsminor.yy132 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy83);
}
#line 1328 "grammar.c"
  yymsp[-4].minor.yy132 = yylhsminor.yy132;
        break;
      case 14: /* indexOpToken ::= CREATE */
#line 100 "grammar.y"
{ yymsp[0].minor.yy83 = CREATE_INDEX; }
#line 1334 "grammar.c"
        break;
      case 15: /* indexOpToken ::= DROP */
#line 101 "grammar.y"
{ yymsp[0].minor.yy83 = DROP_INDEX; }
#line 1339 "grammar.c"
        break;
      case 16: /* indexLabel ::= COLON UQSTRING */
#line 103 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1346 "grammar.c"
        break;
      case 17: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 107 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1353 "grammar.c"
        break;
      case 18: /* mergeClause ::= MERGE chain */
#line 113 "grammar.y"
{
	yymsp[-1].minor.yy164 = New_AST_MergeNode(yymsp[0].minor.yy102);
}
#line 1360 "grammar.c"
        break;
      case 19: /* setClause ::= SET setList */
#line 118 "grammar.y"
{
	yymsp[-1].minor.yy20 = New_AST_SetNode(yymsp[0].minor.yy102);
}
#line 1367 "grammar.c"
        break;
      case 20: /* setList ::= setElement */
#line 123 "grammar.y"
{
	yylhsminor.yy102 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy54);
}
#line 1375 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 21: /* setList ::= setList COMMA setElement */
#line 127 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy54);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1384 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 22: /* setElement ::= variable EQ arithmetic_expression */
#line 133 "grammar.y"
{
	yylhsminor.yy54 = New_AST_SetElement(yymsp[-2].minor.yy66, yymsp[0].minor.yy64);
}
#line 1392 "grammar.c"
  yymsp[-2].minor.yy54 = yylhsminor.yy54;
        break;
      case 23: /* chain ::= node */
#line 139 "grammar.y"
{
	yylhsminor.yy102 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy33);
}
#line 1401 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 24: /* chain ::= chain link node */
#line 144 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[-1].minor.yy87);
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy33);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1411 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 25: /* chains ::= chain */
#line 152 "grammar.y"
{
	yylhsminor.yy102 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy102);
}
#line 1420 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 26: /* chains ::= chains COMMA chain */
#line 157 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy102);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1429 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 27: /* deleteClause ::= DELETE deleteExpression */
#line 165 "grammar.y"
{
	yymsp[-1].minor.yy119 = New_AST_DeleteNode(yymsp[0].minor.yy102);
}
#line 1437 "grammar.c"
        break;
      case 28: /* deleteExpression ::= UQSTRING */
#line 171 "grammar.y"
{
	yylhsminor.yy102 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy0.strval);
}
#line 1445 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 29: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 176 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy0.strval);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1454 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 30: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 184 "grammar.y"
{
	yymsp[-5].minor.yy33 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy102);
}
#line 1462 "grammar.c"
        break;
      case 31: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 189 "grammar.y"
{
	yymsp[-4].minor.yy33 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy102);
}
#line 1469 "grammar.c"
        break;
      case 32: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 194 "grammar.y"
{
	yymsp[-3].minor.yy33 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy102);
}
#line 1476 "grammar.c"
        break;
      case 33: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 199 "grammar.y"
{
	yymsp[-2].minor.yy33 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy102);
}
#line 1483 "grammar.c"
        break;
      case 34: /* link ::= DASH edge RIGHT_ARROW */
#line 206 "grammar.y"
{
	yymsp[-2].minor.yy87 = yymsp[-1].minor.yy87;
	yymsp[-2].minor.yy87->direction = N_LEFT_TO_RIGHT;
}
#line 1491 "grammar.c"
        break;
      case 35: /* link ::= LEFT_ARROW edge DASH */
#line 212 "grammar.y"
{
	yymsp[-2].minor.yy87 = yymsp[-1].minor.yy87;
	yymsp[-2].minor.yy87->direction = N_RIGHT_TO_LEFT;
}
#line 1499 "grammar.c"
        break;
      case 36: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 219 "grammar.y"
{ 
	yymsp[-2].minor.yy87 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy102, N_DIR_UNKNOWN, NULL);
}
#line 1506 "grammar.c"
        break;
      case 37: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 224 "grammar.y"
{ 
	yymsp[-3].minor.yy87 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy102, N_DIR_UNKNOWN, NULL);
}
#line 1513 "grammar.c"
        break;
      case 38: /* edge ::= LEFT_BRACKET COLON UQSTRING edgeLength properties RIGHT_BRACKET */
#line 229 "grammar.y"
{ 
	yymsp[-5].minor.yy87 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy102, N_DIR_UNKNOWN, yymsp[-2].minor.yy96);
}
#line 1520 "grammar.c"
        break;
      case 39: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING edgeLength properties RIGHT_BRACKET */
#line 234 "grammar.y"
{ 
	yymsp[-6].minor.yy87 = New_AST_LinkEntity(yymsp[-5].minor.yy0.strval, yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy102, N_DIR_UNKNOWN, yymsp[-2].minor.yy96);
}
#line 1527 "grammar.c"
        break;
      case 40: /* edge ::= LEFT_BRACKET UQSTRING COLON edgeLength properties RIGHT_BRACKET */
#line 239 "grammar.y"
{ 
	yymsp[-5].minor.yy87 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, NULL, yymsp[-1].minor.yy102, N_DIR_UNKNOWN, yymsp[-2].minor.yy96);
}
#line 1534 "grammar.c"
        break;
      case 41: /* edge ::= LEFT_BRACKET COLON edgeLength properties RIGHT_BRACKET */
#line 244 "grammar.y"
{ 
	yymsp[-4].minor.yy87 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy102, N_DIR_UNKNOWN, yymsp[-2].minor.yy96);
}
#line 1541 "grammar.c"
        break;
      case 42: /* edgeLength ::= */
#line 251 "grammar.y"
{
	yymsp[1].minor.yy96 = NULL;
}
#line 1548 "grammar.c"
        break;
      case 43: /* edgeLength ::= MUL INTEGER DOT DOT INTEGER */
#line 256 "grammar.y"
{
	yymsp[-4].minor.yy96 = New_AST_LinkLength(yymsp[-3].minor.yy0.intval, yymsp[0].minor.yy0.intval);
}
#line 1555 "grammar.c"
        break;
      case 44: /* edgeLength ::= MUL INTEGER */
#line 261 "grammar.y"
{
	yymsp[-1].minor.yy96 = New_AST_LinkLength(1, yymsp[0].minor.yy0.intval);
}
#line 1562 "grammar.c"
        break;
      case 45: /* edgeLength ::= MUL */
#line 266 "grammar.y"
{
	yymsp[0].minor.yy96 = New_AST_LinkLength(1, INFINITY);
}
#line 1569 "grammar.c"
        break;
      case 46: /* properties ::= */
#line 272 "grammar.y"
{
	yymsp[1].minor.yy102 = NULL;
}
#line 1576 "grammar.c"
        break;
      case 47: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 276 "grammar.y"
{
	yymsp[-2].minor.yy102 = yymsp[-1].minor.yy102;
}
#line 1583 "grammar.c"
        break;
      case 48: /* mapLiteral ::= UQSTRING COLON value */
#line 282 "grammar.y"
{
	yylhsminor.yy102 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy102, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy144;
	Vector_Push(yylhsminor.yy102, val);
}
#line 1598 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 49: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 294 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy102, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy144;
	Vector_Push(yymsp[0].minor.yy102, val);
	
	yylhsminor.yy102 = yymsp[0].minor.yy102;
}
#line 1614 "grammar.c"
  yymsp[-4].minor.yy102 = yylhsminor.yy102;
        break;
      case 50: /* whereClause ::= */
#line 308 "grammar.y"
{ 
	yymsp[1].minor.yy153 = NULL;
}
#line 1622 "grammar.c"
        break;
      case 51: /* whereClause ::= WHERE cond */
#line 311 "grammar.y"
{
	yymsp[-1].minor.yy153 = New_AST_WhereNode(yymsp[0].minor.yy154);
}
#line 1629 "grammar.c"
        break;
      case 52: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 320 "grammar.y"
{ yylhsminor.yy154 = New_AST_PredicateNode(yymsp[-2].minor.yy64, yymsp[-1].minor.yy46, yymsp[0].minor.yy64); }
#line 1634 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 53: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 322 "grammar.y"
{ yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154; }
#line 1640 "grammar.c"
        break;
      case 54: /* cond ::= cond AND cond */
#line 323 "grammar.y"
{ yylhsminor.yy154 = New_AST_ConditionNode(yymsp[-2].minor.yy154, AND, yymsp[0].minor.yy154); }
#line 1645 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 55: /* cond ::= cond OR cond */
#line 324 "grammar.y"
{ yylhsminor.yy154 = New_AST_ConditionNode(yymsp[-2].minor.yy154, OR, yymsp[0].minor.yy154); }
#line 1651 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 56: /* returnClause ::= RETURN returnElements */
#line 328 "grammar.y"
{
	yymsp[-1].minor.yy156 = New_AST_ReturnNode(yymsp[0].minor.yy102, 0);
}
#line 1659 "grammar.c"
        break;
      case 57: /* returnClause ::= RETURN DISTINCT returnElements */
#line 331 "grammar.y"
{
	yymsp[-2].minor.yy156 = New_AST_ReturnNode(yymsp[0].minor.yy102, 1);
}
#line 1666 "grammar.c"
        break;
      case 58: /* returnElements ::= returnElements COMMA returnElement */
#line 338 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy108);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1674 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 59: /* returnElements ::= returnElement */
#line 343 "grammar.y"
{
	yylhsminor.yy102 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy108);
}
#line 1683 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 60: /* returnElement ::= arithmetic_expression */
#line 350 "grammar.y"
{
	yylhsminor.yy108 = New_AST_ReturnElementNode(yymsp[0].minor.yy64, NULL);
}
#line 1691 "grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 61: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 355 "grammar.y"
{
	yylhsminor.yy108 = New_AST_ReturnElementNode(yymsp[-2].minor.yy64, yymsp[0].minor.yy0.strval);
}
#line 1699 "grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 62: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 362 "grammar.y"
{
	yymsp[-2].minor.yy64 = yymsp[-1].minor.yy64;
}
#line 1707 "grammar.c"
        break;
      case 63: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 374 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy64);
	Vector_Push(args, yymsp[0].minor.yy64);
	yylhsminor.yy64 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1717 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 64: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 381 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy64);
	Vector_Push(args, yymsp[0].minor.yy64);
	yylhsminor.yy64 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1728 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 65: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 388 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy64);
	Vector_Push(args, yymsp[0].minor.yy64);
	yylhsminor.yy64 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1739 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 66: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 395 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy64);
	Vector_Push(args, yymsp[0].minor.yy64);
	yylhsminor.yy64 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1750 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 67: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 403 "grammar.y"
{
	yylhsminor.yy64 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy102);
}
#line 1758 "grammar.c"
  yymsp[-3].minor.yy64 = yylhsminor.yy64;
        break;
      case 68: /* arithmetic_expression ::= value */
#line 408 "grammar.y"
{
	yylhsminor.yy64 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy144);
}
#line 1766 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 69: /* arithmetic_expression ::= variable */
#line 413 "grammar.y"
{
	yylhsminor.yy64 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy66->alias, yymsp[0].minor.yy66->property);
}
#line 1774 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 70: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 419 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy64);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1783 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 71: /* arithmetic_expression_list ::= arithmetic_expression */
#line 423 "grammar.y"
{
	yylhsminor.yy102 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy64);
}
#line 1792 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 72: /* variable ::= UQSTRING */
#line 430 "grammar.y"
{
	yylhsminor.yy66 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1800 "grammar.c"
  yymsp[0].minor.yy66 = yylhsminor.yy66;
        break;
      case 73: /* variable ::= UQSTRING DOT UQSTRING */
#line 434 "grammar.y"
{
	yylhsminor.yy66 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1808 "grammar.c"
  yymsp[-2].minor.yy66 = yylhsminor.yy66;
        break;
      case 74: /* orderClause ::= */
#line 440 "grammar.y"
{
	yymsp[1].minor.yy82 = NULL;
}
#line 1816 "grammar.c"
        break;
      case 75: /* orderClause ::= ORDER BY columnNameList */
#line 443 "grammar.y"
{
	yymsp[-2].minor.yy82 = New_AST_OrderNode(yymsp[0].minor.yy102, ORDER_DIR_ASC);
}
#line 1823 "grammar.c"
        break;
      case 76: /* orderClause ::= ORDER BY columnNameList ASC */
#line 446 "grammar.y"
{
	yymsp[-3].minor.yy82 = New_AST_OrderNode(yymsp[-1].minor.yy102, ORDER_DIR_ASC);
}
#line 1830 "grammar.c"
        break;
      case 77: /* orderClause ::= ORDER BY columnNameList DESC */
#line 449 "grammar.y"
{
	yymsp[-3].minor.yy82 = New_AST_OrderNode(yymsp[-1].minor.yy102, ORDER_DIR_DESC);
}
#line 1837 "grammar.c"
        break;
      case 78: /* columnNameList ::= columnNameList COMMA columnName */
#line 454 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy102, yymsp[0].minor.yy106);
	yylhsminor.yy102 = yymsp[-2].minor.yy102;
}
#line 1845 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 79: /* columnNameList ::= columnName */
#line 458 "grammar.y"
{
	yylhsminor.yy102 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy102, yymsp[0].minor.yy106);
}
#line 1854 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 80: /* columnName ::= variable */
#line 464 "grammar.y"
{
	if(yymsp[0].minor.yy66->property != NULL) {
		yylhsminor.yy106 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy66);
	} else {
		yylhsminor.yy106 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy66->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy66);
}
#line 1868 "grammar.c"
  yymsp[0].minor.yy106 = yylhsminor.yy106;
        break;
      case 81: /* skipClause ::= */
#line 476 "grammar.y"
{
	yymsp[1].minor.yy81 = NULL;
}
#line 1876 "grammar.c"
        break;
      case 82: /* skipClause ::= SKIP INTEGER */
#line 479 "grammar.y"
{
	yymsp[-1].minor.yy81 = New_AST_SkipNode(yymsp[0].minor.yy0.intval);
}
#line 1883 "grammar.c"
        break;
      case 83: /* limitClause ::= */
#line 485 "grammar.y"
{
	yymsp[1].minor.yy117 = NULL;
}
#line 1890 "grammar.c"
        break;
      case 84: /* limitClause ::= LIMIT INTEGER */
#line 488 "grammar.y"
{
	yymsp[-1].minor.yy117 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1897 "grammar.c"
        break;
      case 85: /* relation ::= EQ */
#line 494 "grammar.y"
{ yymsp[0].minor.yy46 = EQ; }
#line 1902 "grammar.c"
        break;
      case 86: /* relation ::= GT */
#line 495 "grammar.y"
{ yymsp[0].minor.yy46 = GT; }
#line 1907 "grammar.c"
        break;
      case 87: /* relation ::= LT */
#line 496 "grammar.y"
{ yymsp[0].minor.yy46 = LT; }
#line 1912 "grammar.c"
        break;
      case 88: /* relation ::= LE */
#line 497 "grammar.y"
{ yymsp[0].minor.yy46 = LE; }
#line 1917 "grammar.c"
        break;
      case 89: /* relation ::= GE */
#line 498 "grammar.y"
{ yymsp[0].minor.yy46 = GE; }
#line 1922 "grammar.c"
        break;
      case 90: /* relation ::= NE */
#line 499 "grammar.y"
{ yymsp[0].minor.yy46 = NE; }
#line 1927 "grammar.c"
        break;
      case 91: /* value ::= INTEGER */
#line 510 "grammar.y"
{  yylhsminor.yy144 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1932 "grammar.c"
  yymsp[0].minor.yy144 = yylhsminor.yy144;
        break;
      case 92: /* value ::= DASH INTEGER */
#line 511 "grammar.y"
{  yymsp[-1].minor.yy144 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1938 "grammar.c"
        break;
      case 93: /* value ::= STRING */
#line 512 "grammar.y"
{  yylhsminor.yy144 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1943 "grammar.c"
  yymsp[0].minor.yy144 = yylhsminor.yy144;
        break;
      case 94: /* value ::= FLOAT */
#line 513 "grammar.y"
{  yylhsminor.yy144 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1949 "grammar.c"
  yymsp[0].minor.yy144 = yylhsminor.yy144;
        break;
      case 95: /* value ::= DASH FLOAT */
#line 514 "grammar.y"
{  yymsp[-1].minor.yy144 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1955 "grammar.c"
        break;
      case 96: /* value ::= TRUE */
#line 515 "grammar.y"
{ yymsp[0].minor.yy144 = SI_BoolVal(1); }
#line 1960 "grammar.c"
        break;
      case 97: /* value ::= FALSE */
#line 516 "grammar.y"
{ yymsp[0].minor.yy144 = SI_BoolVal(0); }
#line 1965 "grammar.c"
        break;
      case 98: /* value ::= NULLVAL */
#line 517 "grammar.y"
{ yymsp[0].minor.yy144 = SI_NullVal(); }
#line 1970 "grammar.c"
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
#line 24 "grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'\n", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 2035 "grammar.c"
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
#line 519 "grammar.y"


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
#line 2281 "grammar.c"
