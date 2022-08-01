#ifndef AST_H
#define AST_H

#include <stdint.h>

typedef enum {
  AST_ADD,
  AST_SUB,
  AST_MUL,
  AST_DIV,
  AST_INTLIT
} AST_OP;


struct ASTNode {
  AST_OP op;
  struct ASTNode* left;
  struct ASTNode* right;
  uint64_t intval;
};


struct ASTNode* mkastnode(AST_OP op, struct ASTNode* left, struct ASTNode* right, uint64_t intval);
struct ASTNode* mkastleaf(AST_OP op, int intval);
struct ASTNode* mkastunary(AST_OP op, struct ASTNode* left, int intval);

#endif
