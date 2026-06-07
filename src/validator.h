#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "board.h"
#include <stdint.h>
#include <stdbool.h>

bool can_castle(Board* board, Color color, bool kingside);
bool valid_move(Board *board, Move move);
bool in_check(Board *board, Color color);
bool is_legal(Board* board, Move move);

#endif