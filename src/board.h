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

// For extremely fast lookup, have each piece have an array of the board.
// so 64 squares. Loop through every square (do this at startup)
// and set_bit for each possible move from that starting square.
// aka lookup table

extern uint64_t knight_attacks[64];
extern uint64_t king_attacks[64];
extern uint64_t white_pawn_pushes[64];
extern uint64_t white_pawn_attacks[64];
extern uint64_t black_pawn_pushes[64];
extern uint64_t black_pawn_attacks[64];

extern uint64_t line[64][64];
extern uint64_t between[64][64];

typedef struct {
    int start;
    int end;
    char piece;
} Move;

typedef struct {
    uint64_t pieces[COLOR_NUM][PIECE_NUM];  // one for each piece/all pieces
    uint64_t occupied;
} Board;

Board init_board(void);
void init_attacks(void);
void print_bitboard(uint64_t board);
bool is_legal(Board board, Move move, Color color);
PieceType get_piece(Board board, int sq, Color color);

#endif