#include "ui/UiListModel.h"

namespace ui
{
    UiListModelItem::UiListModelItem(UiListModel* model, UiListModelIndex index)
        : m_model{ model }
        , m_index{ index }
        , m_selected{ false }
    {
    }

    const engine::vector<engine::shared_ptr<UiListModelItem>>& UiListModelItem::childs() const
    {
        return m_childs;
    }

    void UiListModelItem::addChild(engine::shared_ptr<UiListModelItem> child)
    {
        m_childs.emplace_back(child);
    }

    void UiListModelItem::removeChild(engine::shared_ptr<UiListModelItem> child)
    {
        auto exists = std::find(m_childs.begin(), m_childs.end(), child);
        if (exists != m_childs.end())
            m_childs.erase(exists);
        else
            LOG("Tried to remove non-existing child");
    }

    bool UiListModelItem::selected() const
    {
        return m_selected;
    }

    void UiListModelItem::selected(bool val)
    {
        bool change = m_selected != val;
        m_selected = val;
        if (change)
            onSelectChange(m_selected);
        m_model->onIndexSelectChange(m_index, m_selected);
        if (m_notify)
            m_notify();
    }

    UiListModelIndex UiListModelItem::index() const
    {
        return m_index;
    }

    engine::shared_ptr<UiListModelItem> UiListModel::getItem(const UiListModelIndex& index)
    {
        if (index.row() < rowCount() && index.column() < columnCount())
        {
            if (index.row() >= m_items.size())
                m_items.resize(index.row() + 1);

            if (index.column() >= m_items[index.row()].size())
                m_items[index.row()].resize(index.column() + 1);

            if (m_items[index.row()][index.column()])
                return m_items[index.row()][index.column()];
            else
            {
                m_items[index.row()][index.column()] = createItem(index);
                return m_items[index.row()][index.column()];
            }
        }

        return nullptr;
    }

    void UiListModel::invalidateModel()
    {
        m_items.clear();
    }

    engine::vector<UiListModelIndex> UiListModel::selectedIndexes() const
    {
        engine::vector<UiListModelIndex> res;
        for (auto&& row : m_items)
            for (auto&& column : row)
                if(column->selected())
                    res.emplace_back(column->index());
        return res;
    }

    void UiListModel::indexSelected(const UiListModelIndex& index, bool val)
    {
        m_items[index.row()][index.column()]->selected(val);
    }

    bool UiListModel::indexSelected(const UiListModelIndex& index) const
    {
        return m_items[index.row()][index.column()]->selected();
    }
}
