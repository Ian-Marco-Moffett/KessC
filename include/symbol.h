#ifndef SYMBOL_H
#define SYMBOL_H

// TODO: Allocate symbols on heap.
#define MAX_SYMBOLS 2000

#include <stdint.h>
#include <ptype.h>

typedef enum {
  ST_VAR,
  ST_FUNC
} STYPE;

struct SymbolTable {
  char* name;
  PTYPE ptype;
  STYPE stype;
};


int16_t pushglob(const char* name, PTYPE ptype, STYPE stype);
int16_t locateglob(const char* name);

#endif
