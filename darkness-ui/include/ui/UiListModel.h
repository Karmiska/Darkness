#pragma once

#include "containers/unordered_map.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "tools/Debug.h"

namespace ui
{
    /*
    * null
    *   - row 0 [ column 0, column 1 ]
    *       - row 0 [ column 0, column 1 ]
    *       - row 1 [ column 0, column 1 ]
    *   - row 1
    *   - row 2
    */
    class UiListModelIndex
    {
    public:
        UiListModelIndex(int row, int column)
            : m_row{ row }, m_column{ column } {};

        int row() const { return m_row; }
        int column() const { return m_column; }

        auto operator<=>(const UiListModelIndex&) const = default;
    private:
        int m_row;
        int m_column;
    };

    class UiListModel;
    class UiListModelItem
    {
    public:
        UiListModelItem(UiListModel* model, UiListModelIndex index);
        virtual ~UiListModelItem() {};

        const engine::vector<engine::shared_ptr<UiListModelItem>>& childs() const;
        
        virtual void addChild(engine::shared_ptr<UiListModelItem> child);
        virtual void removeChild(engine::shared_ptr<UiListModelItem> child);

        virtual bool selected() const;
        virtual void selected(bool val);

        virtual UiListModelIndex index() const;

        void registerForChangeNotification(std::function<void()> notify)
        {
            m_notify = notify;
        }
        void unregisterForChangeNotification()
        {
            m_notify = {};
        }
    protected:
        virtual void onSelectChange(bool /*selected*/) {};
        
    private:
        UiListModel* m_model;
        UiListModelIndex m_index;
        bool m_selected;
        std::function<void()> m_notify;
        engine::vector<engine::shared_ptr<UiListModelItem>> m_childs;
    };

    struct UiListModelIndexKeyHasher
    {
        std::size_t operator()(const UiListModelIndex& k) const
        {
            return ((std::hash<int>()(k.row())
                ^ (std::hash<int>()(k.column()) << 1)) >> 1);
        }
    };

    class UiListModel
    {
    public:
        virtual ~UiListModel() {};
        virtual int rowCount() const = 0;
        virtual int columnCount() const = 0;
        virtual engine::shared_ptr<UiListModelItem> getItem(const UiListModelIndex& index);
        virtual void invalidateModel();
        virtual bool validModelIndex(const UiListModelIndex& index) const = 0;
        virtual engine::vector<UiListModelIndex> selectedIndexes() const;
        virtual void indexSelected(const UiListModelIndex& index, bool val);
        virtual bool indexSelected(const UiListModelIndex& index) const;
    protected:
        virtual engine::shared_ptr<UiListModelItem> createItem(const UiListModelIndex& index) = 0;
        virtual void onIndexSelectChange(UiListModelIndex /*index*/, bool /*selected*/) {};
    private:
        friend class UiListModelItem;
        //engine::unordered_map<UiListModelIndex, engine::shared_ptr<UiListModelItem>, UiListModelIndexKeyHasher> m_items;
        engine::vector<engine::vector<engine::shared_ptr<UiListModelItem>>> m_items;
    };

}
