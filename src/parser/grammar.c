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
#line 9 "./grammar.y"

	#include <stdlib.h>
	#include <stdio.h>
	#include <assert.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "parse.h"
	#include "../value.h"

	void yyerror(char *s);
#line 40 "./grammar.c"
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
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 70
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  SIValue yy12;
  AST_MatchNode* yy17;
  AST_QueryExpressionNode* yy18;
  AST_Variable* yy19;
  AST_LimitNode* yy21;
  AST_ReturnNode* yy24;
  AST_WhereNode* yy33;
  AST_ArithmeticExpressionNode* yy46;
  int yy52;
  AST_NodeEntity* yy63;
  AST_CreateNode * yy68;
  AST_FilterNode* yy70;
  AST_DeleteNode * yy101;
  Vector* yy108;
  AST_ReturnElementNode* yy109;
  AST_LinkEntity* yy111;
  AST_OrderNode* yy112;
  AST_ColumnNode* yy124;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             89
#define YYNRULE              76
#define YY_MAX_SHIFT         88
#define YY_MIN_SHIFTREDUCE   142
#define YY_MAX_SHIFTREDUCE   217
#define YY_MIN_REDUCE        218
#define YY_MAX_REDUCE        293
#define YY_ERROR_ACTION      294
#define YY_ACCEPT_ACTION     295
#define YY_NO_ACTION         296
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
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if:
**    (1)  The yy_shift_ofst[S]+X value is out of range, or
**    (2)  yy_lookahead[yy_shift_ofst[S]+X] is not equal to X, or
**    (3)  yy_shift_ofst[S] equal YY_SHIFT_USE_DFLT.
** (Implementation note: YY_SHIFT_USE_DFLT is chosen so that
** YY_SHIFT_USE_DFLT+X will be out of range for all possible lookaheads X.
** Hence only tests (1) and (2) need to be evaluated.)
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
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
#define YY_ACTTAB_COUNT (217)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    51,   54,   26,  150,  190,   51,   21,   83,  181,   46,
 /*    10 */    51,  191,   52,    9,   11,   12,  190,   52,    9,   80,
 /*    20 */   181,   46,   57,  191,   85,    2,  205,  206,  209,  207,
 /*    30 */   208,   51,  211,  174,  214,  216,  217,  211,   17,  214,
 /*    40 */   216,  217,  211,  213,  214,  216,  217,   10,    8,    7,
 /*    50 */     6,   66,   10,    8,    7,    6,  210,   37,    3,   10,
 /*    60 */     8,    7,    6,  211,   74,  214,  216,  217,  184,   68,
 /*    70 */    88,  295,   43,  190,  146,   12,  190,   30,   44,  193,
 /*    80 */   191,   44,   81,  191,   55,  190,   82,   70,  190,  180,
 /*    90 */    46,  190,  191,  188,  190,  191,  187,  190,  191,   53,
 /*   100 */   190,  191,   45,   47,  191,   50,   16,  191,   79,   26,
 /*   110 */   150,  201,   48,  200,   27,   28,   64,   72,   22,   24,
 /*   120 */    42,  176,   37,   37,  145,  197,  198,   29,  150,  201,
 /*   130 */    34,  199,   59,   23,   22,   24,  175,   78,    4,    7,
 /*   140 */     6,   49,  212,   37,  215,   39,   67,   31,   84,   73,
 /*   150 */    61,  155,   20,   62,   69,   37,   63,   65,   71,   75,
 /*   160 */    87,  170,   76,   77,  151,   25,    1,  143,   86,   15,
 /*   170 */    40,   41,   14,   24,  173,   56,   18,   58,  156,   60,
 /*   180 */   162,  168,  165,   32,  166,   33,  164,  163,  161,  160,
 /*   190 */    35,  158,  159,   36,  157,   19,   38,    5,  183,  218,
 /*   200 */   189,  194,  204,  220,  220,   84,  220,  220,  220,  220,
 /*   210 */   220,  220,  220,  220,  220,  220,   13,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     4,   51,   52,   53,   59,    4,   17,   62,   63,   64,
 /*    10 */     4,   66,   16,   17,   12,   13,   59,   16,   17,   62,
 /*    20 */    63,   64,   16,   66,   16,   29,    7,    8,    9,   10,
 /*    30 */    11,    4,   36,   59,   38,   39,   40,   36,   61,   38,
 /*    40 */    39,   40,   36,   16,   38,   39,   40,    3,    4,    5,
 /*    50 */     6,   18,    3,    4,    5,    6,   37,   24,   14,    3,
 /*    60 */     4,    5,    6,   36,   59,   38,   39,   40,   19,   56,
 /*    70 */    42,   43,   44,   59,   46,   13,   59,   15,   64,   65,
 /*    80 */    66,   64,   65,   66,   60,   59,   30,   56,   59,   63,
 /*    90 */    64,   59,   66,   64,   59,   66,   64,   59,   66,   64,
 /*   100 */    59,   66,   64,   60,   66,   64,   14,   66,   51,   52,
 /*   110 */    53,   66,   67,   68,   16,   16,   18,   18,    1,    2,
 /*   120 */    46,   60,   24,   24,   50,   33,   34,   52,   53,   66,
 /*   130 */     4,   68,   16,   17,    1,    2,   19,   18,   17,    5,
 /*   140 */     6,   60,   36,   24,   38,   54,   56,   21,   27,   56,
 /*   150 */    55,   16,   22,   57,   57,   24,   56,   56,   56,   16,
 /*   160 */    35,   58,   58,   56,   53,   26,   28,   49,   31,   14,
 /*   170 */    48,   47,   45,    2,   16,   27,   16,   27,   16,   14,
 /*   180 */     4,   25,   23,   16,   23,   16,   23,   23,   20,   19,
 /*   190 */    16,   19,   19,   14,   19,   18,   16,   14,   16,    0,
 /*   200 */    19,   16,   36,   69,   69,   27,   69,   69,   69,   69,
 /*   210 */    69,   69,   69,   69,   69,   69,   32,
};
#define YY_SHIFT_USE_DFLT (217)
#define YY_SHIFT_COUNT    (88)
#define YY_SHIFT_MIN      (-11)
#define YY_SHIFT_MAX      (199)
static const short yy_shift_ofst[] = {
 /*     0 */     2,   -4,    1,    1,    1,    1,    1,    1,    1,    1,
 /*    10 */     1,  -11,  -11,    8,   62,  -11,    8,    6,   19,   27,
 /*    20 */    98,   99,  116,  116,  116,  116,  126,   33,  119,  126,
 /*    30 */   135,  130,  131,  131,  130,  131,  143,  143,  131,  -11,
 /*    40 */   125,  137,  138,  139,   44,   49,   56,  117,   92,  133,
 /*    50 */   134,  106,  121,  134,  155,  171,  158,  148,  160,  150,
 /*    60 */   162,  165,  176,  159,  167,  161,  169,  163,  164,  168,
 /*    70 */   170,  172,  174,  173,  179,  177,  156,  175,  180,  155,
 /*    80 */   183,  181,  182,  183,  185,  178,  184,  166,  199,
};
#define YY_REDUCE_USE_DFLT (-56)
#define YY_REDUCE_COUNT (43)
#define YY_REDUCE_MIN   (-55)
#define YY_REDUCE_MAX   (127)
static const signed char yy_reduce_ofst[] = {
 /*     0 */    28,  -55,  -43,   14,   17,   26,   29,   32,   35,   38,
 /*    10 */    41,  -50,   57,   45,   74,   75,   63,  -26,  -23,    5,
 /*    20 */    13,   31,   24,   43,   61,   81,   91,   90,   93,   91,
 /*    30 */    95,   96,  100,  101,   97,  102,  103,  104,  107,  111,
 /*    40 */   118,  122,  124,  127,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   224,  294,  294,  294,  294,  294,  294,  294,  294,  294,
 /*    10 */   294,  294,  294,  294,  224,  294,  294,  294,  294,  294,
 /*    20 */   243,  243,  294,  294,  294,  294,  228,  243,  243,  229,
 /*    30 */   294,  294,  243,  243,  294,  243,  294,  294,  243,  294,
 /*    40 */   279,  271,  220,  247,  268,  294,  258,  294,  272,  248,
 /*    50 */   261,  294,  289,  262,  223,  253,  294,  289,  294,  294,
 /*    60 */   294,  230,  294,  294,  294,  294,  294,  294,  294,  294,
 /*    70 */   294,  294,  294,  294,  245,  294,  294,  294,  294,  225,
 /*    80 */   255,  294,  294,  254,  294,  278,  294,  294,  294,
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

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "OR",            "AND",           "ADD",         
  "DASH",          "MUL",           "DIV",           "EQ",          
  "GT",            "GE",            "LT",            "LE",          
  "MATCH",         "CREATE",        "COMMA",         "DELETE",      
  "STRING",        "LEFT_PARENTHESIS",  "COLON",         "RIGHT_PARENTHESIS",
  "RIGHT_ARROW",   "LEFT_ARROW",    "LEFT_BRACKET",  "RIGHT_BRACKET",
  "LEFT_CURLY_BRACKET",  "RIGHT_CURLY_BRACKET",  "WHERE",         "DOT",         
  "RETURN",        "DISTINCT",      "AS",            "ORDER",       
  "BY",            "ASC",           "DESC",          "LIMIT",       
  "INTEGER",       "NE",            "FLOAT",         "TRUE",        
  "FALSE",         "error",         "expr",          "query",       
  "matchClause",   "whereClause",   "createClause",  "returnClause",
  "orderClause",   "limitClause",   "deleteClause",  "chains",      
  "chain",         "node",          "link",          "deleteExpression",
  "properties",    "edge",          "mapLiteral",    "value",       
  "cond",          "relation",      "returnElements",  "returnElement",
  "arithmetic_expression",  "arithmetic_expression_list",  "variable",      "columnNameList",
  "columnName",  
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expr",
 /*   1 */ "expr ::= matchClause whereClause createClause returnClause orderClause limitClause",
 /*   2 */ "expr ::= matchClause whereClause createClause",
 /*   3 */ "expr ::= matchClause whereClause deleteClause",
 /*   4 */ "expr ::= createClause",
 /*   5 */ "matchClause ::= MATCH chains",
 /*   6 */ "createClause ::=",
 /*   7 */ "createClause ::= CREATE chains",
 /*   8 */ "chain ::= node",
 /*   9 */ "chain ::= chain link node",
 /*  10 */ "chains ::= chain",
 /*  11 */ "chains ::= chains COMMA chain",
 /*  12 */ "deleteClause ::= DELETE deleteExpression",
 /*  13 */ "deleteExpression ::= STRING",
 /*  14 */ "deleteExpression ::= deleteExpression COMMA STRING",
 /*  15 */ "node ::= LEFT_PARENTHESIS STRING COLON STRING properties RIGHT_PARENTHESIS",
 /*  16 */ "node ::= LEFT_PARENTHESIS COLON STRING properties RIGHT_PARENTHESIS",
 /*  17 */ "node ::= LEFT_PARENTHESIS STRING properties RIGHT_PARENTHESIS",
 /*  18 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  19 */ "link ::= DASH edge RIGHT_ARROW",
 /*  20 */ "link ::= LEFT_ARROW edge DASH",
 /*  21 */ "edge ::= LEFT_BRACKET properties RIGHT_BRACKET",
 /*  22 */ "edge ::= LEFT_BRACKET STRING properties RIGHT_BRACKET",
 /*  23 */ "edge ::= LEFT_BRACKET COLON STRING properties RIGHT_BRACKET",
 /*  24 */ "edge ::= LEFT_BRACKET STRING COLON STRING properties RIGHT_BRACKET",
 /*  25 */ "properties ::=",
 /*  26 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  27 */ "mapLiteral ::= STRING COLON value",
 /*  28 */ "mapLiteral ::= STRING COLON value COMMA mapLiteral",
 /*  29 */ "whereClause ::=",
 /*  30 */ "whereClause ::= WHERE cond",
 /*  31 */ "cond ::= STRING DOT STRING relation STRING DOT STRING",
 /*  32 */ "cond ::= STRING DOT STRING relation value",
 /*  33 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  34 */ "cond ::= cond AND cond",
 /*  35 */ "cond ::= cond OR cond",
 /*  36 */ "returnClause ::= RETURN returnElements",
 /*  37 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  38 */ "returnElements ::= returnElements COMMA returnElement",
 /*  39 */ "returnElements ::= returnElement",
 /*  40 */ "returnElement ::= arithmetic_expression",
 /*  41 */ "returnElement ::= arithmetic_expression AS STRING",
 /*  42 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  43 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  44 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  45 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  46 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  47 */ "arithmetic_expression ::= STRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  48 */ "arithmetic_expression ::= value",
 /*  49 */ "arithmetic_expression ::= variable",
 /*  50 */ "arithmetic_expression_list ::= arithmetic_expression",
 /*  51 */ "arithmetic_expression_list ::= arithmetic_expression COMMA arithmetic_expression_list",
 /*  52 */ "variable ::= STRING DOT STRING",
 /*  53 */ "orderClause ::=",
 /*  54 */ "orderClause ::= ORDER BY columnNameList",
 /*  55 */ "orderClause ::= ORDER BY columnNameList ASC",
 /*  56 */ "orderClause ::= ORDER BY columnNameList DESC",
 /*  57 */ "columnNameList ::= columnNameList COMMA columnName",
 /*  58 */ "columnNameList ::= columnName",
 /*  59 */ "columnName ::= variable",
 /*  60 */ "columnName ::= STRING",
 /*  61 */ "limitClause ::=",
 /*  62 */ "limitClause ::= LIMIT INTEGER",
 /*  63 */ "relation ::= EQ",
 /*  64 */ "relation ::= GT",
 /*  65 */ "relation ::= LT",
 /*  66 */ "relation ::= LE",
 /*  67 */ "relation ::= GE",
 /*  68 */ "relation ::= NE",
 /*  69 */ "value ::= INTEGER",
 /*  70 */ "value ::= DASH INTEGER",
 /*  71 */ "value ::= STRING",
 /*  72 */ "value ::= FLOAT",
 /*  73 */ "value ::= DASH FLOAT",
 /*  74 */ "value ::= TRUE",
 /*  75 */ "value ::= FALSE",
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
  if( pParser ){
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
  }
  return pParser;
}

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
    case 60: /* cond */
{
#line 220 "./grammar.y"
 Free_AST_FilterNode((yypminor->yy70)); 
#line 616 "./grammar.c"
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
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
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
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
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
  assert( i!=YY_REDUCE_USE_DFLT );
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
   yypParser->yytos--;
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
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
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
  if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH] ){
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
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
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 43, 1 },
  { 42, 6 },
  { 42, 3 },
  { 42, 3 },
  { 42, 1 },
  { 44, 2 },
  { 46, 0 },
  { 46, 2 },
  { 52, 1 },
  { 52, 3 },
  { 51, 1 },
  { 51, 3 },
  { 50, 2 },
  { 55, 1 },
  { 55, 3 },
  { 53, 6 },
  { 53, 5 },
  { 53, 4 },
  { 53, 3 },
  { 54, 3 },
  { 54, 3 },
  { 57, 3 },
  { 57, 4 },
  { 57, 5 },
  { 57, 6 },
  { 56, 0 },
  { 56, 3 },
  { 58, 3 },
  { 58, 5 },
  { 45, 0 },
  { 45, 2 },
  { 60, 7 },
  { 60, 5 },
  { 60, 3 },
  { 60, 3 },
  { 60, 3 },
  { 47, 2 },
  { 47, 3 },
  { 62, 3 },
  { 62, 1 },
  { 63, 1 },
  { 63, 3 },
  { 64, 3 },
  { 64, 3 },
  { 64, 3 },
  { 64, 3 },
  { 64, 3 },
  { 64, 4 },
  { 64, 1 },
  { 64, 1 },
  { 65, 1 },
  { 65, 3 },
  { 66, 3 },
  { 48, 0 },
  { 48, 3 },
  { 48, 4 },
  { 48, 4 },
  { 67, 3 },
  { 67, 1 },
  { 68, 1 },
  { 68, 1 },
  { 49, 0 },
  { 49, 2 },
  { 61, 1 },
  { 61, 1 },
  { 61, 1 },
  { 61, 1 },
  { 61, 1 },
  { 61, 1 },
  { 59, 1 },
  { 59, 2 },
  { 59, 1 },
  { 59, 1 },
  { 59, 2 },
  { 59, 1 },
  { 59, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno        /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
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
    if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH-1] ){
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
#line 35 "./grammar.y"
{ ctx->root = yymsp[0].minor.yy18; }
#line 1000 "./grammar.c"
        break;
      case 1: /* expr ::= matchClause whereClause createClause returnClause orderClause limitClause */
#line 37 "./grammar.y"
{
	yylhsminor.yy18 = New_AST_QueryExpressionNode(yymsp[-5].minor.yy17, yymsp[-4].minor.yy33, yymsp[-3].minor.yy68, NULL, yymsp[-2].minor.yy24, yymsp[-1].minor.yy112, yymsp[0].minor.yy21);
}
#line 1007 "./grammar.c"
  yymsp[-5].minor.yy18 = yylhsminor.yy18;
        break;
      case 2: /* expr ::= matchClause whereClause createClause */
#line 41 "./grammar.y"
{
	yylhsminor.yy18 = New_AST_QueryExpressionNode(yymsp[-2].minor.yy17, yymsp[-1].minor.yy33, yymsp[0].minor.yy68, NULL, NULL, NULL, NULL);
}
#line 1015 "./grammar.c"
  yymsp[-2].minor.yy18 = yylhsminor.yy18;
        break;
      case 3: /* expr ::= matchClause whereClause deleteClause */
#line 45 "./grammar.y"
{
	yylhsminor.yy18 = New_AST_QueryExpressionNode(yymsp[-2].minor.yy17, yymsp[-1].minor.yy33, NULL, yymsp[0].minor.yy101, NULL, NULL, NULL);
}
#line 1023 "./grammar.c"
  yymsp[-2].minor.yy18 = yylhsminor.yy18;
        break;
      case 4: /* expr ::= createClause */
#line 49 "./grammar.y"
{
	yylhsminor.yy18 = New_AST_QueryExpressionNode(NULL, NULL, yymsp[0].minor.yy68, NULL, NULL, NULL, NULL);
}
#line 1031 "./grammar.c"
  yymsp[0].minor.yy18 = yylhsminor.yy18;
        break;
      case 5: /* matchClause ::= MATCH chains */
#line 55 "./grammar.y"
{
	yymsp[-1].minor.yy17 = New_AST_MatchNode(yymsp[0].minor.yy108);
}
#line 1039 "./grammar.c"
        break;
      case 6: /* createClause ::= */
#line 62 "./grammar.y"
{
	yymsp[1].minor.yy68 = NULL;
}
#line 1046 "./grammar.c"
        break;
      case 7: /* createClause ::= CREATE chains */
#line 66 "./grammar.y"
{
	yymsp[-1].minor.yy68 = New_AST_CreateNode(yymsp[0].minor.yy108);
}
#line 1053 "./grammar.c"
        break;
      case 8: /* chain ::= node */
#line 72 "./grammar.y"
{
	yylhsminor.yy108 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy108, yymsp[0].minor.yy63);
}
#line 1061 "./grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 9: /* chain ::= chain link node */
#line 77 "./grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy108, yymsp[-1].minor.yy111);
	Vector_Push(yymsp[-2].minor.yy108, yymsp[0].minor.yy63);
	yylhsminor.yy108 = yymsp[-2].minor.yy108;
}
#line 1071 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 10: /* chains ::= chain */
#line 84 "./grammar.y"
{
	yylhsminor.yy108 = yymsp[0].minor.yy108;
}
#line 1079 "./grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 11: /* chains ::= chains COMMA chain */
#line 88 "./grammar.y"
{
	for(int i = 0; i < Vector_Size(yymsp[0].minor.yy108); i++) {
		AST_GraphEntity *entity;
		Vector_Get(yymsp[0].minor.yy108, i, &entity);
		Vector_Push(yymsp[-2].minor.yy108, entity);
	}
	Vector_Free(yymsp[0].minor.yy108);
	yylhsminor.yy108 = yymsp[-2].minor.yy108;
}
#line 1093 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 12: /* deleteClause ::= DELETE deleteExpression */
#line 101 "./grammar.y"
{
	yymsp[-1].minor.yy101 = New_AST_DeleteNode(yymsp[0].minor.yy108);
}
#line 1101 "./grammar.c"
        break;
      case 13: /* deleteExpression ::= STRING */
#line 107 "./grammar.y"
{
	yylhsminor.yy108 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy108, yymsp[0].minor.yy0.strval);
}
#line 1109 "./grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 14: /* deleteExpression ::= deleteExpression COMMA STRING */
#line 112 "./grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy108, yymsp[0].minor.yy0.strval);
	yylhsminor.yy108 = yymsp[-2].minor.yy108;
}
#line 1118 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 15: /* node ::= LEFT_PARENTHESIS STRING COLON STRING properties RIGHT_PARENTHESIS */
#line 120 "./grammar.y"
{
	yymsp[-5].minor.yy63 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy108);
}
#line 1126 "./grammar.c"
        break;
      case 16: /* node ::= LEFT_PARENTHESIS COLON STRING properties RIGHT_PARENTHESIS */
#line 125 "./grammar.y"
{
	yymsp[-4].minor.yy63 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy108);
}
#line 1133 "./grammar.c"
        break;
      case 17: /* node ::= LEFT_PARENTHESIS STRING properties RIGHT_PARENTHESIS */
#line 130 "./grammar.y"
{
	yymsp[-3].minor.yy63 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy108);
}
#line 1140 "./grammar.c"
        break;
      case 18: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 135 "./grammar.y"
{
	yymsp[-2].minor.yy63 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy108);
}
#line 1147 "./grammar.c"
        break;
      case 19: /* link ::= DASH edge RIGHT_ARROW */
#line 142 "./grammar.y"
{
	yymsp[-2].minor.yy111 = yymsp[-1].minor.yy111;
	yymsp[-2].minor.yy111->direction = N_LEFT_TO_RIGHT;
}
#line 1155 "./grammar.c"
        break;
      case 20: /* link ::= LEFT_ARROW edge DASH */
#line 148 "./grammar.y"
{
	yymsp[-2].minor.yy111 = yymsp[-1].minor.yy111;
	yymsp[-2].minor.yy111->direction = N_RIGHT_TO_LEFT;
}
#line 1163 "./grammar.c"
        break;
      case 21: /* edge ::= LEFT_BRACKET properties RIGHT_BRACKET */
#line 155 "./grammar.y"
{ 
	yymsp[-2].minor.yy111 = New_AST_LinkEntity(NULL, NULL, yymsp[-1].minor.yy108, N_DIR_UNKNOWN);
}
#line 1170 "./grammar.c"
        break;
      case 22: /* edge ::= LEFT_BRACKET STRING properties RIGHT_BRACKET */
#line 160 "./grammar.y"
{ 
	yymsp[-3].minor.yy111 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy108, N_DIR_UNKNOWN);
}
#line 1177 "./grammar.c"
        break;
      case 23: /* edge ::= LEFT_BRACKET COLON STRING properties RIGHT_BRACKET */
#line 165 "./grammar.y"
{ 
	yymsp[-4].minor.yy111 = New_AST_LinkEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy108, N_DIR_UNKNOWN);
}
#line 1184 "./grammar.c"
        break;
      case 24: /* edge ::= LEFT_BRACKET STRING COLON STRING properties RIGHT_BRACKET */
#line 170 "./grammar.y"
{ 
	yymsp[-5].minor.yy111 = New_AST_LinkEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy108, N_DIR_UNKNOWN);
}
#line 1191 "./grammar.c"
        break;
      case 25: /* properties ::= */
#line 176 "./grammar.y"
{
	yymsp[1].minor.yy108 = NULL;
}
#line 1198 "./grammar.c"
        break;
      case 26: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 180 "./grammar.y"
{
	yymsp[-2].minor.yy108 = yymsp[-1].minor.yy108;
}
#line 1205 "./grammar.c"
        break;
      case 27: /* mapLiteral ::= STRING COLON value */
#line 185 "./grammar.y"
{
	yylhsminor.yy108 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(yymsp[-2].minor.yy0.strval));
	Vector_Push(yylhsminor.yy108, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy12;
	Vector_Push(yylhsminor.yy108, val);
}
#line 1220 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 28: /* mapLiteral ::= STRING COLON value COMMA mapLiteral */
#line 197 "./grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(yymsp[-4].minor.yy0.strval));
	Vector_Push(yymsp[0].minor.yy108, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy12;
	Vector_Push(yymsp[0].minor.yy108, val);
	
	yylhsminor.yy108 = yymsp[0].minor.yy108;
}
#line 1236 "./grammar.c"
  yymsp[-4].minor.yy108 = yylhsminor.yy108;
        break;
      case 29: /* whereClause ::= */
#line 211 "./grammar.y"
{ 
	yymsp[1].minor.yy33 = NULL;
}
#line 1244 "./grammar.c"
        break;
      case 30: /* whereClause ::= WHERE cond */
#line 214 "./grammar.y"
{
	yymsp[-1].minor.yy33 = New_AST_WhereNode(yymsp[0].minor.yy70);
}
#line 1251 "./grammar.c"
        break;
      case 31: /* cond ::= STRING DOT STRING relation STRING DOT STRING */
#line 222 "./grammar.y"
{ yylhsminor.yy70 = New_AST_VaryingPredicateNode(yymsp[-6].minor.yy0.strval, yymsp[-4].minor.yy0.strval, yymsp[-3].minor.yy52, yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval); }
#line 1256 "./grammar.c"
  yymsp[-6].minor.yy70 = yylhsminor.yy70;
        break;
      case 32: /* cond ::= STRING DOT STRING relation value */
#line 223 "./grammar.y"
{ yylhsminor.yy70 = New_AST_ConstantPredicateNode(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy52, yymsp[0].minor.yy12); }
#line 1262 "./grammar.c"
  yymsp[-4].minor.yy70 = yylhsminor.yy70;
        break;
      case 33: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 224 "./grammar.y"
{ yymsp[-2].minor.yy70 = yymsp[-1].minor.yy70; }
#line 1268 "./grammar.c"
        break;
      case 34: /* cond ::= cond AND cond */
#line 225 "./grammar.y"
{ yylhsminor.yy70 = New_AST_ConditionNode(yymsp[-2].minor.yy70, AND, yymsp[0].minor.yy70); }
#line 1273 "./grammar.c"
  yymsp[-2].minor.yy70 = yylhsminor.yy70;
        break;
      case 35: /* cond ::= cond OR cond */
#line 226 "./grammar.y"
{ yylhsminor.yy70 = New_AST_ConditionNode(yymsp[-2].minor.yy70, OR, yymsp[0].minor.yy70); }
#line 1279 "./grammar.c"
  yymsp[-2].minor.yy70 = yylhsminor.yy70;
        break;
      case 36: /* returnClause ::= RETURN returnElements */
#line 231 "./grammar.y"
{
	yymsp[-1].minor.yy24 = New_AST_ReturnNode(yymsp[0].minor.yy108, 0);
}
#line 1287 "./grammar.c"
        break;
      case 37: /* returnClause ::= RETURN DISTINCT returnElements */
#line 234 "./grammar.y"
{
	yymsp[-2].minor.yy24 = New_AST_ReturnNode(yymsp[0].minor.yy108, 1);
}
#line 1294 "./grammar.c"
        break;
      case 38: /* returnElements ::= returnElements COMMA returnElement */
#line 241 "./grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy108, yymsp[0].minor.yy109);
	yylhsminor.yy108 = yymsp[-2].minor.yy108;
}
#line 1302 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 39: /* returnElements ::= returnElement */
#line 246 "./grammar.y"
{
	yylhsminor.yy108 = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(yylhsminor.yy108, yymsp[0].minor.yy109);
}
#line 1311 "./grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 40: /* returnElement ::= arithmetic_expression */
#line 253 "./grammar.y"
{
	yylhsminor.yy109 = New_AST_ReturnElementNode(yymsp[0].minor.yy46, NULL);
}
#line 1319 "./grammar.c"
  yymsp[0].minor.yy109 = yylhsminor.yy109;
        break;
      case 41: /* returnElement ::= arithmetic_expression AS STRING */
#line 256 "./grammar.y"
{
	yylhsminor.yy109 = New_AST_ReturnElementNode(yymsp[-2].minor.yy46, yymsp[0].minor.yy0.strval);
}
#line 1327 "./grammar.c"
  yymsp[-2].minor.yy109 = yylhsminor.yy109;
        break;
      case 42: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 263 "./grammar.y"
{
	yymsp[-2].minor.yy46 = yymsp[-1].minor.yy46;
}
#line 1335 "./grammar.c"
        break;
      case 43: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 275 "./grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy46);
	Vector_Push(args, yymsp[0].minor.yy46);
	yylhsminor.yy46 = NEW_AST_AR_EXP_OpNode("ADD", args);
}
#line 1345 "./grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 44: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 282 "./grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy46);
	Vector_Push(args, yymsp[0].minor.yy46);
	yylhsminor.yy46 = NEW_AST_AR_EXP_OpNode("SUB", args);
}
#line 1356 "./grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 45: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 289 "./grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy46);
	Vector_Push(args, yymsp[0].minor.yy46);
	yylhsminor.yy46 = NEW_AST_AR_EXP_OpNode("MUL", args);
}
#line 1367 "./grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 46: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 296 "./grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy46);
	Vector_Push(args, yymsp[0].minor.yy46);
	yylhsminor.yy46 = NEW_AST_AR_EXP_OpNode("DIV", args);
}
#line 1378 "./grammar.c"
  yymsp[-2].minor.yy46 = yylhsminor.yy46;
        break;
      case 47: /* arithmetic_expression ::= STRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 304 "./grammar.y"
{
	yylhsminor.yy46 = NEW_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy108);
}
#line 1386 "./grammar.c"
  yymsp[-3].minor.yy46 = yylhsminor.yy46;
        break;
      case 48: /* arithmetic_expression ::= value */
#line 309 "./grammar.y"
{
	if(yymsp[0].minor.yy12.type == T_STRING) {
		yylhsminor.yy46 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy12.stringval.str, NULL);
	} else {
		yylhsminor.yy46 = NEW_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy12);
	}
}
#line 1398 "./grammar.c"
  yymsp[0].minor.yy46 = yylhsminor.yy46;
        break;
      case 49: /* arithmetic_expression ::= variable */
#line 318 "./grammar.y"
{
	yylhsminor.yy46 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy19->alias, yymsp[0].minor.yy19->property);
}
#line 1406 "./grammar.c"
  yymsp[0].minor.yy46 = yylhsminor.yy46;
        break;
      case 50: /* arithmetic_expression_list ::= arithmetic_expression */
#line 328 "./grammar.y"
{
	yylhsminor.yy108 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy108, yymsp[0].minor.yy46);
}
#line 1415 "./grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 51: /* arithmetic_expression_list ::= arithmetic_expression COMMA arithmetic_expression_list */
#line 332 "./grammar.y"
{
	Vector_Push(yymsp[0].minor.yy108, yymsp[-2].minor.yy46);
	yylhsminor.yy108 = yymsp[0].minor.yy108;
}
#line 1424 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 52: /* variable ::= STRING DOT STRING */
#line 339 "./grammar.y"
{
	yylhsminor.yy19 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 1432 "./grammar.c"
  yymsp[-2].minor.yy19 = yylhsminor.yy19;
        break;
      case 53: /* orderClause ::= */
#line 345 "./grammar.y"
{
	yymsp[1].minor.yy112 = NULL;
}
#line 1440 "./grammar.c"
        break;
      case 54: /* orderClause ::= ORDER BY columnNameList */
#line 348 "./grammar.y"
{
	yymsp[-2].minor.yy112 = New_AST_OrderNode(yymsp[0].minor.yy108, ORDER_DIR_ASC);
}
#line 1447 "./grammar.c"
        break;
      case 55: /* orderClause ::= ORDER BY columnNameList ASC */
#line 351 "./grammar.y"
{
	yymsp[-3].minor.yy112 = New_AST_OrderNode(yymsp[-1].minor.yy108, ORDER_DIR_ASC);
}
#line 1454 "./grammar.c"
        break;
      case 56: /* orderClause ::= ORDER BY columnNameList DESC */
#line 354 "./grammar.y"
{
	yymsp[-3].minor.yy112 = New_AST_OrderNode(yymsp[-1].minor.yy108, ORDER_DIR_DESC);
}
#line 1461 "./grammar.c"
        break;
      case 57: /* columnNameList ::= columnNameList COMMA columnName */
#line 359 "./grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy108, yymsp[0].minor.yy124);
	yylhsminor.yy108 = yymsp[-2].minor.yy108;
}
#line 1469 "./grammar.c"
  yymsp[-2].minor.yy108 = yylhsminor.yy108;
        break;
      case 58: /* columnNameList ::= columnName */
#line 363 "./grammar.y"
{
	yylhsminor.yy108 = NewVector(AST_ColumnNode*, 1);
	Vector_Push(yylhsminor.yy108, yymsp[0].minor.yy124);
}
#line 1478 "./grammar.c"
  yymsp[0].minor.yy108 = yylhsminor.yy108;
        break;
      case 59: /* columnName ::= variable */
#line 369 "./grammar.y"
{
	yylhsminor.yy124 = AST_ColumnNodeFromVariable(yymsp[0].minor.yy19);
	Free_AST_Variable(yymsp[0].minor.yy19);
}
#line 1487 "./grammar.c"
  yymsp[0].minor.yy124 = yylhsminor.yy124;
        break;
      case 60: /* columnName ::= STRING */
#line 373 "./grammar.y"
{
	yylhsminor.yy124 = AST_ColumnNodeFromAlias(yymsp[0].minor.yy0.strval);
}
#line 1495 "./grammar.c"
  yymsp[0].minor.yy124 = yylhsminor.yy124;
        break;
      case 61: /* limitClause ::= */
#line 379 "./grammar.y"
{
	yymsp[1].minor.yy21 = NULL;
}
#line 1503 "./grammar.c"
        break;
      case 62: /* limitClause ::= LIMIT INTEGER */
#line 382 "./grammar.y"
{
	yymsp[-1].minor.yy21 = New_AST_LimitNode(yymsp[0].minor.yy0.intval);
}
#line 1510 "./grammar.c"
        break;
      case 63: /* relation ::= EQ */
#line 388 "./grammar.y"
{ yymsp[0].minor.yy52 = EQ; }
#line 1515 "./grammar.c"
        break;
      case 64: /* relation ::= GT */
#line 389 "./grammar.y"
{ yymsp[0].minor.yy52 = GT; }
#line 1520 "./grammar.c"
        break;
      case 65: /* relation ::= LT */
#line 390 "./grammar.y"
{ yymsp[0].minor.yy52 = LT; }
#line 1525 "./grammar.c"
        break;
      case 66: /* relation ::= LE */
#line 391 "./grammar.y"
{ yymsp[0].minor.yy52 = LE; }
#line 1530 "./grammar.c"
        break;
      case 67: /* relation ::= GE */
#line 392 "./grammar.y"
{ yymsp[0].minor.yy52 = GE; }
#line 1535 "./grammar.c"
        break;
      case 68: /* relation ::= NE */
#line 393 "./grammar.y"
{ yymsp[0].minor.yy52 = NE; }
#line 1540 "./grammar.c"
        break;
      case 69: /* value ::= INTEGER */
#line 404 "./grammar.y"
{  yylhsminor.yy12 = SI_DoubleVal(yymsp[0].minor.yy0.intval); }
#line 1545 "./grammar.c"
  yymsp[0].minor.yy12 = yylhsminor.yy12;
        break;
      case 70: /* value ::= DASH INTEGER */
#line 405 "./grammar.y"
{  yymsp[-1].minor.yy12 = SI_DoubleVal(-yymsp[0].minor.yy0.intval); }
#line 1551 "./grammar.c"
        break;
      case 71: /* value ::= STRING */
#line 406 "./grammar.y"
{  yylhsminor.yy12 = SI_StringValC(strdup(yymsp[0].minor.yy0.strval)); }
#line 1556 "./grammar.c"
  yymsp[0].minor.yy12 = yylhsminor.yy12;
        break;
      case 72: /* value ::= FLOAT */
#line 407 "./grammar.y"
{  yylhsminor.yy12 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 1562 "./grammar.c"
  yymsp[0].minor.yy12 = yylhsminor.yy12;
        break;
      case 73: /* value ::= DASH FLOAT */
#line 408 "./grammar.y"
{  yymsp[-1].minor.yy12 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 1568 "./grammar.c"
        break;
      case 74: /* value ::= TRUE */
#line 409 "./grammar.y"
{ yymsp[0].minor.yy12 = SI_BoolVal(1); }
#line 1573 "./grammar.c"
        break;
      case 75: /* value ::= FALSE */
#line 410 "./grammar.y"
{ yymsp[0].minor.yy12 = SI_BoolVal(0); }
#line 1578 "./grammar.c"
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ){
      yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    }
    yymsp -= yysize-1;
    yypParser->yytos = yymsp;
    yymsp->stateno = (YYACTIONTYPE)yyact;
    yymsp->major = (YYCODETYPE)yygoto;
    yyTraceShift(yypParser, yyact);
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yypParser->yytos -= yysize;
    yy_accept(yypParser);
  }
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
#line 22 "./grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'\n", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 1644 "./grammar.c"
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
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
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
        while( yypParser->yytos >= &yypParser->yystack
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
#line 412 "./grammar.y"
	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	AST_QueryExpressionNode *Query_Parse(const char *q, size_t len, char **err) {
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
#line 1879 "./grammar.c"
