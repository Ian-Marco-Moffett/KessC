#include <lexer.h>
#include <ast.h>
#include <err.h>
#include <compile.h>
#include <symbol.h>
#include <stdio.h>

static struct Token cur_token;
extern char idbuf[];

static struct ASTNode* primary(void) {
  struct ASTNode* n; 
  uint64_t id;

  switch (cur_token.type) {
    case TT_INTLIT:
      n = mkastleaf(AST_INTLIT, cur_token.intval);
      break;
    case TT_ID:
      id = locateglob(idbuf);

      if (id == -1) {
        printf(ERR "\"%s\" undeclared.\n");
        panic();
      }

      n = mkastleaf(AST_ID, id);
      break;
    default:
      printf(ERR "Syntax error near line %d\n", get_line_num());
      panic();
  }
  
  scan(&cur_token);
  return n;
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


static void out_statement(void) {
  tok_assert(TT_OUT, "out");
  tok_assert(TT_LPAREN, "(");

  struct ASTNode* tree = binexpr();
  REG reg = mkAST(tree, -1);

  rprint(reg);
  freeall_regs();

  tok_assert(TT_RPAREN, ")");
  semi();
}


static void id(void) {
  tok_assert(TT_ID, "identifier");
}

/*
 *  init should be 1 if variable is made like so:
 *
 *  u8 var = 2;
 *
 *  but if it is like so:
 *
 *  u8 var;
 *  var = 2;
 *
 *  Then init should be 0.
 *
 *
 */

static void assignment(uint8_t init) {
  // Ensure the existance of an identifier if not init.

  if (!(init)) {
    id();
  }

  uint64_t id = locateglob(idbuf);

  if (id == -1) {
    printf(ERR "Referance to an undeclared variable \"%s\" near line %d\n", idbuf, get_line_num());
    panic();
  }

  struct ASTNode* right = mkastleaf(AST_LVID, id);
  tok_assert(TT_EQUALS, "=");
  
  // Parse the expression.
  struct ASTNode* left = binexpr();

  struct ASTNode* tree = mkastnode(AST_ASSIGN, left, right, 0);
  mkAST(tree, -1);
  freeall_regs();
  semi();
}


static void var_dec() {
  // For now we will only have one type (U8),
  // so ensure that the next token is a U*
  // keyword. TODO: Add other types.
  
  tok_assert(TT_U8, "u8");
  id();
  pushglob(idbuf);
  rmkglobsym(idbuf);
  
  if (cur_token.type == TT_EQUALS) {
    assignment(1);
    return;                           // No need to handle semi as assignment() does already.
  }

  semi();
}


static void statement(void) {
  while (1) { 
    switch (cur_token.type) {
      case TT_EOF:
        return;
      case TT_U8:
        var_dec();
        break;
      case TT_OUT:
        out_statement();
        break;
      case TT_ID:
        assignment(0);
        break;
      default:
        printf(ERR "Syntax error near line %d\n", get_line_num());
        panic();
    } 
  }
}


void parse(void) {
  scan(&cur_token);
  compile_init();
  statement();
  compile_end();
}
