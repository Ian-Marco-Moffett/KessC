#include <lexer.h>
#include <ast.h>
#include <err.h>
#include <compile.h>
#include <stdio.h>

static struct Token cur_token;

static struct ASTNode* primary(void) {
  struct ASTNode* n; 

  switch (cur_token.type) {
    case TT_INTLIT:
      n = mkastleaf(AST_INTLIT, cur_token.intval);
      scan(&cur_token);
      return n;
    case TT_EOF:
      return NULL;
    default:
      printf(ERR "Syntax error on line %d near '%c'\n", get_line_num(), cur_token.ch);
      panic();
  }
}

struct ASTNode* binexpr(void) {
  AST_OP ntype;

  struct ASTNode* left = primary();

  if (cur_token.type == TT_EOF) {
    return left;
  }
  
  ntype = arithop(cur_token.type);
  scan(&cur_token);
  
  // Get right hand side.
  struct ASTNode* right = binexpr();
  struct ASTNode* n = mkastnode(ntype, left, right, 0);
  return n;
}


void parse(void) {
  scan(&cur_token);
  struct ASTNode* n = binexpr();
  compile();
}
