#pragma once

#include "ui/Frame.h"
#include "containers/vector.h"

namespace tools
{
    class Settings;
}

namespace ui
{
    class UiLayoutSettingsManager;
    class UiLayoutSettings
    {
    public:
        UiLayoutSettings();
        UiLayoutSettings(const UiLayoutSettings&) = default;
        UiLayoutSettings& operator=(const UiLayoutSettings&) = default;
        UiLayoutSettings(UiLayoutSettings&&) = default;
        UiLayoutSettings& operator=(UiLayoutSettings&&) = default;

        float getSize(int index);
        void setSize(int index, float size);
        void save();
        void load();
    protected:
        friend class UiLayoutSettingsManager;
        UiLayoutSettings(
            UiLayoutSettingsManager* manager,
            const char* componentName);

    private:
        UiLayoutSettingsManager* m_manager;
        const char* m_componentName;
        engine::vector<float> m_sizes;
    };

    class UiLayoutSettingsManager
    {
    public:
        UiLayoutSettingsManager(const engine::string& settingsFile);
        ~UiLayoutSettingsManager();

        UiLayoutSettings settings(const char* componentName);
    private:
        friend class UiLayoutSettings;
        engine::shared_ptr<tools::Settings> m_settings;
    };

    #define ResizeBarSize 4
    #define LayoutPadding 1

    class UiLayoutResizeBar;
    class ResizeEventHandler
    {
    public:
        virtual ~ResizeEventHandler() {};
        virtual void onResize(UiLayoutResizeBar* resizeBar, ui::UiPoint) = 0;
    };

    enum class UiLayoutDirection
    {
        Horizontal,
        Vertical
    };

    class UiLayoutResizeBar : public ui::Frame
    {
    public:
        UiLayoutResizeBar(
            Frame* parent, 
            ResizeEventHandler* resizeHandler,
            UiLayoutDirection direction);
    protected:
        void onDragMove(ui::UiPoint) override;
        //void onPaint(ui::DrawCommandBuffer& cmd) override;
        void onMouseMove(int x, int y) override;
    private:
        ResizeEventHandler* m_resizeHandler;
        UiLayoutDirection m_direction;
    };

    class UiLayout : public Frame,
                     public ResizeEventHandler
    {
    public:
        UiLayout(Frame* parent, UiLayoutSettings layoutSettings, UiLayoutDirection direction = UiLayoutDirection::Horizontal);
        ~UiLayout();

        void forceObjectSize(int index, int size);

        void onResize(UiLayoutResizeBar* resizeBar, ui::UiPoint) override;
    protected:
        void onResize(int /*width*/, int /*height*/) override;
        void onAddChild(UiBaseObjectPtr child) override;
        void onRemoveChild(UiBaseObjectPtr child) override;

    private:
        UiLayoutSettings m_layoutSettings;
        UiLayoutDirection m_direction;
        bool m_refreshingResizeBars;
        void relayout();
        void refreshResizeBars();
        engine::vector<int> m_forcedSizes;
        engine::vector<UiBaseObjectPtr> m_items;
        engine::vector<UiBaseObjectPtr> m_resizeBars;

        struct ItemSizeLocationInfo
        {
            int index;
            int begin;
            int size;
        };
        size_t CalculateStaticSize();
        ItemSizeLocationInfo itemSizeLocationInfoFromSettings(int index);
    };

    
}
