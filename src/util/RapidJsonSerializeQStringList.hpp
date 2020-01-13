#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QStringList>
#include <pajlada/serialize.hpp>

namespace pajlada {

template <>
struct Serialize<QStringList> {
    static rapidjson::Value get(const QStringList &value,
                                rapidjson::Document::AllocatorType &a)
    {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out << value;
        return rapidjson::Value(QString(byteArray.toBase64()).toUtf8(), a);
    }
};

template <>
struct Deserialize<QStringList> {
    static QStringList get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsString())
        {
            PAJLADA_REPORT_ERROR(error)
            return QStringList{};
        }

        try
        {
            QStringList res;
            QByteArray byteArray = QByteArray::fromBase64(value.GetString());
            QDataStream in(&byteArray, QIODevice::ReadOnly);
            in >> res;
            return res;
        }
        catch (const std::exception &)
        {
            //            int x = 5;
        }
        catch (...)
        {
            //            int y = 5;
        }

        return QStringList{};
    }
};

}  // namespace pajlada
