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


static void putback(void) {
  fseek(input, ftell(input) - 1, SEEK_SET);
}

static char next(void) {
  char c = fgetc(input);

  if (c == '\n') {
    ++line_num;
  }

  return c;
}

static int scanint(char c) {
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
    case '+':
      tok->type = TT_PLUS;
      tok->ch = '+';
      break;
    case '-':
      tok->type = TT_MINUS;
      tok->ch = '-';
      break;
    case '*':
      tok->type = TT_STAR;
      tok->ch = '*';
      break;
    case '/':
      tok->type = TT_SLASH;
      tok->ch = '/';
      break;
    case EOF:
      tok->type = TT_EOF;
      tok->ch = '\0';
      return 0;
    default:
      if (isdigit(cur)) {
        tok->intval = scanint(cur);
        tok->type = TT_INTLIT;
        break;
      }

      printf(ERR "Invalid character on line %d ('%c')\n", line_num, cur);
      fclose(input);
      exit(1);
    }

  return 1;
}


uint64_t get_line_num(void) {
  return line_num;
}
