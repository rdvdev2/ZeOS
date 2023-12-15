#include <tetris.h>

#include <colors.h>
#include <libc.h>
#include <random.h>

const int TICKS_PER_FRAME = 1;

const enum CellState TETROMINOS[] = {
    CS_I_PIECE, CS_O_PIECE, CS_T_PIECE, CS_S_PIECE,
    CS_Z_PIECE, CS_J_PIECE, CS_L_PIECE,
};

void input_thread(void *state) {
  for (;;)
    input(state);
}

void render_thread(void *state) {
  struct GameState *liveState = state;
  struct GameState renderState;

  for (;;) {
    // We will copy the state when available and then allow the next update to
    // be processed while we render the frame.
    semWait(liveState->state_copy);

    renderState.isGameOver = liveState->isGameOver;
    renderState.score = liveState->score;
    if (!renderState.isGameOver) { // Board is static in game over
      memcpy(renderState.board, liveState->board, sizeof(liveState->board));
    }

    semSignal(liveState->state_copy_done);

    draw(state);
  }
}

void tetris_main() {
  struct GameState *gameState =
      (struct GameState *)memRegGet(sizeof(struct GameState) / 1024 + 1);
  if (gameState == NULL)
    return;

  gameState->state_copy = (sem_t *)-1;
  gameState->state_copy_done = (sem_t *)-1;
  init_game_state(gameState);

  srand(gettime());

  gameState->currentPiece = CS_T_PIECE;
  gameState->currentPieceX = TETRIS_COLS / 2;
  gameState->currentPieceY = -4;
  gameState->currentPieceRotations = 0;

  if (threadCreateWithStack(input_thread, 1, gameState) != 0)
    return;
  if (threadCreateWithStack(render_thread, 1, gameState) != 0)
    return;

  int lastUpdate = gettime();
  for (;;) {
    int current;
    do {
      current = gettime();
    } while (lastUpdate / TICKS_PER_FRAME == current / TICKS_PER_FRAME);
    gameState->ticks = lastUpdate = current;

    update(gameState);

    // Let the render thread copy the state before processing the next update
    semSignal(gameState->state_copy);
    semWait(gameState->state_copy_done);
  }
}

void input(struct GameState *state) { waitKey(&state->lastInput, 2147483647); }

int try_apply(struct GameState *state, int offsetX, int offsetY,
              int offsetRotation) {
  remove_piece(state, state->currentPieceX, state->currentPieceY,
               state->currentPiece, state->currentPieceRotations);
  if (can_put_piece(state, state->currentPieceX + offsetX,
                    state->currentPieceY + offsetY, state->currentPiece,
                    state->currentPieceRotations + offsetRotation)) {
    state->currentPieceX += offsetX;
    state->currentPieceY += offsetY;
    state->currentPieceRotations += offsetRotation;

    put_piece(state, state->currentPieceX, state->currentPieceY,
              state->currentPiece, state->currentPieceRotations);

    return 1;
  } else {
    put_piece(state, state->currentPieceX, state->currentPieceY,
              state->currentPiece, state->currentPieceRotations);

    return 0;
  }
}

void hard_drop(struct GameState *state) {
  while (try_apply(state, 0, 1, 0))
    state->score += 2;
}

void update(struct GameState *state) {
  if (state->isGameOver) {
    if (state->lastInput == 'r') {
      init_game_state(state);
    } else {
      return;
    }
  }

  int offsetX = 0;
  int offsetY = state->ticks % (TICKS_PER_FRAME * 5) == 0 ? 1 : 0;
  int offsetRotation = 0;

  switch (state->lastInput) {
  case 's':
    hard_drop(state);
    break;
  case 'a':
    --offsetX;
    break;
  case 'd':
    ++offsetX;
    break;
  case 'q':
    --offsetRotation;
    break;
  case 'e':
    ++offsetRotation;
    break;
  default:
    break;
  }
  state->lastInput = 0;

  if (try_apply(state, offsetX, offsetY, offsetRotation) ||
      (offsetX != 0 && try_apply(state, offsetX, 0, 0)) ||
      try_apply(state, 0, offsetY, 0)) {
    // Move suceeded
  } else {

    int line_clears = 0;
    for (int i = 0; i < TETRIS_ROWS; ++i) {
      int is_line = 1;
      for (int j = 0; j < TETRIS_COLS; ++j) {
        if (state->board[j][i + 4] == CS_EMPTY) {
          is_line = 0;
          break;
        }
      }

      if (is_line) {
        for (int x = 0; x < TETRIS_COLS; ++x) {
          for (int y = i; y >= -1; --y) {
            state->board[x][y + 4] = state->board[x][y + 3];
          }
        }

        ++line_clears;
        int clear_level = TETRIS_ROWS - i;
        state->score += clear_level * 100 * line_clears;
      }
    }

    for (int x = 0; x < TETRIS_COLS; ++x) {
      for (int y = 0; y < 4; ++y) {
        if (state->board[x][y] != CS_EMPTY) {
          state->isGameOver = 1;
          return;
        }
      }
    }

    state->currentPiece = TETROMINOS[rand() % 7];
    state->currentPieceX = TETRIS_COLS / 2;
    state->currentPieceY = -4;
    state->currentPieceRotations = 0;
  }
}

void init_game_state(struct GameState *state) {
  for (int i = 0; i < TETRIS_COLS; ++i) {
    for (int j = 0; j < TETRIS_ROWS + 4; ++j) {
      state->board[i][j] = CS_EMPTY;
    }
  }

  if (state->state_copy == (sem_t *)-1) {
    state->state_copy = semCreate(0);
    if ((int)state->state_copy == -1) {
      perror();
      for (;;)
        ;
    }
  }

  if (state->state_copy_done == (sem_t *)-1) {
    state->state_copy_done = semCreate(0);
    if ((int)state->state_copy_done == -1) {
      perror();
      for (;;)
        ;
    }
  }

  state->isGameOver = 0;
  state->score = 0;
}

int get_piece_index(enum CellState cs) {
  switch (cs) {
  case CS_I_PIECE:
    return 0;
  case CS_O_PIECE:
    return 1;
  case CS_T_PIECE:
    return 2;
  case CS_S_PIECE:
    return 3;
  case CS_Z_PIECE:
    return 4;
  case CS_J_PIECE:
    return 5;
  case CS_L_PIECE:
    return 6;
  case CS_CURRENT:
    return 7;
  default:
    for (;;)
      ;
  }
}

int can_put_piece(struct GameState *state, int x, int y, enum CellState cs,
                  int rotations) {
  const char *_template =
      TETROMINO_ROTATIONS[get_piece_index(cs)][rotations & 0b11];

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      int _x = x + j;
      int _y = y + i;
      if (_template[i] & (1 << (3 - j))) {
        if (0 > _x || _x >= TETRIS_COLS || 0 > _y + 4 || _y >= TETRIS_ROWS)
          return 0;
        if (state->board[_x][_y + 4] != CS_EMPTY)
          return 0;
      }
    }
  }

  return 1;
}

void put_piece(struct GameState *state, int x, int y, enum CellState cs,
               int rotations) {
  put_piece_raw(state, x, y, cs,
                TETROMINO_ROTATIONS[get_piece_index(cs)][rotations & 0b11]);
}

void remove_piece(struct GameState *state, int x, int y, enum CellState cs,
                  int rotations) {
  put_piece_raw(state, x, y, CS_EMPTY,
                TETROMINO_ROTATIONS[get_piece_index(cs)][rotations & 0b11]);
}

void put_piece_raw(struct GameState *state, int x, int y, enum CellState cs,
                   const char *_template) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      int _x = x + j;
      int _y = y + i;
      if (_template[i] & (1 << (3 - j)))
        state->board[_x][_y + 4] = cs;
    }
  }
}

void draw(struct GameState *state) {
  const int TETROMINO_COLORS[] = {
      CYAN,  YELLOW, MAGENTA, GREEN, RED, BLUE, GRAY,
      BLACK, // EMPTY
  };

  const int x_off = (SCREEN_COLUMNS - TETRIS_COLS) / 2;
  const int y_off = (SCREEN_ROWS - TETRIS_ROWS) / 2;

  short screen_buff[SCREEN_ROWS][SCREEN_COLUMNS] = {};
  for (int i = 0; i < SCREEN_ROWS; ++i) {
    for (int j = 0; j < SCREEN_COLUMNS; ++j) {
      screen_buff[i][j] = WHITE << 12 | WHITE << 12 | ' ';
    }
  }

  for (int i = 0; i < TETRIS_COLS; ++i) {
    for (int j = 0; j < TETRIS_ROWS; ++j) {
      enum CellState cs = state->board[i][j + 4];
      int idx = 0;
      while (!(cs & 1) && idx < 7) {
        cs >>= 1;
        ++idx;
      }

      screen_buff[y_off + j][x_off + i] =
          BLACK << 12 | TETROMINO_COLORS[idx] << 8 | 'O';
    }
  }

  if (clrscr((char *)screen_buff) < 0) {
    perror();
    for (;;)
      ;
  }

  if (state->isGameOver) {
    changeColor(WHITE, BRIGHT_RED);

    char msg[] = "GAME OVER!";
    int msg_len = strlen(msg);
    gotoXY((SCREEN_COLUMNS - msg_len) / 2, SCREEN_ROWS / 2 - 1);
    write(1, msg, msg_len);

    char msg2[SCREEN_COLUMNS] = "SCORE: ";
    itoa(state->score, &msg2[strlen(msg2)], 10);
    msg_len = strlen(msg2);
    gotoXY((SCREEN_COLUMNS - msg_len) / 2, SCREEN_ROWS / 2 + 1);
    write(1, msg2, msg_len);
  } else {
    changeColor(BRIGHT_CYAN, WHITE);

    char msg[SCREEN_COLUMNS];
    itoa(state->score, msg, 10);
    int msg_len = strlen(msg);
    gotoXY(SCREEN_COLUMNS - msg_len - 1, 1);
    write(1, msg, msg_len);
  }
}
