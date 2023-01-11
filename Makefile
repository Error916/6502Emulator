CFLAGS=-pedantic -Wall -Wextra -Werror -Wfatal-errors -Ofast -flto -march=native -pipe
LIBS=
SRC=src/main.c src/cpu_6502.c
CC=gcc

emu: $(SRC)
	$(CC) $(CFLAGS) -o emu $(SRC) $(LIBS)
