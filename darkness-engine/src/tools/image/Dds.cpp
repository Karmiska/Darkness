#include "tools/image/Dds.h"
#include "engine/graphics/Format.h"
#include <fstream>
#include <algorithm>

using namespace engine;

namespace engine
{
    namespace image
    {

        Dds::Dds(const string& filename)
            : m_filename{ filename }
            , m_slices{ 0 }
            , m_mipmaps{ 0 }
            , m_fileSize{ 0 }
        {
			std::fstream input;
            input.open(filename.c_str(), std::ios::binary | std::ios::in);
            if (input.is_open())
            {
                if (!readHeader(input))
                {
                    printf("Unsupported format.");
                }
                else
                {
                    loadImage(input);
                }
                input.close();
            }
        }

        Format fromDXFormat(DXGIFormat format)
        {
            switch (format)
            {
            case DXGI_FORMAT_UNKNOWN: return Format::UNKNOWN;
            case DXGI_FORMAT_R32G32B32A32_TYPELESS: return Format::R32G32B32A32_TYPELESS;
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return Format::R32G32B32A32_FLOAT;
            case DXGI_FORMAT_R32G32B32A32_UINT: return Format::R32G32B32A32_UINT;
            case DXGI_FORMAT_R32G32B32A32_SINT: return Format::R32G32B32A32_SINT;
            case DXGI_FORMAT_R32G32B32_TYPELESS: return Format::R32G32B32_TYPELESS;
            case DXGI_FORMAT_R32G32B32_FLOAT: return Format::R32G32B32_FLOAT;
            case DXGI_FORMAT_R32G32B32_UINT: return Format::R32G32B32_UINT;
            case DXGI_FORMAT_R32G32B32_SINT: return Format::R32G32B32_SINT;
            case DXGI_FORMAT_R16G16B16A16_TYPELESS: return Format::R16G16B16A16_TYPELESS;
            case DXGI_FORMAT_R16G16B16A16_FLOAT: return Format::R16G16B16A16_FLOAT;
            case DXGI_FORMAT_R16G16B16A16_UNORM: return Format::R16G16B16A16_UNORM;
            case DXGI_FORMAT_R16G16B16A16_UINT: return Format::R16G16B16A16_UINT;
            case DXGI_FORMAT_R16G16B16A16_SNORM: return Format::R16G16B16A16_SNORM;
            case DXGI_FORMAT_R16G16B16A16_SINT: return Format::R16G16B16A16_SINT;
            case DXGI_FORMAT_R32G32_TYPELESS: return Format::R32G32_TYPELESS;
            case DXGI_FORMAT_R32G32_FLOAT: return Format::R32G32_FLOAT;
            case DXGI_FORMAT_R32G32_UINT: return Format::R32G32_UINT;
            case DXGI_FORMAT_R32G32_SINT: return Format::R32G32_SINT;
            case DXGI_FORMAT_R32G8X24_TYPELESS: return Format::R32G8X24_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return Format::D32_FLOAT_S8X24_UINT;
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return Format::R32_FLOAT_X8X24_TYPELESS;
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return Format::X32_TYPELESS_G8X24_UINT;
            case DXGI_FORMAT_R10G10B10A2_TYPELESS: return Format::R10G10B10A2_TYPELESS;
            case DXGI_FORMAT_R10G10B10A2_UNORM: return Format::R10G10B10A2_UNORM;
            case DXGI_FORMAT_R10G10B10A2_UINT: return Format::R10G10B10A2_UINT;
            case DXGI_FORMAT_R11G11B10_FLOAT: return Format::R11G11B10_FLOAT;
            case DXGI_FORMAT_R8G8B8A8_TYPELESS: return Format::R8G8B8A8_TYPELESS;
            case DXGI_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return Format::R8G8B8A8_UNORM_SRGB;
            case DXGI_FORMAT_R8G8B8A8_UINT: return Format::R8G8B8A8_UINT;
            case DXGI_FORMAT_R8G8B8A8_SNORM: return Format::R8G8B8A8_SNORM;
            case DXGI_FORMAT_R8G8B8A8_SINT: return Format::R8G8B8A8_SINT;
            case DXGI_FORMAT_R16G16_TYPELESS: return Format::R16G16_TYPELESS;
            case DXGI_FORMAT_R16G16_FLOAT: return Format::R16G16_FLOAT;
            case DXGI_FORMAT_R16G16_UNORM: return Format::R16G16_UNORM;
            case DXGI_FORMAT_R16G16_UINT: return Format::R16G16_UINT;
            case DXGI_FORMAT_R16G16_SNORM: return Format::R16G16_SNORM;
            case DXGI_FORMAT_R16G16_SINT: return Format::R16G16_SINT;
            case DXGI_FORMAT_R32_TYPELESS: return Format::R32_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT: return Format::D32_FLOAT;
            case DXGI_FORMAT_R32_FLOAT: return Format::R32_FLOAT;
            case DXGI_FORMAT_R32_UINT: return Format::R32_UINT;
            case DXGI_FORMAT_R32_SINT: return Format::R32_SINT;
            case DXGI_FORMAT_R24G8_TYPELESS: return Format::R24G8_TYPELESS;
            case DXGI_FORMAT_D24_UNORM_S8_UINT: return Format::D24_UNORM_S8_UINT;
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return Format::R24_UNORM_X8_TYPELESS;
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return Format::X24_TYPELESS_G8_UINT;
            case DXGI_FORMAT_R8G8_TYPELESS: return Format::R8G8_TYPELESS;
            case DXGI_FORMAT_R8G8_UNORM: return Format::R8G8_UNORM;
            case DXGI_FORMAT_R8G8_UINT: return Format::R8G8_UINT;
            case DXGI_FORMAT_R8G8_SNORM: return Format::R8G8_SNORM;
            case DXGI_FORMAT_R8G8_SINT: return Format::R8G8_SINT;
            case DXGI_FORMAT_R16_TYPELESS: return Format::R16_TYPELESS;
            case DXGI_FORMAT_R16_FLOAT: return Format::R16_FLOAT;
            case DXGI_FORMAT_D16_UNORM: return Format::D16_UNORM;
            case DXGI_FORMAT_R16_UNORM: return Format::R16_UNORM;
            case DXGI_FORMAT_R16_UINT: return Format::R16_UINT;
            case DXGI_FORMAT_R16_SNORM: return Format::R16_SNORM;
            case DXGI_FORMAT_R16_SINT: return Format::R16_SINT;
            case DXGI_FORMAT_R8_TYPELESS: return Format::R8_TYPELESS;
            case DXGI_FORMAT_R8_UNORM: return Format::R8_UNORM;
            case DXGI_FORMAT_R8_UINT: return Format::R8_UINT;
            case DXGI_FORMAT_R8_SNORM: return Format::R8_SNORM;
            case DXGI_FORMAT_R8_SINT: return Format::R8_SINT;
            case DXGI_FORMAT_A8_UNORM: return Format::A8_UNORM;
            case DXGI_FORMAT_R1_UNORM: return Format::R1_UNORM;
            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return Format::R9G9B9E5_SHAREDEXP;
            case DXGI_FORMAT_R8G8_B8G8_UNORM: return Format::R8G8_B8G8_UNORM;
            case DXGI_FORMAT_G8R8_G8B8_UNORM: return Format::G8R8_G8B8_UNORM;
            case DXGI_FORMAT_BC1_TYPELESS: return Format::BC1_TYPELESS;
            case DXGI_FORMAT_BC1_UNORM: return Format::BC1_UNORM;
            case DXGI_FORMAT_BC1_UNORM_SRGB: return Format::BC1_UNORM_SRGB;
            case DXGI_FORMAT_BC2_TYPELESS: return Format::BC2_TYPELESS;
            case DXGI_FORMAT_BC2_UNORM: return Format::BC2_UNORM;
            case DXGI_FORMAT_BC2_UNORM_SRGB: return Format::BC2_UNORM_SRGB;
            case DXGI_FORMAT_BC3_TYPELESS: return Format::BC3_TYPELESS;
            case DXGI_FORMAT_BC3_UNORM: return Format::BC3_UNORM;
            case DXGI_FORMAT_BC3_UNORM_SRGB: return Format::BC3_UNORM_SRGB;
            case DXGI_FORMAT_BC4_TYPELESS: return Format::BC4_TYPELESS;
            case DXGI_FORMAT_BC4_UNORM: return Format::BC4_UNORM;
            case DXGI_FORMAT_BC4_SNORM: return Format::BC4_SNORM;
            case DXGI_FORMAT_BC5_TYPELESS: return Format::BC5_TYPELESS;
            case DXGI_FORMAT_BC5_UNORM: return Format::BC5_UNORM;
            case DXGI_FORMAT_BC5_SNORM: return Format::BC5_SNORM;
            case DXGI_FORMAT_B5G6R5_UNORM: return Format::B5G6R5_UNORM;
            case DXGI_FORMAT_B5G5R5A1_UNORM: return Format::B5G5R5A1_UNORM;
            case DXGI_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
            case DXGI_FORMAT_B8G8R8X8_UNORM: return Format::B8G8R8X8_UNORM;
            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return Format::R10G10B10_XR_BIAS_A2_UNORM;
            case DXGI_FORMAT_B8G8R8A8_TYPELESS: return Format::B8G8R8A8_TYPELESS;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return Format::B8G8R8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8X8_TYPELESS: return Format::B8G8R8X8_TYPELESS;
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return Format::B8G8R8X8_UNORM_SRGB;
            case DXGI_FORMAT_BC6H_TYPELESS: return Format::BC6H_TYPELESS;
            case DXGI_FORMAT_BC6H_UF16: return Format::BC6H_UF16;
            case DXGI_FORMAT_BC6H_SF16: return Format::BC6H_SF16;
            case DXGI_FORMAT_BC7_TYPELESS: return Format::BC7_TYPELESS;
            case DXGI_FORMAT_BC7_UNORM: return Format::BC7_UNORM;
            case DXGI_FORMAT_BC7_UNORM_SRGB: return Format::BC7_UNORM_SRGB;
            case DXGI_FORMAT_AYUV: return Format::AYUV;
            case DXGI_FORMAT_Y410: return Format::Y410;
            case DXGI_FORMAT_Y416: return Format::Y416;
            case DXGI_FORMAT_NV12: return Format::NV12;
            case DXGI_FORMAT_P010: return Format::P010;
            case DXGI_FORMAT_P016: return Format::P016;
            case DXGI_FORMAT_420_OPAQUE: return Format::OPAQUE_420;
            case DXGI_FORMAT_YUY2: return Format::YUY2;
            case DXGI_FORMAT_Y210: return Format::Y210;
            case DXGI_FORMAT_Y216: return Format::Y216;
            case DXGI_FORMAT_NV11: return Format::NV11;
            case DXGI_FORMAT_AI44: return Format::AI44;
            case DXGI_FORMAT_IA44: return Format::IA44;
            case DXGI_FORMAT_P8: return Format::P8;
            case DXGI_FORMAT_A8P8: return Format::A8P8;
            case DXGI_FORMAT_B4G4R4A4_UNORM: return Format::B4G4R4A4_UNORM;
            case DXGI_FORMAT_P208: return Format::P208;
            case DXGI_FORMAT_V208: return Format::V208;
            case DXGI_FORMAT_V408: return Format::V408;
            case DXGI_FORMAT_FORCE_UINT: return Format::R8G8B8A8_UNORM;
            default: return Format::R8G8B8A8_UNORM;
            }
        }

        DXGIFormat dxFormat(Format format)
        {
            switch (format)
            {
            case Format::UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            case Format::R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case Format::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case Format::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
            case Format::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
            case Format::R32G32B32_TYPELESS: return DXGI_FORMAT_R32G32B32_TYPELESS;
            case Format::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
            case Format::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
            case Format::R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
            case Format::R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case Format::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case Format::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
            case Format::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
            case Format::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
            case Format::R32G32_TYPELESS: return DXGI_FORMAT_R32G32_TYPELESS;
            case Format::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
            case Format::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
            case Format::R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
            case Format::R32G8X24_TYPELESS: return DXGI_FORMAT_R32G8X24_TYPELESS;
            case Format::D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case Format::R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            case Format::X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
            case Format::R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
            case Format::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
            case Format::R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
            case Format::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
            case Format::R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case Format::R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case Format::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
            case Format::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
            case Format::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
            case Format::R16G16_TYPELESS: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
            case Format::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
            case Format::R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
            case Format::R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
            case Format::R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
            case Format::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
            case Format::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
            case Format::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
            case Format::R32_UINT: return DXGI_FORMAT_R32_UINT;
            case Format::R32_SINT: return DXGI_FORMAT_R32_SINT;
            case Format::R24G8_TYPELESS: return DXGI_FORMAT_R24G8_TYPELESS;
            case Format::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case Format::R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case Format::X24_TYPELESS_G8_UINT: return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
            case Format::R8G8_TYPELESS: return DXGI_FORMAT_R8G8_TYPELESS;
            case Format::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
            case Format::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
            case Format::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
            case Format::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
            case Format::R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
            case Format::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
            case Format::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
            case Format::R16_UINT: return DXGI_FORMAT_R16_UINT;
            case Format::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
            case Format::R16_SINT: return DXGI_FORMAT_R16_SINT;
            case Format::R8_TYPELESS: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
            case Format::R8_UINT: return DXGI_FORMAT_R8_UINT;
            case Format::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
            case Format::R8_SINT: return DXGI_FORMAT_R8_SINT;
            case Format::A8_UNORM: return DXGI_FORMAT_A8_UNORM;
            case Format::R1_UNORM: return DXGI_FORMAT_R1_UNORM;
            case Format::R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case Format::R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
            case Format::G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
            case Format::BC1_TYPELESS: return DXGI_FORMAT_BC1_TYPELESS;
            case Format::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
            case Format::BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
            case Format::BC2_TYPELESS: return DXGI_FORMAT_BC2_TYPELESS;
            case Format::BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
            case Format::BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
            case Format::BC3_TYPELESS: return DXGI_FORMAT_BC3_TYPELESS;
            case Format::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
            case Format::BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
            case Format::BC4_TYPELESS: return DXGI_FORMAT_BC4_TYPELESS;
            case Format::BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
            case Format::BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
            case Format::BC5_TYPELESS: return DXGI_FORMAT_BC5_TYPELESS;
            case Format::BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
            case Format::BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
            case Format::B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
            case Format::B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
            case Format::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Format::B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
            case Format::R10G10B10_XR_BIAS_A2_UNORM: return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
            case Format::B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
            case Format::B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case Format::B8G8R8X8_TYPELESS: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
            case Format::B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            case Format::BC6H_TYPELESS: return DXGI_FORMAT_BC6H_TYPELESS;
            case Format::BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
            case Format::BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
            case Format::BC7_TYPELESS: return DXGI_FORMAT_BC7_TYPELESS;
            case Format::BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
            case Format::BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
            case Format::AYUV: return DXGI_FORMAT_AYUV;
            case Format::Y410: return DXGI_FORMAT_Y410;
            case Format::Y416: return DXGI_FORMAT_Y416;
            case Format::NV12: return DXGI_FORMAT_NV12;
            case Format::P010: return DXGI_FORMAT_P010;
            case Format::P016: return DXGI_FORMAT_P016;
            case Format::OPAQUE_420: return DXGI_FORMAT_420_OPAQUE;
            case Format::YUY2: return DXGI_FORMAT_YUY2;
            case Format::Y210: return DXGI_FORMAT_Y210;
            case Format::Y216: return DXGI_FORMAT_Y216;
            case Format::NV11: return DXGI_FORMAT_NV11;
            case Format::AI44: return DXGI_FORMAT_AI44;
            case Format::IA44: return DXGI_FORMAT_IA44;
            case Format::P8: return DXGI_FORMAT_P8;
            case Format::A8P8: return DXGI_FORMAT_A8P8;
            case Format::B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;
            case Format::P208: return DXGI_FORMAT_P208;
            case Format::V208: return DXGI_FORMAT_V208;
            case Format::V408: return DXGI_FORMAT_V408;
            default: return DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
                ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

        DXGIFormat dxgiFormatFromDDS(const DdsPixelFormat& ddpf)
        {
            if (ddpf.flags & DDS_RGB)
            {
                switch (ddpf.RGBBitCount)
                {
                case 32:
                    // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB & DXGI_FORMAT_B8G8R8X8_UNORM_SRGB should be
                    // written using the DX10 extended header instead since these formats require
                    // DXGI 1.1
                    //
                    // This code will use the fallback to swizzle BGR to RGB in memory for standard
                    // DDS files which works on 10 and 10.1 devices with WDDM 1.0 drivers
                    //
                    // NOTE: We don't use DXGI_FORMAT_B8G8R8X8_UNORM or DXGI_FORMAT_B8G8R8X8_UNORM
                    // here because they were defined for DXGI 1.0 but were not required for D3D10/10.1

                    if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                        return DXGI_FORMAT_R8G8B8A8_UNORM;

                    if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                        return DXGI_FORMAT_B8G8R8A8_UNORM;

                    // No D3DFMT_X8B8G8R8 in DXGI. We'll deal with it in a swizzle case to ensure
                    // alpha channel is 255 (don't care formats could contain garbage)

                    // Note that many common DDS reader/writers (including D3DX) swap the
                    // the RED/BLUE masks for 10:10:10:2 formats. We assumme
                    // below that the 'backwards' header mask is being used since it is most
                    // likely written by D3DX. The more robust solution is to use the 'DX10'
                    // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                    // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                    if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                        return DXGI_FORMAT_R10G10B10A2_UNORM;

                    if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                        return DXGI_FORMAT_R16G16_UNORM;

                    if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
                        // Only 32-bit color channel format in D3D9 was R32F
                        return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
                    break;

                case 24:
                    // No 24bpp DXGI formats
                    break;

                case 16:
                    // 5:5:5 & 5:6:5 formats are defined for DXGI, but are deprecated for D3D10, 10.0, and 11

                    // No 4bpp, 3:3:2, 3:3:2:8, or paletted DXGI formats
                    break;
                }
            }
            else if (ddpf.flags & DDS_LUMINANCE)
            {
                if (8 == ddpf.RGBBitCount)
                {
                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
                        return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension

                                                     // No 4bpp DXGI formats
                }

                if (16 == ddpf.RGBBitCount)
                {
                    if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
                        return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                        return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
            }
            else if (ddpf.flags & DDS_ALPHA)
            {
                if (8 == ddpf.RGBBitCount)
                {
                    return DXGI_FORMAT_A8_UNORM;
                }
            }
            else if (ddpf.flags & DDS_FOURCC)
            {
                if (MAKEFOURCC('D', 'X', 'T', '1') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC1_UNORM;
                if (MAKEFOURCC('D', 'X', 'T', '3') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC2_UNORM;
                if (MAKEFOURCC('D', 'X', 'T', '5') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC3_UNORM;

                // While pre-mulitplied alpha isn't directly supported by the DXGI formats,
                // they are basically the same as these BC formats so they can be mapped
                if (MAKEFOURCC('D', 'X', 'T', '2') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC2_UNORM;
                if (MAKEFOURCC('D', 'X', 'T', '4') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC3_UNORM;

                if (MAKEFOURCC('A', 'T', 'I', '1') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC4_UNORM;
                if (MAKEFOURCC('B', 'C', '4', 'U') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC4_UNORM;
                if (MAKEFOURCC('B', 'C', '4', 'S') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC4_SNORM;

                if (MAKEFOURCC('A', 'T', 'I', '2') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC5_UNORM;
                if (MAKEFOURCC('B', 'C', '5', 'U') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC5_UNORM;
                if (MAKEFOURCC('B', 'C', '5', 'S') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_BC5_SNORM;

                if (MAKEFOURCC('R', 'G', 'B', 'G') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_R8G8_B8G8_UNORM;
                if (MAKEFOURCC('G', 'R', 'G', 'B') == FOURCCBUF(ddpf.fourCC))
                    return DXGI_FORMAT_G8R8_G8B8_UNORM;

                // Check for D3DFORMAT enums being set here
                switch (FOURCCBUF(ddpf.fourCC))
                {
                case 36: //D3DFMT_A16B16G16R16:
                    return DXGI_FORMAT_R16G16B16A16_UNORM;

                case 110: //D3DFMT_Q16W16V16U16:
                    return DXGI_FORMAT_R16G16B16A16_SNORM;

                case 111: //D3DFMT_R16F:
                    return DXGI_FORMAT_R16_FLOAT;

                case 112: //D3DFMT_G16R16F:
                    return DXGI_FORMAT_R16G16_FLOAT;

                case 113: //D3DFMT_A16B16G16R16F:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case 114: //D3DFMT_R32F:
                    return DXGI_FORMAT_R32_FLOAT;

                case 115: //D3DFMT_G32R32F:
                    return DXGI_FORMAT_R32G32_FLOAT;

                case 116: //D3DFMT_A32B32G32R32F:
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;
                }
            }

            return DXGI_FORMAT_UNKNOWN;
        }

        bool hasDX10Header(const DdsHeader& header)
        {
            return (header.pixelFormat.flags & DDS_FOURCC) &&
                    (FOURCCBUF(header.pixelFormat.fourCC)) == GETFOURCC('D', 'X', '1', '0');
        }

        bool Dds::readHeader(std::fstream& file)
        {
            file.seekg(0, std::ios::end);
            m_fileSize = static_cast<size_t>(file.tellg());
            file.seekg(0, std::ios::beg);

            ASSERT(sizeof(DdsHeader) == 124, "Ivalid DDS Header structure defined");
            ASSERT(m_fileSize > sizeof(DdsHeader) + sizeof(unsigned int), "Too small file for DDS header");

            // read magic
            unsigned int magic = 0;
            file.read(reinterpret_cast<char*>(&magic), sizeof(unsigned int));
            ASSERT(magic == GETFOURCC('D', 'D', 'S', ' '), "Not a DDS file");

            // read dds header
            memset(&m_header, 0, sizeof(m_header));
            file.read(reinterpret_cast<char*>(&m_header), sizeof(DdsHeader));

            // check that it's a format we support
            ASSERT(static_cast<size_t>(file.tellg()) == 128, "Read so far 128 bytes");

            ASSERT(m_header.size == 124,                            "Invalid DDS Header");
            //ASSERT((m_header.flags & DDSD_CAPS) == DDSD_CAPS,        "Invalid DDS Header");
            ASSERT((m_header.flags & DDSD_HEIGHT) == DDSD_HEIGHT,    "Invalid DDS Header");
            ASSERT((m_header.flags & DDSD_WIDTH) == DDSD_WIDTH,        "Invalid DDS Header");
            //ASSERT((m_header.flags & DDSD_PIXELFORMAT) == DDSD_PIXELFORMAT, "Invalid DDS Header");

            //bool hasPitch = (m_header.flags & DDSD_PITCH) == DDSD_PITCH;
            //bool hasMipmaps = (m_header.flags & DDSD_MIPMAPCOUNT) == DDSD_MIPMAPCOUNT;
            //bool hasPitchForCompressedTexture = (m_header.flags & DDSD_LINEARSIZE) == DDSD_LINEARSIZE;
            //bool hasDepth = (m_header.flags & DDSD_DEPTH) == DDSD_DEPTH;

            m_slices = 1;

            if(hasDX10Header(m_header))
            {
                memset(&m_extendedHeader, 0, sizeof(m_extendedHeader));
                file.read(reinterpret_cast<char*>(&m_extendedHeader), sizeof(DdsHeaderDXT10));
                if((m_extendedHeader.miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE) == DDS_RESOURCE_MISC_TEXTURECUBE)
                    m_slices = m_extendedHeader.arraySize;

                m_format = fromDXFormat(dxgiFormatFromDDS(m_header.pixelFormat));
                if (m_format == Format::UNKNOWN)
                {
                    m_format = fromDXFormat(m_extendedHeader.dxgiFormat);
                }

                ASSERT(static_cast<size_t>(file.tellg()) == 148, "Read so far 148 bytes");
            }
            else
            {
                m_format = fromDXFormat(dxgiFormatFromDDS(m_header.pixelFormat));
            }

            m_mipmaps = m_header.mipMapCount;
            return m_format != Format::UNKNOWN;
        }

#if 1
        void Dds::loadImage(std::fstream& file)
        {
            reserve();
            file.read(reinterpret_cast<char*>(m_data.data()), static_cast<std::streamsize>(m_data.size()));
        }

        void Dds::reserve()
        {
            auto size = imageBytes(
                m_format,
                m_header.width, m_header.height,
                static_cast<unsigned int>(m_slices), static_cast<unsigned int>(m_mipmaps));
            m_data.resize(size);
        }

        size_t Dds::mipCount() const
        {
            return m_mipmaps;
        }

        size_t Dds::arraySlices() const
        {
            return m_slices;
        }

        size_t Dds::slicePitch() const
        {
            size_t bytes = 0;
            auto width = m_header.width;
            auto height = m_header.height;

            for (size_t mip = 0; mip < m_mipmaps; ++mip)
            {
                auto mipSurfaceInfo = surfaceInformation(
                    m_format,
                    width,
                    height);

                bytes += mipSurfaceInfo.numBytes;

                width /= 2;
                height /= 2;
                if (width < 1)
                    width = 1;
                if (height < 1)
                    height = 1;
            }
            return bytes;
        }

        ImageSubresource Dds::map(
            size_t mipLevel,
            size_t arraySlice) const
        {
            auto surfaceInfo = surfaceInformation(
                m_format,
				std::max<unsigned int>(m_header.width >> mipLevel, 1u),
				std::max<unsigned int>(m_header.height >> mipLevel, 1u));

            ImageSubresource res;
            res.width = std::max<unsigned int>(m_header.width >> mipLevel, 1u);
            res.height = std::max<unsigned int>(m_header.height >> mipLevel, 1u);
            res.slicePitch = slicePitch();
            res.sizeBytes = surfaceInfo.numBytes;
            res.pitch = surfaceInfo.rowBytes;

            size_t bytes = arraySlice * slicePitch();
            auto width = m_header.width;
            auto height = m_header.height;
            for (size_t mip = 0; mip < mipLevel; ++mip)
            {
                auto mipSurfaceInfo = surfaceInformation(
                    m_format,
                    width,
                    height);

                bytes += mipSurfaceInfo.numBytes;

                width /= 2;
                height /= 2;
                if (width < 1)
                    width = 1;
                if (height < 1)
                    height = 1;
            }

            res.data = m_data.data() + bytes;
            return res;
        }

        /*void Dds::loadImage(std::fstream& file)
        {
            // allocate buffer for dxt data
            unsigned char* dxtBuffer = new unsigned char[m_header.pitchOrLinearSize];
            file.read((char*)dxtBuffer, m_header.pitchOrLinearSize);

            // grow our pixel storage and take pointer there
            mPixels.resize(m_header.width * m_header.height);
            engine::Color4f* arrayData = mPixels.data();

            // loop through the input data in 4x4 chunks
            unsigned char* bufPtr = dxtBuffer;
            for(int y = (int)m_header.height - 4; y >= 0; y -= 4)
            {
                for(int x = 0; x < (int)m_header.width; x += 4)
                {
                    engine::Color4f colors[4];

                    // get two 16bit rgb 565 reference colors (0 and 1)
                    colors[0] = Color::from565rgb(GETUSHORT(bufPtr)); bufPtr += 2;
                    colors[1] = Color::from565rgb(GETUSHORT(bufPtr)); bufPtr += 2;

                    // calculate intermediate colors (2 and 3) by interpolating between 0 and 1.
                    calculateIntermediateColors(colors[0], colors[1], colors[2], colors[3]);

                    // the bufPtr at this point will have 16 values
                    // each are 2bit indices to the 4 colors we have
                    int line = 0;
                    for(int i = 0; i < 16; i += 4)
                    {
                        // the data is vertically reversed
                        // so we will turn it the right way.
                        engine::Color4f* dstPtr = arrayData + (((y + 3 - line) * m_header.width) + x);
                        *dstPtr = colors[((*bufPtr) & 0x3)]; ++dstPtr;
                        *dstPtr = colors[((*bufPtr) & 0xC) >> 2]; ++dstPtr;
                        *dstPtr = colors[((*bufPtr) & 0x30) >> 4];  ++dstPtr;
                        *dstPtr = colors[((*bufPtr) & 0xC0) >> 6];
                        ++line;
                        ++bufPtr;
                    }
                }
            }

            // free the dxt buffer
            delete[] dxtBuffer;
        }*/

        /*void Dds::calculateIntermediateColors(
            const engine::Color4f& c0,
            const engine::Color4f& c1,
            engine::Color4f& c2,
            engine::Color4f& c3)
        {
            // we calculate the colors by using linear interpolation
            // suggested at microsofts documentation (https://msdn.microsoft.com/en-us/library/bb694531(v=VS.85).aspx)
            if(c0 > c1)
            {
                c2 = engine::Color4f((unsigned char)(((2.0 / 3.0)*(double)c0.red()) + ((1.0 / 3.0)*(double)c1.red())),
                           (unsigned char)(((2.0 / 3.0)*(double)c0.green()) + ((1.0 / 3.0)*(double)c1.green())),
                           (unsigned char)(((2.0 / 3.0)*(double)c0.blue()) + ((1.0 / 3.0)*(double)c1.blue())));

                c3 = engine::Color4f((unsigned char)(((1.0 / 3.0)*(double)c0.red()) + ((2.0 / 3.0)*(double)c1.red())),
                           (unsigned char)(((1.0 / 3.0)*(double)c0.green()) + ((2.0 / 3.0)*(double)c1.green())),
                           (unsigned char)(((1.0 / 3.0)*(double)c0.blue()) + ((2.0 / 3.0)*(double)c1.blue())));
            }
            else
            {
                // 1bit alpha
                c2 = engine::Color4f((unsigned char)(((1.0 / 2.0)*(double)c0.red()) + ((1.0 / 2.0)*(double)c1.red())),
                           (unsigned char)(((1.0 / 2.0)*(double)c0.green()) + ((1.0 / 2.0)*(double)c1.green())),
                           (unsigned char)(((1.0 / 2.0)*(double)c0.blue()) + ((1.0 / 2.0)*(double)c1.blue())));
                c3 = engine::Color4f(255, 255, 255, 255);
            }
        }*/

        Dds::Dds(const string& filename,
            Format type,
            unsigned int width,
            unsigned int height,
            unsigned int slices,
            unsigned int mips)
            : m_filename{ filename }
            , m_format{ type }
            , m_slices{ slices }
            , m_mipmaps{ mips }
        {
            createHeader(width, height, type);
        }

        Format Dds::format() const
        {
            return m_format;
        }
#endif
        bool dx9format(Format format)
        {
            return 
                (format != Format::BC4_TYPELESS) &&
                (format != Format::BC4_UNORM) &&
                (format != Format::BC4_SNORM) &&
                (format != Format::BC5_TYPELESS) &&
                (format != Format::BC5_UNORM) &&
                (format != Format::BC5_SNORM) &&
                (format != Format::BC6H_TYPELESS) &&
                (format != Format::BC6H_UF16) &&
                (format != Format::BC6H_SF16) &&
                (format != Format::BC7_TYPELESS) &&
                (format != Format::BC7_UNORM) &&
                (format != Format::BC7_UNORM_SRGB);
        }

        void Dds::createHeader(unsigned int width, unsigned int height, Format format)
        {
            // fill the DdsHeader structure
            memset(&m_header, 0, sizeof(DdsHeader));
            m_header.size = sizeof(DdsHeader);
            m_header.width = width;
            m_header.height = height;
            m_header.mipMapCount = static_cast<unsigned int>(m_mipmaps);
            m_header.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
            m_header.flags |= isBlockCompressedFormat(format) ? DDSD_LINEARSIZE : 0;
            m_header.flags |= !isBlockCompressedFormat(format) ? DDSD_PITCH : 0;
            m_header.pitchOrLinearSize = static_cast<unsigned int>(formatBytes(format, width, height));

            if (!dx9format(format) || m_slices != 1)
            {
                m_header.pixelFormat.size = sizeof(DdsPixelFormat);
                m_header.pixelFormat.flags = DDS_FOURCC;
                m_header.pixelFormat.fourCC[0] = 'D';
                m_header.pixelFormat.fourCC[1] = 'X';
                m_header.pixelFormat.fourCC[2] = '1';
                m_header.pixelFormat.fourCC[3] = '0';

                m_extendedHeader.dxgiFormat = dxFormat(format);
                m_extendedHeader.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
                m_extendedHeader.miscFlag = 0;
                m_extendedHeader.arraySize = static_cast<unsigned int>(m_slices);
                m_extendedHeader.miscFlags2 = 0;
                if (m_slices == 6)
                {
                    m_extendedHeader.miscFlag |= DDS_RESOURCE_MISC_TEXTURECUBE;
                }
            }
            else
            {
                m_header.pixelFormat.size = sizeof(DdsPixelFormat);
                m_header.pixelFormat.flags = isBlockCompressedFormat(format) ? DDS_FOURCC : DDS_RGB;

                if (format == Format::BC1_TYPELESS ||
                    format == Format::BC1_UNORM ||
                    format == Format::BC1_UNORM_SRGB)
                {
                    m_header.pixelFormat.fourCC[0] = 'D';
                    m_header.pixelFormat.fourCC[1] = 'X';
                    m_header.pixelFormat.fourCC[2] = 'T';
                    m_header.pixelFormat.fourCC[3] = '1';
                }
                else if (
                    format == Format::BC3_TYPELESS ||
                    format == Format::BC3_UNORM ||
                    format == Format::BC3_UNORM_SRGB)
                {
                    m_header.pixelFormat.fourCC[0] = 'D';
                    m_header.pixelFormat.fourCC[1] = 'X';
                    m_header.pixelFormat.fourCC[2] = 'T';
                    m_header.pixelFormat.fourCC[3] = '5';
                }

                if ((m_header.pixelFormat.flags & DDS_RGB) == DDS_RGB)
                {
                    m_header.pixelFormat.RGBBitCount = static_cast<unsigned int>(formatBits(format));
                    m_header.pixelFormat.RBitMask = static_cast<unsigned int>(formatMaskRed(format));
                    m_header.pixelFormat.GBitMask = static_cast<unsigned int>(formatMaskGreen(format));
                    m_header.pixelFormat.BBitMask = static_cast<unsigned int>(formatMaskBlue(format));
                    m_header.pixelFormat.ABitMask = static_cast<unsigned int>(formatMaskAlpha(format));
                }
            }
        }
#if 1
        size_t Dds::width() const
        {
            return m_header.width;
        }

        size_t Dds::height() const
        {
            return m_header.height;
        }

        const uint8_t* Dds::data() const
        {
            return m_data.data();
        }

        size_t Dds::bytes() const
        {
            return m_data.size();
        }
#endif
        void Dds::save(const char* data, size_t bytes)
        {
            m_header.pitchOrLinearSize = static_cast<unsigned int>(bytes);

            std::fstream input;
            input.open(m_filename.c_str(), std::ios::binary | std::ios::out);
            if (input.is_open())
            {
                // the header has already been prepared in the constructors
                unsigned int magic = GETFOURCC('D', 'D', 'S', ' ');
                input.write(reinterpret_cast<char*>(&magic), sizeof(unsigned int));
                input.write(reinterpret_cast<char*>(&m_header), sizeof(DdsHeader));

                if ((m_header.pixelFormat.fourCC[0] == 'D') &&
                    (m_header.pixelFormat.fourCC[1] == 'X') &&
                    (m_header.pixelFormat.fourCC[2] == '1') &&
                    (m_header.pixelFormat.fourCC[3] == '0'))
                {
                    input.write(reinterpret_cast<char*>(&m_extendedHeader), sizeof(DdsHeaderDXT10));
                }
                input.write(data, static_cast<std::streamsize>(bytes));
                input.close();
            }
        }

        void Dds::save()
        {
            m_header.pitchOrLinearSize = static_cast<unsigned int>(m_data.size());

            std::fstream input;
            input.open(m_filename.c_str(), std::ios::binary | std::ios::out);
            if (input.is_open())
            {
                // the header has already been prepared in the constructors
                unsigned int magic = GETFOURCC('D', 'D', 'S', ' ');
                input.write(reinterpret_cast<char*>(&magic), sizeof(unsigned int));
                input.write(reinterpret_cast<char*>(&m_header), sizeof(DdsHeader));
                if ((m_header.pixelFormat.fourCC[0] == 'D') &&
                    (m_header.pixelFormat.fourCC[1] == 'X') &&
                    (m_header.pixelFormat.fourCC[2] == '1') &&
                    (m_header.pixelFormat.fourCC[3] == '0'))
                {
                    input.write(reinterpret_cast<char*>(&m_extendedHeader), sizeof(DdsHeaderDXT10));
                }
                input.write(reinterpret_cast<char*>(m_data.data()), static_cast<std::streamsize>(m_data.size()));
                input.close();
            }
        }

        void Dds::flipVertical()
        {
            ASSERT(false, "DDS codec does not support flipping currently");
        }

        void Dds::convert()
        {
            ASSERT(false, "DDS codec does not support conversion currently");
        }

        /*void Dds::writePixels(std::fstream& file)
        {
            // BC1 compression uses 2x 2bytes rgb565 data + 16 2bit indices = 8 bytes;
            unsigned char* dst = new unsigned char[m_header.pitchOrLinearSize];

            // loop through pixels in vertically reverse order in blocks of 4x4 pixels
            unsigned char* dstRun = dst;
            for(int y = (int)m_header.height - 4; y >= 0; y -= 4)
            {
                for(int x = 0; x < (int)m_header.width; x += 4)
                {
                    // get 4x4 block of pixels
                    engine::vector<engine::Color4f>&& block = getBlock(x, y);

                    // get min and max reference colors
                    engine::Color4f colors[4];
                    getMinMax(block, colors[0], colors[1]);

                    // calculate intermediate colors (2 and 3) by interpolating between 0 and 1.
                    calculateIntermediateColors(colors[0], colors[1], colors[2], colors[3]);

                    // write the reference colors to buffer
                    PUTUSHORT(dstRun, colors[0].get565bgr()); dstRun += 2;
                    PUTUSHORT(dstRun, colors[1].get565bgr()); dstRun += 2;

                    // compute indices by checking what indice is closest to the given color
                    int currentByte = 0;
                    for(int i = 16 - 4; i >= 0; i -= 4)
                    {
                        unsigned char i1 = getColorTableIndex(colors, block[i]);
                        unsigned char i2 = getColorTableIndex(colors, block[i + 1]);
                        unsigned char i3 = getColorTableIndex(colors, block[i + 2]);
                        unsigned char i4 = getColorTableIndex(colors, block[i + 3]);

                        // write indices to buffer
                        dstRun[currentByte] = i4 << 6 | i3 << 4 | i2 << 2 | i1;

                        ++currentByte;
                    }

                    dstRun += 4;
                }
            }

            // write buffer to file
            file.write((const char*)dst, m_header.pitchOrLinearSize);
            delete[] dst;
        }*/

        // conveniency function to get 16 pixels (4x4) from given location
        /*engine::vector<engine::Color4f> Dds::getBlock(int xp, int yp)
        {
            engine::vector<engine::Color4f> colorBlock;
            for(int y = yp; y < yp + 4; ++y)
            {
                for(int x = xp; x < xp + 4; ++x)
                {
                    colorBlock.push_back(mPixels[(m_header.width * y) + x]);
                }
            }
            return colorBlock;
        }

        // calculates the smallest and biggest color values
        void Dds::getMinMax(const engine::vector<engine::Color4f>& block, engine::Color4f& minColor, engine::Color4f& maxColor)
        {
            minColor = engine::Color4f(255, 255, 255, 255);
            maxColor = engine::Color4f(0, 0, 0, 0);

            for(auto col : block)
            {
                if(col.red() < minColor.red()) { minColor.setRed(col.red()); }
                if(col.green() < minColor.green()) { minColor.setGreen(col.green()); }
                if(col.blue() < minColor.blue()) { minColor.setBlue(col.blue()); }
                if(col.alpha() < minColor.alpha()) { minColor.setAlpha(col.alpha()); }

                if(col.red() > maxColor.red()) { maxColor.setRed(col.red()); }
                if(col.green() > maxColor.green()) { maxColor.setGreen(col.green()); }
                if(col.blue() > maxColor.blue()) { maxColor.setBlue(col.blue()); }
                if(col.alpha() > maxColor.alpha()) { maxColor.setAlpha(col.alpha()); }
            }
        }

        // calculate the distance of the color from each reference color
        // and return the indice of the closest match
        unsigned char Dds::getColorTableIndex(
            const engine::Color4f(&colors)[4],
            const engine::Color4f& cmpColor)
        {
            unsigned int distanceToC0 = calculateColorDistance(colors[0], cmpColor);
            unsigned int distanceToC1 = calculateColorDistance(colors[1], cmpColor);
            unsigned int distanceToC2 = calculateColorDistance(colors[2], cmpColor);
            unsigned int distanceToC3 = calculateColorDistance(colors[3], cmpColor);

            unsigned int minValue = numeric_limits<unsigned int>::max();
            minValue = minValue > distanceToC0 ? distanceToC0 : minValue;
            minValue = minValue > distanceToC1 ? distanceToC1 : minValue;
            minValue = minValue > distanceToC2 ? distanceToC2 : minValue;
            minValue = minValue > distanceToC3 ? distanceToC3 : minValue;

            if(distanceToC0 == minValue) { return 0; }
            else if(distanceToC1 == minValue) { return 1; }
            else if(distanceToC2 == minValue) { return 2; }
            else { return 3; }
        }

        // calculates color distance from another one based on luminance
        unsigned int Dds::calculateColorDistance(const engine::Color4f& a, const engine::Color4f& b)
        {
            return (unsigned int)(((((double)a.red() - (double)b.red()) * .299) * (((double)a.red() - (double)b.red()) * .299)) +
                                  ((((double)a.green() - (double)b.green()) * .587) * (((double)a.green() - (double)b.green()) * .587)) +
                                  ((((double)a.blue() - (double)b.blue()) * .114) * (((double)a.blue() - (double)b.blue()) * .114)));
        }
        */

    }
}
