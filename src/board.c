#include "board.h"
#include "move_parser.h"
#include "validator.h"
#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SQUARES 64

uint64_t zobrist_pieces[COLOR_NUM][PIECE_NUM][64];
uint64_t zobrist_black_to_move;
uint64_t zobrist_castling[16];
uint64_t zobrist_ep_file[8];

uint64_t knight_attacks[SQUARES];
const int knight_offsets[8] = {
    17,  // 2 (rank),1 (file)
    15,  // 2,-1
    10,  // 1,2
    6,   // 1,-2
    -17, // -2,-1
    -15, // -2,1
    -10, // -1,-2
    -6,  // -1,2
};
uint64_t king_attacks[SQUARES];
const int king_offsets[8] = {
    8,  // 1 (rank), 0 (file)
    9,  // 1,1
    1,  // 0,1
    -7, //-1,1
    -8, //-1,0
    -9, // -1, -1
    -1, // 0,-1
    7   // 1,-1
};
uint64_t white_pawn_pushes[SQUARES];
uint64_t black_pawn_pushes[SQUARES];
uint64_t white_pawn_attacks[SQUARES];
uint64_t black_pawn_attacks[SQUARES];

uint64_t line[SQUARES][SQUARES];
uint64_t between[SQUARES][SQUARES]; // First is the start_pos, second is the end_pos

// bb = (black top, white bottom)
// a8 b8 c8 d8 e8 f8 g8 h8 > 56 to 63
// .....
// a1 b1 c1 d1 e1 f1 g1 h1 > 0 to 7

Board init_board(void)
{
    Board board;
    board.pieces[WHITE][PAWN] = 0x000000000000FF00ULL;
    board.pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    board.pieces[WHITE][BISHOP] = 0x0000000000000024ULL;
    board.pieces[WHITE][ROOK] = 0x0000000000000081ULL;
    board.pieces[WHITE][QUEEN] = 0x0000000000000008ULL;
    board.pieces[WHITE][KING] = 0x0000000000000010ULL;

    board.pieces[BLACK][PAWN] = 0x00FF000000000000ULL;
    board.pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
    board.pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
    board.pieces[BLACK][ROOK] = 0x8100000000000000ULL;
    board.pieces[BLACK][QUEEN] = 0x0800000000000000ULL;
    board.pieces[BLACK][KING] = 0x1000000000000000ULL;

    board.pieces[WHITE][ALL] = board.pieces[WHITE][PAWN] | board.pieces[WHITE][KNIGHT] |
                               board.pieces[WHITE][BISHOP] | board.pieces[WHITE][ROOK] |
                               board.pieces[WHITE][QUEEN] | board.pieces[WHITE][KING];
    board.pieces[BLACK][ALL] = board.pieces[BLACK][PAWN] | board.pieces[BLACK][KNIGHT] |
                               board.pieces[BLACK][BISHOP] | board.pieces[BLACK][ROOK] |
                               board.pieces[BLACK][QUEEN] | board.pieces[BLACK][KING];
    board.occupied = board.pieces[BLACK][ALL] | board.pieces[WHITE][ALL];

    board.white_can_castle_kingside = true;
    board.white_can_castle_queenside = true;
    board.black_can_castle_kingside= true;
    board.black_can_castle_queenside = true;
    board.enpassant_sq = 100;
    board.halfmove_clock = 0;
    board.undo_history_count = 0;
    board.zobrist_history_count = 0;
    return board;
}

uint64_t xorshift(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

void init_zobrist(void) {
    uint64_t state = 123456789ULL;

    for (int color = 0; color < 2; color++) {
        for (int pt = 0; pt < 6; pt++) {
            for (int sq = 0; sq < 64; sq++) {
                zobrist_pieces[color][pt][sq] = xorshift(&state);
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        zobrist_castling[i] = xorshift(&state);
    }
    for (int i = 0; i < 8; i++) {
        zobrist_ep_file[i] = xorshift(&state);
    }
    zobrist_black_to_move = xorshift(&state);
}

void init_zobrist_key(Board *board, Color side_to_move) {
    board->current_zobrist_key = 0;

    for (Color color = WHITE; color <= BLACK; color++) {
        for (PieceType pt = PAWN; pt <= KING; pt++) {
            uint64_t bb = board->pieces[color][pt];
            while (bb) {
                int sq = __builtin_ctzll(bb);
                bb &= bb - 1;

                board->current_zobrist_key ^= zobrist_pieces[color][pt][sq];
            }
        }
    }

    int castling_rights =   (board->white_can_castle_kingside << 0) |
                            (board->white_can_castle_queenside << 1) |
                            (board->black_can_castle_kingside << 2) |
                            (board->black_can_castle_queenside << 3);
    board->current_zobrist_key ^= zobrist_castling[castling_rights];

    if (board->enpassant_sq != 100) {
        int file = FILE_OF(board->enpassant_sq);
        board->current_zobrist_key ^= zobrist_ep_file[file];
    }
    if (side_to_move == BLACK) {
        board->current_zobrist_key ^= zobrist_black_to_move;
    }

    board->zobrist_history[0] = board->current_zobrist_key;
    board->zobrist_history_count = 1;
}

static void init_pawn_attacks(void) {
    for (int sq = 0; sq < SQUARES; sq++) {
        int rank = RANK_OF(sq);
        int file = FILE_OF(sq);

        uint64_t white_attacks = 0;
        if (rank < 7) {
            if (file > 0) set_bit(&white_attacks, sq+7);
            if (file < 7) set_bit(&white_attacks, sq+9);
        }
        white_pawn_attacks[sq] = white_attacks;

        uint64_t black_attacks = 0;
        if (rank > 0) {
            if (file > 0) set_bit(&black_attacks, sq-9);
            if (file < 7) set_bit(&black_attacks, sq-7);
        }
        black_pawn_attacks[sq] = black_attacks;

        uint64_t white_pushes = 0;
        if (rank < 7) {
            set_bit(&white_pushes, sq+8);
            if (rank == 1) set_bit(&white_pushes, sq+16);
        }
        white_pawn_pushes[sq] = white_pushes;

        uint64_t black_pushes = 0;
        if (rank > 0) {
            set_bit(&black_pushes, sq-8);
            if (rank == 6) set_bit(&black_pushes, sq-16);
        }
        black_pawn_pushes[sq] = black_pushes;
    }
}

static void init_knight_attacks(void)
{
    for (int sq = 0; sq < SQUARES; sq++)
    {
        uint64_t attacks = 0;
        int rank = RANK_OF(sq);
        int file = FILE_OF(sq);
        for (int i = 0; i < 8; i++)
        {
            int target = sq + knight_offsets[i];
            int t_rank = RANK_OF(target);
            int t_file = FILE_OF(target);
            if (target >= 0 && target < SQUARES &&
                ((DELTA(t_rank, rank) == 2 && DELTA(t_file, file) == 1) ||
                 (DELTA(t_rank, rank) == 1 && DELTA(t_file, file) == 2)))
            {
                set_bit(&attacks, target);
            }
        }
        knight_attacks[sq] = attacks;
    }
}

static void init_king_attacks(void)
{
    for (int sq = 0; sq < SQUARES; sq++)
    {
        uint64_t attacks = 0;
        int rank = RANK_OF(sq);
        int file = FILE_OF(sq);
        for (int i = 0; i < 8; i++)
        {
            int target = sq + king_offsets[i];
            int delta_rank = DELTA(RANK_OF(target), rank);
            int delta_file = DELTA(FILE_OF(target), file);
            if (target >= 0 && target < SQUARES && target != sq &&
                ((delta_rank == 1 && delta_file == 0) || 
                (delta_rank == 0 && delta_file == 1) || 
                (delta_rank == 1 && delta_file == 1))) {
                set_bit(&attacks, target);
            }
        }
        king_attacks[sq] = attacks;
    }
}

static void init_sliding_tables(void)
{
    const int dirs_rank[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    const int dirs_file[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    // N,S,E,W,NE,NW,SE,SW

    for (int sq = 0; sq < SQUARES; sq++)
    {
        int start_rank = RANK_OF(sq);
        int start_file = FILE_OF(sq);

        for (int dir = 0; dir < 8; dir++)
        {
            int offset_rank = dirs_rank[dir];
            int offset_file = dirs_file[dir];

            // build whole ray (line)
            uint64_t ray = 0;
            int end_rank = start_rank + offset_rank;
            int end_file = start_file + offset_file;
            while (end_rank >= 0 && end_rank < 8 && end_file >= 0 && end_file < 8)
            {
                int end_sq = end_rank * 8 + end_file;
                set_bit(&ray, end_sq);
                end_rank += offset_rank;
                end_file += offset_file;
            }

            // build between
            end_rank = start_rank + offset_rank;
            end_file = start_file + offset_file;
            uint64_t between_mask = 0;
            while (end_rank >= 0 && end_rank < 8 && end_file >= 0 && end_file < 8)
            {
                int end_sq = end_rank * 8 + end_file;
                line[sq][end_sq] = (1ULL << sq) | (1ULL << end_sq) | between_mask; // include starting pos
                between[sq][end_sq] = between_mask;
                set_bit(&between_mask, end_sq);
                end_rank += offset_rank;
                end_file += offset_file;
            }
        }
    }
}

void init_attacks(void)
{
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_sliding_tables();
}

void place_piece(Board *board, int sq, PieceType pt, Color color)
{
    set_bit(&board->pieces[color][pt], sq);
    set_bit(&board->pieces[color][ALL], sq);
    set_bit(&board->occupied, sq);
}

void remove_piece(Board *board, int sq, PieceType pt, Color color)
{
    clear_bit(&board->pieces[color][pt], sq);
    clear_bit(&board->occupied, sq);
    clear_bit(&board->pieces[color][ALL], sq);
}

PieceType get_piece(Board* board, int sq, Color color) {
    for (PieceType pt = PAWN; pt <= KING; pt++)
    {
        if (get_bit(board->pieces[color][pt], sq))
        {
            return pt;
        }
    }
    return NO_PIECE;
}

bool is_castle_move(Move move) {
    if (move.piece != KING) return false;
    int rank_delta = DELTA(RANK_OF(move.start), RANK_OF(move.end));
    int file_delta = DELTA(FILE_OF(move.start), FILE_OF(move.end));
    return (rank_delta == 0 && file_delta == 2);
}

bool is_enpassant(const Board* board, Move move) {
    if (move.piece != PAWN) return false;
    if (move.end != board->enpassant_sq) return false;

    int file_diff = FILE_OF(move.end) - FILE_OF(move.start);
    if (abs(file_diff) != 1) return false;
    if (DELTA(RANK_OF(move.end), RANK_OF(move.start)) != (move.color == WHITE ? 1 : -1)) return false;

    int side_pawn_sq = move.start + (file_diff > 0 ? 1 : -1);
    if (!get_bit(board->pieces[OPP_COLOR(move.color)][PAWN], side_pawn_sq)) return false;

    return true;
}

static void move_castle_rook(Board *board, Move king_move, bool reversing) {
    int rank = RANK_OF(king_move.start);
    bool kingside = FILE_OF(king_move.end) == 6;
    int rook_start = kingside ? (rank * 8 + 7) : (rank * 8 + 0);
    int rook_end = kingside ? (rank * 8 + 5) : (rank * 8 + 3);

    if (reversing) {
        remove_piece(board, rook_end, ROOK, king_move.color);
        place_piece(board, rook_start, ROOK, king_move.color);
    } else {
        board->current_zobrist_key ^= zobrist_pieces[king_move.color][ROOK][rook_end];
        remove_piece(board, rook_start, ROOK, king_move.color);
        board->current_zobrist_key ^= zobrist_pieces[king_move.color][ROOK][rook_start];
        place_piece(board, rook_end, ROOK, king_move.color);
    }
}

void move_piece(Board *board, Move move)
{
    int count = board->undo_history_count;

    board->undo_history[count].halfmove_clock = board->halfmove_clock;
    board->undo_history[count].enpassant_sq = board->enpassant_sq;
    board->undo_history[count].white_can_castle_kingside = board->white_can_castle_kingside;
    board->undo_history[count].white_can_castle_queenside = board->white_can_castle_queenside;
    board->undo_history[count].black_can_castle_kingside = board->black_can_castle_kingside;
    board->undo_history[count].black_can_castle_queenside = board->black_can_castle_queenside;

    int old_castling =  (board->white_can_castle_kingside << 0) |
                        (board->white_can_castle_queenside << 1) |
                        (board->black_can_castle_kingside << 2) |
                        (board->black_can_castle_queenside << 3);
    board->current_zobrist_key ^= zobrist_castling[old_castling];
    
    Color opp = OPP_COLOR(move.color);
    PieceType target_piece = get_piece(board, move.end, opp);
    board->undo_history[count].captured_piece = target_piece;
    board->halfmove_clock++;

    if (target_piece != NO_PIECE || move.piece == PAWN) board->halfmove_clock = 0;

    board->current_zobrist_key ^= zobrist_pieces[move.color][move.piece][move.start];
    remove_piece(board, move.start, move.piece, move.color);
    if (target_piece != NO_PIECE) {
        board->current_zobrist_key ^= zobrist_pieces[opp][target_piece][move.end];
        remove_piece(board, move.end, target_piece, opp);
    }
    if (move.is_enpassant) {
        int side_dir = (FILE_OF(move.end) > FILE_OF(move.start)) ? 1 : -1;
        int side_pawn = move.start + side_dir;
        board->current_zobrist_key ^= zobrist_pieces[opp][PAWN][side_pawn];
        remove_piece(board, side_pawn, PAWN, opp);
    }
    PieceType piece_to_place = (move.promotion != NO_PIECE) ? move.promotion : move.piece;
    board->current_zobrist_key ^= zobrist_pieces[move.color][piece_to_place][move.end];
    place_piece(board, move.end, piece_to_place, move.color);

    if (move.is_castling) move_castle_rook(board, move, false);

    if (move.piece == PAWN && DELTA(RANK_OF(move.end), RANK_OF(move.start)) == 2) {
        board->enpassant_sq = move.end + (move.color == WHITE ? -8 : 8);
        if (board->undo_history[count].enpassant_sq != 100) {
            board->current_zobrist_key ^= zobrist_ep_file[FILE_OF(board->undo_history[count].enpassant_sq)];
        }
        board->current_zobrist_key ^= zobrist_ep_file[FILE_OF(move.start)];
    } else {
        board->enpassant_sq = 100;
        if (board->undo_history[count].enpassant_sq != 100) {
            board->current_zobrist_key ^= zobrist_ep_file[FILE_OF(board->undo_history[count].enpassant_sq)];
        }
    }

    if (move.piece == KING) {
        if (move.color == WHITE) {
            board->white_can_castle_kingside  = false;
            board->white_can_castle_queenside = false;
        } else {
            board->black_can_castle_kingside  = false;
            board->black_can_castle_queenside = false;
        }
    }
    if (move.piece == ROOK) {
        if (move.color == WHITE) {
            if (move.start == H1) board->white_can_castle_kingside  = false;
            if (move.start == A1) board->white_can_castle_queenside = false;
        } else {
            if (move.start == H8) board->black_can_castle_kingside  = false;
            if (move.start == A8) board->black_can_castle_queenside = false;
        }
    }
    if (target_piece == ROOK) {
        if (opp == WHITE) {
            if (move.end == H1) board->white_can_castle_kingside = false;
            if (move.end == A1) board->white_can_castle_queenside = false;
        } else {
            if (move.end == H8) board->black_can_castle_kingside = false;
            if (move.end == A8) board->black_can_castle_queenside = false;
        }
    }

    int new_castling =  (board->white_can_castle_kingside << 0) |
                        (board->white_can_castle_queenside << 1) |
                        (board->black_can_castle_kingside << 2) |
                        (board->black_can_castle_queenside << 3);
    board->current_zobrist_key ^= zobrist_castling[new_castling];

    board->current_zobrist_key ^= zobrist_black_to_move;
    board->zobrist_history[board->zobrist_history_count] = board->current_zobrist_key;
    board->zobrist_history_count++;
    board->undo_history_count++;
}

void reverse_move(Board *board, Move move) {
    board->undo_history_count--;
    int count = board->undo_history_count;

    board->zobrist_history_count--;
    board->current_zobrist_key = board->zobrist_history[board->zobrist_history_count - 1];
    
    Color opp = OPP_COLOR(move.color);

    PieceType piece_to_remove = (move.promotion != NO_PIECE) ? move.promotion : move.piece;
    remove_piece(board, move.end, piece_to_remove, move.color);
    place_piece(board, move.start, move.piece, move.color);

    PieceType target_piece = board->undo_history[count].captured_piece;
    if (target_piece != NO_PIECE) {
        place_piece(board, move.end, target_piece, opp);
    }
    
    if (move.is_castling) move_castle_rook(board, move, true); 

    if (move.is_enpassant)  {
        int side_dir = (FILE_OF(move.end) > FILE_OF(move.start)) ? 1 : -1;
        int side_pawn = move.start + side_dir;
        place_piece(board, side_pawn, PAWN, opp);
    }
    board->halfmove_clock = board->undo_history[count].halfmove_clock;
    board->enpassant_sq = board->undo_history[count].enpassant_sq;
    board->white_can_castle_kingside = board->undo_history[count].white_can_castle_kingside;
    board->white_can_castle_queenside = board->undo_history[count].white_can_castle_queenside;
    board->black_can_castle_kingside = board->undo_history[count].black_can_castle_kingside;
    board->black_can_castle_queenside = board->undo_history[count].black_can_castle_queenside;
}

static uint64_t * const pawn_pushes[2] = {white_pawn_pushes, black_pawn_pushes};
static uint64_t * const pawn_attacks[2] = {white_pawn_attacks, black_pawn_attacks};

int generate_pawn_moves(Board* board, Color color, int sq, int* possible_end_sqs, PieceType pt) {
    (void)pt;
    int count = 0;
    
    uint64_t push_bb = pawn_pushes[color][sq];
    while (push_bb) {
        int end_sq = __builtin_ctzll(push_bb);
        push_bb &= push_bb - 1;
        
        if (get_bit(board->occupied, end_sq)) continue;
        
        if (DELTA(RANK_OF(end_sq), RANK_OF(sq)) == 2) {
            int inter_sq = (color == WHITE) ? sq + 8 : sq - 8;
            if (get_bit(board->occupied, inter_sq)) continue;
        }
        possible_end_sqs[count++] = end_sq;
    }
    uint64_t attack_bb = pawn_attacks[color][sq];
    while (attack_bb) {
        int end_sq = __builtin_ctzll(attack_bb);
        attack_bb &= attack_bb - 1;

        if (get_bit(board->pieces[OPP_COLOR(color)][ALL], end_sq) ||
            end_sq == board->enpassant_sq) {
            possible_end_sqs[count++] = end_sq;
        }
    }
    
    return count;
}

static int generate_castling_moves(Board* board, Color color, int king_sq, int* possible_end_sqs) {
    if ((color == WHITE && king_sq != 4) || (color == BLACK && king_sq != 60)) return 0;

    int count = 0;
    if (can_castle(board, color, true)) { //kingside
        possible_end_sqs[count++] = (color == WHITE) ? G1 : G8;
    }
    if (can_castle(board, color, false)) { //queenside
        possible_end_sqs[count++] = (color == WHITE) ? C1 : C8;
    }
    return count;
}

int generate_knight_king_moves(Board* board, Color color, int sq, int* possible_end_sqs, PieceType pt) {
    int count = 0;
    
    uint64_t attack_bb = (pt == KING) ? king_attacks[sq] : knight_attacks[sq];
    while (attack_bb) {
        int end_sq = __builtin_ctzll(attack_bb);
        attack_bb &= attack_bb - 1;

        if (!get_bit(board->pieces[color][ALL], end_sq)) {
            possible_end_sqs[count++] = end_sq;
        }
    }
    if (pt == KING) count += generate_castling_moves(board, color, sq, possible_end_sqs + count);

    return count;
}

int rook_dirs[4]   = {8, -8,  1, -1};
int bishop_dirs[4] = {9,  7, -7, -9};
int queen_dirs[8]  = {8, -8,  1, -1,  9,  7, -7, -9};

static const int* dir_tables[6] = {NULL, NULL, bishop_dirs, rook_dirs, queen_dirs, NULL};
static const int dir_counts[6] = {0,0,4,4,8,0};

int generate_sliding_moves(Board* board, Color color, int sq, int* possible_end_sqs, PieceType pt) {
    Color opp = OPP_COLOR(color);
    int count = 0;
    
    const int* dirs = dir_tables[pt];
    int dir_count = dir_counts[pt];

    for (int dir = 0; dir < dir_count; dir++) {
        int current_sq = sq;
        while (1) {
            int new_sq = current_sq + dirs[dir];
            if (new_sq < 0 || new_sq > 63) break;

            if (line[sq][new_sq] == 0) break;

            if (DELTA(FILE_OF(new_sq), FILE_OF(current_sq)) > 1) break;

            if (get_bit(board->pieces[color][ALL], new_sq)) break;

            if (get_bit(board->pieces[opp][ALL], new_sq)) {
                possible_end_sqs[count] = new_sq;
                count += 1;
                break;
            } else {
                possible_end_sqs[count] = new_sq;
                count += 1;
            }
            current_sq = new_sq;
        }
    }
    return count;
}

typedef int (*MoveGenFunc)(Board*, Color, int, int*, PieceType);
MoveGenFunc generators[6] = {
    generate_pawn_moves,
    generate_knight_king_moves,
    generate_sliding_moves,
    generate_sliding_moves,
    generate_sliding_moves,
    generate_knight_king_moves  
};


void generate_legal_moves(Board* board, Color color, MoveList* list, bool filter_captures) {
    list->count = 0;
    
    for (PieceType pt = PAWN; pt <= KING; pt++) {
        uint64_t bb = board->pieces[color][pt];
        if (!bb) continue;

        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;

            int possible_end_sqs[64];
            int end_sq_count = generators[pt](board, color, sq, possible_end_sqs, pt);
            for (int i = 0; i < end_sq_count; i++) {
                Move move = {.start = sq, .end = possible_end_sqs[i], .piece = pt, .color = color, 
                            .promotion = NO_PIECE, .is_castling = false, .is_enpassant = false};
                if (filter_captures && !get_bit(board->occupied, possible_end_sqs[i]) && !move.is_enpassant) continue;
                move.is_castling = is_castle_move(move);
                move.is_enpassant = is_enpassant(board, move);
                if (is_legal(board, move)) {
                    if (PROMOTION(move.piece, move.end)) {
                        for (int pt = KNIGHT; pt < KING; pt++) {
                            Move new_move = move;
                            new_move.promotion = pt;
                            list->moves[list->count++] = new_move;
                        }
                    } else {
                        list->moves[list->count++] = move;
                    }
                }
            }
        }
    }
}

bool has_legal_moves(Board *board, Color color) {
    for (PieceType pt = PAWN; pt <= KING; pt++) {
        uint64_t bb = board->pieces[color][pt];
        if (!bb) continue;

        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;

            int possible_end_sqs[64];
            int count = generators[pt](board, color, sq, possible_end_sqs, pt);
            for (int i = 0; i < count; i++) {

                Move temp_move = {.start = sq, .end = possible_end_sqs[i], .piece = pt, .color = color,
                                  .promotion = NO_PIECE, .is_castling = false, .is_enpassant = false};
                temp_move.is_castling = is_castle_move(temp_move);
                temp_move.is_enpassant = is_enpassant(board, temp_move);
                if (is_legal(board, temp_move)) return true;
            }
        }
    }
    return false;
}

bool is_checkmate(Board *board, Color color) {
    if (!in_check(board, color)) return false;

    return (!has_legal_moves(board, color));
}

bool is_stalemate(Board* board, Color color) {
    if (in_check(board, color)) return false;

    return (!has_legal_moves(board, color));
}

bool is_repetition(Board *board) { 
    int count = 0;
    uint64_t current_key = board->zobrist_history[board->zobrist_history_count - 1];
    for (int i = board->zobrist_history_count - 1; i >= 0; i -= 2) {
        if (board->zobrist_history[i] == current_key) count++;
    }
    return count >= 3;
}

bool insufficient_material(Board* board) {
    PieceType non_draw_pieces[3] = {PAWN, ROOK, QUEEN};
    for (int i = 0; i < 3; i++) {
        int count = (__builtin_popcountll(board->pieces[WHITE][non_draw_pieces[i]]) + 
                    __builtin_popcountll(board->pieces[BLACK][non_draw_pieces[i]]));
        if (count > 0) return false;
    }
    int white_kn_count = __builtin_popcountll(board->pieces[WHITE][KNIGHT]);
    int black_kn_count = __builtin_popcountll(board->pieces[BLACK][KNIGHT]);
    int white_b_count = __builtin_popcountll(board->pieces[WHITE][BISHOP]);
    int black_b_count = __builtin_popcountll(board->pieces[BLACK][BISHOP]);
    int white_all_count = white_b_count + white_kn_count;
    int black_all_count = black_b_count + black_kn_count;

    if (black_all_count > 2 || white_all_count > 2) return false;
    if (white_all_count + black_all_count == 0) return true;

    if (white_kn_count == 1 && white_b_count == 0 && black_all_count == 0) return true;
    if (black_kn_count == 1 && black_b_count == 0 && white_all_count == 0) return true;
    if (white_b_count == 1 && white_kn_count == 0 && black_all_count == 0) return true;
    if (black_b_count == 1 && black_kn_count == 0 && white_all_count == 0) return true;

    if (white_kn_count == 2 && white_b_count == 0 && black_all_count == 0) return true;
    if (black_kn_count == 2 && black_b_count == 0 && white_all_count == 0) return true;

    return false;
}

void print_bitboard(uint64_t board) {
    for (int row = 7; row >= 0; row--)
    {
        printf("%d |", row + 1);
        for (int col = 0; col < 8; col++)
        {
            int square = row * 8 + col;
            char symbol = (board & (1ULL << square)) ? '1' : '0';
            printf(" %c", symbol);
        };
        printf(" |\n");
    };
    printf("    a b c d e f g h\n");
}

char *piece_symbols[2][6] = {
    [WHITE] = {"♟", "♞", "♝", "♜", "♛", "♚"},
    [BLACK] = {"♙", "♘", "♗", "♖", "♕", "♔"}};

void print_board(Board *board) {
    char *display_board[8][8];

    for (int r = 0; r < 8; r++)
        for (int f = 0; f < 8; f++)
            display_board[r][f] = ".";

    for (Color color = WHITE; color < COLOR_NUM; color++)
    {
        for (PieceType pt = PAWN; pt <= KING; pt++)
        {
            for (int i = 0; i < SQUARES; i++)
            {
                if (get_bit(board->pieces[color][pt], i))
                {
                    display_board[RANK_OF(i)][FILE_OF(i)] = piece_symbols[color][pt];
                }
            }
        }
    }

    for (int row = 7; row >= 0; row--)
    {
        printf("%d |", row + 1);
        for (int col = 0; col < 8; col++)
        {
            printf(" %s", display_board[row][col]);
        };
        printf(" |\n");
    };
    printf("    a b c d e f g h\n");
}