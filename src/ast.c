#include <ast.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

extern FILE* input;
static struct ASTNode** nodes = NULL;
static uint64_t nodes_idx = 0;

struct ASTNode* mkastnode(AST_OP op, struct ASTNode* left, struct ASTNode* mid, struct ASTNode* right, uint64_t intval) {
  if (nodes == NULL) {
    nodes = (struct ASTNode**) malloc(sizeof(struct ASTNode*));
    if (nodes == NULL) {
      fclose(input);
      printf(ERR "__INTERNAL_ERR__ [Unable to malloc() node list in %s]\n", __FILE__);
      exit(1);
    }
  }

  struct ASTNode* n = (struct ASTNode*)  malloc(sizeof(struct ASTNode));

  if (n == NULL) {
      fclose(input);
      printf(ERR "__INTERNAL_ERR__ [Unable to malloc() node in %s]\n", __FILE__);
      exit(1);
  }

  n->op = op;
  n->left = left;
  n->right = right;
  n->intval = intval;
  n->mid = mid;

  nodes = (struct ASTNode**) realloc(nodes, sizeof(struct ASTNode*) * (nodes_idx + 2));
  
  if (nodes == NULL) {
      fclose(input);
      printf(ERR "__INTERNAL_ERR__ [Unable to realloc() node list in %s]\n", __FILE__);
      exit(1);
  }

  nodes[nodes_idx++] = n;

  return n;
}




struct ASTNode* mkastleaf(AST_OP op, int intval) {
  return mkastnode(op, NULL, NULL, NULL, intval);
}

struct ASTNode* mkastunary(AST_OP op, struct ASTNode* left, int intval) {
  return mkastnode(op, left, NULL, NULL, intval);
}


void ast_destroy(void) {
  for (uint64_t i = 0; i < nodes_idx; ++i) {
    free(nodes[i]);
  }

  free(nodes);
}


AST_OP arithop(TOKEN_TYPE tok_type) {
  switch (tok_type) {
    case TT_PLUS:
      return AST_ADD;
    case TT_MINUS:
      return AST_SUB;
    case TT_STAR:
      return AST_MUL;
    case TT_SLASH:
      return AST_DIV;
    case TT_EQ:
      return AST_EQ;
    case TT_NE:
      return AST_NE;
    case TT_LT:
      return AST_LT;
    case TT_GT:
      return AST_GT;
    case TT_LE:
      return AST_LE;
    case TT_GE:
      return AST_GT;
    default:
      return AST_NONE;
  }
}