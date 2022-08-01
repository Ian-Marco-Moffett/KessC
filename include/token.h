#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

typedef enum {
  TT_PLUS,
  TT_MINUS,
  TT_STAR,
  TT_SLASH,
  TT_INTLIT
} TOKEN_TYPE;


struct Token {
  TOKEN_TYPE type;
  uint64_t intval;
};


#endif
