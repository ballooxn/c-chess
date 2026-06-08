#include "board.h"
#include "move_parser.h"
#include "validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

int material_values[5] = {1, 3, 3, 5, 9};

#define MAX_DEPTH 4
#define MATE_EVAL 100000
#define INF 1000000 

int count_material_value(Board *board, Color color) {
    int count = 0;
    for (PieceType i = PAWN; i < KING; i++) {
        count += (__builtin_popcountll(board->pieces[color][i]) * material_values[i]);
    }
    return count;
}

int evaluate(Board *board, Color engine_color) {
    int white_material = count_material_value(board, WHITE);
    int black_material = count_material_value(board, BLACK);

    int score = (engine_color == WHITE) ? (white_material - black_material) : (black_material - white_material);
    return score;
}

int quiescence(Board *board, Color color, Color engine_color, bool maximizing, int alpha, int beta) {
    int stand_pat = evaluate(board, engine_color);

    if (maximizing) {
        if (stand_pat > alpha) alpha = stand_pat;
        if (alpha >= beta) return stand_pat;

        int best_eval = stand_pat;

        MoveList capture_list;
        generate_legal_moves(board, color, &capture_list, true);

        for (int i = 0; i < capture_list.count; i++) {
            Move move = capture_list.moves[i];
            move_piece(board, move);
            int eval = quiescence(board, OPP_COLOR(color), engine_color, false, alpha, beta);
            reverse_move(board, move);

            if (eval > best_eval) best_eval = eval;
            if (best_eval > alpha) alpha = best_eval;
            if (alpha >= beta) break;
        }
        return best_eval;
    } else {
        if (stand_pat < beta) beta = stand_pat;
        if (alpha >= beta) return stand_pat;

        int best_eval = stand_pat;

        MoveList capture_list;
        generate_legal_moves(board, color, &capture_list, true);

        for (int i = 0; i < capture_list.count; i++) {
            Move move = capture_list.moves[i];
            move_piece(board, move);
            int eval = quiescence(board, OPP_COLOR(color), engine_color, true, alpha, beta);
            reverse_move(board, move);

            if (eval < best_eval) best_eval = eval;
            if (best_eval < beta) beta = best_eval;
            if (alpha >= beta) break;
        }
        return stand_pat;
    }
}

int search(Board *board, int depth, Color color, Color engine_color, bool maximizing, int alpha, int beta) {
    if (depth == 0) {
        if (in_check(board, color)) {
            depth++;
        } else {
            return quiescence(board, color, engine_color, maximizing, alpha, beta);
        }
    }
    MoveList move_list;
    generate_legal_moves(board, color, &move_list, false);

    if (move_list.count == 0) {
        if (in_check(board, color)) {
            return (maximizing) ? -MATE_EVAL : MATE_EVAL;
        } else {
            return 0;
        }
    }

    if (maximizing) {
        int best_eval = -INF;
        for (int i = 0; i < move_list.count; i++) {
            Move move = move_list.moves[i];
            move_piece(board, move);
            int eval = search(board, depth - 1, OPP_COLOR(color), engine_color, false, alpha, beta);
            reverse_move(board, move);

            if (eval > best_eval) best_eval = eval;
            if (best_eval > alpha) alpha = best_eval;
            if (alpha >= beta) break;
        }
        return best_eval;
    } else {
        int best_eval = INF;
        for (int i = 0; i < move_list.count; i++) {
            Move move = move_list.moves[i];
            move_piece(board, move);
            int eval = search(board, depth - 1, OPP_COLOR(color), engine_color, true, alpha, beta);
            reverse_move(board, move);

            if (eval < best_eval) best_eval = eval;
            if (best_eval < beta) beta = best_eval;
            if (alpha >= beta) break;
        }
        return best_eval;
    }
}

Move engine_move(Board *board, Color color) {
    // Dont forget to tack on the promotion piece to the Move struct
    // and is_castling and is_enpassant
    MoveList move_list;
    generate_legal_moves(board, color, &move_list, false);

    Move best_move = move_list.moves[0];
    int best_eval = -INF;
    int alpha = -INF;

    for (int i = 0; i < move_list.count; i++) {
        Move move = move_list.moves[i];
        move_piece(board, move);
        int eval = search(board, MAX_DEPTH - 1, OPP_COLOR(color), color, false, alpha, INF);
        reverse_move(board, move);

        if (eval > best_eval) {
            best_eval = eval;
            best_move = move;
        }
        if (best_eval > alpha) {
            alpha = best_eval;
        }
    }
    printf("Best eval: %d\n", best_eval);
    return best_move;
}