#pragma once

#include "common/ChatterinoSetting.hpp"
#include "common/SignalVector.hpp"
#include "common/Singleton.hpp"
#include "controllers/commands/Command.hpp"
#include "providers/twitch/TwitchChannel.hpp"

#include <QMap>
#include <pajlada/settings.hpp>

#include <memory>
#include <mutex>

namespace chatterino {

class Settings;
class Paths;
class Channel;

class CommandModel;

class CommandController final : public Singleton
{
public:
    SignalVector<Command> items_;

    QString execCommand(const QString &text, std::shared_ptr<Channel> channel,
                        bool dryRun);
    QStringList getDefaultTwitchCommandList();

    virtual void initialize(Settings &, Paths &paths) override;
    virtual void save() override;

    CommandModel *createModel(QObject *parent);

private:
    const int typoRate_ = 10;
    const QMap<QChar, QString> keyMap_{
        {'1', "`2q"},    {'2', "1qw3"},   {'3', "2we4"},   {'4', "3er5"},
        {'5', "4rt6"},   {'6', "5ty7"},   {'7', "6yu8"},   {'8', "7ui9"},
        {'9', "8io0"},   {'0', "9op-"},   {'q', "aw21"},   {'w', "qase32"},
        {'e', "wsdr43"}, {'r', "edft54"}, {'t', "rfgy65"}, {'y', "tghu76"},
        {'u', "yhji87"}, {'i', "ujko98"}, {'o', "iklp09"}, {'p', "ol;[-0"},
        {'a', "zswq"},   {'s', "azxdew"}, {'d', "sxcfre"}, {'f', "dcvgtr"},
        {'g', "fvbhyt"}, {'h', "gbnjuy"}, {'j', "hnmkiu"}, {'k', "jm,loi"},
        {'l', "k,.;po"}, {'z', "xsa"},    {'x', "zcds"},   {'c', "xvfd"},
        {'v', "cbgf"},   {'b', "vnhg"},   {'n', "bmjh"},   {'m', "n,kj"},
        {'!', "~Q@"},    {'@', "~QW#"},   {'#', "@WE$"},   {'$', "#ER%"},
        {'%', "$RT^"},   {'^', "%TY&"},   {'&', "^YU*"},   {'*', "&UI("},
        {'(', "*IO)"},   {')', "(OP_"},   {'Q', "AW@!"},   {'W', "QASE#@"},
        {'E', "WSDR$#"}, {'R', "EDFT%$"}, {'T', "RFGY^%"}, {'Y', "TGHU&^"},
        {'U', "YHJI*&"}, {'I', "UJKO(*"}, {'O', "IKLP)("}, {'P', "OL:{_)"},
        {'A', "ZSWQ"},   {'S', "AZXDEW"}, {'D', "SXCFRE"}, {'F', "DCVGTR"},
        {'G', "FVBHYT"}, {'H', "GBNJUY"}, {'J', "HNMKIU"}, {'K', "JM<LOI"},
        {'L', "K<>:PO"}, {'Z', "XSA"},    {'X', "ZCDS"},   {'C', "XVFD"},
        {'V', "CBGF"},   {'B', "VNHG"},   {'N', "BMJH"},   {'M', "N<KJ"}};

    void load(Paths &paths);

    using CommandFunction =
        std::function<QString(QStringList /*words*/, ChannelPtr /*channel*/)>;

    void registerCommand(QString commandName, CommandFunction commandFunction);

    // Chatterino commands
    QMap<QString, CommandFunction> commands_;

    // User-created commands
    QMap<QString, Command> userCommands_;
    int maxSpaces_ = 0;

    std::shared_ptr<pajlada::Settings::SettingManager> sm_;
    // Because the setting manager is not initialized until the initialize
    // function is called (and not in the constructor), we have to
    // late-initialize the setting, which is why we're storing it as a
    // unique_ptr
    std::unique_ptr<pajlada::Settings::Setting<std::vector<Command>>>
        commandsSetting_;

    QString execCustomCommand(const QStringList &words, const Command &command,
                              bool dryRun, bool hasTypo = false);

    QString generateTypo(const QString &text);

    QStringList commandAutoCompletions_;
};

}  // namespace chatterino
