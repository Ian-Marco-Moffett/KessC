#include <ast.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

static struct ASTNode** nodes = NULL;
static uint64_t nodes_idx = 0;
extern FILE* input;

struct ASTNode* mkastnode(AST_OP op, struct ASTNode* left, struct ASTNode* right, uint64_t intval) {
  if (nodes == NULL) {
    nodes = malloc(sizeof(struct ASTNode*));

    if (nodes == NULL) {
      fclose(input);
      printf(ERR "__INTERNAL_ERR__ [Unable to malloc() node list in %s]\n", __FILE__);
      exit(1);
    }
  }

  struct ASTNode* n = malloc(sizeof(struct ASTNode));

  if (n == NULL) {
      fclose(input);
      printf(ERR "__INTERNAL_ERR__ [Unable to malloc() node in %s]\n", __FILE__);
      exit(1);
  }

  n->op = op;
  n->left = left;
  n->right = right;
  n->intval = intval;
  
  nodes[nodes_idx++] = n;
  nodes = realloc(nodes, sizeof(struct ASTNode*) + (nodes_idx + 1));

  if (nodes == NULL) {
      fclose(input);
      printf(ERR "__INTERNAL_ERR__ [Unable to realloc() node list in %s]\n", __FILE__);
      exit(1);
  }

  return n;
}


struct ASTNode* mkastleaf(AST_OP op, int intval) {
  return mkastnode(op, NULL, NULL, intval);
}

struct ASTNode* mkastunary(AST_OP op, struct ASTNode* left, int intval) {
  return mkastnode(op, left, NULL, intval);
}
