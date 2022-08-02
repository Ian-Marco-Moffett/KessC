#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <ast.h>
#include <flags.h>
#include <symbol.h>

#ifndef linux
#error // Linux only for now.
#endif

FILE* input = NULL;

COMPILE_FLAGS compile_flags = (COMPILE_FLAGS)0;

static void run(void) {
  parse();
  ast_destroy();
}

void panic(void) {
  fclose(input);
  exit(1);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("KessCC: Too few arguments!\n");
    return 1;
  }
  
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 's':
          // Output only asm.
          compile_flags = (COMPILE_FLAGS)(compile_flags | CF_ASMONLY);
          break;
      }

      continue;
    }

    input = fopen(argv[i], "r");

    if (input == NULL) {
      printf("KessCC: Could not open %s, it does not exist!\n", argv[i]);
      return 1;
    }

    run();
  }

  fclose(input);
}