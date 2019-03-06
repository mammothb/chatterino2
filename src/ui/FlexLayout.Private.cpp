#define FLEXLAYOUTPRIVATE public:

#include "FlexLayout.Private.hpp"

#include "FlexLayout.hpp"

#include <QWidget>

namespace chatterino::ui
{
    namespace
    {
        bool isHorizontal(Direction direction)
        {
            return direction == Direction::Left ||
                   direction == Direction::Right;
        }

        bool isVertical(Direction direction)
        {
            return !isHorizontal(direction);
        }

        bool isBefore(Direction direction)
        {
            return direction == Direction::Left ||
                   direction == Direction::Above;
        }

        bool isAfter(Direction direction)
        {
            return !isBefore(direction);
        }
    }  // namespace

    FlexItem::FlexItem()
        : type_(FlexItemType::Empty)
    {
    }

    FlexItem::FlexItem(QLayoutItem* item)
        : type_(FlexItemType::Item)
        , item_(item)
    {
        assert(item != nullptr);
    }

    FlexItem::FlexItem(FlexItemType type)
        : type_(type)
    {
        assert(type != FlexItemType::Item);
    }

    bool FlexItem::isEmpty()
    {
        return this->type_ == Empty;
    }

    bool FlexItem::isItem()
    {
        return this->type_ == Item;
    }

    bool FlexItem::isColumn()
    {
        return this->type_ == Column;
    }

    bool FlexItem::isRow()
    {
        return this->type_ == Row;
    }

    std::shared_ptr<FlexItem> FlexItem::find(QLayoutItem* widget)
    {
        if (!widget)
            return nullptr;

        // check if owned by self
        if (this->isItem() && this->item_ == widget)
            return this->shared_from_this();

        // search in items
        for (auto&& item : this->items_)
            if (auto result = item->find(widget))
                return result;

        // not found
        return nullptr;
    }

    void FlexItem::insert(
        QLayoutItem* item, QLayoutItem* relativeTo, Direction direction)
    {
        // this may only be called on the top level widget

        auto element = std::make_shared<FlexItem>(item);
        auto relativeToItem = this->find(relativeTo);

        // try to insert next to `relativeTo`
        if (!this->insertRec(element, nullptr, relativeToItem.get(), direction))
        {
            if (this->isItem())
            {
                // try to insert next to ourselves
                auto child = this->clone();
                this->clear();
                this->type_ = isHorizontal(direction) ? Row : Column;

                this->items_.append(child);
                this->items_.insert(isBefore(direction) ? 0 : 1,
                    std::make_shared<FlexItem>(item));
            }
            else
            {
                this->add(item);
            }
        }
    }

    bool FlexItem::insertRec(const std::shared_ptr<FlexItem>& item,
        FlexItem* parent, FlexItem* relativeTo, Direction direction)
    {
        auto replaceWithContainer = [&]() {
            auto clone = this->clone();
            this->clear();
            this->type_ = isVertical(direction) ? Column : Row;
            this->items_.append(clone);
            this->items_.insert(isBefore(direction) ? 0 : 1, item);
        };

        if (this == relativeTo)
        {
            if (parent == nullptr)
            {
                if (this->isEmpty())
                {
                    // is empty -> replace with item
                    this->clear();
                    this->type_ = Item;
                    this->item_ = item->item_;
                }
                else if (this->isItem())
                {
                    replaceWithContainer();
                }
                else if (this->isColumn() || this->isRow())
                {
                    if (isVertical(direction) == this->isColumn())
                    {
                        // should never happen
                        // same direction -> add to container.
                        if (isBefore(direction))
                            this->items_.insert(0, item);
                        else
                            this->items_.append(item);
                    }
                    else
                    {
                        replaceWithContainer();
                    }
                }
            }
            else
            {
                if (this->isEmpty())
                {
                    assert(false);
                }
                else
                {
                    if (parent->isColumn() == isVertical(direction))
                    {
                        // insert next to item in parent container
                        auto index =
                            parent->items_.indexOf(this->shared_from_this());
                        parent->items_.insert(
                            index + (isBefore(direction) ? 0 : 1), item);
                    }
                    else
                    {
                        replaceWithContainer();
                    }
                }
            }

            this->flatten();

            return true;
        }
        else
        {
            // search in items
            if (this->isColumn() || this->isRow())
                for (auto&& child : this->items_)
                    if (child->insertRec(item, this, relativeTo, direction))
                        return true;

            // not found
            return false;
        }
    }

    void FlexItem::add(QLayoutItem* item)
    {
        if (this->isEmpty())
        {
            this->item_ = item;
            this->type_ = Item;
        }
        else if (this->isRow())
        {
            this->items_.append(std::make_shared<FlexItem>(item));
        }
        // column or item
        else
        {
            // clone this and add the clone and item to ourselves
            auto clone = this->clone();
            this->clear();
            this->type_ = FlexItem::Column;

            this->items_.append(clone);
            this->items_.append(std::make_shared<FlexItem>(item));
        }
    }

    QLayoutItem* FlexItem::take(QLayoutItem* item)
    {
        // search in children
        return this->takeRec(item, nullptr);
    }

    QLayoutItem* FlexItem::takeRec(QLayoutItem* item, FlexItem* parent)
    {
        // check if this is the item
        if (this->type_ == Item)
        {
            if (this->item_ == item)
            {
                auto result = this->item_;

                // set type to empty
                if (parent == nullptr)
                {
                    this->clear();
                    this->type_ = Empty;
                }
                // remove this node
                else
                {
                    parent->items_.removeOne(this->shared_from_this());
                }

                return result;
            }
        }
        else if (this->isColumn() || this->isRow())
        {
            for (auto&& child : this->items_)
            {
                if (auto result = child->takeRec(item, this))
                    return result;
            }
        }

        // nothing found
        return nullptr;
    }

    void FlexItem::clear()
    {
        this->type_ = FlexItem::Empty;
        this->item_ = nullptr;
        this->items_.clear();
    }

    /// Removes children of the same container type.
    ///
    /// row(a, row(b, c), d) -> row(a, b, c, d)
    void FlexItem::flatten()
    {
        // only containers
        if (!this->isColumn() && !this->isRow())
            return;

        for (auto i = 0; i < this->items_.size(); i++)
        {
            if (this->items_[i]->type_ != this->type_)
                continue;

            // copy children into self
            auto item = this->items_.takeAt(i);
            while (item->items_.size())
            {
                this->items_.insert(i, item->items_.takeFirst());
                i++;
            }
        }
    }

    std::shared_ptr<FlexItem> FlexItem::clone()
    {
        return std::make_shared<FlexItem>(*this);
    }

    void FlexItem::performLayout(const QRect& rect)
    {
        if (this->isItem())
        {
            this->item_->setGeometry(rect);
        }
        else if (this->isColumn() || this->isRow())
        {
            // calculate total flex
            auto totalFlex = 0;

            for (auto&& item : this->items_)
                totalFlex += item->flex;

            // set geometry of children
            auto flex = .0;
            auto pos = 0;

            for (auto&& item : this->items_)
            {
                flex += item->flex;

                if (this->isRow())
                {
                    auto newPos = int(flex / totalFlex * rect.width());
                    item->performLayout(QRect{
                        rect.x() + pos,
                        rect.y(),
                        newPos - pos,
                        rect.height(),
                    });
                    pos = newPos;
                }
                else  // this->isColumn()
                {
                    auto newPos = int(flex / totalFlex * rect.height());
                    item->performLayout(QRect{
                        rect.x(),
                        rect.y() + pos,
                        rect.width(),
                        newPos - pos,
                    });
                    pos = newPos;
                }
            }
        }
    }

    void FlexItem::print(int indent)
    {
        const char* type;

        if (this->type_ == Empty)
            type = "empty";
        if (this->type_ == Item)
            type = "item";
        if (this->type_ == Column)
            type = "column";
        if (this->type_ == Row)
            type = "row";

        qDebug().noquote() << QString(indent, ' ') << type
                           << (reinterpret_cast<int>(this->item_) % 1000);

        for (auto&& item : this->items_)
            item->print(indent + 2);
    }
}  // namespace chatterino::ui
