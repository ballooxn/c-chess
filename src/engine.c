#include "engine.h"
#include "board.h"
#include "move_parser.h"
#include "validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define TT_SIZE 4194304 // power of two, around 160 MB
PositionData trans_table[TT_SIZE];
uint64_t tt_mask = TT_SIZE - 1;
uint8_t tt_current_age = 0;

int material_values[6] = {1, 3, 3, 5, 9, 0};
#define MATERIAL_MULTIPLY 100

const int pst[6][64] = {
    [PAWN] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 0, 0, 25, 25, 0, 0, 5,
        0, 0, 5, 27, 27, 5, 0, 0,
        20, 20, 25, 30, 30, 25, 20, 20,
        30, 30, 35, 45, 45, 35, 30, 30,
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
    }
};
const int endgame_pst[6][64] = {
    [PAWN] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 27, 27, 10, 5, 5,
        0, 0, 0, 25, 25, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -25, -25, 10, 10, 5,
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
    },
    [KING] = {
        -50, -30, -30, -30, -30, -30, -30, -50,
        -30, -10, 0, 0, 0, 0, -10, -30,
        -30, 0, 20, 30, 30, 20, 0, -30,
        -30, 0, 30, 40, 40, 30, 0, -30,
        -30, 0, 30, 40, 40, 30, 0, -30,
        -30, 0, 20, 30, 30, 20, 0, -30,
        -30, -10, 0, 0, 0, 0, -10, -30,
        -50, -30, -30, -30, -30, -30, -30, -50
    }
};

int get_game_phase(Board *board) {
    int phase = 0;

    phase += __builtin_popcountll(board->pieces[WHITE][KNIGHT] | board->pieces[BLACK][KNIGHT]) * 1;
    phase += __builtin_popcountll(board->pieces[WHITE][BISHOP] | board->pieces[BLACK][BISHOP]) * 1;
    phase += __builtin_popcountll(board->pieces[WHITE][ROOK] | board->pieces[BLACK][ROOK]) * 2;
    phase += __builtin_popcountll(board->pieces[WHITE][QUEEN] | board->pieces[BLACK][QUEEN]) * 4;

    return phase;
}

int count_material_positional_value(Board *board, Color color, int phase) {
    int score = 0;
    for (PieceType pt = PAWN; pt <= KING; pt++) {
        uint64_t bb = board->pieces[color][pt];

        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;

            score += (material_values[pt] * MATERIAL_MULTIPLY);

            int pst_sq = (color == WHITE) ? sq : (sq ^ FLIP_BOARD_NUM);

            if (pt == KING) {
                int middle_score = pst[KING][pst_sq];
                int end_score = endgame_pst[KING][pst_sq];
                int blended_score;

                if (phase >= MIDDLE_LIMIT) {
                    blended_score = middle_score;
                } else if (phase <= END_LIMIT) {
                    blended_score = end_score;
                } else {
                    int range = MIDDLE_LIMIT - END_LIMIT;
                    int factor = phase - END_LIMIT;
                    blended_score = ((middle_score * factor) + (end_score * (range - factor))) / range;
                }
                score += blended_score;
            } else {
                score += pst[pt][pst_sq];
            }
        }
    }
    return score;
}
// Only ranks 2-7, dont count back ranks.
int PASSED_PAWN_SCORES[6] = {20, 25, 30, 40, 65, 150};
uint64_t file_masks[8] = {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

int count_pawn_scores(Board *board, Color color) {
    // Loop through pawns.
    int score = 0;

    for (int file = 0; file < 8; file++) {
        // check if pawn is doubled
        uint64_t file_mask = file_masks[file];
        uint64_t pawns_mask = board->pieces[color][PAWN] & file_mask;
        int double_count = __builtin_popcountll(pawns_mask);
        if (double_count > 1) {
            score += ((double_count - 1) * DOUBLED_PAWN_SCORE);
            // increase negative score for each extra pawn 
        } else if (double_count < 1) {
            continue; // no pawn on this file, don't check.
        }

        // Check if the pawn is isolated
        uint64_t target_files = 0ULL;
        if (file > 0) target_files |= file_masks[file - 1];
        if (file < 7) target_files |= file_masks[file + 1];
        if (__builtin_popcountll(board->pieces[color][PAWN] & target_files) == 0) {
            score += ISOLATED_SCORE;
        }
        // loop through each pawn on file, check if passed pawn
        while (pawns_mask) {
            int sq = __builtin_ctzll(pawns_mask);
            pawns_mask &= pawns_mask - 1;
            uint64_t opp_pawns = board->pieces[OPP_COLOR(color)][PAWN];
            if ((opp_pawns & passed_pawn_masks[color][sq]) == 0) {
                if (color == WHITE) {
                    score += PASSED_PAWN_SCORES[(RANK_OF(sq) - 1)];
                } else {
                    score += PASSED_PAWN_SCORES[abs(6 - RANK_OF(sq))];
                }
            }
        }
    }
    return score;
}

uint64_t get_pawn_shield_mask(int king_sq, Color color) {
    int king_file = FILE_OF(king_sq);
    int king_rank = RANK_OF(king_sq);
    int dir = (color == WHITE) ? 1 : -1;
    
    uint64_t shield = 0ULL;
    int shield_rank = king_rank + dir;

    if (shield_rank >= 0 && shield_rank <= 7) {
        set_bit(&shield, shield_rank * 8 + king_file);
        if (king_file > 0) set_bit(&shield, shield_rank * 8 + (king_file - 1));
        if (king_file < 7) set_bit(&shield, shield_rank * 8 + (king_file + 1));
    }
    return shield;
}

int KING_ATTACKER_WEIGHTS[6] = {0, 2, 2, 3, 5, 0};

int king_safety_penalties[30] = {
    0, 0, 5, 10, 20, 35, 60, 90, 120, 160, 225, 280, 300, 
    325, 350, 370, 400, 430, 460, 500, 550, 600, 660, 720,
    800, 900, 1000, 1100, 1200, 1300,
};

int king_safety(Board *board, Color color, int phase) {
    int king_sq = __builtin_ctzll(board->pieces[color][KING]);
    int king_file = FILE_OF(king_sq);
    int king_rank = RANK_OF(king_sq);
    int score = 0;

    int back_rank = (color == WHITE) ? 0 : 7;
    int pawn_rank = (color == WHITE) ? 1 : 6;
    
    if (king_file == 3 || king_file == 4) {
        score += KING_CENTER_SCORE;
    } else if (king_rank == back_rank || king_rank == pawn_rank) {
        // check the three ranks ahead of king for pawns.
        uint64_t shield_1 = get_pawn_shield_mask(king_sq, color);
        uint64_t shield_2 = 0ULL;
        uint64_t shield_3 = 0ULL;
        if (color == WHITE) {
            shield_2 = shield_1 << 8;
            shield_3 = shield_1 << 16;
        } else {
            shield_2 = shield_1 >> 8;
            shield_3 = shield_1 >> 16;
        }
        int count1 = __builtin_popcountll(board->pieces[color][PAWN] & shield_1);
        int count2 = __builtin_popcountll(board->pieces[color][PAWN] & shield_2);
        int count3 = __builtin_popcountll(board->pieces[color][PAWN] & shield_3);
        // Pawn pushed up one rank ahead of king.
        score += (PAWN_ONE_RANK_SCORE * count2);
        score += (PAWN_TWO_RANKS_SCORE * count3);
        int total_count = count1 + count2 + count3;
        if (total_count < 3) {
            score += (PAWN_MISSING * (3 - total_count));
        }
    }
    
    int total_attackers = 0;
    int total_attack_weight = 0;
    uint64_t king_ring = king_ring_masks[king_sq];

    for (PieceType pt = KNIGHT; pt < KING; pt++) {
        uint64_t bb = board->pieces[OPP_COLOR(color)][pt];

        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;

            uint64_t ring_bb = king_ring;

            if (pt == KNIGHT) {
                uint64_t attacks = knight_attacks[sq];
                if (attacks & ring_bb) {
                    total_attackers++;
                    total_attack_weight += KING_ATTACKER_WEIGHTS[pt];
                }
            } else {
                while (ring_bb) {
                    int ring_sq = __builtin_ctzll(ring_bb);
                    ring_bb &= ring_bb - 1;

                    if (line[sq][ring_sq] != 0) {
                        int delta_rank = DELTA(RANK_OF(sq), RANK_OF(ring_sq));
                        int delta_file = DELTA(FILE_OF(sq), FILE_OF(ring_sq));
                        bool valid_dir = false;

                        if (pt == BISHOP && (delta_rank == delta_file)) valid_dir = true;
                        else if (pt == ROOK && (delta_rank == 0 || delta_file == 0)) valid_dir = true;
                        else if (pt == QUEEN) valid_dir = true;

                        if (valid_dir) {
                            if ((between[sq][ring_sq] & board->occupied) == 0) {
                                total_attackers++;
                                total_attack_weight += KING_ATTACKER_WEIGHTS[pt];
                                break;
                            }
                        }
                    } 
                }
            }
        }
    }

    if (total_attackers >= 2) {
        if (total_attack_weight > 29) total_attack_weight = 29;
        int penalty = king_safety_penalties[total_attack_weight];

        penalty = (penalty * phase) / MAX_PHASE;
        score -= penalty;
    }

    return score;  
}

int evaluate(Board *board, Color color) {
    int phase = get_game_phase(board);

    int white_mat_pst = count_material_positional_value(board, WHITE, phase);
    int black_mat_pst = count_material_positional_value(board, BLACK, phase);
    
    int white_pawn = count_pawn_scores(board, WHITE);
    int black_pawn = count_pawn_scores(board, BLACK);

    int white_king_safety = king_safety(board, WHITE, phase);
    int black_king_safety = king_safety(board, BLACK, phase);

    int score = 0;
    if (color == WHITE) {
        score += white_mat_pst - black_mat_pst;
        score += white_pawn - black_pawn;
        score += white_king_safety - black_king_safety;
    } else {
        score += black_mat_pst - white_mat_pst;
        score += black_pawn - white_pawn;
        score += black_king_safety - white_king_safety;
    }
    
    score += TEMPO_BONUS;
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
        if (move->color == BLACK) {
            score += pst[move->piece][move->end ^ 56];
        } else {
            score += pst[move->piece][move->end];
        }
    }
    return score;
}

void sort_moves(Board *board, MoveList *move_list) {
    int moves_count = move_list->count;
    int move_scores[MAX_LEGAL_MOVES];

    uint64_t index = board->current_zobrist_key & tt_mask;
    PositionData *entry = &trans_table[index];
    bool in_entry = (entry->key == board->current_zobrist_key);

    for (int i = 0; i < move_list->count; i++) {
        if (in_entry && move_list->moves[i].start == entry->best_move.start && 
            move_list->moves[i].end == entry->best_move.end && move_list->moves[i].piece == entry->best_move.piece && 
            move_list->moves[i].promotion == entry->best_move.promotion) {
            move_scores[i] = 1000000;
        } else {
            move_scores[i] = score_move(board, &(move_list->moves[i]));
        }
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

int quiescence(Board *board, Color color, int ply, int alpha, int beta) {
    int stand_pat = evaluate(board, color);

    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    MoveList capture_list;
    generate_legal_moves(board, color, &capture_list, true);
    sort_moves(board, &capture_list);

    for (int i = 0; i < capture_list.count; i++) {
        Move move = capture_list.moves[i];
        move_piece(board, move);
        int eval = -quiescence(board, OPP_COLOR(color), ply + 1, -beta, -alpha);
        reverse_move(board, move);

        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    }
    return alpha;
}

int search(Board *board, int depth, int ply, Color color, int alpha, int beta) {
    uint64_t index = board->current_zobrist_key & tt_mask;
    PositionData *entry = &trans_table[index];
    int orig_alpha = alpha;

    if (entry->key == board->current_zobrist_key) {
        if (entry->depth >= depth) {
            int stored_score = entry->score;
            if (stored_score > MATE_EVAL - 100) stored_score -= ply;
            else if (stored_score < -MATE_EVAL + 100) stored_score += ply;

            if (entry->type == TT_EXACT) {
                return stored_score;
            } else if (entry->type == TT_BETA && stored_score >= beta) {
                return beta;
            } else if (entry->type == TT_ALPHA && stored_score <= alpha) {
                return alpha;
            }
        }
    }

    if (is_repetition(board, 3) || board->halfmove_clock >= 100) {
        return 0;
    }
    
    if (depth == 0) {
        if (in_check(board, color)) {
            depth++;
        } else {
            return quiescence(board, color, ply, alpha, beta);
        }
    }
    MoveList move_list;
    generate_legal_moves(board, color, &move_list, false);

    if (move_list.count == 0) {
        if (in_check(board, color)) {
            return -(MATE_EVAL - ply);
        } return 0;
    }

    sort_moves(board, &move_list);
    int best_eval = -INF;
    Move best_move = move_list.moves[0];

    for (int i = 0; i < move_list.count; i++) {
        Move move = move_list.moves[i];
        move_piece(board, move);
        int eval = -search(board, depth - 1, ply + 1, OPP_COLOR(color), -beta, -alpha);
        reverse_move(board, move);

        if (eval > best_eval) {
            best_eval = eval;
            best_move = move;
        }
        if (best_eval > alpha) alpha = best_eval;
        if (alpha >= beta) break;
    }

    if (entry->key == 0 || entry->age != tt_current_age || depth >= entry->depth) {
        int store_score = best_eval;
        if (store_score > MATE_EVAL - 100) store_score += ply;
        else if (store_score < -MATE_EVAL + 100) store_score -= ply;
        entry->score = store_score;

        if (best_eval <= orig_alpha) { 
            entry->type = TT_ALPHA;
        } else if (best_eval >= beta) {
            entry->type = TT_BETA;
        } else {
            entry->type = TT_EXACT;
        }

        entry->key = board->current_zobrist_key;
        entry->depth = depth;
        entry->best_move = best_move;
        entry->ply = ply;
        entry->age = tt_current_age;
    }

    return best_eval;
}

Move engine_move(Board *board, Color color) {
    // Dont forget to tack on the promotion piece to the Move struct
    // and is_castling and is_enpassant
    tt_current_age++;
    MoveList move_list;
    generate_legal_moves(board, color, &move_list, false);

    Move best_move = move_list.moves[0];
    int final_depth_reached = 0;
    int final_best_eval = -INF;

    clock_t start_time = clock();
    double time_limit = 2.5;
    bool time_out = false;

    for (int current_depth = 1; current_depth <= 64; current_depth++) {
        sort_moves(board, &move_list);
        
        int best_eval = -INF;
        int alpha = -INF;
        int beta = INF;
        Move depth_best_move = best_move;

        for (int i = 0; i < move_list.count; i++) {
            if (i > 0) {
                double elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
                if (elapsed >= time_limit) {
                    time_out = true;
                    break;
                }
            }
            Move move = move_list.moves[i];
            move_piece(board, move);
            int eval = -search(board, current_depth - 1, 1, OPP_COLOR(color), -beta, -alpha);
            reverse_move(board, move);

            if (eval > best_eval) {
                best_eval = eval;
                depth_best_move = move;
            }
            if (best_eval > alpha) {
                alpha = best_eval;
            }
        }

        if (time_out) break;

        best_move = depth_best_move;
        final_depth_reached = current_depth;
        final_best_eval = best_eval;

        uint64_t root_index = board->current_zobrist_key & tt_mask;
        PositionData *root_entry = &trans_table[root_index];
        root_entry->key = board->current_zobrist_key;
        root_entry->depth = current_depth;
        root_entry->score = best_eval;
        root_entry->best_move = best_move;
        root_entry->type = TT_EXACT;
        root_entry->age = tt_current_age;
        root_entry->ply = 0;

        double total_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        if (total_elapsed * 4.0 >= time_limit) break;
    }
    double time_spent = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    printf("Depth: %d / Score: %d / Time: %.3f seconds\n", 
            final_depth_reached, final_best_eval, time_spent);
    return best_move;
}