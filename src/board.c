#include "board.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MOVE_PIECE(bitboard, square)  ((bitboard) |= (1ULL << (square)))

// bb = (black top, white bottom)
// a8 b8 c8 d8 e8 f8 g8 h8 > 56 to 63
// .....
// a1 b1 c1 d1 e1 f1 g1 h1 > 0 to 7

Board init_board(void) {
    Board board;
    board.white_pawns   = 0x000000000000FF00ULL;
    board.black_pawns   = 0x00FF000000000000ULL;
    board.white_bishops = 0x0000000000000024ULL;
    board.black_bishops = 0x2400000000000000ULL;
    board.white_rooks   = 0x0000000000000081ULL;
    board.black_rooks   = 0x8100000000000000ULL;
    board.white_knights = 0x0000000000000042ULL;
    board.black_knights = 0x4200000000000000ULL;
    board.white_queens  = 0x0000000000000008ULL;
    board.black_queens  = 0x0800000000000000ULL;
    board.white_kings   = 0x0000000000000010ULL;
    board.black_kings   = 0x1000000000000000ULL;

    board.white_pieces = board.white_pawns | board.white_knights |
                         board.white_bishops | board.white_rooks |
                         board.white_queens | board.white_kings;
    board.black_pieces = board.black_pawns | board.black_knights |
                         board.black_bishops | board.black_rooks |
                         board.black_queens | board.black_kings;
    return board;
}

void print_bitboard(uint64_t board) {
    for (int row = 7; row >= 0; row--) {
        printf("%d |", row + 1);
        for (int col = 0; col < 8; col++) {
            int square = row * 8 + col;
            char symbol = (board & (1ULL << square)) ? '1' : '0';
            printf(" %c", symbol);
        };
        printf(" |\n");
    };
    printf("    a b c d e f g h\n");
}