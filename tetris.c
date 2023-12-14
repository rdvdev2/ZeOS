#include "utils.h"
#include <tetris.h>

#include <colors.h>
#include <libc.h>
#include <random.h>

const int TICKS_PER_FRAME = 10;

const enum CellState TETROMINOS[] = {
  CS_I_PIECE,
  CS_O_PIECE,
  CS_T_PIECE,
  CS_S_PIECE,
  CS_Z_PIECE,
  CS_J_PIECE,
  CS_L_PIECE,
};

void tetris_main() {
  struct SwappingGameState *gameState = (struct SwappingGameState *)memRegGet(
      sizeof(struct SwappingGameState) / 1024 + 1);
  if (gameState == NULL)
    return;
  init_swapping_game_state(gameState);
  srand(gettime());

  gameState->gameStates[0].currentPiece = CS_T_PIECE;
  gameState->gameStates[0].currentPieceX = TETRIS_COLS / 2;
  gameState->gameStates[0].currentPieceY = -4;
  gameState->gameStates[0].currentPieceRotations = 0;

  int lastUpdate = gettime();
  for(;;) {
    int current;
    do {
      current = gettime();
    } while (lastUpdate / TICKS_PER_FRAME == current / TICKS_PER_FRAME);
    lastUpdate = current;
    
    input(&gameState->gameStates[0]);
    update(&gameState->gameStates[0]);
    draw(&gameState->gameStates[0]);
  }
}

void input(struct GameState *state) {
  waitKey(&state->lastInput, 1);
}

void update(struct GameState *state) {
  int offsetX = 0;
  int offsetY = 1;

  switch (state->lastInput) {
    case 'a': --offsetX; break;
    case 'd': ++offsetX; break;
    default: break;
  }

  remove_piece(state, state->currentPieceX, state->currentPieceY, state->currentPiece, state->currentPieceRotations);
  if (can_put_piece(state, state->currentPieceX + offsetX, state->currentPieceY + offsetY, state->currentPiece, state->currentPieceRotations)) {
    state->currentPieceX += offsetX;
    state->currentPieceY += offsetY;
    put_piece(state, state->currentPieceX, state->currentPieceY, state->currentPiece, state->currentPieceRotations);
  } else if (can_put_piece(state, state->currentPieceX, state->currentPieceY + offsetY, state->currentPiece, state->currentPieceRotations)) {
    state->currentPieceY += offsetY;
    put_piece(state, state->currentPieceX, state->currentPieceY, state->currentPiece, state->currentPieceRotations);
  } else {
    put_piece(state, state->currentPieceX, state->currentPieceY, state->currentPiece, state->currentPieceRotations);

    state->currentPiece = TETROMINOS[rand() % 7];
    state->currentPieceX = TETRIS_COLS / 2;
    state->currentPieceY = -4;
    state->currentPieceRotations = 0;
  }
}

void init_swapping_game_state(struct SwappingGameState *state) {
  state->drawerIndex = 0;
  init_game_state(state->gameStates);
  init_game_state(state->gameStates + 1);
}

void init_game_state(struct GameState *state) {
  for (int i = 0; i < TETRIS_COLS; ++i) {
    for (int j = 0; j < TETRIS_ROWS + 4; ++j) {
      state->board[i][j] = CS_EMPTY;
    }
  }
}

int get_piece_index(enum CellState cs) {
  switch (cs) {
    case CS_I_PIECE: return 0;
    case CS_O_PIECE: return 1;
    case CS_T_PIECE: return 2;
    case CS_S_PIECE: return 3;
    case CS_Z_PIECE: return 4;
    case CS_J_PIECE: return 5;
    case CS_L_PIECE: return 6;
    case CS_CURRENT: return 7;
    default: for(;;);
  }
}

int can_put_piece(struct GameState* state, int x, int y, enum CellState cs, int rotations) {
  const char * _template = TETROMINO_ROTATIONS[get_piece_index(cs)][rotations % 4];

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      int _x = x + j;
      int _y = y + i;
      if (_template[i] & (1 << (3 - j))) {
        if (0 > _x || _x >= TETRIS_COLS || 0 > _y + 4 || _y >= TETRIS_ROWS) return 0;
        if (state->board[_x][_y+4] != CS_EMPTY) return 0;
      }
    }
  }

  return 1;
}

void put_piece(struct GameState* state, int x, int y, enum CellState cs, int rotations) {
  put_piece_raw(state, x, y, cs, TETROMINO_ROTATIONS[get_piece_index(cs)][rotations % 4]);
}

void remove_piece(struct GameState* state, int x, int y, enum CellState cs, int rotations) {
  put_piece_raw(state, x, y, CS_EMPTY, TETROMINO_ROTATIONS[get_piece_index(cs)][rotations % 4]);
}

void put_piece_raw(struct GameState* state, int x, int y, enum CellState cs, const char* _template) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      int _x = x + j;
      int _y = y + i;
      if (_template[i] & (1 << (3 - j)))
        state->board[_x][_y+4] = cs;
    }
  }
}

void draw(struct GameState* state) {
  const int TETROMINO_COLORS[] = {
    CYAN,
    YELLOW,
    MAGENTA,
    GREEN,
    RED,
    BLUE,
    GRAY,
    BLACK, // EMPTY
  };

  const int x_off = (SCREEN_COLUMNS - TETRIS_COLS) / 2;
  const int y_off = (SCREEN_ROWS - TETRIS_ROWS) / 2;

  changeColor(WHITE, WHITE);
  clrscr(NULL);

  for (int i = 0; i < TETRIS_COLS; ++i) {
    for (int j = 0; j < TETRIS_ROWS; ++j) {
      enum CellState cs = state->board[i][j+4];
      int idx = 0;
      while (!(cs & 1) && idx < 7) {
        cs >>= 1;
        ++idx;
      }

      gotoXY(x_off + i, y_off + j);
      changeColor(TETROMINO_COLORS[idx], BLACK);
      write(1, "O", 1);
    }
  }
}
