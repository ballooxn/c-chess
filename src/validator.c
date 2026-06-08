#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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

bool is_pawn_valid(Board *board, Move move, bool pawn_double_push) {
    if (pawn_double_push)
    {
        if (between[move.start][move.end] & board->occupied || get_bit(board->occupied, move.end))
            return false; // piece in way
        int home_rank = (move.color == WHITE) ? 1 : 6;
        if (RANK_OF(move.start) != home_rank) return false;
        return true;
    }
    if (is_enpassant(board, move)) {
        // en passant move
        int side_dir = (FILE_OF(move.end) - FILE_OF(move.start) == 1) ? 1 : -1;
        int side_pawn = move.start + side_dir;
        if (!get_bit(board->pieces[OPP_COLOR(move.color)][PAWN], side_pawn)) return false;
        return true;
    }
    switch (move.color) {
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

bool valid_move(Board *board, Move move) {
    bool pawn_double_push = (move.piece == PAWN &&
        FILE_OF(move.start) == FILE_OF(move.end) && DELTA(RANK_OF(move.end), RANK_OF(move.start)) == 2);
    if (move.piece == PAWN)
    {
        return is_pawn_valid(board, move, pawn_double_push);
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
        return is_sliding_valid(board, move);
    }
    return false;
}

// instead of checking each piece to see if it can attack the king
// look at each attack bitboard for the kings square
// and see if the "superking" would be able to attack any enemy pieces
// if yes, then that enemy piece can attack the king, so we are attacked.

int directions[8] = {9, 7, -7, -9, 8, -8, 1, -1};
#define DIR_COUNT 8

bool is_square_attacked(Board *board, int sq, Color attacker_color) {
    if (knight_attacks[sq] & board->pieces[attacker_color][KNIGHT]) return true;
    if (king_attacks[sq] & board->pieces[attacker_color][KING]) return true;
    
    uint64_t pawn_attacks = (attacker_color == WHITE) ? black_pawn_attacks[sq] : white_pawn_attacks[sq];
    if (pawn_attacks & board->pieces[attacker_color][PAWN]) return true;

    for (int i = 0; i < DIR_COUNT; i++) {
        PieceType target_piece = BISHOP;
        if (i >= 4) target_piece = ROOK;
        int current_sq = sq;
        while (1) {
            int new_sq = current_sq + directions[i];
            if (new_sq < 0 || new_sq > 63) break;

            if (DELTA(FILE_OF(new_sq), FILE_OF(current_sq)) > 1) break;
            
            if (get_bit(board->occupied, new_sq)) {
                if (get_bit(board->pieces[attacker_color][target_piece], new_sq) || 
                    get_bit(board->pieces[attacker_color][QUEEN], new_sq)) {
                    return true;
                } else break;
            }
            current_sq = new_sq;
        }
    }
    return false;
}

bool in_check(Board *board, Color color) {
    int king_sq = __builtin_ctzll(board->pieces[color][KING]);
    return is_square_attacked(board, king_sq, OPP_COLOR(color));
}

bool can_castle(Board* board, Color color, bool kingside) {
    if (color == WHITE) {
        if (kingside && !board->white_can_castle_kingside) return false;
        if (!kingside && !board->white_can_castle_queenside) return false;
    } else {
        if (kingside && !board->black_can_castle_kingside) return false;
        if (!kingside && !board->black_can_castle_queenside) return false;
    }
    int king_start = (color == WHITE) ? E1 : E8;
    int rook_start = (color == WHITE) ? (kingside ? 7 : 0) : (kingside ? H8 : A8);

    if (!get_bit(board->pieces[color][ROOK], rook_start)) return false;
    if (in_check(board, color)) return false; // cannot castle while in check

    int step = kingside ? 1 : -1;
    for (int sq = king_start + step; sq != rook_start; sq += step) {
        if (get_bit(board->occupied, sq)) return false;
        // on queenside castle, don't loop through the B file (king wont be touching it)
        if (!kingside && DELTA(rook_start, sq) == 1) continue;
        if (is_square_attacked(board, sq, OPP_COLOR(color))) return false;
    }
    return true;
}

bool is_legal(Board* board, Move move) {

    if (get_bit(board->pieces[move.color][ALL], move.end))
        return false;
    if (move.start == move.end)
        return false;

    if (move.piece == NO_PIECE)
        return false;

    if (move.piece == KING && is_castle_move(move)) {
        bool kingside = FILE_OF(move.end) == 6;
        return can_castle(board, move.color, kingside);
    }

    if (!valid_move(board, move))
        return false;

    move_piece(board, move);
    bool is_in_check = in_check(board, move.color);
    reverse_move(board, move);
    return !is_in_check;
}