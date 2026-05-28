#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {WHITE, BLACK, COLOR_NUM} Color;
typedef enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, ALL, PIECE_NUM, NO_PIECE = -1} PieceType;

static inline void set_bit(uint64_t* bb, int sq) {
    *bb |= (1ULL << sq);
}
static inline void clear_bit(uint64_t* bb, int sq) {
    *bb &= ~(1ULL << sq);
}
static inline bool get_bit(uint64_t bb, int sq) {
    return (bb & (1ULL << sq)) != 0;
}

typedef struct {
    int start;
    int end;
    char piece;
} Move;

typedef struct {
    uint64_t pieces[COLOR_NUM][PIECE_NUM];  // one for each piece/all pieces
} Board;

void print_bitboard(uint64_t board);
Board init_board(void);
bool is_legal(Board board, Move move, Color color);
PieceType get_piece(Board board, int sq, Color color);

#endif