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
#define YYNOCODE 78
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_ArithmeticExpressionNode* yy2;
  SIValue yy22;
  AST_DeleteNode * yy31;
  AST_SetNode* yy34;
  AST_Variable* yy50;
  AST_FilterNode* yy60;
  AST_Query* yy62;
  Vector* yy64;
  AST_NodeEntity* yy69;
  AST_MatchNode* yy89;
  AST_ReturnNode* yy92;
  AST_ColumnNode* yy120;
  AST_WhereNode* yy127;
  int yy130;
  AST_MergeNode* yy132;
  AST_CreateNode* yy140;
  AST_OrderNode* yy141;
  AST_ReturnElementNode* yy148;
  AST_LimitNode* yy149;
  AST_LinkEntity* yy150;
  AST_SetElement* yy154;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             101
#define YYNRULE              86
#define YYNTOKEN             45
#define YY_MAX_SHIFT         100
#define YY_MIN_SHIFTREDUCE   159
#define YY_MAX_SHIFTREDUCE   244
#define YY_ERROR_ACTION      245
#define YY_ACCEPT_ACTION     246
#define YY_NO_ACTION         247
#define YY_MIN_REDUCE        248
#define YY_MAX_REDUCE        333
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
#define YY_ACTTAB_COUNT (249)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   289,  100,  246,   51,   61,  254,  256,   87,   50,  306,
 /*    10 */    53,  255,  251,   47,  306,   53,   39,  305,   63,   10,
 /*    20 */    95,  296,  305,   61,   25,   93,  296,   11,    9,    8,
 /*    30 */     7,    2,   97,   65,   31,   36,   61,   68,  237,  265,
 /*    40 */   239,  240,  242,  243,  244,   61,   11,    9,    8,    7,
 /*    50 */    63,   10,   11,    9,    8,    7,   94,  237,   20,  239,
 /*    60 */   240,  242,  243,  244,  210,  231,  232,  235,  233,  234,
 /*    70 */   237,   44,  239,  240,  242,  243,  244,  224,  225,  237,
 /*    80 */    21,  239,  240,  242,  243,  244,   13,   15,   17,  238,
 /*    90 */   306,   56,  241,   74,  262,   73,   81,  236,  305,  306,
 /*   100 */    53,   27,   29,   62,    1,  306,   54,  305,  306,   55,
 /*   110 */    66,  295,   83,  305,  306,  303,  305,  306,  302,  263,
 /*   120 */    73,  201,  305,  306,   64,  305,  306,   52,  306,   60,
 /*   130 */   317,  305,   92,   31,  305,   26,  305,   15,  265,   14,
 /*   140 */   265,   35,   57,  317,   58,  316,   32,   33,   77,   85,
 /*   150 */    70,   28,   34,   79,   42,   42,   91,  265,  315,   42,
 /*   160 */    27,   29,   42,    8,    7,    6,    3,  291,   59,   80,
 /*   170 */   215,   86,  181,   72,   24,   76,   96,   75,   42,   78,
 /*   180 */    88,   82,   84,  285,   90,   89,  266,  253,   99,   98,
 /*   190 */    45,    1,   46,  249,   48,   49,   30,   12,   19,   29,
 /*   200 */    67,  199,   23,   69,  182,   71,    5,   18,  188,  186,
 /*   210 */   191,   37,  192,   38,   40,  190,  187,  189,   41,  184,
 /*   220 */   248,  185,  247,   22,   43,    4,  183,  247,  194,  209,
 /*   230 */   221,  247,  247,  247,  247,  247,   96,  247,  247,  247,
 /*   240 */   247,  247,   16,  247,  247,  247,  247,  247,  230,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    69,   46,   47,   48,    4,   50,   51,   69,   50,   61,
 /*    10 */    62,   56,   54,   55,   61,   62,    4,   69,   18,   19,
 /*    20 */    72,   73,   69,    4,   19,   72,   73,    3,    4,    5,
 /*    30 */     6,   31,   18,   57,   58,   23,    4,   18,   38,   63,
 /*    40 */    40,   41,   42,   43,   44,    4,    3,    4,    5,    6,
 /*    50 */    18,   19,    3,    4,    5,    6,   32,   38,   16,   40,
 /*    60 */    41,   42,   43,   44,   21,    7,    8,    9,   10,   11,
 /*    70 */    38,   64,   40,   41,   42,   43,   44,   35,   36,   38,
 /*    80 */    71,   40,   41,   42,   43,   44,   12,   13,   14,   38,
 /*    90 */    61,   62,   41,   59,   60,   61,   66,   39,   69,   61,
 /*   100 */    62,    1,    2,   74,   30,   61,   62,   69,   61,   62,
 /*   110 */    70,   73,   66,   69,   61,   62,   69,   61,   62,   60,
 /*   120 */    61,   21,   69,   61,   62,   69,   61,   62,   61,   62,
 /*   130 */    61,   69,   57,   58,   69,   58,   69,   13,   63,   15,
 /*   140 */    63,   17,   70,   61,   75,   76,   18,   18,   20,   20,
 /*   150 */    18,   19,   58,   20,   26,   26,   20,   63,   76,   26,
 /*   160 */     1,    2,   26,    5,    6,   16,   19,   70,   70,   66,
 /*   170 */    21,   66,   18,   65,   24,   66,   29,   67,   26,   66,
 /*   180 */    18,   67,   66,   68,   66,   68,   63,   53,   37,   33,
 /*   190 */    52,   30,   51,   53,   52,   51,   28,   49,   16,    2,
 /*   200 */    29,   18,   18,   29,   18,   16,    7,   16,    4,   21,
 /*   210 */    25,   18,   25,   18,   18,   25,   22,   25,   16,   21,
 /*   220 */     0,   21,   77,   20,   18,   16,   21,   77,   27,   18,
 /*   230 */    18,   77,   77,   77,   77,   77,   29,   77,   77,   77,
 /*   240 */    77,   77,   34,   77,   77,   77,   77,   77,   38,   77,
 /*   250 */    77,   77,   77,   77,   77,   77,   77,   77,   77,   77,
 /*   260 */    77,   77,   77,   77,   77,   77,   77,   77,   77,   77,
 /*   270 */    77,   77,   77,   77,   77,   77,   77,   77,   77,   77,
 /*   280 */    77,   77,   77,   77,   77,   77,   77,   77,   77,   77,
 /*   290 */    77,   77,   77,   77,
};
#define YY_SHIFT_COUNT    (100)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (220)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    74,    0,   32,   32,   32,   32,   32,   32,   32,   32,
 /*    10 */    32,   32,  124,    5,   14,    5,   14,    5,   14,    5,
 /*    20 */    14,   19,   41,   58,  128,  129,   12,  132,  132,  132,
 /*    30 */   132,   12,  133,  136,   12,  154,  150,  152,  152,  150,
 /*    40 */   152,  162,  162,  152,    5,  151,  156,  161,  151,  156,
 /*    50 */   161,  168,   43,   24,   49,   49,   49,  100,   42,  159,
 /*    60 */   158,   51,  149,  147,  158,  182,  197,  183,  171,  184,
 /*    70 */   174,  186,  189,  199,  191,  204,  185,  193,  187,  195,
 /*    80 */   190,  192,  194,  188,  198,  196,  200,  202,  203,  201,
 /*    90 */   205,  206,  182,  209,  211,  209,  212,  207,  208,  210,
 /*   100 */   220,
};
#define YY_REDUCE_COUNT (51)
#define YY_REDUCE_MIN   (-69)
#define YY_REDUCE_MAX   (148)
static const short yy_reduce_ofst[] = {
 /*     0 */   -45,  -52,  -47,   29,   38,   44,   47,   53,   56,   62,
 /*    10 */    65,   67,  -42,  -24,   34,   75,   69,   77,   59,   94,
 /*    20 */    82,  -69,  -62,    9,   30,   46,    7,   40,   72,   97,
 /*    30 */    98,    7,  103,  105,    7,  108,  110,  109,  113,  114,
 /*    40 */   116,  115,  117,  118,  123,  134,  138,  141,  140,  142,
 /*    50 */   144,  148,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   258,  245,  245,  245,  245,  245,  245,  245,  245,  245,
 /*    10 */   245,  245,  258,  245,  245,  245,  245,  245,  245,  245,
 /*    20 */   245,  245,  245,  245,  282,  282,  260,  245,  245,  245,
 /*    30 */   245,  267,  282,  282,  268,  245,  245,  282,  282,  245,
 /*    40 */   282,  245,  245,  282,  245,  318,  311,  252,  318,  311,
 /*    50 */   250,  286,  245,  297,  264,  307,  308,  245,  312,  287,
 /*    60 */   300,  245,  245,  309,  301,  257,  292,  245,  245,  245,
 /*    70 */   245,  245,  269,  245,  261,  245,  245,  245,  245,  245,
 /*    80 */   245,  245,  245,  245,  245,  245,  245,  284,  245,  245,
 /*    90 */   245,  245,  259,  294,  245,  293,  245,  309,  245,  245,
 /*   100 */   245,
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
  /*   44 */ "NULLVAL",
  /*   45 */ "error",
  /*   46 */ "expr",
  /*   47 */ "query",
  /*   48 */ "matchClause",
  /*   49 */ "whereClause",
  /*   50 */ "createClause",
  /*   51 */ "returnClause",
  /*   52 */ "orderClause",
  /*   53 */ "limitClause",
  /*   54 */ "deleteClause",
  /*   55 */ "setClause",
  /*   56 */ "mergeClause",
  /*   57 */ "chains",
  /*   58 */ "chain",
  /*   59 */ "setList",
  /*   60 */ "setElement",
  /*   61 */ "variable",
  /*   62 */ "arithmetic_expression",
  /*   63 */ "node",
  /*   64 */ "link",
  /*   65 */ "deleteExpression",
  /*   66 */ "properties",
  /*   67 */ "edge",
  /*   68 */ "mapLiteral",
  /*   69 */ "value",
  /*   70 */ "cond",
  /*   71 */ "relation",
  /*   72 */ "returnElements",
  /*   73 */ "returnElement",
  /*   74 */ "arithmetic_expression_list",
  /*   75 */ "columnNameList",
  /*   76 */ "columnName",
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
 /*   8 */ "expr ::= returnClause",
 /*   9 */ "matchClause ::= MATCH chains",
 /*  10 */ "createClause ::=",
 /*  11 */ "createClause ::= CREATE chains",
 /*  12 */ "mergeClause ::= MERGE chain",
 /*  13 */ "setClause ::= SET setList",
 /*  14 */ "setList ::= setElement",
 /*  15 */ "setList ::= setList COMMA setElement",
 /*  16 */ "setElement ::= variable EQ arithmetic_expression",
 /*  17 */ "chain ::= node",
 /*  18 */ "chain ::= chain link node",
 /*  19 */ "chains ::= chain",
 /*  20 */ "chains ::= chains COMMA chain",
 /*  21 */ "deleteClause ::= DELETE deleteExpression",
 /*  22 */ "deleteExpression ::= UQSTRING",
 /*  23 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  24 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  25 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  26 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  27 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  28 */ "link ::= DASH edge RIGHT_ARROW",
 /*  29 */ "link ::= LEFT_ARROW edge DASH",
 /*  30 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  31 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  32 */ "edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET",
 /*  33 */ "edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET",
 /*  34 */ "properties ::=",
 /*  35 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  36 */ "mapLiteral ::= UQSTRING COLON value",
 /*  37 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  38 */ "whereClause ::=",
 /*  39 */ "whereClause ::= WHERE cond",
 /*  40 */ "cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING",
 /*  41 */ "cond ::= UQSTRING DOT UQSTRING relation value",
 /*  42 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  43 */ "cond ::= cond AND cond",
 /*  44 */ "cond ::= cond OR cond",
 /*  45 */ "returnClause ::= RETURN returnElements",
 /*  46 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  47 */ "returnElements ::= returnElements COMMA returnElement",
 /*  48 */ "returnElements ::= returnElement",
 /*  49 */ "returnElement ::= arithmetic_expression",
 /*  50 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  51 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  52 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  53 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  54 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  55 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  56 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  57 */ "arithmetic_expression ::= value",
 /*  58 */ "arithmetic_expression ::= variable",
 /*  59 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  60 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  61 */ "variable ::= UQSTRING",
 /*  62 */ "variable ::= UQSTRING DOT UQSTRING",
 /*  63 */ "orderClause ::=",
 /*  64 */ "orderClause ::= ORDER BY columnNameList",
 /*  65 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  66 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  67 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  68 */ "columnNameList ::= columnName",
 /*  69 */ "columnName ::= variable",
 /*  70 */ "limitClause ::=",
 /*  71 */ "limitClause ::= LIMIT INTEGER",
 /*  72 */ "relation ::= EQ",
 /*  73 */ "relation ::= GT",
 /*  74 */ "relation ::= LT",
 /*  75 */ "relation ::= LE",
 /*  76 */ "relation ::= GE",
 /*  77 */ "relation ::= NE",
 /*  78 */ "value ::= INTEGER",
 /*  79 */ "value ::= DASH INTEGER",
 /*  80 */ "value ::= STRING",
 /*  81 */ "value ::= FLOAT",
 /*  82 */ "value ::= DASH FLOAT",
 /*  83 */ "value ::= TRUE",
 /*  84 */ "value ::= FALSE",
 /*  85 */ "value ::= NULLVAL",
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
    case 70: /* cond */
{
#line 261 "grammar.y"
 Free_AST_FilterNode((yypminor->yy60)); 
#line 712 "grammar.c"
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
  {   47,   -1 }, /* (0) query ::= expr */
  {   46,   -6 }, /* (1) expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
  {   46,   -3 }, /* (2) expr ::= matchClause whereClause createClause */
  {   46,   -3 }, /* (3) expr ::= matchClause whereClause deleteClause */
  {   46,   -3 }, /* (4) expr ::= matchClause whereClause setClause */
  {   46,   -6 }, /* (5) expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
  {   46,   -1 }, /* (6) expr ::= createClause */
  {   46,   -1 }, /* (7) expr ::= mergeClause */
  {   46,   -1 }, /* (8) expr ::= returnClause */
  {   48,   -2 }, /* (9) matchClause ::= MATCH chains */
  {   50,    0 }, /* (10) createClause ::= */
  {   50,   -2 }, /* (11) createClause ::= CREATE chains */
  {   56,   -2 }, /* (12) mergeClause ::= MERGE chain */
  {   55,   -2 }, /* (13) setClause ::= SET setList */
  {   59,   -1 }, /* (14) setList ::= setElement */
  {   59,   -3 }, /* (15) setList ::= setList COMMA setElement */
  {   60,   -3 }, /* (16) setElement ::= variable EQ arithmetic_expression */
  {   58,   -1 }, /* (17) chain ::= node */
  {   58,   -3 }, /* (18) chain ::= chain link node */
  {   57,   -1 }, /* (19) chains ::= chain */
  {   57,   -3 }, /* (20) chains ::= chains COMMA chain */
  {   54,   -2 }, /* (21) deleteClause ::= DELETE deleteExpression */
  {   65,   -1 }, /* (22) deleteExpression ::= UQSTRING */
  {   65,   -3 }, /* (23) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   63,   -6 }, /* (24) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   63,   -5 }, /* (25) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   63,   -4 }, /* (26) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   63,   -3 }, /* (27) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   64,   -3 }, /* (28) link ::= DASH edge RIGHT_ARROW */
  {   64,   -3 }, /* (29) link ::= LEFT_ARROW edge DASH */
  {   67,   -3 }, /* (30) edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
  {   67,   -4 }, /* (31) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   67,   -5 }, /* (32) edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
  {   67,   -6 }, /* (33) edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
  {   66,    0 }, /* (34) properties ::= */
  {   66,   -3 }, /* (35) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   68,   -3 }, /* (36) mapLiteral ::= UQSTRING COLON value */
  {   68,   -5 }, /* (37) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   49,    0 }, /* (38) whereClause ::= */
  {   49,   -2 }, /* (39) whereClause ::= WHERE cond */
  {   70,   -7 }, /* (40) cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
  {   70,   -5 }, /* (41) cond ::= UQSTRING DOT UQSTRING relation value */
  {   70,   -3 }, /* (42) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   70,   -3 }, /* (43) cond ::= cond AND cond */
  {   70,   -3 }, /* (44) cond ::= cond OR cond */
  {   51,   -2 }, /* (45) returnClause ::= RETURN returnElements */
  {   51,   -3 }, /* (46) returnClause ::= RETURN DISTINCT returnElements */
  {   72,   -3 }, /* (47) returnElements ::= returnElements COMMA returnElement */
  {   72,   -1 }, /* (48) returnElements ::= returnElement */
  {   73,   -1 }, /* (49) returnElement ::= arithmetic_expression */
  {   73,   -3 }, /* (50) returnElement ::= arithmetic_expression AS UQSTRING */
  {   62,   -3 }, /* (51) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   62,   -3 }, /* (52) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   62,   -3 }, /* (53) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   62,   -3 }, /* (54) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   62,   -3 }, /* (55) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   62,   -4 }, /* (56) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   62,   -1 }, /* (57) arithmetic_expression ::= value */
  {   62,   -1 }, /* (58) arithmetic_expression ::= variable */
  {   74,   -3 }, /* (59) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {   74,   -1 }, /* (60) arithmetic_expression_list ::= arithmetic_expression */
  {   61,   -1 }, /* (61) variable ::= UQSTRING */
  {   61,   -3 }, /* (62) variable ::= UQSTRING DOT UQSTRING */
  {   52,    0 }, /* (63) orderClause ::= */
  {   52,   -3 }, /* (64) orderClause ::= ORDER BY columnNameList */
  {   52,   -4 }, /* (65) orderClause ::= ORDER BY columnNameList ASC */
  {   52,   -4 }, /* (66) orderClause ::= ORDER BY columnNameList DESC */
  {   75,   -3 }, /* (67) columnNameList ::= columnNameList COMMA columnName */
  {   75,   -1 }, /* (68) columnNameList ::= columnName */
  {   76,   -1 }, /* (69) columnName ::= variable */
  {   53,    0 }, /* (70) limitClause ::= */
  {   53,   -2 }, /* (71) limitClause ::= LIMIT INTEGER */
  {   71,   -1 }, /* (72) relation ::= EQ */
  {   71,   -1 }, /* (73) relation ::= GT */
  {   71,   -1 }, /* (74) relation ::= LT */
  {   71,   -1 }, /* (75) relation ::= LE */
  {   71,   -1 }, /* (76) relation ::= GE */
  {   71,   -1 }, /* (77) relation ::= NE */
  {   69,   -1 }, /* (78) value ::= INTEGER */
  {   69,   -2 }, /* (79) value ::= DASH INTEGER */
  {   69,   -1 }, /* (80) value ::= STRING */
  {   69,   -1 }, /* (81) value ::= FLOAT */
  {   69,   -2 }, /* (82) value ::= DASH FLOAT */
  {   69,   -1 }, /* (83) value ::= TRUE */
  {   69,   -1 }, /* (84) value ::= FALSE */
  {   69,   -1 }, /* (85) value ::= NULLVAL */
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
{ ctx->root = yymsp[0].minor.yy62; }
#line 1175 "grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
#line 38 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(yymsp[-5].minor.yy89, yymsp[-4].minor.yy127, yymsp[-3].minor.yy140, NULL, NULL, NULL, yymsp[-2].minor.yy92, yymsp[-1].minor.yy141, yymsp[0].minor.yy149);
}
#line 1182 "grammar.c"
  yymsp[-5].minor.yy62 = yylhsminor.yy62;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 42 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(yymsp[-2].minor.yy89, yymsp[-1].minor.yy127, yymsp[0].minor.yy140, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1190 "grammar.c"
  yymsp[-2].minor.yy62 = yylhsminor.yy62;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 46 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(yymsp[-2].minor.yy89, yymsp[-1].minor.yy127, NULL, NULL, NULL, yymsp[0].minor.yy31, NULL, NULL, NULL);
}
#line 1198 "grammar.c"
  yymsp[-2].minor.yy62 = yylhsminor.yy62;
        break;
      case 4: /* expr ::= matchClause whereClause setClause */
#line 50 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(yymsp[-2].minor.yy89, yymsp[-1].minor.yy127, NULL, NULL, yymsp[0].minor.yy34, NULL, NULL, NULL, NULL);
}
#line 1206 "grammar.c"
  yymsp[-2].minor.yy62 = yylhsminor.yy62;
        break;
      case 5: /* expr ::= matchClause whereClause setClause returnClause orderClause limitClause */
#line 54 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(yymsp[-5].minor.yy89, yymsp[-4].minor.yy127, NULL, NULL, yymsp[-3].minor.yy34, NULL, yymsp[-2].minor.yy92, yymsp[-1].minor.yy141, yymsp[0].minor.yy149);
}
#line 1214 "grammar.c"
  yymsp[-5].minor.yy62 = yylhsminor.yy62;
        break;
      case 6: /* expr ::= createClause */
#line 58 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(NULL, NULL, yymsp[0].minor.yy140, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1222 "grammar.c"
  yymsp[0].minor.yy62 = yylhsminor.yy62;
        break;
      case 7: /* expr ::= mergeClause */
#line 62 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(NULL, NULL, NULL, yymsp[0].minor.yy132, NULL, NULL, NULL, NULL, NULL);
}
#line 1230 "grammar.c"
  yymsp[0].minor.yy62 = yylhsminor.yy62;
        break;
      case 8: /* expr ::= returnClause */
#line 66 "grammar.y"
{
	yylhsminor.yy62 = New_AST_Query(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy92, NULL, NULL);
}
#line 1238 "grammar.c"
  yymsp[0].minor.yy62 = yylhsminor.yy62;
        break;
      case 9: /* matchClause ::= MATCH chains */
#line 72 "grammar.y"
{
	yymsp[-1].minor.yy89 = New_AST_MatchNode(yymsp[0].minor.yy64);
}
#line 1246 "grammar.c"
        break;
      case 10: /* createClause ::= */
#line 79 "grammar.y"
{
	yymsp[1].minor.yy140 = NULL;
}
#line 1253 "grammar.c"
        break;
      case 11: /* createClause ::= CREATE chains */
#line 83 "grammar.y"
{
	yymsp[-1].minor.yy140 = New_AST_CreateNode(yymsp[0].minor.yy64);
}
#line 1260 "grammar.c"
        break;
      case 12: /* mergeClause ::= MERGE chain */
#line 89 "grammar.y"
{
	yymsp[-1].minor.yy132 = New_AST_MergeNode(yymsp[0].minor.yy64);
}
#line 1267 "grammar.c"
        break;
      case 13: /* setClause ::= SET setList */
#line 94 "grammar.y"
{
	yymsp[-1].minor.yy34 = New_AST_SetNode(yymsp[0].minor.yy64);
}
#line 1274 "grammar.c"
        break;
      case 14: /* setList ::= setElement */
#line 99 "grammar.y"
{
	yylhsminor.yy64 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy154);
}
#line 1282 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 15: /* setList ::= setList COMMA setElement */
#line 103 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy154);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1291 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 16: /* setElement ::= variable EQ arithmetic_expression */
#line 109 "grammar.y"
{
	yylhsminor.yy154 = New_AST_SetElement(yymsp[-2].minor.yy50, yymsp[0].minor.yy2);
}
#line 1299 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 17: /* chain ::= node */
#line 115 "grammar.y"
{
	yylhsminor.yy64 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy69);
}
#line 1308 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 18: /* chain ::= chain link node */
#line 120 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[-1].minor.yy150);
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy69);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1318 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 19: /* chains ::= chain */
#line 128 "grammar.y"
{
	yylhsminor.yy64 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy64);
}
#line 1327 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 20: /* chains ::= chains COMMA chain */
#line 133 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy64);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1336 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 21: /* deleteClause ::= DELETE deleteExpression */
#line 141 "grammar.y"
{
	yymsp[-1].minor.yy31 = New_AST_DeleteNode(yymsp[0].minor.yy64);
}
#line 1344 "grammar.c"
        break;
      case 22: /* deleteExpression ::= UQSTRING */
#line 147 "grammar.y"
{
	yylhsminor.yy64 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy0.strval);
}
#line 1352 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 23: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 152 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy0.strval);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1361 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 24: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 160 "grammar.y"
{
	yymsp[-5].minor.yy69 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy64);
}
#line 1369 "grammar.c"
        break;
      case 25: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 165 "grammar.y"
{
	yymsp[-4].minor.yy69 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy64);
}
#line 1376 "grammar.c"
        break;
      case 26: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 170 "grammar.y"
{
	yymsp[-3].minor.yy69 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy64);
}
#line 1383 "grammar.c"
        break;
      case 27: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 175 "grammar.y"
{
	yymsp[-2].minor.yy69 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy64);
}
#line 1390 "grammar.c"
        break;
      case 28: /* link ::= DASH edge RIGHT_ARROW */
#line 182 "grammar.y"
{
	yymsp[-2].minor.yy150 = yymsp[-1].minor.yy150;
	yymsp[-2].minor.yy150->direction = N_LEFT_TO_RIGHT;
}
#line 1398 "grammar.c"
        break;
      case 29: /* link ::= LEFT_ARROW edge DASH */
#line 188 "grammar.y"
{
	yymsp[-2].minor.yy150 = yymsp[-1].minor.yy150;
	yymsp[-2].minor.yy150->direction = N_RIGHT_TO_LEFT;
}
#line 1406 "grammar.c"
        break;
      case 30: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 195 "grammar.y"
{ 
	yymsp[-2].minor.yy150 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy64, N_DIR_UNKNOWN);
}
#line 1413 "grammar.c"
        break;
      case 31: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 200 "grammar.y"
{ 
	yymsp[-3].minor.yy150 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy64, N_DIR_UNKNOWN);
}
#line 1420 "grammar.c"
        break;
      case 32: /* edge ::= LEFT_BRACKET COLON UQSTRING properties RIGHT_BRACKET */
#line 205 "grammar.y"
{ 
	yymsp[-4].minor.yy150 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy64, N_DIR_UNKNOWN);
}
#line 1427 "grammar.c"
        break;
      case 33: /* edge ::= LEFT_BRACKET UQSTRING COLON UQSTRING properties RIGHT_BRACKET */
#line 210 "grammar.y"
{ 
	yymsp[-5].minor.yy150 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy64, N_DIR_UNKNOWN);
}
#line 1434 "grammar.c"
        break;
      case 34: /* properties ::= */
#line 216 "grammar.y"
{
	yymsp[1].minor.yy64 = NULL;
}
#line 1441 "grammar.c"
        break;
      case 35: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 220 "grammar.y"
{
	yymsp[-2].minor.yy64 = yymsp[-1].minor.yy64;
}
#line 1448 "grammar.c"
        break;
      case 36: /* mapLiteral ::= UQSTRING COLON value */
#line 226 "grammar.y"
{
	yylhsminor.yy64 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy64, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy22;
	Vector_Push(yylhsminor.yy64, val);
}
#line 1463 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 37: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 238 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy64, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy22;
	Vector_Push(yymsp[0].minor.yy64, val);
	
	yylhsminor.yy64 = yymsp[0].minor.yy64;
}
#line 1479 "grammar.c"
  yymsp[-4].minor.yy64 = yylhsminor.yy64;
        break;
      case 38: /* whereClause ::= */
#line 252 "grammar.y"
{ 
	yymsp[1].minor.yy127 = NULL;
}
#line 1487 "grammar.c"
        break;
      case 39: /* whereClause ::= WHERE cond */
#line 255 "grammar.y"
{
	yymsp[-1].minor.yy127 = New_AST_WhereNode(yymsp[0].minor.yy60);
}
#line 1494 "grammar.c"
        break;
      case 40: /* cond ::= UQSTRING DOT UQSTRING relation UQSTRING DOT UQSTRING */
#line 264 "grammar.y"
{ yylhsminor.yy60 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy130, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1499 "grammar.c"
  yymsp[-6].minor.yy60 = yylhsminor.yy60;
        break;
      case 41: /* cond ::= UQSTRING DOT UQSTRING relation value */
#line 267 "grammar.y"
{ yylhsminor.yy60 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy130, yymsp[0].minor.yy22); }
#line 1505 "grammar.c"
  yymsp[-4].minor.yy60 = yylhsminor.yy60;
        break;
      case 42: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 268 "grammar.y"
{ yymsp[-2].minor.yy60 = yymsp[-1].minor.yy60; }
#line 1511 "grammar.c"
        break;
      case 43: /* cond ::= cond AND cond */
#line 269 "grammar.y"
{ yylhsminor.yy60 = New_AST_ConditionNode(yymsp[-2].minor.yy60, AND, yymsp[0].minor.yy60); }
#line 1516 "grammar.c"
  yymsp[-2].minor.yy60 = yylhsminor.yy60;
        break;
      case 44: /* cond ::= cond OR cond */
#line 270 "grammar.y"
{ yylhsminor.yy60 = New_AST_ConditionNode(yymsp[-2].minor.yy60, OR, yymsp[0].minor.yy60); }
#line 1522 "grammar.c"
  yymsp[-2].minor.yy60 = yylhsminor.yy60;
        break;
      case 45: /* returnClause ::= RETURN returnElements */
#line 275 "grammar.y"
{
	yymsp[-1].minor.yy92 = New_AST_ReturnNode(yymsp[0].minor.yy64, 0);
}
#line 1530 "grammar.c"
        break;
      case 46: /* returnClause ::= RETURN DISTINCT returnElements */
#line 278 "grammar.y"
{
	yymsp[-2].minor.yy92 = New_AST_ReturnNode(yymsp[0].minor.yy64, 1);
}
#line 1537 "grammar.c"
        break;
      case 47: /* returnElements ::= returnElements COMMA returnElement */
#line 285 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy148);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1545 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 48: /* returnElements ::= returnElement */
#line 290 "grammar.y"
{
	yylhsminor.yy64 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy148);
}
#line 1554 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 49: /* returnElement ::= arithmetic_expression */
#line 297 "grammar.y"
{
	yylhsminor.yy148 = New_AST_ReturnElementNode(yymsp[0].minor.yy2, NULL);
}
#line 1562 "grammar.c"
  yymsp[0].minor.yy148 = yylhsminor.yy148;
        break;
      case 50: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 302 "grammar.y"
{
	yylhsminor.yy148 = New_AST_ReturnElementNode(yymsp[-2].minor.yy2, yymsp[0].minor.yy0.strval);
}
#line 1570 "grammar.c"
  yymsp[-2].minor.yy148 = yylhsminor.yy148;
        break;
      case 51: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 309 "grammar.y"
{
	yymsp[-2].minor.yy2 = yymsp[-1].minor.yy2;
}
#line 1578 "grammar.c"
        break;
      case 52: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 321 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 1588 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 53: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 328 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 1599 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 54: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 335 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 1610 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 55: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 342 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy2);
	Vector_Push(args, yymsp[0].minor.yy2);
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 1621 "grammar.c"
  yymsp[-2].minor.yy2 = yylhsminor.yy2;
        break;
      case 56: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 350 "grammar.y"
{
	yylhsminor.yy2 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy64);
}
#line 1629 "grammar.c"
  yymsp[-3].minor.yy2 = yylhsminor.yy2;
        break;
      case 57: /* arithmetic_expression ::= value */
#line 355 "grammar.y"
{
	yylhsminor.yy2 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy22);
}
#line 1637 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 58: /* arithmetic_expression ::= variable */
#line 360 "grammar.y"
{
	yylhsminor.yy2 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy50->alias, yymsp[0].minor.yy50->property);
}
#line 1645 "grammar.c"
  yymsp[0].minor.yy2 = yylhsminor.yy2;
        break;
      case 59: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 366 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy2);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1654 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 60: /* arithmetic_expression_list ::= arithmetic_expression */
#line 370 "grammar.y"
{
	yylhsminor.yy64 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy2);
}
#line 1663 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 61: /* variable ::= UQSTRING */
#line 377 "grammar.y"
{
	yylhsminor.yy50 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 1671 "grammar.c"
  yymsp[0].minor.yy50 = yylhsminor.yy50;
        break;
      case 62: /* variable ::= UQSTRING DOT UQSTRING */
#line 381 "grammar.y"
{
	yylhsminor.yy50 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1679 "grammar.c"
  yymsp[-2].minor.yy50 = yylhsminor.yy50;
        break;
      case 63: /* orderClause ::= */
#line 387 "grammar.y"
{
	yymsp[1].minor.yy141 = NULL;
}
#line 1687 "grammar.c"
        break;
      case 64: /* orderClause ::= ORDER BY columnNameList */
#line 390 "grammar.y"
{
	yymsp[-2].minor.yy141 = New_AST_OrderNode(yymsp[0].minor.yy64, ORDER_DIR_ASC);
}
#line 1694 "grammar.c"
        break;
      case 65: /* orderClause ::= ORDER BY columnNameList ASC */
#line 393 "grammar.y"
{
	yymsp[-3].minor.yy141 = New_AST_OrderNode(yymsp[-1].minor.yy64, ORDER_DIR_ASC);
}
#line 1701 "grammar.c"
        break;
      case 66: /* orderClause ::= ORDER BY columnNameList DESC */
#line 396 "grammar.y"
{
	yymsp[-3].minor.yy141 = New_AST_OrderNode(yymsp[-1].minor.yy64, ORDER_DIR_DESC);
}
#line 1708 "grammar.c"
        break;
      case 67: /* columnNameList ::= columnNameList COMMA columnName */
#line 401 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy64, yymsp[0].minor.yy120);
	yylhsminor.yy64 = yymsp[-2].minor.yy64;
}
#line 1716 "grammar.c"
  yymsp[-2].minor.yy64 = yylhsminor.yy64;
        break;
      case 68: /* columnNameList ::= columnName */
#line 405 "grammar.y"
{
	yylhsminor.yy64 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy64, yymsp[0].minor.yy120);
}
#line 1725 "grammar.c"
  yymsp[0].minor.yy64 = yylhsminor.yy64;
        break;
      case 69: /* columnName ::= variable */
#line 411 "grammar.y"
{
	if(yymsp[0].minor.yy50->property != NULL) {
		yylhsminor.yy120 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy50);
	} else {
		yylhsminor.yy120 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy50->alias);
	}

	Free_AST_Variable(yymsp[0].minor.yy50);
}
#line 1739 "grammar.c"
  yymsp[0].minor.yy120 = yylhsminor.yy120;
        break;
      case 70: /* limitClause ::= */
#line 423 "grammar.y"
{
	yymsp[1].minor.yy149 = NULL;
}
#line 1747 "grammar.c"
        break;
      case 71: /* limitClause ::= LIMIT INTEGER */
#line 426 "grammar.y"
{
	yymsp[-1].minor.yy149 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1754 "grammar.c"
        break;
      case 72: /* relation ::= EQ */
#line 432 "grammar.y"
{ yymsp[0].minor.yy130 = EQ; }
#line 1759 "grammar.c"
        break;
      case 73: /* relation ::= GT */
#line 433 "grammar.y"
{ yymsp[0].minor.yy130 = GT; }
#line 1764 "grammar.c"
        break;
      case 74: /* relation ::= LT */
#line 434 "grammar.y"
{ yymsp[0].minor.yy130 = LT; }
#line 1769 "grammar.c"
        break;
      case 75: /* relation ::= LE */
#line 435 "grammar.y"
{ yymsp[0].minor.yy130 = LE; }
#line 1774 "grammar.c"
        break;
      case 76: /* relation ::= GE */
#line 436 "grammar.y"
{ yymsp[0].minor.yy130 = GE; }
#line 1779 "grammar.c"
        break;
      case 77: /* relation ::= NE */
#line 437 "grammar.y"
{ yymsp[0].minor.yy130 = NE; }
#line 1784 "grammar.c"
        break;
      case 78: /* value ::= INTEGER */
#line 448 "grammar.y"
{  yylhsminor.yy22 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1789 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 79: /* value ::= DASH INTEGER */
#line 449 "grammar.y"
{  yymsp[-1].minor.yy22 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1795 "grammar.c"
        break;
      case 80: /* value ::= STRING */
#line 450 "grammar.y"
{  yylhsminor.yy22 = SI_StringVal(yymsp[0].minor.yy0.strval); }
#line 1800 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 81: /* value ::= FLOAT */
#line 451 "grammar.y"
{  yylhsminor.yy22 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1806 "grammar.c"
  yymsp[0].minor.yy22 = yylhsminor.yy22;
        break;
      case 82: /* value ::= DASH FLOAT */
#line 452 "grammar.y"
{  yymsp[-1].minor.yy22 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1812 "grammar.c"
        break;
      case 83: /* value ::= TRUE */
#line 453 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(1); }
#line 1817 "grammar.c"
        break;
      case 84: /* value ::= FALSE */
#line 454 "grammar.y"
{ yymsp[0].minor.yy22 = SI_BoolVal(0); }
#line 1822 "grammar.c"
        break;
      case 85: /* value ::= NULLVAL */
#line 455 "grammar.y"
{ yymsp[0].minor.yy22 = SI_NullVal(); }
#line 1827 "grammar.c"
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
#line 1892 "grammar.c"
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
#line 457 "grammar.y"


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
#line 2138 "grammar.c"
