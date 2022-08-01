#include <compile.h>
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


static FILE* out = NULL;


AST_OP interpretAST(struct ASTNode* node) {

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

void compile(void) {
  srand(time(NULL));

  // Randomize the path.
  char out_name[150];
  snprintf(out_name, sizeof(out_name), "/tmp/kesscc-%c%c%c%c%c.out.s", PATH_RAND, PATH_RAND, PATH_RAND, PATH_RAND, PATH_RAND);
  out = fopen(out_name, "a");

  prologue();
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
