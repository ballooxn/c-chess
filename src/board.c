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