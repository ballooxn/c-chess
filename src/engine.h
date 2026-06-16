#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

#define FLIP_BOARD_NUM 56
#define MATE_EVAL 100000
#define INF 1000000

// file masks
#define FILE_A 0x0101010101010101ULL
#define FILE_B (FILE_A << 1)
#define FILE_C (FILE_A << 2)
#define FILE_D (FILE_A << 3)
#define FILE_E (FILE_A << 4)
#define FILE_F (FILE_A << 5)
#define FILE_G (FILE_A << 6)
#define FILE_H (FILE_A << 7)

#define MAX_PHASE 24
#define MIDDLE_LIMIT 12
#define END_LIMIT 4

// pawn scores
#define ISOLATED_SCORE -10
#define DOUBLED_PAWN_SCORE -25

// king safety scores
#define KING_CENTER_SCORE -50
#define PAWN_ONE_RANK_SCORE -10
#define PAWN_TWO_RANKS_SCORE -20
#define PAWN_MISSING -35

#define TEMPO_BONUS 15

Move engine_move(Board *board, Color color);

#endif