#ifndef LEXER_H
#define LEXER_H

#include <token.h>

// Returns zero if no more tokens.
uint8_t scan(struct Token* tok);
uint64_t get_line_num(void);

#endif
