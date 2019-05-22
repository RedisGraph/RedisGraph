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
#define YYNSTATE             164
#define YYNRULE              138
#define YYNTOKEN             55
#define YY_MAX_SHIFT         163
#define YY_MIN_SHIFTREDUCE   258
#define YY_MAX_SHIFTREDUCE   395
#define YY_ERROR_ACTION      396
#define YY_ACCEPT_ACTION     397
#define YY_NO_ACTION         398
#define YY_MIN_REDUCE        399
#define YY_MAX_REDUCE        536
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
#define YY_ACTTAB_COUNT (441)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   402,   17,  401,   32,   95,   42,   91,   98,   96,   88,
 /*    10 */   146,  415,   16,  417,   66,   15,   41,   43,  509,   99,
 /*    20 */    56,  436,  454,   79,  441,  125,  397,   51,  400,  508,
 /*    30 */   156,  451,  155,  420,  117,  499,  129,   88,   44,  415,
 /*    40 */    16,  417,   66,   15,   73,   29,   34,   71,   56,  436,
 /*    50 */    76,   79,  441,  125,   29,   28,  111,  351,  305,   27,
 /*    60 */    33,  514,  514,   11,  484,   24,  154,  134,  390,  113,
 /*    70 */    69,    8,   10,    2,  509,   99,  290,  163,   76,  509,
 /*    80 */   100,  157,  158,  120,  346,  508,  388,  111,  352,    2,
 /*    90 */   508,  500,    3,  161,  495,   30,   24,  126,  158,  390,
 /*   100 */   113,   31,   89,  391,  393,  394,  395,   23,   22,   21,
 /*   110 */    20,  382,  383,  386,  384,  385,  142,  388,  361,   23,
 /*   120 */    22,   21,   20,  382,  383,  386,  384,  385,  407,  111,
 /*   130 */   408,  409,  509,  100,  391,  393,  394,  395,   24,  111,
 /*   140 */   128,  390,  113,  508,  292,  293,  159,  495,    9,   68,
 /*   150 */    78,  390,  113,  111,  387,  416,   83,  412,   93,  388,
 /*   160 */    87,   23,   22,   21,   20,  390,  387,   79,  441,  388,
 /*   170 */   509,  100,   79,  441,   30,   50,  391,  393,  394,  395,
 /*   180 */   454,  508,   34,  388,  127,  494,  391,  393,  394,  395,
 /*   190 */    29,   28,    2,   70,  305,  135,   33,  509,  105,  116,
 /*   200 */   391,  393,  394,  395,   23,   22,   21,   20,  508,    2,
 /*   210 */    23,   22,   21,   20,  145,  108,  509,   38,  399,  120,
 /*   220 */   134,  361,  509,   37,    1,  509,   38,  508,  131,  509,
 /*   230 */    38,   76,  123,  508,  106,  110,  508,  488,   61,   65,
 /*   240 */   508,  109,  160,  509,  105,  444,  509,  105,  121,   56,
 /*   250 */   436,  509,  102,   45,  508,  452,  155,  508,  509,  103,
 /*   260 */     4,  112,  508,   71,  107,   23,   22,   21,   20,  508,
 /*   270 */   150,  509,  104,  509,  506,  509,  505,  509,  114,  509,
 /*   280 */   115,   76,  508,   40,  508,  366,  508,   19,  508,   19,
 /*   290 */   508,  509,  101,   72,   31,   89,   47,  472,   19,   13,
 /*   300 */   136,  119,  508,   48,  472,  162,    2,  151,  130,   49,
 /*   310 */   389,  163,  288,   46,   46,  514,  514,  144,  454,  454,
 /*   320 */    52,  292,  293,    8,   10,  375,  376,  392,   76,  139,
 /*   330 */   137,   21,   20,   29,  437,  158,  424,  157,   57,   58,
 /*   340 */   163,    2,   59,   11,  423,   62,   60,   63,   64,   25,
 /*   350 */    31,  421,   67,  419,  132,   76,  134,  133,  473,  110,
 /*   360 */   140,  141,  147,   30,  143,   81,  414,   80,    5,   84,
 /*   370 */    82,  483,  148,  410,   85,  149,  442,   86,  318,   26,
 /*   380 */   404,   90,  360,  406,  455,   92,    6,  381,   94,  118,
 /*   390 */    97,  153,  405,  403,    7,  306,  307,  122,   53,  124,
 /*   400 */   289,   54,  291,   55,   10,   35,  328,  325,  330,  323,
 /*   410 */   336,  329,  321,  327,   74,  322,   77,  334,   75,   39,
 /*   420 */   320,  326,  324,  138,  319,   18,  152,  356,  162,  340,
 /*   430 */    36,   12,  372,  398,  398,  398,  398,  398,  378,  380,
 /*   440 */    14,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    58,  102,   60,   61,   62,   63,   64,   62,   63,   67,
 /*    10 */   100,   69,   70,   71,   72,   73,   13,   86,   89,   90,
 /*    20 */    78,   79,   91,   81,   82,   83,   56,   57,   58,  100,
 /*    30 */    87,   88,   89,   63,  105,  106,   74,   67,   76,   69,
 /*    40 */    70,   71,   72,   73,    4,   20,   12,   33,   78,   79,
 /*    50 */    36,   81,   82,   83,   20,   21,    4,    5,   24,   17,
 /*    60 */    26,   27,   28,   38,   39,   13,   17,   25,   16,   17,
 /*    70 */    30,    1,    2,   39,   89,   90,   17,   43,   36,   89,
 /*    80 */    90,   47,   48,   49,   14,  100,   34,    4,    5,   39,
 /*    90 */   100,  106,   40,  103,  104,   21,   13,   77,   48,   16,
 /*   100 */    17,   27,   28,   51,   52,   53,   54,    3,    4,    5,
 /*   110 */     6,    7,    8,    9,   10,   11,   94,   34,   14,    3,
 /*   120 */     4,    5,    6,    7,    8,    9,   10,   11,   63,    4,
 /*   130 */    65,   66,   89,   90,   51,   52,   53,   54,   13,    4,
 /*   140 */    14,   16,   17,  100,   18,   19,  103,  104,   13,   63,
 /*   150 */    92,   16,   17,    4,   50,   69,   65,   66,   64,   34,
 /*   160 */    69,    3,    4,    5,    6,   16,   50,   81,   82,   34,
 /*   170 */    89,   90,   81,   82,   21,   86,   51,   52,   53,   54,
 /*   180 */    91,  100,   12,   34,   77,  104,   51,   52,   53,   54,
 /*   190 */    20,   21,   39,   96,   24,   94,   26,   89,   90,   41,
 /*   200 */    51,   52,   53,   54,    3,    4,    5,    6,  100,   39,
 /*   210 */     3,    4,    5,    6,   94,  107,   89,   90,    0,   49,
 /*   220 */    25,   14,   89,   90,   59,   89,   90,  100,  101,   89,
 /*   230 */    90,   36,   13,  100,  101,    5,  100,  101,   67,   68,
 /*   240 */   100,  101,   41,   89,   90,   85,   89,   90,   25,   78,
 /*   250 */    79,   89,   90,   76,  100,   88,   89,  100,   89,   90,
 /*   260 */    42,  107,  100,   33,  107,    3,    4,    5,    6,  100,
 /*   270 */    25,   89,   90,   89,   90,   89,   90,   89,   90,   89,
 /*   280 */    90,   36,  100,   75,  100,   14,  100,   18,  100,   18,
 /*   290 */   100,   89,   90,   94,   27,   28,   97,   98,   18,   13,
 /*   300 */    94,   32,  100,   97,   98,   19,   39,   80,   80,   17,
 /*   310 */    34,   43,   16,   86,   86,   47,   48,   25,   91,   91,
 /*   320 */    84,   18,   19,    1,    2,   45,   46,   51,   36,   34,
 /*   330 */    35,    5,    6,   20,   79,   48,   62,   47,   61,   64,
 /*   340 */    43,   39,   63,   38,   62,   61,   68,   64,   63,   31,
 /*   350 */    27,   62,   61,   65,   95,   36,   25,   94,   98,    5,
 /*   360 */    96,   95,   17,   21,   94,   64,   62,   61,   68,   61,
 /*   370 */    63,   99,   99,   62,   64,   94,   82,   63,   17,   62,
 /*   380 */    64,   61,   17,   62,   91,   61,   18,   17,   63,   41,
 /*   390 */    63,   93,   64,   64,   31,   17,   14,   17,   23,   22,
 /*   400 */    16,   15,   17,   13,    2,   18,   32,    4,   17,   14,
 /*   410 */    34,   32,   14,   32,   17,   14,   17,   34,   18,   25,
 /*   420 */    14,   32,   29,   35,   17,    7,   18,   17,   19,   37,
 /*   430 */    18,   18,   17,  108,  108,  108,  108,  108,   34,   34,
 /*   440 */    44,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   450 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   460 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   470 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   480 */   108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
 /*   490 */   108,  108,  108,  108,  108,  108,
};
#define YY_SHIFT_COUNT    (163)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (418)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */   170,   34,   52,   83,  125,   74,  125,  125,  135,  135,
 /*    10 */   135,  135,  125,  125,  125,   25,  153,  125,  125,  125,
 /*    20 */   125,  125,  125,  125,  125,   42,  267,  195,    3,    3,
 /*    30 */     3,   49,   50,    3,   59,    3,   49,  104,  116,  149,
 /*    40 */   126,  292,  268,   40,  303,  303,   40,  230,   14,  245,
 /*    50 */    40,  218,  219,  223,   59,  296,  313,  287,  290,  297,
 /*    60 */   302,  305,  287,  290,  297,  302,  323,  287,  290,  318,
 /*    70 */   319,  331,  354,  318,  319,  345,  345,  319,    3,  342,
 /*    80 */   287,  290,  297,  302,  287,  290,  297,  302,  305,  361,
 /*    90 */   287,  290,  287,  290,  297,  302,  297,  297,  302,  158,
 /*   100 */   201,  207,  262,  262,  262,  262,   70,  280,  269,  322,
 /*   110 */   295,  276,  271,  286,  326,  326,  365,  368,  370,  348,
 /*   120 */   363,  378,  382,  380,  375,  377,  384,  385,  386,  390,
 /*   130 */   387,  402,  403,  374,  391,  379,  381,  376,  383,  388,
 /*   140 */   389,  393,  395,  398,  397,  401,  400,  394,  392,  406,
 /*   150 */   399,  387,  407,  408,  409,  418,  412,  404,  405,  413,
 /*   160 */   410,  413,  415,  396,
};
#define YY_REDUCE_COUNT (98)
#define YY_REDUCE_MIN   (-101)
#define YY_REDUCE_MAX   (329)
static const short yy_reduce_ofst[] = {
 /*     0 */   -30,  -58,  -10,   43,  -71,   91,  -15,  108,  127,  133,
 /*    10 */   136,  140,   81,  154,  157,  171,   86,  162,  169,  182,
 /*    20 */   184,  186,  188,  190,  202,  199,   65,  206,  227,  228,
 /*    30 */   227,  -57,  -55,  -69,  -38,   89,  167, -101, -101,  -90,
 /*    40 */    20,   22,   94,   58,  107,  107,   58,   97,  101,  120,
 /*    50 */    58,  165,  160,  236,  177,  208,  255,  274,  277,  275,
 /*    60 */   279,  278,  282,  284,  283,  285,  288,  289,  291,  259,
 /*    70 */   263,  260,  264,  266,  270,  272,  273,  281,  293,  294,
 /*    80 */   304,  306,  301,  307,  311,  308,  310,  314,  300,  298,
 /*    90 */   317,  320,  321,  324,  316,  325,  328,  329,  327,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   439,  439,  396,  396,  396,  439,  396,  396,  396,  396,
 /*    10 */   396,  396,  396,  396,  396,  422,  439,  396,  396,  396,
 /*    20 */   396,  396,  396,  396,  396,  480,  396,  480,  445,  396,
 /*    30 */   396,  396,  396,  396,  396,  396,  396,  396,  396,  396,
 /*    40 */   396,  480,  420,  449,  427,  425,  456,  474,  480,  480,
 /*    50 */   457,  396,  396,  396,  396,  428,  435,  520,  518,  514,
 /*    60 */   396,  484,  520,  518,  514,  396,  418,  520,  518,  396,
 /*    70 */   480,  396,  474,  396,  480,  396,  396,  480,  396,  440,
 /*    80 */   520,  518,  514,  413,  520,  518,  514,  411,  484,  396,
 /*    90 */   520,  518,  520,  518,  514,  396,  514,  514,  396,  396,
 /*   100 */   496,  396,  486,  453,  510,  511,  396,  515,  396,  485,
 /*   110 */   479,  396,  396,  512,  504,  503,  396,  498,  396,  396,
 /*   120 */   396,  396,  396,  396,  396,  396,  396,  396,  426,  396,
 /*   130 */   438,  489,  396,  396,  396,  396,  396,  396,  476,  478,
 /*   140 */   396,  396,  396,  396,  396,  396,  482,  396,  396,  396,
 /*   150 */   396,  443,  396,  458,  512,  396,  450,  396,  396,  491,
 /*   160 */   396,  490,  396,  396,
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
 /*  24 */ "expr ::= procedureCallClause whereClause returnClause orderClause skipClause limitClause",
 /*  25 */ "expr ::= procedureCallClause multipleMatchClause whereClause returnClause orderClause skipClause limitClause",
 /*  26 */ "procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList",
 /*  27 */ "procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS",
 /*  28 */ "procedureName ::= unquotedStringList",
 /*  29 */ "stringList ::=",
 /*  30 */ "stringList ::= STRING",
 /*  31 */ "stringList ::= stringList delimiter STRING",
 /*  32 */ "unquotedStringList ::= UQSTRING",
 /*  33 */ "unquotedStringList ::= unquotedStringList delimiter UQSTRING",
 /*  34 */ "delimiter ::= COMMA",
 /*  35 */ "delimiter ::= DOT",
 /*  36 */ "multipleMatchClause ::= matchClauses",
 /*  37 */ "matchClauses ::= matchClause",
 /*  38 */ "matchClauses ::= matchClauses matchClause",
 /*  39 */ "matchClause ::= MATCH chains",
 /*  40 */ "multipleCreateClause ::=",
 /*  41 */ "multipleCreateClause ::= createClauses",
 /*  42 */ "createClauses ::= createClause",
 /*  43 */ "createClauses ::= createClauses createClause",
 /*  44 */ "createClause ::= CREATE chains",
 /*  45 */ "indexClause ::= indexOpToken INDEX ON indexLabel indexProp",
 /*  46 */ "indexOpToken ::= CREATE",
 /*  47 */ "indexOpToken ::= DROP",
 /*  48 */ "indexLabel ::= COLON UQSTRING",
 /*  49 */ "indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS",
 /*  50 */ "mergeClause ::= MERGE chain",
 /*  51 */ "setClause ::= SET setList",
 /*  52 */ "setList ::= setElement",
 /*  53 */ "setList ::= setList COMMA setElement",
 /*  54 */ "setElement ::= variable EQ arithmetic_expression",
 /*  55 */ "chain ::= node",
 /*  56 */ "chain ::= chain link node",
 /*  57 */ "chains ::= chain",
 /*  58 */ "chains ::= chains COMMA chain",
 /*  59 */ "deleteClause ::= DELETE deleteExpression",
 /*  60 */ "deleteExpression ::= UQSTRING",
 /*  61 */ "deleteExpression ::= deleteExpression COMMA UQSTRING",
 /*  62 */ "node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  63 */ "node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS",
 /*  64 */ "node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS",
 /*  65 */ "node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS",
 /*  66 */ "link ::= DASH edge RIGHT_ARROW",
 /*  67 */ "link ::= LEFT_ARROW edge DASH",
 /*  68 */ "edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET",
 /*  69 */ "edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET",
 /*  70 */ "edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET",
 /*  71 */ "edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET",
 /*  72 */ "edgeLabel ::= COLON UQSTRING",
 /*  73 */ "edgeLabels ::= edgeLabel",
 /*  74 */ "edgeLabels ::= edgeLabels PIPE edgeLabel",
 /*  75 */ "edgeLength ::=",
 /*  76 */ "edgeLength ::= MUL INTEGER DOTDOT INTEGER",
 /*  77 */ "edgeLength ::= MUL INTEGER DOTDOT",
 /*  78 */ "edgeLength ::= MUL DOTDOT INTEGER",
 /*  79 */ "edgeLength ::= MUL INTEGER",
 /*  80 */ "edgeLength ::= MUL",
 /*  81 */ "properties ::=",
 /*  82 */ "properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET",
 /*  83 */ "mapLiteral ::= UQSTRING COLON value",
 /*  84 */ "mapLiteral ::= UQSTRING COLON value COMMA mapLiteral",
 /*  85 */ "whereClause ::=",
 /*  86 */ "whereClause ::= WHERE cond",
 /*  87 */ "cond ::= arithmetic_expression relation arithmetic_expression",
 /*  88 */ "cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS",
 /*  89 */ "cond ::= cond AND cond",
 /*  90 */ "cond ::= cond OR cond",
 /*  91 */ "returnClause ::= RETURN returnElements",
 /*  92 */ "returnClause ::= RETURN DISTINCT returnElements",
 /*  93 */ "returnClause ::= RETURN MUL",
 /*  94 */ "returnClause ::= RETURN DISTINCT MUL",
 /*  95 */ "returnElements ::= returnElements COMMA returnElement",
 /*  96 */ "returnElements ::= returnElement",
 /*  97 */ "returnElement ::= arithmetic_expression",
 /*  98 */ "returnElement ::= arithmetic_expression AS UQSTRING",
 /*  99 */ "withClause ::= WITH withElements",
 /* 100 */ "withElements ::= withElement",
 /* 101 */ "withElements ::= withElements COMMA withElement",
 /* 102 */ "withElement ::= arithmetic_expression AS UQSTRING",
 /* 103 */ "arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS",
 /* 104 */ "arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression",
 /* 105 */ "arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression",
 /* 106 */ "arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression",
 /* 107 */ "arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression",
 /* 108 */ "arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS",
 /* 109 */ "arithmetic_expression ::= value",
 /* 110 */ "arithmetic_expression ::= variable",
 /* 111 */ "arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression",
 /* 112 */ "arithmetic_expression_list ::= arithmetic_expression",
 /* 113 */ "variable ::= UQSTRING",
 /* 114 */ "variable ::= UQSTRING DOT UQSTRING",
 /* 115 */ "orderClause ::=",
 /* 116 */ "orderClause ::= ORDER BY arithmetic_expression_list",
 /* 117 */ "orderClause ::= ORDER BY arithmetic_expression_list ASC",
 /* 118 */ "orderClause ::= ORDER BY arithmetic_expression_list DESC",
 /* 119 */ "skipClause ::=",
 /* 120 */ "skipClause ::= SKIP INTEGER",
 /* 121 */ "limitClause ::=",
 /* 122 */ "limitClause ::= LIMIT INTEGER",
 /* 123 */ "unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING",
 /* 124 */ "relation ::= EQ",
 /* 125 */ "relation ::= GT",
 /* 126 */ "relation ::= LT",
 /* 127 */ "relation ::= LE",
 /* 128 */ "relation ::= GE",
 /* 129 */ "relation ::= NE",
 /* 130 */ "value ::= INTEGER",
 /* 131 */ "value ::= DASH INTEGER",
 /* 132 */ "value ::= STRING",
 /* 133 */ "value ::= FLOAT",
 /* 134 */ "value ::= DASH FLOAT",
 /* 135 */ "value ::= TRUE",
 /* 136 */ "value ::= FALSE",
 /* 137 */ "value ::= NULLVAL",
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
#line 519 "grammar.y"
 Free_AST_FilterNode((yypminor->yy10)); 
#line 873 "grammar.c"
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
  {   58,   -6 }, /* (24) expr ::= procedureCallClause whereClause returnClause orderClause skipClause limitClause */
  {   58,   -7 }, /* (25) expr ::= procedureCallClause multipleMatchClause whereClause returnClause orderClause skipClause limitClause */
  {   73,   -7 }, /* (26) procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList */
  {   73,   -5 }, /* (27) procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS */
  {   74,   -1 }, /* (28) procedureName ::= unquotedStringList */
  {   75,    0 }, /* (29) stringList ::= */
  {   75,   -1 }, /* (30) stringList ::= STRING */
  {   75,   -3 }, /* (31) stringList ::= stringList delimiter STRING */
  {   76,   -1 }, /* (32) unquotedStringList ::= UQSTRING */
  {   76,   -3 }, /* (33) unquotedStringList ::= unquotedStringList delimiter UQSTRING */
  {   77,   -1 }, /* (34) delimiter ::= COMMA */
  {   77,   -1 }, /* (35) delimiter ::= DOT */
  {   67,   -1 }, /* (36) multipleMatchClause ::= matchClauses */
  {   78,   -1 }, /* (37) matchClauses ::= matchClause */
  {   78,   -2 }, /* (38) matchClauses ::= matchClauses matchClause */
  {   79,   -2 }, /* (39) matchClause ::= MATCH chains */
  {   69,    0 }, /* (40) multipleCreateClause ::= */
  {   69,   -1 }, /* (41) multipleCreateClause ::= createClauses */
  {   81,   -1 }, /* (42) createClauses ::= createClause */
  {   81,   -2 }, /* (43) createClauses ::= createClauses createClause */
  {   82,   -2 }, /* (44) createClause ::= CREATE chains */
  {   71,   -5 }, /* (45) indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
  {   83,   -1 }, /* (46) indexOpToken ::= CREATE */
  {   83,   -1 }, /* (47) indexOpToken ::= DROP */
  {   84,   -2 }, /* (48) indexLabel ::= COLON UQSTRING */
  {   85,   -3 }, /* (49) indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
  {   72,   -2 }, /* (50) mergeClause ::= MERGE chain */
  {   65,   -2 }, /* (51) setClause ::= SET setList */
  {   87,   -1 }, /* (52) setList ::= setElement */
  {   87,   -3 }, /* (53) setList ::= setList COMMA setElement */
  {   88,   -3 }, /* (54) setElement ::= variable EQ arithmetic_expression */
  {   86,   -1 }, /* (55) chain ::= node */
  {   86,   -3 }, /* (56) chain ::= chain link node */
  {   80,   -1 }, /* (57) chains ::= chain */
  {   80,   -3 }, /* (58) chains ::= chains COMMA chain */
  {   66,   -2 }, /* (59) deleteClause ::= DELETE deleteExpression */
  {   93,   -1 }, /* (60) deleteExpression ::= UQSTRING */
  {   93,   -3 }, /* (61) deleteExpression ::= deleteExpression COMMA UQSTRING */
  {   91,   -6 }, /* (62) node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   91,   -5 }, /* (63) node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
  {   91,   -4 }, /* (64) node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
  {   91,   -3 }, /* (65) node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
  {   92,   -3 }, /* (66) link ::= DASH edge RIGHT_ARROW */
  {   92,   -3 }, /* (67) link ::= LEFT_ARROW edge DASH */
  {   95,   -4 }, /* (68) edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
  {   95,   -4 }, /* (69) edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
  {   95,   -5 }, /* (70) edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
  {   95,   -5 }, /* (71) edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
  {   98,   -2 }, /* (72) edgeLabel ::= COLON UQSTRING */
  {   97,   -1 }, /* (73) edgeLabels ::= edgeLabel */
  {   97,   -3 }, /* (74) edgeLabels ::= edgeLabels PIPE edgeLabel */
  {   96,    0 }, /* (75) edgeLength ::= */
  {   96,   -4 }, /* (76) edgeLength ::= MUL INTEGER DOTDOT INTEGER */
  {   96,   -3 }, /* (77) edgeLength ::= MUL INTEGER DOTDOT */
  {   96,   -3 }, /* (78) edgeLength ::= MUL DOTDOT INTEGER */
  {   96,   -2 }, /* (79) edgeLength ::= MUL INTEGER */
  {   96,   -1 }, /* (80) edgeLength ::= MUL */
  {   94,    0 }, /* (81) properties ::= */
  {   94,   -3 }, /* (82) properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
  {   99,   -3 }, /* (83) mapLiteral ::= UQSTRING COLON value */
  {   99,   -5 }, /* (84) mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
  {   68,    0 }, /* (85) whereClause ::= */
  {   68,   -2 }, /* (86) whereClause ::= WHERE cond */
  {  101,   -3 }, /* (87) cond ::= arithmetic_expression relation arithmetic_expression */
  {  101,   -3 }, /* (88) cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
  {  101,   -3 }, /* (89) cond ::= cond AND cond */
  {  101,   -3 }, /* (90) cond ::= cond OR cond */
  {   63,   -2 }, /* (91) returnClause ::= RETURN returnElements */
  {   63,   -3 }, /* (92) returnClause ::= RETURN DISTINCT returnElements */
  {   63,   -2 }, /* (93) returnClause ::= RETURN MUL */
  {   63,   -3 }, /* (94) returnClause ::= RETURN DISTINCT MUL */
  {  103,   -3 }, /* (95) returnElements ::= returnElements COMMA returnElement */
  {  103,   -1 }, /* (96) returnElements ::= returnElement */
  {  104,   -1 }, /* (97) returnElement ::= arithmetic_expression */
  {  104,   -3 }, /* (98) returnElement ::= arithmetic_expression AS UQSTRING */
  {   59,   -2 }, /* (99) withClause ::= WITH withElements */
  {  105,   -1 }, /* (100) withElements ::= withElement */
  {  105,   -3 }, /* (101) withElements ::= withElements COMMA withElement */
  {  106,   -3 }, /* (102) withElement ::= arithmetic_expression AS UQSTRING */
  {   90,   -3 }, /* (103) arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
  {   90,   -3 }, /* (104) arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
  {   90,   -3 }, /* (105) arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
  {   90,   -3 }, /* (106) arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
  {   90,   -3 }, /* (107) arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
  {   90,   -4 }, /* (108) arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
  {   90,   -1 }, /* (109) arithmetic_expression ::= value */
  {   90,   -1 }, /* (110) arithmetic_expression ::= variable */
  {  107,   -3 }, /* (111) arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
  {  107,   -1 }, /* (112) arithmetic_expression_list ::= arithmetic_expression */
  {   89,   -1 }, /* (113) variable ::= UQSTRING */
  {   89,   -3 }, /* (114) variable ::= UQSTRING DOT UQSTRING */
  {   64,    0 }, /* (115) orderClause ::= */
  {   64,   -3 }, /* (116) orderClause ::= ORDER BY arithmetic_expression_list */
  {   64,   -4 }, /* (117) orderClause ::= ORDER BY arithmetic_expression_list ASC */
  {   64,   -4 }, /* (118) orderClause ::= ORDER BY arithmetic_expression_list DESC */
  {   61,    0 }, /* (119) skipClause ::= */
  {   61,   -2 }, /* (120) skipClause ::= SKIP INTEGER */
  {   62,    0 }, /* (121) limitClause ::= */
  {   62,   -2 }, /* (122) limitClause ::= LIMIT INTEGER */
  {   70,   -6 }, /* (123) unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
  {  102,   -1 }, /* (124) relation ::= EQ */
  {  102,   -1 }, /* (125) relation ::= GT */
  {  102,   -1 }, /* (126) relation ::= LT */
  {  102,   -1 }, /* (127) relation ::= LE */
  {  102,   -1 }, /* (128) relation ::= GE */
  {  102,   -1 }, /* (129) relation ::= NE */
  {  100,   -1 }, /* (130) value ::= INTEGER */
  {  100,   -2 }, /* (131) value ::= DASH INTEGER */
  {  100,   -1 }, /* (132) value ::= STRING */
  {  100,   -1 }, /* (133) value ::= FLOAT */
  {  100,   -2 }, /* (134) value ::= DASH FLOAT */
  {  100,   -1 }, /* (135) value ::= TRUE */
  {  100,   -1 }, /* (136) value ::= FALSE */
  {  100,   -1 }, /* (137) value ::= NULLVAL */
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
#line 1388 "grammar.c"
        break;
      case 1: /* expressions ::= expr */
#line 47 "grammar.y"
{
	yylhsminor.yy121 = array_new(AST*, 1);
	yylhsminor.yy121 = array_append(yylhsminor.yy121, yymsp[0].minor.yy31);
}
#line 1396 "grammar.c"
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
#line 1407 "grammar.c"
  yymsp[-2].minor.yy121 = yylhsminor.yy121;
        break;
      case 3: /* singlePartQuery ::= expr */
#line 60 "grammar.y"
{
	yylhsminor.yy31 = yymsp[0].minor.yy31;
}
#line 1415 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 4: /* singlePartQuery ::= skipClause limitClause returnClause orderClause */
#line 64 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, yymsp[-3].minor.yy147, yymsp[-2].minor.yy39, NULL, NULL, NULL);
}
#line 1423 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 5: /* singlePartQuery ::= limitClause returnClause orderClause */
#line 68 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, NULL, yymsp[-2].minor.yy39, NULL, NULL, NULL);
}
#line 1431 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 6: /* singlePartQuery ::= skipClause returnClause orderClause */
#line 72 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy120, yymsp[0].minor.yy196, yymsp[-2].minor.yy147, NULL, NULL, NULL, NULL);
}
#line 1439 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 7: /* singlePartQuery ::= returnClause orderClause skipClause limitClause */
#line 76 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1447 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 8: /* singlePartQuery ::= orderClause skipClause limitClause returnClause */
#line 80 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1455 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 9: /* singlePartQuery ::= orderClause skipClause limitClause setClause */
#line 84 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, yymsp[0].minor.yy44, NULL, NULL, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1463 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 10: /* singlePartQuery ::= orderClause skipClause limitClause deleteClause */
#line 88 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy47, NULL, yymsp[-3].minor.yy196, yymsp[-2].minor.yy147, yymsp[-1].minor.yy39, NULL, NULL, NULL);
}
#line 1471 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 11: /* expr ::= multipleMatchClause whereClause multipleCreateClause returnClause orderClause skipClause limitClause */
#line 93 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-6].minor.yy173, yymsp[-5].minor.yy3, yymsp[-4].minor.yy4, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1479 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 12: /* expr ::= multipleMatchClause whereClause multipleCreateClause */
#line 97 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1487 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 13: /* expr ::= multipleMatchClause whereClause deleteClause */
#line 101 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, NULL, NULL, NULL, yymsp[0].minor.yy47, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1495 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 14: /* expr ::= multipleMatchClause whereClause setClause */
#line 105 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-2].minor.yy173, yymsp[-1].minor.yy3, NULL, NULL, yymsp[0].minor.yy44, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1503 "grammar.c"
  yymsp[-2].minor.yy31 = yylhsminor.yy31;
        break;
      case 15: /* expr ::= multipleMatchClause whereClause setClause returnClause orderClause skipClause limitClause */
#line 109 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-6].minor.yy173, yymsp[-5].minor.yy3, NULL, NULL, yymsp[-4].minor.yy44, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, NULL);
}
#line 1511 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 16: /* expr ::= multipleCreateClause */
#line 113 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1519 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 17: /* expr ::= unwindClause multipleCreateClause */
#line 117 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, yymsp[0].minor.yy4, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-1].minor.yy97, NULL);
}
#line 1527 "grammar.c"
  yymsp[-1].minor.yy31 = yylhsminor.yy31;
        break;
      case 18: /* expr ::= indexClause */
#line 121 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy169, NULL, NULL);
}
#line 1535 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 19: /* expr ::= mergeClause */
#line 125 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, yymsp[0].minor.yy164, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1543 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 20: /* expr ::= mergeClause setClause */
#line 129 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, yymsp[-1].minor.yy164, yymsp[0].minor.yy44, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1551 "grammar.c"
  yymsp[-1].minor.yy31 = yylhsminor.yy31;
        break;
      case 21: /* expr ::= returnClause */
#line 133 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy120, NULL, NULL, NULL, NULL, NULL, NULL);
}
#line 1559 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 22: /* expr ::= unwindClause returnClause skipClause limitClause */
#line 137 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, yymsp[-2].minor.yy120, NULL, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, yymsp[-3].minor.yy97, NULL);
}
#line 1567 "grammar.c"
  yymsp[-3].minor.yy31 = yylhsminor.yy31;
        break;
      case 23: /* expr ::= procedureCallClause */
#line 143 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yymsp[0].minor.yy157);
}
#line 1575 "grammar.c"
  yymsp[0].minor.yy31 = yylhsminor.yy31;
        break;
      case 24: /* expr ::= procedureCallClause whereClause returnClause orderClause skipClause limitClause */
#line 147 "grammar.y"
{
	yylhsminor.yy31 = AST_New(NULL, yymsp[-4].minor.yy3, NULL, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, yymsp[-5].minor.yy157);
}
#line 1583 "grammar.c"
  yymsp[-5].minor.yy31 = yylhsminor.yy31;
        break;
      case 25: /* expr ::= procedureCallClause multipleMatchClause whereClause returnClause orderClause skipClause limitClause */
#line 151 "grammar.y"
{
	yylhsminor.yy31 = AST_New(yymsp[-5].minor.yy173, yymsp[-4].minor.yy3, NULL, NULL, NULL, NULL, yymsp[-3].minor.yy120, yymsp[-2].minor.yy196, yymsp[-1].minor.yy147, yymsp[0].minor.yy39, NULL, NULL, yymsp[-6].minor.yy157);
}
#line 1591 "grammar.c"
  yymsp[-6].minor.yy31 = yylhsminor.yy31;
        break;
      case 26: /* procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS YIELD unquotedStringList */
#line 156 "grammar.y"
{
	yymsp[-6].minor.yy157 = New_AST_ProcedureCallNode(yymsp[-5].minor.yy129, yymsp[-3].minor.yy5, yymsp[0].minor.yy5);
}
#line 1599 "grammar.c"
        break;
      case 27: /* procedureCallClause ::= CALL procedureName LEFT_PARENTHESIS stringList RIGHT_PARENTHESIS */
#line 160 "grammar.y"
{	
	yymsp[-4].minor.yy157 = New_AST_ProcedureCallNode(yymsp[-3].minor.yy129, yymsp[-1].minor.yy5, NULL);
}
#line 1606 "grammar.c"
        break;
      case 28: /* procedureName ::= unquotedStringList */
#line 165 "grammar.y"
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
#line 1633 "grammar.c"
  yymsp[0].minor.yy129 = yylhsminor.yy129;
        break;
      case 29: /* stringList ::= */
#line 190 "grammar.y"
{
	yymsp[1].minor.yy5 = array_new(char*, 0);
}
#line 1641 "grammar.c"
        break;
      case 30: /* stringList ::= STRING */
      case 32: /* unquotedStringList ::= UQSTRING */ yytestcase(yyruleno==32);
#line 194 "grammar.y"
{
	yylhsminor.yy5 = array_new(char*, 1);
	yylhsminor.yy5 = array_append(yylhsminor.yy5, yymsp[0].minor.yy0.strval);
}
#line 1650 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 31: /* stringList ::= stringList delimiter STRING */
      case 33: /* unquotedStringList ::= unquotedStringList delimiter UQSTRING */ yytestcase(yyruleno==33);
#line 200 "grammar.y"
{
	yymsp[-2].minor.yy5 = array_append(yymsp[-2].minor.yy5, yymsp[0].minor.yy0.strval);
	yylhsminor.yy5 = yymsp[-2].minor.yy5;
}
#line 1660 "grammar.c"
  yymsp[-2].minor.yy5 = yylhsminor.yy5;
        break;
      case 34: /* delimiter ::= COMMA */
#line 218 "grammar.y"
{ yymsp[0].minor.yy6 = COMMA; }
#line 1666 "grammar.c"
        break;
      case 35: /* delimiter ::= DOT */
#line 219 "grammar.y"
{ yymsp[0].minor.yy6 = DOT; }
#line 1671 "grammar.c"
        break;
      case 36: /* multipleMatchClause ::= matchClauses */
#line 222 "grammar.y"
{
	yylhsminor.yy173 = New_AST_MatchNode(yymsp[0].minor.yy210);
}
#line 1678 "grammar.c"
  yymsp[0].minor.yy173 = yylhsminor.yy173;
        break;
      case 37: /* matchClauses ::= matchClause */
      case 42: /* createClauses ::= createClause */ yytestcase(yyruleno==42);
#line 228 "grammar.y"
{
	yylhsminor.yy210 = yymsp[0].minor.yy210;
}
#line 1687 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 38: /* matchClauses ::= matchClauses matchClause */
      case 43: /* createClauses ::= createClauses createClause */ yytestcase(yyruleno==43);
#line 232 "grammar.y"
{
	Vector *v;
	while(Vector_Pop(yymsp[0].minor.yy210, &v)) Vector_Push(yymsp[-1].minor.yy210, v);
	Vector_Free(yymsp[0].minor.yy210);
	yylhsminor.yy210 = yymsp[-1].minor.yy210;
}
#line 1699 "grammar.c"
  yymsp[-1].minor.yy210 = yylhsminor.yy210;
        break;
      case 39: /* matchClause ::= MATCH chains */
      case 44: /* createClause ::= CREATE chains */ yytestcase(yyruleno==44);
#line 241 "grammar.y"
{
	yymsp[-1].minor.yy210 = yymsp[0].minor.yy210;
}
#line 1708 "grammar.c"
        break;
      case 40: /* multipleCreateClause ::= */
#line 246 "grammar.y"
{
	yymsp[1].minor.yy4 = NULL;
}
#line 1715 "grammar.c"
        break;
      case 41: /* multipleCreateClause ::= createClauses */
#line 250 "grammar.y"
{
	yylhsminor.yy4 = New_AST_CreateNode(yymsp[0].minor.yy210);
}
#line 1722 "grammar.c"
  yymsp[0].minor.yy4 = yylhsminor.yy4;
        break;
      case 45: /* indexClause ::= indexOpToken INDEX ON indexLabel indexProp */
#line 276 "grammar.y"
{
  yylhsminor.yy169 = New_AST_IndexNode(yymsp[-1].minor.yy0.strval, yymsp[0].minor.yy0.strval, yymsp[-4].minor.yy177);
}
#line 1730 "grammar.c"
  yymsp[-4].minor.yy169 = yylhsminor.yy169;
        break;
      case 46: /* indexOpToken ::= CREATE */
#line 282 "grammar.y"
{ yymsp[0].minor.yy177 = CREATE_INDEX; }
#line 1736 "grammar.c"
        break;
      case 47: /* indexOpToken ::= DROP */
#line 283 "grammar.y"
{ yymsp[0].minor.yy177 = DROP_INDEX; }
#line 1741 "grammar.c"
        break;
      case 48: /* indexLabel ::= COLON UQSTRING */
#line 285 "grammar.y"
{
  yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
}
#line 1748 "grammar.c"
        break;
      case 49: /* indexProp ::= LEFT_PARENTHESIS UQSTRING RIGHT_PARENTHESIS */
#line 289 "grammar.y"
{
  yymsp[-2].minor.yy0 = yymsp[-1].minor.yy0;
}
#line 1755 "grammar.c"
        break;
      case 50: /* mergeClause ::= MERGE chain */
#line 295 "grammar.y"
{
	yymsp[-1].minor.yy164 = New_AST_MergeNode(yymsp[0].minor.yy210);
}
#line 1762 "grammar.c"
        break;
      case 51: /* setClause ::= SET setList */
#line 300 "grammar.y"
{
	yymsp[-1].minor.yy44 = New_AST_SetNode(yymsp[0].minor.yy210);
}
#line 1769 "grammar.c"
        break;
      case 52: /* setList ::= setElement */
#line 305 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_SetElement*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy170);
}
#line 1777 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 53: /* setList ::= setList COMMA setElement */
#line 309 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy170);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1786 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 54: /* setElement ::= variable EQ arithmetic_expression */
#line 315 "grammar.y"
{
	yylhsminor.yy170 = New_AST_SetElement(yymsp[-2].minor.yy156, yymsp[0].minor.yy154);
}
#line 1794 "grammar.c"
  yymsp[-2].minor.yy170 = yylhsminor.yy170;
        break;
      case 55: /* chain ::= node */
#line 321 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_GraphEntity*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy117);
}
#line 1803 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 56: /* chain ::= chain link node */
#line 326 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[-1].minor.yy69);
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy117);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1813 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 57: /* chains ::= chain */
#line 334 "grammar.y"
{
	yylhsminor.yy210 = NewVector(Vector*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy210);
}
#line 1822 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 58: /* chains ::= chains COMMA chain */
#line 339 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy210);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1831 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 59: /* deleteClause ::= DELETE deleteExpression */
#line 347 "grammar.y"
{
	yymsp[-1].minor.yy47 = New_AST_DeleteNode(yymsp[0].minor.yy210);
}
#line 1839 "grammar.c"
        break;
      case 60: /* deleteExpression ::= UQSTRING */
#line 353 "grammar.y"
{
	yylhsminor.yy210 = NewVector(char*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy0.strval);
}
#line 1847 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 61: /* deleteExpression ::= deleteExpression COMMA UQSTRING */
#line 358 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy0.strval);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 1856 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 62: /* node ::= LEFT_PARENTHESIS UQSTRING COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 366 "grammar.y"
{
	yymsp[-5].minor.yy117 = New_AST_NodeEntity(yymsp[-4].minor.yy0.strval, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 1864 "grammar.c"
        break;
      case 63: /* node ::= LEFT_PARENTHESIS COLON UQSTRING properties RIGHT_PARENTHESIS */
#line 371 "grammar.y"
{
	yymsp[-4].minor.yy117 = New_AST_NodeEntity(NULL, yymsp[-2].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 1871 "grammar.c"
        break;
      case 64: /* node ::= LEFT_PARENTHESIS UQSTRING properties RIGHT_PARENTHESIS */
#line 376 "grammar.y"
{
	yymsp[-3].minor.yy117 = New_AST_NodeEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy210);
}
#line 1878 "grammar.c"
        break;
      case 65: /* node ::= LEFT_PARENTHESIS properties RIGHT_PARENTHESIS */
#line 381 "grammar.y"
{
	yymsp[-2].minor.yy117 = New_AST_NodeEntity(NULL, NULL, yymsp[-1].minor.yy210);
}
#line 1885 "grammar.c"
        break;
      case 66: /* link ::= DASH edge RIGHT_ARROW */
#line 388 "grammar.y"
{
	yymsp[-2].minor.yy69 = yymsp[-1].minor.yy69;
	yymsp[-2].minor.yy69->direction = N_LEFT_TO_RIGHT;
}
#line 1893 "grammar.c"
        break;
      case 67: /* link ::= LEFT_ARROW edge DASH */
#line 394 "grammar.y"
{
	yymsp[-2].minor.yy69 = yymsp[-1].minor.yy69;
	yymsp[-2].minor.yy69->direction = N_RIGHT_TO_LEFT;
}
#line 1901 "grammar.c"
        break;
      case 68: /* edge ::= LEFT_BRACKET properties edgeLength RIGHT_BRACKET */
#line 401 "grammar.y"
{ 
	yymsp[-3].minor.yy69 = New_AST_LinkEntity(NULL, NULL, yymsp[-2].minor.yy210, N_DIR_UNKNOWN, yymsp[-1].minor.yy138);
}
#line 1908 "grammar.c"
        break;
      case 69: /* edge ::= LEFT_BRACKET UQSTRING properties RIGHT_BRACKET */
#line 406 "grammar.y"
{ 
	yymsp[-3].minor.yy69 = New_AST_LinkEntity(yymsp[-2].minor.yy0.strval, NULL, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, NULL);
}
#line 1915 "grammar.c"
        break;
      case 70: /* edge ::= LEFT_BRACKET edgeLabels edgeLength properties RIGHT_BRACKET */
#line 411 "grammar.y"
{ 
	yymsp[-4].minor.yy69 = New_AST_LinkEntity(NULL, yymsp[-3].minor.yy5, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, yymsp[-2].minor.yy138);
}
#line 1922 "grammar.c"
        break;
      case 71: /* edge ::= LEFT_BRACKET UQSTRING edgeLabels properties RIGHT_BRACKET */
#line 416 "grammar.y"
{ 
	yymsp[-4].minor.yy69 = New_AST_LinkEntity(yymsp[-3].minor.yy0.strval, yymsp[-2].minor.yy5, yymsp[-1].minor.yy210, N_DIR_UNKNOWN, NULL);
}
#line 1929 "grammar.c"
        break;
      case 72: /* edgeLabel ::= COLON UQSTRING */
#line 423 "grammar.y"
{
	yymsp[-1].minor.yy129 = yymsp[0].minor.yy0.strval;
}
#line 1936 "grammar.c"
        break;
      case 73: /* edgeLabels ::= edgeLabel */
#line 428 "grammar.y"
{
	yylhsminor.yy5 = array_new(char*, 1);
	yylhsminor.yy5 = array_append(yylhsminor.yy5, yymsp[0].minor.yy129);
}
#line 1944 "grammar.c"
  yymsp[0].minor.yy5 = yylhsminor.yy5;
        break;
      case 74: /* edgeLabels ::= edgeLabels PIPE edgeLabel */
#line 434 "grammar.y"
{
	char *label = yymsp[0].minor.yy129;
	yymsp[-2].minor.yy5 = array_append(yymsp[-2].minor.yy5, label);
	yylhsminor.yy5 = yymsp[-2].minor.yy5;
}
#line 1954 "grammar.c"
  yymsp[-2].minor.yy5 = yylhsminor.yy5;
        break;
      case 75: /* edgeLength ::= */
#line 443 "grammar.y"
{
	yymsp[1].minor.yy138 = NULL;
}
#line 1962 "grammar.c"
        break;
      case 76: /* edgeLength ::= MUL INTEGER DOTDOT INTEGER */
#line 448 "grammar.y"
{
	yymsp[-3].minor.yy138 = New_AST_LinkLength(yymsp[-2].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1969 "grammar.c"
        break;
      case 77: /* edgeLength ::= MUL INTEGER DOTDOT */
#line 453 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(yymsp[-1].minor.yy0.longval, UINT_MAX-2);
}
#line 1976 "grammar.c"
        break;
      case 78: /* edgeLength ::= MUL DOTDOT INTEGER */
#line 458 "grammar.y"
{
	yymsp[-2].minor.yy138 = New_AST_LinkLength(1, yymsp[0].minor.yy0.longval);
}
#line 1983 "grammar.c"
        break;
      case 79: /* edgeLength ::= MUL INTEGER */
#line 463 "grammar.y"
{
	yymsp[-1].minor.yy138 = New_AST_LinkLength(yymsp[0].minor.yy0.longval, yymsp[0].minor.yy0.longval);
}
#line 1990 "grammar.c"
        break;
      case 80: /* edgeLength ::= MUL */
#line 468 "grammar.y"
{
	yymsp[0].minor.yy138 = New_AST_LinkLength(1, UINT_MAX-2);
}
#line 1997 "grammar.c"
        break;
      case 81: /* properties ::= */
#line 474 "grammar.y"
{
	yymsp[1].minor.yy210 = NULL;
}
#line 2004 "grammar.c"
        break;
      case 82: /* properties ::= LEFT_CURLY_BRACKET mapLiteral RIGHT_CURLY_BRACKET */
#line 478 "grammar.y"
{
	yymsp[-2].minor.yy210 = yymsp[-1].minor.yy210;
}
#line 2011 "grammar.c"
        break;
      case 83: /* mapLiteral ::= UQSTRING COLON value */
#line 484 "grammar.y"
{
	yylhsminor.yy210 = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-2].minor.yy0.strval);
	Vector_Push(yylhsminor.yy210, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[0].minor.yy150;
	Vector_Push(yylhsminor.yy210, val);
}
#line 2026 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 84: /* mapLiteral ::= UQSTRING COLON value COMMA mapLiteral */
#line 496 "grammar.y"
{
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(yymsp[-4].minor.yy0.strval);
	Vector_Push(yymsp[0].minor.yy210, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = yymsp[-2].minor.yy150;
	Vector_Push(yymsp[0].minor.yy210, val);
	
	yylhsminor.yy210 = yymsp[0].minor.yy210;
}
#line 2042 "grammar.c"
  yymsp[-4].minor.yy210 = yylhsminor.yy210;
        break;
      case 85: /* whereClause ::= */
#line 510 "grammar.y"
{ 
	yymsp[1].minor.yy3 = NULL;
}
#line 2050 "grammar.c"
        break;
      case 86: /* whereClause ::= WHERE cond */
#line 513 "grammar.y"
{
	yymsp[-1].minor.yy3 = New_AST_WhereNode(yymsp[0].minor.yy10);
}
#line 2057 "grammar.c"
        break;
      case 87: /* cond ::= arithmetic_expression relation arithmetic_expression */
#line 522 "grammar.y"
{ yylhsminor.yy10 = New_AST_PredicateNode(yymsp[-2].minor.yy154, yymsp[-1].minor.yy6, yymsp[0].minor.yy154); }
#line 2062 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 88: /* cond ::= LEFT_PARENTHESIS cond RIGHT_PARENTHESIS */
#line 524 "grammar.y"
{ yymsp[-2].minor.yy10 = yymsp[-1].minor.yy10; }
#line 2068 "grammar.c"
        break;
      case 89: /* cond ::= cond AND cond */
#line 525 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, AND, yymsp[0].minor.yy10); }
#line 2073 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 90: /* cond ::= cond OR cond */
#line 526 "grammar.y"
{ yylhsminor.yy10 = New_AST_ConditionNode(yymsp[-2].minor.yy10, OR, yymsp[0].minor.yy10); }
#line 2079 "grammar.c"
  yymsp[-2].minor.yy10 = yylhsminor.yy10;
        break;
      case 91: /* returnClause ::= RETURN returnElements */
#line 530 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy122, 0);
}
#line 2087 "grammar.c"
        break;
      case 92: /* returnClause ::= RETURN DISTINCT returnElements */
#line 533 "grammar.y"
{
	yymsp[-2].minor.yy120 = New_AST_ReturnNode(yymsp[0].minor.yy122, 1);
}
#line 2094 "grammar.c"
        break;
      case 93: /* returnClause ::= RETURN MUL */
#line 537 "grammar.y"
{
	yymsp[-1].minor.yy120 = New_AST_ReturnNode(NULL, 0);
}
#line 2101 "grammar.c"
        break;
      case 94: /* returnClause ::= RETURN DISTINCT MUL */
#line 540 "grammar.y"
{
	yymsp[-2].minor.yy120 = New_AST_ReturnNode(NULL, 1);
}
#line 2108 "grammar.c"
        break;
      case 95: /* returnElements ::= returnElements COMMA returnElement */
#line 546 "grammar.y"
{
	yylhsminor.yy122 = array_append(yymsp[-2].minor.yy122, yymsp[0].minor.yy139);
}
#line 2115 "grammar.c"
  yymsp[-2].minor.yy122 = yylhsminor.yy122;
        break;
      case 96: /* returnElements ::= returnElement */
#line 550 "grammar.y"
{
	yylhsminor.yy122 = array_new(AST_ReturnElementNode*, 1);
	array_append(yylhsminor.yy122, yymsp[0].minor.yy139);
}
#line 2124 "grammar.c"
  yymsp[0].minor.yy122 = yylhsminor.yy122;
        break;
      case 97: /* returnElement ::= arithmetic_expression */
#line 557 "grammar.y"
{
	yylhsminor.yy139 = New_AST_ReturnElementNode(yymsp[0].minor.yy154, NULL);
}
#line 2132 "grammar.c"
  yymsp[0].minor.yy139 = yylhsminor.yy139;
        break;
      case 98: /* returnElement ::= arithmetic_expression AS UQSTRING */
#line 561 "grammar.y"
{
	yylhsminor.yy139 = New_AST_ReturnElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 2140 "grammar.c"
  yymsp[-2].minor.yy139 = yylhsminor.yy139;
        break;
      case 99: /* withClause ::= WITH withElements */
#line 566 "grammar.y"
{
	yymsp[-1].minor.yy168 = New_AST_WithNode(yymsp[0].minor.yy112);
}
#line 2148 "grammar.c"
        break;
      case 100: /* withElements ::= withElement */
#line 571 "grammar.y"
{
	yylhsminor.yy112 = array_new(AST_WithElementNode*, 1);
	array_append(yylhsminor.yy112, yymsp[0].minor.yy98);
}
#line 2156 "grammar.c"
  yymsp[0].minor.yy112 = yylhsminor.yy112;
        break;
      case 101: /* withElements ::= withElements COMMA withElement */
#line 575 "grammar.y"
{
	yylhsminor.yy112 = array_append(yymsp[-2].minor.yy112, yymsp[0].minor.yy98);
}
#line 2164 "grammar.c"
  yymsp[-2].minor.yy112 = yylhsminor.yy112;
        break;
      case 102: /* withElement ::= arithmetic_expression AS UQSTRING */
#line 580 "grammar.y"
{
	yylhsminor.yy98 = New_AST_WithElementNode(yymsp[-2].minor.yy154, yymsp[0].minor.yy0.strval);
}
#line 2172 "grammar.c"
  yymsp[-2].minor.yy98 = yylhsminor.yy98;
        break;
      case 103: /* arithmetic_expression ::= LEFT_PARENTHESIS arithmetic_expression RIGHT_PARENTHESIS */
#line 587 "grammar.y"
{
	yymsp[-2].minor.yy154 = yymsp[-1].minor.yy154;
}
#line 2180 "grammar.c"
        break;
      case 104: /* arithmetic_expression ::= arithmetic_expression ADD arithmetic_expression */
#line 593 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("ADD", args);
}
#line 2190 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 105: /* arithmetic_expression ::= arithmetic_expression DASH arithmetic_expression */
#line 600 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("SUB", args);
}
#line 2201 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 106: /* arithmetic_expression ::= arithmetic_expression MUL arithmetic_expression */
#line 607 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("MUL", args);
}
#line 2212 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 107: /* arithmetic_expression ::= arithmetic_expression DIV arithmetic_expression */
#line 614 "grammar.y"
{
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, yymsp[-2].minor.yy154);
	Vector_Push(args, yymsp[0].minor.yy154);
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode("DIV", args);
}
#line 2223 "grammar.c"
  yymsp[-2].minor.yy154 = yylhsminor.yy154;
        break;
      case 108: /* arithmetic_expression ::= UQSTRING LEFT_PARENTHESIS arithmetic_expression_list RIGHT_PARENTHESIS */
#line 622 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_OpNode(yymsp[-3].minor.yy0.strval, yymsp[-1].minor.yy210);
}
#line 2231 "grammar.c"
  yymsp[-3].minor.yy154 = yylhsminor.yy154;
        break;
      case 109: /* arithmetic_expression ::= value */
#line 627 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_ConstOperandNode(yymsp[0].minor.yy150);
}
#line 2239 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 110: /* arithmetic_expression ::= variable */
#line 632 "grammar.y"
{
	yylhsminor.yy154 = New_AST_AR_EXP_VariableOperandNode(yymsp[0].minor.yy156->alias, yymsp[0].minor.yy156->property);
	free(yymsp[0].minor.yy156);
}
#line 2248 "grammar.c"
  yymsp[0].minor.yy154 = yylhsminor.yy154;
        break;
      case 111: /* arithmetic_expression_list ::= arithmetic_expression_list COMMA arithmetic_expression */
#line 639 "grammar.y"
{
	Vector_Push(yymsp[-2].minor.yy210, yymsp[0].minor.yy154);
	yylhsminor.yy210 = yymsp[-2].minor.yy210;
}
#line 2257 "grammar.c"
  yymsp[-2].minor.yy210 = yylhsminor.yy210;
        break;
      case 112: /* arithmetic_expression_list ::= arithmetic_expression */
#line 643 "grammar.y"
{
	yylhsminor.yy210 = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(yylhsminor.yy210, yymsp[0].minor.yy154);
}
#line 2266 "grammar.c"
  yymsp[0].minor.yy210 = yylhsminor.yy210;
        break;
      case 113: /* variable ::= UQSTRING */
#line 650 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[0].minor.yy0.strval, NULL);
}
#line 2274 "grammar.c"
  yymsp[0].minor.yy156 = yylhsminor.yy156;
        break;
      case 114: /* variable ::= UQSTRING DOT UQSTRING */
#line 654 "grammar.y"
{
	yylhsminor.yy156 = New_AST_Variable(yymsp[-2].minor.yy0.strval, yymsp[0].minor.yy0.strval);
}
#line 2282 "grammar.c"
  yymsp[-2].minor.yy156 = yylhsminor.yy156;
        break;
      case 115: /* orderClause ::= */
#line 660 "grammar.y"
{
	yymsp[1].minor.yy196 = NULL;
}
#line 2290 "grammar.c"
        break;
      case 116: /* orderClause ::= ORDER BY arithmetic_expression_list */
#line 663 "grammar.y"
{
	yymsp[-2].minor.yy196 = New_AST_OrderNode(yymsp[0].minor.yy210, ORDER_DIR_ASC);
}
#line 2297 "grammar.c"
        break;
      case 117: /* orderClause ::= ORDER BY arithmetic_expression_list ASC */
#line 666 "grammar.y"
{
	yymsp[-3].minor.yy196 = New_AST_OrderNode(yymsp[-1].minor.yy210, ORDER_DIR_ASC);
}
#line 2304 "grammar.c"
        break;
      case 118: /* orderClause ::= ORDER BY arithmetic_expression_list DESC */
#line 669 "grammar.y"
{
	yymsp[-3].minor.yy196 = New_AST_OrderNode(yymsp[-1].minor.yy210, ORDER_DIR_DESC);
}
#line 2311 "grammar.c"
        break;
      case 119: /* skipClause ::= */
#line 675 "grammar.y"
{
	yymsp[1].minor.yy147 = NULL;
}
#line 2318 "grammar.c"
        break;
      case 120: /* skipClause ::= SKIP INTEGER */
#line 678 "grammar.y"
{
	yymsp[-1].minor.yy147 = New_AST_SkipNode(yymsp[0].minor.yy0.longval);
}
#line 2325 "grammar.c"
        break;
      case 121: /* limitClause ::= */
#line 684 "grammar.y"
{
	yymsp[1].minor.yy39 = NULL;
}
#line 2332 "grammar.c"
        break;
      case 122: /* limitClause ::= LIMIT INTEGER */
#line 687 "grammar.y"
{
	yymsp[-1].minor.yy39 = New_AST_LimitNode(yymsp[0].minor.yy0.longval);
}
#line 2339 "grammar.c"
        break;
      case 123: /* unwindClause ::= UNWIND LEFT_BRACKET arithmetic_expression_list RIGHT_BRACKET AS UQSTRING */
#line 693 "grammar.y"
{
	yymsp[-5].minor.yy97 = New_AST_UnwindNode(yymsp[-3].minor.yy210, yymsp[0].minor.yy0.strval);
}
#line 2346 "grammar.c"
        break;
      case 124: /* relation ::= EQ */
#line 698 "grammar.y"
{ yymsp[0].minor.yy6 = EQ; }
#line 2351 "grammar.c"
        break;
      case 125: /* relation ::= GT */
#line 699 "grammar.y"
{ yymsp[0].minor.yy6 = GT; }
#line 2356 "grammar.c"
        break;
      case 126: /* relation ::= LT */
#line 700 "grammar.y"
{ yymsp[0].minor.yy6 = LT; }
#line 2361 "grammar.c"
        break;
      case 127: /* relation ::= LE */
#line 701 "grammar.y"
{ yymsp[0].minor.yy6 = LE; }
#line 2366 "grammar.c"
        break;
      case 128: /* relation ::= GE */
#line 702 "grammar.y"
{ yymsp[0].minor.yy6 = GE; }
#line 2371 "grammar.c"
        break;
      case 129: /* relation ::= NE */
#line 703 "grammar.y"
{ yymsp[0].minor.yy6 = NE; }
#line 2376 "grammar.c"
        break;
      case 130: /* value ::= INTEGER */
#line 708 "grammar.y"
{  yylhsminor.yy150 = SI_LongVal(yymsp[0].minor.yy0.longval); }
#line 2381 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 131: /* value ::= DASH INTEGER */
#line 709 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_LongVal(-yymsp[0].minor.yy0.longval); }
#line 2387 "grammar.c"
        break;
      case 132: /* value ::= STRING */
#line 710 "grammar.y"
{  yylhsminor.yy150 = SI_ConstStringVal(yymsp[0].minor.yy0.strval); }
#line 2392 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 133: /* value ::= FLOAT */
#line 711 "grammar.y"
{  yylhsminor.yy150 = SI_DoubleVal(yymsp[0].minor.yy0.dval); }
#line 2398 "grammar.c"
  yymsp[0].minor.yy150 = yylhsminor.yy150;
        break;
      case 134: /* value ::= DASH FLOAT */
#line 712 "grammar.y"
{  yymsp[-1].minor.yy150 = SI_DoubleVal(-yymsp[0].minor.yy0.dval); }
#line 2404 "grammar.c"
        break;
      case 135: /* value ::= TRUE */
#line 713 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(1); }
#line 2409 "grammar.c"
        break;
      case 136: /* value ::= FALSE */
#line 714 "grammar.y"
{ yymsp[0].minor.yy150 = SI_BoolVal(0); }
#line 2414 "grammar.c"
        break;
      case 137: /* value ::= NULLVAL */
#line 715 "grammar.y"
{ yymsp[0].minor.yy150 = SI_NullVal(); }
#line 2419 "grammar.c"
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
#line 2484 "grammar.c"
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
#line 717 "grammar.y"


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
#line 2732 "grammar.c"
