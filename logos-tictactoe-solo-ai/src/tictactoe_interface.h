#ifndef TICTACTOE_INTERFACE_H
#define TICTACTOE_INTERFACE_H

#include <QObject>
#include "interface.h"

/**
 * Interface for the tictactoe_solo_ai module.
 *
 * All enum values are returned as plain int so QML callers don't need
 * to include the C header.
 *
 * TicTacToeCell  : 0=EMPTY, 1=X, 2=O
 * TicTacToeStatus: 0=ONGOING, 1=X_WINS, 2=O_WINS, 3=DRAW
 * TicTacToeError : 0=OK, 1=INVALID_POSITION, 2=CELL_OCCUPIED,
 *                  3=GAME_OVER, 4=NULL_GAME
 */
class TicTacToeInterface : public PluginInterface
{
public:
    virtual ~TicTacToeInterface() = default;

    /**
     * Start a new game. X always moves first. If vsAi is true, the
     * plugin plays O's replies automatically inside play().
     */
    Q_INVOKABLE virtual void newGame(bool vsAi) = 0;

    /**
     * Place the current player's mark at (row, col). When AI mode is on
     * and O is next, the AI's reply is applied before this returns.
     * Returns TicTacToeError as int (0 = OK).
     */
    Q_INVOKABLE virtual int play(int row, int col) = 0;

    /** Content of (row, col) as a TicTacToeCell int. */
    Q_INVOKABLE virtual int getCell(int row, int col) = 0;

    /** Current game status as a TicTacToeStatus int. */
    Q_INVOKABLE virtual int status() = 0;

    /** Whose turn it is as a TicTacToeCell int (1=X, 2=O). */
    Q_INVOKABLE virtual int currentPlayer() = 0;

    /** Whether AI mode is on for the current game. */
    Q_INVOKABLE virtual bool vsAiEnabled() = 0;
};

#define TicTacToeInterface_iid "org.logos.TicTacToeInterface"
Q_DECLARE_INTERFACE(TicTacToeInterface, TicTacToeInterface_iid)

#endif // TICTACTOE_INTERFACE_H
