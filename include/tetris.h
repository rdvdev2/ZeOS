#ifndef TETRIS_H
#define TETRIS_H

#include <libc.h>

#define TETRIS_COLS 10
#define TETRIS_ROWS 20

enum CellState {
  CS_EMPTY = 0b00000000,
  CS_I_PIECE = 0b00000001,
  CS_O_PIECE = 0b00000010,
  CS_T_PIECE = 0b00000100,
  CS_S_PIECE = 0b00001000,
  CS_Z_PIECE = 0b00010000,
  CS_J_PIECE = 0b00100000,
  CS_L_PIECE = 0b01000000,
  CS_CURRENT = 0b10000000,
};

struct GameState {
  enum CellState board[TETRIS_COLS][TETRIS_ROWS + 4];
  enum CellState currentPiece;
  int currentPieceX, currentPieceY, currentPieceRotations;
  char lastInput;
  int ticks;
  sem_t *state_copy, *state_copy_done;
  int isGameOver;
  int score;
};

const char TETROMINO_ROTATIONS[7][4][4];

void tetris_main();

void init_game_state(struct GameState *state);

int can_put_piece(struct GameState *state, int x, int y, enum CellState cs,
                  int rotations);
void put_piece(struct GameState *state, int x, int y, enum CellState cs,
               int rotations);
void remove_piece(struct GameState *state, int x, int y, enum CellState cs,
                  int rotations);
void put_piece_raw(struct GameState *state, int x, int y, enum CellState cs,
                   const char *_template);

void input(struct GameState *state);
void update(struct GameState *state);
void draw(struct GameState *state);
#endif
