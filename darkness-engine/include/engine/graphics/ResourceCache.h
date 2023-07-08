#pragma once

#include "engine/graphics/Resources.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/rendering/Mesh.h"
#include "tools/image/Image.h"
#include <functional>
#include "containers/unordered_map.h"

#define RESOURCE_CACHE_TEXTURESRV
#define RESOURCE_CACHE_BUFFERSRV
#define RESOURCE_CACHE_BUFFERIBV
#define RESOURCE_CACHE_IMAGE
#define RESOURCE_CACHE_MESH

namespace engine
{
    class Mesh;
    class ModelResources;
    using ResourceCreateTextureSRV = std::function<TextureSRVOwner()>;
    using ResourceCreateBufferSRV = std::function<BufferSRVOwner()>;
    using ResourceCreateBufferIBV = std::function<BufferIBVOwner()>;
    class ResourceCache
    {
    public:
		TextureSRVOwner createTextureSRV(ResourceKey key, ResourceCreateTextureSRV create);
		BufferSRVOwner createBufferSRV(ResourceKey key, ResourceCreateBufferSRV create);
		BufferIBVOwner createBufferIBV(ResourceKey key, ResourceCreateBufferIBV create);

        engine::shared_ptr<image::ImageIf> createImage(
            ResourceKey key, 
            const engine::string& filename,
            Format type = Format::BC7_UNORM,
            int width = -1,
            int height = -1,
            int slices = -1,
            int mips = -1,
			image::ImageType imageType = image::ImageType::DDS);

        engine::shared_ptr<SubMeshInstance> createMesh(
            Device& device,
            ResourceKey key,
            const engine::string& filename,
            uint32_t meshIndex);

        template<typename T>
        bool cachedDataExists(ResourceKey key) const;

        void clear();

        const engine::unordered_map<ResourceKey, engine::shared_ptr<Mesh>>& meshes() const;
    private:
        engine::unordered_map<ResourceKey, TextureSRVOwner> m_textureSRV;
        engine::unordered_map<ResourceKey, BufferSRVOwner> m_bufferSRV;
        engine::unordered_map<ResourceKey, BufferIBVOwner> m_bufferIBV;
        engine::unordered_map<ResourceKey, engine::shared_ptr<image::ImageIf>> m_images;
        engine::unordered_map<ResourceKey, engine::shared_ptr<Mesh>> m_meshes;
    };
}
