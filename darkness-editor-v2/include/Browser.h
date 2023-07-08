#pragma once

#include "ui/Frame.h"
#include "ui/UiListView.h"
#include "ui/UiListModel.h"
#include "ui/DrawCommandBuffer.h"
#include "platform/Directory.h"

namespace ui
{
	class Label;
	class Frame;
	class UiLayout;
	class UiLayoutSettingsManager;
	class UiImage;
}

namespace editor
{
	enum class BrowserItemType
	{
		Filename,
		Icon
	};

	class BrowserModelItem : public ui::UiListModelItem
	{
	public:
		BrowserModelItem(ui::UiListModel* model, ui::UiListModelIndex index, 
			const engine::string& name, BrowserItemType browserItemType)
			: ui::UiListModelItem(model, index)
			, m_name{ name }
			, m_browserItemType{ browserItemType }
		{}

		const engine::string& name() const { return m_name; }
		const BrowserItemType type() const { return m_browserItemType; }
	private:
		engine::string m_name;
		BrowserItemType m_browserItemType;
	};

	class BrowserDataModel : public ui::UiListModel
	{
	public:
		BrowserDataModel(const engine::string& rootFolder);
		int rowCount() const override;
		int columnCount() const override;
		bool validModelIndex(const ui::UiListModelIndex& index) const override;
		void registerForFolderSelectNotification(std::function<void(const engine::string& filepath)> onFolderSelect)
		{
			m_onFolderSelect = onFolderSelect;
		}
	protected:
		engine::shared_ptr<ui::UiListModelItem> createItem(const ui::UiListModelIndex& index) override;
		void onIndexSelectChange(ui::UiListModelIndex /*index*/, bool /*selected*/) override;
	private:
		engine::string m_rootFolder;
		engine::vector<engine::string> m_files;
		engine::vector<engine::string> m_folders;
		bool m_indexChangeInProgress;
		std::function<void(const engine::string& filepath)> m_onFolderSelect;
	};

	class BrowserListViewItem : public ui::UiListViewItem
	{
	public:
		BrowserListViewItem(ui::Frame* parent, ui::UiListModelItem* modelItem, BrowserItemType type);

		BrowserItemType type() const
		{
			return m_browserItemType;
		}
	private:
		BrowserItemType m_browserItemType;
	};

	class BrowserListViewFilenameItem : public BrowserListViewItem
	{
	public:
		BrowserListViewFilenameItem(ui::Frame* parent, ui::UiListModelItem* modelItem, const engine::string& val);
		~BrowserListViewFilenameItem();

	protected:
		void onMouseEnter(int /*x*/, int /*y*/) override;
		void onMouseLeave(int /*x*/, int /*y*/) override;
		void onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/);
		void onMouseUp(engine::MouseButton /*button*/, int /*x*/, int /*y*/);
		void onMouseDoubleClick(engine::MouseButton /*button*/, int /*x*/, int /*y*/);
	private:
		engine::shared_ptr<ui::Label> m_label;
		bool m_mousePresent;
		void updateItem();
	};

	class BrowserListViewIconItem : public BrowserListViewItem
	{
	public:
		BrowserListViewIconItem(ui::Frame* parent, ui::UiListModelItem* modelItem, const engine::string& val);
	protected:
		void onPaint(ui::DrawCommandBuffer& cmd) override;
	private:
		engine::shared_ptr<engine::image::ImageIf> m_image;
	};

	class BrowserListView : public ui::UiListView
	{
	public:
		BrowserListView(Frame* parent)
			: ui::UiListView(parent)
		{}
	protected:
		engine::shared_ptr<ui::UiListViewItem> createItem(ui::Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem) override;
		ui::UiRect evaluateItemSize(ui::UiListModelItem* modelItem) const override;
	};

	class BrowserFileView : public ui::UiListView
	{
	public:
		BrowserFileView(Frame* parent) : ui::UiListView(parent) {};
	protected:
		engine::shared_ptr<ui::UiListViewItem> createItem(ui::Frame* parent, engine::shared_ptr<ui::UiListModelItem> modelItem) override;
		ui::UiRect evaluateItemSize(ui::UiListModelItem* modelItem) const override;
	};

	class BrowserFileViewModelItem : public ui::UiListModelItem
	{
	public:
		BrowserFileViewModelItem(ui::UiListModel* model, ui::UiListModelIndex index,
			const engine::string& filepath)
			: ui::UiListModelItem(model, index)
			, m_filepath{ filepath }
		{}

		const engine::string& filepath() const { return m_filepath; }
	private:
		engine::string m_filepath;
	};

	class BrowserFileViewItem : public ui::UiListViewItem
	{
	public:
		BrowserFileViewItem(ui::Frame* parent, ui::UiListModelItem* modelItem, const engine::string& filePath);

	private:
		engine::string m_filePath;
		engine::shared_ptr<ui::UiImage> m_image;
		engine::shared_ptr<ui::Label> m_label;
	};

	class SizeRequest
	{
	public:
		virtual ~SizeRequest() {};
		virtual ui::UiPoint size() const = 0;
	};

	class BrowserFileViewDataModel : public ui::UiListModel
	{
	public:
		BrowserFileViewDataModel(SizeRequest* sizeRequest, const engine::string& folder);
		int rowCount() const override;
		int columnCount() const override;
		engine::shared_ptr<ui::UiListModelItem> getItem(const ui::UiListModelIndex& index) override;
		bool validModelIndex(const ui::UiListModelIndex& index) const override;
		void setFolder(const engine::string& folder);
	protected:
		engine::shared_ptr<ui::UiListModelItem> createItem(const ui::UiListModelIndex& index) override;
		void onIndexSelectChange(ui::UiListModelIndex /*index*/, bool /*selected*/) override;
	private:
		SizeRequest* m_sizeRequest;
		engine::string m_rootPath;
		engine::vector<engine::string> m_files;
		engine::vector<engine::string> m_folders;
		int m_lastColumnCount;
		void updatePath(const engine::string& path);
	};

	class Browser : public ui::Frame,
					public SizeRequest
	{
	public:
		Browser(Frame* parent, const char* name);
		~Browser();

		ui::UiPoint size() const override;

	private:
		engine::shared_ptr<ui::Label> m_label;
		engine::shared_ptr<ui::UiLayoutSettingsManager> m_layoutSettingsManager;
		engine::shared_ptr<ui::UiLayout> m_horizontalLayout;
		engine::shared_ptr<BrowserListView> m_listView;
		engine::shared_ptr<BrowserDataModel> m_model;

		engine::shared_ptr<BrowserFileView> m_fileView;
		engine::shared_ptr<BrowserFileViewDataModel> m_fileViewModel;
	};
}
