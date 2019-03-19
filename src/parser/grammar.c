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
#define YYNRULE              135
#define YYNTOKEN             55
#define YY_MAX_SHIFT         153
#define YY_MIN_SHIFTREDUCE   247
#define YY_MAX_SHIFTREDUCE   381
#define YY_ERROR_ACTION      382
#define YY_ACCEPT_ACTION     383
#define YY_NO_ACTION         384
#define YY_MIN_REDUCE        385
#define YY_MAX_REDUCE        519
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
 /*     0 */   388,   16,  387,   31,   85,   41,   81,   88,   86,   78,
 /*    10 */   136,  401,   15,  403,   56,  408,   40,   42,  492,   89,
 /*    20 */    53,  419,  437,   69,  424,  115,  383,   50,  386,  491,
 /*    30 */   146,  434,  145,  406,  107,  482,  120,   78,   43,  401,
 /*    40 */    15,  403,   56,  408,   18,   49,   33,  144,   53,  419,
 /*    50 */   437,   69,  424,  115,   28,   27,  101,  337,  291,   26,
 /*    60 */    32,  497,  497,    8,   10,   23,   29,  124,  376,  103,
 /*    70 */   276,  361,  362,    2,  492,   89,  332,  153,   66,  492,
 /*    80 */    90,  147,  148,  110,    2,  491,  374,  101,  338,    2,
 /*    90 */   491,  483,    3,  151,  478,   29,   23,  117,  148,  376,
 /*   100 */   103,   30,   79,  377,  379,  380,  381,   22,   21,   20,
 /*   110 */    19,  368,  369,  372,  370,  371,  132,  374,  347,   22,
 /*   120 */    21,   20,   19,  368,  369,  372,  370,  371,   83,  101,
 /*   130 */   278,  279,  492,   90,  377,  379,  380,  381,   23,  101,
 /*   140 */   119,  376,  103,  491,  278,  279,  149,  478,    9,   58,
 /*   150 */    68,  376,  103,  101,  373,  402,   73,  398,  113,  374,
 /*   160 */    77,   22,   21,   20,   19,  376,  373,   69,  424,  374,
 /*   170 */   492,   90,   69,  424,    8,   10,  377,  379,  380,  381,
 /*   180 */   118,  491,   33,  374,   60,  477,  377,  379,  380,  381,
 /*   190 */    28,   27,   63,  125,  291,  135,   32,  492,   95,  106,
 /*   200 */   377,  379,  380,  381,   22,   21,   20,   19,  491,    2,
 /*   210 */    22,   21,   20,   19,    1,   98,  492,   37,   59,  110,
 /*   220 */   124,  347,  492,   36,  140,  492,   37,  491,  121,  492,
 /*   230 */    37,   66,  100,  491,   96,   66,  491,  471,  435,  145,
 /*   240 */   491,   99,  150,  492,   95,  427,  492,   95,   22,   21,
 /*   250 */    20,   19,  492,   92,  491,  492,   93,  491,  492,   94,
 /*   260 */    61,  102,   61,  491,   97,   66,  491,  492,  489,  491,
 /*   270 */    28,  492,  488,  492,  104,  492,  105,  385,  491,  492,
 /*   280 */    91,  111,  491,  420,  491,   62,  491,   18,   46,  455,
 /*   290 */   491,   30,   79,  393,  126,  394,  395,   47,  455,  375,
 /*   300 */   141,  109,  116,    2,   48,  153,   45,   51,   45,  497,
 /*   310 */   497,  437,  134,  437,  129,  127,  378,  352,   13,    4,
 /*   320 */    44,   18,  274,   66,  152,   20,   19,   39,   30,  148,
 /*   330 */   407,  405,  147,   57,  122,   24,   66,  123,  124,  456,
 /*   340 */   100,  133,  130,  131,  137,  438,   29,  139,  466,  425,
 /*   350 */   138,  400,   70,  153,   71,    2,   72,  396,   74,   11,
 /*   360 */    75,   76,    5,  304,  143,  346,  390,    6,  367,   25,
 /*   370 */    80,   84,  108,  392,    7,   82,  391,  292,  293,  389,
 /*   380 */    87,   52,  112,  114,   34,  275,   54,  277,   55,   10,
 /*   390 */   311,  314,  128,  316,  315,  313,  309,  307,   64,  308,
 /*   400 */    14,  322,  320,  310,  312,   38,   65,  306,   67,  305,
 /*   410 */   326,   17,  142,  364,  152,   35,   12,  342,  366,  358,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    58,  102,   60,   61,   62,   63,   64,   62,   63,   67,
 /*    10 */   100,   69,   70,   71,   72,   73,   13,   86,   89,   90,
 /*    20 */    78,   79,   91,   81,   82,   83,   56,   57,   58,  100,
 /*    30 */    87,   88,   89,   63,  105,  106,   74,   67,   76,   69,
 /*    40 */    70,   71,   72,   73,   18,   86,   12,   17,   78,   79,
 /*    50 */    91,   81,   82,   83,   20,   21,    4,    5,   24,   17,
 /*    60 */    26,   27,   28,    1,    2,   13,   21,   25,   16,   17,
 /*    70 */    17,   45,   46,   39,   89,   90,   14,   43,   36,   89,
 /*    80 */    90,   47,   48,   49,   39,  100,   34,    4,    5,   39,
 /*    90 */   100,  106,   40,  103,  104,   21,   13,   77,   48,   16,
 /*   100 */    17,   27,   28,   51,   52,   53,   54,    3,    4,    5,
 /*   110 */     6,    7,    8,    9,   10,   11,   94,   34,   14,    3,
 /*   120 */     4,    5,    6,    7,    8,    9,   10,   11,   64,    4,
 /*   130 */    18,   19,   89,   90,   51,   52,   53,   54,   13,    4,
 /*   140 */    14,   16,   17,  100,   18,   19,  103,  104,   13,   63,
 /*   150 */    92,   16,   17,    4,   50,   69,   65,   66,   13,   34,
 /*   160 */    69,    3,    4,    5,    6,   16,   50,   81,   82,   34,
 /*   170 */    89,   90,   81,   82,    1,    2,   51,   52,   53,   54,
 /*   180 */    77,  100,   12,   34,   96,  104,   51,   52,   53,   54,
 /*   190 */    20,   21,    4,   94,   24,   94,   26,   89,   90,   41,
 /*   200 */    51,   52,   53,   54,    3,    4,    5,    6,  100,   39,
 /*   210 */     3,    4,    5,    6,   59,  107,   89,   90,   30,   49,
 /*   220 */    25,   14,   89,   90,   25,   89,   90,  100,  101,   89,
 /*   230 */    90,   36,    5,  100,  101,   36,  100,  101,   88,   89,
 /*   240 */   100,  101,   41,   89,   90,   85,   89,   90,    3,    4,
 /*   250 */     5,    6,   89,   90,  100,   89,   90,  100,   89,   90,
 /*   260 */    33,  107,   33,  100,  107,   36,  100,   89,   90,  100,
 /*   270 */    20,   89,   90,   89,   90,   89,   90,    0,  100,   89,
 /*   280 */    90,   25,  100,   79,  100,   94,  100,   18,   97,   98,
 /*   290 */   100,   27,   28,   63,   94,   65,   66,   97,   98,   34,
 /*   300 */    80,   32,   80,   39,   17,   43,   86,   84,   86,   47,
 /*   310 */    48,   91,   25,   91,   34,   35,   51,   14,   13,   42,
 /*   320 */    76,   18,   16,   36,   19,    5,    6,   75,   27,   48,
 /*   330 */    62,   65,   47,   61,   95,   31,   36,   94,   25,   98,
 /*   340 */     5,   94,   96,   95,   17,   91,   21,   94,   99,   82,
 /*   350 */    99,   62,   61,   43,   64,   39,   63,   62,   61,   38,
 /*   360 */    64,   63,   68,   17,   93,   17,   64,   18,   17,   62,
 /*   370 */    61,   63,   41,   62,   31,   61,   64,   17,   14,   64,
 /*   380 */    63,   23,   17,   22,   18,   16,   15,   17,   13,    2,
 /*   390 */     4,   32,   35,   17,   32,   32,   14,   14,   17,   14,
 /*   400 */    44,   34,   34,   29,   32,   25,   18,   14,   17,   17,
 /*   410 */    37,    7,   18,   34,   19,   18,   18,   17,   34,   17,
 /*   420 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   430 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   440 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   450 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   460 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   470 */   108,  108,  108,  108,  108,
};
#define YY_SHIFT_COUNT    (153)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (404)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */   170,   34,   52,   83,  125,   74,  125,  125,  135,  135,
 /*    10 */   135,  135,  125,  125,  125,   45,  125,  125,  125,  125,
 /*    20 */   125,  125,  125,  125,   42,  264,  195,    3,    3,    3,
 /*    30 */    30,   50,    3,   53,    3,   30,  104,  116,  149,  126,
 /*    40 */   287,  262,  188,  112,  112,  188,  227,  229,  199,  188,
 /*    50 */   277,  145,  256,  250,   53,  306,  301,  281,  285,  304,
 /*    60 */   300,  313,  335,  304,  300,  327,  327,  300,    3,  325,
 /*    70 */   281,  285,  310,  316,  281,  285,  310,  316,  321,  346,
 /*    80 */   281,  285,  281,  285,  310,  316,  310,  310,  316,  158,
 /*    90 */   201,  207,  245,  245,  245,  245,   62,   26,  269,  173,
 /*   100 */   280,  265,  303,  305,  320,  320,  348,  349,  351,  331,
 /*   110 */   343,  360,  364,  365,  358,  361,  366,  369,  370,  371,
 /*   120 */   375,  387,  386,  359,  376,  362,  363,  367,  368,  357,
 /*   130 */   372,  374,  382,  383,  381,  385,  388,  380,  373,  393,
 /*   140 */   391,  366,  392,  394,  395,  404,  397,  379,  384,  398,
 /*   150 */   400,  398,  402,  356,
};
#define YY_REDUCE_COUNT (88)
#define YY_REDUCE_MIN   (-101)
#define YY_REDUCE_MAX   (317)
static const short yy_reduce_ofst[] = {
 /*     0 */   -30,  -58,  -10,   43,  -71,   91,  -15,  108,  127,  133,
 /*    10 */   136,  140,   81,  154,  157,   86,  163,  166,  169,  178,
 /*    20 */   182,  184,  186,  190,  191,  230,  200,  220,  222,  220,
 /*    30 */   -57,  -55,  -69,  -38,  -41,  150, -101, -101,  -90,   20,
 /*    40 */    22,   64,   58,  103,  103,   58,   88,   99,  101,   58,
 /*    50 */   155,  160,  223,  204,  244,  252,  266,  268,  272,  239,
 /*    60 */   243,  241,  246,  248,  247,  249,  251,  253,  254,  267,
 /*    70 */   289,  291,  290,  293,  295,  297,  296,  298,  294,  271,
 /*    80 */   307,  309,  311,  314,  302,  308,  312,  315,  317,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   422,  422,  382,  382,  382,  422,  382,  382,  382,  382,
 /*    10 */   382,  382,  382,  382,  382,  422,  382,  382,  382,  382,
 /*    20 */   382,  382,  382,  382,  463,  382,  463,  428,  382,  382,
 /*    30 */   382,  382,  382,  382,  382,  382,  382,  382,  382,  382,
 /*    40 */   463,  406,  432,  411,  409,  439,  457,  463,  463,  440,
 /*    50 */   382,  382,  382,  418,  382,  382,  404,  503,  501,  382,
 /*    60 */   463,  382,  457,  382,  463,  382,  382,  463,  382,  423,
 /*    70 */   503,  501,  497,  399,  503,  501,  497,  397,  467,  382,
 /*    80 */   503,  501,  503,  501,  497,  382,  497,  497,  382,  382,
 /*    90 */   479,  382,  469,  436,  493,  494,  382,  498,  382,  468,
 /*   100 */   462,  382,  382,  495,  487,  486,  382,  481,  382,  382,
 /*   110 */   382,  382,  382,  382,  382,  382,  421,  382,  382,  410,
 /*   120 */   382,  472,  382,  382,  382,  382,  382,  382,  459,  461,
 /*   130 */   382,  382,  382,  382,  382,  382,  465,  382,  382,  382,
 /*   140 */   382,  426,  382,  441,  495,  382,  433,  382,  382,  474,
 /*   150 */   382,  473,  382,  382,
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
 /*  90 */ "returnClause ::= RETURN MUL",
 /*  91 */ "returnClause ::= RETURN DISTINCT MUL",
 /*  92 */ "returnElements ::= returnElements COMMA returnElement",
 /*  93 */ "returnElements ::= returnElement",
 /*  94 */ "returnElement ::= arithmetic_expression",
 /*  95 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  96 */ "withClause ::= WITH withElements",
 /*  97 */ "withElements ::= withElement",
 /*  98 */ "withElements ::= withElements COMMA withElement",
 /*  99 */ "withElement ::= arithmetic_expression AS UQSTRING",
 /* 100 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /* 101 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /* 102 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /* 103 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /* 104 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /* 105 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /* 106 */ "arithmetic_expression ::= value",
 /* 107 */ "arithmetic_expression ::= variable",
 /* 108 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /* 109 */ "arithmetic_expression_list ::= arithmetic_expression",
 /* 110 */ "variable ::= UQSTRING",
 /* 111 */ "variable ::= UQSTRING DOT UQSTRING",
 /* 112 */ "orderClause ::=",
 /* 113 */ "orderClause ::= ORDER BY arithmetic_expression_list",
 /* 114 */ "orderClause ::= ORDER BY arithmetic_expression_list ASC",
 /* 115 */ "orderClause ::= ORDER BY arithmetic_expression_list DESC",
 /* 116 */ "skipClause ::=",
 /* 117 */ "skipClause ::= SKIP INTEGER",
 /* 118 */ "limitClause ::=",
 /* 119 */ "limitClause ::= LIMIT INTEGER",
 /* 120 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /* 121 */ "relation ::= EQ",
 /* 122 */ "relation ::= GT",
 /* 123 */ "relation ::= LT",
 /* 124 */ "relation ::= LE",
 /* 125 */ "relation ::= GE",
 /* 126 */ "relation ::= NE",
 /* 127 */ "value ::= INTEGER",
 /* 128 */ "value ::= DASH INTEGER",
 /* 129 */ "value ::= STRING",
 /* 130 */ "value ::= FLOAT",
 /* 131 */ "value ::= DASH FLOAT",
 /* 132 */ "value ::= TRUE",
 /* 133 */ "value ::= FALSE",
 /* 134 */ "value ::= NULLVAL",
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
#line 505 "grammar.y"
 Free_AST_FilterNode((yypminor->yy10)); 
#line 862 "grammar.c"
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
  {   63,   -2 }, /* (90) returnClause ::= RETURN MUL */
  {   63,   -3 }, /* (91) returnClause ::= RETURN DISTINCT MUL */
  {  103,   -3 }, /* (92) returnElements ::= returnElements COMMA returnElement */
  {  103,   -1 }, /* (93) returnElements ::= returnElement */
  {  104,   -1 }, /* (94) returnElement ::= arithmetic_expression */
  {  104,   -3 }, /* (95) returnElement ::= arithmetic_expression AS UQSTRING */
  {   59,   -2 }, /* (96) withClause ::= WITH withElements */
  {  105,   -1 }, /* (97) withElements ::= withElement */
  {  105,   -3 }, /* (98) withElements ::= withElements COMMA withElement */
  {  106,   -3 }, /* (99) withElement ::= arithmetic_expression AS UQSTRING */
  {   90,   -3 }, /* (100) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   90,   -3 }, /* (101) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   90,   -3 }, /* (102) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   90,   -3 }, /* (103) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   90,   -3 }, /* (104) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   90,   -4 }, /* (105) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   90,   -1 }, /* (106) arithmetic_expression ::= value */
  {   90,   -1 }, /* (107) arithmetic_expression ::= variable */
  {  107,   -3 }, /* (108) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {  107,   -1 }, /* (109) arithmetic_expression_list ::= arithmetic_expression */
  {   89,   -1 }, /* (110) variable ::= UQSTRING */
  {   89,   -3 }, /* (111) variable ::= UQSTRING DOT UQSTRING */
  {   64,    0 }, /* (112) orderClause ::= */
  {   64,   -3 }, /* (113) orderClause ::= ORDER BY arithmetic_expression_list */
  {   64,   -4 }, /* (114) orderClause ::= ORDER BY arithmetic_expression_list ASC */
  {   64,   -4 }, /* (115) orderClause ::= ORDER BY arithmetic_expression_list DESC */
  {   61,    0 }, /* (116) skipClause ::= */
  {   61,   -2 }, /* (117) skipClause ::= SKIP INTEGER */
  {   62,    0 }, /* (118) limitClause ::= */
  {   62,   -2 }, /* (119) limitClause ::= LIMIT INTEGER */
  {   70,   -6 }, /* (120) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {  102,   -1 }, /* (121) relation ::= EQ */
  {  102,   -1 }, /* (122) relation ::= GT */
  {  102,   -1 }, /* (123) relation ::= LT */
  {  102,   -1 }, /* (124) relation ::= LE */
  {  102,   -1 }, /* (125) relation ::= GE */
  {  102,   -1 }, /* (126) relation ::= NE */
  {  100,   -1 }, /* (127) value ::= INTEGER */
  {  100,   -2 }, /* (128) value ::= DASH INTEGER */
  {  100,   -1 }, /* (129) value ::= STRING */
  {  100,   -1 }, /* (130) value ::= FLOAT */
  {  100,   -2 }, /* (131) value ::= DASH FLOAT */
  {  100,   -1 }, /* (132) value ::= TRUE */
  {  100,   -1 }, /* (133) value ::= FALSE */
  {  100,   -1 }, /* (134) value ::= NULLVAL */
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
{ ctx->root = yymsp[0].minor.yy121; }
#line 1374 "grammar.c"
        break;
      case 1: /* expressions ::= expr */
#line 47 "grammar.y"
{
	yylhsminor.yy121 = array_new(AST*, 1);
	yylhsminor.yy121 = array_append(yylhsminor.yy121, yymsp[0].minor.yy31);
}
#line 1382 "grammar.c"
  yymsp[0].minor.yy121 = yylhsminor.yy121;
        break;
      case 2: /* expressions ::= expressions withClause singlePartQuery */
#line 52 "grammar.y"
{
	AST *ast = yymsp[-2].minor.yy121[array_len(yymsp[-2].minor.yy121)-1];
	ast->withNode = yymsp[-1].minor.yy168;
	yylhsminor.yy121 = array_append(yymsp[-2].minor.yy121, yymsp[0].minor.yy31);
	yylhsminor.yy121=yymsp[-2].minor.yy121;
}
#line 1393 "grammar.c"
  yymsp[-2].minor.yy121 = yylhsminor.yy121;
        break;
      case 3: /* singlePartQuery ::= expr */
#line 60 "grammar.y"
{
	yylhsminor.yy31 = yymsp[0].minor.yy31;
}
#line 1401 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 4: /* singlePartQuery ::= skipClause limitClause returnClause orderClause */
#line 64 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, yymsp[-3].minor.yy147, yymsp[-2].minor.yy39, NULL, NULL, NULL);
}
#line 1409 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 5: /* singlePartQuery ::= limitClause returnClause orderClause */
#line 68 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, NULL, yymsp[-2].minor.yy39, NULL, NULL, NULL);
}
#line 1417 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 6: /* singlePartQuery ::= skipClause returnClause orderClause */
#line 72 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, yymsp[-2].minor.yy147, NULL, NULL, NULL, NULL);
}
#line 1425 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 7: /* singlePartQuery ::= returnClause orderClause skipClause limitClause */
#line 76 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1433 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 8: /* singlePartQuery ::= orderClause skipClause limitClause returnClause */
#line 80 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1441 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 9: /* singlePartQuery ::= orderClause skipClause limitClause setClause */
#line 84 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, yymsp[0].minor.yy44, NULL, NULL, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1449 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 10: /* singlePartQuery ::= orderClause skipClause limitClause deleteClause */
#line 88 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy47, NULL, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1457 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 11: /* expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
#line 93 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-6].minor.yy173, yymsp[-5].minor.yy3, yymsp[-4].minor.yy4, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1465 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 12: /* expr ::= multipleMatchClause whereClause multipleCreateClause */
#line 97 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1473 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 13: /* expr ::= multipleMatchClause whereClause deleteClause */
#line 101 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, NULL, NULL, NULL, yymsp[0].minor.yy47, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1481 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 14: /* expr ::= multipleMatchClause whereClause setClause */
#line 105 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, NULL, NULL, yymsp[0].minor.yy44, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1489 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 15: /* expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 109 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-6].minor.yy173, yymsp[-5].minor.yy3, NULL, NULL, yymsp[-4].minor.yy44, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1497 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 16: /* expr ::= multipleCreateClause */
#line 113 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1505 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 17: /* expr ::= unwindClause multipleCreateClause */
#line 117 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy97, NULL);
}
#line 1513 "grammar.c"
  yymsp[-1].minor.yy31 = yylhsminor.yy31;
        break;
      case 18: /* expr ::= indexClause */
#line 121 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy169, NULL, NULL);
}
#line 1521 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 19: /* expr ::= mergeClause */
#line 125 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, yymsp[0].minor.yy164, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1529 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 20: /* expr ::= mergeClause setClause */
#line 129 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, yymsp[-1].minor.yy164, yymsp[0].minor.yy44, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1537 "grammar.c"
  yymsp[-1].minor.yy31 = yylhsminor.yy31;
        break;
      case 21: /* expr ::= returnClause */
#line 133 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1545 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 22: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 137 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy120, NULL, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, yymsp[-3].minor.yy97, NULL);
}
#line 1553 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 23: /* expr ::= procedureCallClause */
#line 141 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy157);
}
#line 1561 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 24: /* procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList */
#line 146 "grammar.y"
{
	yymsp[-6].minor.yy157 = New_AST_ProcedureCallNode(yymsp[-5].minor.yy129, yymsp[-3].minor.yy5, yymsp[0].minor.yy5);
}
#line 1569 "grammar.c"
        break;
      case 25: /* procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS */
#line 150 "grammar.y"
{	
	yymsp[-4].minor.yy157 = New_AST_ProcedureCallNode(yymsp[-3].minor.yy129, yymsp[-1].minor.yy5, NULL);
}
#line 1576 "grammar.c"
        break;
      case 26: /* procedureName ::= unquotedStringList */
#line 155 "grammar.y"
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
#line 1603 "grammar.c"
  yymsp[0].minor.yy129 = yylhsminor.yy129;
        break;
      case 27: /* stringList ::= STRING */
      case 29: /* unquotedStringList ::= UQSTRING */ yytestcase(yyruleno==29);
#line 180 "grammar.y"
{
	yylhsminor.yy5 = array_new(char*, 1);
	yylhsminor.yy5 = array_append(yylhsminor.yy5, yymsp[0].minor.yy0.strval);
}
#line 1613 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 28: /* stringList ::= stringList delimiter STRING */
      case 30: /* unquotedStringList ::= unquotedStringList delimiter UQSTRING */ yytestcase(yyruleno==30);
#line 186 "grammar.y"
{
	yymsp[-2].minor.yy5 = array_append(yymsp[-2].minor.yy5, yymsp[0].minor.yy0.strval);
	yylhsminor.yy5 = yymsp[-2].minor.yy5;
}
#line 1623 "grammar.c"
  yymsp[-2].minor.yy5 = yylhsminor.yy5;
        break;
      case 31: /* delimiter ::= COMMA */
#line 204 "grammar.y"
{ yymsp[0].minor.yy6 = COMMA; }
#line 1629 "grammar.c"
        break;
      case 32: /* delimiter ::= DOT */
#line 205 "grammar.y"
{ yymsp[0].minor.yy6 = DOT; }
#line 1634 "grammar.c"
        break;
      case 33: /* multipleMatchClause ::= matchClauses */
#line 208 "grammar.y"
{
	yylhsminor.yy173 = New_AST_MatchNode(yymsp[0].minor.yy210);
}
#line 1641 "grammar.c"
  yymsp[0].minor.yy173 = yylhsminor.yy173;
        break;
      case 34: /* matchClauses ::= matchClause */
      case 39: /* createClauses ::= createClause */ yytestcase(yyruleno==39);
#line 214 "grammar.y"
{
	yylhsminor.yy210 = yymsp[0].minor.yy210;
}
#line 1650 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 35: /* matchClauses ::= matchClauses matchClause */
      case 40: /* createClauses ::= createClauses createClause */ yytestcase(yyruleno==40);
#line 218 "grammar.y"
{
	Vector *v;
	while(Vector_Pop(yymsp[0].minor.yy210, &v)) Vector_Push(yymsp[-1].minor.yy210, v);
	Vector_Free(yymsp[0].minor.yy210);
	yylhsminor.yy210 = yymsp[-1].minor.yy210;
}
#line 1662 "grammar.c"
  yymsp[-1].minor.yy210 = yylhsminor.yy210;
        break;
      case 36: /* matchClause ::= MATCH chains */
      case 41: /* createClause ::= CREATE chains */ yytestcase(yyruleno==41);
#line 227 "grammar.y"
{
	yymsp[-1].minor.yy210 = yymsp[0].minor.yy210;
}
#line 1671 "grammar.c"
        break;
      case 37: /* multipleCreateClause ::= */
#line 232 "grammar.y"
{
	yymsp[1].minor.yy4 = NULL;
}
#line 1678 "grammar.c"
        break;
      case 38: /* multipleCreateClause ::= createClauses */
#line 236 "grammar.y"
{
	yylhsminor.yy4 = New_AST_CreateNode(yymsp[0].minor.yy210);
}
#line 1685 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 42: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 262 "grammar.y"
{
  yylhsminor.yy169 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy177);
}
#line 1693 "grammar.c"
  yymsp[-4].minor.yy169 = yylhsminor.yy169;
        break;
      case 43: /* indexOpToken ::= CREATE */
#line 268 "grammar.y"
{ yymsp[0].minor.yy177 = CREATE_INDEX; }
#line 1699 "grammar.c"
        break;
      case 44: /* indexOpToken ::= DROP */
#line 269 "grammar.y"
{ yymsp[0].minor.yy177 = DROP_INDEX; }
#line 1704 "grammar.c"
        break;
      case 45: /* indexLabel ::= COLON UQSTRING */
#line 271 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1711 "grammar.c"
        break;
      case 46: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 275 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1718 "grammar.c"
        break;
      case 47: /* mergeClause ::= MERGE chain */
#line 281 "grammar.y"
{
	yymsp[-1].minor.yy164 = New_AST_MergeNode(yymsp[0].minor.yy210);
}
#line 1725 "grammar.c"
        break;
      case 48: /* setClause ::= SET setList */
#line 286 "grammar.y"
{
	yymsp[-1].minor.yy44 = New_AST_SetNode(yymsp[0].minor.yy210);
}
#line 1732 "grammar.c"
        break;
      case 49: /* setList ::= setElement */
#line 291 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy170);
}
#line 1740 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 50: /* setList ::= setList COMMA setElement */
#line 295 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy170);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1749 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 51: /* setElement ::= variable EQ arithmetic_expression */
#line 301 "grammar.y"
{
	yylhsminor.yy170 = New_AST_SetElement(yymsp[-2].minor.yy156, yymsp[0].minor.yy154);
}
#line 1757 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 52: /* chain ::= node */
#line 307 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy117);
}
#line 1766 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 53: /* chain ::= chain link node */
#line 312 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[-1].minor.yy69);
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy117);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1776 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 54: /* chains ::= chain */
#line 320 "grammar.y"
{
	yylhsminor.yy210 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy210);
}
#line 1785 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 55: /* chains ::= chains COMMA chain */
#line 325 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy210);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1794 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 56: /* deleteClause ::= DELETE deleteExpression */
#line 333 "grammar.y"
{
	yymsp[-1].minor.yy47 = New_AST_DeleteNode(yymsp[0].minor.yy210);
}
#line 1802 "grammar.c"
        break;
      case 57: /* deleteExpression ::= UQSTRING */
#line 339 "grammar.y"
{
	yylhsminor.yy210 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy0.strval);
}
#line 1810 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 58: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 344 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy0.strval);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1819 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 59: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 352 "grammar.y"
{
	yymsp[-5].minor.yy117 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 1827 "grammar.c"
        break;
      case 60: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 357 "grammar.y"
{
	yymsp[-4].minor.yy117 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 1834 "grammar.c"
        break;
      case 61: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 362 "grammar.y"
{
	yymsp[-3].minor.yy117 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy210);
}
#line 1841 "grammar.c"
        break;
      case 62: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 367 "grammar.y"
{
	yymsp[-2].minor.yy117 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy210);
}
#line 1848 "grammar.c"
        break;
      case 63: /* link ::= DASH edge RIGHT_ARROW */
#line 374 "grammar.y"
{
	yymsp[-2].minor.yy69 = yymsp[-1].minor.yy69;
	yymsp[-2].minor.yy69->direction = N_LEFT_TO_RIGHT;
}
#line 1856 "grammar.c"
        break;
      case 64: /* link ::= LEFT_ARROW edge DASH */
#line 380 "grammar.y"
{
	yymsp[-2].minor.yy69 = yymsp[-1].minor.yy69;
	yymsp[-2].minor.yy69->direction = N_RIGHT_TO_LEFT;
}
#line 1864 "grammar.c"
        break;
      case 65: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 387 "grammar.y"
{ 
	yymsp[-3].minor.yy69 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy210, N_DIR_UNKNOWN, yymsp[-1].minor.yy138);
}
#line 1871 "grammar.c"
        break;
      case 66: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 392 "grammar.y"
{ 
	yymsp[-3].minor.yy69 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, NULL);
}
#line 1878 "grammar.c"
        break;
      case 67: /* edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
#line 397 "grammar.y"
{ 
	yymsp[-4].minor.yy69 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy5, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, yymsp[-2].minor.yy138);
}
#line 1885 "grammar.c"
        break;
      case 68: /* edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
#line 402 "grammar.y"
{ 
	yymsp[-4].minor.yy69 = New_AST_LinkEntity(yymsp[-3].minor.yy0.strval, yymsp[-2].minor.yy5, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, NULL);
}
#line 1892 "grammar.c"
        break;
      case 69: /* edgeLabel ::= COLON UQSTRING */
#line 409 "grammar.y"
{
	yymsp[-1].minor.yy129 = yymsp[0].minor.yy0.strval;
}
#line 1899 "grammar.c"
        break;
      case 70: /* edgeLabels ::= edgeLabel */
#line 414 "grammar.y"
{
	yylhsminor.yy5 = array_new(char*, 1);
	yylhsminor.yy5 = array_append(yylhsminor.yy5, yymsp[0].minor.yy129);
}
#line 1907 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 71: /* edgeLabels ::= edgeLabels PIPE edgeLabel */
#line 420 "grammar.y"
{
	char *label = yymsp[0].minor.yy129;
	yymsp[-2].minor.yy5 = array_append(yymsp[-2].minor.yy5, label);
	yylhsminor.yy5 = yymsp[-2].minor.yy5;
}
#line 1917 "grammar.c"
  yymsp[-2].minor.yy5 = yylhsminor.yy5;
        break;
      case 72: /* edgeLength ::= */
#line 429 "grammar.y"
{
	yymsp[1].minor.yy138 = NULL;
}
#line 1925 "grammar.c"
        break;
      case 73: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 434 "grammar.y"
{
	yymsp[-3].minor.yy138 = New_AST_LinkLength(yymsp[-2].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1932 "grammar.c"
        break;
      case 74: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 439 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(yymsp[-1].minor.yy0.longval, UINT_MAX-2);
}
#line 1939 "grammar.c"
        break;
      case 75: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 444 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(1, yymsp[0].minor.yy0.longval);
}
#line 1946 "grammar.c"
        break;
      case 76: /* edgeLength ::= MUL INTEGER */
#line 449 "grammar.y"
{
	yymsp[-1].minor.yy138 = New_AST_LinkLength(yymsp[0].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1953 "grammar.c"
        break;
      case 77: /* edgeLength ::= MUL */
#line 454 "grammar.y"
{
	yymsp[0].minor.yy138 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1960 "grammar.c"
        break;
      case 78: /* properties ::= */
#line 460 "grammar.y"
{
	yymsp[1].minor.yy210 = NULL;
}
#line 1967 "grammar.c"
        break;
      case 79: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 464 "grammar.y"
{
	yymsp[-2].minor.yy210 = yymsp[-1].minor.yy210;
}
#line 1974 "grammar.c"
        break;
      case 80: /* mapLiteral ::= UQSTRING COLON value */
#line 470 "grammar.y"
{
	yylhsminor.yy210 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy210, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy150;
	Vector_Push(yylhsminor.yy210, val);
}
#line 1989 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 81: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 482 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy210, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy150;
	Vector_Push(yymsp[0].minor.yy210, val);
	
	yylhsminor.yy210 = yymsp[0].minor.yy210;
}
#line 2005 "grammar.c"
  yymsp[-4].minor.yy210 = yylhsminor.yy210;
        break;
      case 82: /* whereClause ::= */
#line 496 "grammar.y"
{ 
	yymsp[1].minor.yy3 = NULL;
}
#line 2013 "grammar.c"
        break;
      case 83: /* whereClause ::= WHERE cond */
#line 499 "grammar.y"
{
	yymsp[-1].minor.yy3 = New_AST_WhereNode(yymsp[0].minor.yy10);
}
#line 2020 "grammar.c"
        break;
      case 84: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 508 "grammar.y"
{ yylhsminor.yy10 = New_AST_PredicateNode(yymsp[-2].minor.yy154, yymsp[-1].minor.yy6, yymsp[0].minor.yy154); }
#line 2025 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 85: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 510 "grammar.y"
{ yymsp[-2].minor.yy10 = yymsp[-1].minor.yy10; }
#line 2031 "grammar.c"
        break;
      case 86: /* cond ::= cond AND cond */
#line 511 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, AND, yymsp[0].minor.yy10); }
#line 2036 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 87: /* cond ::= cond OR cond */
#line 512 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, OR, yymsp[0].minor.yy10); }
#line 2042 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 88: /* returnClause ::= RETURN returnElements */
#line 516 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy122, 0);
}
#line 2050 "grammar.c"
        break;
      case 89: /* returnClause ::= RETURN DISTINCT returnElements */
#line 519 "grammar.y"
{
	yymsp[-2].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy122, 1);
}
#line 2057 "grammar.c"
        break;
      case 90: /* returnClause ::= RETURN MUL */
#line 523 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_ReturnNode(NULL, 0);
}
#line 2064 "grammar.c"
        break;
      case 91: /* returnClause ::= RETURN DISTINCT MUL */
#line 526 "grammar.y"
{
	yymsp[-2].minor.yy120 = New_AST_ReturnNode(NULL, 1);
}
#line 2071 "grammar.c"
        break;
      case 92: /* returnElements ::= returnElements COMMA returnElement */
#line 532 "grammar.y"
{
	yylhsminor.yy122 = array_append(yymsp[-2].minor.yy122, yymsp[0].minor.yy139);
}
#line 2078 "grammar.c"
  yymsp[-2].minor.yy122 = yylhsminor.yy122;
        break;
      case 93: /* returnElements ::= returnElement */
#line 536 "grammar.y"
{
	yylhsminor.yy122 = array_new(AST_ReturnElementNode*, 1);
	array_append(yylhsminor.yy122, yymsp[0].minor.yy139);
}
#line 2087 "grammar.c"
  yymsp[0].minor.yy122 = yylhsminor.yy122;
        break;
      case 94: /* returnElement ::= arithmetic_expression */
#line 543 "grammar.y"
{
	yylhsminor.yy139 = New_AST_ReturnElementNode(yymsp[0].minor.yy154, NULL);
}
#line 2095 "grammar.c"
  yymsp[0].minor.yy139 = yylhsminor.yy139;
        break;
      case 95: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 547 "grammar.y"
{
	yylhsminor.yy139 = New_AST_ReturnElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 2103 "grammar.c"
  yymsp[-2].minor.yy139 = yylhsminor.yy139;
        break;
      case 96: /* withClause ::= WITH withElements */
#line 552 "grammar.y"
{
	yymsp[-1].minor.yy168 = New_AST_WithNode(yymsp[0].minor.yy112);
}
#line 2111 "grammar.c"
        break;
      case 97: /* withElements ::= withElement */
#line 557 "grammar.y"
{
	yylhsminor.yy112 = array_new(AST_WithElementNode*, 1);
	array_append(yylhsminor.yy112, yymsp[0].minor.yy98);
}
#line 2119 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 98: /* withElements ::= withElements COMMA withElement */
#line 561 "grammar.y"
{
	yylhsminor.yy112 = array_append(yymsp[-2].minor.yy112, yymsp[0].minor.yy98);
}
#line 2127 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 99: /* withElement ::= arithmetic_expression AS UQSTRING */
#line 566 "grammar.y"
{
	yylhsminor.yy98 = New_AST_WithElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 2135 "grammar.c"
  yymsp[-2].minor.yy98 = yylhsminor.yy98;
        break;
      case 100: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 573 "grammar.y"
{
	yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154;
}
#line 2143 "grammar.c"
        break;
      case 101: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 579 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 2153 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 102: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 586 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 2164 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 103: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 593 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 2175 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 104: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 600 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 2186 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 105: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 608 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 2194 "grammar.c"
  yymsp[-3].minor.yy154 = yylhsminor.yy154;
        break;
      case 106: /* arithmetic_expression ::= value */
#line 613 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy150);
}
#line 2202 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 107: /* arithmetic_expression ::= variable */
#line 618 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy156->alias, yymsp[0].minor.yy156->property);
	free(yymsp[0].minor.yy156);
}
#line 2211 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 108: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 625 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy154);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 2220 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 109: /* arithmetic_expression_list ::= arithmetic_expression */
#line 629 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy154);
}
#line 2229 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 110: /* variable ::= UQSTRING */
#line 636 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 2237 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 111: /* variable ::= UQSTRING DOT UQSTRING */
#line 640 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 2245 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 112: /* orderClause ::= */
#line 646 "grammar.y"
{
	yymsp[1].minor.yy196 = NULL;
}
#line 2253 "grammar.c"
        break;
      case 113: /* orderClause ::= ORDER BY arithmetic_expression_list */
#line 649 "grammar.y"
{
	yymsp[-2].minor.yy196 = New_AST_OrderNode(yymsp[0].minor.yy210, ORDER_DIR_ASC);
}
#line 2260 "grammar.c"
        break;
      case 114: /* orderClause ::= ORDER BY arithmetic_expression_list ASC */
#line 652 "grammar.y"
{
	yymsp[-3].minor.yy196 = New_AST_OrderNode(yymsp[-1].minor.yy210, ORDER_DIR_ASC);
}
#line 2267 "grammar.c"
        break;
      case 115: /* orderClause ::= ORDER BY arithmetic_expression_list DESC */
#line 655 "grammar.y"
{
	yymsp[-3].minor.yy196 = New_AST_OrderNode(yymsp[-1].minor.yy210, ORDER_DIR_DESC);
}
#line 2274 "grammar.c"
        break;
      case 116: /* skipClause ::= */
#line 661 "grammar.y"
{
	yymsp[1].minor.yy147 = NULL;
}
#line 2281 "grammar.c"
        break;
      case 117: /* skipClause ::= SKIP INTEGER */
#line 664 "grammar.y"
{
	yymsp[-1].minor.yy147 = New_AST_SkipNode(yymsp[0].minor.yy0.longval);
}
#line 2288 "grammar.c"
        break;
      case 118: /* limitClause ::= */
#line 670 "grammar.y"
{
	yymsp[1].minor.yy39 = NULL;
}
#line 2295 "grammar.c"
        break;
      case 119: /* limitClause ::= LIMIT INTEGER */
#line 673 "grammar.y"
{
	yymsp[-1].minor.yy39 = New_AST_LimitNode(yymsp[0].minor.yy0.longval);
}
#line 2302 "grammar.c"
        break;
      case 120: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 679 "grammar.y"
{
	yymsp[-5].minor.yy97 = New_AST_UnwindNode(yymsp[-3].minor.yy210, yymsp[0].minor.yy0.strval);
}
#line 2309 "grammar.c"
        break;
      case 121: /* relation ::= EQ */
#line 684 "grammar.y"
{ yymsp[0].minor.yy6 = EQ; }
#line 2314 "grammar.c"
        break;
      case 122: /* relation ::= GT */
#line 685 "grammar.y"
{ yymsp[0].minor.yy6 = GT; }
#line 2319 "grammar.c"
        break;
      case 123: /* relation ::= LT */
#line 686 "grammar.y"
{ yymsp[0].minor.yy6 = LT; }
#line 2324 "grammar.c"
        break;
      case 124: /* relation ::= LE */
#line 687 "grammar.y"
{ yymsp[0].minor.yy6 = LE; }
#line 2329 "grammar.c"
        break;
      case 125: /* relation ::= GE */
#line 688 "grammar.y"
{ yymsp[0].minor.yy6 = GE; }
#line 2334 "grammar.c"
        break;
      case 126: /* relation ::= NE */
#line 689 "grammar.y"
{ yymsp[0].minor.yy6 = NE; }
#line 2339 "grammar.c"
        break;
      case 127: /* value ::= INTEGER */
#line 694 "grammar.y"
{  yylhsminor.yy150 = SI_LongVal(yymsp[0].minor.yy0.longval); }
#line 2344 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 128: /* value ::= DASH INTEGER */
#line 695 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_LongVal(-yymsp[0].minor.yy0.longval); }
#line 2350 "grammar.c"
        break;
      case 129: /* value ::= STRING */
#line 696 "grammar.y"
{  yylhsminor.yy150 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2355 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 130: /* value ::= FLOAT */
#line 697 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2361 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 131: /* value ::= DASH FLOAT */
#line 698 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2367 "grammar.c"
        break;
      case 132: /* value ::= TRUE */
#line 699 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(1); }
#line 2372 "grammar.c"
        break;
      case 133: /* value ::= FALSE */
#line 700 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(0); }
#line 2377 "grammar.c"
        break;
      case 134: /* value ::= NULLVAL */
#line 701 "grammar.y"
{ yymsp[0].minor.yy150 = SI_NullVal(); }
#line 2382 "grammar.c"
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
#line 2447 "grammar.c"
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
#line 703 "grammar.y"


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
#line 2695 "grammar.c"
