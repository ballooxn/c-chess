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
uint64_t white_pawn_pushes[64];
uint64_t black_pawn_pushes[64];
uint64_t white_pawn_attacks[64];
uint64_t black_pawn_attacks[64];
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

uint64_t line[64][64];
uint64_t between[64][64]; // First is the start_pos, second is the end_pos

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
    board.occupied = board.pieces[BLACK][ALL] | board.pieces[WHITE][ALL];
    return board;
}

typedef struct {
    uint64_t *table;
    const int *offsets;
    bool is_attack;
} PawnInit;

void init_pawn_attacks(void) {

    PawnInit configs[4] = {
        {white_pawn_pushes,  white_pawn_push_offsets,   false},
        {black_pawn_pushes,  black_pawn_push_offsets,   false},
        {white_pawn_attacks, white_pawn_attack_offsets, true},
        {black_pawn_attacks, black_pawn_attack_offsets, true}
    };

    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8;
        int file = sq % 8;

        for (int config = 0; config < 4; config++) {
            uint64_t attacks = 0;

            for (int i = 0; i < 2; i++) {
                int target = sq + configs[config].offsets[i];
                int t_rank = target / 8;
                int t_file = target % 8;
                if (target >= 0 && target < 64) {
                    if (configs[config].is_attack) {
                        if (abs(t_rank - rank) == 1 && abs(t_file - file) == 1)
                            set_bit(&attacks, target);
                    } else {
                        if ((abs(t_rank - rank) == 1 || abs(t_rank - rank) == 2) && t_file == file)
                            set_bit(&attacks, target);
                    }
                }
            }
            configs[config].table[sq] = attacks;
        }
    }
}

void init_knight_attacks(void) {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacks = 0;
        int rank = sq / 8;
        int file = sq % 8;
        for (int i = 0; i < 8; i++) {
            int target = sq + knight_offsets[i];
            int t_rank = target / 8;
            int t_file = target % 8;
            if (target >= 0 && target < 64 &&
                ((abs(t_rank - rank) == 2 && abs(t_file - file) == 1) ||
                (abs(t_rank - rank) == 1 && abs(t_file - file) == 2))) {
                set_bit(&attacks, target);
            }
        }
        knight_attacks[sq] = attacks;
    }
}

void init_king_attacks(void) {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacks = 0;
        int rank = sq / 8;
        int file = sq % 8;
        for (int i = 0; i < 8; i++) {
            int target = sq + king_offsets[i];
            int t_rank = target / 8;
            int t_file = target % 8;
            if (target >= 0 && target < 64 && target != sq &&
                ((abs(t_rank - rank) == 1 && abs(t_file - file) == 1) ||
                (abs(t_rank - rank) == 0 || abs(t_file - file) == 0))) {
                set_bit(&attacks, target);
                // do castling stuff later
            }
        }
        king_attacks[sq] = attacks;
    }
}

void init_sliding_tables(void) {
    const int dirs_rank[8] = { 1, -1,  0,  0,  1,  1, -1, -1 };
    const int dirs_file[8] = { 0,  0,  1, -1,  1, -1,  1, -1 };
    // N,S,E,W,NE,NW,SE,SW

    for (int sq1 = 0; sq1 < 64; sq1++) {
        int rank1 = sq1 / 8; 
        int file1 = sq1 % 8;

        for (int dir = 0; dir < 8; dir++) {
            int offset_rank = dirs_rank[dir];
            int offset_file = dirs_file[dir];

            // build whole ray (line)
            uint64_t ray = 0;
            int rank2 = rank1 + offset_rank;
            int file2 = file1 + offset_file;
            while (rank2 >= 0 && rank2 < 8 && file2 >= 0 && file2 < 8) {
                int sq2 = rank2 * 8 + file2;
                set_bit(&ray, sq2);
                rank2 += offset_rank;
                file2 += offset_file;
            }

            // build between
            rank2 = rank1 + offset_rank;
            file2 = file1 + offset_file;
            uint64_t between_mask = 0;
            while (rank2 >= 0 && rank2 < 8 && file2 >= 0 && file2 < 8) {
                int sq2 = rank2 * 8 + file2;
                line[sq1][sq2] = (1ULL << sq1) | (1ULL << sq2) | between_mask; //include starting pos
                between[sq1][sq2] = between_mask;
                set_bit(&between_mask, sq2);
                rank2 += offset_rank;
                file2 += offset_file;
            }
        }
    }
}

void init_attacks(void) {
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_sliding_tables();
}

PieceType get_piece(Board board, int sq, Color color) {
    for (PieceType pt = PAWN; pt < PIECE_NUM; pt++) {
        if (get_bit(board.pieces[color][pt], sq)) {
            return pt;
        }
    }
    return NO_PIECE;
}

void move_piece(Board* board, Move move, Color color) {
    PieceType piece = get_piece(*board, move.start, color);
    Color opp = (color == WHITE) ? BLACK : WHITE;
    PieceType target_piece = get_piece(*board, move.end, opp);
    clear_bit(&board->pieces[color][piece], move.start);
    clear_bit(&board->occupied, move.start);
    clear_bit(&board->pieces[color][ALL], move.start);
    if (target_piece != NO_PIECE) {
        clear_bit(&board->pieces[opp][target_piece], move.end);
        clear_bit(&board->pieces[opp][ALL], move.end);
    }
    set_bit(&board->pieces[color][piece], move.end);
    set_bit(&board->pieces[color][ALL], move.end);
    set_bit(&board->occupied, move.end);
}

bool valid_move(Board board, PieceType piece, Move move, Color color) {
    bool pawn_double_push = (piece == PAWN && abs((move.start / 8) - (move.end / 8)) == 2);
    if (piece == PAWN && !pawn_double_push) {
        switch (color) {
            case WHITE: 
                if (get_bit(white_pawn_attacks[move.start], move.end) && get_bit(board.pieces[BLACK][ALL], move.end)) return true;
                if (get_bit(white_pawn_pushes[move.start], move.end) && !get_bit(board.occupied, move.end)) return true;
                break;
            case BLACK:
                if (get_bit(black_pawn_attacks[move.start], move.end) && get_bit(board.pieces[WHITE][ALL], move.end)) return true;
                if (get_bit(black_pawn_pushes[move.start], move.end) && !get_bit(board.occupied, move.end)) return true;
                break;
            default:
                return false;
        }
    } else if (piece == KNIGHT && get_bit(knight_attacks[move.start], move.end)) {
        return true;
    } else if (piece == KING && get_bit(king_attacks[move.start], move.end)) {
        return true;
    } else if (piece == QUEEN || piece == BISHOP || piece == ROOK || pawn_double_push) {
        // handle sliding piece
        if (line[move.start][move.end] == 0) return false;
        if (between[move.start][move.end] & board.occupied) return false;

        if (pawn_double_push) return true;

        int delta_rank = abs((move.start / 8) - (move.end / 8));
        int delta_file = abs((move.start % 8) - (move.end % 8));
        if (piece == ROOK && !(delta_rank == 0 || delta_file == 0)) return false;
        if (piece == BISHOP && delta_rank != delta_file) return false;

        return true;
    } else {
        puts("YIKES!");
    }

    return false;
}

bool is_legal(Board board, Move move, Color color) {
    if (get_bit(board.pieces[color][ALL], move.end)) return false;
    PieceType piece = get_piece(board, move.start, color);

    if (piece == NO_PIECE) return false;

    if (!valid_move(board, piece, move, color)) return false;

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

char* piece_symbols[2][6] = {
        [WHITE] = { "♟", "♞", "♝", "♜", "♛", "♚" },
        [BLACK] = { "♙", "♘", "♗", "♖", "♕", "♔" }
    };

void print_board(Board* board) {
    char* display_board[8][8];

    for (int r = 0; r < 8; r++)
        for (int f = 0; f < 8; f++)
            display_board[r][f] = ".";

    for (Color color = WHITE; color < COLOR_NUM; color++) {
        for (PieceType pt = PAWN; pt <= KING; pt++) {
            for (int i = 0; i < 64; i++) {
                if (get_bit(board->pieces[color][pt], i)) {
                    display_board[i / 8][i % 8] = piece_symbols[color][pt];
                }
            }
        }
    }

    for (int row = 7; row >= 0; row--) {
        printf("%d |", row + 1);
        for (int col = 0; col < 8; col++) {
            printf(" %s", display_board[row][col]);
        };
        printf(" |\n");
    };
    printf("    a b c d e f g h\n");
}