#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// bb = (black top, white bottom)
// a8 b8 c8 d8 e8 f8 g8 h8 > 56 to 63
// .....
// a1 b1 c1 d1 e1 f1 g1 h1 > 0 to 7

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

int main(void) {

    Board board = {0};

    board.white_pieces |= (1ULL << 28);

    printf("Hello\n");
    printf("This is the white_pieces in decimal: %lu\n", board.white_pieces);
    return 0;
}