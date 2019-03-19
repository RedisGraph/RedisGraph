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
	#include "../util/arr.h"

	void yyerror(char *s);

	/*
	**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
	**                       zero the stack is dynamically sized using realloc()
	*/
	// Increase depth from 100 to 1000 to handel deep recursion.
	#define YYSTACKDEPTH 1000
#line 50 "grammar.c"
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
#define YYNOCODE 109
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE Token
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  AST_WhereNode* yy3;
  AST_CreateNode* yy4;
  char** yy5;
  int yy6;
  AST_FilterNode* yy10;
  AST* yy31;
  AST_LimitNode* yy39;
  AST_SetNode* yy44;
  AST_DeleteNode * yy47;
  AST_LinkEntity* yy69;
  AST_UnwindNode* yy97;
  AST_WithElementNode* yy98;
  AST_WithElementNode** yy112;
  AST_NodeEntity* yy117;
  AST_ReturnNode* yy120;
  AST** yy121;
  AST_ReturnElementNode** yy122;
  char* yy129;
  AST_LinkLength* yy138;
  AST_ReturnElementNode* yy139;
  AST_SkipNode* yy147;
  SIValue yy150;
  AST_ArithmeticExpressionNode* yy154;
  AST_Variable* yy156;
  AST_ProcedureCallNode* yy157;
  AST_MergeNode* yy164;
  AST_WithNode* yy168;
  AST_IndexNode* yy169;
  AST_SetElement* yy170;
  AST_MatchNode* yy173;
  AST_IndexOpType yy177;
  AST_OrderNode* yy196;
  Vector* yy210;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  parseCtx *ctx ;
#define ParseARG_PDECL , parseCtx *ctx 
#define ParseARG_FETCH  parseCtx *ctx  = yypParser->ctx 
#define ParseARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE             154
#define YYNRULE              134
#define YYNTOKEN             55
#define YY_MAX_SHIFT         153
#define YY_MIN_SHIFTREDUCE   246
#define YY_MAX_SHIFTREDUCE   379
#define YY_ERROR_ACTION      380
#define YY_ACCEPT_ACTION     381
#define YY_NO_ACTION         382
#define YY_MIN_REDUCE        383
#define YY_MAX_REDUCE        516
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
#define YY_ACTTAB_COUNT (420)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   386,   16,  385,   31,   85,   41,   81,   88,   86,   78,
 /*    10 */   136,  399,   15,  401,   56,  406,   40,   42,  489,   89,
 /*    20 */    53,  417,  435,   69,  422,  115,  381,   50,  384,  488,
 /*    30 */   146,  432,  145,  404,  107,  479,  120,   78,   43,  399,
 /*    40 */    15,  401,   56,  406,  144,   49,   33,  383,   53,  417,
 /*    50 */   435,   69,  422,  115,   28,   27,  101,  338,  290,  275,
 /*    60 */    32,  494,  494,  489,   90,   23,  117,   62,  374,  103,
 /*    70 */    46,  453,  126,    2,  488,   47,  453,  153,  473,  489,
 /*    80 */    90,  147,  148,  110,  489,   89,  372,  101,  338,    4,
 /*    90 */   488,   29,    3,  151,  474,  488,   23,   30,   79,  374,
 /*   100 */   103,  480,   18,  375,  377,  378,  379,   22,   21,   20,
 /*   110 */    19,  366,  367,  370,  368,  369,  109,  372,  345,   22,
 /*   120 */    21,   20,   19,  366,  367,  370,  368,  369,  391,  101,
 /*   130 */   392,  393,  489,   90,  375,  377,  378,  379,   23,  101,
 /*   140 */   153,  374,  103,  488,  494,  494,  149,  474,   10,   58,
 /*   150 */   132,  374,  103,  101,  371,  400,   73,  396,   83,  372,
 /*   160 */    77,   22,   21,   20,   19,  374,  371,   69,  422,  372,
 /*   170 */   489,   92,   69,  422,  489,   95,  375,  377,  378,  379,
 /*   180 */    68,  488,   33,  372,    2,  488,  375,  377,  378,  379,
 /*   190 */    28,   27,   98,  148,  290,  118,   32,  489,   93,  106,
 /*   200 */   375,  377,  378,  379,   22,   21,   20,   19,  488,    2,
 /*   210 */    22,   21,   20,   19,  141,  489,   37,  489,   36,  110,
 /*   220 */    45,  345,  489,   37,  116,  435,  488,  121,  488,   96,
 /*   230 */    45,  433,  145,  488,  469,  435,  489,   37,   22,   21,
 /*   240 */    20,   19,  150,  489,   95,  489,   94,  488,   99,  489,
 /*   250 */    95,  489,  486,   63,  488,   60,  488,  489,  485,  100,
 /*   260 */   488,  102,  488,  489,  104,  489,  105,   97,  488,  489,
 /*   270 */    91,   18,   30,   79,  488,   26,  488,  119,   48,   59,
 /*   280 */   488,  277,  278,  124,    2,   29,  134,   61,    9,   11,
 /*   290 */   125,  124,  277,  278,   66,  140,  373,   66,  359,  360,
 /*   300 */   135,  331,   66,    2,   61,    1,   66,   66,    9,   11,
 /*   310 */   129,  127,  350,  376,  113,   13,   18,   20,   19,  425,
 /*   320 */   111,  152,   51,   28,   44,  273,  418,   39,  148,  405,
 /*   330 */   403,   30,  147,   57,  122,   66,   24,  123,  124,  454,
 /*   340 */   100,  137,   29,  464,  398,  133,  130,   70,   71,  153,
 /*   350 */   131,  139,   72,    2,  394,  138,   76,  423,   74,  436,
 /*   360 */    75,   12,    5,  303,  388,  344,  143,    7,  365,  108,
 /*   370 */    25,   80,    8,  390,  291,  292,   82,  112,   52,   84,
 /*   380 */   114,  389,  387,   87,   34,  274,   54,  276,   55,   11,
 /*   390 */   310,  313,  308,  315,  314,  312,  306,   64,  321,  319,
 /*   400 */   311,  309,  128,  307,   38,  305,   65,   67,   17,  304,
 /*   410 */   325,  142,  340,  152,   35,  362,    6,  364,   14,  356,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    58,  102,   60,   61,   62,   63,   64,   62,   63,   67,
 /*    10 */   100,   69,   70,   71,   72,   73,   13,   86,   89,   90,
 /*    20 */    78,   79,   91,   81,   82,   83,   56,   57,   58,  100,
 /*    30 */    87,   88,   89,   63,  105,  106,   74,   67,   76,   69,
 /*    40 */    70,   71,   72,   73,   17,   86,   12,    0,   78,   79,
 /*    50 */    91,   81,   82,   83,   20,   21,    4,    5,   24,   17,
 /*    60 */    26,   27,   28,   89,   90,   13,   77,   94,   16,   17,
 /*    70 */    97,   98,   94,   39,  100,   97,   98,   43,  104,   89,
 /*    80 */    90,   47,   48,   49,   89,   90,   34,    4,    5,   42,
 /*    90 */   100,   21,   40,  103,  104,  100,   13,   27,   28,   16,
 /*   100 */    17,  106,   18,   51,   52,   53,   54,    3,    4,    5,
 /*   110 */     6,    7,    8,    9,   10,   11,   32,   34,   14,    3,
 /*   120 */     4,    5,    6,    7,    8,    9,   10,   11,   63,    4,
 /*   130 */    65,   66,   89,   90,   51,   52,   53,   54,   13,    4,
 /*   140 */    43,   16,   17,  100,   47,   48,  103,  104,   13,   63,
 /*   150 */    94,   16,   17,    4,   50,   69,   65,   66,   64,   34,
 /*   160 */    69,    3,    4,    5,    6,   16,   50,   81,   82,   34,
 /*   170 */    89,   90,   81,   82,   89,   90,   51,   52,   53,   54,
 /*   180 */    92,  100,   12,   34,   39,  100,   51,   52,   53,   54,
 /*   190 */    20,   21,  107,   48,   24,   77,   26,   89,   90,   41,
 /*   200 */    51,   52,   53,   54,    3,    4,    5,    6,  100,   39,
 /*   210 */     3,    4,    5,    6,   80,   89,   90,   89,   90,   49,
 /*   220 */    86,   14,   89,   90,   80,   91,  100,  101,  100,  101,
 /*   230 */    86,   88,   89,  100,  101,   91,   89,   90,    3,    4,
 /*   240 */     5,    6,   41,   89,   90,   89,   90,  100,  101,   89,
 /*   250 */    90,   89,   90,    4,  100,   96,  100,   89,   90,    5,
 /*   260 */   100,  107,  100,   89,   90,   89,   90,  107,  100,   89,
 /*   270 */    90,   18,   27,   28,  100,   17,  100,   14,   17,   30,
 /*   280 */   100,   18,   19,   25,   39,   21,   25,   33,    1,    2,
 /*   290 */    94,   25,   18,   19,   36,   25,   34,   36,   45,   46,
 /*   300 */    94,   14,   36,   39,   33,   59,   36,   36,    1,    2,
 /*   310 */    34,   35,   14,   51,   13,   13,   18,    5,    6,   85,
 /*   320 */    25,   19,   84,   20,   76,   16,   79,   75,   48,   62,
 /*   330 */    65,   27,   47,   61,   95,   36,   31,   94,   25,   98,
 /*   340 */     5,   17,   21,   99,   62,   94,   96,   61,   64,   43,
 /*   350 */    95,   94,   63,   39,   62,   99,   63,   82,   61,   91,
 /*   360 */    64,   38,   68,   17,   64,   17,   93,   18,   17,   41,
 /*   370 */    62,   61,   31,   62,   17,   14,   61,   17,   23,   63,
 /*   380 */    22,   64,   64,   63,   18,   16,   15,   17,   13,    2,
 /*   390 */     4,   32,   14,   17,   32,   32,   14,   17,   34,   34,
 /*   400 */    32,   29,   35,   14,   25,   14,   18,   17,    7,   17,
 /*   410 */    37,   18,   17,   19,   18,   34,   18,   34,   44,   17,
 /*   420 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   430 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   440 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   450 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   460 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   470 */   108,  108,  108,  108,  108,
};
#define YY_SHIFT_COUNT    (153)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (402)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */   170,   34,   52,   83,  125,   70,   83,  125,  125,  135,
 /*    10 */   135,  135,  135,  125,  125,  264,  125,  125,  125,  125,
 /*    20 */   125,  125,  125,  125,  258,  245,  266,    3,    3,    3,
 /*    30 */    27,  145,    3,   42,    3,   27,  104,  116,  149,  263,
 /*    40 */   261,   97,  249,  274,  274,  249,  254,  271,  270,  249,
 /*    50 */    47,  301,  295,  303,   42,  309,  304,  280,  285,  305,
 /*    60 */   299,  313,  335,  305,  299,  324,  324,  299,    3,  321,
 /*    70 */   280,  285,  306,  314,  280,  285,  306,  314,  323,  346,
 /*    80 */   280,  285,  280,  285,  306,  314,  306,  306,  314,  158,
 /*    90 */   201,  207,  235,  235,  235,  235,  287,  253,   84,  307,
 /*   100 */   276,  262,  298,  302,  312,  312,  348,  349,  351,  328,
 /*   110 */   341,  357,  361,  360,  355,  358,  366,  369,  370,  371,
 /*   120 */   375,  387,  386,  359,  376,  362,  363,  364,  365,  367,
 /*   130 */   368,  372,  378,  382,  380,  389,  388,  379,  373,  391,
 /*   140 */   390,  366,  392,  393,  394,  401,  396,  381,  383,  398,
 /*   150 */   395,  398,  402,  374,
};
#define YY_REDUCE_COUNT (88)
#define YY_REDUCE_MIN   (-101)
#define YY_REDUCE_MAX   (320)
static const short yy_reduce_ofst[] = {
 /*     0 */   -30,  -58,  -10,   43,  -71,   91,  -26,   -5,   85,  126,
 /*    10 */   128,  133,  147,  154,  160,   86,   81,  108,  156,  162,
 /*    20 */   168,  174,  176,  180,  -27,   65,  -22,  134,  144,  134,
 /*    30 */   -57,  -55,  -69,  -38,  -41,  143, -101, -101,  -90,  -11,
 /*    40 */    56,   94,   88,  118,  118,   88,  159,  196,  206,   88,
 /*    50 */   246,  234,  238,  247,  248,  252,  265,  267,  272,  239,
 /*    60 */   243,  241,  250,  255,  251,  244,  256,  257,  268,  275,
 /*    70 */   282,  286,  284,  289,  292,  297,  296,  293,  294,  273,
 /*    80 */   308,  310,  311,  315,  300,  316,  317,  318,  320,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   420,  420,  380,  380,  380,  420,  380,  380,  380,  380,
 /*    10 */   380,  380,  380,  380,  380,  420,  380,  380,  380,  380,
 /*    20 */   380,  380,  380,  380,  461,  380,  461,  426,  380,  380,
 /*    30 */   380,  380,  380,  380,  380,  380,  380,  380,  380,  380,
 /*    40 */   461,  404,  430,  409,  407,  437,  455,  461,  461,  438,
 /*    50 */   380,  380,  380,  416,  380,  380,  402,  500,  498,  380,
 /*    60 */   461,  380,  455,  380,  461,  380,  380,  461,  380,  421,
 /*    70 */   500,  498,  494,  397,  500,  498,  494,  395,  465,  380,
 /*    80 */   500,  498,  500,  498,  494,  380,  494,  494,  380,  380,
 /*    90 */   476,  380,  467,  434,  490,  491,  380,  495,  380,  466,
 /*   100 */   460,  380,  380,  492,  484,  483,  380,  478,  380,  380,
 /*   110 */   380,  380,  380,  380,  380,  380,  419,  380,  380,  408,
 /*   120 */   380,  470,  380,  380,  380,  380,  380,  380,  457,  459,
 /*   130 */   380,  380,  380,  380,  380,  380,  463,  380,  380,  380,
 /*   140 */   380,  424,  380,  439,  492,  380,  431,  380,  380,  472,
 /*   150 */   380,  471,  380,  380,
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
  /*   12 */ "CALL",
  /*   13 */ "LEFT_PARENTHESIS",
  /*   14 */ "RIGHT_PARENTHESIS",
  /*   15 */ "YIELD",
  /*   16 */ "STRING",
  /*   17 */ "UQSTRING",
  /*   18 */ "COMMA",
  /*   19 */ "DOT",
  /*   20 */ "MATCH",
  /*   21 */ "CREATE",
  /*   22 */ "INDEX",
  /*   23 */ "ON",
  /*   24 */ "DROP",
  /*   25 */ "COLON",
  /*   26 */ "MERGE",
  /*   27 */ "SET",
  /*   28 */ "DELETE",
  /*   29 */ "RIGHT_ARROW",
  /*   30 */ "LEFT_ARROW",
  /*   31 */ "LEFT_BRACKET",
  /*   32 */ "RIGHT_BRACKET",
  /*   33 */ "PIPE",
  /*   34 */ "INTEGER",
  /*   35 */ "DOTDOT",
  /*   36 */ "LEFT_CURLY_BRACKET",
  /*   37 */ "RIGHT_CURLY_BRACKET",
  /*   38 */ "WHERE",
  /*   39 */ "RETURN",
  /*   40 */ "DISTINCT",
  /*   41 */ "AS",
  /*   42 */ "WITH",
  /*   43 */ "ORDER",
  /*   44 */ "BY",
  /*   45 */ "ASC",
  /*   46 */ "DESC",
  /*   47 */ "SKIP",
  /*   48 */ "LIMIT",
  /*   49 */ "UNWIND",
  /*   50 */ "NE",
  /*   51 */ "FLOAT",
  /*   52 */ "TRUE",
  /*   53 */ "FALSE",
  /*   54 */ "NULLVAL",
  /*   55 */ "error",
  /*   56 */ "query",
  /*   57 */ "expressions",
  /*   58 */ "expr",
  /*   59 */ "withClause",
  /*   60 */ "singlePartQuery",
  /*   61 */ "skipClause",
  /*   62 */ "limitClause",
  /*   63 */ "returnClause",
  /*   64 */ "orderClause",
  /*   65 */ "setClause",
  /*   66 */ "deleteClause",
  /*   67 */ "multipleMatchClause",
  /*   68 */ "whereClause",
  /*   69 */ "multipleCreateClause",
  /*   70 */ "unwindClause",
  /*   71 */ "indexClause",
  /*   72 */ "mergeClause",
  /*   73 */ "procedureCallClause",
  /*   74 */ "procedureName",
  /*   75 */ "stringList",
  /*   76 */ "unquotedStringList",
  /*   77 */ "delimiter",
  /*   78 */ "matchClauses",
  /*   79 */ "matchClause",
  /*   80 */ "chains",
  /*   81 */ "createClauses",
  /*   82 */ "createClause",
  /*   83 */ "indexOpToken",
  /*   84 */ "indexLabel",
  /*   85 */ "indexProp",
  /*   86 */ "chain",
  /*   87 */ "setList",
  /*   88 */ "setElement",
  /*   89 */ "variable",
  /*   90 */ "arithmetic_expression",
  /*   91 */ "node",
  /*   92 */ "link",
  /*   93 */ "deleteExpression",
  /*   94 */ "properties",
  /*   95 */ "edge",
  /*   96 */ "edgeLength",
  /*   97 */ "edgeLabels",
  /*   98 */ "edgeLabel",
  /*   99 */ "mapLiteral",
  /*  100 */ "value",
  /*  101 */ "cond",
  /*  102 */ "relation",
  /*  103 */ "returnElements",
  /*  104 */ "returnElement",
  /*  105 */ "withElements",
  /*  106 */ "withElement",
  /*  107 */ "arithmetic_expression_list",
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
 /*  23 */ "expr ::= procedureCallClause",
 /*  24 */ "procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList",
 /*  25 */ "procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS",
 /*  26 */ "procedureName ::= unquotedStringList",
 /*  27 */ "stringList ::= STRING",
 /*  28 */ "stringList ::= stringList delimiter STRING",
 /*  29 */ "unquotedStringList ::= UQSTRING",
 /*  30 */ "unquotedStringList ::= unquotedStringList delimiter UQSTRING",
 /*  31 */ "delimiter ::= COMMA",
 /*  32 */ "delimiter ::= DOT",
 /*  33 */ "multipleMatchClause ::= matchClauses",
 /*  34 */ "matchClauses ::= matchClause",
 /*  35 */ "matchClauses ::= matchClauses matchClause",
 /*  36 */ "matchClause ::= MATCH chains",
 /*  37 */ "multipleCreateClause ::=",
 /*  38 */ "multipleCreateClause ::= createClauses",
 /*  39 */ "createClauses ::= createClause",
 /*  40 */ "createClauses ::= createClauses createClause",
 /*  41 */ "createClause ::= CREATE chains",
 /*  42 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  43 */ "indexOpToken ::= CREATE",
 /*  44 */ "indexOpToken ::= DROP",
 /*  45 */ "indexLabel ::= COLON UQSTRING",
 /*  46 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  47 */ "mergeClause ::= MERGE chain",
 /*  48 */ "setClause ::= SET setList",
 /*  49 */ "setList ::= setElement",
 /*  50 */ "setList ::= setList COMMA setElement",
 /*  51 */ "setElement ::= variable EQ arithmetic_expression",
 /*  52 */ "chain ::= node",
 /*  53 */ "chain ::= chain link node",
 /*  54 */ "chains ::= chain",
 /*  55 */ "chains ::= chains COMMA chain",
 /*  56 */ "deleteClause ::= DELETE deleteExpression",
 /*  57 */ "deleteExpression ::= UQSTRING",
 /*  58 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  59 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  60 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  61 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  62 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  63 */ "link ::= DASH edge RIGHT_ARROW",
 /*  64 */ "link ::= LEFT_ARROW edge DASH",
 /*  65 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  66 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  67 */ "edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET",
 /*  68 */ "edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET",
 /*  69 */ "edgeLabel ::= COLON UQSTRING",
 /*  70 */ "edgeLabels ::= edgeLabel",
 /*  71 */ "edgeLabels ::= edgeLabels PIPE edgeLabel",
 /*  72 */ "edgeLength ::=",
 /*  73 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  74 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  75 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  76 */ "edgeLength ::= MUL INTEGER",
 /*  77 */ "edgeLength ::= MUL",
 /*  78 */ "properties ::=",
 /*  79 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  80 */ "mapLiteral ::= UQSTRING COLON value",
 /*  81 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  82 */ "whereClause ::=",
 /*  83 */ "whereClause ::= WHERE cond",
 /*  84 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  85 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  86 */ "cond ::= cond AND cond",
 /*  87 */ "cond ::= cond OR cond",
 /*  88 */ "returnClause ::= RETURN returnElements",
 /*  89 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  90 */ "returnElements ::= returnElements COMMA returnElement",
 /*  91 */ "returnElements ::= returnElement",
 /*  92 */ "returnElement ::= MUL",
 /*  93 */ "returnElement ::= arithmetic_expression",
 /*  94 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  95 */ "withClause ::= WITH withElements",
 /*  96 */ "withElements ::= withElement",
 /*  97 */ "withElements ::= withElements COMMA withElement",
 /*  98 */ "withElement ::= arithmetic_expression AS UQSTRING",
 /*  99 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /* 100 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /* 101 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /* 102 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /* 103 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /* 104 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /* 105 */ "arithmetic_expression ::= value",
 /* 106 */ "arithmetic_expression ::= variable",
 /* 107 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /* 108 */ "arithmetic_expression_list ::= arithmetic_expression",
 /* 109 */ "variable ::= UQSTRING",
 /* 110 */ "variable ::= UQSTRING DOT UQSTRING",
 /* 111 */ "orderClause ::=",
 /* 112 */ "orderClause ::= ORDER BY arithmetic_expression_list",
 /* 113 */ "orderClause ::= ORDER BY arithmetic_expression_list ASC",
 /* 114 */ "orderClause ::= ORDER BY arithmetic_expression_list DESC",
 /* 115 */ "skipClause ::=",
 /* 116 */ "skipClause ::= SKIP INTEGER",
 /* 117 */ "limitClause ::=",
 /* 118 */ "limitClause ::= LIMIT INTEGER",
 /* 119 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /* 120 */ "relation ::= EQ",
 /* 121 */ "relation ::= GT",
 /* 122 */ "relation ::= LT",
 /* 123 */ "relation ::= LE",
 /* 124 */ "relation ::= GE",
 /* 125 */ "relation ::= NE",
 /* 126 */ "value ::= INTEGER",
 /* 127 */ "value ::= DASH INTEGER",
 /* 128 */ "value ::= STRING",
 /* 129 */ "value ::= FLOAT",
 /* 130 */ "value ::= DASH FLOAT",
 /* 131 */ "value ::= TRUE",
 /* 132 */ "value ::= FALSE",
 /* 133 */ "value ::= NULLVAL",
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
    case 101: /* cond */
{
#line 504 "grammar.y"
 Free_AST_FilterNode((yypminor->yy10)); 
#line 860 "grammar.c"
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
  {   56,   -1 }, /* (0) query ::= expressions */
  {   57,   -1 }, /* (1) expressions ::= expr */
  {   57,   -3 }, /* (2) expressions ::= expressions withClause singlePartQuery */
  {   60,   -1 }, /* (3) singlePartQuery ::= expr */
  {   60,   -4 }, /* (4) singlePartQuery ::= skipClause limitClause returnClause orderClause */
  {   60,   -3 }, /* (5) singlePartQuery ::= limitClause returnClause orderClause */
  {   60,   -3 }, /* (6) singlePartQuery ::= skipClause returnClause orderClause */
  {   60,   -4 }, /* (7) singlePartQuery ::= returnClause orderClause skipClause limitClause */
  {   60,   -4 }, /* (8) singlePartQuery ::= orderClause skipClause limitClause returnClause */
  {   60,   -4 }, /* (9) singlePartQuery ::= orderClause skipClause limitClause setClause */
  {   60,   -4 }, /* (10) singlePartQuery ::= orderClause skipClause limitClause deleteClause */
  {   58,   -7 }, /* (11) expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
  {   58,   -3 }, /* (12) expr ::= multipleMatchClause whereClause multipleCreateClause */
  {   58,   -3 }, /* (13) expr ::= multipleMatchClause whereClause deleteClause */
  {   58,   -3 }, /* (14) expr ::= multipleMatchClause whereClause setClause */
  {   58,   -7 }, /* (15) expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
  {   58,   -1 }, /* (16) expr ::= multipleCreateClause */
  {   58,   -2 }, /* (17) expr ::= unwindClause multipleCreateClause */
  {   58,   -1 }, /* (18) expr ::= indexClause */
  {   58,   -1 }, /* (19) expr ::= mergeClause */
  {   58,   -2 }, /* (20) expr ::= mergeClause setClause */
  {   58,   -1 }, /* (21) expr ::= returnClause */
  {   58,   -4 }, /* (22) expr ::= unwindClause returnClause skipClause limitClause */
  {   58,   -1 }, /* (23) expr ::= procedureCallClause */
  {   73,   -7 }, /* (24) procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList */
  {   73,   -5 }, /* (25) procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS */
  {   74,   -1 }, /* (26) procedureName ::= unquotedStringList */
  {   75,   -1 }, /* (27) stringList ::= STRING */
  {   75,   -3 }, /* (28) stringList ::= stringList delimiter STRING */
  {   76,   -1 }, /* (29) unquotedStringList ::= UQSTRING */
  {   76,   -3 }, /* (30) unquotedStringList ::= unquotedStringList delimiter UQSTRING */
  {   77,   -1 }, /* (31) delimiter ::= COMMA */
  {   77,   -1 }, /* (32) delimiter ::= DOT */
  {   67,   -1 }, /* (33) multipleMatchClause ::= matchClauses */
  {   78,   -1 }, /* (34) matchClauses ::= matchClause */
  {   78,   -2 }, /* (35) matchClauses ::= matchClauses matchClause */
  {   79,   -2 }, /* (36) matchClause ::= MATCH chains */
  {   69,    0 }, /* (37) multipleCreateClause ::= */
  {   69,   -1 }, /* (38) multipleCreateClause ::= createClauses */
  {   81,   -1 }, /* (39) createClauses ::= createClause */
  {   81,   -2 }, /* (40) createClauses ::= createClauses createClause */
  {   82,   -2 }, /* (41) createClause ::= CREATE chains */
  {   71,   -5 }, /* (42) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   83,   -1 }, /* (43) indexOpToken ::= CREATE */
  {   83,   -1 }, /* (44) indexOpToken ::= DROP */
  {   84,   -2 }, /* (45) indexLabel ::= COLON UQSTRING */
  {   85,   -3 }, /* (46) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   72,   -2 }, /* (47) mergeClause ::= MERGE chain */
  {   65,   -2 }, /* (48) setClause ::= SET setList */
  {   87,   -1 }, /* (49) setList ::= setElement */
  {   87,   -3 }, /* (50) setList ::= setList COMMA setElement */
  {   88,   -3 }, /* (51) setElement ::= variable EQ arithmetic_expression */
  {   86,   -1 }, /* (52) chain ::= node */
  {   86,   -3 }, /* (53) chain ::= chain link node */
  {   80,   -1 }, /* (54) chains ::= chain */
  {   80,   -3 }, /* (55) chains ::= chains COMMA chain */
  {   66,   -2 }, /* (56) deleteClause ::= DELETE deleteExpression */
  {   93,   -1 }, /* (57) deleteExpression ::= UQSTRING */
  {   93,   -3 }, /* (58) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   91,   -6 }, /* (59) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   91,   -5 }, /* (60) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   91,   -4 }, /* (61) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   91,   -3 }, /* (62) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   92,   -3 }, /* (63) link ::= DASH edge RIGHT_ARROW */
  {   92,   -3 }, /* (64) link ::= LEFT_ARROW edge DASH */
  {   95,   -4 }, /* (65) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   95,   -4 }, /* (66) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   95,   -5 }, /* (67) edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
  {   95,   -5 }, /* (68) edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
  {   98,   -2 }, /* (69) edgeLabel ::= COLON UQSTRING */
  {   97,   -1 }, /* (70) edgeLabels ::= edgeLabel */
  {   97,   -3 }, /* (71) edgeLabels ::= edgeLabels PIPE edgeLabel */
  {   96,    0 }, /* (72) edgeLength ::= */
  {   96,   -4 }, /* (73) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   96,   -3 }, /* (74) edgeLength ::= MUL INTEGER DOTDOT */
  {   96,   -3 }, /* (75) edgeLength ::= MUL DOTDOT INTEGER */
  {   96,   -2 }, /* (76) edgeLength ::= MUL INTEGER */
  {   96,   -1 }, /* (77) edgeLength ::= MUL */
  {   94,    0 }, /* (78) properties ::= */
  {   94,   -3 }, /* (79) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   99,   -3 }, /* (80) mapLiteral ::= UQSTRING COLON value */
  {   99,   -5 }, /* (81) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   68,    0 }, /* (82) whereClause ::= */
  {   68,   -2 }, /* (83) whereClause ::= WHERE cond */
  {  101,   -3 }, /* (84) cond ::= arithmetic_expression relation arithmetic_expression */
  {  101,   -3 }, /* (85) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {  101,   -3 }, /* (86) cond ::= cond AND cond */
  {  101,   -3 }, /* (87) cond ::= cond OR cond */
  {   63,   -2 }, /* (88) returnClause ::= RETURN returnElements */
  {   63,   -3 }, /* (89) returnClause ::= RETURN DISTINCT returnElements */
  {  103,   -3 }, /* (90) returnElements ::= returnElements COMMA returnElement */
  {  103,   -1 }, /* (91) returnElements ::= returnElement */
  {  104,   -1 }, /* (92) returnElement ::= MUL */
  {  104,   -1 }, /* (93) returnElement ::= arithmetic_expression */
  {  104,   -3 }, /* (94) returnElement ::= arithmetic_expression AS UQSTRING */
  {   59,   -2 }, /* (95) withClause ::= WITH withElements */
  {  105,   -1 }, /* (96) withElements ::= withElement */
  {  105,   -3 }, /* (97) withElements ::= withElements COMMA withElement */
  {  106,   -3 }, /* (98) withElement ::= arithmetic_expression AS UQSTRING */
  {   90,   -3 }, /* (99) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   90,   -3 }, /* (100) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   90,   -3 }, /* (101) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   90,   -3 }, /* (102) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   90,   -3 }, /* (103) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   90,   -4 }, /* (104) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   90,   -1 }, /* (105) arithmetic_expression ::= value */
  {   90,   -1 }, /* (106) arithmetic_expression ::= variable */
  {  107,   -3 }, /* (107) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {  107,   -1 }, /* (108) arithmetic_expression_list ::= arithmetic_expression */
  {   89,   -1 }, /* (109) variable ::= UQSTRING */
  {   89,   -3 }, /* (110) variable ::= UQSTRING DOT UQSTRING */
  {   64,    0 }, /* (111) orderClause ::= */
  {   64,   -3 }, /* (112) orderClause ::= ORDER BY arithmetic_expression_list */
  {   64,   -4 }, /* (113) orderClause ::= ORDER BY arithmetic_expression_list ASC */
  {   64,   -4 }, /* (114) orderClause ::= ORDER BY arithmetic_expression_list DESC */
  {   61,    0 }, /* (115) skipClause ::= */
  {   61,   -2 }, /* (116) skipClause ::= SKIP INTEGER */
  {   62,    0 }, /* (117) limitClause ::= */
  {   62,   -2 }, /* (118) limitClause ::= LIMIT INTEGER */
  {   70,   -6 }, /* (119) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {  102,   -1 }, /* (120) relation ::= EQ */
  {  102,   -1 }, /* (121) relation ::= GT */
  {  102,   -1 }, /* (122) relation ::= LT */
  {  102,   -1 }, /* (123) relation ::= LE */
  {  102,   -1 }, /* (124) relation ::= GE */
  {  102,   -1 }, /* (125) relation ::= NE */
  {  100,   -1 }, /* (126) value ::= INTEGER */
  {  100,   -2 }, /* (127) value ::= DASH INTEGER */
  {  100,   -1 }, /* (128) value ::= STRING */
  {  100,   -1 }, /* (129) value ::= FLOAT */
  {  100,   -2 }, /* (130) value ::= DASH FLOAT */
  {  100,   -1 }, /* (131) value ::= TRUE */
  {  100,   -1 }, /* (132) value ::= FALSE */
  {  100,   -1 }, /* (133) value ::= NULLVAL */
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
#line 42 "grammar.y"
{ ctx->root = yymsp[0].minor.yy121; }
#line 1371 "grammar.c"
        break;
      case 1: /* expressions ::= expr */
#line 46 "grammar.y"
{
	yylhsminor.yy121 = array_new(AST*, 1);
	yylhsminor.yy121 = array_append(yylhsminor.yy121, yymsp[0].minor.yy31);
}
#line 1379 "grammar.c"
  yymsp[0].minor.yy121 = yylhsminor.yy121;
        break;
      case 2: /* expressions ::= expressions withClause singlePartQuery */
#line 51 "grammar.y"
{
	AST *ast = yymsp[-2].minor.yy121[array_len(yymsp[-2].minor.yy121)-1];
	ast->withNode = yymsp[-1].minor.yy168;
	yylhsminor.yy121 = array_append(yymsp[-2].minor.yy121, yymsp[0].minor.yy31);
	yylhsminor.yy121=yymsp[-2].minor.yy121;
}
#line 1390 "grammar.c"
  yymsp[-2].minor.yy121 = yylhsminor.yy121;
        break;
      case 3: /* singlePartQuery ::= expr */
#line 59 "grammar.y"
{
	yylhsminor.yy31 = yymsp[0].minor.yy31;
}
#line 1398 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 4: /* singlePartQuery ::= skipClause limitClause returnClause orderClause */
#line 63 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, yymsp[-3].minor.yy147, yymsp[-2].minor.yy39, NULL, NULL, NULL);
}
#line 1406 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 5: /* singlePartQuery ::= limitClause returnClause orderClause */
#line 67 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, NULL, yymsp[-2].minor.yy39, NULL, NULL, NULL);
}
#line 1414 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 6: /* singlePartQuery ::= skipClause returnClause orderClause */
#line 71 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, yymsp[-2].minor.yy147, NULL, NULL, NULL, NULL);
}
#line 1422 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 7: /* singlePartQuery ::= returnClause orderClause skipClause limitClause */
#line 75 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1430 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 8: /* singlePartQuery ::= orderClause skipClause limitClause returnClause */
#line 79 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1438 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 9: /* singlePartQuery ::= orderClause skipClause limitClause setClause */
#line 83 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, yymsp[0].minor.yy44, NULL, NULL, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1446 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 10: /* singlePartQuery ::= orderClause skipClause limitClause deleteClause */
#line 87 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy47, NULL, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1454 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 11: /* expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
#line 92 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-6].minor.yy173, yymsp[-5].minor.yy3, yymsp[-4].minor.yy4, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1462 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 12: /* expr ::= multipleMatchClause whereClause multipleCreateClause */
#line 96 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1470 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 13: /* expr ::= multipleMatchClause whereClause deleteClause */
#line 100 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, NULL, NULL, NULL, yymsp[0].minor.yy47, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1478 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 14: /* expr ::= multipleMatchClause whereClause setClause */
#line 104 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, NULL, NULL, yymsp[0].minor.yy44, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1486 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 15: /* expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 108 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-6].minor.yy173, yymsp[-5].minor.yy3, NULL, NULL, yymsp[-4].minor.yy44, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1494 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 16: /* expr ::= multipleCreateClause */
#line 112 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1502 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 17: /* expr ::= unwindClause multipleCreateClause */
#line 116 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy97, NULL);
}
#line 1510 "grammar.c"
  yymsp[-1].minor.yy31 = yylhsminor.yy31;
        break;
      case 18: /* expr ::= indexClause */
#line 120 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy169, NULL, NULL);
}
#line 1518 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 19: /* expr ::= mergeClause */
#line 124 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, yymsp[0].minor.yy164, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1526 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 20: /* expr ::= mergeClause setClause */
#line 128 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, yymsp[-1].minor.yy164, yymsp[0].minor.yy44, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1534 "grammar.c"
  yymsp[-1].minor.yy31 = yylhsminor.yy31;
        break;
      case 21: /* expr ::= returnClause */
#line 132 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1542 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 22: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 136 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy120, NULL, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, yymsp[-3].minor.yy97, NULL);
}
#line 1550 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 23: /* expr ::= procedureCallClause */
#line 140 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy157);
}
#line 1558 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 24: /* procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList */
#line 145 "grammar.y"
{
	yymsp[-6].minor.yy157 = New_AST_ProcedureCallNode(yymsp[-5].minor.yy129, yymsp[-3].minor.yy5, yymsp[0].minor.yy5);
}
#line 1566 "grammar.c"
        break;
      case 25: /* procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS */
#line 149 "grammar.y"
{	
	yymsp[-4].minor.yy157 = New_AST_ProcedureCallNode(yymsp[-3].minor.yy129, yymsp[-1].minor.yy5, NULL);
}
#line 1573 "grammar.c"
        break;
      case 26: /* procedureName ::= unquotedStringList */
#line 154 "grammar.y"
{
	// Concatenate strings with dots.
	// Determine required string length.
	int buffLen = 0;
	for(int i = 0; i < array_len(yymsp[0].minor.yy5); i++) {
		buffLen += strlen(yymsp[0].minor.yy5[i]) + 1;
	}

	int offset = 0;
	char *procedure_name = malloc(buffLen);
	for(int i = 0; i < array_len(yymsp[0].minor.yy5); i++) {
		int n = strlen(yymsp[0].minor.yy5[i]);
		memcpy(procedure_name + offset, yymsp[0].minor.yy5[i], n);
		offset += n;
		procedure_name[offset] = '.';
		offset++;
	}

	// Discard last dot and trerminate string.
	offset--;
	procedure_name[offset] = '\0';
	yylhsminor.yy129 = procedure_name;
}
#line 1600 "grammar.c"
  yymsp[0].minor.yy129 = yylhsminor.yy129;
        break;
      case 27: /* stringList ::= STRING */
      case 29: /* unquotedStringList ::= UQSTRING */ yytestcase(yyruleno==29);
#line 179 "grammar.y"
{
	yylhsminor.yy5 = array_new(char*, 1);
	yylhsminor.yy5 = array_append(yylhsminor.yy5, yymsp[0].minor.yy0.strval);
}
#line 1610 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 28: /* stringList ::= stringList delimiter STRING */
      case 30: /* unquotedStringList ::= unquotedStringList delimiter UQSTRING */ yytestcase(yyruleno==30);
#line 185 "grammar.y"
{
	yymsp[-2].minor.yy5 = array_append(yymsp[-2].minor.yy5, yymsp[0].minor.yy0.strval);
	yylhsminor.yy5 = yymsp[-2].minor.yy5;
}
#line 1620 "grammar.c"
  yymsp[-2].minor.yy5 = yylhsminor.yy5;
        break;
      case 31: /* delimiter ::= COMMA */
#line 203 "grammar.y"
{ yymsp[0].minor.yy6 = COMMA; }
#line 1626 "grammar.c"
        break;
      case 32: /* delimiter ::= DOT */
#line 204 "grammar.y"
{ yymsp[0].minor.yy6 = DOT; }
#line 1631 "grammar.c"
        break;
      case 33: /* multipleMatchClause ::= matchClauses */
#line 207 "grammar.y"
{
	yylhsminor.yy173 = New_AST_MatchNode(yymsp[0].minor.yy210);
}
#line 1638 "grammar.c"
  yymsp[0].minor.yy173 = yylhsminor.yy173;
        break;
      case 34: /* matchClauses ::= matchClause */
      case 39: /* createClauses ::= createClause */ yytestcase(yyruleno==39);
#line 213 "grammar.y"
{
	yylhsminor.yy210 = yymsp[0].minor.yy210;
}
#line 1647 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 35: /* matchClauses ::= matchClauses matchClause */
      case 40: /* createClauses ::= createClauses createClause */ yytestcase(yyruleno==40);
#line 217 "grammar.y"
{
	Vector *v;
	while(Vector_Pop(yymsp[0].minor.yy210, &v)) Vector_Push(yymsp[-1].minor.yy210, v);
	Vector_Free(yymsp[0].minor.yy210);
	yylhsminor.yy210 = yymsp[-1].minor.yy210;
}
#line 1659 "grammar.c"
  yymsp[-1].minor.yy210 = yylhsminor.yy210;
        break;
      case 36: /* matchClause ::= MATCH chains */
      case 41: /* createClause ::= CREATE chains */ yytestcase(yyruleno==41);
#line 226 "grammar.y"
{
	yymsp[-1].minor.yy210 = yymsp[0].minor.yy210;
}
#line 1668 "grammar.c"
        break;
      case 37: /* multipleCreateClause ::= */
#line 231 "grammar.y"
{
	yymsp[1].minor.yy4 = NULL;
}
#line 1675 "grammar.c"
        break;
      case 38: /* multipleCreateClause ::= createClauses */
#line 235 "grammar.y"
{
	yylhsminor.yy4 = New_AST_CreateNode(yymsp[0].minor.yy210);
}
#line 1682 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 42: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 261 "grammar.y"
{
  yylhsminor.yy169 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy177);
}
#line 1690 "grammar.c"
  yymsp[-4].minor.yy169 = yylhsminor.yy169;
        break;
      case 43: /* indexOpToken ::= CREATE */
#line 267 "grammar.y"
{ yymsp[0].minor.yy177 = CREATE_INDEX; }
#line 1696 "grammar.c"
        break;
      case 44: /* indexOpToken ::= DROP */
#line 268 "grammar.y"
{ yymsp[0].minor.yy177 = DROP_INDEX; }
#line 1701 "grammar.c"
        break;
      case 45: /* indexLabel ::= COLON UQSTRING */
#line 270 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1708 "grammar.c"
        break;
      case 46: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 274 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1715 "grammar.c"
        break;
      case 47: /* mergeClause ::= MERGE chain */
#line 280 "grammar.y"
{
	yymsp[-1].minor.yy164 = New_AST_MergeNode(yymsp[0].minor.yy210);
}
#line 1722 "grammar.c"
        break;
      case 48: /* setClause ::= SET setList */
#line 285 "grammar.y"
{
	yymsp[-1].minor.yy44 = New_AST_SetNode(yymsp[0].minor.yy210);
}
#line 1729 "grammar.c"
        break;
      case 49: /* setList ::= setElement */
#line 290 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy170);
}
#line 1737 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 50: /* setList ::= setList COMMA setElement */
#line 294 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy170);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1746 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 51: /* setElement ::= variable EQ arithmetic_expression */
#line 300 "grammar.y"
{
	yylhsminor.yy170 = New_AST_SetElement(yymsp[-2].minor.yy156, yymsp[0].minor.yy154);
}
#line 1754 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 52: /* chain ::= node */
#line 306 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy117);
}
#line 1763 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 53: /* chain ::= chain link node */
#line 311 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[-1].minor.yy69);
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy117);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1773 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 54: /* chains ::= chain */
#line 319 "grammar.y"
{
	yylhsminor.yy210 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy210);
}
#line 1782 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 55: /* chains ::= chains COMMA chain */
#line 324 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy210);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1791 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 56: /* deleteClause ::= DELETE deleteExpression */
#line 332 "grammar.y"
{
	yymsp[-1].minor.yy47 = New_AST_DeleteNode(yymsp[0].minor.yy210);
}
#line 1799 "grammar.c"
        break;
      case 57: /* deleteExpression ::= UQSTRING */
#line 338 "grammar.y"
{
	yylhsminor.yy210 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy0.strval);
}
#line 1807 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 58: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 343 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy0.strval);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1816 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 59: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 351 "grammar.y"
{
	yymsp[-5].minor.yy117 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 1824 "grammar.c"
        break;
      case 60: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 356 "grammar.y"
{
	yymsp[-4].minor.yy117 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 1831 "grammar.c"
        break;
      case 61: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 361 "grammar.y"
{
	yymsp[-3].minor.yy117 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy210);
}
#line 1838 "grammar.c"
        break;
      case 62: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 366 "grammar.y"
{
	yymsp[-2].minor.yy117 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy210);
}
#line 1845 "grammar.c"
        break;
      case 63: /* link ::= DASH edge RIGHT_ARROW */
#line 373 "grammar.y"
{
	yymsp[-2].minor.yy69 = yymsp[-1].minor.yy69;
	yymsp[-2].minor.yy69->direction = N_LEFT_TO_RIGHT;
}
#line 1853 "grammar.c"
        break;
      case 64: /* link ::= LEFT_ARROW edge DASH */
#line 379 "grammar.y"
{
	yymsp[-2].minor.yy69 = yymsp[-1].minor.yy69;
	yymsp[-2].minor.yy69->direction = N_RIGHT_TO_LEFT;
}
#line 1861 "grammar.c"
        break;
      case 65: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 386 "grammar.y"
{ 
	yymsp[-3].minor.yy69 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy210, N_DIR_UNKNOWN, yymsp[-1].minor.yy138);
}
#line 1868 "grammar.c"
        break;
      case 66: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 391 "grammar.y"
{ 
	yymsp[-3].minor.yy69 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, NULL);
}
#line 1875 "grammar.c"
        break;
      case 67: /* edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
#line 396 "grammar.y"
{ 
	yymsp[-4].minor.yy69 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy5, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, yymsp[-2].minor.yy138);
}
#line 1882 "grammar.c"
        break;
      case 68: /* edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
#line 401 "grammar.y"
{ 
	yymsp[-4].minor.yy69 = New_AST_LinkEntity(yymsp[-3].minor.yy0.strval, yymsp[-2].minor.yy5, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, NULL);
}
#line 1889 "grammar.c"
        break;
      case 69: /* edgeLabel ::= COLON UQSTRING */
#line 408 "grammar.y"
{
	yymsp[-1].minor.yy129 = yymsp[0].minor.yy0.strval;
}
#line 1896 "grammar.c"
        break;
      case 70: /* edgeLabels ::= edgeLabel */
#line 413 "grammar.y"
{
	yylhsminor.yy5 = array_new(char*, 1);
	yylhsminor.yy5 = array_append(yylhsminor.yy5, yymsp[0].minor.yy129);
}
#line 1904 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 71: /* edgeLabels ::= edgeLabels PIPE edgeLabel */
#line 419 "grammar.y"
{
	char *label = yymsp[0].minor.yy129;
	yymsp[-2].minor.yy5 = array_append(yymsp[-2].minor.yy5, label);
	yylhsminor.yy5 = yymsp[-2].minor.yy5;
}
#line 1914 "grammar.c"
  yymsp[-2].minor.yy5 = yylhsminor.yy5;
        break;
      case 72: /* edgeLength ::= */
#line 428 "grammar.y"
{
	yymsp[1].minor.yy138 = NULL;
}
#line 1922 "grammar.c"
        break;
      case 73: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 433 "grammar.y"
{
	yymsp[-3].minor.yy138 = New_AST_LinkLength(yymsp[-2].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1929 "grammar.c"
        break;
      case 74: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 438 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(yymsp[-1].minor.yy0.longval, UINT_MAX-2);
}
#line 1936 "grammar.c"
        break;
      case 75: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 443 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(1, yymsp[0].minor.yy0.longval);
}
#line 1943 "grammar.c"
        break;
      case 76: /* edgeLength ::= MUL INTEGER */
#line 448 "grammar.y"
{
	yymsp[-1].minor.yy138 = New_AST_LinkLength(yymsp[0].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1950 "grammar.c"
        break;
      case 77: /* edgeLength ::= MUL */
#line 453 "grammar.y"
{
	yymsp[0].minor.yy138 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1957 "grammar.c"
        break;
      case 78: /* properties ::= */
#line 459 "grammar.y"
{
	yymsp[1].minor.yy210 = NULL;
}
#line 1964 "grammar.c"
        break;
      case 79: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 463 "grammar.y"
{
	yymsp[-2].minor.yy210 = yymsp[-1].minor.yy210;
}
#line 1971 "grammar.c"
        break;
      case 80: /* mapLiteral ::= UQSTRING COLON value */
#line 469 "grammar.y"
{
	yylhsminor.yy210 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy210, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy150;
	Vector_Push(yylhsminor.yy210, val);
}
#line 1986 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 81: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 481 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy210, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy150;
	Vector_Push(yymsp[0].minor.yy210, val);
	
	yylhsminor.yy210 = yymsp[0].minor.yy210;
}
#line 2002 "grammar.c"
  yymsp[-4].minor.yy210 = yylhsminor.yy210;
        break;
      case 82: /* whereClause ::= */
#line 495 "grammar.y"
{ 
	yymsp[1].minor.yy3 = NULL;
}
#line 2010 "grammar.c"
        break;
      case 83: /* whereClause ::= WHERE cond */
#line 498 "grammar.y"
{
	yymsp[-1].minor.yy3 = New_AST_WhereNode(yymsp[0].minor.yy10);
}
#line 2017 "grammar.c"
        break;
      case 84: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 507 "grammar.y"
{ yylhsminor.yy10 = New_AST_PredicateNode(yymsp[-2].minor.yy154, yymsp[-1].minor.yy6, yymsp[0].minor.yy154); }
#line 2022 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 85: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 509 "grammar.y"
{ yymsp[-2].minor.yy10 = yymsp[-1].minor.yy10; }
#line 2028 "grammar.c"
        break;
      case 86: /* cond ::= cond AND cond */
#line 510 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, AND, yymsp[0].minor.yy10); }
#line 2033 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 87: /* cond ::= cond OR cond */
#line 511 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, OR, yymsp[0].minor.yy10); }
#line 2039 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 88: /* returnClause ::= RETURN returnElements */
#line 515 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy122, 0);
}
#line 2047 "grammar.c"
        break;
      case 89: /* returnClause ::= RETURN DISTINCT returnElements */
#line 518 "grammar.y"
{
	yymsp[-2].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy122, 1);
}
#line 2054 "grammar.c"
        break;
      case 90: /* returnElements ::= returnElements COMMA returnElement */
#line 524 "grammar.y"
{
	yylhsminor.yy122 = array_append(yymsp[-2].minor.yy122, yymsp[0].minor.yy139);
}
#line 2061 "grammar.c"
  yymsp[-2].minor.yy122 = yylhsminor.yy122;
        break;
      case 91: /* returnElements ::= returnElement */
#line 528 "grammar.y"
{
	yylhsminor.yy122 = array_new(AST_ReturnElementNode*, 1);
	array_append(yylhsminor.yy122, yymsp[0].minor.yy139);
}
#line 2070 "grammar.c"
  yymsp[0].minor.yy122 = yylhsminor.yy122;
        break;
      case 92: /* returnElement ::= MUL */
#line 536 "grammar.y"
{
	yymsp[0].minor.yy139 = New_AST_ReturnElementExpandALL();
}
#line 2078 "grammar.c"
        break;
      case 93: /* returnElement ::= arithmetic_expression */
#line 539 "grammar.y"
{
	yylhsminor.yy139 = New_AST_ReturnElementNode(yymsp[0].minor.yy154, NULL);
}
#line 2085 "grammar.c"
  yymsp[0].minor.yy139 = yylhsminor.yy139;
        break;
      case 94: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 543 "grammar.y"
{
	yylhsminor.yy139 = New_AST_ReturnElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 2093 "grammar.c"
  yymsp[-2].minor.yy139 = yylhsminor.yy139;
        break;
      case 95: /* withClause ::= WITH withElements */
#line 548 "grammar.y"
{
	yymsp[-1].minor.yy168 = New_AST_WithNode(yymsp[0].minor.yy112);
}
#line 2101 "grammar.c"
        break;
      case 96: /* withElements ::= withElement */
#line 553 "grammar.y"
{
	yylhsminor.yy112 = array_new(AST_WithElementNode*, 1);
	array_append(yylhsminor.yy112, yymsp[0].minor.yy98);
}
#line 2109 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 97: /* withElements ::= withElements COMMA withElement */
#line 557 "grammar.y"
{
	yylhsminor.yy112 = array_append(yymsp[-2].minor.yy112, yymsp[0].minor.yy98);
}
#line 2117 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 98: /* withElement ::= arithmetic_expression AS UQSTRING */
#line 562 "grammar.y"
{
	yylhsminor.yy98 = New_AST_WithElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 2125 "grammar.c"
  yymsp[-2].minor.yy98 = yylhsminor.yy98;
        break;
      case 99: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 569 "grammar.y"
{
	yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154;
}
#line 2133 "grammar.c"
        break;
      case 100: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 575 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 2143 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 101: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 582 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 2154 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 102: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 589 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 2165 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 103: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 596 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 2176 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 104: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 604 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 2184 "grammar.c"
  yymsp[-3].minor.yy154 = yylhsminor.yy154;
        break;
      case 105: /* arithmetic_expression ::= value */
#line 609 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy150);
}
#line 2192 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 106: /* arithmetic_expression ::= variable */
#line 614 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy156->alias, yymsp[0].minor.yy156->property);
	free(yymsp[0].minor.yy156);
}
#line 2201 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 107: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 621 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy154);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 2210 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 108: /* arithmetic_expression_list ::= arithmetic_expression */
#line 625 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy154);
}
#line 2219 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 109: /* variable ::= UQSTRING */
#line 632 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 2227 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 110: /* variable ::= UQSTRING DOT UQSTRING */
#line 636 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 2235 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 111: /* orderClause ::= */
#line 642 "grammar.y"
{
	yymsp[1].minor.yy196 = NULL;
}
#line 2243 "grammar.c"
        break;
      case 112: /* orderClause ::= ORDER BY arithmetic_expression_list */
#line 645 "grammar.y"
{
	yymsp[-2].minor.yy196 = New_AST_OrderNode(yymsp[0].minor.yy210, ORDER_DIR_ASC);
}
#line 2250 "grammar.c"
        break;
      case 113: /* orderClause ::= ORDER BY arithmetic_expression_list ASC */
#line 648 "grammar.y"
{
	yymsp[-3].minor.yy196 = New_AST_OrderNode(yymsp[-1].minor.yy210, ORDER_DIR_ASC);
}
#line 2257 "grammar.c"
        break;
      case 114: /* orderClause ::= ORDER BY arithmetic_expression_list DESC */
#line 651 "grammar.y"
{
	yymsp[-3].minor.yy196 = New_AST_OrderNode(yymsp[-1].minor.yy210, ORDER_DIR_DESC);
}
#line 2264 "grammar.c"
        break;
      case 115: /* skipClause ::= */
#line 657 "grammar.y"
{
	yymsp[1].minor.yy147 = NULL;
}
#line 2271 "grammar.c"
        break;
      case 116: /* skipClause ::= SKIP INTEGER */
#line 660 "grammar.y"
{
	yymsp[-1].minor.yy147 = New_AST_SkipNode(yymsp[0].minor.yy0.longval);
}
#line 2278 "grammar.c"
        break;
      case 117: /* limitClause ::= */
#line 666 "grammar.y"
{
	yymsp[1].minor.yy39 = NULL;
}
#line 2285 "grammar.c"
        break;
      case 118: /* limitClause ::= LIMIT INTEGER */
#line 669 "grammar.y"
{
	yymsp[-1].minor.yy39 = New_AST_LimitNode(yymsp[0].minor.yy0.longval);
}
#line 2292 "grammar.c"
        break;
      case 119: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 675 "grammar.y"
{
	yymsp[-5].minor.yy97 = New_AST_UnwindNode(yymsp[-3].minor.yy210, yymsp[0].minor.yy0.strval);
}
#line 2299 "grammar.c"
        break;
      case 120: /* relation ::= EQ */
#line 680 "grammar.y"
{ yymsp[0].minor.yy6 = EQ; }
#line 2304 "grammar.c"
        break;
      case 121: /* relation ::= GT */
#line 681 "grammar.y"
{ yymsp[0].minor.yy6 = GT; }
#line 2309 "grammar.c"
        break;
      case 122: /* relation ::= LT */
#line 682 "grammar.y"
{ yymsp[0].minor.yy6 = LT; }
#line 2314 "grammar.c"
        break;
      case 123: /* relation ::= LE */
#line 683 "grammar.y"
{ yymsp[0].minor.yy6 = LE; }
#line 2319 "grammar.c"
        break;
      case 124: /* relation ::= GE */
#line 684 "grammar.y"
{ yymsp[0].minor.yy6 = GE; }
#line 2324 "grammar.c"
        break;
      case 125: /* relation ::= NE */
#line 685 "grammar.y"
{ yymsp[0].minor.yy6 = NE; }
#line 2329 "grammar.c"
        break;
      case 126: /* value ::= INTEGER */
#line 690 "grammar.y"
{  yylhsminor.yy150 = SI_LongVal(yymsp[0].minor.yy0.longval); }
#line 2334 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 127: /* value ::= DASH INTEGER */
#line 691 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_LongVal(-yymsp[0].minor.yy0.longval); }
#line 2340 "grammar.c"
        break;
      case 128: /* value ::= STRING */
#line 692 "grammar.y"
{  yylhsminor.yy150 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2345 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 129: /* value ::= FLOAT */
#line 693 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2351 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 130: /* value ::= DASH FLOAT */
#line 694 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2357 "grammar.c"
        break;
      case 131: /* value ::= TRUE */
#line 695 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(1); }
#line 2362 "grammar.c"
        break;
      case 132: /* value ::= FALSE */
#line 696 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(0); }
#line 2367 "grammar.c"
        break;
      case 133: /* value ::= NULLVAL */
#line 697 "grammar.y"
{ yymsp[0].minor.yy150 = SI_NullVal(); }
#line 2372 "grammar.c"
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
#line 32 "grammar.y"

	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
#line 2437 "grammar.c"
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
#line 699 "grammar.y"


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
#line 2685 "grammar.c"
