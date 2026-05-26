#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h> 

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
} Move;

typedef struct {
    uint64_t white_pieces;
    uint64_t black_pieces;
    uint64_t white_pawns;
    uint64_t black_pawns;
    uint64_t white_bishops;
    uint64_t black_bishops;
    uint64_t white_rooks;
    uint64_t black_rooks;
    uint64_t white_knights;
    uint64_t black_knights;
    uint64_t white_queens;
    uint64_t black_queens;
    uint64_t white_kings;
    uint64_t black_kings;
} Board;

void print_bitboard(uint64_t board);

#endif