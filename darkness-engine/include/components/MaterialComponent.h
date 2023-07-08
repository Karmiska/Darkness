#pragma once

#include "engine/EngineComponent.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/rendering/tools/ExtractAlphaMask.h"
#include "engine/filesystem/VirtualFilesystem.h"
#include "tools/image/Image.h"
#include "tools/Property.h"
#include "tools/hash/Hash.h"
#include "tools/PathTools.h"
#include "platform/File.h"

namespace engine
{
    class MaterialComponent : public EngineComponent
    {
        Property m_color;

        bool m_hasAlbedo;
        bool m_hasRoughness;
        bool m_hasNormal;
        bool m_hasMetalness;
        bool m_hasOcclusion;

        Property m_albedoPath;
        Property m_normalPath;
        Property m_roughnessPath;
        Property m_roughnessStrength;
        Property m_metalnessPath;
        Property m_metalnessStrength;
        Property m_occlusionPath;
        Property m_occlusionStrength;
        Property m_materialScaleX;
        Property m_materialScaleY;
        Property m_transparent;
        Property m_alphaclipped;

        Property m_albedoUVIndex;
        Property m_roughnessUVIndex;
        Property m_normalUVIndex;
        Property m_metalnessUVIndex;
        Property m_occlusionUVIndex;

        uint32_t m_cpuDirty;
        uint32_t m_gpuDirty;

        engine::shared_ptr<image::ImageIf> m_albedo;
        engine::shared_ptr<image::ImageIf> m_roughness;
        engine::shared_ptr<image::ImageIf> m_normal;
        engine::shared_ptr<image::ImageIf> m_metalness;
        engine::shared_ptr<image::ImageIf> m_occlusion;

        TextureSRVOwner m_albedoSrv;
		TextureSRVOwner m_albedoAlphaMaskSrv;
        TextureSRVOwner m_roughnessSrv;
        TextureSRVOwner m_normalSrv;
        TextureSRVOwner m_metalnessSrv;
        TextureSRVOwner m_occlusionSrv;

        bool m_albedoChanged;
        bool m_roughnessChanged;
        bool m_normalChanged;
        bool m_metalnessChanged;
        bool m_occlusionChanged;

        bool m_materialDirty;

		engine::shared_ptr<ExtractAlphaMask> m_extractAlphaMask;

        ResourceKey m_albedoKey;
		ResourceKey m_albedoAlphaMaskKey;
        ResourceKey m_roughnessKey;
        ResourceKey m_normalKey;
        ResourceKey m_metalnessKey;
        ResourceKey m_occlusionKey;

        TextureSRVOwner createTexture(ResourceKey key, Device& device, image::ImageIf* image, bool srgb = false)
        {
            return device.createTextureSRV(key, TextureDescription()
                .name("color")
                .width(image->width())
                .height(image->height())
                .format(srgb ? srgbFormat(image->format()) : image->format())
                .arraySlices(image->arraySlices())
                .mipLevels(image->mipCount() == 0 ? 1 : image->mipCount())
                .setInitialData(TextureDescription::InitialData(
                    tools::ByteRange(
                        image->data(), 
                        image->data() + image->bytes()),
                    image->width(), image->width() * image->height())));
        }

    public:

        engine::shared_ptr<EngineComponent> clone() const override
        {
            auto res = engine::make_shared<engine::MaterialComponent>(
                m_albedoPath.value<engine::string>(),
                m_roughnessPath.value<engine::string>(),
                m_normalPath.value<engine::string>(),
                m_metalnessPath.value<engine::string>(),
                m_occlusionPath.value<engine::string>(),
                m_albedoUVIndex.value<int>(),
                m_roughnessUVIndex.value<int>(),
                m_normalUVIndex.value<int>(),
                m_metalnessUVIndex.value<int>(),
                m_occlusionUVIndex.value<int>());
            res->m_color.value<Vector3f>(m_color.value<Vector3f>());
            res->m_hasAlbedo = m_hasAlbedo;
            res->m_hasRoughness = m_hasRoughness;
            res->m_hasNormal = m_hasNormal;
            res->m_hasMetalness = m_hasMetalness;
            res->m_hasOcclusion = m_hasOcclusion;

            res->roughnessStrength(roughnessStrength());
            res->metalnessStrength(metalnessStrength());
            res->occlusionStrength(occlusionStrength());
            res->materialScaleX(materialScaleX());
            res->materialScaleY(materialScaleY());
            res->transparent(transparent());
            res->alphaclipped(alphaclipped());
            res->name(m_name);
            return res;
        }

        MaterialComponent()
            : m_color{ this, "Color", Vector3f{ 1.0f, 1.0f, 1.0f }, [this]() { this->m_materialDirty = true; } }
            , m_hasAlbedo{ false }
            , m_hasRoughness{ false }
            , m_hasNormal{ false }
            , m_hasMetalness{ false }
            , m_hasOcclusion{ false }
            , m_albedoPath{ this, "Albedo", engine::string(""), [this]() { this->m_cpuDirty |= TextureTypeBits::Albedo; this->m_albedoChanged = true; m_hasAlbedo = checkMaterialFile(m_albedoPath, "albedo"); this->m_materialDirty = true; } }
            , m_normalPath{ this, "Normal", engine::string(""), [this]() { this->m_cpuDirty |= TextureTypeBits::Normal; this->m_normalChanged = true; m_hasNormal = checkMaterialFile(m_normalPath, "normal"); this->m_materialDirty = true; } }
            , m_roughnessPath{ this, "Roughness", engine::string(""), [this]() { this->m_cpuDirty |= TextureTypeBits::Roughness; this->m_roughnessChanged = true; m_hasRoughness = checkMaterialFile(m_roughnessPath, "roughness"); this->m_materialDirty = true; } }
            , m_roughnessStrength{ this, "Roughness Strength", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_metalnessPath{ this, "Metalness", engine::string(""), [this]() { this->m_cpuDirty |= TextureTypeBits::Metalness; this->m_metalnessChanged = true; m_hasMetalness = checkMaterialFile(m_metalnessPath, "metalness"); this->m_materialDirty = true; } }
            , m_metalnessStrength{ this, "Metalness Strength", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_occlusionPath{ this, "Occlusion", engine::string(""), [this]() { this->m_cpuDirty |= TextureTypeBits::Occlusion; this->m_occlusionChanged = true; m_hasOcclusion = checkMaterialFile(m_occlusionPath, "occlusion"); this->m_materialDirty = true; } }
            , m_occlusionStrength{ this, "Occlusion Strength", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_materialScaleX{ this, "Scale X", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_materialScaleY{ this, "Scale y", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_transparent{ this, "Transparent", ButtonToggle::NotPressed, [this]() { this->m_materialDirty = true; } }
            , m_alphaclipped{ this, "Alphaclipped", ButtonToggle::NotPressed, [this]() { this->m_materialDirty = true; } }
            , m_albedoUVIndex{ this, "Albedo UV Index", static_cast<int>(0), [this]() { this->m_materialDirty = true; } }
            , m_roughnessUVIndex{ this, "Roughness UV Index", static_cast<int>(0), [this]() { this->m_materialDirty = true; } }
            , m_normalUVIndex{ this, "Normal UV Index", static_cast<int>(0), [this]() { this->m_materialDirty = true; } }
            , m_metalnessUVIndex{ this, "Metalness UV Index", static_cast<int>(0), [this]() { this->m_materialDirty = true; }  }
            , m_occlusionUVIndex{ this, "Occlusion UV Index", static_cast<int>(0), [this]() { this->m_materialDirty = true; }  }
            , m_cpuDirty{ TextureTypeBits::All }
            , m_gpuDirty{ 0 }
            , m_albedoChanged{ true }
            , m_roughnessChanged{ true }
            , m_normalChanged{ true }
            , m_metalnessChanged{ true }
            , m_occlusionChanged{ true }
            , m_materialDirty{ true }
			, m_extractAlphaMask{ nullptr }
        {
            m_name = "MaterialComponent";
        }

        MaterialComponent(
            const engine::string& albedoPath,
            const engine::string& roughnessPath,
            const engine::string& normalPath,
            const engine::string& metalnessPath,
            const engine::string& occlusionPath,
            int albedoUVIndex,
            int roughnessUVIndex,
            int normalUVIndex,
            int metalnessUVIndex,
            int occlusionUVIndex)
            : m_color{ this, "Color", Vector3f{ 1.0f, 1.0f, 1.0f }, [this]() { this->m_materialDirty = true; } }
            , m_hasAlbedo{ false }
            , m_hasRoughness{ false }
            , m_hasNormal{ false }
            , m_hasMetalness{ false }
            , m_hasOcclusion{ false }
            , m_albedoPath{ this, "Albedo", albedoPath, [this]() { this->m_cpuDirty |= TextureTypeBits::Albedo; this->m_albedoChanged = true; m_hasAlbedo = checkMaterialFile(m_albedoPath, "albedo");this->m_materialDirty = true; } }
            , m_normalPath{ this, "Normal", normalPath, [this]() { this->m_cpuDirty |= TextureTypeBits::Normal; this->m_normalChanged = true; m_hasNormal = checkMaterialFile(m_normalPath, "normal");this->m_materialDirty = true; } }
            , m_roughnessPath{ this, "Roughness", roughnessPath, [this]() { this->m_cpuDirty |= TextureTypeBits::Roughness; this->m_roughnessChanged = true; m_hasRoughness = checkMaterialFile(m_roughnessPath, "roughness"); this->m_materialDirty = true; } }
            , m_roughnessStrength{ this, "Roughness Strength", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_metalnessPath{ this, "Metalness", metalnessPath, [this]() { this->m_cpuDirty |= TextureTypeBits::Metalness; this->m_metalnessChanged = true; m_hasMetalness = checkMaterialFile(m_metalnessPath, "metalness"); this->m_materialDirty = true; } }
            , m_metalnessStrength{ this, "Metalness Strength", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_occlusionPath{ this, "Occlusion", occlusionPath, [this]() { this->m_cpuDirty |= TextureTypeBits::Occlusion; this->m_occlusionChanged = true; m_hasOcclusion = checkMaterialFile(m_occlusionPath, "occlusion"); this->m_materialDirty = true; } }
            , m_occlusionStrength{ this, "Occlusion Strength", 1.0f, [this]() { this->m_materialDirty = true; } }
            , m_materialScaleX{ this, "Scale X", 1.0f, [this]() { this->m_materialDirty = true; }  }
            , m_materialScaleY{ this, "Scale y", 1.0f, [this]() { this->m_materialDirty = true; }  }
            , m_transparent{ this, "Transparent", ButtonToggle::NotPressed, [this]() { this->m_materialDirty = true; } }
            , m_alphaclipped{ this, "Alphaclipped", ButtonToggle::NotPressed, [this]() { this->m_materialDirty = true; } }
            , m_albedoUVIndex{ this, "Albedo UV Index", albedoUVIndex, [this]() { this->m_materialDirty = true; } }
            , m_roughnessUVIndex{ this, "Roughness UV Index", roughnessUVIndex, [this]() { this->m_materialDirty = true; } }
            , m_normalUVIndex{ this, "Normal UV Index", normalUVIndex, [this]() { this->m_materialDirty = true; } }
            , m_metalnessUVIndex{ this, "Metalness UV Index", metalnessUVIndex, [this]() { this->m_materialDirty = true; }  }
            , m_occlusionUVIndex{ this, "Occlusion UV Index", occlusionUVIndex, [this]() { this->m_materialDirty = true; }  }
            , m_cpuDirty{ TextureTypeBits::All }
            , m_gpuDirty{ 0 }
            , m_albedoChanged{ true }
            , m_roughnessChanged{ true }
            , m_normalChanged{ true }
            , m_metalnessChanged{ true }
            , m_occlusionChanged{ true }
            , m_materialDirty{ true }
			, m_extractAlphaMask{ nullptr }
        {
            m_name = "MaterialComponent";
            m_hasAlbedo = checkMaterialFile(m_albedoPath, "albedo");
            m_hasNormal = checkMaterialFile(m_normalPath, "normal");
            m_hasRoughness = checkMaterialFile(m_roughnessPath, "roughness");
            m_hasMetalness = checkMaterialFile(m_metalnessPath, "metalness");
            m_hasOcclusion = checkMaterialFile(m_occlusionPath, "occlusion");
        }

        TextureSRV albedo() { return m_albedoSrv; }
		TextureSRV albedoAlphaMask() { return m_albedoAlphaMaskSrv; }
        TextureSRV normal() { return m_normalSrv; }
        TextureSRV roughness() { return m_roughnessSrv; }
        TextureSRV metalness() { return m_metalnessSrv; }
        TextureSRV occlusion() { return m_occlusionSrv; }

        Vector3f color() const { return m_color.value<Vector3f>(); }
        void color(const Vector3f& col) { m_color.value<Vector3f>(col); m_materialDirty = true; }

        float roughnessStrength() const { return m_roughnessStrength.value<float>(); }
        float metalnessStrength() const { return m_metalnessStrength.value<float>(); }
        float occlusionStrength() const { return m_occlusionStrength.value<float>(); }

        float materialScaleX() const { return m_materialScaleX.value<float>(); }
        float materialScaleY() const { return m_materialScaleY.value<float>(); }

        void roughnessStrength(float val) { m_roughnessStrength.value<float>(val); m_materialDirty = true; }
        void metalnessStrength(float val) { m_metalnessStrength.value<float>(val); m_materialDirty = true; }
        void occlusionStrength(float val) { m_occlusionStrength.value<float>(val); m_materialDirty = true; }

        void materialScaleX(float val) { m_materialScaleX.value<float>(val); m_materialDirty = true; }
        void materialScaleY(float val) { m_materialScaleY.value<float>(val); m_materialDirty = true; }

        bool albedoChanged(bool clear = false)        { bool res = m_albedoChanged; if(clear) m_albedoChanged = false; return res; }
        bool roughnessChanged(bool clear = false)    { bool res = m_roughnessChanged; if(clear) m_roughnessChanged = false; return res; }
        bool normalChanged(bool clear = false)        { bool res = m_normalChanged; if(clear) m_normalChanged = false; return res; }
        bool metalnessChanged(bool clear = false)    { bool res = m_metalnessChanged; if(clear) m_metalnessChanged = false; return res; }
        bool occlusionChanged(bool clear = false)    { bool res = m_occlusionChanged; if(clear) m_occlusionChanged = false; return res; }

        bool hasAlbedo() { return m_hasAlbedo; }
        bool hasRoughness() { return m_hasRoughness; }
        bool hasNormal() { return m_hasNormal; }
        bool hasMetalness() { return m_hasMetalness; }
        bool hasOcclusion() { return m_hasOcclusion; }

        ResourceKey albedoKey() { return m_albedoKey; }
        ResourceKey roughnessKey() { return m_roughnessKey; }
        ResourceKey normalKey() { return m_normalKey; }
        ResourceKey metalnessKey() { return m_metalnessKey; }
        ResourceKey occlusionKey() { return m_occlusionKey; }

        bool transparent() const { return static_cast<bool>(m_transparent.value<ButtonToggle>()); }
        void transparent(bool transparent) { m_transparent.value<ButtonToggle>(static_cast<ButtonToggle>(transparent)); }

        bool alphaclipped() const { return static_cast<bool>(m_alphaclipped.value<ButtonToggle>()); }
        void alphaclipped(bool alphaclipped) { m_alphaclipped.value<ButtonToggle>(static_cast<ButtonToggle>(alphaclipped)); }

        int albedoUVIndex() const { return m_albedoUVIndex.value<int>(); }
        void albedoUVIndex(int index) { m_albedoUVIndex.value<int>(index); }
        int roughnessUVIndex() const { return m_roughnessUVIndex.value<int>(); }
        void roughnessUVIndex(int index) { m_roughnessUVIndex.value<int>(index); }
        int normalUVIndex() const { return m_normalUVIndex.value<int>(); }
        void normalUVIndex(int index) { m_normalUVIndex.value<int>(index); }
        int metalnessUVIndex() const { return m_metalnessUVIndex.value<int>(); }
        void metalnessUVIndex(int index) { m_metalnessUVIndex.value<int>(index); }
        int occlusionUVIndex() const { return m_occlusionUVIndex.value<int>(); }
        void occlusionUVIndex(int index) { m_occlusionUVIndex.value<int>(index); }
        
    private:
        bool checkMaterialFile(const Property& path, const char* materialType)
        {
            auto pathStr = path.value<engine::string>();
            bool hasPath = pathStr != "";

            bool foundFile = false;
            if (hasPath)
                foundFile = engine::fileExists(resolvePath(pathStr));

            if (hasPath && !foundFile)
                LOG_WARNING("Missing %s material file: %s", materialType, pathStr.c_str());

            return hasPath && foundFile;
        }

    public:
        void invalidateGpu()
        {
			m_gpuDirty = TextureTypeBits::All;
        }

    private:
        InstanceMaterial m_clusterMaterial;

		enum InstanceMaterialSets
		{
			HasAlbedo = 0x1,
			HasMetalness = 0x2,
			HasRoughness = 0x4,
			HasNormal = 0x8,
			HasOcclusion = 0x10,
			HasAlphaClipped = 0x20,
			HasTransparent = 0x40,
			HasTerrain = 0x80
		};


        void grabMaterialParameters()
        {
            m_clusterMaterial.materialSet = 0;
            if (hasAlbedo()) m_clusterMaterial.materialSet |= 0x1;
            if (hasMetalness()) m_clusterMaterial.materialSet |= 0x2;
            if (hasRoughness()) m_clusterMaterial.materialSet |= 0x4;
            if (hasNormal()) m_clusterMaterial.materialSet |= 0x8;
            if (hasOcclusion()) m_clusterMaterial.materialSet |= 0x10;
            if (alphaclipped()) m_clusterMaterial.materialSet |= 0x20;
            if (transparent()) m_clusterMaterial.materialSet |= 0x40;

            m_clusterMaterial.roughnessStrength = roughnessStrength();
            m_clusterMaterial.metalnessStrength = metalnessStrength();
            m_clusterMaterial.occlusionStrength = occlusionStrength();

            m_clusterMaterial.scaleX = materialScaleX();
            m_clusterMaterial.scaleY = materialScaleY();

            m_clusterMaterial.color = color();
        }

    public:
        void cpuRefresh(Device& device)
        {
			if (!m_cpuDirty)
				return;

            if (m_cpuDirty & TextureTypeBits::Albedo)
            {
                m_cpuDirty &= ~(TextureTypeBits::Albedo);
                if (hasAlbedo())
                {
                    auto path = m_albedoPath.value<engine::string>();
                    m_albedoKey = tools::hash(pathClean(path));
					m_albedoAlphaMaskKey = tools::hash(pathClean(path)+"AlphaMask");
                    m_albedo = device.createImage(
                        m_albedoKey,
                        path);
                }
                else
                    m_albedo = nullptr;
                m_gpuDirty |= TextureTypeBits::Albedo;
            }

			if (m_cpuDirty & TextureTypeBits::Roughness)
            {
				m_cpuDirty &= ~(TextureTypeBits::Roughness);
                if (hasRoughness())
                {
                    auto path = m_roughnessPath.value<engine::string>();
                    m_roughnessKey = tools::hash(pathClean(path));
                    m_roughness = device.createImage(m_roughnessKey, path);
                }
                else
                    m_roughness = nullptr;
				m_gpuDirty |= TextureTypeBits::Roughness;
            }

			if (m_cpuDirty & TextureTypeBits::Normal)
            {
				m_cpuDirty &= ~(TextureTypeBits::Normal);
                if (hasNormal())
                {
                    auto path = m_normalPath.value<engine::string>();
                    m_normalKey = tools::hash(pathClean(path));
                    m_normal = device.createImage(m_normalKey, path);
                }
                else
                    m_normal = nullptr;
				m_gpuDirty |= TextureTypeBits::Normal;
            }

			if (m_cpuDirty & TextureTypeBits::Metalness)
            {
				m_cpuDirty &= ~(TextureTypeBits::Metalness);
                if (hasMetalness())
                {
                    auto path = m_metalnessPath.value<engine::string>();
                    m_metalnessKey = tools::hash(pathClean(path));
                    m_metalness = device.createImage(m_metalnessKey, path);
                }
                else
                    m_metalness = nullptr;
				m_gpuDirty |= TextureTypeBits::Metalness;
            }

			if (m_cpuDirty & TextureTypeBits::Occlusion)
            {
				m_cpuDirty &= ~(TextureTypeBits::Occlusion);
                if (hasOcclusion())
                {
                    auto path = m_occlusionPath.value<engine::string>();
                    m_occlusionKey = tools::hash(pathClean(path));
                    m_occlusion = device.createImage(m_occlusionKey, path);
                }
                else
                    m_occlusion = nullptr;
				m_gpuDirty |= TextureTypeBits::Occlusion;
            }
        }

        bool gpuRefresh(Device& device)
        {
			if (!m_extractAlphaMask)
			{
				m_extractAlphaMask = engine::make_shared<ExtractAlphaMask>(device);
			}

			if (!m_gpuDirty)
				return false;

            bool change = false;
			if (m_gpuDirty & TextureTypeBits::Albedo)
            {
				m_gpuDirty &= ~(TextureTypeBits::Albedo);
                change = true;
                if (hasAlbedo())
                {
                    m_albedoSrv = createTexture(m_albedoKey, device, m_albedo.get(), true);
                    m_clusterMaterial.albedo = device.modelResources().addMaterial(m_albedoKey, m_albedo.get(), true);
					//m_albedoAlphaMaskSrv = m_extractAlphaMask->extract(m_albedoSrv);
					//m_clusterMaterial.albedoAlphaMask = device.modelResources().addAlphaMask(m_albedoAlphaMaskKey, m_albedoAlphaMaskSrv);
					m_albedoAlphaMaskSrv = {};
                }
				else
				{
					m_albedoSrv = {};
					m_albedoAlphaMaskSrv = {};
				}
            }

			if (m_gpuDirty & TextureTypeBits::Roughness)
            {
				m_gpuDirty &= ~(TextureTypeBits::Roughness);
                change = true;
                if (hasRoughness())
                {
                    m_roughnessSrv = createTexture(m_roughnessKey, device, m_roughness.get());

                    m_clusterMaterial.roughness = device.modelResources().addMaterial(m_roughnessKey, m_roughness.get());
                }
                else
                    m_roughnessSrv = {};
            }

			if (m_gpuDirty & TextureTypeBits::Normal)
            {
				m_gpuDirty &= ~(TextureTypeBits::Normal);
                change = true;
                if (hasNormal())
                {
                    m_normalSrv = createTexture(m_normalKey, device, m_normal.get());

                    m_clusterMaterial.normal = device.modelResources().addMaterial(m_normalKey, m_normal.get());
                }
                else
                    m_normalSrv = {};
            }

			if (m_gpuDirty & TextureTypeBits::Metalness)
            {
				m_gpuDirty &= ~(TextureTypeBits::Metalness);
                change = true;
                if (hasMetalness())
                {
                    m_metalnessSrv = createTexture(m_metalnessKey, device, m_metalness.get());

                    m_clusterMaterial.metalness = device.modelResources().addMaterial(m_metalnessKey, m_metalness.get());
                }
                else
                    m_metalnessSrv = {};
            }

			if (m_gpuDirty & TextureTypeBits::Occlusion)
            {
				m_gpuDirty &= ~(TextureTypeBits::Occlusion);
                change = true;
                if (hasOcclusion())
                {
                    m_occlusionSrv = createTexture(m_occlusionKey, device, m_occlusion.get());

                    m_clusterMaterial.ao = device.modelResources().addMaterial(m_occlusionKey, m_occlusion.get());
                }
                else
                    m_occlusionSrv = {};
            }

			if (change || m_materialDirty)
			{
				m_materialDirty = false;
				auto mesh = getComponent<MeshRendererComponent>();
				if (mesh)
				{
					grabMaterialParameters();
					device.modelResources().updateSubmeshMaterial(*mesh->meshBuffer().modelAllocations, m_clusterMaterial);
				}
			}

            return change;
        }
    };
}
