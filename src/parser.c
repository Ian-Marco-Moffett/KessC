#include <lexer.h>
#include <ast.h>
#include <err.h>
#include <ptype.h>
#include <compile.h>
#include <symbol.h>
#include <stdio.h>

static struct Token cur_token;
extern char idbuf[];
extern struct SymbolTable globsyms[MAX_SYMBOLS];

static struct ASTNode* statement(void);
static struct ASTNode* funccall(void);
uint64_t current_function_id = 0;

static struct ASTNode* primary(void) {
  struct ASTNode* n; 
  uint64_t id;

  switch (cur_token.type) {
    case TT_INTLIT:
      // TODO: Support bigger types.
      if (cur_token.intval >= 0 && cur_token.intval < 256) {
        n = mkastleaf(AST_INTLIT, PT_U8, cur_token.intval);
      } else {
        printf(ERR "Value assigned to U8 overflows/underflows near line %d\n", get_line_num());
        panic();
      }
      break;
    case TT_ID:
      // This could be a variable or function call.
      scan(&cur_token);

      // Theres an open parenthesis, thus
      // it's a function call.
      if (cur_token.type == TT_LPAREN) {
        return funccall();
      }
    
      // Not a function call, reject token.
      reject_token(&cur_token);

      id = locateglob(idbuf);

      if (id == -1) {
        printf(ERR "\"%s\" undeclared.\n");
        panic();
      }

      n = mkastleaf(AST_ID, globsyms[id].ptype, id);
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


static struct ASTNode* while_statement(void) {
  struct ASTNode* conditionAST;
  struct ASTNode* bodyAST;
  
  tok_assert(TT_WHILE, "while");
  tok_assert(TT_LPAREN, "(");

  // Parse expression.
  conditionAST = binexpr();

  if (conditionAST->op < AST_EQ || conditionAST->op > AST_GE) {
    printf(ERR "Bad comparison operator used on line %d\n", get_line_num());
    panic();
  }

  tok_assert(TT_RPAREN, ")");

  bodyAST = statement();
  return mkastnode(AST_WHILE, conditionAST, NULL, bodyAST, 0);
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
    
    // This could be a variable or a function call, 
    // if we have an lparen then this is a function call.
    if (cur_token.type == TT_LPAREN) {
      return funccall();
    }
  }

  uint64_t id = locateglob(idbuf);

  if (id == -1) {
    printf(ERR "Referance to an undeclared variable \"%s\" near line %d\n", idbuf, get_line_num());
    panic();
  }

  struct ASTNode* right = mkastleaf(AST_LVID, globsyms[id].ptype, id);
  tok_assert(TT_EQUALS, "=");
  
  // Parse the expression.
  struct ASTNode* left = binexpr();

  struct ASTNode* tree = mkastnode(AST_ASSIGN, left, NULL, right, 0); 
  semi();
  return tree;
}


static PTYPE parse_type(TOKEN_TYPE t) {
  switch (t) {
    case TT_U8:
      return PT_U8;
    case TT_VOID:
      return PT_VOID;
    default:
      printf(ERR "Illegal type used on line %d\n", get_line_num());
  }
}


static struct ASTNode* var_dec() {
  PTYPE type = parse_type(cur_token.type);

  if (type == PT_VOID) {
    printf(ERR "Invalid variable type 'void' used on line %d\n", get_line_num());
    panic();
  }
  
  scan(&cur_token);
  id();
  rmkglobsym(pushglob(idbuf, type, ST_VAR));
  
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

static struct ASTNode* funccall(void) {
  struct ASTNode* tree;
  uint64_t id = locateglob(idbuf);

  // Panic if function is not defined.
  if (id == -1) {
    printf(ERR "Undefined function %s, line %d. It's okay, everybody forgets things :)", get_line_num());
    panic();
  }

  tok_assert(TT_LPAREN, "(");

  // Parse expression inside parenthesis.
  tree = binexpr();

  // Build function call AST node.
  // Functions return type will be the node's type.
  // Also keep track of the function's symbol ID.
  tree = mkastunarytype(AST_FUNCCALL, globsyms[id].ptype, tree, id);
  tok_assert(TT_RPAREN, ")");
  semi();
  return tree;
}


uint8_t type_compatible(PTYPE* left, PTYPE* right, int rightonly) {
  uint64_t leftsize, rightsize;

  // Same types, they are compatible.
  if (*left == *right) {
    *left = *right = 0;
    return 1;
  }

  // Get sizes for each time.
  rprimsize(*left);
  rprimsize(*right);

  // Types with zero size won't work with anything at all.
  if ((leftsize == 0) || (rightsize == 0)) {
    return 0;
  }

  if (leftsize < rightsize) {
    *left = AST_WIDEN;
    *right = 0;
    return 1;
  }

  if (rightsize < leftsize) {
    if (rightonly) {
      *left = 0;
      *right = AST_WIDEN;
      return 1;
    }
  }

  *left = *right = 0;
  return 1;
}


static struct ASTNode* return_statement(void) {
  struct ASTNode* tree;
  PTYPE returntype, functype;

  if (globsyms[current_function_id].ptype == PT_VOID) {
    printf(ERR "Value being returned from void function, near line %d\n", get_line_num());
    panic();
  }

  // Ensure there is a return statement.
  tok_assert(TT_RETURN, "return");
  
  // Parse expression.
  tree = binexpr();
  returntype = tree->type;
  functype = globsyms[current_function_id].ptype;

  // Ensure type is good with this functions return type.
  if (!(type_compatible(&returntype, &functype, 1))) {
    printf(ERR "Incompatible return type near line %d\n", get_line_num());
    panic();
  }

  // TODO: Do something here when types get bigger.
  tree = mkastunarytype(AST_RETURN, PT_NONE, tree, 0);
  semi();
  return tree;
}

static struct ASTNode* func_def(void) {
  struct ASTNode* tree;
  uint64_t symslot;

  PTYPE type = parse_type(cur_token.type);
  scan(&cur_token);
  id();
  symslot = pushglob(idbuf, type, ST_FUNC);
  current_function_id = symslot;
  tok_assert(TT_LPAREN, "(");
  tok_assert(TT_RPAREN, ")");

  tree = statement();

  return mkastunary(AST_FUNC, tree, symslot);
}


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
      case TT_WHILE:
        tree = while_statement();
        break;
      case TT_RETURN:
          tree = return_statement();
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

  while (1) {
    mkAST(func_def(), -1, 0);
  
    if (cur_token.type == TT_EOF) {
      break;
    }

  }

  compile_end();
}
