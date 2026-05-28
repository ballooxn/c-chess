#include "board.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MOVE_PIECE(bitboard, square)  ((bitboard) |= (1ULL << (square)))

uint64_t knight_attacks[64];
const int knight_offsets[8] = {
    17, // 2 (rank),1 (file)
    15, // 2,-1
    10, // 1,2
    6, //1,-2
    -17, // -2,-1
    -15, // -2,1
    -10, // -1,-2
    -6, // -1,2
};
uint64_t king_attacks[64];
const int king_offsets[8] = {
    8, // 1 (rank), 0 (file)
    9, // 1,1
    1, // 0,1
    -7, //-1,1
    -8, //-1,0
    -9,// -1, -1
    -1, // 0,-1
    7 // 1,-1
};
uint64_t pawn_attacks[2][64];
const int white_pawn_attack_offsets[2] = {
    9, // 1, 1
    7, // 1,-1
};
const  int black_pawn_attack_offsets[2] = {
    -7, // -1,1
    -9, // -1, -1
};
const int white_pawn_push_offsets[2] = {
    8, // 1, 0
    16, // 2, 0
};
const int black_pawn_push_offsets[2] = {
    -8, // -1,0
    -16, //-2,0
};


// bb = (black top, white bottom)
// a8 b8 c8 d8 e8 f8 g8 h8 > 56 to 63
// .....
// a1 b1 c1 d1 e1 f1 g1 h1 > 0 to 7

Board init_board(void) {
    Board board;
    board.pieces[WHITE][PAWN]   = 0x000000000000FF00ULL;
    board.pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    board.pieces[WHITE]][BISHOP] = 0x0000000000000024ULL;
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

void init_pawn_attacks(void) {

}

void init_knight_attacks(void) {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacks = 0;
    }
}

void init_king_attacks(void) {

}

void init_attacks(void) {

}

PieceType get_piece(Board board, int sq, Color color) {
    for (PieceType pt = PAWN; pt < PIECE_NUM; pt++) {
        if (get_bit(board.pieces[color][pt], sq)) {
            return pt;
        }
    }
    return NO_PIECE;
}

bool valid_move(Board board, PieceType piece, Move move) {



    return false;
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