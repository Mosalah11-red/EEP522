CC=gcc
CFLAGS=-O2 -pthread -Wall -Wextra

BIN_DIR=bin
SRC_DIR=src

all: $(BIN_DIR)/pingpong_latency

$(BIN_DIR)/pingpong_latency: $(SRC_DIR)/pingpong_latency.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BIN_DIR) *.o
