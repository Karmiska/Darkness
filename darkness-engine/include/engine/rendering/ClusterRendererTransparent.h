#pragma once

#include "shaders/core/model/ClusterRenderTransparent.h"
#include "shaders/core/tools/BlurDownsample.h"
#include "engine/graphics/Pipeline.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/graphics/ResourceOwners.h"

namespace engine
{
    class Device;
    class CommandList;
    class ModelResources;
    class GBuffer;
    class TextureDSV;
    class LightData;

    struct ClusterRendererTransparencyArgs
    {
        CommandList* cmd;
        Camera* camera;

        TextureRTV target;
        TextureDSV depthBuffer;
        TextureSRV depthView;
        TextureSRV ssao;
        TextureSRV motion;

        uint64_t frameNumber;
        Vector2<int> virtualResolution;

        // temporal stuff
        Matrix4f previousViewMatrix;
        Matrix4f previousProjectionMatrix;

        TextureSRV* shadowMap;
        BufferSRV* shadowVP;

        Matrix4f cameraProjectionMatrix;
        Matrix4f cameraViewMatrix;
        LightData* lights;
        Vector3f probePosition;
        float probeRange;

        int currentRenderMode;
    };

    class ClusterRendererTransparent
    {
    public:
        ClusterRendererTransparent(Device& device);

        void createDownsamples(CommandList& cmd, TextureSRV frame);

        void render(
            ClusterRendererTransparencyArgs args,
            ClusterDataLine& clusters,
            IndexDataLine& indexes,
            DrawDataLine& draws,
			BufferSRV lightBins,
            TextureSRV frame);

        TextureSRV frameDownsampleChain()
        {
			return m_frameBlurSRV;
        }
    private:
        Device& m_device;
        engine::vector<engine::Pipeline<shaders::ClusterRenderTransparent>> m_pipelines;
		engine::Pipeline<shaders::BlurDownsample> m_blurDownsample;

		TextureRTVOwner m_frameBlurRTV;
		TextureSRVOwner m_frameBlurSRV;
		engine::vector<TextureUAVOwner> m_frameBlurMipsUAV;
		engine::vector<TextureSRVOwner> m_frameBlurMipsSRV;
		TextureSRV m_frame;
    };
}
