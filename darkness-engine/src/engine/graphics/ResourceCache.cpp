#include "engine/graphics/ResourceCache.h"
#include "engine/rendering/Mesh.h"
#include "engine/graphics/Device.h"

namespace engine
{
	TextureSRVOwner ResourceCache::createTextureSRV(ResourceKey key, ResourceCreateTextureSRV create)
    {
#ifdef RESOURCE_CACHE_TEXTURESRV
        auto existing = m_textureSRV.find(key);
        if (existing != m_textureSRV.end())
        {
            return existing->second;
        }

		TextureSRVOwner result = create();
        m_textureSRV[key] = result;
        return result;
#else
        return create();
#endif
    }

	BufferSRVOwner ResourceCache::createBufferSRV(ResourceKey key, ResourceCreateBufferSRV create)
    {
#ifdef RESOURCE_CACHE_BUFFERSRV
        auto existing = m_bufferSRV.find(key);
        if (existing != m_bufferSRV.end())
        {
            return existing->second;
        }

		BufferSRVOwner result = create();
        m_bufferSRV[key] = result;
        return result;
#else
        return create();
#endif
    }

	BufferIBVOwner ResourceCache::createBufferIBV(ResourceKey key, ResourceCreateBufferIBV create)
    {
#ifdef RESOURCE_CACHE_BUFFERIBV
        auto existing = m_bufferIBV.find(key);
        if (existing != m_bufferIBV.end())
        {
            return existing->second;
        }

		BufferIBVOwner result = create();
        m_bufferIBV[key] = result;
        return result;
#else
        return create();
#endif
    }

    template<>
    bool ResourceCache::cachedDataExists<TextureSRV>(ResourceKey key) const
    {
#ifdef RESOURCE_CACHE_TEXTURESRV
        return m_textureSRV.find(key) != m_textureSRV.end();
#else
        return false;
#endif
    }

    template<>
    bool ResourceCache::cachedDataExists<BufferSRV>(ResourceKey key) const
    {
#ifdef RESOURCE_CACHE_BUFFERSRV
        return m_bufferSRV.find(key) != m_bufferSRV.end();
#else
        return false;
#endif
    }

    template<>
    bool ResourceCache::cachedDataExists<BufferIBV>(ResourceKey key) const
    {
#ifdef RESOURCE_CACHE_BUFFERIBV
        return m_bufferIBV.find(key) != m_bufferIBV.end();
#else
        return false;
#endif
    }

    template<>
    bool ResourceCache::cachedDataExists<image::ImageIf>(ResourceKey key) const
    {
#ifdef RESOURCE_CACHE_IMAGE
        return m_images.find(key) != m_images.end();
#else
        return false;
#endif
    }

    template<>
    bool ResourceCache::cachedDataExists<Mesh>(ResourceKey key) const
    {
#ifdef RESOURCE_CACHE_MESH
        return m_meshes.find(key) != m_meshes.end();
#else
        return false;
#endif
    }

    engine::shared_ptr<image::ImageIf> ResourceCache::createImage(
        ResourceKey key,
        const engine::string& filename,
        Format type,
        int width,
        int height,
        int slices,
        int mips,
		image::ImageType imageType)
    {
#ifdef RESOURCE_CACHE_IMAGE
        auto existing = m_images.find(key);
        if (existing != m_images.end())
        {
            return existing->second;
        }

        engine::shared_ptr<image::ImageIf> result = image::Image::createImage(
            filename, imageType, type, width, height, slices, mips);
        m_images[key] = result;
        return result;
#else
        return image::Image::createImage(
            filename, type, width, height, slices, mips);
#endif
    }

    const engine::unordered_map<ResourceKey, engine::shared_ptr<Mesh>>& ResourceCache::meshes() const
    {
        return m_meshes;
    }

    engine::shared_ptr<SubMeshInstance> ResourceCache::createMesh(
        Device& device,
        ResourceKey key,
        const engine::string& filename,
        uint32_t meshIndex)
    {
#ifdef RESOURCE_CACHE_MESH
        auto existing = m_meshes.find(key);
        if (existing != m_meshes.end())
        {
            return existing->second->subMeshes()[meshIndex].createInstance(device);
        }

        engine::shared_ptr<Mesh> result = engine::make_shared<Mesh>(device.residencyV2(), device.modelResources(), filename);
        m_meshes[key] = result;
        return result->subMeshes()[meshIndex].createInstance(device);
#else
        return engine::make_shared<Mesh>(filename);
#endif
    }

    void ResourceCache::clear()
    {
        m_textureSRV.clear();
        m_bufferSRV.clear();
        m_bufferIBV.clear();
        m_images.clear();
        m_meshes.clear();
    }
}
