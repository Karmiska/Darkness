#pragma once

#include "ui/Frame.h"
#include "ui/UiListView.h"
#include "ui/UiListModel.h"

namespace ui
{
	class Label;
	class Frame;
}

namespace editor
{
	class HierarchyModelItem : public ui::UiListModelItem
	{
	public:
		HierarchyModelItem(ui::UiListModel* model, ui::UiListModelIndex index, 
			const engine::string& name)
			: ui::UiListModelItem(model, index)
			, m_name{ name }
		{}

		const engine::string& name() const { return m_name; }
	private:
		engine::string m_name;
	};

	class HierarchyDataModel : public ui::UiListModel
	{
	public:
		int rowCount() const override;
		int columnCount() const override;
		bool validModelIndex(const ui::UiListModelIndex& index) const override;
	protected:
		engine::shared_ptr<ui::UiListModelItem> createItem(const ui::UiListModelIndex& index) override;
		//engine::Ecs m_ecs;
	};

	class HierarchyListViewItem : public ui::UiListViewItem
	{
	public:
		HierarchyListViewItem(ui::Frame* parent, ui::UiListModelItem* modelItem, const engine::string& name);
	private:
		engine::shared_ptr<ui::Label> m_label;
	};

	class HierarchyListView : public ui::UiListView
	{
	public:
		HierarchyListView(Frame* parent)
			: ui::UiListView(parent)
		{}
	protected:
		engine::shared_ptr<ui::UiListViewItem> createItem(ui::Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem) override;
		ui::UiRect evaluateItemSize(ui::UiListModelItem* modelItem) const override;
	};

	class Hierarchy : public ui::Frame
	{
	public:
		Hierarchy(Frame* parent, const char* name);

	private:
		engine::shared_ptr<ui::Label> m_label;
		engine::shared_ptr<HierarchyListView> m_listView;
		engine::shared_ptr<HierarchyDataModel> m_model;
	};
}
