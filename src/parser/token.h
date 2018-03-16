#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <stdlib.h>

typedef struct {
  int64_t intval;
  double dval;
  char *strval;
  char *s;  // token string
  int pos;  // position in the query
} Token;

extern Token tok;

#endif