#include <compile.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Make an executable binary with GCC.
// KessCC has nothing to do with GCC.
// Assembling the assembly is easier with GCC.
#define GCC_PATH "/bin/gcc"
#define PATH_LOWEST_ASCII_VAL 97
#define PATH_HIGHEST_ASCII_VAL 122
#define PATH_RAND (rand() % (PATH_HIGHEST_ASCII_VAL - PATH_LOWEST_ASCII_VAL + 1)) + PATH_LOWEST_ASCII_VAL
#define MAX_USED_REGS 4

typedef int8_t REG;
static FILE* out = NULL;
static char* regs[MAX_USED_REGS] = {"%r8", "%r9", "%r10", "%r11"};
static uint8_t reg_bitmap = 0b1111;

// Returns -1 if no regs are found.
static REG alloc_reg(void) {
  for (REG i = 0; i < MAX_USED_REGS; ++i) {
    if (reg_bitmap & (1 << i)) {
      reg_bitmap &= ~(1 << i);
      return i;
    }
  }

  return -1;
}


static void free_reg(REG r) {
  if (r > MAX_USED_REGS - 1 || r < 0) return;
  reg_bitmap |= (1 << r);
}


// Returns the register number that has been loaded with @param value.
static REG rload(uint64_t value) {
  REG r = alloc_reg();

  if (r < 0) {
    return -1;
  }

  fprintf(out, "\tmovq\t$%d, %s\n", value, regs[r]);
  return r;

}


static void check_regs(REG r1, REG r2) {
  if (r1 < 0 || r2 < 0) {
    printf(ERR "Ran out of registers.");
    exit(1);
  }
}


// Adds to registers together and returns register with result.
static REG radd(REG r1, REG r2) {
  check_regs(r1, r2);
  fprintf(out, "\taddq\t%s, %s\n", regs[r1], regs[r2]);
  free_reg(r1);
  return r2;
}


// Multiplies two registers together, returns register with result.
static REG rmul(REG r1, REG r2) {
  check_regs(r1, r2) ;
  fprintf(out, "\timulq\t%s, %s\n", regs[r1], regs[r2]);
  free_reg(r1);
  return r2;
}


// Subtract the second register from the first and return register with result.
static REG rsub(REG r1, REG r2) {
  check_regs(r1, r2) ;
  fprintf(out, "\tsubq\t%s, %s\n", regs[r2], regs[r1]);
  free_reg(r2);
  return r1;
}


// Divide first register by the second register and return register withr result.
static REG rdiv(REG r1, REG r2) {
  check_regs(r1, r2) ;
  fprintf(out, "\tmovq\t%s, %%rax\n", regs[r1]);
  fprintf(out, "\tcqo\n");
  fprintf(out, "\tidivq\t%s\n", regs[r2]);
  fprintf(out, "\tmovq\t%%rax, %s\n", regs[r1]);
  free_reg(r2);
  return r1;
}


static void rprint(REG r) {
  check_regs(r, 0);
  fprintf(out, "\tmovq\t%s, %%rdi\n", regs[r]);
  fprintf(out, "\tcall\tprintint\n");
  free_reg(r);
}



static void prologue(void) {
  fputs(
	  "\t.text\n"
	  ".LC0:\n"
	  "\t.string\t\"%d\\n\"\n"
	  "printint:\n"
	  "\tpushq\t%rbp\n"
	  "\tmovq\t%rsp, %rbp\n"
	  "\tsubq\t$16, %rsp\n"
	  "\tmovl\t%edi, -4(%rbp)\n"
	  "\tmovl\t-4(%rbp), %eax\n"
	  "\tmovl\t%eax, %esi\n"
	  "\tleaq	.LC0(%rip), %rdi\n"
	  "\tmovl	$0, %eax\n"
	  "\tcall	printf@PLT\n"
	  "\tnop\n"
	  "\tleave\n"
	  "\tret\n"
	  "\n"
	  "\t.globl\tmain\n"
	  "\t.type\tmain, @function\n"
	  "main:\n"
	  "\tpushq\t%rbp\n"
	  "\tmovq	%rsp, %rbp\n",
    out);
}

static void epilogue(void) {
  fputs(
	  "\tmovl	$0, %eax\n"
	  "\tpopq	%rbp\n"
	  "\tret\n",
    out);
}


AST_OP interpretAST(struct ASTNode* node) {
  uint64_t leftreg, rightreg;

  if (node->left) {
    leftreg = interpretAST(node->left);
  }

  if (node->right) {
    rightreg = interpretAST(node->right);
  }

  switch (node->op) {
    case AST_ADD:
      return radd(leftreg, rightreg);
    case AST_SUB:
      return rsub(leftreg, rightreg);
    case AST_MUL:
      return rmul(leftreg, rightreg);
    case AST_DIV:
      return rdiv(leftreg, rightreg);
    case AST_INTLIT:
      return rload(node->intval);
  }
}

void compile(struct ASTNode* tree) {
  srand(time(NULL));

  // Randomize the path.
  char out_name[150];
  snprintf(out_name, sizeof(out_name), "/tmp/kesscc-%c%c%c%c%c.out.s", PATH_RAND, PATH_RAND, PATH_RAND, PATH_RAND, PATH_RAND);
  out = fopen(out_name, "a");

  prologue();

  REG r = interpretAST(tree);
  rprint(r);
    
  epilogue();

  fclose(out);
  pid_t child = fork();

  if (child != 0) {
    execl(GCC_PATH, GCC_PATH, out_name, "-oa.out", NULL);
  } else {
    sleep(2);
    remove(out_name);
  }
}
