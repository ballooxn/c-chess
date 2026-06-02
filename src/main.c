#include "board.h"
#include "move_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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

    Color winner = NO_WINNER;
    Color player = BLACK;

    do {
        player = (player == BLACK) ? WHITE : BLACK;
        print_board(&board);
        switch (player) {
            case WHITE: printf("White, "); break;
            case BLACK: printf("Black, "); break;
            default: break;
        }
        puts("please choose a start and end location");
        Move move;
        bool is_valid = false;
        do {
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
        if (in_checkmate(&board, opp)) {
            winner = player;
        } else if (in_stalemate(&board, opp)) {
            winner = STALEMATE;
        }
    } while (winner == NO_WINNER);
    if (winner == STALEMATE) {
        puts("Stalemate! Nobody wins!");
    } else {
        printf("Checkmate! the winner is: ");
        switch (winner) {
            case WHITE: printf("White.\n"); break;
            case BLACK: printf("Black.\n"); break;
            default: break;
        }
    }
    print_board(&board);
    return 0;
}