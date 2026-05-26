#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define TO_BITS(rank, file)     ((rank) * 8 + (file))
#define MOVE_PIECE(bitboard, square)  ((bitboard) |= (1ULL << (square)))

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

typedef struct {
    int start;
    int end;
} Move;

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
bool valid_input(char* pos) {
    if (strlen(pos) != 2) return false;

    char file = pos[0];
    char rank = pos[1];

    bool valid = ((file >= 'a' && file <= 'h') && (rank >= '1' && rank <= '8'));
    return valid;
}

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

int file_to_int(char file) {
    return tolower(file) - 'a';
}

int rank_to_int(char rank) {
    return rank - '1';
}

int pos_to_int(char rank, char file) {
    int int_rank = rank_to_int(rank);
    int int_file = file_to_int(file);
    return TO_BITS(int_rank, int_file);
}

Move string_to_move(char* string) {
    int start = pos_to_int(string[1], string[0]);
    int end = pos_to_int(string[3], string[2]);
    Move move = {.start = start, .end = end};
    return move;
}

int main(void) {

    Board board = {0};

    MOVE_PIECE(board.white_pieces, 28);

    printf("This is the white_pieces in decimal: %lu\n", board.white_pieces);
    print_bitboard(board.white_pieces);
    puts("Please choose a start and end location");
    char move_string[5];
    char start[3] = "";
    char end[3] = "";
    while (!valid_input(start) || !valid_input(end)) {
        player_input(move_string, sizeof(move_string));
        if (strlen(move_string) != 4) continue;

        start[0] = move_string[0];
        start[1] = move_string[1];
        start[2] = '\0';
        end[0] = move_string[2];
        end[1] = move_string[3];
        end[2] = '\0';
    }
    Move move = string_to_move(move_string);
    printf("%d\n", move.start);
    printf("%d\n", move.end);
    return 0;
}