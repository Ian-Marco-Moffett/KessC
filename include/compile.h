#ifndef COMPILE_H
#define COMPILE_H

#include <ast.h>


AST_OP interpretAST(struct ASTNode* node);
void compile(void);

#endif
