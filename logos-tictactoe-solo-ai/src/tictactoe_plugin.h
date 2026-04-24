#ifndef TICTACTOE_PLUGIN_H
#define TICTACTOE_PLUGIN_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include "tictactoe_interface.h"
#include "libtictactoe.h"

class LogosAPI;

class TicTacToePlugin : public QObject, public TicTacToeInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TicTacToeInterface_iid FILE "metadata.json")
    Q_INTERFACES(TicTacToeInterface PluginInterface)

public:
    explicit TicTacToePlugin(QObject* parent = nullptr);
    ~TicTacToePlugin() override;

    QString name()    const override { return "tictactoe_solo_ai"; }
    QString version() const override { return "1.0.0"; }

    // NOT marked override — it is invoked reflectively via QMetaObject.
    Q_INVOKABLE void initLogos(LogosAPI* api);

    Q_INVOKABLE void newGame(bool vsAi)        override;
    Q_INVOKABLE int  play(int row, int col)    override;
    Q_INVOKABLE int  getCell(int row, int col) override;
    Q_INVOKABLE int  status()                  override;
    Q_INVOKABLE int  currentPlayer()           override;
    Q_INVOKABLE bool vsAiEnabled()             override;

signals:
    void eventResponse(const QString& eventName, const QVariantList& args);

private:
    TicTacToeGame* m_game = nullptr;
    bool           m_vsAi = false;
};

#endif // TICTACTOE_PLUGIN_H
