#include <lexer.h>

static struct Token last_tok;


void parse(void) {
  scan(&last_tok);
}
