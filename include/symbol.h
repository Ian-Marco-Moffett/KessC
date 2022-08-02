#ifndef SYMBOL_H
#define SYMBOL_H

// TODO: Allocate symbols on heap.
#define MAX_SYMBOLS 2000

#include <stdint.h>


struct SymbolTable {
  char* name;
};


void destroy_symtbl(void);
int16_t pushglob(const char* name);
int16_t locateglob(const char* name);

#endif
