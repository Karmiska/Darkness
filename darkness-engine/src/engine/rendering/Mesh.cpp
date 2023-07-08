#include "engine/rendering/Mesh.h"
#include "tools/Debug.h"

using namespace engine;

namespace engine
{
    MeshVersion getMeshVersion(const engine::string& filename)
    {
        CompressedFile file;
        file.open(filename, std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            MeshVersion version;
            file.read(reinterpret_cast<char*>(&version), sizeof(MeshVersion));
            file.close();
            return version;
        }
        return MeshVersion{};
    }

    Mesh::Mesh()
    {
    }

    Mesh::Mesh(ResidencyManagerV2& residency, ModelResources& modelResources, const string& filename)
        : m_filename{ filename }
    {
        if (getMeshVersion(filename) == SupportedMeshVersion)
        {
            load(residency, modelResources);
        }
    }

    void Mesh::load(ResidencyManagerV2& residency, ModelResources& modelResources)
    {
        CompressedFile file;
        file.open(m_filename, std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            MeshVersion version;
            file.read(reinterpret_cast<char*>(&version), sizeof(MeshVersion));

            while (!file.eof())
            {
                m_subMeshes.emplace_back(SubMesh());
                if (!m_subMeshes.back().load(residency, modelResources, file))
                {
                    m_subMeshes.pop_back();
                }
            }

            file.close();
        }
    }

    void Mesh::save()
    {
        CompressedFile file;
        file.open(m_filename, std::ios::out | std::ios::binary);
        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(&SupportedMeshVersion), sizeof(MeshVersion));
            for (const auto& mesh : m_subMeshes)
            {
                mesh.save(file);
            }

            file.close();
        }
    }

    void Mesh::saveToMemory(engine::vector<char>& mem)
    {
        CompressedFile file;
        file.open(mem, std::ios::out | std::ios::binary);
        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(&SupportedMeshVersion), sizeof(MeshVersion));
            for (const auto& mesh : m_subMeshes)
            {
                mesh.save(file);
            }

            file.close();
        }
    }

    void Mesh::setFilename(const engine::string& filename)
    {
        m_filename = filename;
    }

    engine::vector<SubMesh>& Mesh::subMeshes()
    {
        return m_subMeshes;
    }

    const engine::vector<SubMesh>& Mesh::subMeshes() const
    {
        return m_subMeshes;
    }

}
