#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

// input will be like "pe2e4" (piece, starting pos, ending pos)
// make it more explanatory in the future (simple for me rn)
char* player_input(char* buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return NULL;
    }

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return NULL;
    }

    // remove potential newline
    buffer[strcspn(buffer, "\n")] = '\0';

    return buffer;
}

int main(void) {

    Board board = {0};

    board.white_pieces |= (1ULL << 28);

    printf("This is the white_pieces in decimal: %lu\n", board.white_pieces);
    print_bitboard(board.white_pieces);
    puts("Please choose a starting location");
    char start[3];
    player_input(start, sizeof(start));
    printf("%s\n", start);
    return 0;
}