#include "components/ImagePropertiesComponent.h"

namespace engine
{
    int ImagePropertiesComponent::size() const
    {
        return m_size.value<int>();
    }

    void ImagePropertiesComponent::size(int _size)
    {
        m_size.value<int>(_size);
    }

    TextureType ImagePropertiesComponent::textureType() const
    {
        return m_textureType.value<TextureType>();
    }

    void ImagePropertiesComponent::textureType(TextureType _type)
    {
        m_textureType.value<TextureType>(_type);
    }

    ButtonToggle ImagePropertiesComponent::generateMips() const
    {
        return m_generateMips.value<ButtonToggle>();
    }

    void ImagePropertiesComponent::generateMips(ButtonToggle val)
    {
        m_generateMips.value<ButtonToggle>(val);
    }

    template <>
    void writeJsonValue<TextureType>(
        rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, 
        const engine::string& name, 
        TextureType& value)
    {
        LOG_INFO("TODO: ImagePropertiesComponent does not use name parameter: %s", name.c_str());
        writer.String("TextureType");
        writer.String(textureTypeToString(value).data());
    }

}
