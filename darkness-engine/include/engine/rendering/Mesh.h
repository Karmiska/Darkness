#pragma once

#include "tools/CompressedFile.h"
#include "SubMesh.h"
#include "containers/vector.h"
#include "containers/string.h"

namespace engine
{
    typedef uint32_t MeshVersion;
    MeshVersion getMeshVersion(const engine::string& filename);

	class ResidencyManagerV2;

    class Mesh
    {
    public:
        Mesh();
        Mesh(ResidencyManagerV2& residency, ModelResources& modelResources, const engine::string& filename);
        static const MeshVersion SupportedMeshVersion{ 1 };

        void setFilename(const engine::string& filename);
        void save();
        void saveToMemory(engine::vector<char>& mem);
        void load(ResidencyManagerV2& residency, ModelResources& modelResources);
        engine::vector<SubMesh>& subMeshes();
        const engine::vector<SubMesh>& subMeshes() const;
    private:
        engine::string m_filename;
        engine::vector<SubMesh> m_subMeshes;
    };
}
