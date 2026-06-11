#include "board.h"
#include "move_parser.h"
#include "validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

int material_values[6] = {1, 3, 3, 5, 9, 0};
#define MATERIAL_MULTIPLY 100

#define FLIP_BOARD_NUM 56
#define MAX_DEPTH 6
#define MATE_EVAL 100000
#define INF 1000000 

#define MAX_LEGAL_MOVES 218

const int pst[6][64] = {
    [PAWN] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 10, 10, -25, -25, 10, 10, 5,
        5, -5, -10, 0, 0, -10, -5, 5,
        0, 0, 0, 25, 25, 0, 0, 0,
        5, 5, 10, 27, 27, 10, 5, 5,
        10, 10, 20, 30, 30, 20, 10, 10,
        50, 50, 50, 50, 50, 50, 50, 50,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    [KNIGHT] = {
        -50, -40, -20, -30, -30, -20, -40, -50,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    },
    [BISHOP] = {
        -20, -10, -40, -10, -10, -40, -10, -20,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    },
    [KING] = {
        20,  30,  10,  0,  0,  10,  30,  20,
        20,  20,  0,  0,  0,  0,  20,  20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30
    },
    [ROOK] = {
        -5, 0, 0, 10, 10, 5, 0, -5,
        0, 0, 0, 10, 10, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10
    },
    [QUEEN] = {
        -20, -10, -10, 0, 0, -10, -10, -20,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        -5, 0, 5, 5, 5, 5, 0, -5,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -20, -10, 0, 0, 0, 0, -10, -20
    }
};

int count_material_positional_value(Board *board, Color color) {
    int score = 0;
    for (PieceType pt = PAWN; pt <= KING; pt++) {
        uint64_t bb = board->pieces[color][pt];

        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;

            score += (material_values[pt] * MATERIAL_MULTIPLY);

            if (color == WHITE) {
                score += pst[pt][sq ^ 56];
            } else {
                score += pst[pt][sq];
            }
        }
    }
    return score;
}

int evaluate(Board *board, Color engine_color) {
    int white = count_material_positional_value(board, WHITE);
    int black = count_material_positional_value(board, BLACK);

    int score = (engine_color == WHITE) ? (white - black) : (black - white);
    return score;
}

int score_move(Board *board, Move *move) {
    int score = 0;
    if (get_bit(board->occupied, move->end)) {
        PieceType victim = get_piece(board, move->end, OPP_COLOR(move->color));

        if (victim != NO_PIECE) {
            score = 1000 + (material_values[victim] * 10) - material_values[move->piece];
        }
    } else {
        if (move->color == WHITE) {
            score += pst[move->piece][move->end ^ 56];
        } else {
            score += pst[move->piece][move->end];
        }
    }
    return score;
}

void sort_moves(Board *board, MoveList *move_list) {
    int moves_count = move_list->count;
    int move_scores[MAX_MOVES];

    for (int i = 0; i < move_list->count; i++) {
        move_scores[i] = score_move(board, &(move_list->moves[i]));
    }

    // sort moves

    for (int step = 0; step < moves_count - 1; ++step) {
        for (int i = 0; i < moves_count - step - 1; ++i) {
            if (move_scores[i] < move_scores[i + 1]) {
                Move temp = move_list->moves[i];
                move_list->moves[i] = move_list->moves[i + 1];
                move_list->moves[i + 1] = temp;

                int temp_score = move_scores[i];
                move_scores[i] = move_scores[i + 1];
                move_scores[i + 1] = temp_score;
            }
        }
    }
}

int quiescence(Board *board, Color color, Color engine_color, bool maximizing, int alpha, int beta) {
    int stand_pat = evaluate(board, engine_color);

    if (maximizing) {
        if (stand_pat > alpha) alpha = stand_pat;
        if (alpha >= beta) return stand_pat;

        int best_eval = stand_pat;

        MoveList capture_list;
        generate_legal_moves(board, color, &capture_list, true);
        sort_moves(board, &capture_list);

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
        sort_moves(board, &capture_list);

        for (int i = 0; i < capture_list.count; i++) {
            Move move = capture_list.moves[i];
            move_piece(board, move);
            int eval = quiescence(board, OPP_COLOR(color), engine_color, true, alpha, beta);
            reverse_move(board, move);

            if (eval < best_eval) best_eval = eval;
            if (best_eval < beta) beta = best_eval;
            if (alpha >= beta) break;
        }
        return best_eval;
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

    sort_moves(board, &move_list);

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
    sort_moves(board, &move_list);

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
    return best_move;
}