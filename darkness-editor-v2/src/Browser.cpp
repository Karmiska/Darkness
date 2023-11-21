#include "Browser.h"
#include "ui/Label.h"
#include "ui/UiAnchors.h"

#include "platform/Environment.h"
#include "tools/PathTools.h"
#include "ui/Theme.h"
#include "engine/graphics/Device.h"
#include "ui/UiLayout.h"
#include "ui/UiImage.h"
#include "tools/AssetTools.h"

namespace editor
{
	BrowserDataModel::BrowserDataModel(const engine::string& rootFolder)
		: m_rootFolder{ rootFolder }
		, m_indexChangeInProgress{ false }
	{
		engine::Directory dir(m_rootFolder);
		m_files = dir.files();
		m_folders = dir.directories();
	}

	int BrowserDataModel::rowCount() const
	{
		return m_folders.size();
	}

	int BrowserDataModel::columnCount() const
	{
		return 2;
	}

	engine::shared_ptr<ui::UiListModelItem> BrowserDataModel::createItem(const ui::UiListModelIndex& index)
	{
		if (index.column() == 0)
		{
			return engine::make_shared<BrowserModelItem>(this, index, THEME.image("folder_icon"), BrowserItemType::Icon);
		}
		else if (index.column() == 1)
		{
			auto filename = m_folders[index.row()];
			return engine::make_shared<BrowserModelItem>(this, index, filename, BrowserItemType::Filename);
		}
	}

	void BrowserDataModel::onIndexSelectChange(ui::UiListModelIndex index, bool selected)
	{
		if (m_indexChangeInProgress)
			return;
		m_indexChangeInProgress = true;
		for(int column = 0; column < columnCount(); ++column)
			for (int row = 0; row < rowCount(); ++row)
			{
				ui::UiListModelIndex ind(row, column);
				if (validModelIndex(ind) && ind != index && indexSelected(ind))
					indexSelected(ind, false);
			}
		m_indexChangeInProgress = false;
		LOG("Index [column %i, row: %i]: %s", index.column(), index.row(), selected ? "selected" : "not selected");
		if (m_onFolderSelect)
		{
			auto item = getItem(index);
			m_onFolderSelect(static_cast<BrowserModelItem*>(item.get())->name());
		}
	}

	bool BrowserDataModel::validModelIndex(const ui::UiListModelIndex& index) const
	{
		return index.row() < m_folders.size();
	}

	BrowserListViewItem::BrowserListViewItem(Frame* parent, ui::UiListModelItem* modelItem, BrowserItemType type)
		: UiListViewItem(parent, modelItem)
		, m_browserItemType{ type }
	{
	}

	BrowserListViewFilenameItem::BrowserListViewFilenameItem(Frame* parent, ui::UiListModelItem* modelItem, const engine::string& val)
		: BrowserListViewItem(parent, modelItem, BrowserItemType::Filename)
		, m_label{ nullptr }
		, m_mousePresent{ false }
	{
		themeSet(false);
		//drawBackground(false);
		m_label = engine::make_shared<ui::Label>(this, val);
		addChild(m_label);
		addAnchor(ui::UiAnchor{ m_label.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft, { 0, 0 } });
		addAnchor(ui::UiAnchor{ m_label.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight, { 0, 0 } });

		modelItem->registerForChangeNotification([&]() { updateItem(); });
		updateItem();
	}

	BrowserListViewFilenameItem::~BrowserListViewFilenameItem()
	{
		m_modelItem->unregisterForChangeNotification();
	}

	void BrowserListViewFilenameItem::onMouseEnter(int /*x*/, int /*y*/)
	{
		m_mousePresent = true;
		m_label->drawBackground(true);
		m_label->backgroundColor(engine::Vector4f{ 0.1f, 0.1f, 0.1f, 0.01f });
	};

	void BrowserListViewFilenameItem::onMouseLeave(int /*x*/, int /*y*/)
	{
		if (!m_modelItem->selected())
			m_label->drawBackground(false);
		else
			m_label->backgroundColor(engine::Vector4f{ 0.5f, 0.5f, 0.5f, 0.1f });

		m_mousePresent = false;
	};

	void BrowserListViewFilenameItem::onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/)
	{
		m_modelItem->selected(!m_modelItem->selected());
	}

	void BrowserListViewFilenameItem::onMouseUp(engine::MouseButton /*button*/, int /*x*/, int /*y*/)
	{

	}

	void BrowserListViewFilenameItem::onMouseDoubleClick(engine::MouseButton /*button*/, int /*x*/, int /*y*/)
	{

	}

	void BrowserListViewFilenameItem::updateItem()
	{
		if (m_modelItem->selected())
		{
			m_label->drawBackground(true);
			m_label->backgroundColor(engine::Vector4f{ 0.5f, 0.5f, 0.5f, 0.1f });
		}
		else
		{
			if (m_mousePresent)
			{
				m_label->backgroundColor(engine::Vector4f{ 0.1f, 0.1f, 0.1f, 0.01f });
			}
			else
			{
				m_label->drawBackground(false);
			}
		}
	}

	BrowserListViewIconItem::BrowserListViewIconItem(Frame* parent, ui::UiListModelItem* modelItem, const engine::string& val)
		: BrowserListViewItem(parent, modelItem, BrowserItemType::Icon)
		, m_image{ nullptr }
	{
		themeSet(false);
		//drawBackground(false);
		m_image = device().createImage(val, engine::Format::UNKNOWN,
			-1, -1, -1, -1, engine::image::ImageType::EXTERNAL);
	}

	void BrowserListViewIconItem::onPaint(ui::DrawCommandBuffer& cmd)
	{
		cmd.drawImage(2, 2, width()-4, height()-4, m_image);
	}

	engine::shared_ptr<ui::UiListViewItem> BrowserListView::createItem(Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem)
	{
		if (std::dynamic_pointer_cast<BrowserModelItem>(modelItem)->type() == BrowserItemType::Filename)
		{
			auto res = engine::make_shared<BrowserListViewFilenameItem>(
				parent, modelItem.get(),
				std::dynamic_pointer_cast<BrowserModelItem>(modelItem)->name());
			res->height(16);
			return res;
		}
		else if (std::dynamic_pointer_cast<BrowserModelItem>(modelItem)->type() == BrowserItemType::Icon)
		{
			auto res = engine::make_shared<BrowserListViewIconItem>(
				parent, modelItem.get(),
				std::dynamic_pointer_cast<BrowserModelItem>(modelItem)->name());
			res->height(16);
			return res;
		}
	}

	ui::UiRect BrowserListView::evaluateItemSize(ui::UiListModelItem* modelItem) const
	{
		if (static_cast<BrowserModelItem*>(modelItem)->type() == BrowserItemType::Filename)
			return { 0, 0, 100, 16 };
		else
			return { 0, 0, 16, 16 };
	}

	Browser::Browser(Frame* parent, const char* name)
		: Frame{ 500, 500, parent->api(), parent }
		, m_label{ engine::make_shared<ui::Label>(this, name) }
		, m_layoutSettingsManager{ engine::make_shared<ui::UiLayoutSettingsManager>(
			engine::pathClean(engine::pathJoin(engine::pathJoin(engine::pathExtractFolder(engine::getExecutableDirectory()), "../../../data/"), "BrowserLayoutSettings.json"))) }
		, m_horizontalLayout{ engine::make_shared<ui::UiLayout>(this, m_layoutSettingsManager->settings("horizontalLayout"), ui::UiLayoutDirection::Horizontal) }
		, m_model{ engine::make_shared<BrowserDataModel>(engine::pathClean("C:/Users/aleks/Documents/TestDarknessProject/content")) }
		, m_listView{ engine::make_shared<BrowserListView>(this) }
		, m_fileViewModel{ engine::make_shared<BrowserFileViewDataModel>(this, engine::pathClean("C:/Users/aleks/Documents/TestDarknessProject/content/pbr/bamboo")) }
		, m_fileView{ engine::make_shared<BrowserFileView>(this) }
	{
		addChild(m_horizontalLayout);
		m_horizontalLayout->drawBackground(true);
		m_horizontalLayout->backgroundColor({ 0.161f, 0.165f, 0.169f, 1.0f });
		addAnchor(ui::UiAnchor{ m_horizontalLayout.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft, { 2, 21 } });
		addAnchor(ui::UiAnchor{ m_horizontalLayout.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight, { -2, -2 } });

		m_listView->model(m_model);
		m_listView->allItemsSameHeight(true);
		m_listView->canMove(ui::AllowedMovement::None);
		m_horizontalLayout->addChild(m_listView);

		m_model->registerForFolderSelectNotification([&](const engine::string& path)
			{
				m_fileViewModel->setFolder(engine::pathJoin("C:/Users/aleks/Documents/TestDarknessProject/content", path));
				m_fileView->invalidateModel();
			});

		m_fileView->fillHorizontalSpace(false);
		m_fileView->model(m_fileViewModel);
		m_fileView->allItemsSameHeight(true);
		m_fileView->canMove(ui::AllowedMovement::None);
		m_horizontalLayout->addChild(m_fileView);
		
		canResize(true);
		canFocus(true);
		themeSet(true);
		//position(0, 50);

		m_label->left(2);
		m_label->top(2);
		m_label->width(116);
		m_label->height(16);
		addChild(m_label);
		//addChild(m_listView);

		//addAnchor(ui::UiAnchor{ m_listView.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft, { 2, 21 } });
		//addAnchor(ui::UiAnchor{ m_listView.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight, { -2, -2 } });

		clientArea().left = 4;
		clientArea().top = 21;
		clientArea().right = 4;
		clientArea().bottom = 4;
	}

	ui::UiPoint Browser::size() const
	{
		return m_fileView->size();
	}

	Browser::~Browser()
	{
		m_listView->model(nullptr);
		m_fileView->model(nullptr);

		m_listView->invalidateModel();
		m_fileView->invalidateModel();
		m_model->invalidateModel();
		m_fileViewModel->invalidateModel();

		removeChild(m_fileView);
		removeChild(m_listView);
		removeChild(m_horizontalLayout);

		m_fileView = nullptr;
		m_listView = nullptr;

		m_model = nullptr;
		m_fileViewModel = nullptr;
	}

	BrowserFileViewItem::BrowserFileViewItem(ui::Frame* parent, ui::UiListModelItem* modelItem, const engine::string& filePath)
		: UiListViewItem(parent, modelItem)
		, m_filePath{ filePath }
		, m_label{ nullptr }
		, m_image{ nullptr }
	{
		if (engine::isImageFormat(filePath))
			m_image = engine::make_shared<ui::UiImage>(this, filePath);
		else if(engine::isModelFormat(filePath))
			m_image = engine::make_shared<ui::UiImage>(this, THEME.image("folder_icon"));

		themeSet(false);
		//drawBackground(false);
		backgroundColor({ 0.161f, 0.165f, 0.169f, 1.0f });

		m_image->canMove(ui::AllowedMovement::None);
		addChild(m_image);
		addAnchor(ui::UiAnchor{ m_image.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft, { 2, 2 } });
		addAnchor(ui::UiAnchor{ m_image.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight, { -2, -2 } });

		m_label = engine::make_shared<ui::Label>(this, engine::pathExtractFilename(filePath));
		m_label->left(2);
		m_label->top(2);
		m_label->width(116);
		m_label->height(16);
		addChild(m_label);
	}

	engine::shared_ptr<ui::UiListViewItem> BrowserFileView::createItem(Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem)
	{
		if (!modelItem)
			return nullptr;
		auto res = engine::make_shared<BrowserFileViewItem>(
			parent, modelItem.get(),
			std::dynamic_pointer_cast<BrowserFileViewModelItem>(modelItem)->filepath());
		res->height(90);
		return res;
	}

	ui::UiRect BrowserFileView::evaluateItemSize(ui::UiListModelItem* modelItem) const
	{
		return { 0, 0, 90, 90 };
	}

	BrowserFileViewDataModel::BrowserFileViewDataModel(SizeRequest* sizeRequest, const engine::string& folder)
		: m_sizeRequest{ sizeRequest }
		, m_lastColumnCount{ 0 }
		, m_rootPath{ folder }
	{
		updatePath(m_rootPath);
	}

	void BrowserFileViewDataModel::updatePath(const engine::string& path)
	{
		engine::Directory dir(path);
		m_files = dir.files();
		m_folders = dir.directories();
		m_rootPath = path;

		for (auto i = m_files.begin(); i != m_files.end();)
			if (!engine::isImageFormat(*i) && !engine::isModelFormat(*i))
				i = m_files.erase(i);
			else
				++i;
	}

	int BrowserFileViewDataModel::rowCount() const
	{
		auto rowCnt = 
			static_cast<int>(
			ceil(static_cast<float>(m_files.size()) /
			static_cast<float>(columnCount())) + 0.5);
		return rowCnt;
	}

	int BrowserFileViewDataModel::columnCount() const
	{
		return m_sizeRequest->size().x / 90;
	}

	engine::shared_ptr<ui::UiListModelItem> BrowserFileViewDataModel::getItem(const ui::UiListModelIndex& index)
	{
		auto s = m_sizeRequest->size();
		auto colCount = s.x / 90;
		if (colCount != m_lastColumnCount)
		{
			invalidateModel();
			m_lastColumnCount = colCount;
		}
		return UiListModel::getItem(index);
	}

	bool BrowserFileViewDataModel::validModelIndex(const ui::UiListModelIndex& index) const
	{
		auto colCount = columnCount();
		auto fileIndex = (index.row() * colCount) + index.column();
		if (fileIndex > m_files.size() - 1)
			return false;
		return true;
	}

	void BrowserFileViewDataModel::setFolder(const engine::string& folder)
	{
		updatePath(folder);
		invalidateModel();
	}

	engine::shared_ptr<ui::UiListModelItem> BrowserFileViewDataModel::createItem(const ui::UiListModelIndex& index)
	{
		auto colCount = columnCount();
		auto fileIndex = (index.row() * colCount) + index.column();
		if (fileIndex > m_files.size() - 1)
			return nullptr;
		auto filename = m_files[fileIndex];
		return engine::make_shared<BrowserFileViewModelItem>(this, index, engine::pathJoin(m_rootPath, filename));
	}

	void BrowserFileViewDataModel::onIndexSelectChange(ui::UiListModelIndex index, bool selected)
	{
		LOG("Index [column %i, row: %i]: %s", index.column(), index.row(), selected ? "selected" : "not selected");
	}
}
