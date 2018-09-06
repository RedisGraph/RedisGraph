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
#define YYNOCODE 87
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_Query* yy4;
  AST_DeleteNode * yy7;
  AST_ReturnNode* yy12;
  AST_CreateNode* yy16;
  AST_ArithmeticExpressionNode* yy22;
  SIValue yy23;
  AST_LinkEntity* yy37;
  AST_MatchNode* yy45;
  AST_ReturnElementNode* yy62;
  int yy64;
  AST_SetNode* yy76;
  AST_Variable* yy80;
  AST_SetElement* yy92;
  AST_NodeEntity* yy101;
  AST_MergeNode* yy108;
  Vector* yy110;
  AST_IndexNode* yy116;
  AST_WhereNode* yy119;
  AST_SkipNode* yy120;
  AST_LimitNode* yy121;
  AST_ColumnNode* yy138;
  AST_IndexOpType yy149;
  AST_FilterNode* yy150;
  AST_OrderNode* yy168;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             110
#define YYNRULE              93
#define YYNTOKEN             49
#define YY_MAX_SHIFT         109
#define YY_MIN_SHIFTREDUCE   172
#define YY_MAX_SHIFTREDUCE   264
#define YY_ERROR_ACTION      265
#define YY_ACCEPT_ACTION     266
#define YY_NO_ACTION         267
#define YY_MIN_REDUCE        268
#define YY_MAX_REDUCE        360
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
#define YY_ACTTAB_COUNT (292)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    82,  288,   81,   16,   14,   13,   12,  251,  252,  255,
 /*    10 */   253,  254,   68,   56,   16,   14,   13,   12,  271,   52,
 /*    20 */   228,   16,   14,   13,   12,   68,   70,   15,   13,   12,
 /*    30 */    16,   14,   13,   12,  251,  252,  255,  253,  254,   70,
 /*    40 */    15,    2,  100,  256,   68,  102,   33,  289,   81,  257,
 /*    50 */    31,  291,  259,  260,  262,  263,  264,   68,   70,    4,
 /*    60 */    85,   34,  257,  331,   59,  259,  260,  262,  263,  264,
 /*    70 */   256,  330,   46,   95,  103,  321,    3,    5,  109,  266,
 /*    80 */    57,  257,  274,  277,  259,  260,  262,  263,  264,  275,
 /*    90 */   276,   32,   76,    9,  257,  219,  291,  259,  260,  262,
 /*   100 */   263,  264,   19,   18,  331,   59,  187,   16,   14,   13,
 /*   110 */    12,   23,  330,   36,  105,  101,  321,  342,  291,   89,
 /*   120 */   331,   28,    1,   43,  228,  331,   27,   91,  330,   78,
 /*   130 */   331,   28,  340,  330,   64,  331,   28,   48,  330,  316,
 /*   140 */   331,   63,   88,  330,   66,   40,  331,   59,  330,  331,
 /*   150 */    60,  331,   61,   69,  330,  331,   62,  330,  320,  330,
 /*   160 */    74,  331,  328,  330,   94,  331,  327,    3,    5,  330,
 /*   170 */   331,   71,   87,  330,  281,  331,   58,   21,  330,  331,
 /*   180 */    67,  342,   26,  330,   46,   77,   20,  330,   39,   33,
 /*   190 */    99,   93,   35,   72,  291,   65,  341,  242,  243,    7,
 /*   200 */    37,  258,   46,   46,  233,  261,  200,   11,   80,   30,
 /*   210 */    46,   96,   83,   84,   86,  104,   90,   92,  292,  108,
 /*   220 */   311,   97,   98,  273,   49,  107,  106,   50,    1,   53,
 /*   230 */    51,    6,  188,  189,   73,   54,  269,   38,   55,   17,
 /*   240 */    75,   25,    5,  201,   79,   10,   24,  207,  210,   41,
 /*   250 */    42,   29,  211,  205,  206,  209,  208,  203,   45,   44,
 /*   260 */   213,  268,  204,   47,  202,  227,  239,  267,  267,    8,
 /*   270 */   267,  104,  267,  267,  267,  267,   22,  267,  267,  267,
 /*   280 */   267,  267,  267,  267,  267,  267,  267,  267,  267,  267,
 /*   290 */   248,  250,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    68,   69,   70,    3,    4,    5,    6,    7,    8,    9,
 /*    10 */    10,   11,    4,   54,    3,    4,    5,    6,   59,   60,
 /*    20 */    20,    3,    4,    5,    6,    4,   18,   19,    5,    6,
 /*    30 */     3,    4,    5,    6,    7,    8,    9,   10,   11,   18,
 /*    40 */    19,   33,   63,   43,    4,   34,   67,   69,   70,   41,
 /*    50 */    19,   72,   44,   45,   46,   47,   48,    4,   18,   19,
 /*    60 */    17,   18,   41,   70,   71,   44,   45,   46,   47,   48,
 /*    70 */    43,   78,   29,   78,   81,   82,    1,    2,   50,   51,
 /*    80 */    52,   41,   54,   55,   44,   45,   46,   47,   48,   61,
 /*    90 */    62,   67,   64,   80,   41,   20,   72,   44,   45,   46,
 /*   100 */    47,   48,   12,   13,   70,   71,   16,    3,    4,    5,
 /*   110 */     6,   21,   78,   67,   18,   81,   82,   70,   72,   75,
 /*   120 */    70,   71,   32,    4,   20,   70,   71,   75,   78,   79,
 /*   130 */    70,   71,   85,   78,   79,   70,   71,   73,   78,   79,
 /*   140 */    70,   71,   75,   78,   79,   26,   70,   71,   78,   70,
 /*   150 */    71,   70,   71,   83,   78,   70,   71,   78,   82,   78,
 /*   160 */    19,   70,   71,   78,   75,   70,   71,    1,    2,   78,
 /*   170 */    70,   71,   17,   78,   66,   70,   71,   13,   78,   70,
 /*   180 */    71,   70,   23,   78,   29,   63,   22,   78,   24,   67,
 /*   190 */    17,   17,   18,   17,   72,   84,   85,   38,   39,   19,
 /*   200 */    65,   41,   29,   29,   20,   45,   18,   23,   74,   27,
 /*   210 */    29,   18,   76,   75,   75,   35,   76,   75,   72,   42,
 /*   220 */    77,   77,   75,   58,   57,   40,   36,   56,   32,   57,
 /*   230 */    55,   31,   18,   20,   18,   56,   58,   15,   55,   53,
 /*   240 */    14,   23,    2,   18,   23,    7,   23,    4,   28,   18,
 /*   250 */    18,   17,   28,   20,   25,   28,   28,   20,   23,   18,
 /*   260 */    30,    0,   20,   18,   20,   18,   18,   86,   86,   23,
 /*   270 */    86,   35,   86,   86,   86,   86,   37,   86,   86,   86,
 /*   280 */    86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
 /*   290 */    41,   41,   86,   86,   86,   86,   86,   86,   86,   86,
 /*   300 */    86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
 /*   310 */    86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
 /*   320 */    86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
 /*   330 */    86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
 /*   340 */    86,
};
#define YY_SHIFT_COUNT    (109)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (261)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    90,    8,   21,   40,   40,   40,   40,   21,   21,   21,
 /*    10 */    21,   21,   21,   21,   21,   21,   21,  164,   31,   31,
 /*    20 */    96,   31,   96,   31,   96,   31,   96,    0,   27,   53,
 /*    30 */    43,  174,  119,  119,  155,  173,  119,  141,  176,  188,
 /*    40 */   182,  181,  181,  182,  181,  193,  193,  181,   31,  177,
 /*    50 */   185,  190,  196,  177,  185,  190,  196,  200,  104,   11,
 /*    60 */    18,   18,   18,   18,   75,  159,  166,   23,  160,  184,
 /*    70 */   180,   23,  214,  213,  216,  222,  226,  218,  240,  225,
 /*    80 */   221,  238,  223,  243,  220,  231,  224,  232,  227,  228,
 /*    90 */   229,  233,  237,  241,  242,  235,  234,  230,  244,  245,
 /*   100 */   218,  246,  247,  246,  248,  236,  239,  249,  250,  261,
};
#define YY_REDUCE_COUNT (57)
#define YY_REDUCE_MIN   (-68)
#define YY_REDUCE_MAX   (186)
static const short yy_reduce_ofst[] = {
 /*     0 */    28,   -7,   34,   50,   55,   60,   65,   70,   76,   79,
 /*    10 */    81,   85,   91,   95,  100,  105,  109,  -41,  -21,  122,
 /*    20 */   -68,  -21,  111,   24,  -22,   46,   47,   13,   13,   -5,
 /*    30 */    44,   52,   64,   64,   67,   89,   64,  108,  135,  134,
 /*    40 */   136,  138,  139,  140,  142,  143,  144,  147,  146,  165,
 /*    50 */   167,  171,  175,  178,  172,  179,  183,  186,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   279,  265,  265,  265,  265,  265,  265,  265,  265,  265,
 /*    10 */   265,  265,  265,  265,  265,  265,  265,  279,  282,  265,
 /*    20 */   265,  265,  265,  265,  265,  265,  265,  265,  265,  265,
 /*    30 */   308,  308,  286,  293,  308,  308,  294,  265,  265,  265,
 /*    40 */   265,  308,  308,  265,  308,  265,  265,  308,  265,  345,
 /*    50 */   343,  336,  272,  345,  343,  336,  270,  312,  265,  322,
 /*    60 */   314,  290,  332,  333,  265,  337,  313,  325,  265,  265,
 /*    70 */   334,  326,  265,  265,  265,  265,  265,  278,  317,  265,
 /*    80 */   295,  265,  287,  265,  265,  265,  265,  265,  265,  265,
 /*    90 */   265,  265,  265,  265,  265,  310,  265,  265,  265,  265,
 /*   100 */   280,  319,  265,  318,  265,  334,  265,  265,  265,  265,
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
  /*   32 */ "RETURN",
  /*   33 */ "DISTINCT",
  /*   34 */ "AS",
  /*   35 */ "DOT",
  /*   36 */ "ORDER",
  /*   37 */ "BY",
  /*   38 */ "ASC",
  /*   39 */ "DESC",
  /*   40 */ "SKIP",
  /*   41 */ "INTEGER",
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
  /*   77 */ "mapLiteral",
  /*   78 */ "value",
  /*   79 */ "cond",
  /*   80 */ "relation",
  /*   81 */ "returnElements",
  /*   82 */ "returnElement",
  /*   83 */ "arithmetic_expression_list",
  /*   84 */ "columnNameList",
  /*   85 */ "columnName",
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
 /*  38 */ "edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET",
 /*  39 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  40 */ "properties ::=",
 /*  41 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  42 */ "mapLiteral ::= UQSTRING COLON value",
 /*  43 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  44 */ "whereClause ::=",
 /*  45 */ "whereClause ::= WHERE cond",
 /*  46 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  47 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  48 */ "cond ::= cond AND cond",
 /*  49 */ "cond ::= cond OR cond",
 /*  50 */ "returnClause ::= RETURN returnElements",
 /*  51 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  52 */ "returnElements ::= returnElements COMMA returnElement",
 /*  53 */ "returnElements ::= returnElement",
 /*  54 */ "returnElement ::= arithmetic_expression",
 /*  55 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  56 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  57 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  58 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  59 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  60 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  61 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  62 */ "arithmetic_expression ::= value",
 /*  63 */ "arithmetic_expression ::= variable",
 /*  64 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  65 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  66 */ "variable ::= UQSTRING",
 /*  67 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  68 */ "orderClause ::=",
 /*  69 */ "orderClause ::= ORDER BY columnNameList",
 /*  70 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  71 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  72 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  73 */ "columnNameList ::= columnName",
 /*  74 */ "columnName ::= variable",
 /*  75 */ "skipClause ::=",
 /*  76 */ "skipClause ::= SKIP INTEGER",
 /*  77 */ "limitClause ::=",
 /*  78 */ "limitClause ::= LIMIT INTEGER",
 /*  79 */ "relation ::= EQ",
 /*  80 */ "relation ::= GT",
 /*  81 */ "relation ::= LT",
 /*  82 */ "relation ::= LE",
 /*  83 */ "relation ::= GE",
 /*  84 */ "relation ::= NE",
 /*  85 */ "value ::= INTEGER",
 /*  86 */ "value ::= DASH INTEGER",
 /*  87 */ "value ::= STRING",
 /*  88 */ "value ::= FLOAT",
 /*  89 */ "value ::= DASH FLOAT",
 /*  90 */ "value ::= TRUE",
 /*  91 */ "value ::= FALSE",
 /*  92 */ "value ::= NULLVAL",
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
    case 79: /* cond */
{
#line 284 "grammar.y"
 Free_AST_FilterNode((yypminor->yy150)); 
#line 741 "grammar.c"
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
  {   76,   -5 }, /* (38) edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
  {   76,   -6 }, /* (39) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   75,    0 }, /* (40) properties ::= */
  {   75,   -3 }, /* (41) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   77,   -3 }, /* (42) mapLiteral ::= UQSTRING COLON value */
  {   77,   -5 }, /* (43) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   53,    0 }, /* (44) whereClause ::= */
  {   53,   -2 }, /* (45) whereClause ::= WHERE cond */
  {   79,   -3 }, /* (46) cond ::= arithmetic_expression relation arithmetic_expression */
  {   79,   -3 }, /* (47) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   79,   -3 }, /* (48) cond ::= cond AND cond */
  {   79,   -3 }, /* (49) cond ::= cond OR cond */
  {   55,   -2 }, /* (50) returnClause ::= RETURN returnElements */
  {   55,   -3 }, /* (51) returnClause ::= RETURN DISTINCT returnElements */
  {   81,   -3 }, /* (52) returnElements ::= returnElements COMMA returnElement */
  {   81,   -1 }, /* (53) returnElements ::= returnElement */
  {   82,   -1 }, /* (54) returnElement ::= arithmetic_expression */
  {   82,   -3 }, /* (55) returnElement ::= arithmetic_expression AS UQSTRING */
  {   71,   -3 }, /* (56) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   71,   -3 }, /* (57) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   71,   -3 }, /* (58) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   71,   -3 }, /* (59) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   71,   -3 }, /* (60) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   71,   -4 }, /* (61) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   71,   -1 }, /* (62) arithmetic_expression ::= value */
  {   71,   -1 }, /* (63) arithmetic_expression ::= variable */
  {   83,   -3 }, /* (64) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   83,   -1 }, /* (65) arithmetic_expression_list ::= arithmetic_expression */
  {   70,   -1 }, /* (66) variable ::= UQSTRING */
  {   70,   -3 }, /* (67) variable ::= UQSTRING DOT UQSTRING */
  {   56,    0 }, /* (68) orderClause ::= */
  {   56,   -3 }, /* (69) orderClause ::= ORDER BY columnNameList */
  {   56,   -4 }, /* (70) orderClause ::= ORDER BY columnNameList ASC */
  {   56,   -4 }, /* (71) orderClause ::= ORDER BY columnNameList DESC */
  {   84,   -3 }, /* (72) columnNameList ::= columnNameList COMMA columnName */
  {   84,   -1 }, /* (73) columnNameList ::= columnName */
  {   85,   -1 }, /* (74) columnName ::= variable */
  {   57,    0 }, /* (75) skipClause ::= */
  {   57,   -2 }, /* (76) skipClause ::= SKIP INTEGER */
  {   58,    0 }, /* (77) limitClause ::= */
  {   58,   -2 }, /* (78) limitClause ::= LIMIT INTEGER */
  {   80,   -1 }, /* (79) relation ::= EQ */
  {   80,   -1 }, /* (80) relation ::= GT */
  {   80,   -1 }, /* (81) relation ::= LT */
  {   80,   -1 }, /* (82) relation ::= LE */
  {   80,   -1 }, /* (83) relation ::= GE */
  {   80,   -1 }, /* (84) relation ::= NE */
  {   78,   -1 }, /* (85) value ::= INTEGER */
  {   78,   -2 }, /* (86) value ::= DASH INTEGER */
  {   78,   -1 }, /* (87) value ::= STRING */
  {   78,   -1 }, /* (88) value ::= FLOAT */
  {   78,   -2 }, /* (89) value ::= DASH FLOAT */
  {   78,   -1 }, /* (90) value ::= TRUE */
  {   78,   -1 }, /* (91) value ::= FALSE */
  {   78,   -1 }, /* (92) value ::= NULLVAL */
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
{ ctx->root = yymsp[0].minor.yy4; }
#line 1211 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause skipClause limitClause */
#line 38 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(yymsp[-6].minor.yy45, yymsp[-5].minor.yy119, yymsp[-4].minor.yy16, NULL, NULL, NULL, yymsp[-3].minor.yy12, yymsp[-2].minor.yy168, yymsp[-1].minor.yy120, yymsp[0].minor.yy121, NULL);
}
#line 1218 "grammar.c"
  yymsp[-6].minor.yy4 = yylhsminor.yy4;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 42 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(yymsp[-2].minor.yy45, yymsp[-1].minor.yy119, yymsp[0].minor.yy16, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1226 "grammar.c"
  yymsp[-2].minor.yy4 = yylhsminor.yy4;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 46 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(yymsp[-2].minor.yy45, yymsp[-1].minor.yy119, NULL, NULL, NULL, yymsp[0].minor.yy7, NULL, NULL, NULL, NULL, NULL);
}
#line 1234 "grammar.c"
  yymsp[-2].minor.yy4 = yylhsminor.yy4;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 50 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(yymsp[-2].minor.yy45, yymsp[-1].minor.yy119, NULL, NULL, yymsp[0].minor.yy76, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1242 "grammar.c"
  yymsp[-2].minor.yy4 = yylhsminor.yy4;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 54 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(yymsp[-6].minor.yy45, yymsp[-5].minor.yy119, NULL, NULL, yymsp[-4].minor.yy76, NULL, yymsp[-3].minor.yy12, yymsp[-2].minor.yy168, yymsp[-1].minor.yy120, yymsp[0].minor.yy121, NULL);
}
#line 1250 "grammar.c"
  yymsp[-6].minor.yy4 = yylhsminor.yy4;
        break;
      case 6: /* expr ::= createClause */
#line 58 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy16, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1258 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 7: /* expr ::= indexClause */
#line 62 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy116);
}
#line 1266 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 8: /* expr ::= mergeClause */
#line 66 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy108, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1274 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 9: /* expr ::= returnClause */
#line 70 "grammar.y"
{
	yylhsminor.yy4 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy12, NULL, NULL, NULL, NULL);
}
#line 1282 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 10: /* matchClause ::= MATCH chains */
#line 76 "grammar.y"
{
	yymsp[-1].minor.yy45 = New_AST_MatchNode(yymsp[0].minor.yy110);
}
#line 1290 "grammar.c"
        break;
      case 11: /* createClause ::= */
#line 83 "grammar.y"
{
	yymsp[1].minor.yy16 = NULL;
}
#line 1297 "grammar.c"
        break;
      case 12: /* createClause ::= CREATE chains */
#line 87 "grammar.y"
{
	yymsp[-1].minor.yy16 = New_AST_CreateNode(yymsp[0].minor.yy110);
}
#line 1304 "grammar.c"
        break;
      case 13: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 93 "grammar.y"
{
  yylhsminor.yy116 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy149);
}
#line 1311 "grammar.c"
  yymsp[-4].minor.yy116 = yylhsminor.yy116;
        break;
      case 14: /* indexOpToken ::= CREATE */
#line 99 "grammar.y"
{ yymsp[0].minor.yy149 = CREATE_INDEX; }
#line 1317 "grammar.c"
        break;
      case 15: /* indexOpToken ::= DROP */
#line 100 "grammar.y"
{ yymsp[0].minor.yy149 = DROP_INDEX; }
#line 1322 "grammar.c"
        break;
      case 16: /* indexLabel ::= COLON UQSTRING */
#line 102 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1329 "grammar.c"
        break;
      case 17: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 106 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1336 "grammar.c"
        break;
      case 18: /* mergeClause ::= MERGE chain */
#line 112 "grammar.y"
{
	yymsp[-1].minor.yy108 = New_AST_MergeNode(yymsp[0].minor.yy110);
}
#line 1343 "grammar.c"
        break;
      case 19: /* setClause ::= SET setList */
#line 117 "grammar.y"
{
	yymsp[-1].minor.yy76 = New_AST_SetNode(yymsp[0].minor.yy110);
}
#line 1350 "grammar.c"
        break;
      case 20: /* setList ::= setElement */
#line 122 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy92);
}
#line 1358 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 21: /* setList ::= setList COMMA setElement */
#line 126 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy92);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1367 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 22: /* setElement ::= variable EQ arithmetic_expression */
#line 132 "grammar.y"
{
	yylhsminor.yy92 = New_AST_SetElement(yymsp[-2].minor.yy80, yymsp[0].minor.yy22);
}
#line 1375 "grammar.c"
  yymsp[-2].minor.yy92 = yylhsminor.yy92;
        break;
      case 23: /* chain ::= node */
#line 138 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy101);
}
#line 1384 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 24: /* chain ::= chain link node */
#line 143 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[-1].minor.yy37);
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy101);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1394 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 25: /* chains ::= chain */
#line 151 "grammar.y"
{
	yylhsminor.yy110 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy110);
}
#line 1403 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 26: /* chains ::= chains COMMA chain */
#line 156 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy110);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1412 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 27: /* deleteClause ::= DELETE deleteExpression */
#line 164 "grammar.y"
{
	yymsp[-1].minor.yy7 = New_AST_DeleteNode(yymsp[0].minor.yy110);
}
#line 1420 "grammar.c"
        break;
      case 28: /* deleteExpression ::= UQSTRING */
#line 170 "grammar.y"
{
	yylhsminor.yy110 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy0.strval);
}
#line 1428 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 29: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 175 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy0.strval);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1437 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 30: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 183 "grammar.y"
{
	yymsp[-5].minor.yy101 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110);
}
#line 1445 "grammar.c"
        break;
      case 31: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 188 "grammar.y"
{
	yymsp[-4].minor.yy101 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110);
}
#line 1452 "grammar.c"
        break;
      case 32: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 193 "grammar.y"
{
	yymsp[-3].minor.yy101 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy110);
}
#line 1459 "grammar.c"
        break;
      case 33: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 198 "grammar.y"
{
	yymsp[-2].minor.yy101 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy110);
}
#line 1466 "grammar.c"
        break;
      case 34: /* link ::= DASH edge RIGHT_ARROW */
#line 205 "grammar.y"
{
	yymsp[-2].minor.yy37 = yymsp[-1].minor.yy37;
	yymsp[-2].minor.yy37->direction = N_LEFT_TO_RIGHT;
}
#line 1474 "grammar.c"
        break;
      case 35: /* link ::= LEFT_ARROW edge DASH */
#line 211 "grammar.y"
{
	yymsp[-2].minor.yy37 = yymsp[-1].minor.yy37;
	yymsp[-2].minor.yy37->direction = N_RIGHT_TO_LEFT;
}
#line 1482 "grammar.c"
        break;
      case 36: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 218 "grammar.y"
{ 
	yymsp[-2].minor.yy37 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1489 "grammar.c"
        break;
      case 37: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 223 "grammar.y"
{ 
	yymsp[-3].minor.yy37 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1496 "grammar.c"
        break;
      case 38: /* edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
#line 228 "grammar.y"
{ 
	yymsp[-4].minor.yy37 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1503 "grammar.c"
        break;
      case 39: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 233 "grammar.y"
{ 
	yymsp[-5].minor.yy37 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy110, N_DIR_UNKNOWN);
}
#line 1510 "grammar.c"
        break;
      case 40: /* properties ::= */
#line 239 "grammar.y"
{
	yymsp[1].minor.yy110 = NULL;
}
#line 1517 "grammar.c"
        break;
      case 41: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 243 "grammar.y"
{
	yymsp[-2].minor.yy110 = yymsp[-1].minor.yy110;
}
#line 1524 "grammar.c"
        break;
      case 42: /* mapLiteral ::= UQSTRING COLON value */
#line 249 "grammar.y"
{
	yylhsminor.yy110 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy110, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy23;
	Vector_Push(yylhsminor.yy110, val);
}
#line 1539 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 43: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 261 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy110, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy23;
	Vector_Push(yymsp[0].minor.yy110, val);
	
	yylhsminor.yy110 = yymsp[0].minor.yy110;
}
#line 1555 "grammar.c"
  yymsp[-4].minor.yy110 = yylhsminor.yy110;
        break;
      case 44: /* whereClause ::= */
#line 275 "grammar.y"
{ 
	yymsp[1].minor.yy119 = NULL;
}
#line 1563 "grammar.c"
        break;
      case 45: /* whereClause ::= WHERE cond */
#line 278 "grammar.y"
{
	yymsp[-1].minor.yy119 = New_AST_WhereNode(yymsp[0].minor.yy150);
}
#line 1570 "grammar.c"
        break;
      case 46: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 287 "grammar.y"
{ yylhsminor.yy150 = New_AST_PredicateNode(yymsp[-2].minor.yy22, yymsp[-1].minor.yy64, yymsp[0].minor.yy22); }
#line 1575 "grammar.c"
  yymsp[-2].minor.yy150 = yylhsminor.yy150;
        break;
      case 47: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 289 "grammar.y"
{ yymsp[-2].minor.yy150 = yymsp[-1].minor.yy150; }
#line 1581 "grammar.c"
        break;
      case 48: /* cond ::= cond AND cond */
#line 290 "grammar.y"
{ yylhsminor.yy150 = New_AST_ConditionNode(yymsp[-2].minor.yy150, AND, yymsp[0].minor.yy150); }
#line 1586 "grammar.c"
  yymsp[-2].minor.yy150 = yylhsminor.yy150;
        break;
      case 49: /* cond ::= cond OR cond */
#line 291 "grammar.y"
{ yylhsminor.yy150 = New_AST_ConditionNode(yymsp[-2].minor.yy150, OR, yymsp[0].minor.yy150); }
#line 1592 "grammar.c"
  yymsp[-2].minor.yy150 = yylhsminor.yy150;
        break;
      case 50: /* returnClause ::= RETURN returnElements */
#line 295 "grammar.y"
{
	yymsp[-1].minor.yy12 = New_AST_ReturnNode(yymsp[0].minor.yy110, 0);
}
#line 1600 "grammar.c"
        break;
      case 51: /* returnClause ::= RETURN DISTINCT returnElements */
#line 298 "grammar.y"
{
	yymsp[-2].minor.yy12 = New_AST_ReturnNode(yymsp[0].minor.yy110, 1);
}
#line 1607 "grammar.c"
        break;
      case 52: /* returnElements ::= returnElements COMMA returnElement */
#line 305 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy62);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1615 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 53: /* returnElements ::= returnElement */
#line 310 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy62);
}
#line 1624 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 54: /* returnElement ::= arithmetic_expression */
#line 317 "grammar.y"
{
	yylhsminor.yy62 = New_AST_ReturnElementNode(yymsp[0].minor.yy22, NULL);
}
#line 1632 "grammar.c"
  yymsp[0].minor.yy62 = yylhsminor.yy62;
        break;
      case 55: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 322 "grammar.y"
{
	yylhsminor.yy62 = New_AST_ReturnElementNode(yymsp[-2].minor.yy22, yymsp[0].minor.yy0.strval);
}
#line 1640 "grammar.c"
  yymsp[-2].minor.yy62 = yylhsminor.yy62;
        break;
      case 56: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 329 "grammar.y"
{
	yymsp[-2].minor.yy22 = yymsp[-1].minor.yy22;
}
#line 1648 "grammar.c"
        break;
      case 57: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 341 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy22);
	Vector_Push(args, yymsp[0].minor.yy22);
	yylhsminor.yy22 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1658 "grammar.c"
  yymsp[-2].minor.yy22 = yylhsminor.yy22;
        break;
      case 58: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 348 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy22);
	Vector_Push(args, yymsp[0].minor.yy22);
	yylhsminor.yy22 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1669 "grammar.c"
  yymsp[-2].minor.yy22 = yylhsminor.yy22;
        break;
      case 59: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 355 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy22);
	Vector_Push(args, yymsp[0].minor.yy22);
	yylhsminor.yy22 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1680 "grammar.c"
  yymsp[-2].minor.yy22 = yylhsminor.yy22;
        break;
      case 60: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 362 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy22);
	Vector_Push(args, yymsp[0].minor.yy22);
	yylhsminor.yy22 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1691 "grammar.c"
  yymsp[-2].minor.yy22 = yylhsminor.yy22;
        break;
      case 61: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 370 "grammar.y"
{
	yylhsminor.yy22 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy110);
}
#line 1699 "grammar.c"
  yymsp[-3].minor.yy22 = yylhsminor.yy22;
        break;
      case 62: /* arithmetic_expression ::= value */
#line 375 "grammar.y"
{
	yylhsminor.yy22 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy23);
}
#line 1707 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 63: /* arithmetic_expression ::= variable */
#line 380 "grammar.y"
{
	yylhsminor.yy22 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy80->alias, yymsp[0].minor.yy80->property);
}
#line 1715 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 64: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 386 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy22);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1724 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 65: /* arithmetic_expression_list ::= arithmetic_expression */
#line 390 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy22);
}
#line 1733 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 66: /* variable ::= UQSTRING */
#line 397 "grammar.y"
{
	yylhsminor.yy80 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1741 "grammar.c"
  yymsp[0].minor.yy80 = yylhsminor.yy80;
        break;
      case 67: /* variable ::= UQSTRING DOT UQSTRING */
#line 401 "grammar.y"
{
	yylhsminor.yy80 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1749 "grammar.c"
  yymsp[-2].minor.yy80 = yylhsminor.yy80;
        break;
      case 68: /* orderClause ::= */
#line 407 "grammar.y"
{
	yymsp[1].minor.yy168 = NULL;
}
#line 1757 "grammar.c"
        break;
      case 69: /* orderClause ::= ORDER BY columnNameList */
#line 410 "grammar.y"
{
	yymsp[-2].minor.yy168 = New_AST_OrderNode(yymsp[0].minor.yy110, ORDER_DIR_ASC);
}
#line 1764 "grammar.c"
        break;
      case 70: /* orderClause ::= ORDER BY columnNameList ASC */
#line 413 "grammar.y"
{
	yymsp[-3].minor.yy168 = New_AST_OrderNode(yymsp[-1].minor.yy110, ORDER_DIR_ASC);
}
#line 1771 "grammar.c"
        break;
      case 71: /* orderClause ::= ORDER BY columnNameList DESC */
#line 416 "grammar.y"
{
	yymsp[-3].minor.yy168 = New_AST_OrderNode(yymsp[-1].minor.yy110, ORDER_DIR_DESC);
}
#line 1778 "grammar.c"
        break;
      case 72: /* columnNameList ::= columnNameList COMMA columnName */
#line 421 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy110, yymsp[0].minor.yy138);
	yylhsminor.yy110 = yymsp[-2].minor.yy110;
}
#line 1786 "grammar.c"
  yymsp[-2].minor.yy110 = yylhsminor.yy110;
        break;
      case 73: /* columnNameList ::= columnName */
#line 425 "grammar.y"
{
	yylhsminor.yy110 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy110, yymsp[0].minor.yy138);
}
#line 1795 "grammar.c"
  yymsp[0].minor.yy110 = yylhsminor.yy110;
        break;
      case 74: /* columnName ::= variable */
#line 431 "grammar.y"
{
	if(yymsp[0].minor.yy80->property != NULL) {
		yylhsminor.yy138 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy80);
	} else {
		yylhsminor.yy138 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy80->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy80);
}
#line 1809 "grammar.c"
  yymsp[0].minor.yy138 = yylhsminor.yy138;
        break;
      case 75: /* skipClause ::= */
#line 443 "grammar.y"
{
	yymsp[1].minor.yy120 = NULL;
}
#line 1817 "grammar.c"
        break;
      case 76: /* skipClause ::= SKIP INTEGER */
#line 446 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_SkipNode(yymsp[0].minor.yy0.intval);
}
#line 1824 "grammar.c"
        break;
      case 77: /* limitClause ::= */
#line 452 "grammar.y"
{
	yymsp[1].minor.yy121 = NULL;
}
#line 1831 "grammar.c"
        break;
      case 78: /* limitClause ::= LIMIT INTEGER */
#line 455 "grammar.y"
{
	yymsp[-1].minor.yy121 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1838 "grammar.c"
        break;
      case 79: /* relation ::= EQ */
#line 461 "grammar.y"
{ yymsp[0].minor.yy64 = EQ; }
#line 1843 "grammar.c"
        break;
      case 80: /* relation ::= GT */
#line 462 "grammar.y"
{ yymsp[0].minor.yy64 = GT; }
#line 1848 "grammar.c"
        break;
      case 81: /* relation ::= LT */
#line 463 "grammar.y"
{ yymsp[0].minor.yy64 = LT; }
#line 1853 "grammar.c"
        break;
      case 82: /* relation ::= LE */
#line 464 "grammar.y"
{ yymsp[0].minor.yy64 = LE; }
#line 1858 "grammar.c"
        break;
      case 83: /* relation ::= GE */
#line 465 "grammar.y"
{ yymsp[0].minor.yy64 = GE; }
#line 1863 "grammar.c"
        break;
      case 84: /* relation ::= NE */
#line 466 "grammar.y"
{ yymsp[0].minor.yy64 = NE; }
#line 1868 "grammar.c"
        break;
      case 85: /* value ::= INTEGER */
#line 477 "grammar.y"
{  yylhsminor.yy23 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1873 "grammar.c"
  yymsp[0].minor.yy23 = yylhsminor.yy23;
        break;
      case 86: /* value ::= DASH INTEGER */
#line 478 "grammar.y"
{  yymsp[-1].minor.yy23 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1879 "grammar.c"
        break;
      case 87: /* value ::= STRING */
#line 479 "grammar.y"
{  yylhsminor.yy23 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1884 "grammar.c"
  yymsp[0].minor.yy23 = yylhsminor.yy23;
        break;
      case 88: /* value ::= FLOAT */
#line 480 "grammar.y"
{  yylhsminor.yy23 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1890 "grammar.c"
  yymsp[0].minor.yy23 = yylhsminor.yy23;
        break;
      case 89: /* value ::= DASH FLOAT */
#line 481 "grammar.y"
{  yymsp[-1].minor.yy23 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1896 "grammar.c"
        break;
      case 90: /* value ::= TRUE */
#line 482 "grammar.y"
{ yymsp[0].minor.yy23 = SI_BoolVal(1); }
#line 1901 "grammar.c"
        break;
      case 91: /* value ::= FALSE */
#line 483 "grammar.y"
{ yymsp[0].minor.yy23 = SI_BoolVal(0); }
#line 1906 "grammar.c"
        break;
      case 92: /* value ::= NULLVAL */
#line 484 "grammar.y"
{ yymsp[0].minor.yy23 = SI_NullVal(); }
#line 1911 "grammar.c"
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
#line 1976 "grammar.c"
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
#line 486 "grammar.y"


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
#line 2222 "grammar.c"
