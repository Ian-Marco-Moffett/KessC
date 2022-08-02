#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

typedef enum {
  TT_EOF,
  TT_PLUS,
  TT_MINUS,
  TT_STAR,
  TT_SLASH,
  TT_INTLIT,
  TT_SEMI,
  TT_LPAREN,
  TT_RPAREN,
  TT_EQUALS,
  TT_U8,
  TT_ID,

  // Comparison operators.
  TT_LT,
  TT_GT,
  TT_LE,
  TT_GE,
  TT_EQ,
  TT_NE,

  // Keywords.
  TT_OUT,             // Like print.
} TOKEN_TYPE;


struct Token {
  TOKEN_TYPE type;
  uint64_t intval;
  char ch;
};


#endif
