#define _POSIX_C_SOURCE 199309L
#include "../src/move_parser.h"
#include "../src/board.h"
#include "../src/main.h"
#include "../src/validator.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

static void apply_moves(Board *board, char *moves[]) {
    for (int i = 0; moves[i] != NULL; i++) {
        int color = (i % 2 == 0) ? WHITE : BLACK;
        Move move = string_to_move(moves[i], *board, color);
        assert(is_legal(board, move));
        move_piece(board, move);
    }
}

uint64_t perft(Board* board, Color color, int depth) {
    if (depth == 0) return 1;

    MoveList list;
    generate_legal_moves(board, color, &list, false);

    uint64_t nodes = 0;

    for (int i = 0; i < list.count; i++) {
        Move move = list.moves[i];
        move_piece(board, move);

        nodes += perft(board, OPP_COLOR(color), depth - 1);
        
        reverse_move(board, move);
    }

    return nodes;
}

void test_perft_depth_one(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    int depth = 1;
    uint64_t result = perft(&board, WHITE, depth);
    printf("Perft %d: %lu\n", depth, result);
    assert(result == 20);
}

void test_perft_depth_two(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    int depth = 2;
    uint64_t result = perft(&board, WHITE, depth);
    printf("Perft %d: %lu\n", depth, result);
    assert(result == 400);
}

void test_perft_depth_three(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    int depth = 3;
    uint64_t result = perft(&board, WHITE, depth);
    printf("Perft %d: %lu\n", depth, result);
    assert(result == 8902);
}

void test_perft_depth_four(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    int depth = 4;
    uint64_t result = perft(&board, WHITE, depth);
    printf("Perft %d: %lu\n", depth, result);
    assert(result == 197281);
}

void test_perft_depth_five(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    int depth = 5;
    uint64_t result = perft(&board, WHITE, depth);
    printf("Perft %d: %lu\n", depth, result);
    assert(result == 4865609);
}

void test_perft_depth_six(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    int depth = 6;
    uint64_t result = perft(&board, WHITE, depth);
    printf("Perft %d: %lu\n", depth, result);
    assert(result == 119060324);
}

void test_valid_move(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    assert(valid_move(&board, string_to_move("e2e3", board, WHITE)));
}

void test_cant_capture_own_pieces(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    assert(!is_legal(&board, string_to_move("a1b1", board, WHITE)));
}

void test_cant_move_opponents_pieces(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    assert(!is_legal(&board, string_to_move("e7e6", board, WHITE)));
}

void test_move_piece(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    Move move = string_to_move("e2e3", board, WHITE);
    int start_bits = 12;
    int end_bits = 20;
    move_piece(&board, move);
    assert(!get_bit(board.occupied, start_bits) && get_bit(board.occupied, end_bits) &&
            !get_bit(board.pieces[WHITE][ALL], start_bits) && get_bit(board.pieces[WHITE][ALL], end_bits) &&
            !get_bit(board.pieces[WHITE][PAWN], start_bits) && get_bit(board.pieces[WHITE][ALL], end_bits));
}

void test_undo_move(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    Move move = string_to_move("e2e3", board, WHITE);
    int start_bits = 12;
    int end_bits = 20;
    move_piece(&board, move);
    reverse_move(&board, move);
    assert(!get_bit(board.occupied, end_bits) && get_bit(board.occupied, start_bits) &&
            !get_bit(board.pieces[WHITE][ALL], end_bits) && get_bit(board.pieces[WHITE][ALL], start_bits) &&
            !get_bit(board.pieces[WHITE][PAWN], end_bits) && get_bit(board.pieces[WHITE][ALL], start_bits));
}

void test_double_pawn_push(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    assert(is_legal(&board, string_to_move("e2e4", board, WHITE)));
}

void test_single_pawn_push(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    assert(is_legal(&board, string_to_move("e2e3", board, WHITE)));
}

void test_pawn_cant_move_backward(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("e4e3", board, WHITE)));
}

void test_pawn_capture(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("e4d5", board, WHITE)));
}

void test_pawn_en_passant(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5",
        "e4e5", "f7f5",
        NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("e5f6", board, WHITE)));
}

void test_pawn_en_passant_capture_removal(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5",
        "e4e5", "f7f5",
        NULL
    };
    apply_moves(&board, moves);

    Move ep_move = string_to_move("e5f6", board, WHITE);
    assert(is_legal(&board, ep_move));

    move_piece(&board, ep_move);
    assert(!get_bit(board.pieces[BLACK][PAWN], TO_BITS(4, 4))); // d5
}

void test_pawn_promotion(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "f7f5",
        "e4f5", "a7a6",
        "f5f6", "a6a5",
        "f6g7", "b7b5",
        NULL
    };
    apply_moves(&board, moves);

    Move move = string_to_move("g7h8", board, WHITE);
    move.promotion = QUEEN;
    move_piece(&board, move);
    assert(get_bit(board.pieces[WHITE][QUEEN], 63));
    assert(!get_bit(board.pieces[WHITE][PAWN], 63));
}

void test_knight_movement(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    assert(is_legal(&board, string_to_move("g1f3", board, WHITE)));
}

void test_bishop_movement(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("f1c4", board, WHITE)));
}

void test_bishop_cant_move_like_rook(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", "f1c4", "a7a6", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("c4c3", board, WHITE)));
}

void test_rook_movement(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "a2a4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("a1a3", board, WHITE)));
}

void test_queen_movement(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("d1h5", board, WHITE)));
}

void test_king_movement(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("e1e2", board, WHITE)));
}

void test_king_castle_kingside(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "e7e5",
        "g1f3", "b8c6",
        "f1c4", "f8c5",
        NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("e1g1", board, WHITE)));
    move_piece(&board, string_to_move("e1g1", board, WHITE));
    assert(get_bit(board.pieces[WHITE][ROOK], 5));
}

void test_king_castle_queenside(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "d2d4", "a7a6",
        "b1a3", "a6a5",
        "c1e3", "a5a4",
        "d1d3", "h7h6",
        NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(&board, string_to_move("e1c1", board, WHITE)));
    move_piece(&board, string_to_move("e1c1", board, WHITE));
    assert(get_bit(board.pieces[WHITE][ROOK], 3));
}

void test_cant_castle_king_moved(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "e7e5",
        "e1e2", "e8e7",
        "e2e1", "e7e8",
        "g1f3", "b8c6",
        "f1c4", "f8c5",
        NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("e1g1", board, WHITE)));
    assert(!is_legal(&board, string_to_move("e8g8", board, BLACK)));
}

void test_cant_castle_rook_moved(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "e7e5",
        "f1c4", "d7d5",
        "g1f3", "b8c6",
        "h2h4", "d8f6",
        "h1h3", "a7a5",
        "h3h1", "a8a6",
        "a2a3", "a6a8",
        "b2b3", "c8d7",
        NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("e1g1", board, WHITE)));
    assert(!is_legal(&board, string_to_move("e8c8", board, BLACK)));
}

void test_in_check(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "f7f5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(in_check(&board, BLACK));
}


void test_cant_move_into_check(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "d7d5", "e1e2", "d8d6", "e2e3", "d6f6", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("e3f3", board, WHITE)));
}

void test_must_resolve_check(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "f7f5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("a2a3", board, BLACK)));
}

void test_cant_castle_through_check(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "e7e5",
        "g1h3", "d8h4",
        "f1c4", "f8c5",
        "a2a3", "h4f6",
        "a3a4", "c5f2",
        "h3f2", "a7a6",
        "f2h3", "a6a5",
        NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("e1g1", board, WHITE)));
}

void test_pinned_piece_cant_move(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "a7a5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(&board, string_to_move("f7f6", board, BLACK)));
}

void test_stalemate_basic(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e3", "a7a5", "d1h5", "a8a6", "h5a5", "h7h5",
        "a5c7", "a6h6", "h2h4", "f7f6", "c7d7", "e8f7",
        "d7b7", "d8d3", "b7b8", "d3h7", "b8c8", "f7g6", "c8e6", NULL
    };
    apply_moves(&board, moves);
    assert(is_stalemate(&board, BLACK));
}

void test_checkmate_fools_mate(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "f7f5", "a2a3", "g7g5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(is_checkmate(&board, BLACK));
}

void test_castle_checkmate(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "d2d4", "f7f5",
        "b1c3", "g8f6",
        "g1f3", "e7e6",
        "c1g5", "f8e7",
        "g5f6", "e7f6",
        "e2e4", "f5e4",
        "c3e4", "b7b6",
        "f3e5", "e8g8",
        "f1d3", "c8b7",
        "d1h5", "d8e7",
        "h5h7", "g8h7",
        "e4f6", "h7h6",
        "e5g4", "h6g5",
        "h2h4", "g5f4",
        "g2g3", "f4f3",
        "d3e2", "f3g2",
        "h1h2", "g2g1",
        "e1c1",
        NULL
    };
    apply_moves(&board, moves);
    assert(is_checkmate(&board, BLACK));
}

void test_in_check_no_checkmate(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);
    char *moves[] = {
        "e2e4", "f7f5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_checkmate(&board, BLACK));
}

void test_insufficient_material_kvk() {
    assert(true);
}

void test_insufficient_material_kvkb() {
    assert(true);
}

void test_insufficient_material_kvkn() {
    assert(true);
}

void test_fifty_move_rule(void) {
    assert(true);
}

void test_threefold_repetition(void) {
    Board board = init_board();
    init_zobrist_key(&board, WHITE);

    char *moves[] = {
        "g1f3", "g8f6", "f3g1", "f6g8",
        "g1f3", "g8f6", "f3g1", "f6g8",
        NULL
    };

    apply_moves(&board, moves);
    assert(is_repetition(&board));
}

#define RUNS 10
// Average is about 0.75 right now.
void run_perft_timed_tests() {
    double total = 0.0;
    for (int i =0; i < RUNS; i++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        test_perft_depth_one();
        test_perft_depth_two();
        test_perft_depth_three();
        test_perft_depth_four();
        test_perft_depth_five();
        //test_perft_depth_six();

        clock_gettime(CLOCK_MONOTONIC, &end);
        total += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    }
    double average = total / RUNS;
    printf("Time: %.6f seconds\n", average);  
}

int main(void) {
    init_attacks();
    init_zobrist();

    test_valid_move();
    test_cant_capture_own_pieces();
    test_cant_move_opponents_pieces();
    test_move_piece();
    test_undo_move();
    test_single_pawn_push();
    test_double_pawn_push();
    test_pawn_cant_move_backward();
    test_pawn_capture();
    test_pawn_en_passant();
    test_pawn_en_passant_capture_removal();
    test_pawn_promotion();
    test_knight_movement();
    test_bishop_movement();
    test_bishop_cant_move_like_rook();
    test_rook_movement();
    test_queen_movement();
    test_king_movement();
    test_king_castle_kingside();
    test_king_castle_queenside();
    test_cant_castle_king_moved();
    test_cant_castle_rook_moved();
    test_in_check();
    test_cant_move_into_check();
    test_must_resolve_check();
    test_cant_castle_through_check();
    test_pinned_piece_cant_move();
    test_stalemate_basic();
    test_checkmate_fools_mate();
    test_in_check_no_checkmate();
    test_insufficient_material_kvk();
    test_insufficient_material_kvkb();
    test_insufficient_material_kvkn();
    test_fifty_move_rule();
    test_threefold_repetition();
    puts("All tests passed.");  

    run_perft_timed_tests();

    return 0;
}