#include <stdio.h>
#include <stdlib.h>
#include <parser.h>

FILE* input = NULL;

static void run(void) {
  parse();
}


int main(int argc, char** argv) {
  if (argc < 2) {
    printf("KessCC: Too few arguments!\n");
    return 1;
  }
  
  for (int i = 1; i < argc; ++i) {
    input = fopen(argv[i], "r");

    if (input == NULL) {
      printf("KessCC: %s does not exist!\n", input);
      return 1;
    }

    run();
  }

  fclose(input);
}
