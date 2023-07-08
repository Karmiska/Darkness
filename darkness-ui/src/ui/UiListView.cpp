#include "ui/UiListView.h"
#include "ui/Label.h"
#include "ui/UiListModel.h"

namespace ui
{
    UiListViewItem::UiListViewItem(Frame* parent, UiListModelItem* modelItem)
        : Frame(0, 0, parent->api(), parent)
        , m_modelItem{ modelItem }
    {
        auto listView = dynamic_cast<UiListViewWheelEvent*>(parent);
        ASSERT(listView != nullptr);
        m_wheelEvent = listView;
    }

    void UiListViewItem::onMouseWheel(int x, int y, int delta)
    {
        m_wheelEvent->onListMouseWheel(this, x, y, delta);
    }

    UiListView::UiListView(Frame* parent)
        : Frame(100, 100, parent->api(), parent)
        , m_allItemsSameHeight{ false }
        , m_fillHorizontalSpace{ true }
        , m_scrollBar{ nullptr }
        , m_model{ nullptr }
        , m_lastColumnCount{ 0 }
    {
        drawBackground(false);
        backgroundColor(engine::Vector3f{ 1.0f, 0.0f, 0.0f });
    }

    void UiListView::addScrollBar()
    {
        if (m_scrollBar)
            return;

        m_scrollBar = engine::make_shared<ui::UiScrollBar>(this);
        m_scrollBar->alwaysOntop(true);
        m_scrollBar->onScrollValueChange = [this](float /*value*/)
        {
            updateList();
        };

        addChild(m_scrollBar);
        m_scrollBar->left(width() - 9);
        m_scrollBar->top(0);
        m_scrollBar->width(9);
        m_scrollBar->height(height());


        addAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Left, ui::AnchorType::Right, { -9, 0 } });
        addAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Right, ui::AnchorType::Right });
        addAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Top, ui::AnchorType::Top });
        addAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Bottom, ui::AnchorType::Bottom });
        
    }

    void UiListView::removeScrollBar()
    {
        if (!m_scrollBar)
            return;

        removeAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Left, ui::AnchorType::Right, { -9, 0 } });
        removeAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Right, ui::AnchorType::Right });
        removeAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Top, ui::AnchorType::Top });
        removeAnchor(ui::UiAnchor{ m_scrollBar.get(), ui::AnchorType::Bottom, ui::AnchorType::Bottom });
        

        removeChild(m_scrollBar);

        m_scrollBar = nullptr;
    }

    void UiListView::onListMouseWheel(UiListViewItem* /*item*/, int /*x*/, int /*y*/, int delta)
    {
        delta *= ScrollBarJump;

        if (!m_scrollBar || delta == 0)
            return;

        int height = this->height();
        float listRange = m_allItemsSameHeight ? getSameRange(m_model.get()) : getRange(m_model.get());
        float drawLimitedRange = listRange > height ? listRange - height : listRange;
        engine::vector<engine::vector<ItemContainer>> items;
        auto scrollBarValue = (m_scrollBar ? m_scrollBar->value() : 0.0f);
        auto firstItem = m_allItemsSameHeight ?
            getFirstItemSameLocation(m_model.get(), listRange, scrollBarValue * drawLimitedRange) :
            getFirstItemLocation(m_model.get(), listRange, scrollBarValue * drawLimitedRange);
        int drawItem = firstItem.first;

        // delta < 0 == down
        float moveAmount = 0.0f;
        int steps = abs(delta);
        for (int i = 0; i < steps; ++i)
        {
            int newDrawItem = drawItem + (delta > 0 ? -1 : 1);
            newDrawItem = min(max(newDrawItem, 0), m_model->rowCount() - 1);
            if (newDrawItem == drawItem)
            {
                if (newDrawItem == 0)
                    m_scrollBar->value(0.0f);
                else
                    m_scrollBar->value(1.0f);
                break;
            }
            
            if (newDrawItem > drawItem)
            {
                auto itemHeight = evaluateItemSize(m_model->getItem({ drawItem, 0 }).get()).height();
                moveAmount += static_cast<float>(itemHeight);
            }
            else
            {
                auto itemHeight = evaluateItemSize(m_model->getItem({ newDrawItem, 0 }).get()).height();
                moveAmount += static_cast<float>(itemHeight);
            }
            

            drawItem = newDrawItem;
        }

        moveAmount = delta > 0 ? moveAmount * -1 : moveAmount;
        float newScrollbarValue = m_scrollBar->value() + (moveAmount / (listRange - height));
        newScrollbarValue = max(min(newScrollbarValue, 1.0f), 0.0f);
        m_scrollBar->value(newScrollbarValue);
    }

    void UiListView::model(engine::shared_ptr<UiListModel> model)
    {
        m_model = model;
    }

    engine::shared_ptr<UiListModel> UiListView::model() const
    {
        return m_model;
    }

    void UiListView::onResize(int /*width*/, int /*height*/)
    {
        updateList();
    }

    void UiListView::invalidateModel()
    {
        clearItems();
        updateList();
    }

    void UiListView::allItemsSameHeight(bool value)
    {
        m_allItemsSameHeight = value;
    }

    bool UiListView::allItemsSameHeight() const
    {
        return m_allItemsSameHeight;
    }

    void UiListView::fillHorizontalSpace(bool val)
    {
        m_fillHorizontalSpace = val;
    }

    bool UiListView::fillHorizontalSpave() const
    {
        return m_fillHorizontalSpace;
    }

    float UiListView::getRange(UiListModel* model)
    {
        float res = 0.0f;
        int count = model->rowCount();
        for (int i = 0; i < count; ++i)
        {
            res += evaluateItemSize(model->getItem({ i, 0 }).get()).height();
        }
        return res;
    };

    float UiListView::getSameRange(UiListModel* model)
    {
        if (model->rowCount() > 0)
        {
            return static_cast<float>(model->rowCount() * evaluateItemSize(model->getItem({ 0, 0 }).get()).height());
        }
        return 0.0f;
    };

    std::pair<int, float> UiListView::getFirstItemLocation(UiListModel* model, float height, float windowPosition)
    {
        float currentPosition = 0.0;
        int count = model->rowCount();
        for (int i = 0; i < count; ++i)
        {
            auto itemHeight = evaluateItemSize(model->getItem({ i, 0 }).get()).height();
            if (
                (currentPosition < windowPosition && currentPosition + itemHeight > windowPosition) ||
                (currentPosition >= windowPosition && currentPosition < windowPosition + height)
                )
            {
                return std::make_pair(i, currentPosition);
            }
            currentPosition += itemHeight;
        }
        return std::make_pair(0, height);
    };

    std::pair<int, float> UiListView::getFirstItemSameLocation(UiListModel* model, float height, float windowPosition)
    {
        if (model->rowCount() == 0)
            return std::make_pair(0, height);

        auto itemHeight = evaluateItemSize(model->getItem({ 0, 0 }).get()).height();
        auto itemIndex = static_cast<int>(windowPosition / itemHeight);
        return std::make_pair(itemIndex, itemIndex * itemHeight);
    };

    void UiListView::clearItems()
    {
        for (int i = 0; i < m_items.size(); ++i)
            for (int a = 0; a < m_items[i].size(); ++a)
            {
                removeAnchor(ui::UiAnchor{ m_items[i][a].item.get(), ui::AnchorType::Left, ui::AnchorType::Left });
                if (m_fillHorizontalSpace && i == m_items.size() - 1)
                {
                    removeAnchor(ui::UiAnchor{ m_items[i][a].item.get(), ui::AnchorType::Right, ui::AnchorType::Right });
                }
                removeChild(m_items[i][a].item);
            }
        m_items.clear();
    }

    void UiListView::updateList()
    {
        if (!m_model)
            return;

        int height = this->height();

        float listRange = m_allItemsSameHeight ? getSameRange(m_model.get()) : getRange(m_model.get());
        if (height > listRange)
        {
            //m_scrollBar->value(0.0f);
            removeScrollBar();
        }
        else
        {
            addScrollBar();
        }

        auto colCount = m_model->columnCount();
        if (m_lastColumnCount != colCount)
        {
            clearItems();
            m_lastColumnCount = colCount;
        }

        float drawLimitedRange = listRange > height ? listRange - height : listRange;
        engine::vector<engine::vector<ItemContainer>> items;
        auto scrollBarValue = (m_scrollBar ? m_scrollBar->value() : 0.0f);
        auto firstItem = m_allItemsSameHeight ? 
            getFirstItemSameLocation(m_model.get(), listRange, scrollBarValue * drawLimitedRange) :
            getFirstItemLocation(m_model.get(), listRange, scrollBarValue * drawLimitedRange);
        float drawPosition = firstItem.second - (scrollBarValue * drawLimitedRange);
        int drawItem = firstItem.first;
        
        int previousSize = 0;

        while (drawPosition < height && drawItem < m_model->rowCount())
        {
            int widthUsed = 0;
            for (int column = 0; column < colCount; ++column)
            {
                int color = drawItem % 2 == 0 ? 0 : 1;

                UiListModelIndex index{ drawItem, column };

                bool validIndex = m_model->validModelIndex(index);
                auto itemWidth = validIndex ? evaluateItemSize(m_model->getItem(index).get()).width() : previousSize;
                previousSize = itemWidth;

                while (column >= m_items.size())
                    m_items.emplace_back(engine::vector<ItemContainer>());

                int exists = -1;
                for (int i = 0; i < m_items[column].size(); ++i)
                    if (m_items[column][i].index == index)
                    {
                        exists = i;
                        break;
                    }

                engine::shared_ptr<UiListViewItem> item = nullptr;

                bool validItem = false;
                if (exists != -1 && m_model->getItem(index))
                {
                    while (column >= items.size())
                        items.emplace_back(engine::vector<ItemContainer>());
                    items[column].emplace_back(m_items[column][exists]);
                    m_items[column].erase(m_items[column].begin() + exists);
                    validItem = true;
                }
                else if(validIndex)
                {
                    while (column >= items.size())
                        items.emplace_back(engine::vector<ItemContainer>());
                    items[column].emplace_back(ItemContainer{ createItem(this, m_model->getItem(index)), index });
                    item = items[column].back().item;
                    if (item)
                    {
                        item->canMove(ui::AllowedMovement::None);
                        item->canResize(false);
                        
                        item->width(itemWidth);
                        addChild(item);

                        addAnchor(ui::UiAnchor{ item.get(), ui::AnchorType::Left, ui::AnchorType::Left, { widthUsed, 0 } });

                        if (m_fillHorizontalSpace && (column == m_model->columnCount() - 1))
                        {
                            addAnchor(ui::UiAnchor{ item.get(), ui::AnchorType::Right, ui::AnchorType::Right, { 0, 0 } });
                        }
                        validItem = true;
                    }
                    else
                    {
                        items[column].erase(items[column].end() - 1);
                    }
                }

                if (validItem)
                {
                    if (items[column].size() > 0)
                        item = items[column].back().item;

                    if (item)
                    {
                        item->area().y(static_cast<int>(drawPosition));

                        if (color)
                            item->backgroundColor(engine::Vector3f{ 0.133f, 0.137f, 0.141f });
                        else
                            item->backgroundColor(engine::Vector3f{ 0.161f, 0.165f, 0.169f });
                    }
                }

                if (color)
                    color = 0;
                else
                    color = 1;

                widthUsed += itemWidth;
            }
            // just for sanity
            if(items.size() > 0)
                if (items[0].back().item)
                {
                    float newItemHeight = static_cast<float>(items[0].back().item->height());
                    if (newItemHeight == 0.0f)
                        break;

                    drawPosition += newItemHeight;
                }

            
            drawPosition = min(drawPosition, height);
            ++drawItem;
        }

        clearItems();

        m_items = items;

        if (m_scrollBar)
        {
            m_scrollBar->range(listRange);
            m_scrollBar->viewRange(drawPosition);
        }
    }
}
