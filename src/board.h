#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

#define RANK_OF(pos)            ((pos) / 8)
#define FILE_OF(pos)            ((pos) % 8)
#define DELTA(new, old)    (abs((new) - (old)))

#define A1 0
#define C1 2
#define E1 4
#define G1 6
#define H1 7
#define A8 56
#define C8 58
#define E8 60
#define G8 62
#define H8 63

typedef enum {WHITE, BLACK, COLOR_NUM} Color;
typedef enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, ALL, PIECE_NUM, NO_PIECE = -1} PieceType;
typedef enum {RESULT_NONE, RESULT_WHITE_WINS, RESULT_BLACK_WINS, RESULT_STALEMATE, RESULT_INSUFF_MATERIAL} GameResult;

#define OPP_COLOR(color)    (((color) == WHITE) ? BLACK : WHITE)

inline void set_bit(uint64_t* bb, int sq) {
    *bb |= (1ULL << sq);
}
inline void clear_bit(uint64_t* bb, int sq) {
    *bb &= ~(1ULL << sq);
}
inline bool get_bit(uint64_t bb, int sq) {
    return (bb & (1ULL << sq)) != 0;
}

extern uint64_t knight_attacks[64];
extern uint64_t king_attacks[64];
extern uint64_t white_pawn_pushes[64];
extern uint64_t white_pawn_attacks[64];
extern uint64_t black_pawn_pushes[64];
extern uint64_t black_pawn_attacks[64];

extern uint64_t line[64][64];
extern uint64_t between[64][64];

typedef struct {
    uint64_t pieces[COLOR_NUM][PIECE_NUM];  // one for each piece/all pieces
    uint64_t occupied;
    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;
    int last_double_push;
    int enpassant_sq;
} Board;

typedef struct {
    int start;
    int end;
    PieceType piece;
    Color color;
} Move;

Board init_board(void);
void init_attacks(void);
PieceType get_piece(Board* board, int sq, Color color);
void move_piece(Board* board, Move move, bool about_to_reverse);
void reverse_simulated_move(Board *board, Move move, PieceType target_piece);
void promote_pawn(Board* board, int sq, char promo_char, Color color);
bool is_enpassant(const Board* board, Move move);
bool valid_move(Board *board, Move move);
bool in_check(Board *board, Color color);
bool is_legal(Board* board, Move move);
bool has_legal_moves(Board *board, Color color);
bool is_checkmate(Board *board, Color color);
bool is_stalemate(Board* board, Color color);
bool insufficient_material(Board* board);
void print_bitboard(uint64_t board);
void print_board(Board* board);

#endif