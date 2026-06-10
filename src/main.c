#include "board.h"
#include "move_parser.h"
#include "validator.h"
#include "engine.h"
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

    GameResult winner = RESULT_NONE;
    Color player;

    bool play_engine = get_engine_choice();

    if (play_engine) {
        player = get_color_choice();
    } else {
        player = WHITE;
    }

    Color current_color = BLACK;
    do {
        Move move;
        current_color = OPP_COLOR(current_color);
        if (current_color == player || play_engine == false) {
            print_board(&board);
            switch (current_color) {
                case WHITE: printf("White, "); break;
                case BLACK: printf("Black, "); break;
                default: break;
            }
            puts("please choose a start and end location");
            
            bool is_valid = false;
            do {
                char buffer[5] = "";
                get_player_move(buffer, sizeof(buffer));
                move = string_to_move(buffer, board, current_color);
                is_valid = is_legal(&board, move);
                if (!is_valid) puts("Not a valid move.");
            } while (!is_valid);
            move_piece(&board, move);
            // pawn promotion
            if (PROMOTION(move.piece, move.end)) {
                char promo_choice = get_promotion_choice(move.color);
                PieceType promo_piece;
                switch (promo_choice) {
                    case 'q': promo_piece = QUEEN; break;
                    case 'r': promo_piece = ROOK; break;
                    case 'b': promo_piece = BISHOP; break;
                    case 'n': promo_piece = KNIGHT; break;
                    default: promo_piece = QUEEN; break;
                }
                promote_pawn(&board, move.end, promo_piece, move.color);
            }
        } else {
            move = engine_move(&board, current_color);
            move_piece(&board, move);
            if (move.piece == PAWN && (RANK_OF(move.end) == 7 || RANK_OF(move.end) == 0)) {
                promote_pawn(&board, move.end, move.engine_promotion, move.color);
            }
        }

        Color opp = (current_color == WHITE) ? BLACK : WHITE;
        if (is_checkmate(&board, opp)) {
            winner = (current_color == WHITE) ? RESULT_WHITE_WINS : RESULT_BLACK_WINS;
        } else if (is_stalemate(&board, opp)) {
            winner = RESULT_STALEMATE;
        } else if (insufficient_material(&board)) {
            winner = RESULT_INSUFF_MATERIAL;
        }

    } while (winner == RESULT_NONE);
    if (winner == RESULT_STALEMATE) {
        puts("Stalemate! Nobody wins!");
    } else {
        printf("Checkmate! the winner is: ");
        switch (winner) {
            case RESULT_WHITE_WINS: printf("White.\n"); break;
            case RESULT_BLACK_WINS: printf("Black.\n"); break;
            default: break;
        }
    }
    print_board(&board);
    return 0;
}