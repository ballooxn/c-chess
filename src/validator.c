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
        if (board->last_double_push != side_pawn) return false;
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

bool in_check(Board *board, Color color) {
    int king_sq = __builtin_ctzll(board->pieces[color][KING]);
    Color opp = OPP_COLOR(color);
    uint64_t bb = board->pieces[opp][ALL];
    while (bb)
    {
        int sq = __builtin_ctzll(bb);
        bb &= bb - 1;
        PieceType piece_type = get_piece(board, sq, opp);
        Move temp_move = {.start = sq, .end = king_sq, .piece = piece_type, .color = opp};

        if (piece_type == KING || piece_type == NO_PIECE)
            continue;

        if (valid_move(board, temp_move)) {
            return true;
        }
    }
    return false;
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
        Board temp = *board;
        remove_piece(&temp, king_start, KING, color);
        place_piece(&temp, sq, KING, color);
        if (in_check(&temp, color)) return false;
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
    PieceType target_piece = get_piece(board, move.end, OPP_COLOR(move.color));

    move_piece(board, move, true);
    bool is_in_check = in_check(board, move.color);
    reverse_simulated_move(board, move, target_piece);
    return !is_in_check;
}