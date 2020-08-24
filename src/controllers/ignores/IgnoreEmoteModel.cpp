#include "IgnoreEmoteModel.hpp"

#include "Application.hpp"
#include "singletons/Settings.hpp"
#include "util/StandardItemHelper.hpp"

namespace chatterino {

// commandmodel
IgnoreEmoteModel::IgnoreEmoteModel(QObject *parent)
    : SignalVectorModel<QString>(1, parent)
{
}

// turn a vector item into a model row
QString IgnoreEmoteModel::getItemFromRow(std::vector<QStandardItem *> &row,
                                         const QString &original)
{
    // key, regex

    return row[0]->data(Qt::DisplayRole).toString();
}

// turns a row in the model into a vector item
void IgnoreEmoteModel::getRowFromItem(const QString &item,
                                      std::vector<QStandardItem *> &row)
{
    setStringItem(row[0], item);
}

}  // namespace chatterino
