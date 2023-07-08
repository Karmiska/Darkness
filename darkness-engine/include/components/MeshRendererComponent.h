#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "tools/hash/Hash.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"
#include "engine/graphics/Common.h"
#include "engine/rendering/Mesh.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Pipeline.h"
#include "engine/rendering/ModelResources.h"
#include "engine/filesystem/VirtualFilesystem.h"
#include "tools/PathTools.h"
#include "platform/File.h"
#include "containers/memory.h"
#include "containers/string.h"

namespace engine
{
    struct MeshBuffers
    {
        engine::shared_ptr<ModelResources::ModelAllocation> modelAllocations;
    };

    MeshBuffers convert(
        const engine::string& path,
        Device& device, 
        engine::shared_ptr<engine::SubMeshInstance>& source);

    class MeshRendererComponent : public EngineComponent
    {
        Property m_meshPath;
        Property m_meshIndex;
        engine::shared_ptr<engine::SubMeshInstance> m_mesh;

        MeshBuffers m_meshBuffers;
        
        bool m_cpuDirty;
        bool m_gpuDirty;
        bool m_modelsChanged;
    protected:
        engine::string m_oldMeshPath;
        void onValueChanged() override
        {
            if (m_oldMeshPath != m_meshPath.value<engine::string>())
            {
                m_oldMeshPath = m_meshPath.value<engine::string>();
                m_cpuDirty = true;
            }
        }
    public:
        MeshRendererComponent()
            : m_meshPath{ this, "MeshPath", engine::string("") }
            , m_meshIndex{ this, "MeshIndex", static_cast<int>(0), [this]() { m_gpuDirty = true; } }
            , m_cpuDirty{ false }
            , m_gpuDirty{ false }
            , m_modelsChanged{ true }
            , m_matrixUpdate{ false }
            , m_objectIdUpdate{ false }
        {
            m_name = "MeshRenderer";
        }

        MeshRendererComponent(const engine::string& meshPath, int meshIndex)
            : m_meshPath{ this, "MeshPath", meshPath }
            , m_meshIndex{ this, "MeshIndex", meshIndex }
            , m_cpuDirty{ true }
            , m_gpuDirty{ false }
            , m_modelsChanged{ true }
            , m_matrixUpdate{ false }
            , m_objectIdUpdate{ false }
        {
            m_name = "MeshRenderer";
        }

        engine::shared_ptr<engine::EngineComponent> clone() const override
        {
            return engine::make_shared<MeshRendererComponent>(m_meshPath.value<engine::string>(), m_meshIndex.value<int>());
        }

        bool modelsChanged(bool clear = false)
        {
            bool res = m_modelsChanged; 
            if (clear) 
                m_modelsChanged = false; 
            return res;
        }

        engine::string meshPath() const
        {
            return m_meshPath.value<engine::string>();
        }

        void setMeshPath(const engine::string& path)
        {
            m_cpuDirty = m_meshPath.value<engine::string>() != path;
            m_gpuDirty = m_cpuDirty;
            m_meshPath.value<engine::string>(path);
        }

        MeshBuffers& meshBuffer()
        {
            return m_meshBuffers;
        }

        /*const SubMesh& subMesh() const
        {
            return m_mesh->subMeshes()[m_meshIndex.value<int>()];
        }*/

    public:
        void invalidateGpu()
        {
            m_gpuDirty = true;
        }

    private:
        Matrix4f m_lastMatrix;
        uint32_t m_lastObjectId;
        bool m_matrixUpdate;
        bool m_objectIdUpdate;

    public:
        void cpuRefresh(Device& device)
        {
            if (m_cpuDirty)
            {
                m_cpuDirty = false;

                auto meshPathStr = resolvePath(m_meshPath.value<engine::string>());
                if (meshPathStr != "")
                {
                    bool foundFile = fileExists(meshPathStr);
                    if (foundFile)
                    {
                        m_mesh = device.createMesh(
                            tools::hash(pathClean(meshPathStr)),
                            meshPathStr,
                            static_cast<uint32_t>(m_meshIndex.value<int>()));
                        m_gpuDirty = true;
                    }
                    else
                        LOG_WARNING("Missing model file: %s", meshPathStr.c_str());
                }
            }
        }

        bool gpuRefresh(Device& device)
        {
            bool change = false;
            if(m_gpuDirty)
            {
                m_gpuDirty = false;
                change = true;
                if (m_mesh)
                {
                    m_meshBuffers = convert(
                        m_meshPath.value<engine::string>() + std::to_string(m_meshIndex.value<int>()).c_str(),
                        device,
                        m_mesh);
                }
                m_modelsChanged = true;
            }

            if (m_matrixUpdate && m_meshBuffers.modelAllocations)
            {
                m_matrixUpdate = false;
                device.modelResources().updateSubmeshTransform(*m_meshBuffers.modelAllocations, m_lastMatrix);
            }

            if (m_objectIdUpdate && m_meshBuffers.modelAllocations)
            {
                m_objectIdUpdate = false;
                device.modelResources().updateSubmeshObjectId(*m_meshBuffers.modelAllocations, m_lastObjectId);
            }

            return change;
        }

        void updateTransform(const Matrix4f& mat)
        {
            m_lastMatrix = mat;
            m_matrixUpdate = true;
        }

        void updateObjectId(uint32_t objectId)
        {
            m_lastObjectId = objectId;
            m_objectIdUpdate = true;
        }
    };
}
