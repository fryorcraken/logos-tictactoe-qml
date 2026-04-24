#include "libtictactoe.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Win patterns: indices into the flat 3x3 board array (row*3 + col). */
static const int WIN_LINES[8][3] = {
    /* rows */
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    /* cols */
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    /* diagonals */
    {0, 4, 8}, {2, 4, 6}
};

struct TicTacToeGame {
    TicTacToeCell   board[9];    /* flat 3x3: index = row*3 + col */
    TicTacToeCell   current;     /* whose turn: X or O            */
    TicTacToeStatus status;
    int             moves_made;
};

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

static TicTacToeStatus check_winner(const TicTacToeGame* game)
{
    int i;
    for (i = 0; i < 8; i++) {
        TicTacToeCell a = game->board[WIN_LINES[i][0]];
        TicTacToeCell b = game->board[WIN_LINES[i][1]];
        TicTacToeCell c = game->board[WIN_LINES[i][2]];
        if (a != TICTACTOE_CELL_EMPTY && a == b && b == c) {
            return (a == TICTACTOE_CELL_X)
                       ? TICTACTOE_STATUS_X_WINS
                       : TICTACTOE_STATUS_O_WINS;
        }
    }
    if (game->moves_made == 9)
        return TICTACTOE_STATUS_DRAW;
    return TICTACTOE_STATUS_ONGOING;
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

TicTacToeGame* tictactoe_new(void)
{
    TicTacToeGame* game = (TicTacToeGame*)malloc(sizeof(TicTacToeGame));
    if (!game)
        return NULL;
    tictactoe_reset(game);
    return game;
}

void tictactoe_free(TicTacToeGame* game)
{
    free(game); /* free(NULL) is defined to be a no-op */
}

void tictactoe_reset(TicTacToeGame* game)
{
    if (!game)
        return;
    memset(game->board, TICTACTOE_CELL_EMPTY, sizeof(game->board));
    game->current    = TICTACTOE_CELL_X;
    game->status     = TICTACTOE_STATUS_ONGOING;
    game->moves_made = 0;
}

TicTacToeError tictactoe_play(TicTacToeGame* game, int row, int col)
{
    int idx;

    if (!game)
        return TICTACTOE_ERR_NULL_GAME;
    if (game->status != TICTACTOE_STATUS_ONGOING)
        return TICTACTOE_ERR_GAME_OVER;
    if (row < 0 || row > 2 || col < 0 || col > 2)
        return TICTACTOE_ERR_INVALID_POSITION;

    idx = row * 3 + col;
    if (game->board[idx] != TICTACTOE_CELL_EMPTY)
        return TICTACTOE_ERR_CELL_OCCUPIED;

    game->board[idx] = game->current;
    game->moves_made++;

    game->status = check_winner(game);

    /* Always advance the turn; on game-over current becomes the player
       who would have moved next (i.e. the one who did not just win). */
    game->current = (game->current == TICTACTOE_CELL_X)
                        ? TICTACTOE_CELL_O
                        : TICTACTOE_CELL_X;

    return TICTACTOE_OK;
}

TicTacToeStatus tictactoe_status(const TicTacToeGame* game)
{
    if (!game)
        return TICTACTOE_STATUS_ONGOING;
    return game->status;
}

TicTacToeCell tictactoe_get_cell(const TicTacToeGame* game, int row, int col)
{
    if (!game || row < 0 || row > 2 || col < 0 || col > 2)
        return TICTACTOE_CELL_EMPTY;
    return game->board[row * 3 + col];
}

TicTacToeCell tictactoe_current_player(const TicTacToeGame* game)
{
    if (!game)
        return TICTACTOE_CELL_X;
    return game->current;
}

/* ------------------------------------------------------------------ */
/* Minimax AI                                                           */
/* ------------------------------------------------------------------ */

/* Score from the `ai` player's perspective:
   +10 minus depth if ai wins (prefer fast wins),
   depth minus 10 if ai loses (prefer slow losses),
   0 on draw. */
static int minimax(TicTacToeGame* g, TicTacToeCell ai, int depth, int maximizing)
{
    TicTacToeStatus s = check_winner(g);
    if (s != TICTACTOE_STATUS_ONGOING) {
        if (s == TICTACTOE_STATUS_DRAW)
            return 0;
        int ai_won =
            (s == TICTACTOE_STATUS_X_WINS && ai == TICTACTOE_CELL_X) ||
            (s == TICTACTOE_STATUS_O_WINS && ai == TICTACTOE_CELL_O);
        return ai_won ? (10 - depth) : (depth - 10);
    }

    int best = maximizing ? INT_MIN : INT_MAX;
    int i;
    for (i = 0; i < 9; i++) {
        if (g->board[i] != TICTACTOE_CELL_EMPTY)
            continue;

        TicTacToeCell saved = g->current;
        g->board[i] = saved;
        g->moves_made++;
        g->current = (saved == TICTACTOE_CELL_X)
                         ? TICTACTOE_CELL_O
                         : TICTACTOE_CELL_X;

        int score = minimax(g, ai, depth + 1, !maximizing);

        g->board[i] = TICTACTOE_CELL_EMPTY;
        g->moves_made--;
        g->current = saved;

        if (maximizing) {
            if (score > best) best = score;
        } else {
            if (score < best) best = score;
        }
    }
    return best;
}

int tictactoe_ai_move(const TicTacToeGame* game, int* out_row, int* out_col)
{
    if (!game || !out_row || !out_col)
        return -1;
    if (game->status != TICTACTOE_STATUS_ONGOING)
        return -1;

    /* Minimax mutates the board during search; work on a copy so we
       keep the const contract with the caller. */
    TicTacToeGame copy = *game;
    TicTacToeCell ai = copy.current;

    int best_score = INT_MIN;
    int best_idx = -1;
    int i;

    for (i = 0; i < 9; i++) {
        if (copy.board[i] != TICTACTOE_CELL_EMPTY)
            continue;

        copy.board[i] = ai;
        copy.moves_made++;
        copy.current = (ai == TICTACTOE_CELL_X)
                           ? TICTACTOE_CELL_O
                           : TICTACTOE_CELL_X;

        int score = minimax(&copy, ai, 1, 0);

        copy.board[i] = TICTACTOE_CELL_EMPTY;
        copy.moves_made--;
        copy.current = ai;

        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }

    if (best_idx < 0)
        return -1;

    *out_row = best_idx / 3;
    *out_col = best_idx % 3;
    return 0;
}
