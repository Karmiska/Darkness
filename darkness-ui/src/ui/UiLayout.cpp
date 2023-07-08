#include "ui/UiLayout.h"
#include "tools/Settings.h"

namespace ui
{
    size_t UiLayout::CalculateStaticSize()
    {
        size_t res = 0;
        for (auto&& fs : m_forcedSizes)
            if (fs != -1)
                res += fs;

        if (m_items.size() == 0)
            return res;

        res += (m_items.size()-1) * LayoutPadding;

        return res;
    };

    int DynamicItemCount(const engine::vector<UiBaseObjectPtr>& items, const engine::vector<int>& forcedSizes)
    {
        int count = 0;
        for (int i = 0; i < items.size(); ++i)
        {
            if (i < forcedSizes.size())
            {
                if (forcedSizes[i] == -1)
                    ++count;
            }
            else
            {
                ++count;
            }
        }
        return max(count, 1);
    };

    int ItemSize(int forcedSize, float layoutSizePercentage, int dynamicSpace, int defaultSize)
    {
        if (forcedSize != -1)
        {
            return forcedSize;
        }
        else
        {
            if (layoutSizePercentage >= 0.0f)
                return static_cast<int>(dynamicSpace * layoutSizePercentage);
            else return defaultSize;
        }
    };

    UiLayoutSettingsManager::UiLayoutSettingsManager(const engine::string& settingsFile)
        : m_settings{ engine::make_shared<tools::Settings>(settingsFile) }
    {}

    UiLayoutSettingsManager::~UiLayoutSettingsManager()
    {
        m_settings->save();
    }

    UiLayoutSettings UiLayoutSettingsManager::settings(const char* componentName)
    {
        return UiLayoutSettings(this, componentName);
    }

    UiLayoutSettings::UiLayoutSettings()
        : m_manager{ nullptr }
        , m_componentName{ nullptr }
        , m_sizes{}
    {}

    UiLayoutSettings::UiLayoutSettings(
        UiLayoutSettingsManager* manager,
        const char* componentName)
        : m_manager{ manager }
        , m_componentName{ componentName }
    {
        load();
    }

    float UiLayoutSettings::getSize(int index)
    {
        if (index >= m_sizes.size())
        {
            auto oldSize = m_sizes.size();
            m_sizes.resize(index + 1);
            for (size_t i = oldSize; i < m_sizes.size(); ++i)
                m_sizes[i] = -1;
        }
        return m_sizes[index];
    }

    void UiLayoutSettings::setSize(int index, float size)
    {
        if (index >= m_sizes.size())
        {
            auto oldSize = m_sizes.size();
            m_sizes.resize(index + 1);
            for (size_t i = oldSize; i < m_sizes.size(); ++i)
                m_sizes[i] = -1;
        }
        m_sizes[index] = size;
    }

    void UiLayoutSettings::save()
    {
        tools::Settings* settings = m_manager->m_settings.get();
        settings->beginGroup(m_componentName);
        for (int i = 0; i < m_sizes.size(); ++i)
            settings->set(std::to_string(i), m_sizes[i]);
        settings->endGroup();
    }

    void UiLayoutSettings::load()
    {
        tools::Settings* settings = m_manager->m_settings.get();
        settings->beginGroup(m_componentName);
        m_sizes.resize(settings->keys().size());
        for (int i = 0; i < m_sizes.size(); ++i)
            m_sizes[i] = settings->get<float>(std::to_string(i));
        settings->endGroup();
    }

    UiLayoutResizeBar::UiLayoutResizeBar(
        Frame* parent, 
        ResizeEventHandler* resizeHandler,
        UiLayoutDirection direction)
        : Frame{ parent }
        , m_resizeHandler{ resizeHandler }
        , m_direction{ direction }
    {
        themeSet(false);
        drawBackground(false);
    }

    void UiLayoutResizeBar::onDragMove(ui::UiPoint pt)
    {
        if (m_resizeHandler)
            m_resizeHandler->onResize(this, pt);
    }

    void UiLayoutResizeBar::onMouseMove(int x, int y)
    {
        Frame::onMouseMove(x, y);
        auto frame = dynamic_cast<Frame*>(this);
        auto window = frame->windowShared().get();
        window->mouseCursor(
            (m_direction == UiLayoutDirection::Horizontal) ? platform::MouseCursor::SizeWE : platform::MouseCursor::SizeNS);
    }

    //void UiLayoutResizeBar::onPaint(ui::DrawCommandBuffer& cmd)
    //{
    //    //cmd.drawRectangle(0, 0, width(), height(), engine::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
    //}

    UiLayout::UiLayout(Frame* parent, UiLayoutSettings layoutSettings, UiLayoutDirection direction)
        : Frame(0, 0, parent->api(), parent)
        , m_layoutSettings{ layoutSettings }
        , m_direction{ direction }
        , m_refreshingResizeBars{ false }
    {
        drawBackground(false);
        themeSet(false);
        canFocus(false);
        canMove(AllowedMovement::None);
        canResize(false);
        size(parent->size());
    }

    UiLayout::~UiLayout()
    {
        int entire_size = (m_direction == UiLayoutDirection::Horizontal) ? width() : height();
        int static_size = CalculateStaticSize();
        int dynamic_space = entire_size - static_size;

        for (int i = 0; i < m_items.size(); ++i)
        {
            if (m_forcedSizes[i] != -1)
                m_layoutSettings.setSize(i, -1);
            else
            {
                if (m_direction == UiLayoutDirection::Horizontal)
                {
                    m_layoutSettings.setSize(i, static_cast<float>(std::dynamic_pointer_cast<Frame>(m_items[i])->width()) / static_cast<float>(dynamic_space));
                }
                else if (m_direction == UiLayoutDirection::Vertical)
                {
                    m_layoutSettings.setSize(i, static_cast<float>(std::dynamic_pointer_cast<Frame>(m_items[i])->height()) / static_cast<float>(dynamic_space));
                }
            }
        }
        m_layoutSettings.save();
    }

    void UiLayout::forceObjectSize(int index, int size)
    {
        if (index >= m_forcedSizes.size())
        {
            auto oldSize = m_forcedSizes.size();
            m_forcedSizes.resize(index + 1);
            for (size_t i = oldSize; i < m_forcedSizes.size(); ++i)
                m_forcedSizes[i] = -1;
        }
        m_forcedSizes[index] = size;
    }

    void UiLayout::onResize(UiLayoutResizeBar* resizeBar, ui::UiPoint /*pt*/)
    {
        int entire_size = (m_direction == UiLayoutDirection::Horizontal) ? width() : height();
        int static_size = CalculateStaticSize();
        int dynamic_space = entire_size - static_size;

        for (int i = 0; i < m_resizeBars.size(); ++i)
        {
            if (m_resizeBars[i].get() == resizeBar)
            {
                int resizeableItemIndex = 0;
                for (int a = 0; a < m_items.size(); ++a)
                {
                    if (m_forcedSizes[a] == -1)
                    {
                        if (i == resizeableItemIndex)
                        {
                            auto itemFrame = std::dynamic_pointer_cast<Frame>(m_items[a]);
                            constexpr int HalfBar = ResizeBarSize / 2;
                            auto point = (m_direction == UiLayoutDirection::Horizontal) ? resizeBar->x() : resizeBar->y();
                            auto resizeBarBasedLocation = point + HalfBar;
                            auto oldSize = m_layoutSettings.getSize(a);
                            auto itemPoint = (m_direction == UiLayoutDirection::Horizontal) ? itemFrame->x() : itemFrame->y();
                            auto newSize = static_cast<float>(resizeBarBasedLocation - itemPoint) / static_cast<float>(dynamic_space);
                            m_layoutSettings.setSize(a, newSize);
                            if (a < m_resizeBars.size() - 1)
                            {
                                //itemFrame = std::dynamic_pointer_cast<Frame>(m_items[a + 1]);
                                //auto resizeBarFrame = std::dynamic_pointer_cast<Frame>(m_resizeBars[a + 1]);
                                auto oldSize2 = m_layoutSettings.getSize(a + 1);
                                auto newSize2 = oldSize2 - (newSize - oldSize);
                                m_layoutSettings.setSize(a + 1, newSize2);
                                //m_layoutSettings.setSize(a + 1, static_cast<float>((resizeBarFrame->x() + HalfBar) - itemFrame->x()) / static_cast<float>(dynamic_space));
                            }
                        }
                        ++resizeableItemIndex;
                    }

                }
            }
        }
        relayout();
    }

    void UiLayout::onResize(int /*width*/, int /*height*/)
    {
        relayout();
    }

    void UiLayout::onAddChild(UiBaseObjectPtr child)
    {
        if (m_refreshingResizeBars)
            return;

        m_items.emplace_back(child);
        m_forcedSizes.resize(m_items.size(), -1);
        relayout();
        
    }

    void UiLayout::onRemoveChild(UiBaseObjectPtr child)
    {
        if (m_refreshingResizeBars)
            return;

        m_items.erase(std::find(m_items.begin(), m_items.end(), child));
        m_forcedSizes.resize(m_items.size(), -1);
        relayout();
    }

    void UiLayout::relayout()
    {
        for (int i = 0; i < m_items.size(); ++i)
        {
            auto childPtr = std::dynamic_pointer_cast<Frame>(m_items[i]);
            childPtr->canResize(false);
            childPtr->canMove(ui::AllowedMovement::None);

            auto childInfo = itemSizeLocationInfoFromSettings(i);

            if (m_direction == UiLayoutDirection::Horizontal)
            {
                childPtr->position(UiPoint{ childInfo.begin, 0 });
                childPtr->size(UiPoint{ childInfo.size, height() });
            }
            else
            {
                childPtr->position(UiPoint{ 0, childInfo.begin });
                childPtr->size(UiPoint{ width(), childInfo.size });
            }
        }
        refreshResizeBars();
    }

    void UiLayout::refreshResizeBars()
    {
        m_refreshingResizeBars = true;

        if (m_items.size() == 0)
        {
            m_resizeBars.clear();
        }
        else
        {
            auto createNewBar = [this]()->engine::shared_ptr<UiLayoutResizeBar>
            {
                auto resizeBar = engine::make_shared<UiLayoutResizeBar>(this, this, m_direction);
                resizeBar->alwaysOntop(true);
                addChild(resizeBar);

                if (m_direction == UiLayoutDirection::Horizontal)
                {
                    resizeBar->canMove(AllowedMovement::Horizontal);
                    resizeBar->size({ ResizeBarSize, height() });
                }
                else if (m_direction == UiLayoutDirection::Vertical)
                {
                    resizeBar->canMove(AllowedMovement::Vertical);
                    resizeBar->size({ width(), ResizeBarSize });
                }
                return resizeBar;
            };

            int barCount = 0;
            for (int i = 0; i < m_items.size()-1; ++i)
            {
                if (m_forcedSizes[i] == -1 && m_forcedSizes[i + 1] == -1)
                {
                    auto childInfo = itemSizeLocationInfoFromSettings(i + 1);

                    while (barCount >= m_resizeBars.size())
                    {
                        m_resizeBars.emplace_back(createNewBar());
                    }

                    UiLayoutResizeBar* bar = static_cast<UiLayoutResizeBar*>(m_resizeBars[barCount].get());
                    if (m_direction == UiLayoutDirection::Horizontal)
                    {
                        bar->position({ childInfo.begin - (ResizeBarSize / 2), 0 });
                        bar->size({ ResizeBarSize, height() });
                    }
                    else if (m_direction == UiLayoutDirection::Vertical)
                    {
                        bar->position({ 0, childInfo.begin - (ResizeBarSize / 2) });
                        bar->size({ width(), ResizeBarSize });
                    }

                    ++barCount;
                }
            }
            while (barCount < m_resizeBars.size())
            {
                auto resbar = m_resizeBars.end() - 1;
                removeChild(*resbar);
                m_resizeBars.erase(resbar);
            }
        }

        m_refreshingResizeBars = false;
    }

    UiLayout::ItemSizeLocationInfo UiLayout::itemSizeLocationInfoFromSettings(int index)
    {
        ItemSizeLocationInfo info{ index, 0, 0 };
        int entire_size = (m_direction == UiLayoutDirection::Horizontal) ? width() : height();
        int static_size = CalculateStaticSize();
        int dynamic_space = entire_size - static_size;

        for (int i = 0; i < index; ++i)
            info.begin += ItemSize(m_forcedSizes[i], m_layoutSettings.getSize(i), dynamic_space, dynamic_space / DynamicItemCount(m_items, m_forcedSizes)) + LayoutPadding;

        if (index == m_items.size() - 1 && m_forcedSizes[index] == -1)
        {
            // last item index
            info.size = (m_direction == UiLayoutDirection::Horizontal) ? width() - info.begin : height() - info.begin;
        }
        else
            info.size = ItemSize(m_forcedSizes[index], m_layoutSettings.getSize(index), dynamic_space, dynamic_space / DynamicItemCount(m_items, m_forcedSizes));

        return info;
    }
}
