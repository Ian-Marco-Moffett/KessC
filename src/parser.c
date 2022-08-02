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
  struct ASTNode* n = mkastnode(ntype, left, NULL, right, 0);
  return n;
}


static struct ASTNode* out_statement(void) {
  tok_assert(TT_OUT, "out");
  tok_assert(TT_LPAREN, "(");

  struct ASTNode* tree = binexpr();
  tok_assert(TT_RPAREN, ")");
  semi();

  tree = mkastunary(AST_OUT, tree, 0);

  return tree;
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

static struct ASTNode* assignment(uint8_t init) {
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

  struct ASTNode* tree = mkastnode(AST_ASSIGN, left, NULL, right, 0); 
  semi();
  return tree;
}


static struct ASTNode* var_dec() {
  // For now we will only have one type (U8),
  // so ensure that the next token is a U*
  // keyword. TODO: Add other types.
  
  tok_assert(TT_U8, "u8");
  id();
  pushglob(idbuf);
  rmkglobsym(idbuf);
  
  if (cur_token.type == TT_EQUALS) {
    return assignment(1);    // No need to handle semi as assignment() does already
  }

  semi();
  return NULL;
}


static void lbrace(void) {
  tok_assert(TT_LBRACE, "{");
}


static void rbrace(void) {
  tok_assert(TT_RBRACE, "}");
}


static struct ASTNode* statement(void);


static struct ASTNode* if_statement(void) {
  struct ASTNode* conditionAST = NULL;
  struct ASTNode* trueAST = NULL;
  struct ASTNode* falseAST = NULL;

  tok_assert(TT_IF, "if");
  tok_assert(TT_LPAREN, "(");
  conditionAST = binexpr();

  if (conditionAST->op < AST_EQ || conditionAST->op > AST_GE) {
    printf(ERR "Bad comparison operator used on line %d\n", get_line_num());
    panic();
  }

  tok_assert(TT_RPAREN, ")");

  trueAST = statement();

  if (cur_token.type == TT_ELSE) {
    scan(&cur_token);
    falseAST = statement();
  }

  return mkastnode(AST_IF, conditionAST, trueAST, falseAST, 0);
}


static struct ASTNode* statement(void) {
  struct ASTNode* left = NULL;
  struct ASTNode* tree = NULL;

  lbrace();

  while (1) { 
    switch (cur_token.type) {
      case TT_EOF:
        return NULL;
      case TT_U8:
        tree = var_dec();

        if (tree != NULL) {
          mkAST(tree, -1, 0);
          freeall_regs();
        }

        break;
      case TT_OUT:
        tree = out_statement();
        break;
      case TT_IF:
        tree = if_statement();
        break;
      case TT_ID:
        tree = assignment(0);
        mkAST(tree, -1, 0);
        freeall_regs();
        break;
      case TT_RBRACE:
        rbrace();
        return left;
      default:
        printf(ERR "Syntax error near line %d\n", get_line_num());
        panic();
    }

    if (tree) {
      if (left == NULL) {
        left = tree;
      } else {
        left = mkastnode(AST_GLUE, left, NULL, tree, 0);
      }
    }
  }
}


void parse(void) {
  scan(&cur_token);
  compile_init();
  mkAST(statement(), -1, 0);
  compile_end();
}
