#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

#define RANK_OF(pos)            ((pos) / 8)
#define FILE_OF(pos)            ((pos) % 8)
#define DELTA(new, old)         (abs((new) - (old)))

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
typedef enum {RESULT_NONE, RESULT_WHITE_WINS, RESULT_BLACK_WINS, RESULT_STALEMATE, RESULT_INSUFF_MATERIAL, RESULT_REPETITION, RESULT_50_MOVE} GameResult;

#define OPP_COLOR(color)    (((color) == WHITE) ? BLACK : WHITE)
#define PROMOTION(piece, end)   ((piece) == PAWN && (RANK_OF(end) == 7 || RANK_OF(end) == 0))

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

extern uint64_t zobrist_pieces[COLOR_NUM][PIECE_NUM][64];
extern uint64_t zobrist_black_to_move;
extern uint64_t zobrist_castling[16];
extern uint64_t zobrist_ep_file[8];

typedef struct {
    int start;
    int end;
    PieceType piece;
    Color color;
    PieceType promotion;
    bool is_castling;
    bool is_enpassant;
} Move;

typedef struct {
    int enpassant_sq;
    int halfmove_clock;
    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;
    PieceType captured_piece;
} UndoState;

typedef struct {
    int score;
    int depth;
    Move best_move;
} PositionData;

#define MAX_MOVE_HISTORY 1024 // max moves in game

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
    int halfmove_clock;

    UndoState undo_history[MAX_MOVE_HISTORY];
    int undo_history_count;

    uint64_t current_zobrist_key;
    uint64_t zobrist_history[MAX_MOVE_HISTORY];
    int zobrist_history_count;
} Board;

#define MAX_MOVES 256

typedef struct {
    Move moves[MAX_MOVES];
    int count;
} MoveList;


Board init_board(void);
uint64_t xorshift(uint64_t *state);
void init_zobrist(void);
void init_zobrist_key(Board *board, Color side_to_move);
void init_attacks(void);
PieceType get_piece(Board* board, int sq, Color color);
bool is_castle_move(Move move);
bool is_enpassant(const Board* board, Move move);
void place_piece(Board *board, int sq, PieceType pt, Color color);
void remove_piece(Board *board, int sq, PieceType pt, Color color);
void move_piece(Board* board, Move move);
void reverse_move(Board *board, Move move);
void generate_legal_moves(Board* board, Color color, MoveList* list, bool filter_captures);
bool has_legal_moves(Board *board, Color color);
bool is_checkmate(Board *board, Color color);
bool is_stalemate(Board* board, Color color);
bool insufficient_material(Board* board);
bool is_repetition(Board *board);
void print_bitboard(uint64_t board);
void print_board(Board* board);

#endif