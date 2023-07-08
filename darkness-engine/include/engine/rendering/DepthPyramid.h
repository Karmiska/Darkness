#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Sampler.h"
#include "shaders/core/tools/DepthDownsampleMinMax.h"
#include "shaders/core/tools/DepthDownsampleMinMaxSingle.h"
#include "containers/vector.h"

namespace engine
{
    class CommandList;
    class DepthPyramid
    {
    public:
        DepthPyramid(
            Device& device,
            int width,
            int height);

        void refresh(engine::Vector2<int> virtualResolution);

        TextureDSV dsv()
        {
            return m_dsv;
        }

        TextureSRV srv()
        {
            return m_dsvSRV;
        }

        TextureSRV depth()
        {
            return m_depthPyramidSRV;
        }

        TextureSRV sl()
        {
            return m_srvs[0];
        }

		TextureSRV depthMax()
		{
			return m_depthPyramidSRVMax;
		}

		TextureSRV slMax()
		{
			return m_srvsMax[0];
		}

        // 3. Depth downsample
        void performDownsample(CommandList& cmd);

		engine::vector<TextureSRV>& srvs()
		{
			return m_srvsView;
		}

		engine::vector<TextureSRV>& srvsMax()
		{
			return m_srvsViewMax;
		}
    private:
        Device& m_device;
        engine::Pipeline<shaders::DepthDownsampleMinMax> m_depthDownsample;
        engine::Pipeline<shaders::DepthDownsampleMinMaxSingle> m_depthDownsampleSingle;

        Sampler m_sampler;
        int m_width;
        int m_height;
        int m_widthPow2;
        int m_heightPow2;
        bool m_needClean;

        TextureDSVOwner m_dsv;
        TextureSRVOwner m_dsvSRV;

        TextureOwner m_depthPyramid;
        TextureSRVOwner m_depthPyramidSRV;
        engine::vector<TextureSRVOwner> m_srvs;
		engine::vector<TextureSRV> m_srvsView;
        engine::vector<TextureUAVOwner> m_uavs;

		TextureOwner m_depthPyramidMax;
		TextureSRVOwner m_depthPyramidSRVMax;
		engine::vector<TextureSRVOwner> m_srvsMax;
		engine::vector<TextureSRV> m_srvsViewMax;
		engine::vector<TextureUAVOwner> m_uavsMax;

    };
}
