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
    board.pieces[WHITE][PAWN]   = 0x000000000000FF00ULL;
    board.pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    board.pieces[WHITE][BISHOP] = 0x0000000000000024ULL;
    board.pieces[WHITE][ROOK]   = 0x0000000000000081ULL;
    board.pieces[WHITE][QUEEN]  = 0x0000000000000008ULL;
    board.pieces[WHITE][KING]   = 0x0000000000000010ULL;

    board.pieces[BLACK][PAWN]   = 0x00FF000000000000ULL;
    board.pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
    board.pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
    board.pieces[BLACK][ROOK]   = 0x8100000000000000ULL;
    board.pieces[BLACK][QUEEN]  = 0x0800000000000000ULL;
    board.pieces[BLACK][KING]   = 0x1000000000000000ULL;

    board.pieces[WHITE][ALL] =  board.pieces[WHITE][PAWN] | board.pieces[WHITE][KNIGHT] |
                                board.pieces[WHITE][BISHOP] | board.pieces[WHITE][ROOK] |
                                board.pieces[WHITE][QUEEN] | board.pieces[WHITE][KING];
    board.pieces[BLACK][ALL] =  board.pieces[BLACK][PAWN] | board.pieces[BLACK][KNIGHT] |
                                board.pieces[BLACK][BISHOP] | board.pieces[BLACK][ROOK] |
                                board.pieces[BLACK][QUEEN] | board.pieces[BLACK][KING];
    return board;
}

PieceType get_piece(Board board, int sq, Color color) {
    for (PieceType pt = PAWN; pt < PIECE_NUM; pt++) {
        if (get_bit(board.pieces[color][pt], sq)) {
            return pt;
        }
    }
    return NO_PIECE;
}

bool is_legal(Board board, Move move, Color color) {
    if (get_bit(board.pieces[color][ALL], move.end)) return false;

    return true;
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