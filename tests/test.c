#include "../src/move_parser.h"
#include "../src/board.h"
#include <stdio.h>
#include <assert.h>

static void apply_moves(Board *board, char *moves[]) {
    for (int i = 0; moves[i] != NULL; i++) {
        int color = (i % 2 == 0) ? WHITE : BLACK;
        Move move = string_to_move(moves[i], *board, color);
        assert(is_legal(*board, move));
        move_piece(board, move);
    }
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

int main(void) {
    init_attacks();
    test_stalemate_basic();
    test_checkmate_fools_mate();
    puts("All tests passed.");
    return 0;
}