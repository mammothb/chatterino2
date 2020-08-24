#pragma once

#include <QObject>

#include "common/SignalVectorModel.hpp"

namespace chatterino {

class IgnoreEmoteModel : public SignalVectorModel<QString>
{
public:
    explicit IgnoreEmoteModel(QObject *parent);

protected:
    // turn a vector item into a model row
    virtual QString getItemFromRow(std::vector<QStandardItem *> &row,
                                   const QString &original) override;

    // turns a row in the model into a vector item
    virtual void getRowFromItem(const QString &item,
                                std::vector<QStandardItem *> &row) override;
};

}  // namespace chatterino
