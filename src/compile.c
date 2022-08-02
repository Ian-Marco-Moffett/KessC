#include <compile.h>
#include <err.h>
#include <symbol.h>
#include <flags.h>
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

extern struct SymbolTable globsyms[MAX_SYMBOLS];
static FILE* out = NULL;
static char* regs[MAX_USED_REGS] = {"%r8", "%r9", "%r10", "%r11"};
static char* bregs[MAX_USED_REGS] = {"%r8b", "%r9b", "%r10b", "%r11b"};
static uint8_t reg_bitmap = 0b1111;
static char out_name[150];

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


void freeall_regs(void) {
  reg_bitmap = 0b1111;
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


void check_regs(REG r1, REG r2) {
  if (r1 < 0 || r2 < 0) {
    printf(ERR "Ran out of registers.");
    panic();
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


void rprintint(REG r) {
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

static REG rloadglob(const char* name) {
  REG reg = alloc_reg();
  check_regs(reg, 0);
  fprintf(out, "\tmovq\t%s(%%rip), %s\n", name, regs[reg]);
  return reg;
}


static REG rstoreglob(REG r, const char* name) {
  fprintf(out, "\tmovq\t%s, %s(%%rip)\n", regs[r], name);
  return r;
}

// Compare two regs.
static REG rcmp(REG r1, REG r2, const char* how) {
  fprintf(out, "\tcmpq\t%s, %s\n", regs[r2], regs[r1]);
  fprintf(out, "\t%s\t%s\n", how, bregs[r2]);
  fprintf(out, "\tandq\t$0xFF, %s\n", regs[r2]);
  free_reg(r1);
  return r2;
}

static REG requal(REG r1, REG r2) {
  return rcmp(r1, r2, "sete");
}

static REG rnotequal(REG r1, REG r2) {
  return rcmp(r1, r2, "setne");
}

static REG rlessthan(REG r1, REG r2) {
  return rcmp(r1, r2, "setl");
}

static REG rgreaterthan(REG r1, REG r2) {
  return rcmp(r1, r2, "setg");
}


static REG rlessequal(REG r1, REG r2) {
  return rcmp(r1, r2, "setle");
}

static REG rgreaterequal(REG r1, REG r2) {
  return rcmp(r1, r2, "setge");
}

void rmkglobsym(const char* name) {
  fprintf(out, "\t.comm\t%s, 8, 8\n", name);
}

static void epilogue(void) {
  fputs(
	  "\tmovl	$0, %eax\n"
	  "\tpopq	%rbp\n"
	  "\tret\n",
    out);
}


static uint64_t alloc_label(void) {
  static uint64_t id = 1;
  return id++;
}

REG mkAST(struct ASTNode* tree, REG r, AST_OP parent_op);

static char *cmplist[] = {"sete", "setne", "setl", "setg", "setle", "setge"};
static char *invcmplist[] = {"jne", "je", "jge", "jle", "jg", "jl"};

REG rcmpnset(AST_OP op, REG r1, REG r2) {
  if (op < AST_EQ || op > AST_GE) {
    printf(ERR "__INTERNAL_ERR__: Bad op in %s\n", __func__);
    panic();
  }

  fprintf(out, "\tcmpq\t%s, %s\n", regs[r2], regs[r1]);
  fprintf(out, "\t%s\t%s\n", cmplist[op - AST_EQ], bregs[r2]);
  fprintf(out, "\tmovzbq\t%s, %s\n", bregs[r2], regs[r2]);
  free_reg(r1);
  return r2;
}


REG rcmpnjmp(AST_OP op, REG r1, REG r2, uint64_t label) {
  if (op < AST_EQ || op > AST_GE) {
    printf(ERR "__INTERNAL_ERR__: Bad op in %s\n", __func__);
    panic();
  }

  fprintf(out, "\tcmpq\t%s, %s\n", regs[r2], regs[r1]);
  fprintf(out, "\t%s\tL%d\n", invcmplist[op - AST_EQ], label);
  freeall_regs();
  return -1;
}


static void rjmp(uint64_t label) {
  fprintf(out, "\tjmp\tL%d\n", label);
}


static void rlabel(uint64_t label) {
  fprintf(out, "L%d:\n", label);
}


static REG mkifast(struct ASTNode* n) {
  uint64_t false_label, end_label;

  false_label = alloc_label();

  if (n->right) {
    end_label = alloc_label();
  }

  mkAST(n->left, false_label, n->op);
  freeall_regs();

  // Make true compound statement.
  mkAST(n->mid, -1, n->op);
  freeall_regs();

  if (n->right) {
    rjmp(end_label);
  }

  rlabel(false_label);

  if (n->right) {
    mkAST(n->right, -1, n->op);
    freeall_regs();
    rlabel(end_label);
  }

  return -1;
}


REG mkAST(struct ASTNode* node, REG r, AST_OP parent_op) {
  uint64_t leftreg, rightreg;

  switch (node->op) {
    case AST_IF:
      return mkifast(node);
    case AST_GLUE:
      mkAST(node->left, -1, node->op);
      freeall_regs();
      mkAST(node->right, -1, node->op);
      freeall_regs();
      return -1;
  }

  if (node->left) {
    leftreg = mkAST(node->left, -1, node->op);
  }

  if (node->right) {
    rightreg = mkAST(node->right, leftreg, node->op);
  }

  switch (node->op) {
    case AST_INTLIT:
      return rload(node->intval);
    case AST_ID:
      return rloadglob(globsyms[node->id].name);
    case AST_LVID:
      return rstoreglob(r, globsyms[node->id].name);
    case AST_ADD:
      return radd(leftreg, rightreg);
    case AST_SUB:
      return rsub(leftreg, rightreg);
    case AST_MUL:
      return rmul(leftreg, rightreg);
    case AST_DIV:
      return rdiv(leftreg, rightreg);
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_GT:
    case AST_LE:
    case AST_GE:
      if (parent_op == AST_IF) {
        return rcmpnjmp(node->op, leftreg, rightreg, r);
      }

      return rcmpnset(node->op, leftreg, rightreg);
    case AST_OUT:
      rprintint(leftreg);
      freeall_regs();
      return -1;
  }
}

void compile_init(void) {
  srand(time(NULL));

  // Randomize the path.
  snprintf(out_name, sizeof(out_name), "/tmp/kesscc-%c%c%c%c%c.out.s", PATH_RAND, PATH_RAND, PATH_RAND, PATH_RAND, PATH_RAND);
  out = fopen(out_name, "a");

  prologue(); 
}


void compile_end(void) {
  extern COMPILE_FLAGS compile_flags;

  epilogue();
  fclose(out);

  if (compile_flags & CF_ASMONLY) {
    char cmd[300];
    snprintf(cmd, sizeof(cmd), "mv %s ./", out_name);
    system(cmd);
    return;
  }

  pid_t child = fork();
  if (child != 0) {
    execl(GCC_PATH, GCC_PATH, out_name, "-oa.out", NULL);
  } else {
    sleep(2);
    remove(out_name);
  }
}
