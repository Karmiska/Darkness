#include "engine/rendering/Material.h"
#include "engine/graphics/Format.h"
#include <algorithm>

namespace engine
{
    engine::string textureTypeToString(TextureType type)
    {
        switch (type)
        {
        case TextureType::Albedo: return "Albedo";
        case TextureType::Roughness: return "Roughness";
        case TextureType::Ambient: return "Ambient";
        case TextureType::Emissive: return "Emissive";
        case TextureType::Height: return "Height";
        case TextureType::Normal: return "Normal";
        case TextureType::Shininess: return "Shininess";
        case TextureType::Opacity: return "Opacity";
        case TextureType::Displacement: return "Displacement";
        case TextureType::Lightmap: return "Lightmap";
        case TextureType::Reflection: return "Reflection";
        case TextureType::Metalness: return "Metalness";
        case TextureType::Occlusion: return "Occlusion";
        case TextureType::Hdr: return "Hdr";
        default: return "";
        }
    }

    TextureType textureTypeFromString(const engine::string& type)
    {
        if (type == "Albedo") return TextureType::Albedo;
        else if (type == "Roughness") return TextureType::Roughness;
        else if (type == "Ambient") return TextureType::Ambient;
        else if (type == "Emissive") return TextureType::Emissive;
        else if (type == "Height") return TextureType::Height;
        else if (type == "Normal") return TextureType::Normal;
        else if (type == "Shininess") return TextureType::Shininess;
        else if (type == "Opacity") return TextureType::Opacity;
        else if (type == "Displacement") return TextureType::Displacement;
        else if (type == "Lightmap") return TextureType::Lightmap;
        else if (type == "Reflection") return TextureType::Reflection;
        else if (type == "Metalness") return TextureType::Metalness;
        else if (type == "Occlusion") return TextureType::Occlusion;
        else if (type == "Hdr") return TextureType::Hdr;
        return TextureType::Albedo;
    }

    engine::vector<Format> possibleEncodingFormatRGB(TextureType type)
    {
        switch (type)
        {
        case TextureType::Normal: return { Format::BC5_UNORM, Format::BC1_UNORM };
        case TextureType::Albedo: return { Format::BC7_UNORM, Format::BC3_UNORM, Format::BC1_UNORM };
        case TextureType::Shininess: return { Format::BC4_UNORM };
        case TextureType::Ambient: return { Format::BC4_UNORM };
        case TextureType::Displacement: return { Format::BC6H_UF16, Format::BC4_UNORM };
        case TextureType::Emissive: return { Format::BC6H_UF16, Format::BC4_UNORM };
        case TextureType::Hdr: return { Format::BC6H_UF16 };
        case TextureType::Height: return { Format::BC4_UNORM };
        case TextureType::Lightmap: return { Format::BC4_UNORM };
        case TextureType::Opacity: return { Format::BC4_UNORM };
        case TextureType::Reflection: return { Format::BC4_UNORM };
        case TextureType::Roughness: return { Format::BC4_UNORM };
        case TextureType::Occlusion: return { Format::BC4_UNORM };
        }
        return { Format::BC1_UNORM };
    }

    TextureType possibleTextureTypeFromFormat(Format format)
    {
        switch (format)
        {
        case Format::BC1_UNORM: return TextureType::Albedo;
        case Format::BC3_UNORM: return TextureType::Albedo;
        case Format::BC4_UNORM: return TextureType::Roughness;
        case Format::BC5_UNORM: return TextureType::Normal;
        case Format::BC6H_UF16: return TextureType::Hdr;
        case Format::BC7_UNORM: return TextureType::Albedo;

        case Format::BC1_UNORM_SRGB: return TextureType::Albedo;
        case Format::BC3_UNORM_SRGB: return TextureType::Albedo;
        case Format::BC7_UNORM_SRGB: return TextureType::Albedo;

        default: return TextureType::Albedo;
        }
    }

    engine::vector<Format> possibleEncodingFormatRGB(engine::vector<TextureType> types)
    {
        engine::vector<Format> formats;
        for (auto&& type : types)
        {
            auto f = possibleEncodingFormatRGB(type);
            formats.reserve(formats.size() + std::distance(f.begin(), f.end()));
            formats.insert(formats.end(), f.begin(), f.end());
        }
        formats.erase(std::unique(formats.begin(), formats.end()), formats.end());
        return formats;
    }

    engine::vector<Format> possibleEncodingFormatSRGB(TextureType type)
    {
        switch (type)
        {
        case TextureType::Albedo: return { Format::BC7_UNORM_SRGB, Format::BC3_UNORM_SRGB, Format::BC1_UNORM_SRGB };

                                  // really makes no sense for these
        case TextureType::Normal: return { Format::BC1_UNORM_SRGB };
        case TextureType::Shininess: return { Format::BC6H_UF16, Format::BC4_UNORM };
        case TextureType::Ambient: return { Format::BC4_UNORM };
        case TextureType::Displacement: return { Format::BC6H_UF16, Format::BC4_UNORM };
        case TextureType::Emissive: return { Format::BC6H_UF16, Format::BC4_UNORM };
        case TextureType::Hdr: return { Format::BC6H_UF16 };
        case TextureType::Height: return { Format::BC4_UNORM };
        case TextureType::Lightmap: return { Format::BC4_UNORM };
        case TextureType::Opacity: return { Format::BC4_UNORM };
        case TextureType::Reflection: return { Format::BC4_UNORM };
        case TextureType::Roughness: return { Format::BC4_UNORM };
        case TextureType::Occlusion: return { Format::BC4_UNORM };
        }
        return { Format::BC1_UNORM_SRGB };
    }

    engine::vector<Format> possibleEncodingFormatSRGB(engine::vector<TextureType> types)
    {
        engine::vector<Format> formats;
        for (auto&& type : types)
        {
            auto f = possibleEncodingFormatSRGB(type);
            formats.reserve(formats.size() + std::distance(f.begin(), f.end()));
            formats.insert(formats.end(), f.begin(), f.end());
        }
        formats.erase(std::unique(formats.begin(), formats.end()), formats.end());
        return formats;
    }

    size_t MaterialTexture::size() const
    {
        return 
            sizeof(uint32_t) + filePath.size() + // path
            sizeof(TextureMapping) + 
            sizeof(TextureMapMode) + 
            sizeof(TextureType) + 
            sizeof(TextureOp) +
            sizeof(unsigned int); // uv index
    }

    engine::vector<uint8_t> MaterialTexture::data() const
    {
        engine::vector<uint8_t> data(size());
        uint32_t size = static_cast<uint32_t>(filePath.size());
        int pos = 0;
        memcpy(&data[pos], &size, sizeof(uint32_t)); pos += sizeof(uint32_t);
        memcpy(&data[pos], filePath.data(), filePath.size()); pos += static_cast<int>(filePath.size());
        memcpy(&data[pos], &mapping, sizeof(TextureMapping)); pos += sizeof(TextureMapping);
        memcpy(&data[pos], &mode, sizeof(TextureMapMode)); pos += sizeof(TextureMapMode);
        memcpy(&data[pos], &type, sizeof(TextureType)); pos += sizeof(TextureType);
        memcpy(&data[pos], &op, sizeof(TextureOp)); pos += sizeof(TextureOp);
        memcpy(&data[pos], &uvIndex, sizeof(unsigned int)); pos += sizeof(unsigned int);
        return data;
    }

    size_t MaterialTexture::load(const uint8_t* data)
    {
        auto copy = data;
        uint32_t strsize;
        memcpy(&strsize, copy, sizeof(uint32_t)); copy += sizeof(uint32_t);
        filePath.resize(strsize);
        memcpy(&filePath[0], copy, strsize); copy += strsize;

        memcpy(&mapping, copy, sizeof(TextureMapping)); copy += sizeof(TextureMapping);
        memcpy(&mode, copy, sizeof(TextureMapMode)); copy += sizeof(TextureMapMode);
        memcpy(&type, copy, sizeof(TextureType)); copy += sizeof(TextureType);
        memcpy(&op, copy, sizeof(TextureOp)); copy += sizeof(TextureOp);
        memcpy(&uvIndex, copy, sizeof(unsigned int)); copy += sizeof(unsigned int);
        return size();
    }

    size_t Material::size() const
    {
        size_t res = 0;
        for (auto&& tex : textures)
        {
            res += tex.size();
        }
        return res;
    }

    engine::vector<uint8_t> Material::data() const
    {
        engine::vector<uint8_t> data;
        for (auto&& tex : textures)
        {
            auto texData = tex.data();
            data.insert(data.end(), texData.begin(), texData.end());
        }
        return data;
    }

    void Material::load(engine::vector<uint8_t> data)
    {
        size_t pos = 0;
        while (pos < data.size())
        {
            textures.emplace_back(MaterialTexture{});
            pos += textures.back().load(&data[pos]);
        }
    }
}
