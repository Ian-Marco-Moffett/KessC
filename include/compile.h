#ifndef COMPILE_H
#define COMPILE_H

#include <ast.h>

typedef int8_t REG;

REG mkAST(struct ASTNode* tree);
void compile_init(void);
void compile_end(void);
void check_regs(REG r1, REG r2);
void rprint(REG r);
void freeall_regs(void);

#endif
