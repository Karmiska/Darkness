#include "Hierarchy.h"
#include "ui/Label.h"
#include "ui/UiAnchors.h"

namespace editor
{
	int HierarchyDataModel::rowCount() const
	{
		return 20;
	}

	int HierarchyDataModel::columnCount() const
	{
		return 1;
	}

	bool HierarchyDataModel::validModelIndex(const ui::UiListModelIndex& index) const
	{
		return false;
	}

	engine::shared_ptr<ui::UiListModelItem> HierarchyDataModel::createItem(const ui::UiListModelIndex& index)
	{
		//auto entity = m_ecs.createEntity();
		//entity.addComponent<int>();
		//
		//engine::string name = "item ";
		//name += std::to_string(index.row());
		//return engine::make_shared<HierarchyModelItem>(this, index, name);
		return {};
	}

	HierarchyListViewItem::HierarchyListViewItem(Frame* parent, ui::UiListModelItem* modelItem, const engine::string& name)
		: UiListViewItem(parent, modelItem)
		, m_label{ engine::make_shared<ui::Label>(this, name) }
	{
		m_label->left(2);
		m_label->top(2);
		m_label->width(116);
		m_label->height(16);
		addChild(m_label);
	}

	engine::shared_ptr<ui::UiListViewItem> HierarchyListView::createItem(Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem)
	{
		auto res = engine::make_shared<HierarchyListViewItem>(parent, modelItem.get(), std::dynamic_pointer_cast<HierarchyModelItem>(modelItem)->name());
		res->height(16);
		return res;
	}

	ui::UiRect HierarchyListView::evaluateItemSize(ui::UiListModelItem* /*modelItem*/) const
	{
		return { 0, 0, 100, 16 };
	}

	Hierarchy::Hierarchy(Frame* parent, const char* name)
		: Frame{ 500, 500, parent->api(), parent }
		, m_label{ engine::make_shared<ui::Label>(this, name) }
		, m_listView{ engine::make_shared<HierarchyListView>(this) }
		, m_model{ engine::make_shared<HierarchyDataModel>() }
	{
		m_listView->model(m_model);
		m_listView->allItemsSameHeight(true);
		m_listView->canMove(ui::AllowedMovement::None);
		canResize(true);
		canFocus(true);
		themeSet(true);
		position(0, 50);

		m_label->left(2);
		m_label->top(2);
		m_label->width(116);
		m_label->height(16);
		addChild(m_label);

		//m_listView->left(4);
		//m_listView->top(21);
		//m_listView->width(area().width() - 8);
		//m_listView->height(area().height() - 25);
		addChild(m_listView);

		addAnchor(ui::UiAnchor{ m_listView.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft, { 2, 21 } });
		addAnchor(ui::UiAnchor{ m_listView.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight, { -2, -2 } });

		clientArea().left =		4;
		clientArea().top =		21;
		clientArea().right =	4;
		clientArea().bottom =	4;
	}
}
