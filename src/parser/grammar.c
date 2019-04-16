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
	#include <stdint.h>
	#include <assert.h>
	#include <limits.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "./clauses/clauses.h"
	#include "parse.h"
	#include "../value.h"
	#include "../util/arr.h"

	void yyerror(char *s);

	/*
	**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
	**                       zero the stack is dynamically sized using realloc()
	*/
	// Increase depth from 100 to 1000 to handel deep recursion.
	#define YYSTACKDEPTH 1000
#line 51 "grammar.c"
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
#define YYNOCODE 102
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_WhereNode* yy11;
  AST_CreateNode* yy40;
  AST_WithNode* yy48;
  AST_LimitNode* yy65;
  AST_NodeEntity* yy71;
  AST_LinkLength* yy76;
  AST_IndexNode* yy82;
  AST* yy83;
  AST_WithElementNode** yy86;
  char* yy95;
  AST_SetElement* yy98;
  AST_ArithmeticExpressionNode* yy102;
  AST_LinkEntity* yy105;
  AST_OrderNode* yy114;
  int yy115;
  SIValue yy136;
  AST_SkipNode* yy137;
  char** yy138;
  AST** yy147;
  AST_MergeNode* yy152;
  AST_FilterNode* yy153;
  AST_Variable* yy154;
  AST_MatchNode* yy161;
  AST_ReturnNode* yy162;
  AST_UnwindNode* yy169;
  AST_ReturnElementNode** yy172;
  AST_ReturnElementNode* yy173;
  AST_WithElementNode* yy174;
  AST_DeleteNode * yy179;
  AST_IndexOpType yy187;
  AST_SetNode* yy200;
  Vector* yy202;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             144
#define YYNRULE              125
#define YYNTOKEN             53
#define YY_MAX_SHIFT         143
#define YY_MIN_SHIFTREDUCE   229
#define YY_MAX_SHIFTREDUCE   353
#define YY_ERROR_ACTION      354
#define YY_ACCEPT_ACTION     355
#define YY_NO_ACTION         356
#define YY_MIN_REDUCE        357
#define YY_MAX_REDUCE        481
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
#define YY_ACTTAB_COUNT (415)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   360,   16,  359,   31,   79,   39,   75,  397,  135,   72,
 /*    10 */   126,  373,   15,  375,   50,   49,  381,  122,   63,  386,
 /*    20 */   109,  355,   46,  358,  365,   38,  366,  367,  378,  453,
 /*    30 */    84,  134,   72,   77,  373,   15,  375,   50,   49,  381,
 /*    40 */   452,   63,  386,  109,  437,   28,   27,  453,   84,  263,
 /*    50 */    95,  311,  453,   84,   32,  459,  115,  459,  452,  453,
 /*    60 */    36,  141,  438,  452,   97,   23,  139,  438,    2,  131,
 /*    70 */   452,  111,   62,  143,   54,   41,  346,  137,  138,  104,
 /*    80 */   399,  125,    3,   22,   21,   20,   19,  340,  341,  344,
 /*    90 */   342,  343,   95,  311,  348,  349,  351,  352,  353,    2,
 /*   100 */   318,   28,   27,  453,   83,  263,   97,   23,   40,  138,
 /*   110 */    32,   67,  370,  399,  452,   71,   82,   80,  346,  101,
 /*   120 */   443,  143,   63,  386,    2,  459,  459,  345,   22,   21,
 /*   130 */    20,   19,  136,  396,  135,  104,  348,  349,  351,  352,
 /*   140 */   353,   22,   21,   20,   19,  340,  341,  344,  342,  343,
 /*   150 */    95,   22,   21,   20,   19,   95,  453,   83,   55,    1,
 /*   160 */    95,   60,  100,   18,   97,   23,   52,  452,  318,   97,
 /*   170 */    10,   56,  374,  444,   42,  417,  346,  453,   86,   63,
 /*   180 */   386,  346,  333,  334,  107,  345,  346,   29,  452,   22,
 /*   190 */    21,   20,   19,   57,  348,  349,  351,  352,  353,  348,
 /*   200 */   349,  351,  352,  353,  348,  349,  351,  352,  353,    2,
 /*   210 */   453,   89,   45,  453,   35,   53,  105,  399,  453,   36,
 /*   220 */    30,  452,   73,  140,  452,   90,  453,   36,   92,  452,
 /*   230 */   433,  453,   89,    2,  453,   89,   47,  452,   93,   29,
 /*   240 */   453,   87,  452,  119,  117,  452,  389,   94,   30,   96,
 /*   250 */    73,  452,   91,   22,   21,   20,   19,  357,  114,  453,
 /*   260 */    88,  453,  450,  453,  449,  116,  114,   26,   43,  417,
 /*   270 */   452,   55,  452,   60,  452,  453,   98,  453,   99,  453,
 /*   280 */    85,   60,  124,   44,  130,  110,  452,   28,  452,   18,
 /*   290 */   452,   41,    9,   11,  103,    4,  399,   60,  347,   60,
 /*   300 */   323,    9,   11,   18,   13,  382,   20,   19,   30,  377,
 /*   310 */    51,  304,  379,  112,  138,  137,   24,  350,  113,   60,
 /*   320 */   114,  418,   94,  120,  142,  121,  123,  127,  428,  400,
 /*   330 */   128,  129,   29,  387,  372,   64,  143,    2,  368,   65,
 /*   340 */    66,   68,   12,   69,   70,  276,  133,  317,   25,   74,
 /*   350 */     5,    7,  364,   76,  339,  362,   78,  102,    8,  363,
 /*   360 */   361,  264,  265,  106,   48,   81,  108,   33,   11,  283,
 /*   370 */   294,  288,  118,   58,  286,  287,  285,  281,  279,  284,
 /*   380 */   292,  282,  280,   59,   37,   17,  132,  298,  278,   61,
 /*   390 */   277,  336,   34,    6,  338,  313,  356,  330,  142,  356,
 /*   400 */   356,  356,  356,  356,  356,  356,  356,  356,  356,  356,
 /*   410 */   356,  356,  356,  356,   14,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    56,   95,   58,   59,   60,   61,   62,   81,   82,   65,
 /*    10 */    93,   67,   68,   69,   70,   71,   72,   87,   74,   75,
 /*    20 */    76,   54,   55,   56,   61,   19,   63,   64,   61,   82,
 /*    30 */    83,   18,   65,   62,   67,   68,   69,   70,   71,   72,
 /*    40 */    93,   74,   75,   76,   97,   12,   13,   82,   83,   16,
 /*    50 */     4,    5,   82,   83,   21,   22,   87,   24,   93,   82,
 /*    60 */    83,   96,   97,   93,   18,   19,   96,   97,   35,   73,
 /*    70 */    93,   94,   85,   40,   89,   79,   30,   44,   45,   46,
 /*    80 */    84,   87,   36,    3,    4,    5,    6,    7,    8,    9,
 /*    90 */    10,   11,    4,    5,   48,   49,   50,   51,   52,   35,
 /*   100 */    20,   12,   13,   82,   83,   16,   18,   19,   79,   45,
 /*   110 */    21,   63,   64,   84,   93,   67,   60,   61,   30,   98,
 /*   120 */    99,   40,   74,   75,   35,   44,   45,   47,    3,    4,
 /*   130 */     5,    6,   80,   81,   82,   46,   48,   49,   50,   51,
 /*   140 */    52,    3,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   150 */     4,    3,    4,    5,    6,    4,   82,   83,   29,   57,
 /*   160 */     4,   32,   37,   23,   18,   19,   61,   93,   20,   18,
 /*   170 */    19,   87,   67,   99,   90,   91,   30,   82,   83,   74,
 /*   180 */    75,   30,   42,   43,   19,   47,   30,   13,   93,    3,
 /*   190 */     4,    5,    6,    4,   48,   49,   50,   51,   52,   48,
 /*   200 */    49,   50,   51,   52,   48,   49,   50,   51,   52,   35,
 /*   210 */    82,   83,   79,   82,   83,   26,   17,   84,   82,   83,
 /*   220 */    22,   93,   24,   37,   93,   94,   82,   83,  100,   93,
 /*   230 */    94,   82,   83,   35,   82,   83,   77,   93,   94,   13,
 /*   240 */    82,   83,   93,   30,   31,   93,   78,    5,   22,  100,
 /*   250 */    24,   93,  100,    3,    4,    5,    6,    0,   17,   82,
 /*   260 */    83,   82,   83,   82,   83,   87,   17,   18,   90,   91,
 /*   270 */    93,   29,   93,   32,   93,   82,   83,   82,   83,   82,
 /*   280 */    83,   32,   17,   18,   17,   73,   93,   12,   93,   23,
 /*   290 */    93,   79,    1,    2,   28,   38,   84,   32,   30,   32,
 /*   300 */    20,    1,    2,   23,   19,   72,    5,    6,   22,   63,
 /*   310 */    59,   20,   60,   88,   45,   44,   27,   49,   87,   32,
 /*   320 */    17,   91,    5,   89,   39,   88,   87,   18,   92,   84,
 /*   330 */    92,   87,   13,   75,   60,   59,   40,   35,   60,   62,
 /*   340 */    61,   59,   34,   62,   61,   18,   86,   18,   60,   59,
 /*   350 */    66,   23,   60,   59,   18,   62,   61,   37,   27,   62,
 /*   360 */    62,   18,   20,   18,   15,   61,   14,   23,    2,    4,
 /*   370 */    30,   18,   31,   18,   28,   28,   28,   20,   20,   28,
 /*   380 */    30,   25,   20,   23,   17,    7,   23,   33,   20,   18,
 /*   390 */    18,   30,   23,   23,   30,   18,  101,   18,   39,  101,
 /*   400 */   101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
 /*   410 */   101,  101,  101,  101,   41,  101,  101,  101,  101,  101,
 /*   420 */   101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
 /*   430 */   101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
 /*   440 */   101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
 /*   450 */   101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
 /*   460 */   101,  101,  101,  101,  101,  101,  101,  101,
};
#define YY_SHIFT_COUNT    (143)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (379)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    89,   33,   46,   88,  146,  226,   88,  146,  146,  151,
 /*    10 */   151,  151,  151,  146,  146,  174,  146,  146,  146,  146,
 /*    20 */   146,  146,  146,  146,  249,  198,  241,    6,    6,    6,
 /*    30 */    13,   64,    6,    6,   13,   80,  138,  156,  265,   81,
 /*    40 */   189,  189,  242,  129,  267,  189,  257,  165,  199,  275,
 /*    50 */   286,  269,  271,  289,  287,  303,  317,  289,  287,  309,
 /*    60 */   309,  287,    6,  319,  269,  271,  296,  302,  269,  271,
 /*    70 */   296,  302,  308,  327,  269,  271,  269,  271,  296,  302,
 /*    80 */   296,  296,  302,  125,  186,  148,  250,  250,  250,  250,
 /*    90 */   291,  140,  266,  300,  213,  268,  280,  285,  301,  301,
 /*   100 */   329,  328,  336,  320,  331,  343,  342,  345,  349,  352,
 /*   110 */   344,  366,  365,  346,  353,  347,  348,  340,  350,  341,
 /*   120 */   351,  356,  357,  358,  355,  362,  360,  367,  354,  368,
 /*   130 */   371,  344,  372,  363,  359,  378,  369,  361,  364,  370,
 /*   140 */   377,  370,  379,  373,
};
#define YY_REDUCE_COUNT (82)
#define YY_REDUCE_MIN   (-94)
#define YY_REDUCE_MAX   (304)
static const short yy_reduce_ofst[] = {
 /*     0 */   -33,  -56,  -35,  -30,   21,   48,  -53,   74,  128,  -23,
 /*    10 */   131,  136,  144,  149,  152,  105,   95,  158,  177,  179,
 /*    20 */   181,  193,  195,  197,   84,  -37,  178,   -4,  212,   -4,
 /*    30 */    52,   56,   29,  133,  -74,  -94,  -94,  -83,  -70,  -29,
 /*    40 */   -13,  -13,  -15,  -31,   -6,  -13,  102,  168,  159,  233,
 /*    50 */   246,  252,  251,  225,  231,  230,  234,  237,  239,  236,
 /*    60 */   238,  244,  245,  258,  274,  276,  277,  279,  278,  282,
 /*    70 */   281,  283,  284,  260,  288,  290,  292,  294,  293,  295,
 /*    80 */   297,  298,  304,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   384,  384,  354,  354,  354,  384,  354,  354,  454,  354,
 /*    10 */   354,  354,  354,  454,  454,  384,  354,  354,  354,  354,
 /*    20 */   354,  354,  354,  354,  425,  354,  425,  390,  354,  354,
 /*    30 */   354,  354,  354,  354,  354,  354,  354,  354,  425,  378,
 /*    40 */   394,  401,  419,  425,  425,  402,  354,  354,  354,  380,
 /*    50 */   376,  465,  463,  354,  425,  354,  419,  354,  425,  354,
 /*    60 */   354,  425,  354,  385,  465,  463,  459,  371,  465,  463,
 /*    70 */   459,  369,  429,  354,  465,  463,  465,  463,  459,  354,
 /*    80 */   459,  459,  354,  354,  440,  354,  431,  398,  455,  456,
 /*    90 */   354,  460,  354,  430,  424,  354,  354,  457,  448,  447,
 /*   100 */   354,  442,  354,  354,  354,  354,  354,  354,  354,  354,
 /*   110 */   383,  434,  354,  354,  354,  354,  354,  354,  421,  423,
 /*   120 */   354,  354,  354,  354,  354,  354,  427,  354,  354,  354,
 /*   130 */   354,  388,  354,  403,  457,  354,  395,  354,  354,  436,
 /*   140 */   354,  435,  354,  354,
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
  /*   29 */ "PIPE",
  /*   30 */ "INTEGER",
  /*   31 */ "DOTDOT",
  /*   32 */ "LEFT_CURLY_BRACKET",
  /*   33 */ "RIGHT_CURLY_BRACKET",
  /*   34 */ "WHERE",
  /*   35 */ "RETURN",
  /*   36 */ "DISTINCT",
  /*   37 */ "AS",
  /*   38 */ "WITH",
  /*   39 */ "DOT",
  /*   40 */ "ORDER",
  /*   41 */ "BY",
  /*   42 */ "ASC",
  /*   43 */ "DESC",
  /*   44 */ "SKIP",
  /*   45 */ "LIMIT",
  /*   46 */ "UNWIND",
  /*   47 */ "NE",
  /*   48 */ "STRING",
  /*   49 */ "FLOAT",
  /*   50 */ "TRUE",
  /*   51 */ "FALSE",
  /*   52 */ "NULLVAL",
  /*   53 */ "error",
  /*   54 */ "query",
  /*   55 */ "expressions",
  /*   56 */ "expr",
  /*   57 */ "withClause",
  /*   58 */ "singlePartQuery",
  /*   59 */ "skipClause",
  /*   60 */ "limitClause",
  /*   61 */ "returnClause",
  /*   62 */ "orderClause",
  /*   63 */ "setClause",
  /*   64 */ "deleteClause",
  /*   65 */ "multipleMatchClause",
  /*   66 */ "whereClause",
  /*   67 */ "multipleCreateClause",
  /*   68 */ "unwindClause",
  /*   69 */ "indexClause",
  /*   70 */ "mergeClause",
  /*   71 */ "matchClauses",
  /*   72 */ "matchClause",
  /*   73 */ "chains",
  /*   74 */ "createClauses",
  /*   75 */ "createClause",
  /*   76 */ "indexOpToken",
  /*   77 */ "indexLabel",
  /*   78 */ "indexProp",
  /*   79 */ "chain",
  /*   80 */ "setList",
  /*   81 */ "setElement",
  /*   82 */ "variable",
  /*   83 */ "arithmetic_expression",
  /*   84 */ "node",
  /*   85 */ "link",
  /*   86 */ "deleteExpression",
  /*   87 */ "properties",
  /*   88 */ "edge",
  /*   89 */ "edgeLength",
  /*   90 */ "edgeLabels",
  /*   91 */ "edgeLabel",
  /*   92 */ "mapLiteral",
  /*   93 */ "value",
  /*   94 */ "cond",
  /*   95 */ "relation",
  /*   96 */ "returnElements",
  /*   97 */ "returnElement",
  /*   98 */ "withElements",
  /*   99 */ "withElement",
  /*  100 */ "arithmetic_expression_list",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "query ::= expressions",
 /*   1 */ "expressions ::= expr",
 /*   2 */ "expressions ::= expressions withClause singlePartQuery",
 /*   3 */ "singlePartQuery ::= expr",
 /*   4 */ "singlePartQuery ::= skipClause limitClause returnClause orderClause",
 /*   5 */ "singlePartQuery ::= limitClause returnClause orderClause",
 /*   6 */ "singlePartQuery ::= skipClause returnClause orderClause",
 /*   7 */ "singlePartQuery ::= returnClause orderClause skipClause limitClause",
 /*   8 */ "singlePartQuery ::= orderClause skipClause limitClause returnClause",
 /*   9 */ "singlePartQuery ::= orderClause skipClause limitClause setClause",
 /*  10 */ "singlePartQuery ::= orderClause skipClause limitClause deleteClause",
 /*  11 */ "expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause",
 /*  12 */ "expr ::= multipleMatchClause whereClause multipleCreateClause",
 /*  13 */ "expr ::= multipleMatchClause whereClause deleteClause",
 /*  14 */ "expr ::= multipleMatchClause whereClause setClause",
 /*  15 */ "expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause",
 /*  16 */ "expr ::= multipleCreateClause",
 /*  17 */ "expr ::= unwindClause multipleCreateClause",
 /*  18 */ "expr ::= indexClause",
 /*  19 */ "expr ::= mergeClause",
 /*  20 */ "expr ::= mergeClause setClause",
 /*  21 */ "expr ::= returnClause",
 /*  22 */ "expr ::= unwindClause returnClause skipClause limitClause",
 /*  23 */ "multipleMatchClause ::= matchClauses",
 /*  24 */ "matchClauses ::= matchClause",
 /*  25 */ "matchClauses ::= matchClauses matchClause",
 /*  26 */ "matchClause ::= MATCH chains",
 /*  27 */ "multipleCreateClause ::=",
 /*  28 */ "multipleCreateClause ::= createClauses",
 /*  29 */ "createClauses ::= createClause",
 /*  30 */ "createClauses ::= createClauses createClause",
 /*  31 */ "createClause ::= CREATE chains",
 /*  32 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  33 */ "indexOpToken ::= CREATE",
 /*  34 */ "indexOpToken ::= DROP",
 /*  35 */ "indexLabel ::= COLON UQSTRING",
 /*  36 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  37 */ "mergeClause ::= MERGE chain",
 /*  38 */ "setClause ::= SET setList",
 /*  39 */ "setList ::= setElement",
 /*  40 */ "setList ::= setList COMMA setElement",
 /*  41 */ "setElement ::= variable EQ arithmetic_expression",
 /*  42 */ "chain ::= node",
 /*  43 */ "chain ::= chain link node",
 /*  44 */ "chains ::= chain",
 /*  45 */ "chains ::= chains COMMA chain",
 /*  46 */ "deleteClause ::= DELETE deleteExpression",
 /*  47 */ "deleteExpression ::= UQSTRING",
 /*  48 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  49 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  50 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  51 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  52 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  53 */ "link ::= DASH edge RIGHT_ARROW",
 /*  54 */ "link ::= LEFT_ARROW edge DASH",
 /*  55 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  56 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  57 */ "edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET",
 /*  58 */ "edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET",
 /*  59 */ "edgeLabel ::= COLON UQSTRING",
 /*  60 */ "edgeLabels ::= edgeLabel",
 /*  61 */ "edgeLabels ::= edgeLabels PIPE edgeLabel",
 /*  62 */ "edgeLength ::=",
 /*  63 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  64 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  65 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  66 */ "edgeLength ::= MUL INTEGER",
 /*  67 */ "edgeLength ::= MUL",
 /*  68 */ "properties ::=",
 /*  69 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  70 */ "mapLiteral ::= UQSTRING COLON value",
 /*  71 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  72 */ "whereClause ::=",
 /*  73 */ "whereClause ::= WHERE cond",
 /*  74 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  75 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  76 */ "cond ::= cond AND cond",
 /*  77 */ "cond ::= cond OR cond",
 /*  78 */ "returnClause ::= RETURN returnElements",
 /*  79 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  80 */ "returnElements ::= returnElements COMMA returnElement",
 /*  81 */ "returnElements ::= returnElement",
 /*  82 */ "returnElement ::= MUL",
 /*  83 */ "returnElement ::= arithmetic_expression",
 /*  84 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  85 */ "withClause ::= WITH withElements",
 /*  86 */ "withElements ::= withElement",
 /*  87 */ "withElements ::= withElements COMMA withElement",
 /*  88 */ "withElement ::= arithmetic_expression AS UQSTRING",
 /*  89 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /*  90 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /*  91 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /*  92 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /*  93 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /*  94 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /*  95 */ "arithmetic_expression ::= value",
 /*  96 */ "arithmetic_expression ::= variable",
 /*  97 */ "arithmetic_expression_list ::=",
 /*  98 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /*  99 */ "arithmetic_expression_list ::= arithmetic_expression",
 /* 100 */ "variable ::= UQSTRING",
 /* 101 */ "variable ::= UQSTRING DOT UQSTRING",
 /* 102 */ "orderClause ::=",
 /* 103 */ "orderClause ::= ORDER BY arithmetic_expression_list",
 /* 104 */ "orderClause ::= ORDER BY arithmetic_expression_list ASC",
 /* 105 */ "orderClause ::= ORDER BY arithmetic_expression_list DESC",
 /* 106 */ "skipClause ::=",
 /* 107 */ "skipClause ::= SKIP INTEGER",
 /* 108 */ "limitClause ::=",
 /* 109 */ "limitClause ::= LIMIT INTEGER",
 /* 110 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /* 111 */ "relation ::= EQ",
 /* 112 */ "relation ::= GT",
 /* 113 */ "relation ::= LT",
 /* 114 */ "relation ::= LE",
 /* 115 */ "relation ::= GE",
 /* 116 */ "relation ::= NE",
 /* 117 */ "value ::= INTEGER",
 /* 118 */ "value ::= DASH INTEGER",
 /* 119 */ "value ::= STRING",
 /* 120 */ "value ::= FLOAT",
 /* 121 */ "value ::= DASH FLOAT",
 /* 122 */ "value ::= TRUE",
 /* 123 */ "value ::= FALSE",
 /* 124 */ "value ::= NULLVAL",
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
    case 94: /* cond */
{
#line 439 "grammar.y"
 Free_AST_FilterNode((yypminor->yy153)); 
#line 841 "grammar.c"
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
  {   54,   -1 }, /* (0) query ::= expressions */
  {   55,   -1 }, /* (1) expressions ::= expr */
  {   55,   -3 }, /* (2) expressions ::= expressions withClause singlePartQuery */
  {   58,   -1 }, /* (3) singlePartQuery ::= expr */
  {   58,   -4 }, /* (4) singlePartQuery ::= skipClause limitClause returnClause orderClause */
  {   58,   -3 }, /* (5) singlePartQuery ::= limitClause returnClause orderClause */
  {   58,   -3 }, /* (6) singlePartQuery ::= skipClause returnClause orderClause */
  {   58,   -4 }, /* (7) singlePartQuery ::= returnClause orderClause skipClause limitClause */
  {   58,   -4 }, /* (8) singlePartQuery ::= orderClause skipClause limitClause returnClause */
  {   58,   -4 }, /* (9) singlePartQuery ::= orderClause skipClause limitClause setClause */
  {   58,   -4 }, /* (10) singlePartQuery ::= orderClause skipClause limitClause deleteClause */
  {   56,   -7 }, /* (11) expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
  {   56,   -3 }, /* (12) expr ::= multipleMatchClause whereClause multipleCreateClause */
  {   56,   -3 }, /* (13) expr ::= multipleMatchClause whereClause deleteClause */
  {   56,   -3 }, /* (14) expr ::= multipleMatchClause whereClause setClause */
  {   56,   -7 }, /* (15) expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   56,   -1 }, /* (16) expr ::= multipleCreateClause */
  {   56,   -2 }, /* (17) expr ::= unwindClause multipleCreateClause */
  {   56,   -1 }, /* (18) expr ::= indexClause */
  {   56,   -1 }, /* (19) expr ::= mergeClause */
  {   56,   -2 }, /* (20) expr ::= mergeClause setClause */
  {   56,   -1 }, /* (21) expr ::= returnClause */
  {   56,   -4 }, /* (22) expr ::= unwindClause returnClause skipClause limitClause */
  {   65,   -1 }, /* (23) multipleMatchClause ::= matchClauses */
  {   71,   -1 }, /* (24) matchClauses ::= matchClause */
  {   71,   -2 }, /* (25) matchClauses ::= matchClauses matchClause */
  {   72,   -2 }, /* (26) matchClause ::= MATCH chains */
  {   67,    0 }, /* (27) multipleCreateClause ::= */
  {   67,   -1 }, /* (28) multipleCreateClause ::= createClauses */
  {   74,   -1 }, /* (29) createClauses ::= createClause */
  {   74,   -2 }, /* (30) createClauses ::= createClauses createClause */
  {   75,   -2 }, /* (31) createClause ::= CREATE chains */
  {   69,   -5 }, /* (32) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   76,   -1 }, /* (33) indexOpToken ::= CREATE */
  {   76,   -1 }, /* (34) indexOpToken ::= DROP */
  {   77,   -2 }, /* (35) indexLabel ::= COLON UQSTRING */
  {   78,   -3 }, /* (36) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   70,   -2 }, /* (37) mergeClause ::= MERGE chain */
  {   63,   -2 }, /* (38) setClause ::= SET setList */
  {   80,   -1 }, /* (39) setList ::= setElement */
  {   80,   -3 }, /* (40) setList ::= setList COMMA setElement */
  {   81,   -3 }, /* (41) setElement ::= variable EQ arithmetic_expression */
  {   79,   -1 }, /* (42) chain ::= node */
  {   79,   -3 }, /* (43) chain ::= chain link node */
  {   73,   -1 }, /* (44) chains ::= chain */
  {   73,   -3 }, /* (45) chains ::= chains COMMA chain */
  {   64,   -2 }, /* (46) deleteClause ::= DELETE deleteExpression */
  {   86,   -1 }, /* (47) deleteExpression ::= UQSTRING */
  {   86,   -3 }, /* (48) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   84,   -6 }, /* (49) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   84,   -5 }, /* (50) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   84,   -4 }, /* (51) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   84,   -3 }, /* (52) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   85,   -3 }, /* (53) link ::= DASH edge RIGHT_ARROW */
  {   85,   -3 }, /* (54) link ::= LEFT_ARROW edge DASH */
  {   88,   -4 }, /* (55) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   88,   -4 }, /* (56) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   88,   -5 }, /* (57) edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
  {   88,   -5 }, /* (58) edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
  {   91,   -2 }, /* (59) edgeLabel ::= COLON UQSTRING */
  {   90,   -1 }, /* (60) edgeLabels ::= edgeLabel */
  {   90,   -3 }, /* (61) edgeLabels ::= edgeLabels PIPE edgeLabel */
  {   89,    0 }, /* (62) edgeLength ::= */
  {   89,   -4 }, /* (63) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   89,   -3 }, /* (64) edgeLength ::= MUL INTEGER DOTDOT */
  {   89,   -3 }, /* (65) edgeLength ::= MUL DOTDOT INTEGER */
  {   89,   -2 }, /* (66) edgeLength ::= MUL INTEGER */
  {   89,   -1 }, /* (67) edgeLength ::= MUL */
  {   87,    0 }, /* (68) properties ::= */
  {   87,   -3 }, /* (69) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   92,   -3 }, /* (70) mapLiteral ::= UQSTRING COLON value */
  {   92,   -5 }, /* (71) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   66,    0 }, /* (72) whereClause ::= */
  {   66,   -2 }, /* (73) whereClause ::= WHERE cond */
  {   94,   -3 }, /* (74) cond ::= arithmetic_expression relation arithmetic_expression */
  {   94,   -3 }, /* (75) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {   94,   -3 }, /* (76) cond ::= cond AND cond */
  {   94,   -3 }, /* (77) cond ::= cond OR cond */
  {   61,   -2 }, /* (78) returnClause ::= RETURN returnElements */
  {   61,   -3 }, /* (79) returnClause ::= RETURN DISTINCT returnElements */
  {   96,   -3 }, /* (80) returnElements ::= returnElements COMMA returnElement */
  {   96,   -1 }, /* (81) returnElements ::= returnElement */
  {   97,   -1 }, /* (82) returnElement ::= MUL */
  {   97,   -1 }, /* (83) returnElement ::= arithmetic_expression */
  {   97,   -3 }, /* (84) returnElement ::= arithmetic_expression AS UQSTRING */
  {   57,   -2 }, /* (85) withClause ::= WITH withElements */
  {   98,   -1 }, /* (86) withElements ::= withElement */
  {   98,   -3 }, /* (87) withElements ::= withElements COMMA withElement */
  {   99,   -3 }, /* (88) withElement ::= arithmetic_expression AS UQSTRING */
  {   83,   -3 }, /* (89) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   83,   -3 }, /* (90) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   83,   -3 }, /* (91) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   83,   -3 }, /* (92) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   83,   -3 }, /* (93) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   83,   -4 }, /* (94) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   83,   -1 }, /* (95) arithmetic_expression ::= value */
  {   83,   -1 }, /* (96) arithmetic_expression ::= variable */
  {  100,    0 }, /* (97) arithmetic_expression_list ::= */
  {  100,   -3 }, /* (98) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {  100,   -1 }, /* (99) arithmetic_expression_list ::= arithmetic_expression */
  {   82,   -1 }, /* (100) variable ::= UQSTRING */
  {   82,   -3 }, /* (101) variable ::= UQSTRING DOT UQSTRING */
  {   62,    0 }, /* (102) orderClause ::= */
  {   62,   -3 }, /* (103) orderClause ::= ORDER BY arithmetic_expression_list */
  {   62,   -4 }, /* (104) orderClause ::= ORDER BY arithmetic_expression_list ASC */
  {   62,   -4 }, /* (105) orderClause ::= ORDER BY arithmetic_expression_list DESC */
  {   59,    0 }, /* (106) skipClause ::= */
  {   59,   -2 }, /* (107) skipClause ::= SKIP INTEGER */
  {   60,    0 }, /* (108) limitClause ::= */
  {   60,   -2 }, /* (109) limitClause ::= LIMIT INTEGER */
  {   68,   -6 }, /* (110) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {   95,   -1 }, /* (111) relation ::= EQ */
  {   95,   -1 }, /* (112) relation ::= GT */
  {   95,   -1 }, /* (113) relation ::= LT */
  {   95,   -1 }, /* (114) relation ::= LE */
  {   95,   -1 }, /* (115) relation ::= GE */
  {   95,   -1 }, /* (116) relation ::= NE */
  {   93,   -1 }, /* (117) value ::= INTEGER */
  {   93,   -2 }, /* (118) value ::= DASH INTEGER */
  {   93,   -1 }, /* (119) value ::= STRING */
  {   93,   -1 }, /* (120) value ::= FLOAT */
  {   93,   -2 }, /* (121) value ::= DASH FLOAT */
  {   93,   -1 }, /* (122) value ::= TRUE */
  {   93,   -1 }, /* (123) value ::= FALSE */
  {   93,   -1 }, /* (124) value ::= NULLVAL */
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
      case 0: /* query ::= expressions */
#line 43 "grammar.y"
{ ctx->root = yymsp[0].minor.yy147; }
#line 1343 "grammar.c"
        break;
      case 1: /* expressions ::= expr */
#line 47 "grammar.y"
{
	yylhsminor.yy147 = array_new(AST*, 1);
	yylhsminor.yy147 = array_append(yylhsminor.yy147, yymsp[0].minor.yy83);
}
#line 1351 "grammar.c"
  yymsp[0].minor.yy147 = yylhsminor.yy147;
        break;
      case 2: /* expressions ::= expressions withClause singlePartQuery */
#line 52 "grammar.y"
{
	AST *ast = yymsp[-2].minor.yy147[array_len(yymsp[-2].minor.yy147)-1];
	ast->withNode = yymsp[-1].minor.yy48;
	yylhsminor.yy147 = array_append(yymsp[-2].minor.yy147, yymsp[0].minor.yy83);
	yylhsminor.yy147=yymsp[-2].minor.yy147;
}
#line 1362 "grammar.c"
  yymsp[-2].minor.yy147 = yylhsminor.yy147;
        break;
      case 3: /* singlePartQuery ::= expr */
#line 60 "grammar.y"
{
	yylhsminor.yy83 = yymsp[0].minor.yy83;
}
#line 1370 "grammar.c"
  yymsp[0].minor.yy83 = yylhsminor.yy83;
        break;
      case 4: /* singlePartQuery ::= skipClause limitClause returnClause orderClause */
#line 64 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy162, yymsp[0].minor.yy114, yymsp[-3].minor.yy137, yymsp[-2].minor.yy65, NULL, NULL);
}
#line 1378 "grammar.c"
  yymsp[-3].minor.yy83 = yylhsminor.yy83;
        break;
      case 5: /* singlePartQuery ::= limitClause returnClause orderClause */
#line 68 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy162, yymsp[0].minor.yy114, NULL, yymsp[-2].minor.yy65, NULL, NULL);
}
#line 1386 "grammar.c"
  yymsp[-2].minor.yy83 = yylhsminor.yy83;
        break;
      case 6: /* singlePartQuery ::= skipClause returnClause orderClause */
#line 72 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy162, yymsp[0].minor.yy114, yymsp[-2].minor.yy137, NULL, NULL, NULL);
}
#line 1394 "grammar.c"
  yymsp[-2].minor.yy83 = yylhsminor.yy83;
        break;
      case 7: /* singlePartQuery ::= returnClause orderClause skipClause limitClause */
#line 76 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-3].minor.yy162, yymsp[-2].minor.yy114, yymsp[-1].minor.yy137, yymsp[0].minor.yy65, NULL, NULL);
}
#line 1402 "grammar.c"
  yymsp[-3].minor.yy83 = yylhsminor.yy83;
        break;
      case 8: /* singlePartQuery ::= orderClause skipClause limitClause returnClause */
#line 80 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy162, yymsp[-3].minor.yy114, yymsp[-2].minor.yy137, yymsp[-1].minor.yy65, NULL, NULL);
}
#line 1410 "grammar.c"
  yymsp[-3].minor.yy83 = yylhsminor.yy83;
        break;
      case 9: /* singlePartQuery ::= orderClause skipClause limitClause setClause */
#line 84 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, yymsp[0].minor.yy200, NULL, NULL, yymsp[-3].minor.yy114, yymsp[-2].minor.yy137, yymsp[-1].minor.yy65, NULL, NULL);
}
#line 1418 "grammar.c"
  yymsp[-3].minor.yy83 = yylhsminor.yy83;
        break;
      case 10: /* singlePartQuery ::= orderClause skipClause limitClause deleteClause */
#line 88 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy179, NULL, yymsp[-3].minor.yy114, yymsp[-2].minor.yy137, yymsp[-1].minor.yy65, NULL, NULL);
}
#line 1426 "grammar.c"
  yymsp[-3].minor.yy83 = yylhsminor.yy83;
        break;
      case 11: /* expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
#line 93 "grammar.y"
{
	yylhsminor.yy83 = AST_New(yymsp[-6].minor.yy161, yymsp[-5].minor.yy11, yymsp[-4].minor.yy40, NULL, NULL, NULL, yymsp[-3].minor.yy162, yymsp[-2].minor.yy114, yymsp[-1].minor.yy137, yymsp[0].minor.yy65, NULL, NULL);
}
#line 1434 "grammar.c"
  yymsp[-6].minor.yy83 = yylhsminor.yy83;
        break;
      case 12: /* expr ::= multipleMatchClause whereClause multipleCreateClause */
#line 97 "grammar.y"
{
	yylhsminor.yy83 = AST_New(yymsp[-2].minor.yy161, yymsp[-1].minor.yy11, yymsp[0].minor.yy40, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1442 "grammar.c"
  yymsp[-2].minor.yy83 = yylhsminor.yy83;
        break;
      case 13: /* expr ::= multipleMatchClause whereClause deleteClause */
#line 101 "grammar.y"
{
	yylhsminor.yy83 = AST_New(yymsp[-2].minor.yy161, yymsp[-1].minor.yy11, NULL, NULL, NULL, yymsp[0].minor.yy179, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1450 "grammar.c"
  yymsp[-2].minor.yy83 = yylhsminor.yy83;
        break;
      case 14: /* expr ::= multipleMatchClause whereClause setClause */
#line 105 "grammar.y"
{
	yylhsminor.yy83 = AST_New(yymsp[-2].minor.yy161, yymsp[-1].minor.yy11, NULL, NULL, yymsp[0].minor.yy200, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1458 "grammar.c"
  yymsp[-2].minor.yy83 = yylhsminor.yy83;
        break;
      case 15: /* expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 109 "grammar.y"
{
	yylhsminor.yy83 = AST_New(yymsp[-6].minor.yy161, yymsp[-5].minor.yy11, NULL, NULL, yymsp[-4].minor.yy200, NULL, yymsp[-3].minor.yy162, yymsp[-2].minor.yy114, yymsp[-1].minor.yy137, yymsp[0].minor.yy65, NULL, NULL);
}
#line 1466 "grammar.c"
  yymsp[-6].minor.yy83 = yylhsminor.yy83;
        break;
      case 16: /* expr ::= multipleCreateClause */
#line 113 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, yymsp[0].minor.yy40, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1474 "grammar.c"
  yymsp[0].minor.yy83 = yylhsminor.yy83;
        break;
      case 17: /* expr ::= unwindClause multipleCreateClause */
#line 117 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, yymsp[0].minor.yy40, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy169);
}
#line 1482 "grammar.c"
  yymsp[-1].minor.yy83 = yylhsminor.yy83;
        break;
      case 18: /* expr ::= indexClause */
#line 121 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy82, NULL);
}
#line 1490 "grammar.c"
  yymsp[0].minor.yy83 = yylhsminor.yy83;
        break;
      case 19: /* expr ::= mergeClause */
#line 125 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, yymsp[0].minor.yy152, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1498 "grammar.c"
  yymsp[0].minor.yy83 = yylhsminor.yy83;
        break;
      case 20: /* expr ::= mergeClause setClause */
#line 129 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, yymsp[-1].minor.yy152, yymsp[0].minor.yy200, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1506 "grammar.c"
  yymsp[-1].minor.yy83 = yylhsminor.yy83;
        break;
      case 21: /* expr ::= returnClause */
#line 133 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy162, NULL, NULL, NULL, NULL, NULL);
}
#line 1514 "grammar.c"
  yymsp[0].minor.yy83 = yylhsminor.yy83;
        break;
      case 22: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 137 "grammar.y"
{
	yylhsminor.yy83 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy162, NULL, yymsp[-1].minor.yy137, yymsp[0].minor.yy65, NULL, yymsp[-3].minor.yy169);
}
#line 1522 "grammar.c"
  yymsp[-3].minor.yy83 = yylhsminor.yy83;
        break;
      case 23: /* multipleMatchClause ::= matchClauses */
#line 142 "grammar.y"
{
	yylhsminor.yy161 = New_AST_MatchNode(yymsp[0].minor.yy202);
}
#line 1530 "grammar.c"
  yymsp[0].minor.yy161 = yylhsminor.yy161;
        break;
      case 24: /* matchClauses ::= matchClause */
      case 29: /* createClauses ::= createClause */ yytestcase(yyruleno==29);
#line 148 "grammar.y"
{
	yylhsminor.yy202 = yymsp[0].minor.yy202;
}
#line 1539 "grammar.c"
  yymsp[0].minor.yy202 = yylhsminor.yy202;
        break;
      case 25: /* matchClauses ::= matchClauses matchClause */
      case 30: /* createClauses ::= createClauses createClause */ yytestcase(yyruleno==30);
#line 152 "grammar.y"
{
	Vector *v;
	while(Vector_Pop(yymsp[0].minor.yy202, &v)) Vector_Push(yymsp[-1].minor.yy202, v);
	Vector_Free(yymsp[0].minor.yy202);
	yylhsminor.yy202 = yymsp[-1].minor.yy202;
}
#line 1551 "grammar.c"
  yymsp[-1].minor.yy202 = yylhsminor.yy202;
        break;
      case 26: /* matchClause ::= MATCH chains */
      case 31: /* createClause ::= CREATE chains */ yytestcase(yyruleno==31);
#line 161 "grammar.y"
{
	yymsp[-1].minor.yy202 = yymsp[0].minor.yy202;
}
#line 1560 "grammar.c"
        break;
      case 27: /* multipleCreateClause ::= */
#line 166 "grammar.y"
{
	yymsp[1].minor.yy40 = NULL;
}
#line 1567 "grammar.c"
        break;
      case 28: /* multipleCreateClause ::= createClauses */
#line 170 "grammar.y"
{
	yylhsminor.yy40 = New_AST_CreateNode(yymsp[0].minor.yy202);
}
#line 1574 "grammar.c"
  yymsp[0].minor.yy40 = yylhsminor.yy40;
        break;
      case 32: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 196 "grammar.y"
{
  yylhsminor.yy82 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy187);
}
#line 1582 "grammar.c"
  yymsp[-4].minor.yy82 = yylhsminor.yy82;
        break;
      case 33: /* indexOpToken ::= CREATE */
#line 202 "grammar.y"
{ yymsp[0].minor.yy187 = CREATE_INDEX; }
#line 1588 "grammar.c"
        break;
      case 34: /* indexOpToken ::= DROP */
#line 203 "grammar.y"
{ yymsp[0].minor.yy187 = DROP_INDEX; }
#line 1593 "grammar.c"
        break;
      case 35: /* indexLabel ::= COLON UQSTRING */
#line 205 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1600 "grammar.c"
        break;
      case 36: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 209 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1607 "grammar.c"
        break;
      case 37: /* mergeClause ::= MERGE chain */
#line 215 "grammar.y"
{
	yymsp[-1].minor.yy152 = New_AST_MergeNode(yymsp[0].minor.yy202);
}
#line 1614 "grammar.c"
        break;
      case 38: /* setClause ::= SET setList */
#line 220 "grammar.y"
{
	yymsp[-1].minor.yy200 = New_AST_SetNode(yymsp[0].minor.yy202);
}
#line 1621 "grammar.c"
        break;
      case 39: /* setList ::= setElement */
#line 225 "grammar.y"
{
	yylhsminor.yy202 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy202, yymsp[0].minor.yy98);
}
#line 1629 "grammar.c"
  yymsp[0].minor.yy202 = yylhsminor.yy202;
        break;
      case 40: /* setList ::= setList COMMA setElement */
#line 229 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy202, yymsp[0].minor.yy98);
	yylhsminor.yy202 = yymsp[-2].minor.yy202;
}
#line 1638 "grammar.c"
  yymsp[-2].minor.yy202 = yylhsminor.yy202;
        break;
      case 41: /* setElement ::= variable EQ arithmetic_expression */
#line 235 "grammar.y"
{
	yylhsminor.yy98 = New_AST_SetElement(yymsp[-2].minor.yy154, yymsp[0].minor.yy102);
}
#line 1646 "grammar.c"
  yymsp[-2].minor.yy98 = yylhsminor.yy98;
        break;
      case 42: /* chain ::= node */
#line 241 "grammar.y"
{
	yylhsminor.yy202 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy202, yymsp[0].minor.yy71);
}
#line 1655 "grammar.c"
  yymsp[0].minor.yy202 = yylhsminor.yy202;
        break;
      case 43: /* chain ::= chain link node */
#line 246 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy202, yymsp[-1].minor.yy105);
	Vector_Push(yymsp[-2].minor.yy202, yymsp[0].minor.yy71);
	yylhsminor.yy202 = yymsp[-2].minor.yy202;
}
#line 1665 "grammar.c"
  yymsp[-2].minor.yy202 = yylhsminor.yy202;
        break;
      case 44: /* chains ::= chain */
#line 254 "grammar.y"
{
	yylhsminor.yy202 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy202, yymsp[0].minor.yy202);
}
#line 1674 "grammar.c"
  yymsp[0].minor.yy202 = yylhsminor.yy202;
        break;
      case 45: /* chains ::= chains COMMA chain */
#line 259 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy202, yymsp[0].minor.yy202);
	yylhsminor.yy202 = yymsp[-2].minor.yy202;
}
#line 1683 "grammar.c"
  yymsp[-2].minor.yy202 = yylhsminor.yy202;
        break;
      case 46: /* deleteClause ::= DELETE deleteExpression */
#line 267 "grammar.y"
{
	yymsp[-1].minor.yy179 = New_AST_DeleteNode(yymsp[0].minor.yy202);
}
#line 1691 "grammar.c"
        break;
      case 47: /* deleteExpression ::= UQSTRING */
#line 273 "grammar.y"
{
	yylhsminor.yy202 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy202, yymsp[0].minor.yy0.strval);
}
#line 1699 "grammar.c"
  yymsp[0].minor.yy202 = yylhsminor.yy202;
        break;
      case 48: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 278 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy202, yymsp[0].minor.yy0.strval);
	yylhsminor.yy202 = yymsp[-2].minor.yy202;
}
#line 1708 "grammar.c"
  yymsp[-2].minor.yy202 = yylhsminor.yy202;
        break;
      case 49: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 286 "grammar.y"
{
	yymsp[-5].minor.yy71 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy202);
}
#line 1716 "grammar.c"
        break;
      case 50: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 291 "grammar.y"
{
	yymsp[-4].minor.yy71 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy202);
}
#line 1723 "grammar.c"
        break;
      case 51: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 296 "grammar.y"
{
	yymsp[-3].minor.yy71 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy202);
}
#line 1730 "grammar.c"
        break;
      case 52: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 301 "grammar.y"
{
	yymsp[-2].minor.yy71 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy202);
}
#line 1737 "grammar.c"
        break;
      case 53: /* link ::= DASH edge RIGHT_ARROW */
#line 308 "grammar.y"
{
	yymsp[-2].minor.yy105 = yymsp[-1].minor.yy105;
	yymsp[-2].minor.yy105->direction = N_LEFT_TO_RIGHT;
}
#line 1745 "grammar.c"
        break;
      case 54: /* link ::= LEFT_ARROW edge DASH */
#line 314 "grammar.y"
{
	yymsp[-2].minor.yy105 = yymsp[-1].minor.yy105;
	yymsp[-2].minor.yy105->direction = N_RIGHT_TO_LEFT;
}
#line 1753 "grammar.c"
        break;
      case 55: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 321 "grammar.y"
{ 
	yymsp[-3].minor.yy105 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy202, N_DIR_UNKNOWN, yymsp[-1].minor.yy76);
}
#line 1760 "grammar.c"
        break;
      case 56: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 326 "grammar.y"
{ 
	yymsp[-3].minor.yy105 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy202, N_DIR_UNKNOWN, NULL);
}
#line 1767 "grammar.c"
        break;
      case 57: /* edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
#line 331 "grammar.y"
{ 
	yymsp[-4].minor.yy105 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy138, yymsp[-1].minor.yy202, N_DIR_UNKNOWN, yymsp[-2].minor.yy76);
}
#line 1774 "grammar.c"
        break;
      case 58: /* edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
#line 336 "grammar.y"
{ 
	yymsp[-4].minor.yy105 = New_AST_LinkEntity(yymsp[-3].minor.yy0.strval, yymsp[-2].minor.yy138, yymsp[-1].minor.yy202, N_DIR_UNKNOWN, NULL);
}
#line 1781 "grammar.c"
        break;
      case 59: /* edgeLabel ::= COLON UQSTRING */
#line 343 "grammar.y"
{
	yymsp[-1].minor.yy95 = yymsp[0].minor.yy0.strval;
}
#line 1788 "grammar.c"
        break;
      case 60: /* edgeLabels ::= edgeLabel */
#line 348 "grammar.y"
{
	yylhsminor.yy138 = array_new(char*, 1);
	yylhsminor.yy138 = array_append(yylhsminor.yy138, yymsp[0].minor.yy95);
}
#line 1796 "grammar.c"
  yymsp[0].minor.yy138 = yylhsminor.yy138;
        break;
      case 61: /* edgeLabels ::= edgeLabels PIPE edgeLabel */
#line 354 "grammar.y"
{
	char *label = yymsp[0].minor.yy95;
	yymsp[-2].minor.yy138 = array_append(yymsp[-2].minor.yy138, label);
	yylhsminor.yy138 = yymsp[-2].minor.yy138;
}
#line 1806 "grammar.c"
  yymsp[-2].minor.yy138 = yylhsminor.yy138;
        break;
      case 62: /* edgeLength ::= */
#line 363 "grammar.y"
{
	yymsp[1].minor.yy76 = NULL;
}
#line 1814 "grammar.c"
        break;
      case 63: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 368 "grammar.y"
{
	yymsp[-3].minor.yy76 = New_AST_LinkLength(yymsp[-2].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1821 "grammar.c"
        break;
      case 64: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 373 "grammar.y"
{
	yymsp[-2].minor.yy76 = New_AST_LinkLength(yymsp[-1].minor.yy0.longval, UINT_MAX-2);
}
#line 1828 "grammar.c"
        break;
      case 65: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 378 "grammar.y"
{
	yymsp[-2].minor.yy76 = New_AST_LinkLength(1, yymsp[0].minor.yy0.longval);
}
#line 1835 "grammar.c"
        break;
      case 66: /* edgeLength ::= MUL INTEGER */
#line 383 "grammar.y"
{
	yymsp[-1].minor.yy76 = New_AST_LinkLength(yymsp[0].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1842 "grammar.c"
        break;
      case 67: /* edgeLength ::= MUL */
#line 388 "grammar.y"
{
	yymsp[0].minor.yy76 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1849 "grammar.c"
        break;
      case 68: /* properties ::= */
#line 394 "grammar.y"
{
	yymsp[1].minor.yy202 = NULL;
}
#line 1856 "grammar.c"
        break;
      case 69: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 398 "grammar.y"
{
	yymsp[-2].minor.yy202 = yymsp[-1].minor.yy202;
}
#line 1863 "grammar.c"
        break;
      case 70: /* mapLiteral ::= UQSTRING COLON value */
#line 404 "grammar.y"
{
	yylhsminor.yy202 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy202, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy136;
	Vector_Push(yylhsminor.yy202, val);
}
#line 1878 "grammar.c"
  yymsp[-2].minor.yy202 = yylhsminor.yy202;
        break;
      case 71: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 416 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy202, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy136;
	Vector_Push(yymsp[0].minor.yy202, val);
	
	yylhsminor.yy202 = yymsp[0].minor.yy202;
}
#line 1894 "grammar.c"
  yymsp[-4].minor.yy202 = yylhsminor.yy202;
        break;
      case 72: /* whereClause ::= */
#line 430 "grammar.y"
{ 
	yymsp[1].minor.yy11 = NULL;
}
#line 1902 "grammar.c"
        break;
      case 73: /* whereClause ::= WHERE cond */
#line 433 "grammar.y"
{
	yymsp[-1].minor.yy11 = New_AST_WhereNode(yymsp[0].minor.yy153);
}
#line 1909 "grammar.c"
        break;
      case 74: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 442 "grammar.y"
{ yylhsminor.yy153 = New_AST_PredicateNode(yymsp[-2].minor.yy102, yymsp[-1].minor.yy115, yymsp[0].minor.yy102); }
#line 1914 "grammar.c"
  yymsp[-2].minor.yy153 = yylhsminor.yy153;
        break;
      case 75: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 444 "grammar.y"
{ yymsp[-2].minor.yy153 = yymsp[-1].minor.yy153; }
#line 1920 "grammar.c"
        break;
      case 76: /* cond ::= cond AND cond */
#line 445 "grammar.y"
{ yylhsminor.yy153 = New_AST_ConditionNode(yymsp[-2].minor.yy153, AND, yymsp[0].minor.yy153); }
#line 1925 "grammar.c"
  yymsp[-2].minor.yy153 = yylhsminor.yy153;
        break;
      case 77: /* cond ::= cond OR cond */
#line 446 "grammar.y"
{ yylhsminor.yy153 = New_AST_ConditionNode(yymsp[-2].minor.yy153, OR, yymsp[0].minor.yy153); }
#line 1931 "grammar.c"
  yymsp[-2].minor.yy153 = yylhsminor.yy153;
        break;
      case 78: /* returnClause ::= RETURN returnElements */
#line 450 "grammar.y"
{
	yymsp[-1].minor.yy162 = New_AST_ReturnNode(yymsp[0].minor.yy172, 0);
}
#line 1939 "grammar.c"
        break;
      case 79: /* returnClause ::= RETURN DISTINCT returnElements */
#line 453 "grammar.y"
{
	yymsp[-2].minor.yy162 = New_AST_ReturnNode(yymsp[0].minor.yy172, 1);
}
#line 1946 "grammar.c"
        break;
      case 80: /* returnElements ::= returnElements COMMA returnElement */
#line 459 "grammar.y"
{
	yylhsminor.yy172 = array_append(yymsp[-2].minor.yy172, yymsp[0].minor.yy173);
}
#line 1953 "grammar.c"
  yymsp[-2].minor.yy172 = yylhsminor.yy172;
        break;
      case 81: /* returnElements ::= returnElement */
#line 463 "grammar.y"
{
	yylhsminor.yy172 = array_new(AST_ReturnElementNode*, 1);
	array_append(yylhsminor.yy172, yymsp[0].minor.yy173);
}
#line 1962 "grammar.c"
  yymsp[0].minor.yy172 = yylhsminor.yy172;
        break;
      case 82: /* returnElement ::= MUL */
#line 471 "grammar.y"
{
	yymsp[0].minor.yy173 = New_AST_ReturnElementExpandALL();
}
#line 1970 "grammar.c"
        break;
      case 83: /* returnElement ::= arithmetic_expression */
#line 474 "grammar.y"
{
	yylhsminor.yy173 = New_AST_ReturnElementNode(yymsp[0].minor.yy102, NULL);
}
#line 1977 "grammar.c"
  yymsp[0].minor.yy173 = yylhsminor.yy173;
        break;
      case 84: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 478 "grammar.y"
{
	yylhsminor.yy173 = New_AST_ReturnElementNode(yymsp[-2].minor.yy102, yymsp[0].minor.yy0.strval);
}
#line 1985 "grammar.c"
  yymsp[-2].minor.yy173 = yylhsminor.yy173;
        break;
      case 85: /* withClause ::= WITH withElements */
#line 483 "grammar.y"
{
	yymsp[-1].minor.yy48 = New_AST_WithNode(yymsp[0].minor.yy86);
}
#line 1993 "grammar.c"
        break;
      case 86: /* withElements ::= withElement */
#line 488 "grammar.y"
{
	yylhsminor.yy86 = array_new(AST_WithElementNode*, 1);
	array_append(yylhsminor.yy86, yymsp[0].minor.yy174);
}
#line 2001 "grammar.c"
  yymsp[0].minor.yy86 = yylhsminor.yy86;
        break;
      case 87: /* withElements ::= withElements COMMA withElement */
#line 492 "grammar.y"
{
	yylhsminor.yy86 = array_append(yymsp[-2].minor.yy86, yymsp[0].minor.yy174);
}
#line 2009 "grammar.c"
  yymsp[-2].minor.yy86 = yylhsminor.yy86;
        break;
      case 88: /* withElement ::= arithmetic_expression AS UQSTRING */
#line 497 "grammar.y"
{
	yylhsminor.yy174 = New_AST_WithElementNode(yymsp[-2].minor.yy102, yymsp[0].minor.yy0.strval);
}
#line 2017 "grammar.c"
  yymsp[-2].minor.yy174 = yylhsminor.yy174;
        break;
      case 89: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 504 "grammar.y"
{
	yymsp[-2].minor.yy102 = yymsp[-1].minor.yy102;
}
#line 2025 "grammar.c"
        break;
      case 90: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 510 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy102);
	Vector_Push(args, yymsp[0].minor.yy102);
	yylhsminor.yy102 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 2035 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 91: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 517 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy102);
	Vector_Push(args, yymsp[0].minor.yy102);
	yylhsminor.yy102 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 2046 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 92: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 524 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy102);
	Vector_Push(args, yymsp[0].minor.yy102);
	yylhsminor.yy102 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 2057 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 93: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 531 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy102);
	Vector_Push(args, yymsp[0].minor.yy102);
	yylhsminor.yy102 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 2068 "grammar.c"
  yymsp[-2].minor.yy102 = yylhsminor.yy102;
        break;
      case 94: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 539 "grammar.y"
{
	yylhsminor.yy102 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy202);
}
#line 2076 "grammar.c"
  yymsp[-3].minor.yy102 = yylhsminor.yy102;
        break;
      case 95: /* arithmetic_expression ::= value */
#line 544 "grammar.y"
{
	yylhsminor.yy102 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy136);
}
#line 2084 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 96: /* arithmetic_expression ::= variable */
#line 549 "grammar.y"
{
	yylhsminor.yy102 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy154->alias, yymsp[0].minor.yy154->property);
	free(yymsp[0].minor.yy154->alias);
	free(yymsp[0].minor.yy154->property);
	free(yymsp[0].minor.yy154);
}
#line 2095 "grammar.c"
  yymsp[0].minor.yy102 = yylhsminor.yy102;
        break;
      case 97: /* arithmetic_expression_list ::= */
#line 558 "grammar.y"
{
	yymsp[1].minor.yy202 = NewVector(AST_ArithmeticExpressionNode*, 0);
}
#line 2103 "grammar.c"
        break;
      case 98: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 561 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy202, yymsp[0].minor.yy102);
	yylhsminor.yy202 = yymsp[-2].minor.yy202;
}
#line 2111 "grammar.c"
  yymsp[-2].minor.yy202 = yylhsminor.yy202;
        break;
      case 99: /* arithmetic_expression_list ::= arithmetic_expression */
#line 565 "grammar.y"
{
	yylhsminor.yy202 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy202, yymsp[0].minor.yy102);
}
#line 2120 "grammar.c"
  yymsp[0].minor.yy202 = yylhsminor.yy202;
        break;
      case 100: /* variable ::= UQSTRING */
#line 572 "grammar.y"
{
	yylhsminor.yy154 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 2128 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 101: /* variable ::= UQSTRING DOT UQSTRING */
#line 576 "grammar.y"
{
	yylhsminor.yy154 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 2136 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 102: /* orderClause ::= */
#line 582 "grammar.y"
{
	yymsp[1].minor.yy114 = NULL;
}
#line 2144 "grammar.c"
        break;
      case 103: /* orderClause ::= ORDER BY arithmetic_expression_list */
#line 585 "grammar.y"
{
	yymsp[-2].minor.yy114 = New_AST_OrderNode(yymsp[0].minor.yy202, ORDER_DIR_ASC);
}
#line 2151 "grammar.c"
        break;
      case 104: /* orderClause ::= ORDER BY arithmetic_expression_list ASC */
#line 588 "grammar.y"
{
	yymsp[-3].minor.yy114 = New_AST_OrderNode(yymsp[-1].minor.yy202, ORDER_DIR_ASC);
}
#line 2158 "grammar.c"
        break;
      case 105: /* orderClause ::= ORDER BY arithmetic_expression_list DESC */
#line 591 "grammar.y"
{
	yymsp[-3].minor.yy114 = New_AST_OrderNode(yymsp[-1].minor.yy202, ORDER_DIR_DESC);
}
#line 2165 "grammar.c"
        break;
      case 106: /* skipClause ::= */
#line 597 "grammar.y"
{
	yymsp[1].minor.yy137 = NULL;
}
#line 2172 "grammar.c"
        break;
      case 107: /* skipClause ::= SKIP INTEGER */
#line 600 "grammar.y"
{
	yymsp[-1].minor.yy137 = New_AST_SkipNode(yymsp[0].minor.yy0.longval);
}
#line 2179 "grammar.c"
        break;
      case 108: /* limitClause ::= */
#line 606 "grammar.y"
{
	yymsp[1].minor.yy65 = NULL;
}
#line 2186 "grammar.c"
        break;
      case 109: /* limitClause ::= LIMIT INTEGER */
#line 609 "grammar.y"
{
	yymsp[-1].minor.yy65 = New_AST_LimitNode(yymsp[0].minor.yy0.longval);
}
#line 2193 "grammar.c"
        break;
      case 110: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 615 "grammar.y"
{
	yymsp[-5].minor.yy169 = New_AST_UnwindNode(yymsp[-3].minor.yy202, yymsp[0].minor.yy0.strval);
}
#line 2200 "grammar.c"
        break;
      case 111: /* relation ::= EQ */
#line 620 "grammar.y"
{ yymsp[0].minor.yy115 = EQ; }
#line 2205 "grammar.c"
        break;
      case 112: /* relation ::= GT */
#line 621 "grammar.y"
{ yymsp[0].minor.yy115 = GT; }
#line 2210 "grammar.c"
        break;
      case 113: /* relation ::= LT */
#line 622 "grammar.y"
{ yymsp[0].minor.yy115 = LT; }
#line 2215 "grammar.c"
        break;
      case 114: /* relation ::= LE */
#line 623 "grammar.y"
{ yymsp[0].minor.yy115 = LE; }
#line 2220 "grammar.c"
        break;
      case 115: /* relation ::= GE */
#line 624 "grammar.y"
{ yymsp[0].minor.yy115 = GE; }
#line 2225 "grammar.c"
        break;
      case 116: /* relation ::= NE */
#line 625 "grammar.y"
{ yymsp[0].minor.yy115 = NE; }
#line 2230 "grammar.c"
        break;
      case 117: /* value ::= INTEGER */
#line 636 "grammar.y"
{  yylhsminor.yy136 = SI_LongVal(yymsp[0].minor.yy0.longval); }
#line 2235 "grammar.c"
  yymsp[0].minor.yy136 = yylhsminor.yy136;
        break;
      case 118: /* value ::= DASH INTEGER */
#line 637 "grammar.y"
{  yymsp[-1].minor.yy136 = SI_LongVal(-yymsp[0].minor.yy0.longval); }
#line 2241 "grammar.c"
        break;
      case 119: /* value ::= STRING */
#line 638 "grammar.y"
{  yylhsminor.yy136 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2246 "grammar.c"
  yymsp[0].minor.yy136 = yylhsminor.yy136;
        break;
      case 120: /* value ::= FLOAT */
#line 639 "grammar.y"
{  yylhsminor.yy136 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2252 "grammar.c"
  yymsp[0].minor.yy136 = yylhsminor.yy136;
        break;
      case 121: /* value ::= DASH FLOAT */
#line 640 "grammar.y"
{  yymsp[-1].minor.yy136 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2258 "grammar.c"
        break;
      case 122: /* value ::= TRUE */
#line 641 "grammar.y"
{ yymsp[0].minor.yy136 = SI_BoolVal(1); }
#line 2263 "grammar.c"
        break;
      case 123: /* value ::= FALSE */
#line 642 "grammar.y"
{ yymsp[0].minor.yy136 = SI_BoolVal(0); }
#line 2268 "grammar.c"
        break;
      case 124: /* value ::= NULLVAL */
#line 643 "grammar.y"
{ yymsp[0].minor.yy136 = SI_NullVal(); }
#line 2273 "grammar.c"
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
#line 33 "grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 2338 "grammar.c"
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
#line 645 "grammar.y"


	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	extern int             yylex_destroy (void);
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	AST **Query_Parse(const char *q, size_t len, char **err) {
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
#line 2586 "grammar.c"
