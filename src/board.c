#include "board.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SQUARES 64

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
    return board;
}

void init_pawn_attacks(void) {
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

void init_knight_attacks(void)
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

void init_king_attacks(void)
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
                // do castling stuff later
            }
        }
        king_attacks[sq] = attacks;
    }
}

void init_sliding_tables(void)
{
    const int dirs_rank[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    const int dirs_file[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    // N,S,E,W,NE,NW,SE,SW

    for (int sq1 = 0; sq1 < SQUARES; sq1++)
    {
        int rank1 = RANK_OF(sq1);
        int file1 = FILE_OF(sq1);

        for (int dir = 0; dir < 8; dir++)
        {
            int offset_rank = dirs_rank[dir];
            int offset_file = dirs_file[dir];

            // build whole ray (line)
            uint64_t ray = 0;
            int rank2 = rank1 + offset_rank;
            int file2 = file1 + offset_file;
            while (rank2 >= 0 && rank2 < 8 && file2 >= 0 && file2 < 8)
            {
                int sq2 = rank2 * 8 + file2;
                set_bit(&ray, sq2);
                rank2 += offset_rank;
                file2 += offset_file;
            }

            // build between
            rank2 = rank1 + offset_rank;
            file2 = file1 + offset_file;
            uint64_t between_mask = 0;
            while (rank2 >= 0 && rank2 < 8 && file2 >= 0 && file2 < 8)
            {
                int sq2 = rank2 * 8 + file2;
                line[sq1][sq2] = (1ULL << sq1) | (1ULL << sq2) | between_mask; // include starting pos
                between[sq1][sq2] = between_mask;
                set_bit(&between_mask, sq2);
                rank2 += offset_rank;
                file2 += offset_file;
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

PieceType get_piece(Board board, int sq, Color color)
{
    for (PieceType pt = PAWN; pt <= KING; pt++)
    {
        if (get_bit(board.pieces[color][pt], sq))
        {
            return pt;
        }
    }
    return NO_PIECE;
}

void place_piece(Board *board, int pos, PieceType pt, Color color)
{
    set_bit(&board->pieces[color][pt], pos);
    set_bit(&board->pieces[color][ALL], pos);
    set_bit(&board->occupied, pos);
}

void remove_piece(Board *board, int pos, PieceType pt, Color color)
{
    clear_bit(&board->pieces[color][pt], pos);
    clear_bit(&board->occupied, pos);
    clear_bit(&board->pieces[color][ALL], pos);
}

void move_piece(Board *board, Move move)
{
    Color opp = OPP_COLOR(move.color);
    PieceType target_piece = get_piece(*board, move.end, opp);

    remove_piece(board, move.start, move.piece, move.color);
    if (target_piece != NO_PIECE)
    {
        remove_piece(board, move.end, target_piece, opp);
    }
    place_piece(board, move.end, move.piece, move.color);
}

void reverse_simulated_move(Board *board, Move move, PieceType target_piece)
{
    Color opp = OPP_COLOR(move.color);

    remove_piece(board, move.end, move.piece, move.color);
    if (target_piece != NO_PIECE)
    {
        place_piece(board, move.end, target_piece, opp);
    }
    place_piece(board, move.start, move.piece, move.color);
}

bool is_sliding_valid(Board *board, Move move)
{
    if (line[move.start][move.end] == 0)
        return false;
    if (between[move.start][move.end] & board->occupied)
        return false; // piece in way

    int delta_rank = DELTA(RANK_OF(move.start), RANK_OF(move.end));
    int delta_file = DELTA(FILE_OF(move.start), FILE_OF(move.end));
    if (move.piece == ROOK && !(delta_rank == 0 || delta_file == 0))
        return false; // rook cant diagonal
    if (move.piece == BISHOP && delta_rank != delta_file)
        return false; // bishop only diagonal

    return true;
}

bool is_pawn_valid(Board *board, Move move, bool pawn_double_push)
{
    if (pawn_double_push)
    {
        if (between[move.start][move.end] & board->occupied)
            return false; // piece in way
        int home_rank = (move.color == WHITE) ? 1 : 6;
        if (RANK_OF(move.start) != home_rank) return false;
        return true;
    }

    switch (move.color)
    {
    case WHITE:
        if (get_bit(white_pawn_attacks[move.start], move.end) && get_bit(board->pieces[BLACK][ALL], move.end))
            return true;
        if (get_bit(white_pawn_pushes[move.start], move.end) && !get_bit(board->occupied, move.end))
            return true;
        break;
    case BLACK:
        if (get_bit(black_pawn_attacks[move.start], move.end) && get_bit(board->pieces[WHITE][ALL], move.end))
            return true;
        if (get_bit(black_pawn_pushes[move.start], move.end) && !get_bit(board->occupied, move.end))
            return true;
        break;
    default:
        return false;
    }
    return false;
}

bool valid_move(Board board, Move move) {
    bool pawn_double_push = (move.piece == PAWN &&
        FILE_OF(move.start) == FILE_OF(move.end) && DELTA(RANK_OF(move.end), RANK_OF(move.start)) == 2);
    if (move.piece == PAWN)
    {
        return is_pawn_valid(&board, move, pawn_double_push);
    }
    else if (move.piece == KNIGHT && get_bit(knight_attacks[move.start], move.end))
    {
        return true;
    }
    else if (move.piece == KING && get_bit(king_attacks[move.start], move.end))
    {
        return true;
    }
    else if (move.piece == QUEEN || move.piece == BISHOP || move.piece == ROOK)
    {
        return is_sliding_valid(&board, move);
    }
    return false;
}

bool in_check(Board *board, Color color)
{
    int king_sq = __builtin_ctzll(board->pieces[color][KING]);
    Color opp = OPP_COLOR(color);
    uint64_t bb = board->pieces[opp][ALL];
    while (bb)
    {
        int sq = __builtin_ctzll(bb);
        bb &= bb - 1;
        PieceType piece_type = get_piece(*board, sq, opp);
        Move temp_move = {.start = sq, .end = king_sq, .piece = piece_type, .color = opp};

        if (piece_type == KING || piece_type == NO_PIECE)
            continue;

        if (valid_move(*board, temp_move)) {
            printf("Checker: square %d (rank %d, file %c), piece %d\n",
                    sq, RANK_OF(sq), 'a' + FILE_OF(sq), piece_type);
            return true;
        }
    }
    return false;
}

bool is_legal(Board board, Move move) {
    printf("move: %d -> %d, piece: %d\n", move.start, move.end, move.piece);
    printf("friendly dest: %d\n", get_bit(board.pieces[move.color][ALL], move.end));
    printf("valid_move: %d\n", valid_move(board, move));  

    if (get_bit(board.pieces[move.color][ALL], move.end))
        return false;
    if (move.start == move.end)
        return false;

    if (move.piece == NO_PIECE)
        return false;

    if (!valid_move(board, move))
        return false;

    Color opp = OPP_COLOR(move.color);
    PieceType target_piece = get_piece(board, move.end, opp);

    move_piece(&board, move);
    bool is_in_check = in_check(&board, move.color);
    printf("in_check after move: %d\n", is_in_check);
    reverse_simulated_move(&board, move, target_piece);

    if (is_in_check)
    {
        return false;
    }

    return true;
}

static uint64_t * const pawn_pushes[2] = {white_pawn_pushes, black_pawn_pushes};
static uint64_t * const pawn_attacks[2] = {white_pawn_attacks, black_pawn_attacks};

int generate_pawn_moves(Board* board, Color color, int sq, int* possible_end_sqs, PieceType pt) {
    (void)pt;
    Color opp = OPP_COLOR(color);
    int count = 0;
    
    uint64_t push_bb = pawn_pushes[color][sq];
    int inter_sq = (color == WHITE) ? sq + 8 : sq - 8; // for double push
    while (push_bb) {
        int end_sq = __builtin_ctzll(push_bb);
        push_bb &= push_bb - 1;
        if (get_bit(board->pieces[color][ALL], end_sq)) continue;
        if (get_bit(board->pieces[opp][ALL], end_sq)) continue;
        if (DELTA(RANK_OF(end_sq), RANK_OF(sq)) == 2 && get_bit(board->occupied, inter_sq)) continue;
        possible_end_sqs[count] = end_sq;
        count += 1;
    }
    uint64_t diag_bb = pawn_attacks[color][sq];
    while (diag_bb) {
        int end_sq = __builtin_ctzll(diag_bb);
        diag_bb &= diag_bb - 1;
        if (get_bit(board->pieces[color][ALL], end_sq)) continue;
        if (get_bit(board->pieces[opp][ALL], end_sq)) {
            possible_end_sqs[count] = end_sq;
            count += 1;
        }
    }

    return count;
}

int generate_knight_king_moves(Board* board, Color color, int sq, int* possible_end_sqs, PieceType pt) {
    int count = 0;
    
    uint64_t attack_bb = (pt == KING) ? king_attacks[sq] : knight_attacks[sq];
    while (attack_bb) {
        int end_sq = __builtin_ctzll(attack_bb);
        attack_bb &= attack_bb - 1;
        if (get_bit(board->pieces[color][ALL], end_sq)) continue;
        possible_end_sqs[count] = end_sq;
        count += 1;
    }

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
        int new_sq = sq;
        while (1) {
            new_sq += dirs[dir];
            if (new_sq < 0 || new_sq > 63) break;
            if (line[sq][new_sq] == 0) break;
            if (get_bit(board->pieces[color][ALL], new_sq)) break;
            if (get_bit(board->pieces[opp][ALL], new_sq)) {
                possible_end_sqs[count] = new_sq;
                count += 1;
                break;
            } else {
                possible_end_sqs[count] = new_sq;
                count += 1;
            }
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
                Move temp_move = {.start = sq, .end = possible_end_sqs[i], .piece = pt, .color = color};
                if (is_legal(*board, temp_move)) return true;
            }
        }
    }
    return false;
}

bool in_checkmate(Board *board, Color color) {
    if (!in_check(board, color)) return false;

    return (!has_legal_moves(board, color));
}

bool in_stalemate(Board* board, Color color) {
    if (in_check(board, color)) return false;

    return (!has_legal_moves(board, color));
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