#include <lexer.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

extern FILE* input;
static uint64_t line_num = 1;

// Return the position of character c
// in string s, or -1 if c not found
static int chrpos(char* s, int c) {
  char *p;

  p = strchr(s, c);
  return (p ? p - s : -1);
}

static char next(void) {
  char c = fgetc(input);

  if (c == '\n') {
    ++line_num;
  }

  return c;
}

static void putback(void) {
  fseek(input, ftell(input) - 1, SEEK_SET);
}

static int scanint(int c) {
  int k, val = 0;

  while ((k = chrpos("0123456789", c)) >= 0) {
    val = val * 10 + k;
    c = next();
  }

  // We have hit a non-integer character, put it back.
  putback();
  return val;
}


static char skip(void) {
  char c = next();

  while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
    c = next();
  }

  return c;
}


uint8_t scan(struct Token* tok) {
  char cur = skip();

  switch (cur) {
    case EOF:
      return 0;
    case '+':
      tok->type = TT_PLUS;
      break;
    case '-':
      tok->type = TT_MINUS;
      break;
    case '*':
      tok->type = TT_STAR;
      break;
    case '/':
      tok->type = TT_SLASH;
      break;
    default:
      if (isdigit(cur)) {
        tok->intval = TT_INTLIT;
        break;
      }

      printf(ERR "Invalid character on line %d\n", line_num);
      fclose(input);
      exit(1);
    }

  return 1;
}
