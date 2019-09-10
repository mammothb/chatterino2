#pragma once

#include <rapidjson/rapidjson.h>
#include <common/SignalVector.hpp>

#include "providers/irc/IrcChannel2.hpp"
#include "providers/irc/IrcServer.hpp"

class QAbstractTableModel;

namespace chatterino {

//enum IrcAuthType { Anonymous, /*Sals,*/ Pass, MsgNickServ, NickServ };

struct IrcConnection_ {
    QString host;
    int port = 6667;
    bool ssl = false;

    QString user;
    QString nick;
    QString real;

    //    IrcAuthType authType = Anonymous;
    QString password;

    int id;
};

class Irc
{
public:
    Irc();

    static Irc &getInstance();

    UnsortedSignalVector<IrcConnection_> connections;
    QAbstractTableModel *newConnectionModel(QObject *parent);

    ChannelPtr getOrAddChannel(int serverId, QString name);

    void save();
    void load();

    int uniqueId();

signals:
    void connectionUpdated(int id);

private:
    int currentId_{};
    bool loaded_{};

    // Servers have a unique id.
    // When a server gets changed it gets removed and then added again.
    // So we store the channels of that server in abandonedChannels_ temporarily.
    // Or if the server got removed permanently then it's still stored there.
    std::unordered_map<int, std::unique_ptr<IrcServer>> servers_;
    std::unordered_map<int, std::vector<std::weak_ptr<Channel>>>
        abandonedChannels_;
};

}  // namespace chatterino