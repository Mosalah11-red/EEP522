CC=gcc
CFLAGS=-O2 -Wall -Wextra
BIN=bin
SRC=src

all: $(BIN)/memcpy_bench $(BIN)/fs_copy_bench

$(BIN)/memcpy_bench: $(SRC)/memcpy_bench.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/fs_copy_bench: $(SRC)/fs_copy_bench.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN)/*
