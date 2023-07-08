#include "ui/UiImage.h"
#include "engine/graphics/Device.h"

namespace ui
{
    UiImage::UiImage(ui::Frame* parent, const engine::string& imagePath)
        : Frame{ parent }
    {
        m_image = device().createImage(imagePath, engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);
        ASSERT(m_image != nullptr, "Failed to create image");
    }

    void UiImage::onPaint(ui::DrawCommandBuffer& cmd)
    {
        cmd.drawImage(0, 0, width(), height(), m_image);
    }
}
