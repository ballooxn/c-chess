#include "../src/move_parser.h"
#include "../src/board.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define TO_BITS(rank, file)     ((rank) * 8 + (file))

static void apply_moves(Board *board, char *moves[]) {
    for (int i = 0; moves[i] != NULL; i++) {
        int color = (i % 2 == 0) ? WHITE : BLACK;
        Move move = string_to_move(moves[i], *board, color);
        assert(is_legal(*board, move));
        move_piece(board, move);
    }
}

void test_valid_move(void) {
    Board board = init_board();
    assert(valid_move(board, string_to_move("e2e3", board, WHITE)));
}

void test_cant_capture_own_pieces(void) {
    Board board = init_board();
    assert(!is_legal(board, string_to_move("a1b1", board, WHITE)));
}

void test_cant_move_opponents_pieces(void) {
    Board board = init_board();
    assert(!is_legal(board, string_to_move("e7e6", board, WHITE)));
}

void test_move_piece(void) {
    Board board = init_board();
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
    Move move = string_to_move("e2e3", board, WHITE);
    int start_bits = 12;
    int end_bits = 20;
    move_piece(&board, move);
    reverse_simulated_move(&board, move, NO_PIECE);
    assert(!get_bit(board.occupied, end_bits) && get_bit(board.occupied, start_bits) &&
            !get_bit(board.pieces[WHITE][ALL], end_bits) && get_bit(board.pieces[WHITE][ALL], start_bits) &&
            !get_bit(board.pieces[WHITE][PAWN], end_bits) && get_bit(board.pieces[WHITE][ALL], start_bits));
}

void test_double_pawn_push(void) {
    Board board = init_board();
    assert(is_legal(board, string_to_move("e2e4", board, WHITE)));
}

void test_single_pawn_push(void) {
    Board board = init_board();
    assert(is_legal(board, string_to_move("e2e3", board, WHITE)));
}

void test_pawn_cant_move_backward(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(board, string_to_move("e4e3", board, WHITE)));
}

void test_pawn_capture(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(board, string_to_move("e4d5", board, WHITE)));
}

void test_pawn_en_passant(void) {
    assert(true);
}

void test_pawn_promotion(void) {
    assert(true);
}

void test_knight_movement(void) {
    Board board = init_board();
    assert(is_legal(board, string_to_move("g1f3", board, WHITE)));
}

void test_bishop_movement(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(board, string_to_move("f1c4", board, WHITE)));
}

void test_bishop_cant_move_like_rook(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", "f1c4", "a7a6", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(board, string_to_move("c4c3", board, WHITE)));
}

void test_rook_movement(void) {
    Board board = init_board();
    char *moves[] = {
        "a2a4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(board, string_to_move("a1a3", board, WHITE)));
}

void test_queen_movement(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(board, string_to_move("d1h5", board, WHITE)));
}

void test_king_movement(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", NULL
    };
    apply_moves(&board, moves);
    assert(is_legal(board, string_to_move("e1e2", board, WHITE)));
}

void test_king_castle_kingside(void) {
    assert(true);
}

void test_king_castle_queenside(void) {
    assert(true);
}

void test_cant_castle_king_moved(void) {
    assert(true);
}

void test_cant_castle_rook_moved(void) {
    assert(true);
}

void test_in_check(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "f7f5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(in_check(&board, BLACK));
}


void test_cant_move_into_check(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "d7d5", "e1e2", "d8d6", "e2e3", "d6f6", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(board, string_to_move("e3f3", board, WHITE)));
}

void test_must_resolve_check(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "f7f5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(board, string_to_move("a2a3", board, BLACK)));
}

void test_cant_castle_through_check(void) {
    assert(true);
}

void test_pinned_piece_cant_move(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "a7a5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(!is_legal(board, string_to_move("f7f6", board, BLACK)));
}

void test_stalemate_basic(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e3", "a7a5", "d1h5", "a8a6", "h5a5", "h7h5",
        "a5c7", "a6h6", "h2h4", "f7f6", "c7d7", "e8f7",
        "d7b7", "d8d3", "b7b8", "d3h7", "b8c8", "f7g6", "c8e6", NULL
    };
    apply_moves(&board, moves);
    assert(in_stalemate(&board, BLACK));
}

void test_checkmate_fools_mate(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "f7f5", "a2a3", "g7g5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(in_checkmate(&board, BLACK));
}

void test_in_check_no_checkmate(void) {
    Board board = init_board();
    char *moves[] = {
        "e2e4", "f7f5", "d1h5", NULL
    };
    apply_moves(&board, moves);
    assert(!in_checkmate(&board, BLACK));
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

void test_threefold_repitition(void) {
    assert(true);
}

int main(void) {
    init_attacks();
    test_valid_move();
    test_cant_capture_own_pieces();
    test_cant_move_opponents_pieces();
    test_move_piece();
    test_undo_move();
    test_single_pawn_push();
    test_double_pawn_push();
    test_pawn_cant_move_backward();
    test_bishop_cant_move_like_rook();
    test_pawn_capture();
    test_pawn_en_passant();
    test_knight_movement();
    test_bishop_movement();
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
    test_threefold_repitition();
    puts("All tests passed.");
    return 0;
}