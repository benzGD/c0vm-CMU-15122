# C0_INSTALL is the directory containing the c0 deployment
# e.g., /usr/local/share/cc0 if cc0 is at /usr/local/share/cc0/bin/cc0
C0_TMP=$(shell dirname `which cc0`)
C0_INSTALL=$(shell dirname $(C0_TMP))
C0LIBDIR=$(C0_INSTALL)/lib
C0RUNTIMEDIR=$(C0_INSTALL)/runtime

# Compiling c0vm
CFLAGS=-fwrapv -Wall -Wextra -Werror -g
CC=gcc -std=c99 -pedantic
CC_FAST:=$(CC) $(CFLAGS)
CC_SAFE:=$(CC) $(CFLAGS) -fsanitize=undefined -DDEBUG

# Linker flags emptied to avoid searching for missing CMU libraries
LINKERFLAGS= 

# Library objects
LIB=lib/read_program.o lib/stack.o lib/c0v_stack.o lib/c0vm_abort.o
SAFE_LIB=$(LIB:%.o=%-safe.o)
FAST_LIB=$(LIB:%.o=%-fast.o)

.PHONY: c0vm c0vmd clean
default: c0vm c0vmd

# Added natives_stub.c to dependencies and compilation command
c0vm: c0vm.c c0vm_main.c natives_stub.c
	$(CC_FAST) $(FAST_LIB) -o c0vm c0vm_main.c c0vm.c natives_stub.c $(LINKERFLAGS)

# Added natives_stub.c to debug target as well
c0vmd: c0vm.c c0vm_main.c natives_stub.c
	$(CC_SAFE) $(SAFE_LIB) -o c0vmd c0vm_main.c c0vm.c natives_stub.c $(LINKERFLAGS)

clean:
	rm -Rf c0vm c0vmd *.dSYM