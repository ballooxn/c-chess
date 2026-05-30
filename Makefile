CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c17 -O2 -march=native
DEBUG_FLAGS = -g -DDEBUG -fsanitize=address -fsanitize=undefined

SRC_DIR = src
BIN_DIR = bin
TEST_DIR = tests
OBJ_DIR = $(BIN_DIR)/obj

ENGINE = $(BIN_DIR)/chess-engine
TESTER = $(BIN_DIR)/tests

ENGINE_SRCS = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c) $(filter-out $(SRC_DIR)/main.c, $(ENGINE_SRCS))

ENGINE_OBJS = $(ENGINE_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/$(SRC_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=$(OBJ_DIR)/%.o)

.PHONY: all run test clean

all: $(ENGINE)

run: $(ENGINE)
	$(ENGINE)

test: $(TESTER)
	$(TESTER)

$(ENGINE): $(ENGINE_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(TESTER): $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(TEST_DIR) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)
	@echo "Cleaned build files"