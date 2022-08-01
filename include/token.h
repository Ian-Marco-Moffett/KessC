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

  // Keywords.
  TT_OUT,             // Like print.
} TOKEN_TYPE;


struct Token {
  TOKEN_TYPE type;
  uint64_t intval;
  char ch;
};


#endif
