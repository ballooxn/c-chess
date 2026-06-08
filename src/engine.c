#include "board.h"
#include "move_parser.h"
#include "validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

Move engine_move(Board *board, Color color) {
    // Dont forget to tack on the promotion piece to the Move struct
    // and is_castling and is_enpassant
}