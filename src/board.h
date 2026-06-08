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
typedef enum {RESULT_NONE, RESULT_WHITE_WINS, RESULT_BLACK_WINS, RESULT_STALEMATE, RESULT_INSUFF_MATERIAL, RESULT_REPETITION} GameResult;

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
    int start;
    int end;
    PieceType piece;
    Color color;
    PieceType engine_promotion;
    bool is_castling;
    bool is_enpassant;
} Move;

typedef struct {
    int enpassant_sq;
    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;
    PieceType captured_piece;
} UndoState;

#define MAX_DEPTH 200 // max moves in game

typedef struct {
    uint64_t pieces[COLOR_NUM][PIECE_NUM];  // one for each piece/all pieces
    uint64_t occupied;
    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;
    int enpassant_sq;
    Move last_white_move;
    Move last_black_move;
    Move second_last_black_move;
    int repetition_count;

    UndoState history[MAX_DEPTH];
    int history_count;
} Board;

#define MAX_MOVES 256

typedef struct {
    Move moves[MAX_MOVES];
    int count;
} MoveList;

Board init_board(void);
void init_attacks(void);
PieceType get_piece(Board* board, int sq, Color color);
bool is_castle_move(Move move);
bool is_enpassant(const Board* board, Move move);
void place_piece(Board *board, int sq, PieceType pt, Color color);
void remove_piece(Board *board, int sq, PieceType pt, Color color);
void move_piece(Board* board, Move move);
void reverse_move(Board *board, Move move);
void promote_pawn(Board* board, int sq, char promo_char, Color color);
void generate_legal_moves(Board* board, Color color, MoveList* list);
bool has_legal_moves(Board *board, Color color);
bool is_checkmate(Board *board, Color color);
bool is_stalemate(Board* board, Color color);
bool insufficient_material(Board* board);
bool three_move_insufficient(Board* board, Move move);
void print_bitboard(uint64_t board);
void print_board(Board* board);

#endif