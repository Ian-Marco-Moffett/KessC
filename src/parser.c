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
    default:
      printf(ERR "Syntax error near line %d\n", get_line_num());
      panic();
  }
}


static void tok_assert(TOKEN_TYPE tt, char* what) {
  if (cur_token.type == tt) {
    scan(&cur_token);
  } else {
    printf(ERR "'%s' expected near line %d\n", what, get_line_num() - 1);
    panic();
  }
}


static void semi(void) {
  tok_assert(TT_SEMI, ";");
}


struct ASTNode* binexpr(void) {
  AST_OP ntype;

  struct ASTNode* left = primary();

  if (cur_token.type == TT_SEMI || cur_token.type == TT_RPAREN) {
    return left;
  }
  
  ntype = arithop(cur_token.type);
  scan(&cur_token);
  
  // Get right hand side.
  struct ASTNode* right = binexpr();
  struct ASTNode* n = mkastnode(ntype, left, right, 0);
  return n;
}

static void statement(void) {
  while (1) {
    tok_assert(TT_OUT, "out");
    tok_assert(TT_LPAREN, "(");

    struct ASTNode* tree = binexpr();
    REG reg = mkAST(tree);

    rprint(reg);
    freeall_regs();

    tok_assert(TT_RPAREN, ")");
    semi();

    if (cur_token.type == TT_EOF) {
      return;
    }
  }
}


void parse(void) {
  scan(&cur_token);
  compile_init();
  statement();
  compile_end();
}
