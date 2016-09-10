#ifndef YYSTYPE
typedef union {
	char* str;
	int n;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

/* extern YYSTYPE yylval; */
YYSTYPE yylval;