#pragma once

#include "ui/Frame.h"
#include "ui/UiScrollBar.h"
#include "ui/UiListModel.h"
#include "containers/string.h"
#include "containers/vector.h"

namespace ui
{
    class Label;
    class UiListView;
    class UiListViewItem;

    class UiListViewWheelEvent
    {
    public:
        virtual ~UiListViewWheelEvent() {};
    public:
        virtual void onListMouseWheel(UiListViewItem*, int, int, int) = 0;
    };

    class UiListViewItem : public Frame
    {
    public:
        UiListViewItem(Frame* parent, UiListModelItem* modelItem);
        virtual int itemHeight() const { return height(); }

    protected:
        void onMouseWheel(int /*x*/, int /*y*/, int /*delta*/) override;

    protected:
        UiListModelItem* m_modelItem;
    private:
        UiListViewWheelEvent* m_wheelEvent;
    };

    #define ScrollBarJump 3

    class UiListView : public Frame,
                       public UiListViewWheelEvent
    {
    public:
        UiListView(Frame* parent);

        void model(engine::shared_ptr<UiListModel> model);
        engine::shared_ptr<UiListModel> model() const;

        void onResize(int /*width*/, int /*height*/) override;

        void allItemsSameHeight(bool value);
        bool allItemsSameHeight() const;
        
        void fillHorizontalSpace(bool val);
        bool fillHorizontalSpave() const;

        virtual void invalidateModel();
    protected:
        virtual engine::shared_ptr<UiListViewItem> createItem(ui::Frame* parent, engine::shared_ptr<UiListModelItem> modelItem) = 0;
        virtual UiRect evaluateItemSize(UiListModelItem* modelItem) const = 0;
        void onListMouseWheel(UiListViewItem*, int, int, int) override;
    private:
        bool m_allItemsSameHeight;
        bool m_fillHorizontalSpace;
        struct ItemContainer
        {
            engine::shared_ptr<UiListViewItem> item;
            UiListModelIndex index;
        };
        // [column][row]
        engine::vector<engine::vector<ItemContainer>> m_items;
        engine::shared_ptr<UiScrollBar> m_scrollBar;
        engine::shared_ptr<UiListModel> m_model;
        int m_lastColumnCount;
        void updateList();
        void addScrollBar();
        void removeScrollBar();
        float getRange(UiListModel* model);
        float getSameRange(UiListModel* model);
        std::pair<int, float> getFirstItemLocation(UiListModel* model, float height, float windowPosition);
        std::pair<int, float> getFirstItemSameLocation(UiListModel* model, float height, float windowPosition);
        void clearItems();
    };
}

