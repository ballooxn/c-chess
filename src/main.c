#include "main.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define TO_BITS(rank, file)     ((rank) * 8 + (file))

// input will be like "pe2e4" (piece, starting pos, ending pos)
// make it more explanatory in the future (simple for me rn)

static bool valid_input(char* pos) {
    if (strlen(pos) != 2) return false;

    char file = pos[0];
    char rank = pos[1];

    bool valid = ((file >= 'a' && file <= 'h') && (rank >= '1' && rank <= '8'));
    return valid;
}

static char* player_input(char* buffer, size_t size) {
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

static Move string_to_move(char* string, Board board, Color color) {
    int start = pos_to_int(string[1], string[0]);
    int end = pos_to_int(string[3], string[2]);
    char piece = get_piece(board, start, color);
    Move move = {.start = start, .end = end, .piece = piece, .color = color};
    return move;
}

void get_player_move(char* buffer, size_t size) {
    char start[3] = "";
    char end[3] = "";
    do {
        player_input(buffer, size);
        // implement false if no piece on start square
        if (strlen(buffer) != 4) continue;

        start[0] = buffer[0];
        start[1] = buffer[1];
        start[2] = '\0';
        end[0] = buffer[2];
        end[1] = buffer[3];
        end[2] = '\0';
    } while (!valid_input(start) || !valid_input(end));
}

int main(void) {
    Board board = init_board();
    init_attacks();

    bool is_winner = false;
    Color player = BLACK;

    do {
        player = (player == BLACK) ? WHITE : BLACK;
        print_board(&board);
        puts("Please choose a start and end location");
        Move move;
        bool is_valid = false;
        do {
            // change to do while loop
            char buffer[5] = "";
            get_player_move(buffer, sizeof(buffer));
            move = string_to_move(buffer, board, player);
            is_valid = is_legal(board, move);
            if (is_valid) {
                puts("VALID MOVE");
            } else {
                puts("NOT VALID!!!");
            }
        } while (!is_valid);
        move_piece(&board, move);
        Color opp = (player == WHITE) ? BLACK : WHITE;
        if (in_checkmate(&board, opp)) break;
    } while (!is_winner);
    puts("Checkmate!!!!");
    printf("The winner is: %d\n", player);
    print_board(&board);
    return 0;
}