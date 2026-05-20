CC          = gcc
CFLAGS      = -Wall -Wextra -Wpedantic -std=c17 -O2 -march=native
DEBUG_FLAGS = -g -DDEBUG -fsanitize=address -fsanitize=undefined

SRC_DIR     = src
BIN_DIR     = bin
TARGET      = $(BIN_DIR)/chess-engine

SOURCES     = $(wildcard $(SRC_DIR)/*.c)
OBJECTS     = $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

debug: CFLAGS = -Wall -Wextra -Wpedantic -std=c17 -g -DDEBUG -fsanitize=address -fsanitize=undefined
debug: clean $(TARGET)
	@echo "Debug build complete with sanitizers"

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BIN_DIR)
	@echo "Cleaned build files"

.PHONY: all debug run clean