#include "ModelTask.h"
#include "platform/Platform.h"
#include "protocols/network/ResourceProtocols.pb.h"
#include "engine/rendering/Material.h"
#include "tools/PathTools.h"
#include "tools/MeshTools.h"
#include "tools/Clusterize.h"
#include "engine/primitives/Quaternion.h"
#include "engine/primitives/Vector3.h"
#include "engine/rendering/SubMesh.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/ModelCpu.h"
#include "engine/rendering/BufferSettings.h"
#include "engine/Scene.h"
#include "engine/network/MqMessage.h"
#include "shaders/core/shared_types/VertexScale.hlsli"

#include "engine/rendering/ShapeMeshFactory.h"

#include "tools/Debug.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "tootlelib.h"
#include "Simplygon.h"

#include "containers/memory.h"
#include "containers/string.h"

#include <fstream>

#include "fbxsdk.h"

#undef GENERATE_LODS

using namespace engine;
using namespace Assimp;

// all this for creating small random alpha numeric strings
#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <limits>
#include <random>
#include <string>

template <typename T = std::mt19937>
auto random_generator() -> T {
    auto constexpr seed_bits = sizeof(typename T::result_type) * T::state_size;
    auto constexpr seed_len = seed_bits / std::numeric_limits<std::seed_seq::result_type>::digits;
    auto seed = std::array<std::seed_seq::result_type, seed_len>{};
    auto dev = std::random_device{};
    std::generate_n(begin(seed), seed_len, std::ref(dev));
    auto seed_seq = std::seed_seq(begin(seed), end(seed));
    return T{ seed_seq };
}

auto generate_random_alphanumeric_string(std::size_t len) -> std::string {
    static constexpr auto chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    thread_local auto rng = random_generator<>();
    auto dist = std::uniform_int_distribution{ {}, std::strlen(chars) - 1 };
    auto result = std::string(len, '\0');
    std::generate_n(begin(result), len, [&]() { return chars[dist(rng)]; });
    return result;
}
// -----------------

namespace resource_task
{
    TextureMapping mapping(aiTextureMapping map)
    {
        switch (map)
        {
        case aiTextureMapping_UV: return TextureMapping::UV;
        case aiTextureMapping_SPHERE: return TextureMapping::Sphere;
        case aiTextureMapping_CYLINDER: return TextureMapping::Cylinder;
        case aiTextureMapping_BOX: return TextureMapping::Box;
        case aiTextureMapping_PLANE: return TextureMapping::Plane;
        case aiTextureMapping_OTHER: return TextureMapping::UV;
        }
        return TextureMapping::UV;
    }

    TextureMapMode mappingMode(aiTextureMapMode mode)
    {
        switch (mode)
        {
        case aiTextureMapMode_Wrap: return TextureMapMode::Wrap;
        case aiTextureMapMode_Clamp: return TextureMapMode::Clamp;
        case aiTextureMapMode_Decal: return TextureMapMode::Decal;
        case aiTextureMapMode_Mirror: return TextureMapMode::Mirror;
        }
        return TextureMapMode::Wrap;
    }

    TextureOp textureOp(aiTextureOp op)
    {
        switch (op)
        {
        case aiTextureOp_Multiply: return TextureOp::Multiply;
        case aiTextureOp_Add: return TextureOp::Add;
        case aiTextureOp_Subtract: return TextureOp::Subtract;
        case aiTextureOp_Divide: return TextureOp::Divide;
        case aiTextureOp_SmoothAdd: return TextureOp::SmoothAdd;
        case aiTextureOp_SignedAdd: return TextureOp::SignedAdd;
        }
        return TextureOp::Multiply;
    }

    TextureType textureType(aiTextureType type)
    {
        switch (type)
        {
        case aiTextureType_DIFFUSE: return TextureType::Albedo;
        case aiTextureType_SPECULAR: return TextureType::Roughness;
        case aiTextureType_AMBIENT: return TextureType::Ambient;
        case aiTextureType_EMISSIVE: return TextureType::Emissive;
        case aiTextureType_HEIGHT: return TextureType::Height;
        case aiTextureType_NORMALS: return TextureType::Normal;
        case aiTextureType_SHININESS: return TextureType::Shininess;
        case aiTextureType_OPACITY: return TextureType::Opacity;
        case aiTextureType_DISPLACEMENT: return TextureType::Displacement;
        case aiTextureType_LIGHTMAP: return TextureType::Lightmap;
        case aiTextureType_REFLECTION: return TextureType::Reflection;
        default: return TextureType::Albedo;
        }
    }

    bool findTexture(engine::vector<engine::MaterialTexture> list, engine::TextureType type, engine::MaterialTexture& texture)
    {
        for (auto&& tex : list)
        {
            if (tex.type == type)
            {
                texture = tex;
                return true;
            }
        }
        return false;
    }

    engine::string resolveMaterialPath(const engine::string& modelPath, const engine::string& texturePath)
    {
        if (texturePath == "")
            return "";

        auto modelRootPath = pathExtractFolder(modelPath, true);
        auto textureFilename = pathExtractFilename(texturePath);

        // check if the texture is in the same folder as the model
        if (pathExists(pathJoin(modelRootPath, textureFilename)))
        {
            return pathJoin(modelRootPath, textureFilename);
        }

        // check if the texture path is relative to the model
        if (pathExists(pathJoin(modelRootPath, texturePath)))
        {
            return pathJoin(modelRootPath, texturePath);
        }

        // check if the texture path is an absolute path
        if (pathExists(texturePath))
        {
            return texturePath;
        }

        // this is a small hack. allthough if we didn't find the texture anyway
        // it doesn't hurt to try
        if (pathExists(pathJoin(modelRootPath, pathReplaceExtension(textureFilename, "png"))))
        {
            return pathJoin(modelRootPath, pathReplaceExtension(textureFilename, "png"));
        }

        // lets do a recursive search for the file
        auto allFiles = getAllFilesRecursive(modelRootPath);
        for (auto&& rpath : allFiles)
        {
            auto rpathFilename = pathExtractFilename(rpath);
            if (rpathFilename == textureFilename)
            {
                return rpath;
            }
        }

        // lets do a recursive search for the file with Hacked extension
        for (auto&& rpath : allFiles)
        {
            auto rpathFilename = pathExtractFilename(rpath);
            if (rpathFilename == pathReplaceExtension(textureFilename, "png"))
            {
                return rpath;
            }
        }

        // EVEN MORE DESPERATE
        // lets do a recursive search for the file
        auto noExtFilename = pathExtractFilenameWithoutExtension(textureFilename);
        for (auto&& rpath : allFiles)
        {
            if (rpath.find(noExtFilename) != engine::string::npos)
            {
                return rpath;
            }
        }

        return "";
    }

    void createScene(
        engine::Mesh& mesh,
        engine::unordered_map<uint32_t, engine::vector<uint32_t>>& meshSplitMap,
        const engine::Vector3f& importScale,
        const engine::Quaternionf& importRotation,
        const engine::string& destinationPath,
        aiNode* node,
		engine::shared_ptr<SceneNode> parent,
        aiMatrix4x4 transform = aiMatrix4x4())
    {
        engine::string dstPath = destinationPath;
        engine::string nodeName = node->mName.C_Str();

        aiMatrix4x4 nodeTransform = node->mTransformation * transform;
        aiVector3t<float> scale;
        aiVector3t<float> position;
        aiQuaterniont<float> rotation;
        nodeTransform.Decompose(scale, rotation, position);

		engine::shared_ptr<SceneNode> thisNode = nullptr;
        if (//(nodeName.find("$Assimp") == engine::string::npos) &&
            (nodeName.find("RootNode") == engine::string::npos))
        {
            thisNode = engine::make_shared<SceneNode>();
            thisNode->name(nodeName);
            parent->addChild(thisNode);

            auto transformComponent = engine::make_shared<Transform>();
            transformComponent->position(Vector3f{ position.x, position.y, position.z });
            transformComponent->scale(Vector3f{ scale.x, scale.y, scale.z } * importScale);
            transformComponent->rotation(Quaternionf{ rotation.x, rotation.y, rotation.z, rotation.w } * importRotation);
            thisNode->addComponent(transformComponent);
        }
        else
            thisNode = parent;

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            unsigned int meshId = node->mMeshes[i];

            auto& idMap = meshSplitMap[meshId];
            for (auto& splitMesh : idMap)
            {
				engine::shared_ptr<SceneNode> meshNode = engine::make_shared<SceneNode>();
                meshNode->name(node->mName.C_Str());
                thisNode->addChild(meshNode);

                auto transformComponent = engine::make_shared<Transform>();
                meshNode->addComponent(transformComponent);

                auto meshRenderer = engine::make_shared<MeshRendererComponent>(dstPath, splitMesh);
                meshNode->addComponent(meshRenderer);

                engine::Material& mat = mesh.subMeshes()[splitMesh].out_material;

                engine::MaterialTexture albedo;
                bool hasAlbedo = findTexture(mat.textures, TextureType::Albedo, albedo);

                engine::MaterialTexture roughness;
                bool hasRoughness = findTexture(mat.textures, TextureType::Roughness, roughness);

                engine::MaterialTexture normal;
                bool hasNormal = findTexture(mat.textures, TextureType::Normal, normal);

                engine::MaterialTexture metalness;
                bool hasMetalness = findTexture(mat.textures, TextureType::Metalness, metalness);

                engine::MaterialTexture occlusion;
                bool hasOcclusion = findTexture(mat.textures, TextureType::Occlusion, occlusion);

                auto albedoPath = hasAlbedo ? resolveMaterialPath(dstPath, albedo.filePath) : "";
                auto roughnessPath = hasRoughness ? resolveMaterialPath(dstPath, roughness.filePath) : "";
                auto normalPath = hasNormal ? resolveMaterialPath(dstPath, normal.filePath) : "";
                auto metalnessPath = hasMetalness ? resolveMaterialPath(dstPath, metalness.filePath) : "";
                auto occlusionPath = hasOcclusion ? resolveMaterialPath(dstPath, occlusion.filePath) : "";

                auto material = engine::make_shared<MaterialComponent>(
                    hasAlbedo ? resolveMaterialPath(dstPath, albedo.filePath) : "",
                    hasRoughness ? resolveMaterialPath(dstPath, roughness.filePath) : "",
                    hasNormal ? resolveMaterialPath(dstPath, normal.filePath) : "",
                    hasMetalness ? resolveMaterialPath(dstPath, metalness.filePath) : "",
                    hasOcclusion ? resolveMaterialPath(dstPath, occlusion.filePath) : "",
                    hasAlbedo ? albedo.uvIndex : 0,
                    hasRoughness ? roughness.uvIndex : 0,
                    hasNormal ? normal.uvIndex : 0,
                    hasMetalness ? metalness.uvIndex : 0,
                    hasOcclusion ? occlusion.uvIndex : 0
                    );
                meshNode->addComponent(material);
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            createScene(mesh, meshSplitMap, importScale, importRotation, destinationPath, node->mChildren[i], thisNode, nodeTransform);
        }
    }

    void updateProgress(
        const engine::string& hostId,
        const engine::string& taskId,
        zmq::socket_t* socket,
        float progress,
        const engine::string& message)
    {
        ProcessorTaskMessageType type;
        type.set_type(ProcessorTaskMessageType::TaskProgressMessage);
        engine::vector<char> type_message(type.ByteSizeLong());
        if (type_message.size() > 0)
            type.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

        ProcessorTaskMessageProgress prog;
        prog.set_taskid(taskId.c_str());
        prog.set_progress(progress);
        prog.set_message(message.c_str());
        engine::vector<char> progData(prog.ByteSizeLong());
        if (progData.size() > 0)
            prog.SerializeToArray(&progData[0], static_cast<int>(progData.size()));

        MqMessage msg;
        msg.emplace_back(hostId);
        msg.emplace_back("");
        msg.emplace_back(std::move(type_message));
        msg.emplace_back(std::move(progData));
        msg.send(*socket);
    }

    void updateLocalProgress(
        const engine::string& /*hostId*/,
        const engine::string& /*taskId*/,
        zmq::socket_t* /*socket*/,
        float progress,
        const engine::string& message)
    {
        LOG("Message: %s, Progress: %f", message.c_str(), progress);
    }

    void ModelTask::process(
        const engine::string& srcFile,
        const engine::string& dstFile)
    {
        engine::vector<char> srcBuffer;

        std::ifstream src;
        src.open(srcFile.c_str(), std::ios::binary | std::ios::in);
        if (src.is_open())
        {
            src.seekg(0, std::ios::end);
            srcBuffer.resize(src.tellg());
            src.seekg(0, std::ios::beg);
            src.read(&srcBuffer[0], srcBuffer.size());
            src.close();
        }

        auto filename = pathExtractFilename(srcFile);
        auto ext = pathExtractExtension(filename);
        auto plainFilename = filename.substr(0, filename.length() - ext.length() - 1);

        Quaternionf rotation;

        auto modelData = privateProcess(
            srcBuffer.data(),
            srcBuffer.size(),
            "",
            "",
            plainFilename,
            dstFile,
            1.0f,
            1.0f,
            1.0f,
            rotation.x,
            rotation.y,
            rotation.z,
            rotation.w,
            nullptr,
            updateLocalProgress);

        std::ofstream out;
        out.open(dstFile.c_str(), std::ios::binary | std::ios::out);
        out.write(modelData.modelData.data(), modelData.modelData.size());
        out.close();
    }

    ProcessorTaskModelResponse ModelTask::process(
        ProcessorTaskModelRequest& request,
        const engine::string& hostId,
        zmq::socket_t* socket)
    {
        auto res = privateProcess(
            request.modeldata().data(),
            request.modeldata().size(),
            request.taskid().c_str(),
            hostId,
            request.assetname().c_str(),
            request.modeltargetpath().c_str(),
            request.scalex(),
            request.scaley(),
            request.scalez(),
            request.rotationx(),
            request.rotationy(),
            request.rotationz(),
            request.rotationw(),
            socket,
            updateProgress);

        ProcessorTaskModelResponse response;
        response.set_modeldata(res.modelData.data(), res.modelData.size());
        response.set_prefabdata(res.prefabData.data(), res.prefabData.size());
        response.set_taskid(request.taskid());
        return response;
    }

    class DarknessFbxStream : public FbxStream
    {
    public:
        /** Current stream state. */
        /*enum EState
        {
            eClosed,	//!< The stream is closed.
            eOpen,		//!< The stream is open.
            eEmpty		//!< The stream is empty.
        };*/

    private:
        EState m_state;
        const char* m_bufferRead;
        char* m_bufferWrite;
        size_t m_bytes;
        mutable size_t m_position;
    public:
        DarknessFbxStream(const char* buffer, size_t bytes)
            : m_state{ eClosed }
            , m_bufferRead{ buffer }
            , m_bufferWrite{ nullptr }
            , m_bytes{ bytes }
            , m_position{ 0 }
        {}

        DarknessFbxStream(char* buffer, size_t bytes)
            : m_state{ eClosed }
            , m_bufferRead{ buffer }
            , m_bufferWrite{ buffer }
            , m_bytes{ bytes }
            , m_position{ 0 }
        {}

        /** Query the current state of the stream. */
        EState GetState() override
        {
            return m_state;
        }

        /** Open the stream.
        * \return True if successful.
        * \remark Each time the stream is open or closed, the stream position must be reset to zero. */
        bool Open(void* pStreamData) override
        {
            m_state = eOpen;
            return true;
        }

        /** Close the stream.
        * \return True if successful.
        * \remark Each time the stream is open or closed, the stream position must be reset to zero. */
        bool Close() override
        {
            m_position = 0;
            m_state = eClosed;
            return true;
        }

        /** Empties the internal data of the stream.
        * \return True if successful. */
        bool Flush() override
        {
            return true;
        }

        /** Writes a memory block.
        * \param pData Pointer to the memory block to write.
        * \param pSize Size (in bytes) of the memory block to write.
        * \return The number of bytes written in the stream. */
        size_t Write(const void* pData, FbxUInt64 pSize) override
        {
            ASSERT(m_position + pSize <= m_bytes, "Tried to write over the allocated buffer");
            ASSERT(m_bufferWrite != nullptr, "FbxStream has not been created with write");
            memcpy(m_bufferWrite + m_position, pData, pSize);
            m_position += pSize;
            return pSize;
        }

        /** Read bytes from the stream and store them in the memory block.
        * \param pData Pointer to the memory block where the read bytes are stored.
        * \param pSize Number of bytes read from the stream.
        * \return The actual number of bytes successfully read from the stream. */
        size_t Read(void* pData, FbxUInt64 pSize) const override
        {
            //ASSERT(m_position + pSize <= m_bytes, "Tried to read over the allocated buffer");
            auto bytesToRead = std::min(m_bytes - m_position, static_cast<size_t>(pSize));
            if (bytesToRead == 0)
                return 0;
            
            memcpy(pData, m_bufferRead + m_position, bytesToRead);
            m_position += bytesToRead;

            return pSize;
        }

        /** Read a string from the stream.
        * The default implementation is written in terms of Read() but does not cope with DOS line endings.
        * Subclasses may need to override this if DOS line endings are to be supported.
        * \param pBuffer Pointer to the memory block where the read bytes are stored.
        * \param pMaxSize Maximum number of bytes to be read from the stream.
        * \param pStopAtFirstWhiteSpace Stop reading when any whitespace is encountered. Otherwise read to end of line (like fgets()).
        * \return pBuffer, if successful, else NULL.
        * \remark The default implementation terminates the \e pBuffer with a null character and assumes there is enough room for it.
        * For example, a call with \e pMaxSize = 1 will fill \e pBuffer with the null character only. */
        /*char* ReadString(char* pBuffer, int pMaxSize, bool pStopAtFirstWhiteSpace = false) override
        {
            ASSERT(pMaxSize > 0, "Tried to ReadString to zero sized buffer");
            
            //space (0x20, ' ')
            //form feed (0x0c, '\f')
            //line feed (0x0a, '\n')
            //carriage return (0x0d, '\r')
            //horizontal tab (0x09, '\t')
            //vertical tab (0x0b, '\v')
            auto maxStringSize = pMaxSize - 1;

            const char whitespace[] = { 0x20, 0x0c, 0x0a, 0x0d, 0x09, 0x0b };
            const char lineEnd = '\n';
            auto isWhiteSpace = [&](char character)->bool
            {
                for (auto&& space : whitespace)
                {
                    if (character == space)
                        return true;
                }
                return false;
            };

            
            for (int i = 0; i < maxStringSize; ++i)
            {
                if (m_position >= m_bytes)
                {
                    pBuffer[i] = '\0';
                    return pBuffer;
                }

                auto newChar = *(m_bufferRead + m_position);
                ++m_position;

                if ((pStopAtFirstWhiteSpace && isWhiteSpace(newChar)) || (newChar == lineEnd))
                {
                    pBuffer[i] = '\0';
                    return pBuffer;
                }

                pBuffer[i] = newChar;
            }
            pBuffer[maxStringSize] = '\0';
            return pBuffer;
        }*/

        /** If not specified by KFbxImporter::Initialize(), the importer will ask
        * the stream to select an appropriate reader ID to associate with the stream.
        * FbxIOPluginRegistry can be used to locate id by extension or description.
        * Return -1 to allow FBX to select an appropriate default. */
        int GetReaderID() const override
        {
            return -1;
        }

        /** If not specified by KFbxExporter::Initialize(), the exporter will ask
        * the stream to select an appropriate writer ID to associate with the stream.
        * KFbxIOPluginRegistry can be used to locate id by extension or description.
        * Return -1 to allow FBX to select an appropriate default. */
        int GetWriterID() const override
        {
            return -1;
        }

        /** Adjust the current stream position.
        * \param pSeekPos Pre-defined position where offset is added (FbxFile::eBegin, FbxFile::eCurrent:, FbxFile::eEnd)
        * \param pOffset Number of bytes to offset from pSeekPos. */
        void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos) override
        {
            switch (pSeekPos)
            {
                case FbxFile::ESeekPos::eBegin:
                {
                    m_position = pOffset;
                    break;
                }
                case FbxFile::ESeekPos::eCurrent:
                {
                    m_position += pOffset;
                    break;
                }
                case FbxFile::ESeekPos::eEnd:
                {
                    m_position = m_bytes - pOffset;
                    break;
                }
            }
        }

        /** Get the current stream position.
        * \return Current number of bytes from the beginning of the stream. */
        FbxInt64 GetPosition() const override
        {
            return m_position;
        }

        /** Set the current stream position.
        * \param pPosition Number of bytes from the beginning of the stream to seek to. */
        void SetPosition(FbxInt64 pPosition) override
        {
            m_position = pPosition;
        }

        /** Return 0 if no errors occurred. Otherwise, return 1 to indicate
        * an error. This method will be invoked whenever FBX needs to verify
        * that the last operation succeeded. */
        int GetError() const override
        {
            return 0;
        }

        /** Clear current error condition by setting the current error value to 0. */
        void ClearError() override
        {
        }
    };

    template<typename T>
    fbxsdk::FbxVector4 getMappedElement(T* element, int vertexId)
    {
        switch(element->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint:
            {
                switch (element->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    {
                        return element->GetDirectArray().GetAt(vertexId);
                    }
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        auto id = element->GetIndexArray().GetAt(vertexId);
                        return element->GetDirectArray().GetAt(id);
                    }
                    default:
                    {
                        ASSERT(false, "Unsupported reference mode");
                        break;
                    }
                }
            }
            case FbxGeometryElement::eByPolygonVertex:
            {
                switch (element->GetReferenceMode())
                {
                    case FbxGeometryElement::eDirect:
                    {
                        return element->GetDirectArray().GetAt(vertexId);
                    }
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        auto id = element->GetIndexArray().GetAt(vertexId);
                        return element->GetDirectArray().GetAt(id);
                    }
                    default:
                    {
                        ASSERT(false, "Unsupported reference mode");
                        break;
                    }
                }
            }
        }
        ASSERT(false, "Unsupported mapping mode");
        return {};
    }

    template<typename T>
    fbxsdk::FbxColor getMappedColorElement(T* element, int vertexId)
    {
        if (element->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
        {
            switch (element->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                return element->GetDirectArray().GetAt(vertexId);
            }
            case FbxGeometryElement::eIndexToDirect:
            {
                auto id = element->GetIndexArray().GetAt(vertexId);
                return element->GetDirectArray().GetAt(id);
            }
            default:
            {
                ASSERT(false, "Unsupported reference mode");
                break;
            }
            }
        }
        ASSERT(false, "Unsupported mapping mode");
        return {};
    }

    engine::vector<int> hasMultiMaterials(const FbxMesh* pMesh)
    {
        engine::vector<int> multimesh;
        const int lPolygonCount = pMesh->GetPolygonCount();

        FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
        FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
        if (pMesh->GetElementMaterial())
        {
            lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
            lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
            if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
            {
                FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
                if (lMaterialIndice->GetCount() == lPolygonCount)
                {
                    // Count the faces of each material
                    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                    {
                        const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
                        if (multimesh.size() < lMaterialIndex + 1)
                        {
                            multimesh.resize(lMaterialIndex + 1);
                        }
                        multimesh[lMaterialIndex] += 1;
                    }

                    // Record the offset (how many vertex)
                    /*const int lMaterialCount = mSubMeshes.GetCount();
                    int lOffset = 0;
                    for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                    {
                        mSubMeshes[lIndex]->IndexOffset = lOffset;
                        lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
                        // This will be used as counter in the following procedures, reset to zero
                        mSubMeshes[lIndex]->TriangleCount = 0;
                    }
                    FBX_ASSERT(lOffset == lPolygonCount * 3);*/
                }
            }
        }
        return multimesh;
    }

    engine::SubMesh processMesh(FbxMesh* mesh)
    {
#if 1
        bool res = mesh->GenerateTangentsDataForAllUVSets();
        if (!res)
            res = mesh->GetElementTangentCount() > 0;
        ASSERT(res, "Could not generate tangents");

        struct MultiMeshContainer
        {
            int triangleCount;
            int indexOffset;
        };
        engine::vector<MultiMeshContainer> multimesh;
        const int lPolygonCount = mesh->GetPolygonCount();

        FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
        FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
        if (mesh->GetElementMaterial())
        {
            lMaterialIndice = &mesh->GetElementMaterial()->GetIndexArray();
            lMaterialMappingMode = mesh->GetElementMaterial()->GetMappingMode();
            if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
            {
                FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
                if (lMaterialIndice->GetCount() == lPolygonCount)
                {
                    // Count the faces of each material
                    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                    {
                        const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
                        if (multimesh.size() < lMaterialIndex + 1)
                        {
                            multimesh.resize(lMaterialIndex + 1);
                        }
                        multimesh[lMaterialIndex].triangleCount += 1;
                    }

                    // Record the offset (how many vertex)
                    const int lMaterialCount = multimesh.size();
                    int lOffset = 0;
                    for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                    {
                        multimesh[lIndex].indexOffset = lOffset;
                        lOffset += multimesh[lIndex].triangleCount * 3;
                        // This will be used as counter in the following procedures, reset to zero
                        multimesh[lIndex].triangleCount = 0;
                    }
                    //FBX_ASSERT(lOffset == lPolygonCount * 3);
                }
            }
        }

        if (multimesh.size() == 0)
            multimesh.resize(1);
        else if (multimesh.size() > 1)
        {
            LOG("Multiple materials for same faces not implemented yet");
            multimesh.resize(1);
        }

        bool useControlPoints = true;
        bool hasNormals = mesh->GetElementNormalCount() > 0;
        bool hasTangents = mesh->GetElementTangentCount() > 0;
        bool hasUV = mesh->GetElementUVCount() > 0;

        FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
        FbxGeometryElement::EMappingMode lTangentMappingMode = FbxGeometryElement::eNone;
        FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
        if (hasNormals)
        {
            lNormalMappingMode = mesh->GetElementNormal(0)->GetMappingMode();
            if (lNormalMappingMode == FbxGeometryElement::eNone)
            {
                hasNormals = false;
            }
            if (hasNormals && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
            {
                //useControlPoints = false;
            }
        }
        if (hasTangents)
        {
            lTangentMappingMode = mesh->GetElementTangent(0)->GetMappingMode();
            if (lTangentMappingMode == FbxGeometryElement::eNone)
            {
                hasTangents = false;
            }
            if (hasTangents && lTangentMappingMode != FbxGeometryElement::eByControlPoint)
            {
                //useControlPoints = false;
            }
        }
        if (hasUV)
        {
            lUVMappingMode = mesh->GetElementUV(0)->GetMappingMode();
            if (lUVMappingMode == FbxGeometryElement::eNone)
            {
                hasUV = false;
            }
            if (hasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
            {
                //useControlPoints = false;
            }
        }

        int polygonVertexCount = mesh->GetControlPointsCount();
        const int polygonCount = mesh->GetPolygonCount();
        const int triangleVertexCount = 3;

        if (!useControlPoints)
        {
            polygonVertexCount = polygonCount * triangleVertexCount;
        }

        engine::ModelCpu model;
        model.vertex.resize(polygonVertexCount);

        //if (useControlPoints)
        {
            model.index.resize(polygonCount * triangleVertexCount);
        }

        if (hasNormals)
            model.normal.resize(polygonVertexCount);

        if (hasTangents)
            model.tangent.resize(polygonVertexCount);

        engine::string lUVName;
        if (hasUV)
        {
            FbxStringList lUVNames;
            mesh->GetUVSetNames(lUVNames);
            auto uvSetCount = lUVNames.GetCount();
            model.uv.resize(uvSetCount);
            for (auto&& uvSet : model.uv)
            {
                uvSet.resize(polygonVertexCount);
            }
            lUVName = lUVNames[0];
        }

        engine::Vector3f minVertex{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        engine::Vector3f maxVertex{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

        const FbxVector4* lControlPoints = mesh->GetControlPoints();
        FbxVector4 lCurrentVertex;
        FbxVector4 lCurrentNormal;
        FbxVector4 lCurrentTangent;
        FbxVector2 lCurrentUV;
        if (useControlPoints)
        {
            const FbxGeometryElementNormal* lNormalElement = NULL;
            const FbxGeometryElementTangent* lTangentElement = NULL;
            const FbxGeometryElementUV* lUVElement = NULL;
            if (hasNormals)
            {
                lNormalElement = mesh->GetElementNormal(0);
            }
            if (hasTangents)
            {
                lTangentElement = mesh->GetElementTangent(0);
            }
            if (hasUV)
            {
                lUVElement = mesh->GetElementUV(0);
            }
            for (int lIndex = 0; lIndex < polygonVertexCount; ++lIndex)
            {
                // Save the vertex position.
                lCurrentVertex = lControlPoints[lIndex];
                model.vertex[lIndex] = {
                    static_cast<float>(lCurrentVertex[0]),
                    static_cast<float>(lCurrentVertex[1]),
                    static_cast<float>(lCurrentVertex[2]) };

                if (model.vertex[lIndex].x < minVertex.x) minVertex.x = model.vertex[lIndex].x;
                if (model.vertex[lIndex].y < minVertex.y) minVertex.y = model.vertex[lIndex].y;
                if (model.vertex[lIndex].z < minVertex.z) minVertex.z = model.vertex[lIndex].z;
                if (model.vertex[lIndex].x > maxVertex.x) maxVertex.x = model.vertex[lIndex].x;
                if (model.vertex[lIndex].y > maxVertex.y) maxVertex.y = model.vertex[lIndex].y;
                if (model.vertex[lIndex].z > maxVertex.z) maxVertex.z = model.vertex[lIndex].z;

                // Save the normal.
                if (hasNormals)
                {
                    int lNormalIndex = lIndex;
                    if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                    {
                        lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                    }
                    lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                    model.normal[lIndex] = {
                        static_cast<float>(lCurrentNormal[0]),
                        static_cast<float>(lCurrentNormal[1]),
                        static_cast<float>(lCurrentNormal[2]) };
                }

                if (hasTangents)
                {
                    int lTangentIndex = lIndex;
                    if (lTangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                    {
                        lTangentIndex = lTangentElement->GetIndexArray().GetAt(lIndex);
                    }
                    lCurrentTangent = lTangentElement->GetDirectArray().GetAt(lTangentIndex);
                    model.tangent[lIndex] = {
                        static_cast<float>(lCurrentTangent[0]),
                        static_cast<float>(lCurrentTangent[1]),
                        static_cast<float>(lCurrentTangent[2]) };
                }

                // Save the UV.
                if (hasUV)
                {
                    int lUVIndex = lIndex;
                    if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                    {
                        lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                    }
                    lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
                    model.uv[0][lIndex] = {
                        static_cast<float>(lCurrentUV[0]),
                        static_cast<float>(lCurrentUV[1]) };
                }
            }
        }

        
        int lVertexCount = 0;
        for (int lPolygonIndex = 0; lPolygonIndex < polygonCount; ++lPolygonIndex)
        {
            // The material for current face.
            int lMaterialIndex = 0;
            if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
            {
                lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
            }
            if (lMaterialIndex > 0)
                continue;

            // Where should I save the vertex attribute index, according to the material
            const int lIndexOffset = multimesh[lMaterialIndex].indexOffset +
                multimesh[lMaterialIndex].triangleCount * 3;
            for (int lVerticeIndex = 0; lVerticeIndex < triangleVertexCount; ++lVerticeIndex)
            {
                const int lControlPointIndex = mesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
                // If the lControlPointIndex is -1, we probably have a corrupted mesh data. At this point,
                // it is not guaranteed that the cache will work as expected.
                if (lControlPointIndex >= 0)
                {
                    if (useControlPoints)
                    {
                        model.index[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
                    }
                    // Populate the array with vertex attribute, if by polygon vertex.
                    else
                    {
                        //model.index[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);
                        //model.index[lIndexOffset + lVerticeIndex] = lControlPointIndex;
                        model.index[lIndexOffset + lVerticeIndex] = mesh->GetPolygonVertexIndex(lPolygonIndex) + lVerticeIndex;

                        lCurrentVertex = lControlPoints[lControlPointIndex];
                        model.vertex[lVertexCount] = {
                            static_cast<float>(lCurrentVertex[0]),
                            static_cast<float>(lCurrentVertex[1]),
                            static_cast<float>(lCurrentVertex[2]) };

                        if (model.vertex[lVertexCount].x < minVertex.x) minVertex.x = model.vertex[lVertexCount].x;
                        if (model.vertex[lVertexCount].y < minVertex.y) minVertex.y = model.vertex[lVertexCount].y;
                        if (model.vertex[lVertexCount].z < minVertex.z) minVertex.z = model.vertex[lVertexCount].z;
                        if (model.vertex[lVertexCount].x > maxVertex.x) maxVertex.x = model.vertex[lVertexCount].x;
                        if (model.vertex[lVertexCount].y > maxVertex.y) maxVertex.y = model.vertex[lVertexCount].y;
                        if (model.vertex[lVertexCount].z > maxVertex.z) maxVertex.z = model.vertex[lVertexCount].z;

                        if (hasNormals)
                        {
                            mesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
                            model.normal[lVertexCount] = {
                                static_cast<float>(lCurrentNormal[0]),
                                static_cast<float>(lCurrentNormal[1]),
                                static_cast<float>(lCurrentNormal[2]) };
                        }

                        if (hasTangents)
                        {
                            /*mesh->GetPolygonVertexTangent(lPolygonIndex, lVerticeIndex, lCurrentTangent);
                            model.normal[lVertexCount] = {
                                static_cast<float>(lCurrentNormal[0]),
                                static_cast<float>(lCurrentNormal[1]),
                                static_cast<float>(lCurrentNormal[2]) };*/

                            auto tangent = getMappedElement(mesh->GetElementTangent(0), lVertexCount);
                            model.tangent[lVertexCount] = {
                                static_cast<float>(tangent[0]),
                                static_cast<float>(tangent[1]),
                                static_cast<float>(tangent[2]) };
                        }

                        if (hasUV)
                        {
                            bool lUnmappedUV;
                            auto res = mesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName.c_str(), lCurrentUV, lUnmappedUV);
                            model.uv[0][lVertexCount] = {
                                static_cast<float>(lCurrentUV[0]),
                                static_cast<float>(lCurrentUV[1]) };
                        }
                    }
                }
                ++lVertexCount;
            }
            multimesh[lMaterialIndex].triangleCount += 1;
        }

        model.boundingBox.min = { minVertex.x, minVertex.y, minVertex.z };
        model.boundingBox.max = { maxVertex.x, maxVertex.y, maxVertex.z };

#if 0
        //auto cube = ShapeMeshFactory::createCube({}, { 1.0f, 1.0f, 1.0f });
        auto cube = ShapeMeshFactory::createSphere({}, 1.0f, 50, 50);
        model.boundingBox.min = { -1.0f, -1.0f, -1.0f };
        model.boundingBox.max = { 1.0f, 1.0f, 1.0f };
        model.index = cube.indexes;
        model.vertex = cube.vertices;
        model.uv.resize(1);
        model.uv[0] = cube.uvs;
        model.normal = cube.normals;
        model.tangent = cube.tangents;
#endif

        VertexScale vertexScale;
        vertexScale.range.x = maxVertex.x - minVertex.x;
        vertexScale.range.y = maxVertex.y - minVertex.y;
        vertexScale.range.z = maxVertex.z - minVertex.z;
        vertexScale.origo.x = minVertex.x;
        vertexScale.origo.y = minVertex.y;
        vertexScale.origo.z = minVertex.z;

        /*if (!useControlPoints)
        {
            auto indexes = mesh->GetPolygonVertices();
            auto count = mesh->GetPolygonVertexCount();

            // process clusters
            // TODO
            model.index.resize(count);
            for (int i = 0; i < count; ++i)
            {
                model.index[i] = *(indexes + i);
            }
        }*/

        // reduce vertexes
#if 0
#if 1
        engine::vector<Vector3f> clusterVertex;
        engine::vector<Vector3f> clusterNormal;
        engine::vector<Vector3f> clusterTangent;
        engine::vector<engine::vector<engine::Vector4f>> clusterColor;
        engine::vector<engine::vector<engine::Vector2f>> clusterUV;
        engine::vector<uint16_t> clusterIndex;
        engine::vector<uint32_t> clusterIndexStartingPoints;
        engine::vector<uint32_t> clusterVertexStartingPoints;
        engine::Clusterize clusterizer;
        clusterizer.clusterize32to16Metis(
            model.vertex,
            model.normal,
            model.tangent,
            model.color,
            model.uv,
            model.index,
            clusterVertex,
            clusterNormal,
            clusterTangent,
            clusterColor,
            clusterUV,
            clusterIndex,
            clusterIndexStartingPoints,
            clusterVertexStartingPoints);

        model.vertex = clusterVertex;
        model.normal = clusterNormal;
        model.tangent = clusterTangent;
        model.color = clusterColor;
        model.uv = clusterUV;
        model.index.clear();
        for(auto&& index : clusterIndex)
            model.index.emplace_back(index);
#else
        engine::vector<uint32_t> clusterVertexStartingPoints(roundUpToMultiple(model.index.size(), 192) / 192);
        int startPt = 0;
        for (auto&& start : clusterVertexStartingPoints)
        {
            start = startPt;
            startPt += 192;
        }
#endif
#endif
#endif

        engine::SubMesh subMesh;
        subMesh.boundingBox = model.boundingBox;
        subMesh.meshScale = vertexScale;

#if 0
        {
            engine::ModelPackedCpu outputData = packModel(model, vertexScale);

            // create submesh clusters
            {
                //onUpdateProgress(hostId, taskId,
                //    socket, progress(meshNum, scene->mNumMeshes),
                //    debugMsg(meshNum, scene->mNumMeshes, "Creating submesh clusters & BB"));

                auto clusterCount = clusterIndexStartingPoints.size();// outputData.index.size() / ClusterMaxSize;
                //auto extraIndices = outputData.index.size() - (clusterCount * ClusterMaxSize);
                //if (extraIndices > 0)
                //    ++clusterCount;

                outputData.clusterVertexStarts = clusterVertexStartingPoints;
                outputData.clusterIndexStarts = clusterIndexStartingPoints;
                outputData.clusterIndexCount.resize(clusterCount);
                if (clusterCount > 0)
                {
                    for (int i = 0; i < clusterCount - 1; ++i)
                        outputData.clusterIndexCount[i] = clusterIndexStartingPoints[i + 1] - clusterIndexStartingPoints[i];
                    outputData.clusterIndexCount[clusterCount - 1] = clusterIndex.size() - clusterIndexStartingPoints[clusterCount - 1];
                }

                outputData.clusterBounds.resize(clusterCount);

                for (int i = 0; i < clusterCount; ++i)
                {
                    //outputData.clusterId[i] = i * ClusterMaxSize;
                    //outputData.clusterIndexCount[i] = ClusterMaxSize;

                    //if (i == clusterCount - 1 && extraIndices > 0)
                    //    outputData.clusterIndexCount[i] = static_cast<uint32_t>(extraIndices);

                    engine::BoundingBox bb;
                    bb.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                    bb.max = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

                    auto vertexIndex = outputData.clusterVertexStarts[i];
                    for (unsigned int v = 0; v < outputData.clusterIndexCount[i]; ++v)
                    {
                        Vector3f pos = model.vertex[vertexIndex + outputData.index[outputData.clusterIndexStarts[i] + v]];
                        //Vector3f pos = model.vertex[outputData.index[(i * ClusterMaxSize) + v]];
                        if (pos.x < bb.min.x) bb.min.x = pos.x;
                        if (pos.y < bb.min.y) bb.min.y = pos.y;
                        if (pos.z < bb.min.z) bb.min.z = pos.z;
                        if (pos.x > bb.max.x) bb.max.x = pos.x;
                        if (pos.y > bb.max.y) bb.max.y = pos.y;
                        if (pos.z > bb.max.z) bb.max.z = pos.z;
                    }

                    outputData.clusterBounds[i] = bb;
                }
            }

            // create adjacency
            {
                //onUpdateProgress(hostId, taskId,
                //    socket, progress(meshNum, scene->mNumMeshes),
                //    debugMsg(meshNum, scene->mNumMeshes, "Generating submesh adjacency"));

                /*engine::vector<uint32_t> temporaryIndex(outputData.index.size());
                for (size_t i = 0; i < outputData.index.size(); ++i)
                {
                    temporaryIndex[i] = outputData.index[i];
                }
                outputData.adjacency = meshGenerateAdjacency(temporaryIndex, model.vertex);*/
            }

            // compute cluster culling cones
            outputData.clusterCones.resize(outputData.clusterIndexStarts.size());
            for (int cluster = 0; cluster < outputData.clusterIndexStarts.size(); ++cluster)
            {
                auto clusterFaceCount = outputData.clusterIndexCount[cluster] / 3;
                auto indexStart = outputData.clusterIndexStarts[cluster];
                auto vertexStart = outputData.clusterVertexStarts[cluster];

                engine::Vector3f clusterNormal;

                engine::vector<Vector3f> faceNormals(clusterFaceCount);
                for (uint32_t face = 0; face < clusterFaceCount; ++face)
                {
                    auto faceIndex0 = indexStart + (face * 3) + 0;
                    auto faceIndex1 = indexStart + (face * 3) + 1;
                    auto faceIndex2 = indexStart + (face * 3) + 2;
                    auto v0 = outputData.vertex[vertexStart + outputData.index[faceIndex0]];
                    auto v1 = outputData.vertex[vertexStart + outputData.index[faceIndex1]];
                    auto v2 = outputData.vertex[vertexStart + outputData.index[faceIndex2]];
                    //auto v0 = outputData.vertex[outputData.index[faceIndex0]];
                    //auto v1 = outputData.vertex[outputData.index[faceIndex1]];
                    //auto v2 = outputData.vertex[outputData.index[faceIndex2]];
                    auto unpackedVertex0 = unpackVertex(v0, vertexScale);
                    auto unpackedVertex1 = unpackVertex(v1, vertexScale);
                    auto unpackedVertex2 = unpackVertex(v2, vertexScale);
                    engine::Vector3f faceVertex[3] = { unpackedVertex0, unpackedVertex1, unpackedVertex2 };
                    engine::Vector3f faceNormal = (faceVertex[1] - faceVertex[0]).cross(faceVertex[2] - faceVertex[0]);

                    clusterNormal += faceNormal;
                    faceNormal.normalize();
                    faceNormals[face] = faceNormal;
                }
                clusterNormal.normalize();

                float coneAngle = 1.0f;
                for (auto& faceNormal : faceNormals)
                {
                    float dt = clusterNormal.dot(faceNormal);
                    coneAngle = min(dt, coneAngle);
                }
                coneAngle = coneAngle <= 0.0f ? 1.0f : sqrt(1.0f - coneAngle * coneAngle);

                outputData.clusterCones[cluster] = engine::Vector4f{ clusterNormal.x, clusterNormal.y, clusterNormal.z, coneAngle };
            }

            subMesh.outputData.emplace_back(std::move(outputData));
        }
#endif
        {
            engine::ModelPackedCpu outputData = packModel(model, vertexScale);

            // clusterize
            {
                //onUpdateProgress(hostId, taskId,
                //    socket, progress(meshNum, scene->mNumMeshes),
                //    debugMsg(meshNum, scene->mNumMeshes, "Clustering submesh"));

                engine::Clusterize clusterizer;
                auto clusterIndexes = clusterizer.clusterize(model.vertex, model.index);
                for (auto&& index : clusterIndexes)
                    outputData.index.emplace_back(index);
            }

            // create submesh clusters
            {
                //onUpdateProgress(hostId, taskId,
                //    socket, progress(meshNum, scene->mNumMeshes),
                //    debugMsg(meshNum, scene->mNumMeshes, "Creating submesh clusters & BB"));

                auto clusterCount = outputData.index.size() / ClusterMaxSize;
                auto extraIndices = outputData.index.size() - (clusterCount * ClusterMaxSize);
                if (extraIndices > 0)
                    ++clusterCount;

                outputData.clusterVertexStarts.resize(clusterCount);
                outputData.clusterIndexStarts.resize(clusterCount);
                outputData.clusterIndexCount.resize(clusterCount);
                outputData.clusterBounds.resize(clusterCount);

                for (int i = 0; i < clusterCount; ++i)
                {
                    outputData.clusterVertexStarts[i] = 0;
                    outputData.clusterIndexStarts[i] = i * ClusterMaxSize;
                    outputData.clusterIndexCount[i] = ClusterMaxSize;

                    if (i == clusterCount - 1 && extraIndices > 0)
                        outputData.clusterIndexCount[i] = static_cast<uint32_t>(extraIndices);

                    engine::BoundingBox bb;
                    bb.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                    bb.max = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

                    auto vertexIndex = outputData.clusterIndexStarts[i];
                    for (unsigned int v = 0; v < outputData.clusterIndexCount[i]; ++v)
                    {
                        Vector3f pos = model.vertex[outputData.index[vertexIndex]];
                        if (pos.x < bb.min.x) bb.min.x = pos.x;
                        if (pos.y < bb.min.y) bb.min.y = pos.y;
                        if (pos.z < bb.min.z) bb.min.z = pos.z;
                        if (pos.x > bb.max.x) bb.max.x = pos.x;
                        if (pos.y > bb.max.y) bb.max.y = pos.y;
                        if (pos.z > bb.max.z) bb.max.z = pos.z;
                        ++vertexIndex;
                    }

                    outputData.clusterBounds[i] = bb;
                }
            }

            // create adjacency
            {
                //onUpdateProgress(hostId, taskId,
                //    socket, progress(meshNum, scene->mNumMeshes),
                //    debugMsg(meshNum, scene->mNumMeshes, "Generating submesh adjacency"));

                engine::vector<uint32_t> temporaryIndex(outputData.index.size());
                for (size_t i = 0; i < outputData.index.size(); ++i)
                {
                    temporaryIndex[i] = outputData.index[i];
                }
                outputData.adjacency = meshGenerateAdjacency(temporaryIndex, model.vertex);
            }
            subMesh.outputData.emplace_back(std::move(outputData));
        }

        // compute cluster culling cones
        for (auto& pc : subMesh.outputData)
        {
            pc.clusterCones.resize(pc.clusterIndexStarts.size());
            for (int cluster = 0; cluster < pc.clusterIndexStarts.size(); ++cluster)
            {
                auto clusterFaceCount = pc.clusterIndexCount[cluster] / 3;
                auto vertexIndex = pc.clusterIndexStarts[cluster];

                engine::Vector3f clusterNormal;

                engine::vector<Vector3f> faceNormals(clusterFaceCount);
                for (uint32_t face = 0; face < clusterFaceCount; ++face)
                {
                    auto v0 = pc.vertex[pc.index[vertexIndex + 0]];
                    auto v1 = pc.vertex[pc.index[vertexIndex + 1]];
                    auto v2 = pc.vertex[pc.index[vertexIndex + 2]];
                    auto unpackedVertex0 = unpackVertex(v0, vertexScale);
                    auto unpackedVertex1 = unpackVertex(v1, vertexScale);
                    auto unpackedVertex2 = unpackVertex(v2, vertexScale);
                    engine::Vector3f faceVertex[3] = { unpackedVertex0, unpackedVertex1, unpackedVertex2 };
                    engine::Vector3f faceNormal = (faceVertex[1] - faceVertex[0]).cross(faceVertex[2] - faceVertex[0]);

                    clusterNormal += faceNormal;
                    faceNormal.normalize();
                    faceNormals[face] = faceNormal;

                    vertexIndex += 3;
                }
                clusterNormal.normalize();

                float coneAngle = 1.0f;
                for (auto& faceNormal : faceNormals)
                {
                    float dt = clusterNormal.dot(faceNormal);
                    coneAngle = std::min(dt, coneAngle);
                }
                coneAngle = coneAngle <= 0.0f ? 1.0f : sqrt(1.0f - coneAngle * coneAngle);

                pc.clusterCones[cluster] = engine::Vector4f{ clusterNormal.x, clusterNormal.y, clusterNormal.z, coneAngle };
            }
        }

        return subMesh;
    }

    void processScene(
        fbxsdk::FbxNode* node, 
        engine::Mesh& mesh, 
        engine::unordered_map<uint32_t, engine::vector<uint32_t>> meshSplitMap, 
        engine::shared_ptr<SceneNode> parent,
        const engine::Vector3f& importScale,
        const engine::Quaternionf& importRotation,
        const engine::string& destinationPath)
    {
#if 0
        FbxSkeleton* lSkeleton = (FbxSkeleton*)node->GetNodeAttribute();

        if (lSkeleton)
        {
            LOG("Skeleton Name: ", (char*)node->GetName());
            //DisplayMetaDataConnections(lSkeleton);

            const char* lSkeletonTypes[] = { "Root", "Limb", "Limb Node", "Effector" };

            LOG("    Type: ", lSkeletonTypes[lSkeleton->GetSkeletonType()]);

            if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimb)
            {
                LOG("    Limb Length: %f", lSkeleton->LimbLength.Get());
            }
            else if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode)
            {
                LOG("    Limb Node Size: %f", lSkeleton->Size.Get());
            }
            else if (lSkeleton->GetSkeletonType() == FbxSkeleton::eRoot)
            {
                LOG("    Limb Root Size: %f", lSkeleton->Size.Get());
            }
        }
#endif
        //DisplayColor("    Color: ", lSkeleton->GetLimbNodeColor());


        auto submesh = node->GetMesh();
        engine::shared_ptr<SceneNode> thisNode = parent;
        if (submesh)
        {
            FbxDouble3 position = node->LclTranslation.Get();
            FbxDouble3 rotation = node->LclRotation.Get();
            FbxDouble3 scale = node->LclScaling.Get();

            thisNode = engine::make_shared<SceneNode>();
            thisNode->name(node->GetName());
            parent->addChild(thisNode);

            auto transformComponent = engine::make_shared<Transform>();
            transformComponent->position(Vector3f{ 
                static_cast<float>(position.mData[0]), 
                static_cast<float>(position.mData[1]), 
                static_cast<float>(position.mData[2]) });
            transformComponent->scale(Vector3f{ 
                static_cast<float>(scale.mData[0]), 
                static_cast<float>(scale.mData[1]), 
                static_cast<float>(scale.mData[2]) } * importScale);
            transformComponent->rotation(Quaternionf{ 
                static_cast<float>(rotation.mData[0]), 
                static_cast<float>(rotation.mData[1]), 
                static_cast<float>(rotation.mData[2]), 1 } * importRotation);
            thisNode->addComponent(transformComponent);

            auto meshRenderer = engine::make_shared<MeshRendererComponent>(destinationPath, mesh.subMeshes().size());
            thisNode->addComponent(meshRenderer);

            auto materialCount = node->GetMaterialCount();
            if (materialCount > 0)
            {
                engine::string albedo = "";
                engine::string rougness = "";
                engine::string normal = "";
                engine::string metalness = "";
                engine::string occlusion = "";
                

                for (int i = 0; i < materialCount; ++i)
                {
                    FbxPropertyT<FbxDouble3> lKFbxDouble3;
                    FbxPropertyT<FbxDouble> lKFbxDouble1;
                    FbxColor theColor;
                    
                    auto fmaterial = node->GetMaterial(i);
                    LOG("Found material: %s", fmaterial->GetName());

                    auto getFilePath = [](const FbxProperty& pProperty)->engine::string
                    {
                        int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
                        if (lLayeredTextureCount > 0)
                        {
                            ASSERT(false, "Layered texture support missing");
#if 0
                            for (int j = 0; j < lLayeredTextureCount; ++j)
                            {
                                FbxLayeredTexture* lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
                                int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();

                                for (int k = 0; k < lNbTextures; ++k)
                                {
                                    // why are there many here and how to access them?

                                    if (k > 0) ASSERT(false, "Multiple texture support missing");
                                    
                                    auto fileTexture = static_cast<FbxFileTexture*>(lLayer);
                                    return fileTexture->GetFileName();
                                }
                                pConnectionString += "of ";
                                pConnectionString += pProperty.GetName();
                                pConnectionString += " on layer ";
                                pConnectionString += j;
                            }
                            pConnectionString += " |";
#endif
                        }
                        else
                        {
                            //no layered texture simply get on the property
                            int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();

                            for (int j = 0; j < lNbTextures; ++j)
                            {
                                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                                if (lTexture)
                                {
                                    auto fileTexture = static_cast<FbxFileTexture*>(lTexture);
                                    return fileTexture->GetFileName();
                                }
                            }
                        }
                        return "";
                    };

                    // ALBEDO
                    auto newAlbedo = getFilePath(fmaterial->FindProperty(FbxSurfaceMaterial::sDiffuse));
                    if (albedo.empty()) albedo = newAlbedo;

                    // There's no ROUGHNESS
                    //auto newRoughness = getFilePath(fmaterial->FindProperty(FbxSurfaceMaterial::sDiffuse));
                    //if (roughness.empty()) roughness = newRoughness;

                    // NORMAL
                    auto newNormal = getFilePath(fmaterial->FindProperty(FbxSurfaceMaterial::sNormalMap));
                    if (normal.empty()) normal = newNormal;

                    // OCCLUSION ?
                    auto newOcclusion = getFilePath(fmaterial->FindProperty(FbxSurfaceMaterial::sAmbient));
                    if (occlusion.empty()) occlusion = newOcclusion;

                    const FbxImplementation* lImplementation = GetImplementation(fmaterial, FBXSDK_IMPLEMENTATION_HLSL);

                    FbxString lImplemenationType = "HLSL";
                    if (!lImplementation)
                    {
                        lImplementation = GetImplementation(fmaterial, FBXSDK_IMPLEMENTATION_CGFX);
                        lImplemenationType = "CGFX";
                    }
                    if (lImplementation)
                    {
#if 1
                        //Now we have a hardware shader, let's read it
                        LOG("            Hardware Shader Type: %s\n", lImplemenationType.Buffer());
                        const FbxBindingTable* lRootTable = lImplementation->GetRootTable();
                        FbxString lFileName = lRootTable->DescAbsoluteURL.Get();
                        FbxString lTechniqueName = lRootTable->DescTAG.Get();


                        const FbxBindingTable* lTable = lImplementation->GetRootTable();
                        size_t lEntryNum = lTable->GetEntryCount();

                        for (int i = 0; i < (int)lEntryNum; ++i)
                        {
                            const FbxBindingTableEntry& lEntry = lTable->GetEntry(i);
                            const char* lEntrySrcType = lEntry.GetEntryType(true);
                            FbxProperty lFbxProp;


                            FbxString lTest = lEntry.GetSource();
                            LOG("            Entry: %s\n", lTest.Buffer());


                            if (strcmp(FbxPropertyEntryView::sEntryType, lEntrySrcType) == 0)
                            {
                                lFbxProp = fmaterial->FindPropertyHierarchical(lEntry.GetSource());
                                if (!lFbxProp.IsValid())
                                {
                                    lFbxProp = fmaterial->RootProperty.FindHierarchical(lEntry.GetSource());
                                }


                            }
                            else if (strcmp(FbxConstantEntryView::sEntryType, lEntrySrcType) == 0)
                            {
                                lFbxProp = lImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
                            }
                            if (lFbxProp.IsValid())
                            {
                                if (lFbxProp.GetSrcObjectCount<FbxTexture>() > 0)
                                {
                                    //do what you want with the textures
                                    for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
                                    {
                                        FbxFileTexture* lTex = lFbxProp.GetSrcObject<FbxFileTexture>(j);
                                        LOG("           File Texture: %s\n", lTex->GetFileName());
                                    }
                                    for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
                                    {
                                        FbxLayeredTexture* lTex = lFbxProp.GetSrcObject<FbxLayeredTexture>(j);
                                        LOG("        Layered Texture: %s\n", lTex->GetName());
                                    }
                                    for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
                                    {
                                        FbxProceduralTexture* lTex = lFbxProp.GetSrcObject<FbxProceduralTexture>(j);
                                        LOG("     Procedural Texture: %s\n", lTex->GetName());
                                    }
                                }
                                else
                                {
                                    FbxDataType lFbxType = lFbxProp.GetPropertyDataType();
                                    FbxString blah = lFbxType.GetName();
                                    if (FbxBoolDT == lFbxType)
                                    {
                                        //DisplayBool("                Bool: ", lFbxProp.Get<FbxBool>());
                                    }
                                    else if (FbxIntDT == lFbxType || FbxEnumDT == lFbxType)
                                    {
                                        //DisplayInt("                Int: ", lFbxProp.Get<FbxInt>());
                                    }
                                    else if (FbxFloatDT == lFbxType)
                                    {
                                        //DisplayDouble("                Float: ", lFbxProp.Get<FbxFloat>());

                                    }
                                    else if (FbxDoubleDT == lFbxType)
                                    {
                                        //DisplayDouble("                Double: ", lFbxProp.Get<FbxDouble>());
                                    }
                                    else if (FbxStringDT == lFbxType
                                        || FbxUrlDT == lFbxType
                                        || FbxXRefUrlDT == lFbxType)
                                    {
                                        //DisplayString("                String: ", lFbxProp.Get<FbxString>().Buffer());
                                    }
                                    else if (FbxDouble2DT == lFbxType)
                                    {
                                        FbxDouble2 lDouble2 = lFbxProp.Get<FbxDouble2>();
                                        FbxVector2 lVect;
                                        lVect[0] = lDouble2[0];
                                        lVect[1] = lDouble2[1];

                                        //Display2DVector("                2D vector: ", lVect);
                                    }
                                    else if (FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
                                    {
                                        FbxDouble3 lDouble3 = lFbxProp.Get<FbxDouble3>();


                                        FbxVector4 lVect;
                                        lVect[0] = lDouble3[0];
                                        lVect[1] = lDouble3[1];
                                        lVect[2] = lDouble3[2];
                                        //Display3DVector("                3D vector: ", lVect);
                                    }

                                    else if (FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
                                    {
                                        FbxDouble4 lDouble4 = lFbxProp.Get<FbxDouble4>();
                                        FbxVector4 lVect;
                                        lVect[0] = lDouble4[0];
                                        lVect[1] = lDouble4[1];
                                        lVect[2] = lDouble4[2];
                                        lVect[3] = lDouble4[3];
                                        //Display4DVector("                4D vector: ", lVect);
                                    }
                                    else if (FbxDouble4x4DT == lFbxType)
                                    {
                                        FbxDouble4x4 lDouble44 = lFbxProp.Get<FbxDouble4x4>();
                                        for (int j = 0; j < 4; ++j)
                                        {

                                            FbxVector4 lVect;
                                            lVect[0] = lDouble44[j][0];
                                            lVect[1] = lDouble44[j][1];
                                            lVect[2] = lDouble44[j][2];
                                            lVect[3] = lDouble44[j][3];
                                            //Display4DVector("                4x4D vector: ", lVect);
                                        }

                                    }
                                }

                            }
                        }
#endif
                    }
                    else if (fmaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
                    {
                        // We found a Phong material.  Display its properties.

#if 1
                        // Display the Ambient Color
                        lKFbxDouble3 = ((FbxSurfacePhong*)fmaterial)->Ambient;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Ambient: ", theColor);

                        // Display the Diffuse Color
                        lKFbxDouble3 = ((FbxSurfacePhong*)fmaterial)->Diffuse;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Diffuse: ", theColor);

                        // Display the Specular Color (unique to Phong materials)
                        lKFbxDouble3 = ((FbxSurfacePhong*)fmaterial)->Specular;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Specular: ", theColor);

                        // Display the Emissive Color
                        lKFbxDouble3 = ((FbxSurfacePhong*)fmaterial)->Emissive;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Emissive: ", theColor);

                        //Opacity is Transparency factor now
                        lKFbxDouble1 = ((FbxSurfacePhong*)fmaterial)->TransparencyFactor;
                        //DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());

                        // Display the Shininess
                        lKFbxDouble1 = ((FbxSurfacePhong*)fmaterial)->Shininess;
                        //DisplayDouble("            Shininess: ", lKFbxDouble1.Get());

                        // Display the Reflectivity
                        lKFbxDouble1 = ((FbxSurfacePhong*)fmaterial)->ReflectionFactor;
                        //DisplayDouble("            Reflectivity: ", lKFbxDouble1.Get());
#endif
                    }
                    else if (fmaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
                    {
#if 1
                        // We found a Lambert material. Display its properties.
                        // Display the Ambient Color
                        lKFbxDouble3 = ((FbxSurfaceLambert*)fmaterial)->Ambient;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Ambient: ", theColor);

                        // Display the Diffuse Color
                        lKFbxDouble3 = ((FbxSurfaceLambert*)fmaterial)->Diffuse;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Diffuse: ", theColor);

                        // Display the Emissive
                        lKFbxDouble3 = ((FbxSurfaceLambert*)fmaterial)->Emissive;
                        theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                        //DisplayColor("            Emissive: ", theColor);

                        // Display the Opacity
                        lKFbxDouble1 = ((FbxSurfaceLambert*)fmaterial)->TransparencyFactor;
                        //DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
#endif
                    }
                    else
                        LOG("Unknown type of Material");

                }
                
                if (!albedo.empty()) albedo = resolveMaterialPath(destinationPath, albedo);
                if(!rougness.empty()) rougness = resolveMaterialPath(destinationPath, rougness);
                if(!normal.empty()) normal = resolveMaterialPath(destinationPath, normal);
                if(!metalness.empty()) metalness = resolveMaterialPath(destinationPath, metalness);
                if(!occlusion.empty()) occlusion = resolveMaterialPath(destinationPath, occlusion);

                auto material = engine::make_shared<MaterialComponent>(
                    albedo,
                    rougness,
                    normal,
                    metalness,
                    occlusion,
                    0,
                    0,
                    0,
                    0,
                    0
                    );
                thisNode->addComponent(material);
            }
            else
            {
                auto material = engine::make_shared<MaterialComponent>(
                    "",
                    "",
                    "",
                    "",
                    "",
                    0,
                    0,
                    0,
                    0,
                    0
                    );
                thisNode->addComponent(material);
            }

            auto processedMesh = processMesh(submesh);

            mesh.subMeshes().emplace_back(processedMesh);
            auto subMeshCount = static_cast<uint32_t>(mesh.subMeshes().size());
            meshSplitMap[static_cast<uint32_t>(mesh.subMeshes().size()-1)].emplace_back(subMeshCount > 0 ? subMeshCount - 1u : 0);

            //LOG("mesh blaa: %i", processedMesh.outputData[0].index.size());
#if 0
            auto meshFaces = mesh->GetPolygonCount();
            auto controlPoints = mesh->GetControlPoints();
            auto vertexId = 0;

            engine::ModelCpu model;

            for (auto i = 0; i < meshFaces; ++i)
            {
                auto faceIndexCount = mesh->GetPolygonSize(i);
                ASSERT(faceIndexCount == 3, "We support only triangle faces");
                for (auto a = 0; a < faceIndexCount; ++a)
                {
                    auto index = mesh->GetPolygonVertex(i, a);
                    model.index.emplace_back(index);

                    auto vertex = controlPoints[index];
                    model.vertex.emplace_back(vertex);

                    for (auto l = 0; l < mesh->GetElementNormalCount(); ++l)
                    {
                        auto normal = getMappedElement(mesh->GetElementNormal(l), vertexId);
                        model.normal.emplace_back(normal);
                    }

                    for (auto l = 0; l < mesh->GetElementTangentCount(); ++l)
                    {
                        auto tangent = getMappedElement(mesh->GetElementTangent(l), vertexId);
                        model.tangent.emplace_back(tangent);
                    }

                    //for (auto l = 0; l < mesh->GetElementBinormalCount(); ++l)
                    //    auto binormal = getMappedElement(mesh->GetElementBinormal(l), vertexId);

                    for (auto l = 0; l < mesh->GetElementVertexColorCount(); ++l)
                    {
                        auto color = getMappedColorElement(mesh->GetElementVertexColor(l), vertexId);
                        model.color.emplace_back(color);
                    }

                    for (auto l = 0; l < mesh->GetElementUVCount(); ++l)
                    {
                        auto puv = mesh->GetElementUV(l);
                        switch (puv->GetMappingMode())
                        {
                            default:
                                break;
                            case FbxGeometryElement::eByControlPoint:
                            {
                                switch (puv->GetReferenceMode())
                                {
                                    case FbxGeometryElement::eDirect:
                                    {
                                        auto uv = puv->GetDirectArray().GetAt(vertexId);
                                        break;
                                    }
                                    case FbxGeometryElement::eIndexToDirect:
                                    {
                                        int id = puv->GetIndexArray().GetAt(vertexId);
                                        auto uv = puv->GetDirectArray().GetAt(id);
                                        model.uv.emplace_back(uv);
                                        break;
                                    }
                                    default:
                                        break; // other reference modes not shown here!
                                }
                                break;
                            }
                            case FbxGeometryElement::eByPolygonVertex:
                            {
                                int lTextureUVIndex = mesh->GetTextureUVIndex(i, a);
                                switch (puv->GetReferenceMode())
                                {
                                    case FbxGeometryElement::eDirect:
                                    case FbxGeometryElement::eIndexToDirect:
                                    {
                                        auto uv = puv->GetDirectArray().GetAt(lTextureUVIndex);
                                        model.uv.emplace_back(uv);
                                    }
                                    break;
                                    default:
                                        break; // other reference modes not shown here!
                                }
                            }
                            break;

                            case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                            case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                            case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                                break;
                        }
                    }

                    ++vertexId;
                }
            }
#endif
        }

        auto count = node->GetChildCount();
        for (int i = 0; i < count; ++i)
        {
            processScene(node->GetChild(i), mesh, meshSplitMap, thisNode, importScale, importRotation, destinationPath);
        }
    };

    engine::string getTemporaryPath()
    {
        return "C:\\work\\temp";
    }

    void DisplayString(const char* pHeader, const char* pValue = "", const char* pSuffix = "")
    //void DisplayString(const char* pHeader, const char* pValue /* = "" */, const char* pSuffix /* = "" */)
    {
        FbxString lString;

        lString = pHeader;
        lString += pValue;
        lString += pSuffix;
        lString += "\n";
        //LOG(lString);
        LOG("%s", lString.Buffer());
    }

    void DisplayAnimation(FbxAnimStack* pAnimStack, FbxNode* pNode, bool isSwitcher = false);
    void DisplayAnimation(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool isSwitcher = false);
    void DisplayAnimation(FbxAudioLayer* pAudioLayer, bool isSwitcher = false);

    void DisplayChannels(FbxNode* pNode, FbxAnimLayer* pAnimLayer, void (*DisplayCurve) (FbxAnimCurve* pCurve), void (*DisplayListCurve) (FbxAnimCurve* pCurve, FbxProperty* pProperty), bool isSwitcher);
    void DisplayCurveKeys(FbxAnimCurve* pCurve);
    void DisplayListCurveKeys(FbxAnimCurve* pCurve, FbxProperty* pProperty);

    void DisplayAnimation(FbxScene* pScene)
    {
        int i;
        for (i = 0; i < pScene->GetSrcObjectCount<FbxAnimStack>(); i++)
        {
            FbxAnimStack* lAnimStack = pScene->GetSrcObject<FbxAnimStack>(i);

            FbxString lOutputString = "Animation Stack Name: ";
            lOutputString += lAnimStack->GetName();
            lOutputString += "\n";
            LOG(lOutputString);

            DisplayAnimation(lAnimStack, pScene->GetRootNode());
        }
    }

    void DisplayAnimation(FbxAnimStack* pAnimStack, FbxNode* pNode, bool isSwitcher)
    {
        int l;
        int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
        int nbAudioLayers = pAnimStack->GetMemberCount<FbxAudioLayer>();
        FbxString lOutputString;

        lOutputString = "   contains ";
        if (nbAnimLayers == 0 && nbAudioLayers == 0)
            lOutputString += "no layers";

        if (nbAnimLayers)
        {
            lOutputString += nbAnimLayers;
            lOutputString += " Animation Layer";
            if (nbAnimLayers > 1)
                lOutputString += "s";
        }

        if (nbAudioLayers)
        {
            if (nbAnimLayers)
                lOutputString += " and ";

            lOutputString += nbAudioLayers;
            lOutputString += " Audio Layer";
            if (nbAudioLayers > 1)
                lOutputString += "s";
        }
        lOutputString += "\n\n";
        LOG(lOutputString);

        for (l = 0; l < nbAnimLayers; l++)
        {
            FbxAnimLayer* lAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(l);

            lOutputString = "AnimLayer ";
            lOutputString += l;
            lOutputString += "\n";
            LOG(lOutputString);

            DisplayAnimation(lAnimLayer, pNode, isSwitcher);
        }

        for (l = 0; l < nbAudioLayers; l++)
        {
            FbxAudioLayer* lAudioLayer = pAnimStack->GetMember<FbxAudioLayer>(l);

            lOutputString = "AudioLayer ";
            lOutputString += l;
            lOutputString += "\n";
            LOG(lOutputString);

            DisplayAnimation(lAudioLayer, isSwitcher);
            LOG("\n");
        }
    }

    void DisplayAnimation(FbxAudioLayer* pAudioLayer, bool)
    {
        int lClipCount;
        FbxString lOutputString;

        lClipCount = pAudioLayer->GetMemberCount<FbxAudio>();

        lOutputString = "     Name: ";
        lOutputString += pAudioLayer->GetName();
        lOutputString += "\n\n";
        lOutputString += "     Nb Audio Clips: ";
        lOutputString += lClipCount;
        lOutputString += "\n";
        LOG(lOutputString);

        for (int i = 0; i < lClipCount; i++)
        {
            FbxAudio* lClip = pAudioLayer->GetMember<FbxAudio>(i);
            lOutputString = "        Clip[";
            lOutputString += i;
            lOutputString += "]:\t";
            lOutputString += lClip->GetName();
            lOutputString += "\n";
            LOG(lOutputString);
        }
    }

    void DisplayAnimation(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool isSwitcher)
    {
        int lModelCount;
        FbxString lOutputString;

        lOutputString = "     Node Name: ";
        lOutputString += pNode->GetName();
        lOutputString += "\n\n";
        LOG(lOutputString);

        DisplayChannels(pNode, pAnimLayer, DisplayCurveKeys, DisplayListCurveKeys, isSwitcher);
        LOG("\n");

        for (lModelCount = 0; lModelCount < pNode->GetChildCount(); lModelCount++)
        {
            DisplayAnimation(pAnimLayer, pNode->GetChild(lModelCount), isSwitcher);
        }
    }

    void DisplayChannels(FbxNode* pNode, FbxAnimLayer* pAnimLayer, void (*DisplayCurve) (FbxAnimCurve* pCurve), void (*DisplayListCurve) (FbxAnimCurve* pCurve, FbxProperty* pProperty), bool isSwitcher)
    {
        FbxAnimCurve* lAnimCurve = NULL;

        // Display general curves.
        if (!isSwitcher)
        {
            lAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
            if (lAnimCurve)
            {
                LOG("        TX\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
            if (lAnimCurve)
            {
                LOG("        TY\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
            if (lAnimCurve)
            {
                LOG("        TZ\n");
                DisplayCurve(lAnimCurve);
            }

            lAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
            if (lAnimCurve)
            {
                LOG("        RX\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
            if (lAnimCurve)
            {
                LOG("        RY\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
            if (lAnimCurve)
            {
                LOG("        RZ\n");
                DisplayCurve(lAnimCurve);
            }

            lAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
            if (lAnimCurve)
            {
                LOG("        SX\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
            if (lAnimCurve)
            {
                LOG("        SY\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
            if (lAnimCurve)
            {
                LOG("        SZ\n");
                DisplayCurve(lAnimCurve);
            }
        }

        // Display curves specific to a light or marker.
        FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

        if (lNodeAttribute)
        {
            lAnimCurve = lNodeAttribute->Color.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_RED);
            if (lAnimCurve)
            {
                LOG("        Red\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = lNodeAttribute->Color.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_GREEN);
            if (lAnimCurve)
            {
                LOG("        Green\n");
                DisplayCurve(lAnimCurve);
            }
            lAnimCurve = lNodeAttribute->Color.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_BLUE);
            if (lAnimCurve)
            {
                LOG("        Blue\n");
                DisplayCurve(lAnimCurve);
            }

            // Display curves specific to a light.
            FbxLight* light = pNode->GetLight();
            if (light)
            {
                lAnimCurve = light->Intensity.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Intensity\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = light->OuterAngle.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Outer Angle\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = light->Fog.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Fog\n");
                    DisplayCurve(lAnimCurve);
                }
            }

            // Display curves specific to a camera.
            FbxCamera* camera = pNode->GetCamera();
            if (camera)
            {
                lAnimCurve = camera->FieldOfView.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Field of View\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = camera->FieldOfViewX.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Field of View X\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = camera->FieldOfViewY.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Field of View Y\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = camera->OpticalCenterX.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Optical Center X\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = camera->OpticalCenterY.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Optical Center Y\n");
                    DisplayCurve(lAnimCurve);
                }

                lAnimCurve = camera->Roll.GetCurve(pAnimLayer);
                if (lAnimCurve)
                {
                    LOG("        Roll\n");
                    DisplayCurve(lAnimCurve);
                }
            }

            // Display curves specific to a geometry.
            if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
                lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
                lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
            {
                FbxGeometry* lGeometry = (FbxGeometry*)lNodeAttribute;

                int lBlendShapeDeformerCount = lGeometry->GetDeformerCount(FbxDeformer::eBlendShape);
                for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeDeformerCount; ++lBlendShapeIndex)
                {
                    FbxBlendShape* lBlendShape = (FbxBlendShape*)lGeometry->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

                    int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
                    for (int lChannelIndex = 0; lChannelIndex < lBlendShapeChannelCount; ++lChannelIndex)
                    {
                        FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
                        const char* lChannelName = lChannel->GetName();

                        lAnimCurve = lGeometry->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer, true);
                        if (lAnimCurve)
                        {
                            LOG("        Shape %s\n", lChannelName);
                            DisplayCurve(lAnimCurve);
                        }
                    }
                }
            }
        }

        // Display curves specific to properties
        FbxProperty lProperty = pNode->GetFirstProperty();
        while (lProperty.IsValid())
        {
            if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
            {
                FbxString lFbxFCurveNodeName = lProperty.GetName();
                FbxAnimCurveNode* lCurveNode = lProperty.GetCurveNode(pAnimLayer);

                if (!lCurveNode) {
                    lProperty = pNode->GetNextProperty(lProperty);
                    continue;
                }

                FbxDataType lDataType = lProperty.GetPropertyDataType();
                if (lDataType.GetType() == eFbxBool || lDataType.GetType() == eFbxDouble || lDataType.GetType() == eFbxFloat || lDataType.GetType() == eFbxInt)
                {
                    FbxString lMessage;

                    lMessage = "        Property ";
                    lMessage += lProperty.GetName();
                    if (lProperty.GetLabel().GetLen() > 0)
                    {
                        lMessage += " (Label: ";
                        lMessage += lProperty.GetLabel();
                        lMessage += ")";
                    };

                    DisplayString(lMessage.Buffer());

                    for (int c = 0; c < lCurveNode->GetCurveCount(0U); c++)
                    {
                        lAnimCurve = lCurveNode->GetCurve(0U, c);
                        if (lAnimCurve)
                            DisplayCurve(lAnimCurve);
                    }
                }
                else if (lDataType.GetType() == eFbxDouble3 || lDataType.GetType() == eFbxDouble4 || lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT))
                {
                    char* lComponentName1 = (lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_RED : (char*)"X";
                    char* lComponentName2 = (lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_GREEN : (char*)"Y";
                    char* lComponentName3 = (lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_BLUE : (char*)"Z";
                    FbxString      lMessage;

                    lMessage = "        Property ";
                    lMessage += lProperty.GetName();
                    if (lProperty.GetLabel().GetLen() > 0)
                    {
                        lMessage += " (Label: ";
                        lMessage += lProperty.GetLabel();
                        lMessage += ")";
                    }
                    DisplayString(lMessage.Buffer());

                    for (int c = 0; c < lCurveNode->GetCurveCount(0U); c++)
                    {
                        lAnimCurve = lCurveNode->GetCurve(0U, c);
                        if (lAnimCurve)
                        {
                            DisplayString("        Component ", lComponentName1);
                            DisplayCurve(lAnimCurve);
                        }
                    }

                    for (int c = 0; c < lCurveNode->GetCurveCount(1U); c++)
                    {
                        lAnimCurve = lCurveNode->GetCurve(1U, c);
                        if (lAnimCurve)
                        {
                            DisplayString("        Component ", lComponentName2);
                            DisplayCurve(lAnimCurve);
                        }
                    }

                    for (int c = 0; c < lCurveNode->GetCurveCount(2U); c++)
                    {
                        lAnimCurve = lCurveNode->GetCurve(2U, c);
                        if (lAnimCurve)
                        {
                            DisplayString("        Component ", lComponentName3);
                            DisplayCurve(lAnimCurve);
                        }
                    }
                }
                else if (lDataType.GetType() == eFbxEnum)
                {
                    FbxString lMessage;

                    lMessage = "        Property ";
                    lMessage += lProperty.GetName();
                    if (lProperty.GetLabel().GetLen() > 0)
                    {
                        lMessage += " (Label: ";
                        lMessage += lProperty.GetLabel();
                        lMessage += ")";
                    };
                    DisplayString(lMessage.Buffer());

                    for (int c = 0; c < lCurveNode->GetCurveCount(0U); c++)
                    {
                        lAnimCurve = lCurveNode->GetCurve(0U, c);
                        if (lAnimCurve)
                            DisplayListCurve(lAnimCurve, &lProperty);
                    }
                }
            }

            lProperty = pNode->GetNextProperty(lProperty);
        } // while

    }

    static int InterpolationFlagToIndex(int flags)
    {
        if ((flags & FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant) return 1;
        if ((flags & FbxAnimCurveDef::eInterpolationLinear) == FbxAnimCurveDef::eInterpolationLinear) return 2;
        if ((flags & FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic) return 3;
        return 0;
    }

    static int ConstantmodeFlagToIndex(int flags)
    {
        if ((flags & FbxAnimCurveDef::eConstantStandard) == FbxAnimCurveDef::eConstantStandard) return 1;
        if ((flags & FbxAnimCurveDef::eConstantNext) == FbxAnimCurveDef::eConstantNext) return 2;
        return 0;
    }

    static int TangentmodeFlagToIndex(int flags)
    {
        if ((flags & FbxAnimCurveDef::eTangentAuto) == FbxAnimCurveDef::eTangentAuto) return 1;
        if ((flags & FbxAnimCurveDef::eTangentAutoBreak) == FbxAnimCurveDef::eTangentAutoBreak) return 2;
        if ((flags & FbxAnimCurveDef::eTangentTCB) == FbxAnimCurveDef::eTangentTCB) return 3;
        if ((flags & FbxAnimCurveDef::eTangentUser) == FbxAnimCurveDef::eTangentUser) return 4;
        if ((flags & FbxAnimCurveDef::eTangentGenericBreak) == FbxAnimCurveDef::eTangentGenericBreak) return 5;
        if ((flags & FbxAnimCurveDef::eTangentBreak) == FbxAnimCurveDef::eTangentBreak) return 6;
        return 0;
    }

    static int TangentweightFlagToIndex(int flags)
    {
        if ((flags & FbxAnimCurveDef::eWeightedNone) == FbxAnimCurveDef::eWeightedNone) return 1;
        if ((flags & FbxAnimCurveDef::eWeightedRight) == FbxAnimCurveDef::eWeightedRight) return 2;
        if ((flags & FbxAnimCurveDef::eWeightedNextLeft) == FbxAnimCurveDef::eWeightedNextLeft) return 3;
        return 0;
    }

    static int TangentVelocityFlagToIndex(int flags)
    {
        if ((flags & FbxAnimCurveDef::eVelocityNone) == FbxAnimCurveDef::eVelocityNone) return 1;
        if ((flags & FbxAnimCurveDef::eVelocityRight) == FbxAnimCurveDef::eVelocityRight) return 2;
        if ((flags & FbxAnimCurveDef::eVelocityNextLeft) == FbxAnimCurveDef::eVelocityNextLeft) return 3;
        return 0;
    }

    void DisplayCurveKeys(FbxAnimCurve* pCurve)
    {
        static const char* interpolation[] = { "?", "constant", "linear", "cubic" };
        static const char* constantMode[] = { "?", "Standard", "Next" };
        static const char* cubicMode[] = { "?", "Auto", "Auto break", "Tcb", "User", "Break", "User break" };
        static const char* tangentWVMode[] = { "?", "None", "Right", "Next left" };

        FbxTime   lKeyTime;
        float   lKeyValue;
        char    lTimeString[256];
        FbxString lOutputString;
        int     lCount;

        int lKeyCount = pCurve->KeyGetCount();

        for (lCount = 0; lCount < lKeyCount; lCount++)
        {
            lKeyValue = static_cast<float>(pCurve->KeyGetValue(lCount));
            lKeyTime = pCurve->KeyGetTime(lCount);

            lOutputString = "            Key Time: ";
            lOutputString += lKeyTime.GetTimeString(lTimeString, FbxUShort(256));
            lOutputString += ".... Key Value: ";
            lOutputString += lKeyValue;
            lOutputString += " [ ";
            lOutputString += interpolation[InterpolationFlagToIndex(pCurve->KeyGetInterpolation(lCount))];
            if ((pCurve->KeyGetInterpolation(lCount) & FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant)
            {
                lOutputString += " | ";
                lOutputString += constantMode[ConstantmodeFlagToIndex(pCurve->KeyGetConstantMode(lCount))];
            }
            else if ((pCurve->KeyGetInterpolation(lCount) & FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic)
            {
                lOutputString += " | ";
                lOutputString += cubicMode[TangentmodeFlagToIndex(pCurve->KeyGetTangentMode(lCount))];
                lOutputString += " | ";
                lOutputString += tangentWVMode[TangentweightFlagToIndex(pCurve->KeyGet(lCount).GetTangentWeightMode())];
                lOutputString += " | ";
                lOutputString += tangentWVMode[TangentVelocityFlagToIndex(pCurve->KeyGet(lCount).GetTangentVelocityMode())];
            }
            lOutputString += " ]";
            lOutputString += "\n";
            LOG(lOutputString);
        }
    }

    void DisplayListCurveKeys(FbxAnimCurve* pCurve, FbxProperty* pProperty)
    {
        FbxTime   lKeyTime;
        int     lKeyValue;
        char    lTimeString[256];
        FbxString lListValue;
        FbxString lOutputString;
        int     lCount;

        int lKeyCount = pCurve->KeyGetCount();

        for (lCount = 0; lCount < lKeyCount; lCount++)
        {
            lKeyValue = static_cast<int>(pCurve->KeyGetValue(lCount));
            lKeyTime = pCurve->KeyGetTime(lCount);

            lOutputString = "            Key Time: ";
            lOutputString += lKeyTime.GetTimeString(lTimeString, FbxUShort(256));
            lOutputString += ".... Key Value: ";
            lOutputString += lKeyValue;
            lOutputString += " (";
            lOutputString += pProperty->GetEnumValue(lKeyValue);
            lOutputString += ")";

            lOutputString += "\n";
            LOG(lOutputString);
        }
    }


    ModelTask::FinishedData ModelTask::privateProcess(
        const char* buffer,
        size_t bytes,
        const engine::string& taskId,
        const engine::string& hostId,
        const engine::string& assetName,
        const engine::string& modelTargetPath,
        float scaleX,
        float scaleY,
        float scaleZ,
        float rotationX,
        float rotationY,
        float rotationZ,
        float rotationW,
        zmq::socket_t* socket,
        std::function<void(
            const engine::string&,
            const engine::string&,
            zmq::socket_t*,
            float,
            const engine::string&
            )> onUpdateProgress)
    {
        onUpdateProgress(hostId, taskId, socket, 0.0f, "Reading model file");

#if 0
        // create a SdkManager
        FbxManager* lSdkManager = FbxManager::Create();

        auto pluginRegistry = lSdkManager->GetIOPluginRegistry();

        // create an IOSettings object
        FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);

        // set some IOSettings options 
        ios->SetBoolProp(IMP_FBX_MATERIAL, true);
        ios->SetBoolProp(IMP_FBX_TEXTURE, true);

        // create an empty scene
        FbxScene* lScene = FbxScene::Create(lSdkManager, "");

        // Create an importer.
        FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

        auto ext = pathExtractExtension(modelTargetPath);
        auto readerId = lSdkManager->GetIOPluginRegistry()->FindReaderIDByExtension(ext.c_str());
        ASSERT(readerId != -1, "Can't import unsupported format");

        // Initialize the importer by providing a filename and the IOSettings to use
        DarknessFbxStream fbxStream(buffer, bytes);
        bool createdTempFile = false;
        engine::string tempFilePath = "";
        if (!lImporter->Initialize(&fbxStream, nullptr, readerId))
        {
            auto status = lImporter->GetStatus();

            auto tempPath = getTemporaryPath();
            auto targetFile = pathExtractFilenameWithoutExtension(modelTargetPath);
            auto targetFileExt = pathExtractExtension(modelTargetPath);
            auto randomPart = generate_random_alphanumeric_string(10);
            tempFilePath = pathJoin(tempPath, targetFile + "_" + randomPart + "." + targetFileExt);

            ofstream tempFile;
            tempFile.open(tempFilePath, std::ios::out | std::ios::binary);
            if (tempFile.is_open())
            {
                tempFile.write(buffer, bytes);
                tempFile.close();
                createdTempFile = true;
            }

            if (!lImporter->Initialize(tempFilePath.c_str()))
            {
                if (status.Error())
                    ASSERT(false, "Fbx imported failed with: %s", status.GetErrorString());
            }
        }

        // Import the scene.
        if (!lImporter->Import(lScene))
        {
            auto status = lImporter->GetStatus();
            if (status.Error())
                ASSERT(false, "Fbx imported failed with: %s", status.GetErrorString());
        }

        // Destroy the importer.
        lImporter->Destroy();

        // triangulate scene
        FbxGeometryConverter converter(lSdkManager);
        converter.Triangulate(lScene, true);

        engine::unordered_map<uint32_t, engine::vector<uint32_t>> meshSplitMap;

        engine::shared_ptr<SceneNode> node = engine::make_shared<SceneNode>();
        {
            node->name(assetName);
            engine::shared_ptr<Transform> rootTransform = engine::make_shared<Transform>();
            node->addComponent(rootTransform);
        }

        Vector3f scale{ scaleX, scaleY, scaleZ };
        Quaternionf rotation{ rotationX, rotationY, rotationZ, rotationW };

        engine::Mesh mesh;
        mesh.setFilename("");

        // process scene
        auto rootNode = lScene->GetRootNode();
        processScene(rootNode, mesh, meshSplitMap, node, scale, rotation, modelTargetPath);

        auto poseCount = lScene->GetPoseCount();
        for (int i = 0; i < poseCount; ++i)
        {
            auto pose = lScene->GetPose(i);
            LOG("Pose name: %s. Bind pose: %s, items: %i", pose->GetName(), pose->IsBindPose() ? "true" : "false", pose->GetCount());
        }

        for (int i = 0; i < lScene->GetSrcObjectCount<FbxAnimStack>(); i++)
        {
            FbxAnimStack* lAnimStack = lScene->GetSrcObject<FbxAnimStack>(i);

            FbxString lOutputString = "Animation Stack Name: ";
            lOutputString += lAnimStack->GetName();
            lOutputString += "\n";
            LOG(lOutputString);

            DisplayAnimation(lAnimStack, lScene->GetRootNode());
        }

        FinishedData outputData;
        mesh.saveToMemory(outputData.modelData);

        {
            //Vector3f scale{ scaleX, scaleY, scaleZ };
            //Quaternionf rotation{ rotationX, rotationY, rotationZ, rotationW };

            //createScene(mesh, meshSplitMap, scale, rotation, modelTargetPath, scene->mRootNode, node);

            rapidjson::StringBuffer strBuffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strBuffer);
            node->serialize(writer);

            outputData.prefabData.resize(strBuffer.GetSize());
            memcpy(&outputData.prefabData[0], strBuffer.GetString(), outputData.prefabData.size());
        }



        lScene->Destroy(true);
        lSdkManager->Destroy();

        if (createdTempFile)
        {
            ::remove(tempFilePath.c_str());
        }

        return outputData;
#else
        




        Importer importer;
        auto scene = importer.ReadFileFromMemory(buffer, bytes,
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_GenSmoothNormals);

        if(!scene)
            return {};

        /*for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        {
            const aiMaterial* material = scene->mMaterials[i];
            auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
            for (uint32_t dc = 0; dc < diffuseCount; ++dc)
            {
                aiString path;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
                {
                    LOG("texture: %s", path.C_Str());
                }
            }
        }*/

        Simplygon simplygon;

        FinishedData finishedData;

        auto debugMsg = [](size_t meshNum, size_t numMeshes, const engine::string& msg)->engine::string
        {
            engine::string resmsg = msg;
            resmsg += " ";
            resmsg += std::to_string(meshNum + 1).c_str();
            resmsg += "/";
            resmsg += std::to_string(numMeshes).c_str();
            return resmsg;
        };
        auto progress = [](size_t meshNum, size_t numMeshes)->float
        {
            return static_cast<float>(meshNum) / static_cast<float>(numMeshes);
        };

        engine::unordered_map<uint32_t, engine::vector<uint32_t>> meshSplitMap;

        if (scene->HasMeshes())
        {
            engine::Mesh mesh;
            mesh.setFilename("");

            for (size_t meshNum = 0; meshNum < scene->mNumMeshes; ++meshNum)
            {
                // PROGRESS: START
                onUpdateProgress(hostId, taskId,
                    socket, progress(meshNum, scene->mNumMeshes),
                    debugMsg(meshNum, scene->mNumMeshes, "Processing submesh vertice"));

                auto assmesh = scene->mMeshes[meshNum];
                auto hasTangents = assmesh->HasTangentsAndBitangents();

                auto meshIndexes = assmesh->mNumFaces * 3;
                auto currentIndexCount = 0u;

                engine::Vector3f minVertex{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                engine::Vector3f maxVertex{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

                auto meshFaces = meshIndexes / 3;
                for (uint32_t i = 0; i < meshFaces; ++i)
                {
                    auto& face = *(assmesh->mFaces + i);
                    for (int a = 0; a < static_cast<int>(face.mNumIndices); a++)
                    {
                        auto thisIndex = face.mIndices[a];
                        auto& vertex = *(assmesh->mVertices + thisIndex);

                        if (vertex.x < minVertex.x) minVertex.x = vertex.x;
                        if (vertex.y < minVertex.y) minVertex.y = vertex.y;
                        if (vertex.z < minVertex.z) minVertex.z = vertex.z;
                        if (vertex.x > maxVertex.x) maxVertex.x = vertex.x;
                        if (vertex.y > maxVertex.y) maxVertex.y = vertex.y;
                        if (vertex.z > maxVertex.z) maxVertex.z = vertex.z;
                    }
                }
                VertexScale vertexScale;
                vertexScale.range.x = maxVertex.x - minVertex.x;
                vertexScale.range.y = maxVertex.y - minVertex.y;
                vertexScale.range.z = maxVertex.z - minVertex.z;
                vertexScale.origo.x = minVertex.x;
                vertexScale.origo.y = minVertex.y;
                vertexScale.origo.z = minVertex.z;

                // we actually need to clusterize the whole mesh first,
                // to make sure that indexes close to each other are within the
                // broken down index blocks of 65535
                engine::vector<uint32_t> clusterizedIndexSource;
                {
                    engine::vector<uint32_t> indexSource;
                    for (uint32_t i = 0; i < assmesh->mNumFaces; ++i)
                    {
                        auto& face = *(assmesh->mFaces + i);
                        for (uint32_t indice = 0; indice < face.mNumIndices; ++indice)
                        {
                            indexSource.emplace_back(face.mIndices[indice]);
                        }
                    }

                    engine::vector<engine::Vector3f> vertexSource;
                    for (uint32_t vert = 0; vert < assmesh->mNumVertices; ++vert)
                    {
                        auto& vertex = *(assmesh->mVertices + vert);
                        vertexSource.emplace_back(engine::Vector3f{ vertex.x, vertex.y, vertex.z });
                    }

                    engine::Clusterize clusterizer;
                    clusterizedIndexSource = clusterizer.clusterize(vertexSource, indexSource);
                }

                while (currentIndexCount < meshIndexes)
                {
                    auto indexesLeft = meshIndexes - currentIndexCount;
                    auto maxIndexesPerMesh = 65535u; // this is not a mistake. 65535 is divisible by 3 (ie. one face) where 65536 is not.

                    auto startIndex = currentIndexCount;
                    auto indexCount = indexesLeft > maxIndexesPerMesh ? maxIndexesPerMesh : indexesLeft;
                    engine::vector<engine::Vector3f> vertexBuffer;
                    engine::vector<engine::Vector3f> normalBuffer;
                    engine::vector<engine::Vector3f> tangentBuffer;
                    engine::vector<engine::vector<engine::Vector2f>> uvBuffer;
                    engine::vector<engine::vector<engine::Vector4f>> colorBuffer;
                    engine::vector<uint16_t> indexBuffer(indexCount);
                    engine::unordered_map<uint32_t, uint16_t> indexMap;
                    auto targetIndex = 0;
                    auto faceCount = indexCount / 3;
                    auto currentFaceCount = startIndex / 3;

                    uvBuffer.resize(assmesh->GetNumUVChannels());
                    colorBuffer.resize(assmesh->GetNumColorChannels());

                    for (int i = 0; i < indexCount; ++i)
                    {
                        auto thisIndex = clusterizedIndexSource[startIndex + i];
                        auto indexMapping = indexMap.find(thisIndex);
                        if (indexMapping == indexMap.end())
                        {
                            auto& vertex = *(assmesh->mVertices + thisIndex);
                            vertexBuffer.emplace_back(engine::Vector3f{ vertex.x, vertex.y, vertex.z });

                            auto& normal = *(assmesh->mNormals + thisIndex);
                            normalBuffer.emplace_back(engine::Vector3f{ normal.x, normal.y, normal.z });

                            if (hasTangents)
                            {
                                auto& tangent = *(assmesh->mTangents + thisIndex);
                                tangentBuffer.emplace_back(engine::Vector3f{ tangent.x, tangent.y, tangent.z });
                            }
                            else
                            {
                                Vector3f tempnormal = Vector3f(normalBuffer.back().x, normalBuffer.back().y, normalBuffer.back().z);
                                Vector3f tangent = tempnormal.cross(Vector3f(0.0f, 0.0f, 1.0f)).normalize();
                                tangentBuffer.emplace_back(tangent);
                            }

                            for (int uvindex = 0; uvindex < uvBuffer.size(); ++uvindex)
                            {
                                uvBuffer[uvindex].emplace_back(
                                    engine::Vector2f{
                                        assmesh->mTextureCoords[uvindex][thisIndex].x,
                                        assmesh->mTextureCoords[uvindex][thisIndex].y });
                            }


                            for (int channel = 0; channel < colorBuffer.size(); ++channel)
                            {
                                if (assmesh->HasVertexColors(channel))
                                {
                                    colorBuffer[channel].emplace_back(
                                        engine::Vector4f{
                                            assmesh->mColors[channel][thisIndex].r,
                                            assmesh->mColors[channel][thisIndex].g,
                                            assmesh->mColors[channel][thisIndex].b,
                                            assmesh->mColors[channel][thisIndex].a });
                                }
                            }

                            indexMap[thisIndex] = static_cast<uint16_t>(vertexBuffer.size() - 1);
                            indexBuffer[targetIndex] = static_cast<uint16_t>(vertexBuffer.size() - 1);
                        }
                        else
                        {
                            indexBuffer[targetIndex] = indexMapping->second;
                        }
                        ++targetIndex;
                    }
                    /*for (uint32_t i = 0; i < faceCount; ++i)
                    {
                        auto& face = *(assmesh->mFaces + currentFaceCount + i);
                        for (int a = 0; a < static_cast<int>(face.mNumIndices); a++)
                        {
                            auto thisIndex = face.mIndices[a];
                            auto indexMapping = indexMap.find(thisIndex);
                            if(indexMapping == indexMap.end())
                            {
                                auto& vertex = *(assmesh->mVertices + thisIndex);
                                vertexBuffer.emplace_back(engine::Vector3f{ vertex.x, vertex.y, vertex.z });

                                auto& normal = *(assmesh->mNormals + thisIndex);
                                normalBuffer.emplace_back(engine::Vector3f{ normal.x, normal.y, normal.z });

                                if (hasTangents)
                                {
                                    auto& tangent = *(assmesh->mTangents + thisIndex);
                                    tangentBuffer.emplace_back(engine::Vector3f{ tangent.x, tangent.y, tangent.z });
                                }
                                else
                                {
                                    Vector3f tempnormal = Vector3f(normalBuffer.back().x, normalBuffer.back().y, normalBuffer.back().z);
                                    Vector3f tangent = tempnormal.cross(Vector3f(0.0f, 0.0f, 1.0f)).normalize();
                                    tangentBuffer.emplace_back(tangent);
                                }

                                for (int uvindex = 0; uvindex < uvBuffer.size(); ++uvindex)
                                {
                                    uvBuffer[uvindex].emplace_back(
                                        engine::Vector2f{ 
                                            assmesh->mTextureCoords[uvindex][thisIndex].x,
                                            assmesh->mTextureCoords[uvindex][thisIndex].y });
                                }

                                
                                for (int channel = 0; channel < colorBuffer.size(); ++channel)
                                {
                                    if (assmesh->HasVertexColors(channel))
                                    {
                                        colorBuffer[channel].emplace_back(
                                            engine::Vector4f{
                                                assmesh->mColors[channel][thisIndex].r,
                                                assmesh->mColors[channel][thisIndex].g,
                                                assmesh->mColors[channel][thisIndex].b,
                                                assmesh->mColors[channel][thisIndex].a });
                                    }
                                }

                                indexMap[thisIndex] = static_cast<uint16_t>(vertexBuffer.size() - 1);
                                indexBuffer[targetIndex] = static_cast<uint16_t>(vertexBuffer.size() - 1);
                            }
                            else
                            {
                                indexBuffer[targetIndex] = indexMapping->second;
                            }
                            ++targetIndex;
                        }
                    }*/
                    currentIndexCount += indexCount;


                    engine::ModelCpu model;

                    model.boundingBox.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                    model.boundingBox.max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

                    // vertices
                    if (assmesh->HasFaces())
                    {
                        model.vertex.resize(vertexBuffer.size());
                        for (int vert = 0; vert < vertexBuffer.size(); ++vert)
                        {
                            model.vertex[vert][0] = vertexBuffer[vert].x;
                            model.vertex[vert][1] = vertexBuffer[vert].y;
                            model.vertex[vert][2] = vertexBuffer[vert].z;

                            if (vertexBuffer[vert].x < model.boundingBox.min.x) model.boundingBox.min.x = vertexBuffer[vert].x;
                            if (vertexBuffer[vert].y < model.boundingBox.min.y) model.boundingBox.min.y = vertexBuffer[vert].y;
                            if (vertexBuffer[vert].z < model.boundingBox.min.z) model.boundingBox.min.z = vertexBuffer[vert].z;

                            if (vertexBuffer[vert].x > model.boundingBox.max.x) model.boundingBox.max.x = vertexBuffer[vert].x;
                            if (vertexBuffer[vert].y > model.boundingBox.max.y) model.boundingBox.max.y = vertexBuffer[vert].y;
                            if (vertexBuffer[vert].z > model.boundingBox.max.z) model.boundingBox.max.z = vertexBuffer[vert].z;
                        }
                    }

                    // normals
                    if (assmesh->HasNormals())
                    {
                        model.normal = normalBuffer;
                    }

                    // tangents and bitangents
                    {
                        model.tangent = tangentBuffer;
                    }

                    // uv
                    {
                        model.uv = uvBuffer;
                    }

                    // colors
                    {
                        model.color = colorBuffer;
                    }

                    // indices 
                    {
                        for (auto&& index : indexBuffer)
                            model.index.emplace_back(index);
                    }

#ifdef GENERATE_LODS
                    uint32_t lodCount = 8;
                    // generate simplygon lod
                    auto lodLevels = simplygon.generateLod(
                        lodCount,
                        model);
#endif

                    engine::SubMesh subMesh;
                    subMesh.boundingBox = model.boundingBox;
                    subMesh.meshScale = vertexScale;

#if 1
                    {
                        engine::ModelPackedCpu outputData = packModel(model, vertexScale);

                        // clusterize
                        {
                            onUpdateProgress(hostId, taskId,
                                socket, progress(meshNum, scene->mNumMeshes),
                                debugMsg(meshNum, scene->mNumMeshes, "Clustering submesh"));

                            engine::Clusterize clusterizer;
                            auto clusterIndexes = clusterizer.clusterize(model.vertex, model.index);
                            for (auto&& index : clusterIndexes)
                                outputData.index.emplace_back(index);
                        }

                        // create submesh clusters
                        {
                            onUpdateProgress(hostId, taskId,
                                socket, progress(meshNum, scene->mNumMeshes),
                                debugMsg(meshNum, scene->mNumMeshes, "Creating submesh clusters & BB"));

                            auto clusterCount = outputData.index.size() / ClusterMaxSize;
                            auto extraIndices = outputData.index.size() - (clusterCount * ClusterMaxSize);
                            if (extraIndices > 0)
                                ++clusterCount;

                            outputData.clusterVertexStarts.resize(clusterCount);
                            outputData.clusterIndexStarts.resize(clusterCount);
                            outputData.clusterIndexCount.resize(clusterCount);
                            outputData.clusterBounds.resize(clusterCount);

                            for (int i = 0; i < clusterCount; ++i)
                            {
                                outputData.clusterVertexStarts[i] = 0;
                                outputData.clusterIndexStarts[i] = i * ClusterMaxSize;
                                outputData.clusterIndexCount[i] = ClusterMaxSize;

                                if (i == clusterCount - 1 && extraIndices > 0)
                                    outputData.clusterIndexCount[i] = static_cast<uint32_t>(extraIndices);

                                engine::BoundingBox bb;
                                bb.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                                bb.max = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

                                auto vertexIndex = outputData.clusterIndexStarts[i];
                                for (unsigned int v = 0; v < outputData.clusterIndexCount[i]; ++v)
                                {
                                    Vector3f pos = model.vertex[outputData.index[vertexIndex]];
                                    if (pos.x < bb.min.x) bb.min.x = pos.x;
                                    if (pos.y < bb.min.y) bb.min.y = pos.y;
                                    if (pos.z < bb.min.z) bb.min.z = pos.z;
                                    if (pos.x > bb.max.x) bb.max.x = pos.x;
                                    if (pos.y > bb.max.y) bb.max.y = pos.y;
                                    if (pos.z > bb.max.z) bb.max.z = pos.z;
                                    ++vertexIndex;
                                }

                                outputData.clusterBounds[i] = bb;
                            }
                        }

                        // create adjacency
                        {
                            onUpdateProgress(hostId, taskId,
                                socket, progress(meshNum, scene->mNumMeshes),
                                debugMsg(meshNum, scene->mNumMeshes, "Generating submesh adjacency"));

                            engine::vector<uint32_t> temporaryIndex(outputData.index.size());
                            for (size_t i = 0; i < outputData.index.size(); ++i)
                            {
                                temporaryIndex[i] = outputData.index[i];
                            }
                            outputData.adjacency = meshGenerateAdjacency(temporaryIndex, model.vertex);
                        }
                        subMesh.outputData.emplace_back(std::move(outputData));
                    }

#endif

#ifdef GENERATE_LODS
                    for (uint32_t currentLod = 0; currentLod < lodCount; ++currentLod)
                    {
                        auto& lod = lodLevels.lods[currentLod];

                        engine::ModelPackedCpu outputData = packModel(lod, vertexScale);

                        // clusterize
                        {
                            onUpdateProgress(hostId, taskId,
                                socket, progress(meshNum, scene->mNumMeshes),
                                debugMsg(meshNum, scene->mNumMeshes, "Clustering submesh"));

                            engine::Clusterize clusterizer;
                            outputData.index = clusterizer.clusterize(lod.vertex, lod.index);
                        }

                        // create submesh clusters
                        {
                            onUpdateProgress(hostId, taskId,
                                socket, progress(meshNum, scene->mNumMeshes),
                                debugMsg(meshNum, scene->mNumMeshes, "Creating submesh clusters & BB"));

                            auto clusterCount = outputData.index.size() / ClusterMaxSize;
                            auto extraIndices = outputData.index.size() - (clusterCount * ClusterMaxSize);
                            if (extraIndices > 0)
                                ++clusterCount;

                            outputData.clusterId.resize(clusterCount);
                            outputData.clusterIndexCount.resize(clusterCount);
                            outputData.clusterBounds.resize(clusterCount);

                            for (int i = 0; i < clusterCount; ++i)
                            {
                                outputData.clusterId[i] = i * ClusterMaxSize;
                                outputData.clusterIndexCount[i] = ClusterMaxSize;

                                if (i == clusterCount - 1 && extraIndices > 0)
                                    outputData.clusterIndexCount[i] = static_cast<uint32_t>(extraIndices);

                                engine::BoundingBox bb;
                                bb.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                                bb.max = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

                                auto vertexIndex = outputData.clusterId[i];
                                for (unsigned int v = 0; v < outputData.clusterIndexCount[i]; ++v)
                                {
                                    Vector3f pos = lod.vertex[outputData.index[vertexIndex]];
                                    if (pos.x < bb.min.x) bb.min.x = pos.x;
                                    if (pos.y < bb.min.y) bb.min.y = pos.y;
                                    if (pos.z < bb.min.z) bb.min.z = pos.z;
                                    if (pos.x > bb.max.x) bb.max.x = pos.x;
                                    if (pos.y > bb.max.y) bb.max.y = pos.y;
                                    if (pos.z > bb.max.z) bb.max.z = pos.z;
                                    ++vertexIndex;
                                }

                                outputData.clusterBounds[i] = bb;
                            }
                        }

                        // create adjacency
                        {
                            onUpdateProgress(hostId, taskId,
                                socket, progress(meshNum, scene->mNumMeshes),
                                debugMsg(meshNum, scene->mNumMeshes, "Generating submesh adjacency"));

                            engine::vector<uint32_t> temporaryIndex(outputData.index.size());
                            for (size_t i = 0; i < outputData.index.size(); ++i)
                            {
                                temporaryIndex[i] = outputData.index[i];
                            }
                            outputData.adjacency = meshGenerateAdjacency(temporaryIndex, lod.vertex);
                        }
                        subMesh.outputData.emplace_back(std::move(outputData));
                    }
#endif

                    // compute cluster culling cones
                    for (auto& pc : subMesh.outputData)
                    {
                        pc.clusterCones.resize(pc.clusterIndexStarts.size());
                        for (int cluster = 0; cluster < pc.clusterIndexStarts.size(); ++cluster)
                        {
                            auto clusterFaceCount = pc.clusterIndexCount[cluster] / 3;
                            auto vertexIndex = pc.clusterIndexStarts[cluster];

                            engine::Vector3f clusterNormal;

                            engine::vector<Vector3f> faceNormals(clusterFaceCount);
                            for (uint32_t face = 0; face < clusterFaceCount; ++face)
                            {
                                auto v0 = pc.vertex[pc.index[vertexIndex + 0]];
                                auto v1 = pc.vertex[pc.index[vertexIndex + 1]];
                                auto v2 = pc.vertex[pc.index[vertexIndex + 2]];
                                auto unpackedVertex0 = unpackVertex(v0, vertexScale);
                                auto unpackedVertex1 = unpackVertex(v1, vertexScale);
                                auto unpackedVertex2 = unpackVertex(v2, vertexScale);
                                engine::Vector3f faceVertex[3] = { unpackedVertex0, unpackedVertex1, unpackedVertex2 };
                                engine::Vector3f faceNormal = (faceVertex[1] - faceVertex[0]).cross(faceVertex[2] - faceVertex[0]);
                                
                                clusterNormal += faceNormal;
                                faceNormal.normalize();
                                faceNormals[face] = faceNormal;

                                vertexIndex += 3;
                            }
                            clusterNormal.normalize();

                            float coneAngle = 1.0f;
                            for (auto& faceNormal : faceNormals)
                            {
                                float dt = clusterNormal.dot(faceNormal);
                                coneAngle = std::min(dt, coneAngle);
                            }
                            coneAngle = coneAngle <= 0.0f ? 1.0f : sqrt(1.0f - coneAngle * coneAngle);

                            pc.clusterCones[cluster] = engine::Vector4f{ clusterNormal.x, clusterNormal.y, clusterNormal.z, coneAngle };
                        }
                    }


                    // materials
                    {
                        onUpdateProgress(hostId, taskId,
                            socket, progress(meshNum, scene->mNumMeshes),
                            debugMsg(meshNum, scene->mNumMeshes, "Parsing submesh materials"));

                        auto material = scene->mMaterials[assmesh->mMaterialIndex];

                        engine::vector<aiTextureType> textureTypes = {
                            aiTextureType_DIFFUSE,
                            aiTextureType_SPECULAR,
                            aiTextureType_AMBIENT,
                            aiTextureType_EMISSIVE,
                            aiTextureType_HEIGHT,
                            aiTextureType_NORMALS,
                            aiTextureType_SHININESS,
                            aiTextureType_OPACITY,
                            aiTextureType_DISPLACEMENT,
                            aiTextureType_LIGHTMAP,
                            aiTextureType_REFLECTION,
                            aiTextureType_UNKNOWN
                        };

                        for (auto& type : textureTypes)
                        {
                            auto count = material->GetTextureCount(type);
                            if (count != 0)
                            {
                                LOG("some type");
                            }
                        }

                        bool hadMaterial = false;
                        for (auto texType : textureTypes)
                        {
                            for (unsigned int texIndex = 0; texIndex < material->GetTextureCount(texType); ++texIndex)
                            {
                                aiString path;
                                aiTextureMapping textureMapping;
                                unsigned int uvIndex;
                                float blend;
                                aiTextureOp op;
                                aiTextureMapMode mode;
                                if (material->GetTexture(texType, texIndex, &path, &textureMapping,
                                    &uvIndex, &blend, &op, &mode) == aiReturn_SUCCESS)
                                {
                                    subMesh.out_material.textures.emplace_back(MaterialTexture{
                                        engine::string(&path.data[0], path.length),
                                        mapping(textureMapping),
                                        mappingMode(mode),
                                        textureType(texType),
                                        textureOp(op),
                                        uvIndex
                                        });
                                    hadMaterial = true;
                                    LOG("Mesh has material: %s", engine::string(&path.data[0], path.length).c_str());
                                }
                            }
                        }
                        if (!hadMaterial)
                        {
                            //qDebug() << "No material for mesh found.";
                            LOG("No material for mesh found.");
                        }
                    }

                    mesh.subMeshes().emplace_back(subMesh);
                    auto subMeshCount = static_cast<uint32_t>(mesh.subMeshes().size());
                    meshSplitMap[static_cast<uint32_t>(meshNum)].emplace_back(subMeshCount > 0 ? subMeshCount - 1u : 0);
                }
            }
            mesh.saveToMemory(finishedData.modelData);

            if (scene->mRootNode)
            {
				engine::shared_ptr<SceneNode> node = engine::make_shared<SceneNode>();
                node->name(assetName);

                engine::shared_ptr<Transform> rootTransform = engine::make_shared<Transform>();
                node->addComponent(rootTransform);

                Vector3f scale{ scaleX, scaleY, scaleZ };
                Quaternionf rotation{ rotationX, rotationY, rotationZ, rotationW };

                Matrix4f transform;
                createScene(mesh, meshSplitMap, scale, rotation, modelTargetPath, scene->mRootNode, node);

                rapidjson::StringBuffer strBuffer;
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strBuffer);
                node->serialize(writer);

                finishedData.prefabData.resize(strBuffer.GetSize());
                memcpy(&finishedData.prefabData[0], strBuffer.GetString(), finishedData.prefabData.size());
            }
        }

        return finishedData;
#endif
    }
}
