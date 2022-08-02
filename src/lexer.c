#include <lexer.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define IDENT_MAX_LEN 255


extern FILE* input;
static uint64_t line_num = 1;
char idbuf[IDENT_MAX_LEN];

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


static uint16_t scanid(char c, char* buf) {
  uint16_t i = 0;
  while (isalpha(c) || isdigit(c) || c == '_') {
    if (i >= IDENT_MAX_LEN - 1) {
      printf(ERR "Identifier too long near line %d (why the in holy moly heck do you need that many characters???? O_O)\n", line_num);
      fclose(input);
      exit(1);
    }

    buf[i++] = c;
    c = next();
  }

  putback();
  buf[i] = '\0';
  return i;
}


/*
 *  @param what   Possible keyword.
 *
 */
static TOKEN_TYPE keyword(const char* what) {
  if (strcmp(what, "out") == 0) {
    return TT_OUT;
  } else if (strcmp(what, "u8") == 0) {
    return TT_U8;
  }
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
    case '(':
      tok->type = TT_LPAREN;
      tok->ch = '(';
      break;
    case ')':
      tok->type = TT_RPAREN;
      tok->ch = ')';
      break;
    case ';':
      tok->type = TT_SEMI;
      tok->ch = ';';
      break;
    case '=':
      if ((cur = next()) == '=') {
        tok->type = TT_EQ;
      } else {
        putback();
        tok->type = TT_EQUALS;
        tok->ch = '=';
      }
      break;
    case '>':
      if ((cur = next()) == '=') {
        tok->type = TT_GE;
      } else {
        putback();
        tok->type = TT_GT;
      }
      break;
    case '<':
      if ((cur = next()) == '=') {
        tok->type = TT_LE;
      } else {
        putback();
        tok->type = TT_LT;
      }
      break;
    case '!':
      if ((cur = next()) == '=') {
        tok->type = TT_NE;
      } else {
        // TODO: Remove error, and treat this as logical NOT.
        printf(ERR "Invalid token on line %d\n", line_num);
        fclose(input);
        exit(1);
      }
    case EOF:
      tok->type = TT_EOF;
      tok->ch = '\0';
      return 0;
    default:
      if (isdigit(cur)) {
        tok->intval = scanint(cur);
        tok->type = TT_INTLIT;
        return 1;
      }

      if (isalpha(cur) || cur == '_') {
        scanid(cur, idbuf);

        switch (keyword(idbuf)) {
          case TT_OUT:
            tok->type = TT_OUT;
            return 1;
          case TT_U8:
            tok->type = TT_U8;
            return 1;
          default:
            // If it is not a keyword, 
            // it must be an ID.
            tok->type = TT_ID;
            return 1;
        }
      }

      printf(ERR "Invalid token on line %d ('%c')\n", line_num, cur);
      fclose(input);
      exit(1);
    }

  return 1;
}


uint64_t get_line_num(void) {
  return line_num;
}
