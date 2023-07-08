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
	class InspectorModelItem : public ui::UiListModelItem
	{
	public:
		InspectorModelItem(ui::UiListModel* model, ui::UiListModelIndex index, const engine::string& name)
			: ui::UiListModelItem(model, index)
			, m_name{ name }
		{}

		const engine::string& name() const { return m_name; }
	private:
		engine::string m_name;
	};

	class InspectorDataModel : public ui::UiListModel
	{
	public:
		int rowCount() const override;
		int columnCount() const override;
		bool validModelIndex(const ui::UiListModelIndex& index) const override;
	protected:
		engine::shared_ptr<ui::UiListModelItem> createItem(const ui::UiListModelIndex& index) override;
	};

	class InspectorListViewItem : public ui::UiListViewItem
	{
	public:
		InspectorListViewItem(ui::Frame* parent, ui::UiListModelItem* modelItem, const engine::string& name);
	private:
		engine::shared_ptr<ui::Label> m_label;
	};

	class InspectorListView : public ui::UiListView
	{
	public:
		InspectorListView(Frame* parent)
			: ui::UiListView(parent)
		{}
	protected:
		engine::shared_ptr<ui::UiListViewItem> createItem(ui::Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem) override;
		ui::UiRect evaluateItemSize(ui::UiListModelItem* modelItem) const override;
	};

	class Inspector : public ui::Frame
	{
	public:
		Inspector(Frame* parent, const char* name);

	private:
		engine::shared_ptr<ui::Label> m_label;
		engine::shared_ptr<InspectorListView> m_listView;
		engine::shared_ptr<InspectorDataModel> m_model;
	};
}
