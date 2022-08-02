CFILES = $(wildcard src/*.c)
CFLAGS = -Werror=implicit     \
        -Werror=implicit-function-declaration  \
        -Werror=implicit-int \
        -Werror=int-conversion \
        -Werror=incompatible-pointer-types \
        -Werror=int-to-pointer-cast        \
        -fsanitize=address
INCLUDES = -I$(SRCDIR)/include
SRCDIR = $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
VPATH := $(VPATH) $(SRCDIR)
BUILDDIR := $(PWD)/bin

.PHONY: all
all: $(CFILES)
	@ mkdir -p $(BUILDDIR)
	echo [CC] $^ - $@
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $(BUILDDIR)/kesscc