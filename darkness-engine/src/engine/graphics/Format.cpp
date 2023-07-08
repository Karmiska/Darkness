#include "engine/graphics/Format.h"
#include "tools/ToolsCommon.h"
#include "tools/Debug.h"
#include <algorithm>

using namespace engine;

namespace engine
{
    size_t formatBytes(Format format, size_t width, size_t height)
    {
        auto elementBytes = formatBytes(format);
        if (!isBlockCompressedFormat(format))
            return elementBytes * width * height;

        switch (format)
        {
            case Format::BC1_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC1_UNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC1_UNORM_SRGB    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC2_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC2_UNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC2_UNORM_SRGB    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC3_UNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC3_UNORM_SRGB    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC3_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC4_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC4_UNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC4_SNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC5_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC5_UNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC5_SNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC6H_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC6H_UF16        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC6H_SF16        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC7_TYPELESS    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC7_UNORM        : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
            case Format::BC7_UNORM_SRGB    : return std::max<size_t>(1u, ((width + 3u) / 4u)) * std::max<size_t>(1u, ((height + 3u) / 4u)) * elementBytes;
        }
        return elementBytes * width * height;
    }

    size_t formatBytes(Format format)
    {
        return std::max<size_t>(formatBits(format) / 8, 1);
    }

    size_t formatBits(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:
            return 0;

        case Format::R32G32B32A32_TYPELESS:
        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 128;
        
        case Format::R32G32B32_TYPELESS:
        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 96;

        case Format::R16G16B16A16_TYPELESS:
        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
        case Format::R32G32_TYPELESS:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
        case Format::R32G8X24_TYPELESS:
        case Format::D32_FLOAT_S8X24_UINT:
        case Format::R32_FLOAT_X8X24_TYPELESS:
        case Format::X32_TYPELESS_G8X24_UINT:
            return 64;

        case Format::R10G10B10A2_TYPELESS:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
        case Format::R11G11B10_FLOAT:
        case Format::R8G8B8A8_TYPELESS:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
        case Format::R16G16_TYPELESS:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
        case Format::R32_TYPELESS:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
        case Format::R24G8_TYPELESS:
        case Format::D24_UNORM_S8_UINT:
        case Format::R24_UNORM_X8_TYPELESS:
        case Format::X24_TYPELESS_G8_UINT:
        case Format::R9G9B9E5_SHAREDEXP:
        case Format::R8G8_B8G8_UNORM:
        case Format::G8R8_G8B8_UNORM:
        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8X8_UNORM:
        case Format::R10G10B10_XR_BIAS_A2_UNORM:
        case Format::B8G8R8A8_TYPELESS:
        case Format::B8G8R8A8_UNORM_SRGB:
        case Format::B8G8R8X8_TYPELESS:
        case Format::B8G8R8X8_UNORM_SRGB:
            return 32;

        case Format::R8G8_TYPELESS:
        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
        case Format::R16_TYPELESS:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
        case Format::B5G6R5_UNORM:
        case Format::B5G5R5A1_UNORM:
            return 16;

        case Format::R8_TYPELESS:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
            return 8;

        case Format::R1_UNORM:
            return 1;

        case Format::BC1_TYPELESS:
        case Format::BC1_UNORM:
        case Format::BC1_UNORM_SRGB:
        case Format::BC4_TYPELESS:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
            return 64;

        case Format::BC2_TYPELESS:
        case Format::BC2_UNORM:
        case Format::BC2_UNORM_SRGB:
        case Format::BC3_TYPELESS:
        case Format::BC3_UNORM:
        case Format::BC3_UNORM_SRGB:
        case Format::BC5_TYPELESS:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_TYPELESS:
        case Format::BC6H_UF16:
        case Format::BC6H_SF16:
        case Format::BC7_TYPELESS:
        case Format::BC7_UNORM:
        case Format::BC7_UNORM_SRGB:
            return 128;

        case Format::AYUV:
        case Format::Y410:                        return 32;

        // these formats can't really be expressed correctly here
        case Format::Y416:                        return 64;
        case Format::NV12:                        return 16;
        case Format::P010:                        return 32;
        case Format::P016:                        return 32;
        case Format::OPAQUE_420:                  return 8;
        case Format::YUY2:                        return 32;
        case Format::Y210:                        return 64;
        case Format::Y216:                        return 32;

        case Format::NV11:                        return 8;
        case Format::AI44:                        return 4;
        case Format::IA44:                        return 4;
        case Format::P8:                          return 8;
        case Format::A8P8:                        return 8;
        case Format::B4G4R4A4_UNORM:              return 16;

        case Format::P208:                        return 8;
        case Format::V208:                        return 8;
        case Format::V408:                        return 8;
        }
        return 8;
    };

    Format srgbFormat(Format format)
    {
        if ((format == Format::R8G8B8A8_TYPELESS) ||
            (format == Format::R8G8B8A8_UNORM) ||
            (format == Format::R8G8B8A8_UNORM_SRGB) ||
            (format == Format::R8G8B8A8_UINT) ||
            (format == Format::R8G8B8A8_SNORM) ||
            (format == Format::R8G8B8A8_SINT))
            return Format::R8G8B8A8_UNORM_SRGB;

        else if (
            (format == Format::BC1_TYPELESS) ||
            (format == Format::BC1_UNORM) ||
            (format == Format::BC1_UNORM_SRGB))
            return Format::BC1_UNORM_SRGB;

        else if (
            (format == Format::BC2_TYPELESS) ||
            (format == Format::BC2_UNORM) ||
            (format == Format::BC2_UNORM_SRGB))
            return Format::BC2_UNORM_SRGB;

        else if (
            (format == Format::BC3_TYPELESS) ||
            (format == Format::BC3_UNORM) ||
            (format == Format::BC3_UNORM_SRGB))
            return Format::BC3_UNORM_SRGB;

        else if (
            (format == Format::B8G8R8A8_TYPELESS) ||
            (format == Format::B8G8R8A8_UNORM_SRGB) ||
            (format == Format::B8G8R8X8_TYPELESS) ||
            (format == Format::B8G8R8X8_UNORM_SRGB))
            return Format::B8G8R8X8_UNORM_SRGB;

        else if (
            (format == Format::BC7_TYPELESS) ||
            (format == Format::BC7_UNORM) ||
            (format == Format::BC7_UNORM_SRGB))
            return Format::BC7_UNORM_SRGB;

        return format;
    }

    bool isBlockCompressedFormat(Format format)
    {
        return ( // most common first
                (format == Format::BC7_UNORM) ||
                (format == Format::BC7_UNORM_SRGB) ||
                (format == Format::BC3_UNORM) ||
                (format == Format::BC3_UNORM_SRGB) ||

                (format == Format::BC1_TYPELESS) ||
                (format == Format::BC1_UNORM) ||
                (format == Format::BC1_UNORM_SRGB) ||
                (format == Format::BC2_TYPELESS) ||
                (format == Format::BC2_UNORM) ||
                (format == Format::BC2_UNORM_SRGB) ||
                (format == Format::BC3_TYPELESS) ||
                (format == Format::BC4_TYPELESS) ||
                (format == Format::BC4_UNORM) ||
                (format == Format::BC4_SNORM) ||
                (format == Format::BC5_TYPELESS) ||
                (format == Format::BC5_UNORM) ||
                (format == Format::BC5_SNORM) ||
                (format == Format::BC6H_TYPELESS) ||
                (format == Format::BC6H_UF16) ||
                (format == Format::BC6H_SF16) ||
                (format == Format::BC7_TYPELESS)
                );
    }

    engine::string formatToString(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:                       return "UNKNOWN";
        case Format::R32G32B32A32_TYPELESS:         return "R32G32B32A32_TYPELESS";
        case Format::R32G32B32A32_FLOAT:            return "R32G32B32A32_FLOAT";
        case Format::R32G32B32A32_UINT:             return "R32G32B32A32_UINT";
        case Format::R32G32B32A32_SINT:             return "R32G32B32A32_SINT";
        case Format::R32G32B32_TYPELESS:            return "R32G32B32_TYPELESS";
        case Format::R32G32B32_FLOAT:               return "R32G32B32_FLOAT";
        case Format::R32G32B32_UINT:                return "R32G32B32_UINT";
        case Format::R32G32B32_SINT:                return "R32G32B32_SINT";
        case Format::R16G16B16A16_TYPELESS:         return "R16G16B16A16_TYPELESS";
        case Format::R16G16B16A16_FLOAT:            return "R16G16B16A16_FLOAT";
        case Format::R16G16B16A16_UNORM:            return "R16G16B16A16_UNORM";
        case Format::R16G16B16A16_UINT:             return "R16G16B16A16_UINT";
        case Format::R16G16B16A16_SNORM:            return "R16G16B16A16_SNORM";
        case Format::R16G16B16A16_SINT:             return "R16G16B16A16_SINT";
        case Format::R32G32_TYPELESS:               return "R32G32_TYPELESS";
        case Format::R32G32_FLOAT:                  return "R32G32_FLOAT";
        case Format::R32G32_UINT:                   return "R32G32_UINT";
        case Format::R32G32_SINT:                   return "R32G32_SINT";
        case Format::R32G8X24_TYPELESS:             return "R32G8X24_TYPELESS";
        case Format::D32_FLOAT_S8X24_UINT:          return "D32_FLOAT_S8X24_UINT";
        case Format::R32_FLOAT_X8X24_TYPELESS:      return "R32_FLOAT_X8X24_TYPELESS";
        case Format::X32_TYPELESS_G8X24_UINT:       return "X32_TYPELESS_G8X24_UINT";
        case Format::R10G10B10A2_TYPELESS:          return "R10G10B10A2_TYPELESS";
        case Format::R10G10B10A2_UNORM:             return "R10G10B10A2_UNORM";
        case Format::R10G10B10A2_UINT:              return "R10G10B10A2_UINT";
        case Format::R11G11B10_FLOAT:               return "R11G11B10_FLOAT";
        case Format::R8G8B8A8_TYPELESS:             return "R8G8B8A8_TYPELESS";
        case Format::R8G8B8A8_UNORM:                return "R8G8B8A8_UNORM";
        case Format::R8G8B8A8_UNORM_SRGB:           return "R8G8B8A8_UNORM_SRGB";
        case Format::R8G8B8A8_UINT:                 return "R8G8B8A8_UINT";
        case Format::R8G8B8A8_SNORM:                return "R8G8B8A8_SNORM";
        case Format::R8G8B8A8_SINT:                 return "R8G8B8A8_SINT";
        case Format::R16G16_TYPELESS:               return "R16G16_TYPELESS";
        case Format::R16G16_FLOAT:                  return "R16G16_FLOAT";
        case Format::R16G16_UNORM:                  return "R16G16_UNORM";
        case Format::R16G16_UINT:                   return "R16G16_UINT";
        case Format::R16G16_SNORM:                  return "R16G16_SNORM";
        case Format::R16G16_SINT:                   return "R16G16_SINT";
        case Format::R32_TYPELESS:                  return "R32_TYPELESS";
        case Format::D32_FLOAT:                     return "D32_FLOAT";
        case Format::R32_FLOAT:                     return "R32_FLOAT";
        case Format::R32_UINT:                      return "R32_UINT";
        case Format::R32_SINT:                      return "R32_SINT";
        case Format::R24G8_TYPELESS:                return "R24G8_TYPELESS";
        case Format::D24_UNORM_S8_UINT:             return "D24_UNORM_S8_UINT";
        case Format::R24_UNORM_X8_TYPELESS:         return "R24_UNORM_X8_TYPELESS";
        case Format::X24_TYPELESS_G8_UINT:          return "X24_TYPELESS_G8_UINT";
        case Format::R8G8_TYPELESS:                 return "R8G8_TYPELESS";
        case Format::R8G8_UNORM:                    return "R8G8_UNORM";
        case Format::R8G8_UINT:                     return "R8G8_UINT";
        case Format::R8G8_SNORM:                    return "R8G8_SNORM";
        case Format::R8G8_SINT:                     return "R8G8_SINT";
        case Format::R16_TYPELESS:                  return "R16_TYPELESS";
        case Format::R16_FLOAT:                     return "R16_FLOAT";
        case Format::D16_UNORM:                     return "D16_UNORM";
        case Format::R16_UNORM:                     return "R16_UNORM";
        case Format::R16_UINT:                      return "R16_UINT";
        case Format::R16_SNORM:                     return "R16_SNORM";
        case Format::R16_SINT:                      return "R16_SINT";
        case Format::R8_TYPELESS:                   return "R8_TYPELESS";
        case Format::R8_UNORM:                      return "R8_UNORM";
        case Format::R8_UINT:                       return "R8_UINT";
        case Format::R8_SNORM:                      return "R8_SNORM";
        case Format::R8_SINT:                       return "R8_SINT";
        case Format::A8_UNORM:                      return "A8_UNORM";
        case Format::R1_UNORM:                      return "R1_UNORM";
        case Format::R9G9B9E5_SHAREDEXP:            return "R9G9B9E5_SHAREDEXP";
        case Format::R8G8_B8G8_UNORM:               return "R8G8_B8G8_UNORM";
        case Format::G8R8_G8B8_UNORM:               return "G8R8_G8B8_UNORM";
        case Format::BC1_TYPELESS:                  return "BC1_TYPELESS";
        case Format::BC1_UNORM:                     return "BC1_UNORM";
        case Format::BC1_UNORM_SRGB:                return "BC1_UNORM_SRGB";
        case Format::BC2_TYPELESS:                  return "BC2_TYPELESS";
        case Format::BC2_UNORM:                     return "BC2_UNORM";
        case Format::BC2_UNORM_SRGB:                return "BC2_UNORM_SRGB";
        case Format::BC3_TYPELESS:                  return "BC3_TYPELESS";
        case Format::BC3_UNORM:                     return "BC3_UNORM";
        case Format::BC3_UNORM_SRGB:                return "BC3_UNORM_SRGB";
        case Format::BC4_TYPELESS:                  return "BC4_TYPELESS";
        case Format::BC4_UNORM:                     return "BC4_UNORM";
        case Format::BC4_SNORM:                     return "BC4_SNORM";
        case Format::BC5_TYPELESS:                  return "BC5_TYPELESS";
        case Format::BC5_UNORM:                     return "BC5_UNORM";
        case Format::BC5_SNORM:                     return "BC5_SNORM";
        case Format::B5G6R5_UNORM:                  return "B5G6R5_UNORM";
        case Format::B5G5R5A1_UNORM:                return "B5G5R5A1_UNORM";
        case Format::B8G8R8A8_UNORM:                return "B8G8R8A8_UNORM";
        case Format::B8G8R8X8_UNORM:                return "B8G8R8X8_UNORM";
        case Format::R10G10B10_XR_BIAS_A2_UNORM:    return "R10G10B10_XR_BIAS_A2_UNORM";
        case Format::B8G8R8A8_TYPELESS:             return "B8G8R8A8_TYPELESS";
        case Format::B8G8R8A8_UNORM_SRGB:           return "B8G8R8A8_UNORM_SRGB";
        case Format::B8G8R8X8_TYPELESS:             return "B8G8R8X8_TYPELESS";
        case Format::B8G8R8X8_UNORM_SRGB:           return "B8G8R8X8_UNORM_SRGB";
        case Format::BC6H_TYPELESS:                 return "BC6H_TYPELESS";
        case Format::BC6H_UF16:                     return "BC6H_UF16";
        case Format::BC6H_SF16:                     return "BC6H_SF16";
        case Format::BC7_TYPELESS:                  return "BC7_TYPELESS";
        case Format::BC7_UNORM:                     return "BC7_UNORM";
        case Format::BC7_UNORM_SRGB:                return "BC7_UNORM_SRGB";
        case Format::AYUV:                          return "AYUV";
        case Format::Y410:                          return "Y410";
        case Format::Y416:                          return "Y416";
        case Format::NV12:                          return "NV12";
        case Format::P010:                          return "P010";
        case Format::P016:                          return "P016";
        case Format::OPAQUE_420:                    return "OPAQUE_420";
        case Format::YUY2:                          return "YUY2";
        case Format::Y210:                          return "Y210";
        case Format::Y216:                          return "Y216";
        case Format::NV11:                          return "NV11";
        case Format::AI44:                          return "AI44";
        case Format::IA44:                          return "IA44";
        case Format::P8:                            return "P8";
        case Format::A8P8:                          return "A8P8";
        case Format::B4G4R4A4_UNORM:                return "B4G4R4A4_UNORM";
        case Format::P208:                          return "P208";
        case Format::V208:                          return "V208";
        case Format::V408:                          return "V408";
        default: return "BC1_UNORM";
        }
    }

    Format formatFromString(const engine::string& format)
    {
        if (format == "UNKNOWN") return Format::UNKNOWN;
        else if (format == "R32G32B32A32_TYPELESS") return Format::R32G32B32A32_TYPELESS;
        else if (format == "R32G32B32A32_FLOAT") return Format::R32G32B32A32_FLOAT;
        else if (format == "R32G32B32A32_UINT") return Format::R32G32B32A32_UINT;
        else if (format == "R32G32B32A32_SINT") return Format::R32G32B32A32_SINT;
        else if (format == "R32G32B32_TYPELESS") return Format::R32G32B32_TYPELESS;
        else if (format == "R32G32B32_FLOAT") return Format::R32G32B32_FLOAT;
        else if (format == "R32G32B32_UINT") return Format::R32G32B32_UINT;
        else if (format == "R32G32B32_SINT") return Format::R32G32B32_SINT;
        else if (format == "R16G16B16A16_TYPELESS") return Format::R16G16B16A16_TYPELESS;
        else if (format == "R16G16B16A16_FLOAT") return Format::R16G16B16A16_FLOAT;
        else if (format == "R16G16B16A16_UNORM") return Format::R16G16B16A16_UNORM;
        else if (format == "R16G16B16A16_UINT") return Format::R16G16B16A16_UINT;
        else if (format == "R16G16B16A16_SNORM") return Format::R16G16B16A16_SNORM;
        else if (format == "R16G16B16A16_SINT") return Format::R16G16B16A16_SINT;
        else if (format == "R32G32_TYPELESS") return Format::R32G32_TYPELESS;
        else if (format == "R32G32_FLOAT") return Format::R32G32_FLOAT;
        else if (format == "R32G32_UINT") return Format::R32G32_UINT;
        else if (format == "R32G32_SINT") return Format::R32G32_SINT;
        else if (format == "R32G8X24_TYPELESS") return Format::R32G8X24_TYPELESS;
        else if (format == "D32_FLOAT_S8X24_UINT") return Format::D32_FLOAT_S8X24_UINT;
        else if (format == "R32_FLOAT_X8X24_TYPELESS") return Format::R32_FLOAT_X8X24_TYPELESS;
        else if (format == "X32_TYPELESS_G8X24_UINT") return Format::X32_TYPELESS_G8X24_UINT;
        else if (format == "R10G10B10A2_TYPELESS") return Format::R10G10B10A2_TYPELESS;
        else if (format == "R10G10B10A2_UNORM") return Format::R10G10B10A2_UNORM;
        else if (format == "R10G10B10A2_UINT") return Format::R10G10B10A2_UINT;
        else if (format == "R11G11B10_FLOAT") return Format::R11G11B10_FLOAT;
        else if (format == "R8G8B8A8_TYPELESS") return Format::R8G8B8A8_TYPELESS;
        else if (format == "R8G8B8A8_UNORM") return Format::R8G8B8A8_UNORM;
        else if (format == "R8G8B8A8_UNORM_SRGB") return Format::R8G8B8A8_UNORM_SRGB;
        else if (format == "R8G8B8A8_UINT") return Format::R8G8B8A8_UINT;
        else if (format == "R8G8B8A8_SNORM") return Format::R8G8B8A8_SNORM;
        else if (format == "R8G8B8A8_SINT") return Format::R8G8B8A8_SINT;
        else if (format == "R16G16_TYPELESS") return Format::R16G16_TYPELESS;
        else if (format == "R16G16_FLOAT") return Format::R16G16_FLOAT;
        else if (format == "R16G16_UNORM") return Format::R16G16_UNORM;
        else if (format == "R16G16_UINT") return Format::R16G16_UINT;
        else if (format == "R16G16_SNORM") return Format::R16G16_SNORM;
        else if (format == "R16G16_SINT") return Format::R16G16_SINT;
        else if (format == "R32_TYPELESS") return Format::R32_TYPELESS;
        else if (format == "D32_FLOAT") return Format::D32_FLOAT;
        else if (format == "R32_FLOAT") return Format::R32_FLOAT;
        else if (format == "R32_UINT") return Format::R32_UINT;
        else if (format == "R32_SINT") return Format::R32_SINT;
        else if (format == "R24G8_TYPELESS") return Format::R24G8_TYPELESS;
        else if (format == "D24_UNORM_S8_UINT") return Format::D24_UNORM_S8_UINT;
        else if (format == "R24_UNORM_X8_TYPELESS") return Format::R24_UNORM_X8_TYPELESS;
        else if (format == "X24_TYPELESS_G8_UINT") return Format::X24_TYPELESS_G8_UINT;
        else if (format == "R8G8_TYPELESS") return Format::R8G8_TYPELESS;
        else if (format == "R8G8_UNORM") return Format::R8G8_UNORM;
        else if (format == "R8G8_UINT") return Format::R8G8_UINT;
        else if (format == "R8G8_SNORM") return Format::R8G8_SNORM;
        else if (format == "R8G8_SINT") return Format::R8G8_SINT;
        else if (format == "R16_TYPELESS") return Format::R16_TYPELESS;
        else if (format == "R16_FLOAT") return Format::R16_FLOAT;
        else if (format == "D16_UNORM") return Format::D16_UNORM;
        else if (format == "R16_UNORM") return Format::R16_UNORM;
        else if (format == "R16_UINT") return Format::R16_UINT;
        else if (format == "R16_SNORM") return Format::R16_SNORM;
        else if (format == "R16_SINT") return Format::R16_SINT;
        else if (format == "R8_TYPELESS") return Format::R8_TYPELESS;
        else if (format == "R8_UNORM") return Format::R8_UNORM;
        else if (format == "R8_UINT") return Format::R8_UINT;
        else if (format == "R8_SNORM") return Format::R8_SNORM;
        else if (format == "R8_SINT") return Format::R8_SINT;
        else if (format == "A8_UNORM") return Format::A8_UNORM;
        else if (format == "R1_UNORM") return Format::R1_UNORM;
        else if (format == "R9G9B9E5_SHAREDEXP") return Format::R9G9B9E5_SHAREDEXP;
        else if (format == "R8G8_B8G8_UNORM") return Format::R8G8_B8G8_UNORM;
        else if (format == "G8R8_G8B8_UNORM") return Format::G8R8_G8B8_UNORM;
        else if (format == "BC1_TYPELESS") return Format::BC1_TYPELESS;
        else if (format == "BC1_UNORM") return Format::BC1_UNORM;
        else if (format == "BC1_UNORM_SRGB") return Format::BC1_UNORM_SRGB;
        else if (format == "BC2_TYPELESS") return Format::BC2_TYPELESS;
        else if (format == "BC2_UNORM") return Format::BC2_UNORM;
        else if (format == "BC2_UNORM_SRGB") return Format::BC2_UNORM_SRGB;
        else if (format == "BC3_TYPELESS") return Format::BC3_TYPELESS;
        else if (format == "BC3_UNORM") return Format::BC3_UNORM;
        else if (format == "BC3_UNORM_SRGB") return Format::BC3_UNORM_SRGB;
        else if (format == "BC4_TYPELESS") return Format::BC4_TYPELESS;
        else if (format == "BC4_UNORM") return Format::BC4_UNORM;
        else if (format == "BC4_SNORM") return Format::BC4_SNORM;
        else if (format == "BC5_TYPELESS") return Format::BC5_TYPELESS;
        else if (format == "BC5_UNORM") return Format::BC5_UNORM;
        else if (format == "BC5_SNORM") return Format::BC5_SNORM;
        else if (format == "B5G6R5_UNORM") return Format::B5G6R5_UNORM;
        else if (format == "B5G5R5A1_UNORM") return Format::B5G5R5A1_UNORM;
        else if (format == "B8G8R8A8_UNORM") return Format::B8G8R8A8_UNORM;
        else if (format == "B8G8R8X8_UNORM") return Format::B8G8R8X8_UNORM;
        else if (format == "R10G10B10_XR_BIAS_A2_UNORM") return Format::R10G10B10_XR_BIAS_A2_UNORM;
        else if (format == "B8G8R8A8_TYPELESS") return Format::B8G8R8A8_TYPELESS;
        else if (format == "B8G8R8A8_UNORM_SRGB") return Format::B8G8R8A8_UNORM_SRGB;
        else if (format == "B8G8R8X8_TYPELESS") return Format::B8G8R8X8_TYPELESS;
        else if (format == "B8G8R8X8_UNORM_SRGB") return Format::B8G8R8X8_UNORM_SRGB;
        else if (format == "BC6H_TYPELESS") return Format::BC6H_TYPELESS;
        else if (format == "BC6H_UF16") return Format::BC6H_UF16;
        else if (format == "BC6H_SF16") return Format::BC6H_SF16;
        else if (format == "BC7_TYPELESS") return Format::BC7_TYPELESS;
        else if (format == "BC7_UNORM") return Format::BC7_UNORM;
        else if (format == "BC7_UNORM_SRGB") return Format::BC7_UNORM_SRGB;
        else if (format == "AYUV") return Format::AYUV;
        else if (format == "Y410") return Format::Y410;
        else if (format == "Y416") return Format::Y416;
        else if (format == "NV12") return Format::NV12;
        else if (format == "P010") return Format::P010;
        else if (format == "P016") return Format::P016;
        else if (format == "OPAQUE_420") return Format::OPAQUE_420;
        else if (format == "YUY2") return Format::YUY2;
        else if (format == "Y210") return Format::Y210;
        else if (format == "Y216") return Format::Y216;
        else if (format == "NV11") return Format::NV11;
        else if (format == "AI44") return Format::AI44;
        else if (format == "IA44") return Format::IA44;
        else if (format == "P8") return Format::P8;
        else if (format == "A8P8") return Format::A8P8;
        else if (format == "B4G4R4A4_UNORM") return Format::B4G4R4A4_UNORM;
        else if (format == "P208") return Format::P208;
        else if (format == "V208") return Format::V208;
        else if (format == "V408") return Format::V408;

        return Format::BC1_UNORM;
    }

    SurfaceInformation surfaceInformation(Format format, size_t width, size_t height)
    {
        SurfaceInformation info;
        if (isBlockCompressedFormat(format))
        {
            size_t blockWidth = width > 0 ? std::max(1ull, ((width + 3ull) / 4ull)) : 0ull;
            size_t blockHeight = height > 0 ? std::max(1ull, ((height + 3ull) / 4ull)) : 0ull;
            size_t bytesPerBlock = formatBytes(format);
            info.rowBytes = blockWidth * bytesPerBlock;
            info.numRows = blockHeight;
        }
        else if ((format == Format::R8G8_B8G8_UNORM) ||
            (format == Format::G8R8_G8B8_UNORM) ||
            (format == Format::YUY2))
        {
            info.rowBytes = ((width + 1ull) >> 1ull) * 4ull;
            info.numRows = height;
        }
        else
        {
            info.rowBytes = (width * formatBits(format) + 7ull) / 8ull;
            info.numRows = height;
        }
        info.numBytes = info.rowBytes * info.numRows;
        return info;
    }

	engine::vector<SurfacePiece> surfacePieces(
		Format format,
		SurfacePieceImage srcImage,
		SurfacePieceImage dstImage,
		size_t srcX,
		size_t srcY,
		size_t dstX,
		size_t dstY,
		size_t width,
		size_t height)
	{
		auto blockCompression = isBlockCompressedFormat(format);

		if (blockCompression)
		{
			ASSERT(srcX % 4ull == 0, "Trying to copy block compressed image from between blocks");
			ASSERT(srcY % 4ull == 0, "Trying to copy block compressed image from between blocks");
			ASSERT(dstX % 4ull == 0, "Trying to copy block compressed image to between blocks");
			ASSERT(dstY % 4ull == 0, "Trying to copy block compressed image to between blocks");

			ASSERT(width % 4ull == 0, "Trying to copy block compressed image from between blocks");
			ASSERT(height % 4ull == 0, "Trying to copy block compressed image from between blocks");
		}
		
		ASSERT(srcX >= 0ull, "Trying to copy from outside the source image");
		ASSERT(srcY >= 0ull, "Trying to copy from outside the source image");
		ASSERT(srcX + width <= srcImage.width, "Trying to copy from outside the source image");
		ASSERT(srcY + height <= srcImage.height, "Trying to copy from outside the source image");

		ASSERT(dstX >= 0ull, "Trying to copy from outside the destination image");
		ASSERT(dstY >= 0ull, "Trying to copy from outside the destination image");
		ASSERT(dstX + width <= dstImage.width, "Trying to copy to outside the destination image");
		ASSERT(dstY + height <= dstImage.height, "Trying to copy to outside the destination image");

		
		engine::vector<SurfacePiece> result;

		// block compressed formats are 4 x 4
		const auto compressedBlockSize = 4;
		auto minBlockSize = blockCompression ? compressedBlockSize : 1ull;

		// blockSize needs to be a multiple of minBlockSize
		const auto pieceSize = 128ull;
		ASSERT((pieceSize % minBlockSize) == 0, "Block size needs to be a multiple of 4");

		auto pieceCountX = std::max(roundUpToMultiple(width, pieceSize) / pieceSize, 1ull);
		auto pieceCountY = std::max(roundUpToMultiple(height, pieceSize) / pieceSize, 1ull);
		
		auto blockBytes = formatBytes(format);

		auto srcPitchBytes = roundUpToMultiple(roundUpToMultiple(srcImage.width, minBlockSize) / minBlockSize * blockBytes, srcImage.rowPitchAlign);
		auto dstPitchBytes = roundUpToMultiple(roundUpToMultiple(dstImage.width, minBlockSize) / minBlockSize * blockBytes, dstImage.rowPitchAlign);

		for (int y = 0; y < pieceCountY; ++y)
		{
			for (int x = 0; x < pieceCountX; ++x)
			{
				SurfacePiece piece;

				auto srcXbytes = (srcX + (x * pieceSize)) / minBlockSize * blockBytes;
				auto srcYbytes = (srcY + (y * pieceSize)) / minBlockSize * srcPitchBytes;

				auto dstXbytes = (dstX + (x * pieceSize)) / minBlockSize * blockBytes;
				auto dstYbytes = (dstY + (y * pieceSize)) / minBlockSize * dstPitchBytes;

				piece.srcptr = srcImage.ptr + srcXbytes + srcYbytes;
				piece.srcRowPitch = srcPitchBytes;
				piece.srcX = srcX + (x * pieceSize);
				piece.srcY = srcY + (y * pieceSize);

				piece.dstptr = dstImage.ptr + dstXbytes + dstYbytes;
				piece.dstRowPitch = dstPitchBytes;
				piece.dstX = dstX + (x * pieceSize);
				piece.dstY = dstY + (y * pieceSize);
				
				auto pieceWidth = std::min(roundUpToMultiple(pieceSize, minBlockSize) / minBlockSize, srcImage.width - piece.srcX);
				auto pieceHeight = std::min(roundUpToMultiple(pieceSize, minBlockSize) / minBlockSize, srcImage.height - piece.srcY);

				piece.bytes = pieceWidth * pieceHeight * blockBytes;
				piece.width = pieceWidth;
				piece.height = pieceHeight;

				result.emplace_back(piece);
			}
		}
		return result;
	}

    size_t imageBytes(Format format, size_t width, size_t height, size_t slices, size_t mips)
    {
        size_t bytes = 0;
        for (size_t slice = 0; slice < slices; ++slice)
        {
            auto w = width;
            auto h = height;
            for (unsigned int mip = 0; mip < mips; ++mip)
            {
                auto info = surfaceInformation(format, w, h);
                bytes += info.numBytes;
                w /= 2;
                h /= 2;
            }
        }
        return bytes;
    }

    uint64_t formatMaskRed(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:
            return 0;

        case Format::R32G32B32A32_TYPELESS:
        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 0; // can't return 128 bit number mask

        case Format::R32G32B32_TYPELESS:
        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 0; // can't return 96 bit number mask

        case Format::R16G16B16A16_TYPELESS:
        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
            return 0xffff000000000000;

        case Format::R32G32_TYPELESS:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
            return 0xffffffff00000000;

        case Format::R32G8X24_TYPELESS:
            return 0xffffffff00000000;

        case Format::D32_FLOAT_S8X24_UINT:
            return 0xffffffff00000000;

        case Format::R32_FLOAT_X8X24_TYPELESS:
            return 0xffffffff00000000;

        case Format::X32_TYPELESS_G8X24_UINT:
            return 0;

        case Format::R10G10B10A2_TYPELESS:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
            return 0xffc00000;

        case Format::R11G11B10_FLOAT:
            return 0xffe00000;

        case Format::R8G8B8A8_TYPELESS:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
            return 0xff000000;

        case Format::R16G16_TYPELESS:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
            return 0xffff0000;

        case Format::R32_TYPELESS:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
            return 0xffffffff;

        case Format::R24G8_TYPELESS:
        case Format::D24_UNORM_S8_UINT:
        case Format::R24_UNORM_X8_TYPELESS:
        case Format::X24_TYPELESS_G8_UINT:
            return 0xffffff00;
        case Format::R9G9B9E5_SHAREDEXP:
            return 0; // who cares?

        case Format::R8G8_B8G8_UNORM:
        case Format::G8R8_G8B8_UNORM:
            return 0; // who cares?

        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8X8_UNORM:
            return 0xff00;

        case Format::R10G10B10_XR_BIAS_A2_UNORM:
            return 0xffc00000;

        case Format::B8G8R8A8_TYPELESS:
        case Format::B8G8R8A8_UNORM_SRGB:
        case Format::B8G8R8X8_TYPELESS:
        case Format::B8G8R8X8_UNORM_SRGB:
            return 0xff00;

        case Format::R8G8_TYPELESS:
        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
            return 0xff00;

        case Format::R16_TYPELESS:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
            return 0xffff;

        case Format::B5G6R5_UNORM:
            return 0x1f;

        case Format::B5G5R5A1_UNORM:
            return 0x3e;

        case Format::R8_TYPELESS:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
            return 0xff;

        case Format::R1_UNORM:
            return 0x1;

        case Format::BC1_TYPELESS:
        case Format::BC1_UNORM:
        case Format::BC1_UNORM_SRGB:
        case Format::BC4_TYPELESS:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
        case Format::BC2_TYPELESS:
        case Format::BC2_UNORM:
        case Format::BC2_UNORM_SRGB:
        case Format::BC3_TYPELESS:
        case Format::BC3_UNORM:
        case Format::BC3_UNORM_SRGB:
        case Format::BC5_TYPELESS:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_TYPELESS:
        case Format::BC6H_UF16:
        case Format::BC6H_SF16:
        case Format::BC7_TYPELESS:
        case Format::BC7_UNORM:
        case Format::BC7_UNORM_SRGB:
            return 0; // not valid

        case Format::AYUV:
        case Format::Y410:
        case Format::Y416:
        case Format::NV12:
        case Format::P010:
        case Format::P016:
        case Format::OPAQUE_420:
        case Format::YUY2:
        case Format::Y210:
        case Format::Y216:
        case Format::NV11:
        case Format::AI44:
        case Format::IA44:
        case Format::P8:  
        case Format::A8P8:
        case Format::P208:
        case Format::V208:
        case Format::V408:
            return 0; // who cares?

        case Format::B4G4R4A4_UNORM:
            return 0xf0;
        }
        return 0;
    }

    uint64_t formatMaskGreen(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:
            return 0;

        case Format::R32G32B32A32_TYPELESS:
        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 0; // can't return 128 bit number mask

        case Format::R32G32B32_TYPELESS:
        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 0; // can't return 96 bit number mask

        case Format::R16G16B16A16_TYPELESS:
        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
            return 0xffff00000000;

        case Format::R32G32_TYPELESS:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
            return 0xffffffff;

        case Format::R32G8X24_TYPELESS:
            return 0xff000000;

        case Format::D32_FLOAT_S8X24_UINT:
            return 0xff000000;

        case Format::R32_FLOAT_X8X24_TYPELESS:
            return 0xff000000;

        case Format::X32_TYPELESS_G8X24_UINT:
            return 0;

        case Format::R10G10B10A2_TYPELESS:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
            return 0x3ff000;

        case Format::R11G11B10_FLOAT:
            return 0x1ffc00;

        case Format::R8G8B8A8_TYPELESS:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
            return 0x00ff0000;

        case Format::R16G16_TYPELESS:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
            return 0x0000ffff;

        case Format::R32_TYPELESS:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
            return 0;

        case Format::R24G8_TYPELESS:
        case Format::D24_UNORM_S8_UINT:
        case Format::R24_UNORM_X8_TYPELESS:
        case Format::X24_TYPELESS_G8_UINT:
            return 0xff;
        case Format::R9G9B9E5_SHAREDEXP:
            return 0; // who cares?

        case Format::R8G8_B8G8_UNORM:
        case Format::G8R8_G8B8_UNORM:
            return 0; // who cares?

        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8X8_UNORM:
            return 0xff0000;

        case Format::R10G10B10_XR_BIAS_A2_UNORM:
            return 0x3ff000;

        case Format::B8G8R8A8_TYPELESS:
        case Format::B8G8R8A8_UNORM_SRGB:
        case Format::B8G8R8X8_TYPELESS:
        case Format::B8G8R8X8_UNORM_SRGB:
            return 0xff0000;

        case Format::R8G8_TYPELESS:
        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
            return 0xff;

        case Format::R16_TYPELESS:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
            return 0;

        case Format::B5G6R5_UNORM:
            return 0x7e0;

        case Format::B5G5R5A1_UNORM:
            return 0x7c0;

        case Format::R8_TYPELESS:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
            return 0;

        case Format::R1_UNORM:
            return 0;

        case Format::BC1_TYPELESS:
        case Format::BC1_UNORM:
        case Format::BC1_UNORM_SRGB:
        case Format::BC4_TYPELESS:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
        case Format::BC2_TYPELESS:
        case Format::BC2_UNORM:
        case Format::BC2_UNORM_SRGB:
        case Format::BC3_TYPELESS:
        case Format::BC3_UNORM:
        case Format::BC3_UNORM_SRGB:
        case Format::BC5_TYPELESS:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_TYPELESS:
        case Format::BC6H_UF16:
        case Format::BC6H_SF16:
        case Format::BC7_TYPELESS:
        case Format::BC7_UNORM:
        case Format::BC7_UNORM_SRGB:
            return 0; // not valid

        case Format::AYUV:
        case Format::Y410:
        case Format::Y416:
        case Format::NV12:
        case Format::P010:
        case Format::P016:
        case Format::OPAQUE_420:
        case Format::YUY2:
        case Format::Y210:
        case Format::Y216:
        case Format::NV11:
        case Format::AI44:
        case Format::IA44:
        case Format::P8:
        case Format::A8P8:
        case Format::P208:
        case Format::V208:
        case Format::V408:
            return 0; // who cares?

        case Format::B4G4R4A4_UNORM:
            return 0xf00;
        }
        return 0;
    }

    uint64_t formatMaskBlue(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:
            return 0;

        case Format::R32G32B32A32_TYPELESS:
        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 0; // can't return 128 bit number mask

        case Format::R32G32B32_TYPELESS:
        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 0; // can't return 96 bit number mask

        case Format::R16G16B16A16_TYPELESS:
        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
            return 0xffff0000;

        case Format::R32G32_TYPELESS:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
            return 0;

        case Format::R32G8X24_TYPELESS:
            return 0;

        case Format::D32_FLOAT_S8X24_UINT:
            return 0;

        case Format::R32_FLOAT_X8X24_TYPELESS:
            return 0;

        case Format::X32_TYPELESS_G8X24_UINT:
            return 0;

        case Format::R10G10B10A2_TYPELESS:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
            return 0xffc;

        case Format::R11G11B10_FLOAT:
            return 0x3ff;

        case Format::R8G8B8A8_TYPELESS:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
            return 0x0000ff00;

        case Format::R16G16_TYPELESS:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
            return 0;

        case Format::R32_TYPELESS:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
            return 0;

        case Format::R24G8_TYPELESS:
        case Format::D24_UNORM_S8_UINT:
        case Format::R24_UNORM_X8_TYPELESS:
        case Format::X24_TYPELESS_G8_UINT:
            return 0;
        case Format::R9G9B9E5_SHAREDEXP:
            return 0; // who cares?

        case Format::R8G8_B8G8_UNORM:
        case Format::G8R8_G8B8_UNORM:
            return 0; // who cares?

        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8X8_UNORM:
            return 0xff000000;

        case Format::R10G10B10_XR_BIAS_A2_UNORM:
            return 0xffc;

        case Format::B8G8R8A8_TYPELESS:
        case Format::B8G8R8A8_UNORM_SRGB:
        case Format::B8G8R8X8_TYPELESS:
        case Format::B8G8R8X8_UNORM_SRGB:
            return 0xff000000;

        case Format::R8G8_TYPELESS:
        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
            return 0;

        case Format::R16_TYPELESS:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
            return 0;

        case Format::B5G6R5_UNORM:
            return 0xf800;

        case Format::B5G5R5A1_UNORM:
            return 0xf800;

        case Format::R8_TYPELESS:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
            return 0;

        case Format::R1_UNORM:
            return 0;

        case Format::BC1_TYPELESS:
        case Format::BC1_UNORM:
        case Format::BC1_UNORM_SRGB:
        case Format::BC4_TYPELESS:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
        case Format::BC2_TYPELESS:
        case Format::BC2_UNORM:
        case Format::BC2_UNORM_SRGB:
        case Format::BC3_TYPELESS:
        case Format::BC3_UNORM:
        case Format::BC3_UNORM_SRGB:
        case Format::BC5_TYPELESS:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_TYPELESS:
        case Format::BC6H_UF16:
        case Format::BC6H_SF16:
        case Format::BC7_TYPELESS:
        case Format::BC7_UNORM:
        case Format::BC7_UNORM_SRGB:
            return 0; // not valid

        case Format::AYUV:
        case Format::Y410:
        case Format::Y416:
        case Format::NV12:
        case Format::P010:
        case Format::P016:
        case Format::OPAQUE_420:
        case Format::YUY2:
        case Format::Y210:
        case Format::Y216:
        case Format::NV11:
        case Format::AI44:
        case Format::IA44:
        case Format::P8:
        case Format::A8P8:
        case Format::P208:
        case Format::V208:
        case Format::V408:
            return 0; // who cares?

        case Format::B4G4R4A4_UNORM:
            return 0xf000;
        }
        return 0;
    }

    uint64_t formatMaskAlpha(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:
            return 0;

        case Format::R32G32B32A32_TYPELESS:
        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 0; // can't return 128 bit number mask

        case Format::R32G32B32_TYPELESS:
        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 0; // can't return 96 bit number mask

        case Format::R16G16B16A16_TYPELESS:
        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
            return 0xffff;

        case Format::R32G32_TYPELESS:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
            return 0;

        case Format::R32G8X24_TYPELESS:
            return 0;

        case Format::D32_FLOAT_S8X24_UINT:
            return 0;

        case Format::R32_FLOAT_X8X24_TYPELESS:
            return 0;

        case Format::X32_TYPELESS_G8X24_UINT:
            return 0;

        case Format::R10G10B10A2_TYPELESS:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
            return 0x3;

        case Format::R11G11B10_FLOAT:
            return 0;

        case Format::R8G8B8A8_TYPELESS:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
            return 0xff;

        case Format::R16G16_TYPELESS:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
            return 0;

        case Format::R32_TYPELESS:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
            return 0;

        case Format::R24G8_TYPELESS:
        case Format::D24_UNORM_S8_UINT:
        case Format::R24_UNORM_X8_TYPELESS:
        case Format::X24_TYPELESS_G8_UINT:
            return 0;
        case Format::R9G9B9E5_SHAREDEXP:
            return 0; // who cares?

        case Format::R8G8_B8G8_UNORM:
        case Format::G8R8_G8B8_UNORM:
            return 0; // who cares?

        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8X8_UNORM:
            return 0xff;

        case Format::R10G10B10_XR_BIAS_A2_UNORM:
            return 0x3;

        case Format::B8G8R8A8_TYPELESS:
        case Format::B8G8R8A8_UNORM_SRGB:
        case Format::B8G8R8X8_TYPELESS:
        case Format::B8G8R8X8_UNORM_SRGB:
            return 0xff;

        case Format::R8G8_TYPELESS:
        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
            return 0;

        case Format::R16_TYPELESS:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
            return 0;

        case Format::B5G6R5_UNORM:
            return 0xf800;

        case Format::B5G5R5A1_UNORM:
            return 0x1;

        case Format::R8_TYPELESS:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
            return 0xff;

        case Format::R1_UNORM:
            return 0;

        case Format::BC1_TYPELESS:
        case Format::BC1_UNORM:
        case Format::BC1_UNORM_SRGB:
        case Format::BC4_TYPELESS:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
        case Format::BC2_TYPELESS:
        case Format::BC2_UNORM:
        case Format::BC2_UNORM_SRGB:
        case Format::BC3_TYPELESS:
        case Format::BC3_UNORM:
        case Format::BC3_UNORM_SRGB:
        case Format::BC5_TYPELESS:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_TYPELESS:
        case Format::BC6H_UF16:
        case Format::BC6H_SF16:
        case Format::BC7_TYPELESS:
        case Format::BC7_UNORM:
        case Format::BC7_UNORM_SRGB:
            return 0; // not valid

        case Format::AYUV:
        case Format::Y410:
        case Format::Y416:
        case Format::NV12:
        case Format::P010:
        case Format::P016:
        case Format::OPAQUE_420:
        case Format::YUY2:
        case Format::Y210:
        case Format::Y216:
        case Format::NV11:
        case Format::AI44:
        case Format::IA44:
        case Format::P8:
        case Format::A8P8:
        case Format::P208:
        case Format::V208:
        case Format::V408:
            return 0; // who cares?

        case Format::B4G4R4A4_UNORM:
            return 0xf;
        }
        return 0;
    }

    uint32_t formatComponents(Format format)
    {
        switch (format)
        {
        case Format::UNKNOWN:
            return 0;

        case Format::R32G32B32A32_TYPELESS:
        case Format::R32G32B32A32_FLOAT:
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
            return 4; // can't return 128 bit number mask

        case Format::R32G32B32_TYPELESS:
        case Format::R32G32B32_FLOAT:
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
            return 3; // can't return 96 bit number mask

        case Format::R16G16B16A16_TYPELESS:
        case Format::R16G16B16A16_FLOAT:
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_SINT:
            return 4;

        case Format::R32G32_TYPELESS:
        case Format::R32G32_FLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
            return 2;

        case Format::R32G8X24_TYPELESS:
            return 2;

        case Format::D32_FLOAT_S8X24_UINT:
            return 2;

        case Format::R32_FLOAT_X8X24_TYPELESS:
            return 2;

        case Format::X32_TYPELESS_G8X24_UINT:
            return 2;

        case Format::R10G10B10A2_TYPELESS:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_UINT:
            return 4;

        case Format::R11G11B10_FLOAT:
            return 3;

        case Format::R8G8B8A8_TYPELESS:
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_UNORM_SRGB:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_SINT:
            return 4;

        case Format::R16G16_TYPELESS:
        case Format::R16G16_FLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SNORM:
        case Format::R16G16_SINT:
            return 2;

        case Format::R32_TYPELESS:
        case Format::D32_FLOAT:
        case Format::R32_FLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
            return 1;

        case Format::R24G8_TYPELESS:
        case Format::D24_UNORM_S8_UINT:
        case Format::R24_UNORM_X8_TYPELESS:
        case Format::X24_TYPELESS_G8_UINT:
            return 2;
        case Format::R9G9B9E5_SHAREDEXP:
            return 4; // who cares?

        case Format::R8G8_B8G8_UNORM:
        case Format::G8R8_G8B8_UNORM:
            return 4; // who cares?

        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8X8_UNORM:
            return 4;

        case Format::R10G10B10_XR_BIAS_A2_UNORM:
            return 4;

        case Format::B8G8R8A8_TYPELESS:
        case Format::B8G8R8A8_UNORM_SRGB:
        case Format::B8G8R8X8_TYPELESS:
        case Format::B8G8R8X8_UNORM_SRGB:
            return 4;

        case Format::R8G8_TYPELESS:
        case Format::R8G8_UNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SNORM:
        case Format::R8G8_SINT:
            return 2;

        case Format::R16_TYPELESS:
        case Format::R16_FLOAT:
        case Format::D16_UNORM:
        case Format::R16_UNORM:
        case Format::R16_UINT:
        case Format::R16_SNORM:
        case Format::R16_SINT:
            return 1;

        case Format::B5G6R5_UNORM:
            return 3;

        case Format::B5G5R5A1_UNORM:
            return 4;

        case Format::R8_TYPELESS:
        case Format::R8_UNORM:
        case Format::R8_UINT:
        case Format::R8_SNORM:
        case Format::R8_SINT:
        case Format::A8_UNORM:
            return 1;

        case Format::R1_UNORM:
            return 1;

        case Format::BC1_TYPELESS:
        case Format::BC1_UNORM:
        case Format::BC1_UNORM_SRGB:
        case Format::BC4_TYPELESS:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
        case Format::BC2_TYPELESS:
        case Format::BC2_UNORM:
        case Format::BC2_UNORM_SRGB:
        case Format::BC3_TYPELESS:
        case Format::BC3_UNORM:
        case Format::BC3_UNORM_SRGB:
        case Format::BC5_TYPELESS:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_TYPELESS:
        case Format::BC6H_UF16:
        case Format::BC6H_SF16:
        case Format::BC7_TYPELESS:
        case Format::BC7_UNORM:
        case Format::BC7_UNORM_SRGB:
            return 0; // not valid

        case Format::AYUV:
        case Format::Y410:
        case Format::Y416:
        case Format::NV12:
        case Format::P010:
        case Format::P016:
        case Format::OPAQUE_420:
        case Format::YUY2:
        case Format::Y210:
        case Format::Y216:
        case Format::NV11:
        case Format::AI44:
        case Format::IA44:
        case Format::P8:
        case Format::A8P8:
        case Format::P208:
        case Format::V208:
        case Format::V408:
            return 0; // who cares?

        case Format::B4G4R4A4_UNORM:
            return 4;
        }
        return 0;
    }
}
