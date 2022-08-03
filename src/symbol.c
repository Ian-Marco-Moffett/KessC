#include <symbol.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

struct SymbolTable globsyms[MAX_SYMBOLS];
static uint64_t glob_syms_count = 0;

int16_t locateglob(const char* name) {
  for (uint16_t i = 0; i < glob_syms_count; ++i) {
    if (strcmp(name, globsyms[i].name) == 0) {
      return i;
    }
  }

  return -1;
}


static uint16_t mkglob(void) {
  uint16_t p = 0;

  if ((p = glob_syms_count++) >= MAX_SYMBOLS) {
    printf(ERR "Too many global symbols!\n");
  }

  return p;
}

// Return -1 if global symbol already exists.
// Otherwise, returns slot number.

int16_t pushglob(const char* name, PTYPE ptype, STYPE stype) {
  int16_t slot = locateglob(name);

  if (slot != -1) {
    return -1;
  }
  
  slot = mkglob();
  globsyms[slot].name = strdup(name);
  globsyms[slot].stype = stype;
  globsyms[slot].ptype = ptype;
  return slot;
}
