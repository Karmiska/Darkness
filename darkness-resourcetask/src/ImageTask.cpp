#include "ImageTask.h"
#include "platform/Platform.h"
#include "protocols/network/ResourceProtocols.pb.h"
#include "Compressonator.h"
#include "engine/network/MqMessage.h"
#include "engine/graphics/Format.h"
#include "tools/image/Image.h"
#include "tools/Debug.h"
#include "tools/PathTools.h"
#include "ImageHelper.h"

#include "containers/memory.h"
#include "containers/string.h"
#include <fstream>

using namespace engine;
using namespace zmq;
using namespace engine;

namespace resource_task
{
    namespace internals
    {
        CompressonatorInitializer::CompressonatorInitializer()
        {
            auto error = CMP_InitializeBCLibrary();
            if (error != BC_ERROR_NONE)
            {
                switch (error)
                {
                case BC_ERROR_LIBRARY_NOT_INITIALIZED: LOG("BC_ERROR_LIBRARY_NOT_INITIALIZED"); break;
                case BC_ERROR_LIBRARY_ALREADY_INITIALIZED: LOG("BC_ERROR_LIBRARY_ALREADY_INITIALIZED"); break;
                case BC_ERROR_INVALID_PARAMETERS: LOG("BC_ERROR_INVALID_PARAMETERS"); break;
                case BC_ERROR_OUT_OF_MEMORY: LOG("BC_ERROR_OUT_OF_MEMORY"); break;
                }
            }
        }

        CompressonatorInitializer::~CompressonatorInitializer()
        {
            auto error = CMP_ShutdownBCLibrary();
            if (error != BC_ERROR_NONE)
            {
                switch (error)
                {
                case BC_ERROR_LIBRARY_NOT_INITIALIZED: LOG("BC_ERROR_LIBRARY_NOT_INITIALIZED"); break;
                case BC_ERROR_LIBRARY_ALREADY_INITIALIZED: LOG("BC_ERROR_LIBRARY_ALREADY_INITIALIZED"); break;
                case BC_ERROR_INVALID_PARAMETERS: LOG("BC_ERROR_INVALID_PARAMETERS"); break;
                case BC_ERROR_OUT_OF_MEMORY: LOG("BC_ERROR_OUT_OF_MEMORY"); break;
                }
            }
        }
    }

    struct ImporterSurfaceWork
    {
        engine::string taskId;
        zmq::socket_t* socket;
        engine::string hostId;
        float lastProgress;
    };

    bool feedback(float fProgress, DWORD_PTR pUser1, DWORD_PTR /*pUser2*/)
    {
        auto surfaceWork = reinterpret_cast<ImporterSurfaceWork*>(pUser1);

        float progress = fProgress / 100.0f;

        if (progress != surfaceWork->lastProgress)
        {
            surfaceWork->lastProgress = progress;
            ProcessorTaskMessageType type;
            type.set_type(ProcessorTaskMessageType::TaskProgress);
            engine::vector<char> type_message(type.ByteSizeLong());
            if (type_message.size() > 0)
                type.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

            ProcessorTaskProgress prog;
            prog.set_taskid(surfaceWork->taskId.c_str());
            prog.set_progress(progress);
            engine::vector<char> progData(prog.ByteSizeLong());
            if (progData.size() > 0)
                prog.SerializeToArray(&progData[0], static_cast<int>(progData.size()));

            MqMessage msg;
            msg.emplace_back(surfaceWork->hostId);
            msg.emplace_back("");
            msg.emplace_back(std::move(type_message));
            msg.emplace_back(std::move(progData));
            if(surfaceWork->socket)
                msg.send(*surfaceWork->socket);
        }
        return false;
    }

    CMP_FORMAT compressonatorFormat(ImageHelper& helper)
    {
        switch (helper.channels())
        {
            case 1:
            {
                if      (helper.channelFormat() == ChannelFormat::Uint8)    return CMP_FORMAT_R_8;
                else if (helper.channelFormat() == ChannelFormat::Uint16)   return CMP_FORMAT_R_16;
                else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_R_32F;
                break;
            }
            case 2:
            {
                if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_RG_8;
                else if (helper.channelFormat() == ChannelFormat::Uint16)   return CMP_FORMAT_RG_16;
                else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_RG_32F;
                break;
            }
            case 3:
            {
                switch (helper.channelOrder())
                {
                    case ChannelOrder::BGR:
                    {
                        if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_BGR_888;
                        else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_BGR_32F;
                        break;
                    }
                    case ChannelOrder::RGB:
                    {
                        if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_RGB_888;
                        else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_RGB_32F;
                        break;
                    }
                }
                break;
            }
            case 4:
            {
                switch (helper.channelOrder())
                {
                    case ChannelOrder::ABGR:
                    {
                        if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_ABGR_8888;
                        else if (helper.channelFormat() == ChannelFormat::Uint16)   return CMP_FORMAT_ABGR_16;
                        else if (helper.channelFormat() == ChannelFormat::Float16)  return CMP_FORMAT_ABGR_16F;
                        else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_ABGR_32F;
                        break;
                    }
                    case ChannelOrder::ARGB:
                    {
                        if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_ARGB_8888;
                        else if (helper.channelFormat() == ChannelFormat::Uint16)   return CMP_FORMAT_ARGB_16;
                        else if (helper.channelFormat() == ChannelFormat::Float16)  return CMP_FORMAT_ARGB_16F;
                        else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_ARGB_32F;
                        break;
                    }
                    case ChannelOrder::BGRA:
                    {
                        if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_BGRA_8888;
                        else if (helper.channelFormat() == ChannelFormat::Uint16)   return CMP_FORMAT_BGRA_16;
                        else if (helper.channelFormat() == ChannelFormat::Float16)  return CMP_FORMAT_BGRA_16F;
                        else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_BGRA_32F;
                        break;
                    }
                    case ChannelOrder::RGBA:
                    {
                        if (helper.channelFormat() == ChannelFormat::Uint8)         return CMP_FORMAT_RGBA_8888;
                        else if (helper.channelFormat() == ChannelFormat::Uint16)   return CMP_FORMAT_RGBA_16;
                        else if (helper.channelFormat() == ChannelFormat::Float16)  return CMP_FORMAT_RGBA_16F;
                        else if (helper.channelFormat() == ChannelFormat::Float32)  return CMP_FORMAT_RGBA_32F;
                        break;
                    }
                }
                break;
            }
        }
        ASSERT(false, "Unknown source image format");
        return CMP_FORMAT_Unknown;
    }

    void ImageTask::process(
        const engine::string& srcFile,
        const engine::string& dstFile)
    {
        ImageData image(srcFile);
        ImageHelper helper(image);

        // let's make some guesses
        if (helper.channels() == 3 &&
            helper.channelFormat() == ChannelFormat::Uint8)
        {
            engine::vector<char> imageOutputData;
            // propably basic RGB 8 bit per channel image
            // so we're guessing it's diffuse.
            // diffuse we want to pack to BC7
            auto mips = mipInfo(ImageSize{ helper.width(), helper.height() }, true);
            for (auto& mip : mips)
            {
                ImageData mipData(image, mip);

                ProcessorTaskImageRequest request;
                request.set_taskid("");
                request.set_enginepackedformat(static_cast<int32_t>(Format::BC7_UNORM_SRGB));
                request.set_targetcmbcformat(static_cast<int32_t>(CMP_FORMAT_BC7));
                request.set_width(mip.width);
                request.set_height(mip.height);
                request.set_stride(mipData.stride());
                request.set_sourcecmformat(static_cast<int32_t>(compressonatorFormat(helper)));
                request.set_data(mipData.data(), mipData.bytes());
                
                auto resp = process(request, "somehostId", nullptr);

                auto currentSize = imageOutputData.size();
                imageOutputData.resize(currentSize + resp.data().size());
                memcpy(&imageOutputData[currentSize], resp.data().data(), resp.data().size());
            }

            auto outputimage = engine::image::Image::createImage(
                dstFile,
                image::ImageType::DDS,
                Format::BC7_UNORM_SRGB,
                helper.width(), helper.height(),
                1,
                mips.size());
            outputimage->save(imageOutputData.data(), imageOutputData.size());
        }
		else if (helper.channels() == 4 &&
			helper.channelFormat() == ChannelFormat::Uint16)
		{
			engine::vector<char> imageOutputData;
			// propably RGBA 16 bit per channel image
			// so we're guessing it's diffuse.
			// diffuse we want to pack to BC7
			auto mips = mipInfo(ImageSize{ helper.width(), helper.height() }, true);
			for (auto& mip : mips)
			{
				ImageData mipData(image, mip);

				ProcessorTaskImageRequest request;
				request.set_taskid("");
				request.set_enginepackedformat(static_cast<int32_t>(Format::BC7_UNORM_SRGB));
				request.set_targetcmbcformat(static_cast<int32_t>(CMP_FORMAT_BC7));
				request.set_width(mip.width);
				request.set_height(mip.height);
				request.set_stride(mipData.stride());
				request.set_sourcecmformat(static_cast<int32_t>(compressonatorFormat(helper)));
				request.set_data(mipData.data(), mipData.bytes());

				auto resp = process(request, "somehostId", nullptr);

				auto currentSize = imageOutputData.size();
				imageOutputData.resize(currentSize + resp.data().size());
				memcpy(&imageOutputData[currentSize], resp.data().data(), resp.data().size());
			}

			auto outputimage = engine::image::Image::createImage(
				dstFile,
				image::ImageType::DDS,
				Format::BC7_UNORM_SRGB,
				helper.width(), helper.height(),
				1,
				mips.size());
			outputimage->save(imageOutputData.data(), imageOutputData.size());
		}
        else if (helper.channels() == 3 &&
            helper.channelFormat() == ChannelFormat::Float32)
        {
            // HDR image?
            engine::vector<char> imageOutputData;
            // propably basic RGB 8 bit per channel image
            // so we're guessing it's diffuse.
            // diffuse we want to pack to BC7
            auto mips = mipInfo(ImageSize{ helper.width(), helper.height() }, true);
            for (auto& mip : mips)
            {
                ImageData mipData(image, mip);

                ProcessorTaskImageRequest request;
                request.set_taskid("");
                request.set_enginepackedformat(static_cast<int32_t>(Format::BC6H_UF16));
                request.set_targetcmbcformat(static_cast<int32_t>(CMP_FORMAT_BC6H));
                request.set_width(mip.width);
                request.set_height(mip.height);
                request.set_stride(mipData.stride());
                request.set_sourcecmformat(static_cast<int32_t>(compressonatorFormat(helper)));
                request.set_data(mipData.data(), mipData.bytes());

                auto resp = process(request, "somehostId", nullptr);

                auto currentSize = imageOutputData.size();
                imageOutputData.resize(currentSize + resp.data().size());
                memcpy(&imageOutputData[currentSize], resp.data().data(), resp.data().size());
            }

            auto outputimage = engine::image::Image::createImage(
                dstFile,
                image::ImageType::DDS,
                Format::BC6H_UF16,
                helper.width(), helper.height(),
                1,
                mips.size());
            outputimage->save(imageOutputData.data(), imageOutputData.size());
        }
		else if (helper.channels() == 1 &&
			helper.channelFormat() == ChannelFormat::Uint8)
		{
            // 1 channel image.
            // maybe roughness or metalness?
			engine::vector<char> imageOutputData;
			auto mips = mipInfo(ImageSize{ helper.width(), helper.height() }, true);
			for (auto& mip : mips)
			{
				ImageData mipData(image, mip);

				ProcessorTaskImageRequest request;
				request.set_taskid("");
				request.set_enginepackedformat(static_cast<int32_t>(Format::BC4_UNORM));
				request.set_targetcmbcformat(static_cast<int32_t>(CMP_FORMAT_BC4));
				request.set_width(mip.width);
				request.set_height(mip.height);
				request.set_stride(mipData.stride());
				request.set_sourcecmformat(static_cast<int32_t>(compressonatorFormat(helper)));
				request.set_data(mipData.data(), mipData.bytes());

				auto resp = process(request, "somehostId", nullptr);

				auto currentSize = imageOutputData.size();
				imageOutputData.resize(currentSize + resp.data().size());
				memcpy(&imageOutputData[currentSize], resp.data().data(), resp.data().size());
			}

			auto outputimage = engine::image::Image::createImage(
				dstFile,
				image::ImageType::DDS,
				Format::BC4_UNORM,
				helper.width(), helper.height(),
				1,
				mips.size());
			outputimage->save(imageOutputData.data(), imageOutputData.size());
		}
        else if (helper.channels() == 1 &&
			helper.channelFormat() == ChannelFormat::Float32)
		{
            // 1 channel image.
            // 32 bit float data. maybe heightmap?
			engine::vector<char> imageOutputData;
			auto mips = mipInfo(ImageSize{ helper.width(), helper.height() }, true);
			for (auto& mip : mips)
			{
				ImageData mipData(image, mip);

				ProcessorTaskImageRequest request;
				request.set_taskid("");
				request.set_enginepackedformat(static_cast<int32_t>(Format::BC4_UNORM));
				request.set_targetcmbcformat(static_cast<int32_t>(CMP_FORMAT_BC4));
				request.set_width(mip.width);
				request.set_height(mip.height);
				request.set_stride(mipData.stride());
				request.set_sourcecmformat(static_cast<int32_t>(compressonatorFormat(helper)));
				request.set_data(mipData.data(), mipData.bytes());

				auto resp = process(request, "somehostId", nullptr);

				auto currentSize = imageOutputData.size();
				imageOutputData.resize(currentSize + resp.data().size());
				memcpy(&imageOutputData[currentSize], resp.data().data(), resp.data().size());
			}

			auto outputimage = engine::image::Image::createImage(
				dstFile,
				image::ImageType::DDS,
				Format::BC4_UNORM,
				helper.width(), helper.height(),
				1,
				mips.size());
			outputimage->save(imageOutputData.data(), imageOutputData.size());
		}
    }

    ProcessorTaskImageResponse ImageTask::process(
        ProcessorTaskImageRequest& request,
        const engine::string& hostId,
        zmq::socket_t* socket)
    {
        return privateProcess(
            request.taskid().c_str(),
            request.enginepackedformat(),
            request.width(),
            request.height(),
            request.targetcmbcformat(),
            request.stride(),
            request.sourcecmformat(),
            request.data().data(),
            request.data().size(),
            hostId,
            socket);
    }

    ProcessorTaskImageResponse ImageTask::privateProcess(
        const engine::string& taskId,
        int enginepackedformat,
        int width,
        int height,
        int targetcmbcformat,
        int stride,
        int sourcecmformat,
        const char* data,
        size_t bytes,
        const engine::string& hostId,
        zmq::socket_t* socket)
    {
        ProcessorTaskImageResponse response;
        response.set_taskid(taskId.c_str());

        auto destinationSizeBytes = static_cast<uint32_t>(engine::formatBytes(
            static_cast<Format>(enginepackedformat),
            width,
            height));

        engine::vector<char> dstBuffer(destinationSizeBytes);

        CMP_Texture dstTexture;
        dstTexture.dwSize = sizeof(CMP_Texture);
        dstTexture.dwWidth = width;
        dstTexture.dwHeight = height;
        dstTexture.dwPitch = 0;
        dstTexture.format = static_cast<CMP_FORMAT>(targetcmbcformat);
        dstTexture.nBlockHeight = 4;
        dstTexture.nBlockWidth = 4;
        dstTexture.nBlockDepth = 1;
        dstTexture.dwDataSize = static_cast<CMP_DWORD>(destinationSizeBytes);
        dstTexture.pData = reinterpret_cast<uint8_t*>(dstBuffer.data());// +currentDstBytes;

        CMP_Texture srcTexture;
        srcTexture.dwSize = sizeof(CMP_Texture);
        srcTexture.dwWidth = width;
        srcTexture.dwHeight = height;
        srcTexture.dwPitch = stride;
        srcTexture.format = static_cast<CMP_FORMAT>(sourcecmformat);
        srcTexture.dwDataSize = static_cast<CMP_DWORD>(bytes);// dibSize;
        srcTexture.pData = const_cast<CMP_BYTE*>(reinterpret_cast<const CMP_BYTE*>(data));

        CMP_CompressOptions options = {};
        options.dwSize = sizeof(CMP_CompressOptions);
        options.bDisableMultiThreading = true;
        //options.fquality = 1.0;

        ImporterSurfaceWork work;
        work.taskId = taskId;
        work.lastProgress = 0.0f;
        work.socket = socket;
        work.hostId = hostId;

        CMP_ConvertTexture(
            &srcTexture,
            &dstTexture,
            &options,
            feedback,
            reinterpret_cast<DWORD_PTR>(&work),
            reinterpret_cast<DWORD_PTR>(nullptr));

        response.set_width(width);
        response.set_height(height);
        response.set_format(enginepackedformat);
        response.set_data(dstBuffer.data(), dstBuffer.size());
        LOG("Task finished encoding: %s", taskId.c_str());
        return response;
    }
}
