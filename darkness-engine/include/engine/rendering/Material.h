#pragma once

#include "engine/graphics/Format.h"
#include "containers/vector.h"
#include "containers/string.h"

namespace engine
{
    enum class TextureMapping
    {
        UV,
        Sphere,
        Cylinder,
        Box,
        Plane
    };

    enum class TextureMapMode
    {
        Wrap,
        Clamp,
        Decal,
        Mirror
    };

    enum class TextureType
    {
        Albedo,
        Roughness,
        Ambient,
        Emissive,
        Height,
        Normal,
        Shininess,
        Opacity,
        Displacement,
        Lightmap,
        Reflection,
        Metalness,
        Occlusion,
        Hdr,
        COUNT
    };

	enum TextureTypeBits
	{
		Albedo = 0x1,
		Roughness = 0x2,
		Ambient = 0x4,
		Emissive = 0x8,
		Height = 0x10,
		Normal = 0x20,
		Shininess = 0x40,
		Opacity = 0x80,
		Displacement = 0x100,
		Lightmap = 0x200,
		Reflection = 0x400,
		Metalness = 0x800,
		Occlusion = 0x1000,
		Hdr = 0x2000,
		All = 0x3fff,
		None = 0xffffc000
	};

    engine::string textureTypeToString(TextureType type);
    TextureType textureTypeFromString(const engine::string& type);

    engine::vector<engine::Format> possibleEncodingFormatRGB(TextureType type);
    engine::vector<engine::Format> possibleEncodingFormatRGB(engine::vector<TextureType> types);
    engine::vector<engine::Format> possibleEncodingFormatSRGB(TextureType type);
    engine::vector<engine::Format> possibleEncodingFormatSRGB(engine::vector<TextureType> types);
    TextureType possibleTextureTypeFromFormat(engine::Format format);

    enum class TextureOp
    {
        Multiply,
        Add,
        Subtract,
        Divide,
        SmoothAdd,
        SignedAdd,

    };

    struct MaterialTexture
    {
        engine::string filePath;
        TextureMapping mapping;
        TextureMapMode mode;
        TextureType type;
        TextureOp op;
        unsigned int uvIndex;

        size_t size() const;
        engine::vector<uint8_t> data() const;
        size_t load(const uint8_t* data);
    };

    class Material
    {
    public:
        engine::vector<MaterialTexture> textures;
        size_t size() const;
        engine::vector<uint8_t> data() const;
        void load(engine::vector<uint8_t> data);
    };
}
