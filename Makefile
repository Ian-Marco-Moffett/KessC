CFILES = $(wildcard src/*.c)
CFLAGS = -Werror=implicit     \
        -Werror=implicit-function-declaration  \
        -Werror=implicit-int \
        -Werror=int-conversion \
        -Werror=incompatible-pointer-types \
        -Werror=int-to-pointer-cast        \
        -fsanitize=address

.PHONY: all
all: $(CFILES)
	@ mkdir -p bin
	gcc $(CFLAGS) -Iinclude $^ -o bin/kesscc

.PHONY: install
install: bin/kesscc
	sudo install bin/kesscc /usr/local/bin/kesscc
