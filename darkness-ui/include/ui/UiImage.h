#pragma once

#include "ui/Frame.h"

namespace ui
{
    class UiImage : public ui::Frame
    {
    public:
        UiImage(ui::Frame* parent, const engine::string& imagePath);
    protected:
        void onPaint(ui::DrawCommandBuffer& cmd) override;
    private:
        engine::shared_ptr<engine::image::ImageIf> m_image;
    };
}
