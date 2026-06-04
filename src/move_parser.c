#include "board.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// input will be like "pe2e4" (piece, starting pos, ending pos)
// make it more explanatory in the future (simple for me rn)

bool valid_input(char* pos) {
    if (strlen(pos) != 2) return false;

    char file = pos[0];
    char rank = pos[1];

    bool valid = ((file >= 'a' && file <= 'h') && (rank >= '1' && rank <= '8'));
    return valid;
}

char* player_input(char* buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return NULL;
    }

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return NULL;
    }

    // remove potential newline
    buffer[strcspn(buffer, "\n")] = '\0';

    return buffer;
}

static inline int file_to_int(char file) {
    return tolower(file) - 'a';
}

static inline int rank_to_int(char rank) {
    return rank - '1';
}

int pos_to_int(char rank, char file) {
    int int_rank = rank_to_int(rank);
    int int_file = file_to_int(file);
    return TO_BITS(int_rank, int_file);
}

Move string_to_move(char* string, Board board, Color color) {
    int start = pos_to_int(string[1], string[0]);
    int end = pos_to_int(string[3], string[2]);
    char piece = get_piece(&board, start, color);
    Move move = {.start = start, .end = end, .piece = piece, .color = color};
    return move;
}