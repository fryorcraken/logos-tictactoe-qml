#include "tictactoe_plugin.h"
#include "logos_api.h"

#include <QDebug>
#include <QVariantList>

TicTacToePlugin::TicTacToePlugin(QObject* parent)
    : QObject(parent)
    , m_game(tictactoe_new())
{
}

TicTacToePlugin::~TicTacToePlugin()
{
    tictactoe_free(m_game);
}

void TicTacToePlugin::initLogos(LogosAPI* api)
{
    logosAPI = api;
    emit eventResponse("ready",
        QVariantList() << name() << version());
}

void TicTacToePlugin::newGame(bool vsAi)
{
    tictactoe_reset(m_game);
    m_vsAi = vsAi;
    emit eventResponse("newGame", QVariantList() << vsAi);
}

int TicTacToePlugin::play(int row, int col)
{
    TicTacToeError err = tictactoe_play(m_game, row, col);
    if (err != TICTACTOE_OK)
        return static_cast<int>(err);

    emit eventResponse("played",
        QVariantList()
            << row << col
            << static_cast<int>(tictactoe_status(m_game)));

    if (m_vsAi
        && tictactoe_status(m_game) == TICTACTOE_STATUS_ONGOING
        && tictactoe_current_player(m_game) == TICTACTOE_CELL_O) {
        int aiRow = -1, aiCol = -1;
        if (tictactoe_ai_move(m_game, &aiRow, &aiCol) == 0) {
            (void)tictactoe_play(m_game, aiRow, aiCol);
            emit eventResponse("aiMoved",
                QVariantList()
                    << aiRow << aiCol
                    << static_cast<int>(tictactoe_status(m_game)));
        }
    }

    return static_cast<int>(TICTACTOE_OK);
}

int TicTacToePlugin::getCell(int row, int col)
{
    return static_cast<int>(tictactoe_get_cell(m_game, row, col));
}

int TicTacToePlugin::status()
{
    return static_cast<int>(tictactoe_status(m_game));
}

int TicTacToePlugin::currentPlayer()
{
    return static_cast<int>(tictactoe_current_player(m_game));
}

bool TicTacToePlugin::vsAiEnabled()
{
    return m_vsAi;
}
