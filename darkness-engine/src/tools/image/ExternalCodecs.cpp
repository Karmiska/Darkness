#include "tools/image/ExternalCodecs.h"
#include "tools/image/Color.h"

#include <fstream>
#include <stdio.h>
#include <math.h>

#define  FREEIMAGE_LIB
#include "FreeImage.h"
#include "Utilities.h"

using namespace engine;
using namespace bmp;

#define HeaderFieldValue(sig) (((int)sig[0]) | ((int)sig[1] << 8))

void freeImageOutputForExternalCodec(FREE_IMAGE_FORMAT /*fif*/, const char* msg)
{
	LOG("%s", msg);
}

namespace engine
{
	namespace image
	{
		Format engineFormat(FREE_IMAGE_TYPE type, unsigned int channels, unsigned int bytesPerPixel)
		{
			switch (type)
			{
				case FIT_UNKNOWN: ASSERT(false, "Unknown image type!");
				case FIT_BITMAP:
				{
					switch(channels)
					{
						case 1:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return Format::R8_UINT;
								}
								case 16:
								{
									return Format::R16_FLOAT;
								}
								case 24:
								{
									return Format::R32_FLOAT;
								}
								case 32:
								{
									return Format::R32_FLOAT;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 2:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return Format::R8G8_UINT;
								}
								case 16:
								{
									return Format::R16G16_FLOAT;
								}
								case 24:
								{
									return Format::R32G32_FLOAT;
								}
								case 32:
								{
									return Format::R32G32_FLOAT;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 3:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return Format::R8G8B8A8_UINT;
								}
								case 16:
								{
									return Format::R16G16B16A16_FLOAT;
								}
								case 24:
								{
									return Format::R32G32B32_FLOAT;
								}
								case 32:
								{
									return Format::R32G32B32_FLOAT;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 4:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return Format::R8G8B8A8_UINT;
								}
								case 16:
								{
									return Format::R16G16B16A16_FLOAT;
								}
								case 24:
								{
									return Format::R32G32B32A32_FLOAT;
								}
								case 32:
								{
									return Format::R32G32B32A32_FLOAT;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 256:
						{
							// I think this is actually a single channel?
							switch (bytesPerPixel)
							{
								case 8:
								{
									return Format::R8_UINT;
								}
								case 16:
								{
									return Format::R16_FLOAT;
								}
								case 24:
								{
									return Format::R32_FLOAT;
								}
								case 32:
								{
									return Format::R32_FLOAT;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						default:
						{
							ASSERT(false, "Unknown channel count");
						}
					}
					break;
				}
				case FIT_UINT16: return Format::R16_UINT;
				case FIT_INT16: return Format::R16_UINT;
				case FIT_UINT32: return Format::R32_UINT;
				case FIT_INT32: return Format::R32_UINT;
				case FIT_FLOAT: return Format::R32_FLOAT;
				case FIT_DOUBLE: return Format::R32_FLOAT;
				case FIT_COMPLEX: return Format::UNKNOWN;
				case FIT_RGB16: return Format::R16G16B16A16_UINT;
				case FIT_RGBA16: return Format::R16G16B16A16_UINT;
				case FIT_RGBF: return Format::R32G32B32_FLOAT;
				case FIT_RGBAF: return Format::R32G32B32A32_FLOAT;
				default: return Format::UNKNOWN;
			}
			return Format::UNKNOWN;
		}

		bool isIntegerFormat(FREE_IMAGE_TYPE type, unsigned int channels, unsigned int bytesPerPixel)
		{
			switch (type)
			{
				case FIT_UNKNOWN: ASSERT(false, "Unknown image type!");
				case FIT_BITMAP:
				{
					switch(channels)
					{
						case 1:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return true;
								}
								case 16:
								{
									return false;
								}
								case 24:
								{
									return false;
								}
								case 32:
								{
									return false;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 2:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return true;
								}
								case 16:
								{
									return false;
								}
								case 24:
								{
									return false;
								}
								case 32:
								{
									return false;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 3:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return true;
								}
								case 16:
								{
									return false;
								}
								case 24:
								{
									return false;
								}
								case 32:
								{
									return false;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 4:
						{
							switch (bytesPerPixel)
							{
								case 8:
								{
									return true;
								}
								case 16:
								{
									return false;
								}
								case 24:
								{
									return false;
								}
								case 32:
								{
									return false;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						case 256:
						{
							// most likely single channel
							switch (bytesPerPixel)
							{
								case 8:
								{
									return true;
								}
								case 16:
								{
									return false;
								}
								case 24:
								{
									return false;
								}
								case 32:
								{
									return false;
								}
								default:
								{
									ASSERT(false, "Unknown bytes per pixel");
								}
							}
						}
						default:
						{
							ASSERT(false, "Unknown channel count");
						}
					}
					break;
				}
				case FIT_UINT16: return true;
				case FIT_INT16: return true;
				case FIT_UINT32: return true;
				case FIT_INT32: return true;
				case FIT_FLOAT: return false;
				case FIT_DOUBLE: return false;
				case FIT_COMPLEX: { ASSERT(false, "Unknown format"); break; }
				case FIT_RGB16: return true;
				case FIT_RGBA16: return true;
				case FIT_RGBF: return false;
				case FIT_RGBAF: return false;
				default: { ASSERT(false, "Unknown format"); break; }
			}
			return false;
		}


		ExternalCodecs::ExternalCodecs(const engine::string& filename)
			: mFilename(filename)
			, m_width{ 0 }
			, m_height{ 0 }
			, m_format{ Format::R8G8B8A8_UINT }
			, m_mips{ 0 }
			, m_slices{ 0 }
			, m_channels{ 0u }
			, m_integerFormat{ true }
		{
			FreeImage_Initialise();
			FreeImage_SetOutputMessage(freeImageOutputForExternalCodec);

			std::ifstream input;
			input.open(filename.c_str(), std::ios::binary | std::ios::in);
			if (input.is_open())
			{
				input.seekg(0, std::ios::end);
				size_t size = static_cast<size_t>(input.tellg());
				input.seekg(0, std::ios::beg);
				engine::vector<char> data(size);
				input.read(&data[0], size);
				input.close();

				FIMEMORY* imageMemory = FreeImage_OpenMemory(reinterpret_cast<BYTE*>(data.data()), static_cast<DWORD>(data.size()));
				FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeFromMemory(imageMemory);
				FIBITMAP* bitmap = FreeImage_LoadFromMemory(imageFormat, imageMemory, 0);
				auto imageType = FreeImage_GetImageType(bitmap);
				unsigned int bpp = FreeImage_GetBPP(bitmap);
				m_channels = FreeImage_GetColorsUsed(bitmap);
				if (m_channels == 256)
					m_channels = 1;
				if (m_channels == 0)
				{
					switch (imageType)
					{
						case FIT_UINT16: m_channels = 1; break;
						case FIT_INT16: m_channels = 1; break;
						case FIT_UINT32: m_channels = 1; break;
						case FIT_INT32: m_channels = 1; break;
						case FIT_FLOAT: m_channels = 1; break;
						case FIT_DOUBLE: m_channels = 1; break;
						case FIT_COMPLEX: m_channels = 1; break;
						case FIT_RGB16: m_channels = 3; break;
						case FIT_RGBA16: m_channels = 4; break;
						case FIT_RGBF: m_channels = 3; break;
						case FIT_RGBAF: m_channels = 4; break;
						case FIT_BITMAP: m_channels = 4; break;
						default: ASSERT(false, "Unknown channel count");
					}
				}

				m_width = static_cast<size_t>(FreeImage_GetWidth(bitmap));
				m_height = static_cast<size_t>(FreeImage_GetHeight(bitmap));
				m_mips = 1;
				m_slices = 1;

				
				m_format = engineFormat(imageType, m_channels, bpp);
				m_integerFormat = isIntegerFormat(imageType, m_channels, bpp);

				if ((m_channels == 1) || (m_channels == 2))
				{
					FIBITMAP* convertedImage = FreeImage_ConvertToRGBF(bitmap);
					auto imageDataPtr = FreeImage_GetBits(convertedImage);
					auto stride = FreeImage_GetPitch(convertedImage);

					allocateMemoryForImage();
					convertData(reinterpret_cast<const char*>(imageDataPtr), stride);
				}
				else if (m_channels == 3)
				{
#if defined(GRAPHICS_API_VULKAN)
					m_channels = 4;
					m_format = engineFormat(FIT_RGBAF, m_channels, bpp);

					FIBITMAP* convertedImage = FreeImage_ConvertToRGBAF(bitmap);
					auto imageDataPtr = FreeImage_GetBits(convertedImage);
					auto stride = FreeImage_GetPitch(convertedImage);

					allocateMemoryForImage();
					convertData(reinterpret_cast<const char*>(imageDataPtr), stride);
#else
					FIBITMAP* convertedImage = FreeImage_ConvertToRGBF(bitmap);
					auto imageDataPtr = FreeImage_GetBits(convertedImage);
					auto stride = FreeImage_GetPitch(convertedImage);
					
					allocateMemoryForImage();
					convertData(reinterpret_cast<const char*>(imageDataPtr), stride);
#endif
				}
				else if (m_channels == 4)
				{
					FIBITMAP* convertedImage = FreeImage_ConvertToRGBAF(bitmap);
					auto imageDataPtr = FreeImage_GetBits(convertedImage);
					auto stride = FreeImage_GetPitch(convertedImage);

					allocateMemoryForImage();
					convertData(reinterpret_cast<const char*>(imageDataPtr), stride);
				}

				FreeImage_Unload(bitmap);
				FreeImage_CloseMemory(imageMemory);
			}
			else
			{
				LOG_ERROR("Could not open file: %s", filename.c_str());
			}
		}

		ExternalCodecs::~ExternalCodecs()
		{
			FreeImage_DeInitialise();
		}

		void ExternalCodecs::allocateMemoryForImage()
		{
			auto fbytes = formatBytes(m_format);
			m_data.resize(fbytes * m_width * m_height);
		}

		void ExternalCodecs::convertData(const char* data, unsigned int stride)
		{
			auto srcptr = data;
			if (m_integerFormat)
			{
				if (m_channels == 1)
				{
					switch (m_format)
					{
					case Format::R8_UINT:
					{
						auto dstptr = reinterpret_cast<uint8_t*>(m_data.data());
						for (int y = 0; y < m_height; ++y)
						{
							auto srcline = reinterpret_cast<const float*>(srcptr);
							for (int x = 0; x < m_width; ++x)
							{
								// src is always 32 bits floats
								// and in RGBA order
								auto red = *srcline; ++srcline;
								*dstptr = static_cast<uint8_t>(red * 255.0f); ++dstptr;
							}
							srcptr += stride;
						}
						break;
					}
					case Format::R16_UINT:
					{
						auto dstptr = reinterpret_cast<uint16_t*>(m_data.data());
						for (int y = 0; y < m_height; ++y)
						{
							auto srcline = reinterpret_cast<const float*>(srcptr);
							for (int x = 0; x < m_width; ++x)
							{
								// src is always 32 bits floats
								// and in RGBA order
								auto red = *srcline; ++srcline;
								*dstptr = static_cast<uint16_t>(red * 65535.0f); ++dstptr;
							}
							srcptr += stride;
						}
						break;
					}
					case Format::R32_UINT:
					{
						auto dstptr = reinterpret_cast<uint32_t*>(m_data.data());
						for (int y = 0; y < m_height; ++y)
						{
							auto srcline = reinterpret_cast<const float*>(srcptr);
							for (int x = 0; x < m_width; ++x)
							{
								// src is always 32 bits floats
								// and in RGBA order
								auto red = *srcline; ++srcline;
								*dstptr = static_cast<uint32_t>(red * 4294967295.0f); ++dstptr;
							}
							srcptr += stride;
						}
						break;
					}
					default:
					{
						ASSERT(false, "Unhandled destination format");
					}
					}
				}
				else if (m_channels == 2)
				{
					switch (m_format)
					{
						case Format::R8G8_UINT:
						{
							auto dstptr = reinterpret_cast<uint8_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									*dstptr = static_cast<uint8_t>(red * 255.0f); ++dstptr;
									*dstptr = static_cast<uint8_t>(green * 255.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						case Format::R16G16_UINT:
						{
							auto dstptr = reinterpret_cast<uint16_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									*dstptr = static_cast<uint16_t>(red * 65535.0f); ++dstptr;
									*dstptr = static_cast<uint16_t>(green * 65535.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						case Format::R32G32_UINT:
						{
							auto dstptr = reinterpret_cast<uint32_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									*dstptr = static_cast<uint32_t>(red * 4294967295.0f); ++dstptr;
									*dstptr = static_cast<uint32_t>(green * 4294967295.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
						}
					}
				}
				else if (m_channels == 3)
				{
					switch (m_format)
					{
						case Format::R8G8B8A8_UINT:
						{
							auto dstptr = reinterpret_cast<uint8_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									auto blue = *srcline; ++srcline;
									*dstptr = static_cast<uint8_t>(red * 255.0f); ++dstptr;
									*dstptr = static_cast<uint8_t>(green * 255.0f); ++dstptr;
									*dstptr = static_cast<uint8_t>(blue * 255.0f); ++dstptr;
									*dstptr = 0u; ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						case Format::R16G16B16A16_UINT:
						{
							auto dstptr = reinterpret_cast<uint16_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									auto blue = *srcline; ++srcline;
									*dstptr = static_cast<uint16_t>(red * 65535.0f); ++dstptr;
									*dstptr = static_cast<uint16_t>(green * 65535.0f); ++dstptr;
									*dstptr = static_cast<uint16_t>(blue * 65535.0f); ++dstptr;
									*dstptr = 0u; ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						case Format::R32G32B32_UINT:
						{
							auto dstptr = reinterpret_cast<uint32_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									auto blue = *srcline; ++srcline;
									*dstptr = static_cast<uint32_t>(red * 4294967295.0f); ++dstptr;
									*dstptr = static_cast<uint32_t>(green * 4294967295.0f); ++dstptr;
									*dstptr = static_cast<uint32_t>(blue * 4294967295.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
						}
					}
				}
				else if (m_channels == 4)
				{
					switch (m_format)
					{
						case Format::R8G8B8A8_UINT:
						{
							auto dstptr = reinterpret_cast<uint8_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									auto blue = *srcline; ++srcline;
									auto alpha = *srcline; ++srcline;
									*dstptr = static_cast<uint8_t>(red * 255.0f); ++dstptr;
									*dstptr = static_cast<uint8_t>(green * 255.0f); ++dstptr;
									*dstptr = static_cast<uint8_t>(blue * 255.0f); ++dstptr;
									*dstptr = static_cast<uint8_t>(alpha * 255.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						case Format::R16G16B16A16_UINT:
						{
							auto dstptr = reinterpret_cast<uint16_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									auto blue = *srcline; ++srcline;
									auto alpha = *srcline; ++srcline;
									*dstptr = static_cast<uint16_t>(red * 65535.0f); ++dstptr;
									*dstptr = static_cast<uint16_t>(green * 65535.0f); ++dstptr;
									*dstptr = static_cast<uint16_t>(blue * 65535.0f); ++dstptr;
									*dstptr = static_cast<uint16_t>(alpha * 65535.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						case Format::R32G32B32A32_UINT:
						{
							auto dstptr = reinterpret_cast<uint32_t*>(m_data.data());
							for (int y = 0; y < m_height; ++y)
							{
								auto srcline = reinterpret_cast<const float*>(srcptr);
								for (int x = 0; x < m_width; ++x)
								{
									// src is always 32 bits floats
									// and in RGBA order
									auto red = *srcline; ++srcline;
									auto green = *srcline; ++srcline;
									auto blue = *srcline; ++srcline;
									auto alpha = *srcline; ++srcline;
									*dstptr = static_cast<uint32_t>(red * 4294967295.0f); ++dstptr;
									*dstptr = static_cast<uint32_t>(green * 4294967295.0f); ++dstptr;
									*dstptr = static_cast<uint32_t>(blue * 4294967295.0f); ++dstptr;
									*dstptr = static_cast<uint32_t>(alpha * 4294967295.0f); ++dstptr;
								}
								srcptr += stride;
							}
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
						}
					}
				}
			}
			else
			{
				auto copyChannels = [&](int channelCount)
				{
					auto dstptr = reinterpret_cast<float*>(m_data.data());
					auto dststride = m_width * channelCount;
					auto dstbytes = dststride * sizeof(float);

					const bool flipVertical = true;

					if (!flipVertical)
					{
						for (int y = 0; y < m_height; ++y)
						{
							auto srcline = reinterpret_cast<const float*>(srcptr);
							memcpy(dstptr, srcline, dstbytes);
							dstptr += dststride;
							srcptr += stride;
						}
					}
					else
					{
						dstptr += (m_height - 1) * dststride;
						for (int y = 0; y < m_height; ++y)
						{
							auto srcline = reinterpret_cast<const float*>(srcptr);
							memcpy(dstptr, srcline, dstbytes);
							dstptr -= dststride;
							srcptr += stride;
						}
					}
				};

				if (m_channels == 1)
				{
					switch (m_format)
					{
						case Format::R32_FLOAT:
						{
							copyChannels(1);
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
							break;
						}
					}
				}
				else if (m_channels == 2)
				{
					switch (m_format)
					{
						case Format::R32G32_FLOAT:
						{
							copyChannels(2);
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
						}
					}
				}
				else if (m_channels == 3)
				{
					switch (m_format)
					{
						case Format::R32G32B32_FLOAT:
						{
							copyChannels(3);
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
						}
					}
				}
				else if (m_channels == 4)
				{
					switch (m_format)
					{
						case Format::R32G32B32A32_FLOAT:
						{
							copyChannels(4);
							break;
						}
						default:
						{
							ASSERT(false, "Unhandled destination format");
						}
					}
				}
			}
		}

		ExternalCodecs::ExternalCodecs(const engine::string& filename,
			Format format,
			unsigned int width,
			unsigned int height,
			unsigned int slices,
			unsigned int mips)
			: mFilename(filename)
			, m_width{ width }
			, m_height{ height }
			, m_format{ format }
			, m_mips{ mips }
			, m_slices{ slices }
			, m_channels{ 0u }
			, m_integerFormat{ true }
		{
			ASSERT(false, "TODO: External codecs don't support writing");
		}

		size_t ExternalCodecs::width() const
		{
			return m_width;
		}

		size_t ExternalCodecs::height() const
		{
			return m_height;
		}

		Format ExternalCodecs::format() const
		{
			return m_format;
		}

		size_t ExternalCodecs::mipCount() const
		{
			return m_mips;
		}

		size_t ExternalCodecs::arraySlices() const
		{
			return m_slices;
		}

		ImageSubresource ExternalCodecs::map(
			size_t mipLevel,
			size_t arraySlice) const
		{
			ASSERT(mipLevel == 0, "External codec does not currently support other than mip level 0");
			ASSERT(arraySlice == 0, "External codec does not currently support other than slice 0");

			ImageSubresource res;
			res.width = width();
			res.height = height();
			res.data = m_data.data();
			res.pitch = formatBytes(m_format, res.width, 1);
			res.slicePitch = res.pitch * res.height;
			res.sizeBytes = res.slicePitch;
			return res;
		}

		void ExternalCodecs::save()
		{
			ASSERT(false, "TODO: External codecs don't support writing");
		}

		void ExternalCodecs::reserve()
		{
			ASSERT(false, "TODO: External codecs don't support writing");
		}

		void ExternalCodecs::save(const char* /*data*/, size_t /*bytes*/)
		{
			ASSERT(false, "TODO: External codecs don't support writing");
		}

		const uint8_t* ExternalCodecs::data() const
		{
			return m_data.data();
		}

		size_t ExternalCodecs::bytes() const
		{
			return m_data.size();
		}

		void ExternalCodecs::flipVertical()
		{
			auto src = &m_data[0];
			auto dst = src + formatBytes(m_format, static_cast<unsigned int>(width()), static_cast<unsigned int>(height()));

			auto rowBytes = formatBytes(m_format, static_cast<unsigned int>(width()), 1u);

			auto revPtr = dst - rowBytes;

			engine::vector<uint8_t> scratchBuffer(rowBytes);

			for (int y = 0; y < height() / 2; ++y)
			{
				memcpy(&scratchBuffer[0], src, rowBytes);
				memcpy(src, revPtr, rowBytes);
				memcpy(revPtr, &scratchBuffer[0], rowBytes);

				revPtr -= rowBytes;
				src += rowBytes;
			}
		}

		void ExternalCodecs::convert()
		{
			ASSERT(false, "TODO: External codecs don't support conversions");
		}

	}
}
