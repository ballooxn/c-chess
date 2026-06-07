#ifndef MOVE_PARSER_H
#define MOVE_PARSER_H

#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

bool valid_input(char* pos);
char* player_input(char* buffer, size_t size);
char get_promotion_choice(Color color);
Move string_to_move(char* string, Board board, Color color);

#endif