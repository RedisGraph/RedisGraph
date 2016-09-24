#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <stdlib.h>

typedef union {
  int64_t intval;
  double dval;
  char *strval;
} Token;

extern Token tok;

#endif