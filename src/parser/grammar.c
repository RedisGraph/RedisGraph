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
#define YYNOCODE 77
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  Vector* yy2;
  AST_ArithmeticExpressionNode* yy3;
  AST_Query* yy8;
  AST_FilterNode* yy10;
  AST_NodeEntity* yy13;
  AST_ColumnNode* yy14;
  AST_CreateNode* yy20;
  AST_OrderNode* yy28;
  AST_ReturnElementNode* yy34;
  AST_LimitNode* yy39;
  AST_DeleteNode * yy40;
  AST_Variable* yy44;
  AST_SetElement* yy48;
  AST_WhereNode* yy67;
  int yy84;
  AST_SetNode* yy92;
  AST_ReturnNode* yy112;
  AST_MatchNode* yy117;
  AST_MergeNode* yy124;
  AST_LinkEntity* yy149;
  SIValue yy150;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             101
#define YYNRULE              84
#define YYNTOKEN             44
#define YY_MAX_SHIFT         100
#define YY_MIN_SHIFTREDUCE   157
#define YY_MAX_SHIFTREDUCE   240
#define YY_ERROR_ACTION      241
#define YY_ACCEPT_ACTION     242
#define YY_NO_ACTION         243
#define YY_MIN_REDUCE        244
#define YY_MAX_REDUCE        327
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
#define YY_ACTTAB_COUNT (243)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   284,  100,  242,   51,   61,  250,   74,  257,   73,  301,
 /*    10 */    53,  251,  228,  229,  232,  230,  231,  300,   63,   10,
 /*    20 */    95,  291,    3,   61,   11,    9,    8,    7,   13,   15,
 /*    30 */    17,    2,   96,   65,   31,   21,   61,   68,  234,  260,
 /*    40 */   236,  237,  239,  240,  233,   61,   11,    9,    8,    7,
 /*    50 */    63,   10,   25,   94,  312,  301,   53,  234,   87,  236,
 /*    60 */   237,  239,  240,  300,  207,   97,   93,  291,   58,  311,
 /*    70 */   234,   26,  236,  237,  239,  240,  260,  301,   56,  234,
 /*    80 */    81,  236,  237,  239,  240,  300,  301,   53,  301,   54,
 /*    90 */    62,  301,   55,   15,  300,   14,  300,   35,  290,  300,
 /*   100 */   312,   83,   11,    9,    8,    7,  301,  298,  301,  297,
 /*   110 */   301,   64,  301,   52,  300,  310,  300,   44,  300,  178,
 /*   120 */   300,  301,   60,   50,   92,   31,   20,  247,   47,  300,
 /*   130 */   260,   32,   33,   77,   85,   27,   29,  258,   73,   42,
 /*   140 */    42,   39,   34,   70,   28,  221,  222,  260,   79,   80,
 /*   150 */    91,   66,   27,   29,   42,  198,   42,    6,    8,    7,
 /*   160 */    36,  235,  212,   57,  238,  286,   59,   86,   72,   24,
 /*   170 */    42,   75,   76,   78,   88,   82,   84,   99,  249,  280,
 /*   180 */   261,   90,   89,   98,   45,    1,   46,  245,   30,   48,
 /*   190 */    19,   12,   49,   29,  196,   67,   22,   69,  179,   71,
 /*   200 */     5,   18,  185,  183,  188,   37,  189,   38,   40,  187,
 /*   210 */   184,  186,   41,  181,  244,  182,  243,   23,   43,    4,
 /*   220 */   180,  243,  191,  206,  218,  243,  243,  243,  243,  243,
 /*   230 */    96,  243,  243,  243,  243,  243,   16,  243,  243,  243,
 /*   240 */   243,  243,  227,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    68,   45,   46,   47,    4,   49,   58,   59,   60,   60,
 /*    10 */    61,   55,    7,    8,    9,   10,   11,   68,   18,   19,
 /*    20 */    71,   72,   19,    4,    3,    4,    5,    6,   12,   13,
 /*    30 */    14,   31,   29,   56,   57,   70,    4,   18,   38,   62,
 /*    40 */    40,   41,   42,   43,   39,    4,    3,    4,    5,    6,
 /*    50 */    18,   19,   19,   32,   60,   60,   61,   38,   68,   40,
 /*    60 */    41,   42,   43,   68,   21,   18,   71,   72,   74,   75,
 /*    70 */    38,   57,   40,   41,   42,   43,   62,   60,   61,   38,
 /*    80 */    65,   40,   41,   42,   43,   68,   60,   61,   60,   61,
 /*    90 */    73,   60,   61,   13,   68,   15,   68,   17,   72,   68,
 /*   100 */    60,   65,    3,    4,    5,    6,   60,   61,   60,   61,
 /*   110 */    60,   61,   60,   61,   68,   75,   68,   63,   68,   18,
 /*   120 */    68,   60,   61,   49,   56,   57,   16,   53,   54,   68,
 /*   130 */    62,   18,   18,   20,   20,    1,    2,   59,   60,   26,
 /*   140 */    26,    4,   57,   18,   19,   35,   36,   62,   20,   65,
 /*   150 */    20,   69,    1,    2,   26,   21,   26,   16,    5,    6,
 /*   160 */    23,   38,   21,   69,   41,   69,   69,   65,   64,   24,
 /*   170 */    26,   66,   65,   65,   18,   66,   65,   37,   52,   67,
 /*   180 */    62,   65,   67,   33,   51,   30,   50,   52,   28,   51,
 /*   190 */    16,   48,   50,    2,   18,   29,   18,   29,   18,   16,
 /*   200 */     7,   16,    4,   21,   25,   18,   25,   18,   18,   25,
 /*   210 */    22,   25,   16,   21,    0,   21,   76,   20,   18,   16,
 /*   220 */    21,   76,   27,   18,   18,   76,   76,   76,   76,   76,
 /*   230 */    29,   76,   76,   76,   76,   76,   34,   76,   76,   76,
 /*   240 */    76,   76,   38,   76,   76,   76,   76,   76,   76,   76,
 /*   250 */    76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
 /*   260 */    76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
 /*   270 */    76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
 /*   280 */    76,   76,   76,   76,   76,   76,   76,
};
#define YY_SHIFT_COUNT    (100)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (214)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    16,    0,   32,   32,   32,   32,   32,   32,   32,   32,
 /*    10 */    32,   32,   80,   33,   47,   33,   47,   33,   47,   33,
 /*    20 */    47,   19,    5,   41,  113,  114,  137,  125,  125,  125,
 /*    30 */   125,  137,  128,  130,  137,  101,  145,  144,  144,  145,
 /*    40 */   144,  156,  156,  144,   33,  140,  150,  155,  140,  150,
 /*    50 */   155,  160,   43,   21,   99,   99,   99,  134,  110,  151,
 /*    60 */   153,  123,  141,    3,  153,  174,  191,  176,  166,  178,
 /*    70 */   168,  180,  183,  193,  185,  198,  179,  187,  181,  189,
 /*    80 */   184,  186,  188,  182,  192,  190,  194,  196,  197,  195,
 /*    90 */   199,  200,  174,  203,  205,  203,  206,  201,  202,  204,
 /*   100 */   214,
};
#define YY_REDUCE_COUNT (51)
#define YY_REDUCE_MIN   (-68)
#define YY_REDUCE_MAX   (143)
static const short yy_reduce_ofst[] = {
 /*     0 */   -44,  -51,   -5,   17,   26,   28,   31,   46,   48,   50,
 /*    10 */    52,   61,   74,  -23,  -52,   68,   -6,   14,   78,   85,
 /*    20 */    40,  -68,  -35,  -10,   15,   36,   54,   82,   94,   96,
 /*    30 */    97,   54,   84,  102,   54,  104,  105,  107,  108,  109,
 /*    40 */   111,  112,  115,  116,  118,  126,  133,  136,  135,  138,
 /*    50 */   142,  143,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   253,  241,  241,  241,  241,  241,  241,  241,  241,  241,
 /*    10 */   241,  241,  253,  241,  241,  241,  241,  241,  241,  241,
 /*    20 */   241,  241,  241,  241,  277,  277,  255,  241,  241,  241,
 /*    30 */   241,  262,  277,  277,  263,  241,  241,  277,  277,  241,
 /*    40 */   277,  241,  241,  277,  241,  313,  306,  248,  313,  306,
 /*    50 */   246,  281,  241,  292,  259,  302,  303,  241,  307,  282,
 /*    60 */   295,  241,  241,  304,  296,  252,  287,  241,  241,  241,
 /*    70 */   241,  241,  264,  241,  256,  241,  241,  241,  241,  241,
 /*    80 */   241,  241,  241,  241,  241,  241,  241,  279,  241,  241,
 /*    90 */   241,  241,  254,  289,  241,  288,  241,  304,  241,  241,
 /*   100 */   241,
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
  /*   14 */ "MERGE",
  /*   15 */ "SET",
  /*   16 */ "COMMA",
  /*   17 */ "DELETE",
  /*   18 */ "UQSTRING",
  /*   19 */ "LEFT_PARENTHESIS",
  /*   20 */ "COLON",
  /*   21 */ "RIGHT_PARENTHESIS",
  /*   22 */ "RIGHT_ARROW",
  /*   23 */ "LEFT_ARROW",
  /*   24 */ "LEFT_BRACKET",
  /*   25 */ "RIGHT_BRACKET",
  /*   26 */ "LEFT_CURLY_BRACKET",
  /*   27 */ "RIGHT_CURLY_BRACKET",
  /*   28 */ "WHERE",
  /*   29 */ "DOT",
  /*   30 */ "RETURN",
  /*   31 */ "DISTINCT",
  /*   32 */ "AS",
  /*   33 */ "ORDER",
  /*   34 */ "BY",
  /*   35 */ "ASC",
  /*   36 */ "DESC",
  /*   37 */ "LIMIT",
  /*   38 */ "INTEGER",
  /*   39 */ "NE",
  /*   40 */ "STRING",
  /*   41 */ "FLOAT",
  /*   42 */ "TRUE",
  /*   43 */ "FALSE",
  /*   44 */ "error",
  /*   45 */ "expr",
  /*   46 */ "query",
  /*   47 */ "matchClause",
  /*   48 */ "whereClause",
  /*   49 */ "createClause",
  /*   50 */ "returnClause",
  /*   51 */ "orderClause",
  /*   52 */ "limitClause",
  /*   53 */ "deleteClause",
  /*   54 */ "setClause",
  /*   55 */ "mergeClause",
  /*   56 */ "chains",
  /*   57 */ "chain",
  /*   58 */ "setList",
  /*   59 */ "setElement",
  /*   60 */ "variable",
  /*   61 */ "arithmetic_expression",
  /*   62 */ "node",
  /*   63 */ "link",
  /*   64 */ "deleteExpression",
  /*   65 */ "properties",
  /*   66 */ "edge",
  /*   67 */ "mapLiteral",
  /*   68 */ "value",
  /*   69 */ "cond",
  /*   70 */ "relation",
  /*   71 */ "returnElements",
  /*   72 */ "returnElement",
  /*   73 */ "arithmetic_expression_list",
  /*   74 */ "columnNameList",
  /*   75 */ "columnName",
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
 /*   7 */ "expr ::= mergeClause",
 /*   8 */ "matchClause ::= MATCH chains",
 /*   9 */ "createClause ::=",
 /*  10 */ "createClause ::= CREATE chains",
 /*  11 */ "mergeClause ::= MERGE chain",
 /*  12 */ "setClause ::= SET setList",
 /*  13 */ "setList ::= setElement",
 /*  14 */ "setList ::= setList COMMA setElement",
 /*  15 */ "setElement ::= variable EQ arithmetic_expression",
 /*  16 */ "chain ::= node",
 /*  17 */ "chain ::= chain link node",
 /*  18 */ "chains ::= chain",
 /*  19 */ "chains ::= chains COMMA chain",
 /*  20 */ "deleteClause ::= DELETE deleteExpression",
 /*  21 */ "deleteExpression ::= UQSTRING",
 /*  22 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  23 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  24 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  25 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  26 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  27 */ "link ::= DASH edge RIGHT_ARROW",
 /*  28 */ "link ::= LEFT_ARROW edge DASH",
 /*  29 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  30 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  31 */ "edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET",
 /*  32 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  33 */ "properties ::=",
 /*  34 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  35 */ "mapLiteral ::= UQSTRING COLON value",
 /*  36 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  37 */ "whereClause ::=",
 /*  38 */ "whereClause ::= WHERE cond",
 /*  39 */ "cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING",
 /*  40 */ "cond ::= UQSTRING DOT UQSTRING relation value",
 /*  41 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  42 */ "cond ::= cond AND cond",
 /*  43 */ "cond ::= cond OR cond",
 /*  44 */ "returnClause ::= RETURN returnElements",
 /*  45 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  46 */ "returnElements ::= returnElements COMMA returnElement",
 /*  47 */ "returnElements ::= returnElement",
 /*  48 */ "returnElement ::= arithmetic_expression",
 /*  49 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  50 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  51 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  52 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  53 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  54 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  55 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  56 */ "arithmetic_expression ::= value",
 /*  57 */ "arithmetic_expression ::= variable",
 /*  58 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  59 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  60 */ "variable ::= UQSTRING",
 /*  61 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  62 */ "orderClause ::=",
 /*  63 */ "orderClause ::= ORDER BY columnNameList",
 /*  64 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  65 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  66 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  67 */ "columnNameList ::= columnName",
 /*  68 */ "columnName ::= variable",
 /*  69 */ "limitClause ::=",
 /*  70 */ "limitClause ::= LIMIT INTEGER",
 /*  71 */ "relation ::= EQ",
 /*  72 */ "relation ::= GT",
 /*  73 */ "relation ::= LT",
 /*  74 */ "relation ::= LE",
 /*  75 */ "relation ::= GE",
 /*  76 */ "relation ::= NE",
 /*  77 */ "value ::= INTEGER",
 /*  78 */ "value ::= DASH INTEGER",
 /*  79 */ "value ::= STRING",
 /*  80 */ "value ::= FLOAT",
 /*  81 */ "value ::= DASH FLOAT",
 /*  82 */ "value ::= TRUE",
 /*  83 */ "value ::= FALSE",
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
    case 69: /* cond */
{
#line 257 "grammar.y"
 Free_AST_FilterNode((yypminor->yy10)); 
#line 708 "grammar.c"
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
  {   46,   -1 }, /* (0) query ::= expr */
  {   45,   -6 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
  {   45,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   45,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   45,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   45,   -6 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
  {   45,   -1 }, /* (6) expr ::= createClause */
  {   45,   -1 }, /* (7) expr ::= mergeClause */
  {   47,   -2 }, /* (8) matchClause ::= MATCH chains */
  {   49,    0 }, /* (9) createClause ::= */
  {   49,   -2 }, /* (10) createClause ::= CREATE chains */
  {   55,   -2 }, /* (11) mergeClause ::= MERGE chain */
  {   54,   -2 }, /* (12) setClause ::= SET setList */
  {   58,   -1 }, /* (13) setList ::= setElement */
  {   58,   -3 }, /* (14) setList ::= setList COMMA setElement */
  {   59,   -3 }, /* (15) setElement ::= variable EQ arithmetic_expression */
  {   57,   -1 }, /* (16) chain ::= node */
  {   57,   -3 }, /* (17) chain ::= chain link node */
  {   56,   -1 }, /* (18) chains ::= chain */
  {   56,   -3 }, /* (19) chains ::= chains COMMA chain */
  {   53,   -2 }, /* (20) deleteClause ::= DELETE deleteExpression */
  {   64,   -1 }, /* (21) deleteExpression ::= UQSTRING */
  {   64,   -3 }, /* (22) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   62,   -6 }, /* (23) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   62,   -5 }, /* (24) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   62,   -4 }, /* (25) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   62,   -3 }, /* (26) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   63,   -3 }, /* (27) link ::= DASH edge RIGHT_ARROW */
  {   63,   -3 }, /* (28) link ::= LEFT_ARROW edge DASH */
  {   66,   -3 }, /* (29) edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
  {   66,   -4 }, /* (30) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   66,   -5 }, /* (31) edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
  {   66,   -6 }, /* (32) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   65,    0 }, /* (33) properties ::= */
  {   65,   -3 }, /* (34) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   67,   -3 }, /* (35) mapLiteral ::= UQSTRING COLON value */
  {   67,   -5 }, /* (36) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   48,    0 }, /* (37) whereClause ::= */
  {   48,   -2 }, /* (38) whereClause ::= WHERE cond */
  {   69,   -7 }, /* (39) cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
  {   69,   -5 }, /* (40) cond ::= UQSTRING DOT UQSTRING relation value */
  {   69,   -3 }, /* (41) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   69,   -3 }, /* (42) cond ::= cond AND cond */
  {   69,   -3 }, /* (43) cond ::= cond OR cond */
  {   50,   -2 }, /* (44) returnClause ::= RETURN returnElements */
  {   50,   -3 }, /* (45) returnClause ::= RETURN DISTINCT returnElements */
  {   71,   -3 }, /* (46) returnElements ::= returnElements COMMA returnElement */
  {   71,   -1 }, /* (47) returnElements ::= returnElement */
  {   72,   -1 }, /* (48) returnElement ::= arithmetic_expression */
  {   72,   -3 }, /* (49) returnElement ::= arithmetic_expression AS UQSTRING */
  {   61,   -3 }, /* (50) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   61,   -3 }, /* (51) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   61,   -3 }, /* (52) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   61,   -3 }, /* (53) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   61,   -3 }, /* (54) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   61,   -4 }, /* (55) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   61,   -1 }, /* (56) arithmetic_expression ::= value */
  {   61,   -1 }, /* (57) arithmetic_expression ::= variable */
  {   73,   -3 }, /* (58) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   73,   -1 }, /* (59) arithmetic_expression_list ::= arithmetic_expression */
  {   60,   -1 }, /* (60) variable ::= UQSTRING */
  {   60,   -3 }, /* (61) variable ::= UQSTRING DOT UQSTRING */
  {   51,    0 }, /* (62) orderClause ::= */
  {   51,   -3 }, /* (63) orderClause ::= ORDER BY columnNameList */
  {   51,   -4 }, /* (64) orderClause ::= ORDER BY columnNameList ASC */
  {   51,   -4 }, /* (65) orderClause ::= ORDER BY columnNameList DESC */
  {   74,   -3 }, /* (66) columnNameList ::= columnNameList COMMA columnName */
  {   74,   -1 }, /* (67) columnNameList ::= columnName */
  {   75,   -1 }, /* (68) columnName ::= variable */
  {   52,    0 }, /* (69) limitClause ::= */
  {   52,   -2 }, /* (70) limitClause ::= LIMIT INTEGER */
  {   70,   -1 }, /* (71) relation ::= EQ */
  {   70,   -1 }, /* (72) relation ::= GT */
  {   70,   -1 }, /* (73) relation ::= LT */
  {   70,   -1 }, /* (74) relation ::= LE */
  {   70,   -1 }, /* (75) relation ::= GE */
  {   70,   -1 }, /* (76) relation ::= NE */
  {   68,   -1 }, /* (77) value ::= INTEGER */
  {   68,   -2 }, /* (78) value ::= DASH INTEGER */
  {   68,   -1 }, /* (79) value ::= STRING */
  {   68,   -1 }, /* (80) value ::= FLOAT */
  {   68,   -2 }, /* (81) value ::= DASH FLOAT */
  {   68,   -1 }, /* (82) value ::= TRUE */
  {   68,   -1 }, /* (83) value ::= FALSE */
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
{ ctx->root = yymsp[0].minor.yy8; }
#line 1169 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
#line 38 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(yymsp[-5].minor.yy117, yymsp[-4].minor.yy67, yymsp[-3].minor.yy20, NULL, NULL, NULL, yymsp[-2].minor.yy112, yymsp[-1].minor.yy28, yymsp[0].minor.yy39);
}
#line 1176 "grammar.c"
  yymsp[-5].minor.yy8 = yylhsminor.yy8;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 42 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(yymsp[-2].minor.yy117, yymsp[-1].minor.yy67, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1184 "grammar.c"
  yymsp[-2].minor.yy8 = yylhsminor.yy8;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 46 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(yymsp[-2].minor.yy117, yymsp[-1].minor.yy67, NULL, NULL, NULL, yymsp[0].minor.yy40, NULL, NULL, NULL);
}
#line 1192 "grammar.c"
  yymsp[-2].minor.yy8 = yylhsminor.yy8;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 50 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(yymsp[-2].minor.yy117, yymsp[-1].minor.yy67, NULL, NULL, yymsp[0].minor.yy92, NULL, NULL, NULL, NULL);
}
#line 1200 "grammar.c"
  yymsp[-2].minor.yy8 = yylhsminor.yy8;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
#line 54 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(yymsp[-5].minor.yy117, yymsp[-4].minor.yy67, NULL, NULL, yymsp[-3].minor.yy92, NULL, yymsp[-2].minor.yy112, yymsp[-1].minor.yy28, yymsp[0].minor.yy39);
}
#line 1208 "grammar.c"
  yymsp[-5].minor.yy8 = yylhsminor.yy8;
        break;
      case 6: /* expr ::= createClause */
#line 58 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy20, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1216 "grammar.c"
  yymsp[0].minor.yy8 = yylhsminor.yy8;
        break;
      case 7: /* expr ::= mergeClause */
#line 62 "grammar.y"
{
	yylhsminor.yy8 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy124, NULL, NULL, NULL, NULL, NULL);
}
#line 1224 "grammar.c"
  yymsp[0].minor.yy8 = yylhsminor.yy8;
        break;
      case 8: /* matchClause ::= MATCH chains */
#line 68 "grammar.y"
{
	yymsp[-1].minor.yy117 = New_AST_MatchNode(yymsp[0].minor.yy2);
}
#line 1232 "grammar.c"
        break;
      case 9: /* createClause ::= */
#line 75 "grammar.y"
{
	yymsp[1].minor.yy20 = NULL;
}
#line 1239 "grammar.c"
        break;
      case 10: /* createClause ::= CREATE chains */
#line 79 "grammar.y"
{
	yymsp[-1].minor.yy20 = New_AST_CreateNode(yymsp[0].minor.yy2);
}
#line 1246 "grammar.c"
        break;
      case 11: /* mergeClause ::= MERGE chain */
#line 85 "grammar.y"
{
	yymsp[-1].minor.yy124 = New_AST_MergeNode(yymsp[0].minor.yy2);
}
#line 1253 "grammar.c"
        break;
      case 12: /* setClause ::= SET setList */
#line 90 "grammar.y"
{
	yymsp[-1].minor.yy92 = New_AST_SetNode(yymsp[0].minor.yy2);
}
#line 1260 "grammar.c"
        break;
      case 13: /* setList ::= setElement */
#line 95 "grammar.y"
{
	yylhsminor.yy2 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy48);
}
#line 1268 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 14: /* setList ::= setList COMMA setElement */
#line 99 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy48);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1277 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 15: /* setElement ::= variable EQ arithmetic_expression */
#line 105 "grammar.y"
{
	yylhsminor.yy48 = New_AST_SetElement(yymsp[-2].minor.yy44, yymsp[0].minor.yy3);
}
#line 1285 "grammar.c"
  yymsp[-2].minor.yy48 = yylhsminor.yy48;
        break;
      case 16: /* chain ::= node */
#line 111 "grammar.y"
{
	yylhsminor.yy2 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy13);
}
#line 1294 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 17: /* chain ::= chain link node */
#line 116 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[-1].minor.yy149);
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy13);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1304 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 18: /* chains ::= chain */
#line 124 "grammar.y"
{
	yylhsminor.yy2 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy2);
}
#line 1313 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 19: /* chains ::= chains COMMA chain */
#line 129 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy2);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1322 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 20: /* deleteClause ::= DELETE deleteExpression */
#line 137 "grammar.y"
{
	yymsp[-1].minor.yy40 = New_AST_DeleteNode(yymsp[0].minor.yy2);
}
#line 1330 "grammar.c"
        break;
      case 21: /* deleteExpression ::= UQSTRING */
#line 143 "grammar.y"
{
	yylhsminor.yy2 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy0.strval);
}
#line 1338 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 22: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 148 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy0.strval);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1347 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 23: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 156 "grammar.y"
{
	yymsp[-5].minor.yy13 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy2);
}
#line 1355 "grammar.c"
        break;
      case 24: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 161 "grammar.y"
{
	yymsp[-4].minor.yy13 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy2);
}
#line 1362 "grammar.c"
        break;
      case 25: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 166 "grammar.y"
{
	yymsp[-3].minor.yy13 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy2);
}
#line 1369 "grammar.c"
        break;
      case 26: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 171 "grammar.y"
{
	yymsp[-2].minor.yy13 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy2);
}
#line 1376 "grammar.c"
        break;
      case 27: /* link ::= DASH edge RIGHT_ARROW */
#line 178 "grammar.y"
{
	yymsp[-2].minor.yy149 = yymsp[-1].minor.yy149;
	yymsp[-2].minor.yy149->direction = N_LEFT_TO_RIGHT;
}
#line 1384 "grammar.c"
        break;
      case 28: /* link ::= LEFT_ARROW edge DASH */
#line 184 "grammar.y"
{
	yymsp[-2].minor.yy149 = yymsp[-1].minor.yy149;
	yymsp[-2].minor.yy149->direction = N_RIGHT_TO_LEFT;
}
#line 1392 "grammar.c"
        break;
      case 29: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 191 "grammar.y"
{ 
	yymsp[-2].minor.yy149 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy2, N_DIR_UNKNOWN);
}
#line 1399 "grammar.c"
        break;
      case 30: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 196 "grammar.y"
{ 
	yymsp[-3].minor.yy149 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy2, N_DIR_UNKNOWN);
}
#line 1406 "grammar.c"
        break;
      case 31: /* edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
#line 201 "grammar.y"
{ 
	yymsp[-4].minor.yy149 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy2, N_DIR_UNKNOWN);
}
#line 1413 "grammar.c"
        break;
      case 32: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 206 "grammar.y"
{ 
	yymsp[-5].minor.yy149 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy2, N_DIR_UNKNOWN);
}
#line 1420 "grammar.c"
        break;
      case 33: /* properties ::= */
#line 212 "grammar.y"
{
	yymsp[1].minor.yy2 = NULL;
}
#line 1427 "grammar.c"
        break;
      case 34: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 216 "grammar.y"
{
	yymsp[-2].minor.yy2 = yymsp[-1].minor.yy2;
}
#line 1434 "grammar.c"
        break;
      case 35: /* mapLiteral ::= UQSTRING COLON value */
#line 222 "grammar.y"
{
	yylhsminor.yy2 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy2, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy150;
	Vector_Push(yylhsminor.yy2, val);
}
#line 1449 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 36: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 234 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy2, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy150;
	Vector_Push(yymsp[0].minor.yy2, val);
	
	yylhsminor.yy2 = yymsp[0].minor.yy2;
}
#line 1465 "grammar.c"
  yymsp[-4].minor.yy2 = yylhsminor.yy2;
        break;
      case 37: /* whereClause ::= */
#line 248 "grammar.y"
{ 
	yymsp[1].minor.yy67 = NULL;
}
#line 1473 "grammar.c"
        break;
      case 38: /* whereClause ::= WHERE cond */
#line 251 "grammar.y"
{
	yymsp[-1].minor.yy67 = New_AST_WhereNode(yymsp[0].minor.yy10);
}
#line 1480 "grammar.c"
        break;
      case 39: /* cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
#line 260 "grammar.y"
{ yylhsminor.yy10 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy84, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1485 "grammar.c"
  yymsp[-6].minor.yy10 = yylhsminor.yy10;
        break;
      case 40: /* cond ::= UQSTRING DOT UQSTRING relation value */
#line 263 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy84, yymsp[0].minor.yy150); }
#line 1491 "grammar.c"
  yymsp[-4].minor.yy10 = yylhsminor.yy10;
        break;
      case 41: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 264 "grammar.y"
{ yymsp[-2].minor.yy10 = yymsp[-1].minor.yy10; }
#line 1497 "grammar.c"
        break;
      case 42: /* cond ::= cond AND cond */
#line 265 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, AND, yymsp[0].minor.yy10); }
#line 1502 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 43: /* cond ::= cond OR cond */
#line 266 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, OR, yymsp[0].minor.yy10); }
#line 1508 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 44: /* returnClause ::= RETURN returnElements */
#line 271 "grammar.y"
{
	yymsp[-1].minor.yy112 = New_AST_ReturnNode(yymsp[0].minor.yy2, 0);
}
#line 1516 "grammar.c"
        break;
      case 45: /* returnClause ::= RETURN DISTINCT returnElements */
#line 274 "grammar.y"
{
	yymsp[-2].minor.yy112 = New_AST_ReturnNode(yymsp[0].minor.yy2, 1);
}
#line 1523 "grammar.c"
        break;
      case 46: /* returnElements ::= returnElements COMMA returnElement */
#line 281 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy34);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1531 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 47: /* returnElements ::= returnElement */
#line 286 "grammar.y"
{
	yylhsminor.yy2 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy34);
}
#line 1540 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 48: /* returnElement ::= arithmetic_expression */
#line 293 "grammar.y"
{
	yylhsminor.yy34 = New_AST_ReturnElementNode(yymsp[0].minor.yy3, NULL);
}
#line 1548 "grammar.c"
  yymsp[0].minor.yy34 = yylhsminor.yy34;
        break;
      case 49: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 298 "grammar.y"
{
	yylhsminor.yy34 = New_AST_ReturnElementNode(yymsp[-2].minor.yy3, yymsp[0].minor.yy0.strval);
}
#line 1556 "grammar.c"
  yymsp[-2].minor.yy34 = yylhsminor.yy34;
        break;
      case 50: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 305 "grammar.y"
{
	yymsp[-2].minor.yy3 = yymsp[-1].minor.yy3;
}
#line 1564 "grammar.c"
        break;
      case 51: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 317 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy3);
	Vector_Push(args, yymsp[0].minor.yy3);
	yylhsminor.yy3 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1574 "grammar.c"
  yymsp[-2].minor.yy3 = yylhsminor.yy3;
        break;
      case 52: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 324 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy3);
	Vector_Push(args, yymsp[0].minor.yy3);
	yylhsminor.yy3 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1585 "grammar.c"
  yymsp[-2].minor.yy3 = yylhsminor.yy3;
        break;
      case 53: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 331 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy3);
	Vector_Push(args, yymsp[0].minor.yy3);
	yylhsminor.yy3 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1596 "grammar.c"
  yymsp[-2].minor.yy3 = yylhsminor.yy3;
        break;
      case 54: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 338 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy3);
	Vector_Push(args, yymsp[0].minor.yy3);
	yylhsminor.yy3 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1607 "grammar.c"
  yymsp[-2].minor.yy3 = yylhsminor.yy3;
        break;
      case 55: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 346 "grammar.y"
{
	yylhsminor.yy3 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy2);
}
#line 1615 "grammar.c"
  yymsp[-3].minor.yy3 = yylhsminor.yy3;
        break;
      case 56: /* arithmetic_expression ::= value */
#line 351 "grammar.y"
{
	yylhsminor.yy3 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy150);
}
#line 1623 "grammar.c"
  yymsp[0].minor.yy3 = yylhsminor.yy3;
        break;
      case 57: /* arithmetic_expression ::= variable */
#line 356 "grammar.y"
{
	yylhsminor.yy3 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy44->alias, yymsp[0].minor.yy44->property);
}
#line 1631 "grammar.c"
  yymsp[0].minor.yy3 = yylhsminor.yy3;
        break;
      case 58: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 362 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy3);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1640 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 59: /* arithmetic_expression_list ::= arithmetic_expression */
#line 366 "grammar.y"
{
	yylhsminor.yy2 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy3);
}
#line 1649 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 60: /* variable ::= UQSTRING */
#line 373 "grammar.y"
{
	yylhsminor.yy44 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1657 "grammar.c"
  yymsp[0].minor.yy44 = yylhsminor.yy44;
        break;
      case 61: /* variable ::= UQSTRING DOT UQSTRING */
#line 377 "grammar.y"
{
	yylhsminor.yy44 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1665 "grammar.c"
  yymsp[-2].minor.yy44 = yylhsminor.yy44;
        break;
      case 62: /* orderClause ::= */
#line 383 "grammar.y"
{
	yymsp[1].minor.yy28 = NULL;
}
#line 1673 "grammar.c"
        break;
      case 63: /* orderClause ::= ORDER BY columnNameList */
#line 386 "grammar.y"
{
	yymsp[-2].minor.yy28 = New_AST_OrderNode(yymsp[0].minor.yy2, ORDER_DIR_ASC);
}
#line 1680 "grammar.c"
        break;
      case 64: /* orderClause ::= ORDER BY columnNameList ASC */
#line 389 "grammar.y"
{
	yymsp[-3].minor.yy28 = New_AST_OrderNode(yymsp[-1].minor.yy2, ORDER_DIR_ASC);
}
#line 1687 "grammar.c"
        break;
      case 65: /* orderClause ::= ORDER BY columnNameList DESC */
#line 392 "grammar.y"
{
	yymsp[-3].minor.yy28 = New_AST_OrderNode(yymsp[-1].minor.yy2, ORDER_DIR_DESC);
}
#line 1694 "grammar.c"
        break;
      case 66: /* columnNameList ::= columnNameList COMMA columnName */
#line 397 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy2, yymsp[0].minor.yy14);
	yylhsminor.yy2 = yymsp[-2].minor.yy2;
}
#line 1702 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 67: /* columnNameList ::= columnName */
#line 401 "grammar.y"
{
	yylhsminor.yy2 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy2, yymsp[0].minor.yy14);
}
#line 1711 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 68: /* columnName ::= variable */
#line 407 "grammar.y"
{
	if(yymsp[0].minor.yy44->property != NULL) {
		yylhsminor.yy14 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy44);
	} else {
		yylhsminor.yy14 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy44->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy44);
}
#line 1725 "grammar.c"
  yymsp[0].minor.yy14 = yylhsminor.yy14;
        break;
      case 69: /* limitClause ::= */
#line 419 "grammar.y"
{
	yymsp[1].minor.yy39 = NULL;
}
#line 1733 "grammar.c"
        break;
      case 70: /* limitClause ::= LIMIT INTEGER */
#line 422 "grammar.y"
{
	yymsp[-1].minor.yy39 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1740 "grammar.c"
        break;
      case 71: /* relation ::= EQ */
#line 428 "grammar.y"
{ yymsp[0].minor.yy84 = EQ; }
#line 1745 "grammar.c"
        break;
      case 72: /* relation ::= GT */
#line 429 "grammar.y"
{ yymsp[0].minor.yy84 = GT; }
#line 1750 "grammar.c"
        break;
      case 73: /* relation ::= LT */
#line 430 "grammar.y"
{ yymsp[0].minor.yy84 = LT; }
#line 1755 "grammar.c"
        break;
      case 74: /* relation ::= LE */
#line 431 "grammar.y"
{ yymsp[0].minor.yy84 = LE; }
#line 1760 "grammar.c"
        break;
      case 75: /* relation ::= GE */
#line 432 "grammar.y"
{ yymsp[0].minor.yy84 = GE; }
#line 1765 "grammar.c"
        break;
      case 76: /* relation ::= NE */
#line 433 "grammar.y"
{ yymsp[0].minor.yy84 = NE; }
#line 1770 "grammar.c"
        break;
      case 77: /* value ::= INTEGER */
#line 444 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1775 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 78: /* value ::= DASH INTEGER */
#line 445 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1781 "grammar.c"
        break;
      case 79: /* value ::= STRING */
#line 446 "grammar.y"
{  yylhsminor.yy150 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1786 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 80: /* value ::= FLOAT */
#line 447 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1792 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 81: /* value ::= DASH FLOAT */
#line 448 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1798 "grammar.c"
        break;
      case 82: /* value ::= TRUE */
#line 449 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(1); }
#line 1803 "grammar.c"
        break;
      case 83: /* value ::= FALSE */
#line 450 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(0); }
#line 1808 "grammar.c"
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
#line 1873 "grammar.c"
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
#line 452 "grammar.y"


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
#line 2119 "grammar.c"
