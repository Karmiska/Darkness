#include "tools/Clusterize.h"
#include "tools/Debug.h"
#include <stack>
#include "flann/flann.hpp"
#include "engine/rendering/BufferSettings.h"
#include "metis.h"
#include "containers/unordered_map.h"

using namespace engine;

namespace engine
{
    engine::vector<uint16_t> Clusterize::clusterize(const vector<Vector3f>& vertex, const vector<uint16_t>& index)
    {
        ASSERT(index.size() % 3 == 0, "Invalid index count. Needs to be divisible with 3");

        generateTriangleList(vertex, index);

        auto faceCenter = [&](uint32_t face)->Vector3f
        {
            Triangle& tri = m_triangles[face];
            Vector3f aa = vertex[tri.index[0]];
            Vector3f ab = vertex[tri.index[1]];
            Vector3f ac = vertex[tri.index[2]];
            return (aa + ab + ac) / 3.0f;
        };

        engine::vector<Vector3f> centers;
        for (uint32_t i = 0; i < m_triangles.size(); ++i)
        {
            centers.emplace_back(faceCenter(i));
        }

        flann::Matrix<float> datamatrix(&centers[0], centers.size(), 3);
        flann::Index<flann::L2_3D<float>> kdtree(datamatrix, flann::KDTreeSingleIndexParams());
        kdtree.buildIndex();
        
        auto emitFace = [&](vector<uint16_t>& output, uint16_t face)
        {
            for (auto&& faceIndex : { m_triangles[face].index[0], m_triangles[face].index[1], m_triangles[face].index[2] })
            {
                output.emplace_back(static_cast<uint16_t>(faceIndex));
            }
            //m_triangles[face].emitted = true;

            auto pr = std::equal_range(std::begin(m_notEmittedTriangle), std::end(m_notEmittedTriangle), face);
            m_notEmittedTriangle.erase(pr.first, pr.second);
        };

        auto getAnyNotEmittedFace = [&]()->int
        {
            /*for (int i = 0; i < m_triangles.size(); ++i)
                if (!m_triangles[i].emitted)
                    return i;
            return -1;*/
            if (m_notEmittedTriangle.size() > 0)
                return m_notEmittedTriangle[0];
            return -1;
        };
        
        vector<uint16_t> output;
        auto face = 0;
        while (output.size() < index.size())
        {
            vector<uint16_t> cluster;

            face = getAnyNotEmittedFace();

            if (face == -1)
                break;

            float* p = kdtree.getPoint(face);
            Vector3f point;
            memcpy(&point, p, sizeof(Vector3f));
            flann::Matrix<float> searchPoint(&point, 1, 3);

            while (cluster.size() < 192)
            {
                emitFace(cluster, static_cast<uint16_t>(face));
                kdtree.removePoint(face);
                
                engine::vector<engine::vector<int>> res;
                engine::vector<engine::vector<float>> dist;
                flann::SearchParams searchParams;
                searchParams.checks = flann::FLANN_CHECKS_UNLIMITED;
                kdtree.knnSearch(searchPoint, res, dist, 1, searchParams);

                if (res[0].size() == 0)
                    break;
                face = res[0][0];
            }
            output.insert(output.end(), cluster.begin(), cluster.end());
        }
        return output;
    }

    void Clusterize::generateTriangleList(const engine::vector<Vector3f>& vertex, const engine::vector<uint16_t>& index)
    {
        m_triangles.clear();
        m_triangles.resize(index.size() / 3);
        m_notEmittedTriangle.resize(m_triangles.size());

        uint32_t triId = 0;
        m_meshBoundingBox.min = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        m_meshBoundingBox.max = Vector3f(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
        for(uint32_t i = 0; i < index.size(); i += 3, ++triId)
        {
            m_triangles[triId] = Triangle{ index[i], index[i+1], index[i+2], false };
            m_notEmittedTriangle[triId] = triId;

            for (int a = 0; a < 3; ++a)
            {
                const Vector3f& vec = vertex[index[i + a]];
                if (vec.x < m_meshBoundingBox.min.x) m_meshBoundingBox.min.x = vec.x;
                if (vec.y < m_meshBoundingBox.min.y) m_meshBoundingBox.min.y = vec.y;
                if (vec.z < m_meshBoundingBox.min.z) m_meshBoundingBox.min.z = vec.z;
                if (vec.x > m_meshBoundingBox.max.x) m_meshBoundingBox.max.x = vec.x;
                if (vec.y > m_meshBoundingBox.max.y) m_meshBoundingBox.max.y = vec.y;
                if (vec.z > m_meshBoundingBox.max.z) m_meshBoundingBox.max.z = vec.z;
            }
        }
    }

    engine::vector<uint32_t> Clusterize::clusterize(const vector<Vector3f>& vertex, const vector<uint32_t>& index)
    {
        ASSERT(index.size() % 3 == 0, "Invalid index count. Needs to be divisible with 3");

        generateTriangleList(vertex, index);

        auto faceCenter = [&](uint32_t face)->Vector3f
        {
            Triangle& tri = m_triangles[face];
            Vector3f aa = vertex[tri.index[0]];
            Vector3f ab = vertex[tri.index[1]];
            Vector3f ac = vertex[tri.index[2]];
            return (aa + ab + ac) / 3.0f;
        };

        engine::vector<Vector3f> centers;
        for (uint32_t i = 0; i < m_triangles.size(); ++i)
        {
            centers.emplace_back(faceCenter(i));
        }

        flann::Matrix<float> datamatrix(&centers[0], centers.size(), 3);
        flann::Index<flann::L2_3D<float>> kdtree(datamatrix, flann::KDTreeSingleIndexParams());
        kdtree.buildIndex();

        auto emitFace = [&](vector<uint32_t>& output, uint32_t face)
        {
            for (auto&& faceIndex : { m_triangles[face].index[0], m_triangles[face].index[1], m_triangles[face].index[2] })
            {
                output.emplace_back(faceIndex);
            }
            auto pr = std::equal_range(std::begin(m_notEmittedTriangle), std::end(m_notEmittedTriangle), face);
            m_notEmittedTriangle.erase(pr.first, pr.second);
        };

        auto getAnyNotEmittedFace = [&]()->int
        {
            if (m_notEmittedTriangle.size() > 0)
                return m_notEmittedTriangle[0];
            return -1;
        };

        vector<uint32_t> output;
        auto face = 0;
        while (output.size() < index.size())
        {
            vector<uint32_t> cluster;

            face = getAnyNotEmittedFace();

            if (face == -1)
                break;

            float* p = kdtree.getPoint(face);
            Vector3f point;
            memcpy(&point, p, sizeof(Vector3f));
            flann::Matrix<float> searchPoint(&point, 1, 3);

            while (cluster.size() < 192)
            {
                emitFace(cluster, face);
                kdtree.removePoint(face);

                engine::vector<engine::vector<int>> res;
                engine::vector<engine::vector<float>> dist;
                flann::SearchParams searchParams;
                searchParams.checks = flann::FLANN_CHECKS_UNLIMITED;
                kdtree.knnSearch(searchPoint, res, dist, 1, searchParams);

                if (res[0].size() == 0)
                    break;
                face = res[0][0];
            }
            output.insert(output.end(), cluster.begin(), cluster.end());
        }
        return output;
    }

    void Clusterize::generateTriangleList(const engine::vector<Vector3f>& vertex, const engine::vector<uint32_t>& index)
    {
        m_triangles.clear();
        m_triangles.resize(index.size() / 3);
        m_notEmittedTriangle.resize(m_triangles.size());
        uint32_t triId = 0;
        m_meshBoundingBox.min = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        m_meshBoundingBox.max = Vector3f(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
        for (uint32_t i = 0; i < index.size(); i += 3, ++triId)
        {
            m_triangles[triId] = Triangle{ index[i], index[i + 1], index[i + 2], false };
            m_notEmittedTriangle[triId] = triId;

            for (int a = 0; a < 3; ++a)
            {
                const Vector3f& vec = vertex[index[i + a]];
                if (vec.x < m_meshBoundingBox.min.x) m_meshBoundingBox.min.x = vec.x;
                if (vec.y < m_meshBoundingBox.min.y) m_meshBoundingBox.min.y = vec.y;
                if (vec.z < m_meshBoundingBox.min.z) m_meshBoundingBox.min.z = vec.z;
                if (vec.x > m_meshBoundingBox.max.x) m_meshBoundingBox.max.x = vec.x;
                if (vec.y > m_meshBoundingBox.max.y) m_meshBoundingBox.max.y = vec.y;
                if (vec.z > m_meshBoundingBox.max.z) m_meshBoundingBox.max.z = vec.z;
            }
        }
    }

    void Clusterize::clusterize32to16(
        const vector<Vector3f>& vertex,
        const vector<Vector3f>& normal,
        const vector<Vector3f>& tangent,
        const engine::vector<engine::vector<engine::Vector4f>>& color,
        const engine::vector<engine::vector<engine::Vector2f>>& uv,
        const vector<uint32_t>& index,
        engine::vector<Vector3f>& vertexOutput,
        engine::vector<Vector3f>& normalOutput,
        engine::vector<Vector3f>& tangentOutput,
        engine::vector<engine::vector<engine::Vector4f>>& colorOutput,
        engine::vector<engine::vector<engine::Vector2f>>& uvOutput,
        engine::vector<uint16_t>& indexOutput,
        engine::vector<uint32_t>& clusterVertexStartPointers)
    {
        ASSERT(index.size() % 3 == 0, "Invalid index count. Needs to be divisible with 3");

        generateTriangleList(vertex, index);

        auto faceCenter = [&](uint32_t face)->Vector3f
        {
            Triangle& tri = m_triangles[face];
            Vector3f aa = vertex[tri.index[0]];
            Vector3f ab = vertex[tri.index[1]];
            Vector3f ac = vertex[tri.index[2]];
            return (aa + ab + ac) / 3.0f;
        };

        engine::vector<Vector3f> centers;
        for (uint32_t i = 0; i < m_triangles.size(); ++i)
        {
            centers.emplace_back(faceCenter(i));
        }

        flann::Matrix<float> datamatrix(&centers[0], centers.size(), 3);
        flann::Index<flann::L2_3D<float>> kdtree(datamatrix, flann::KDTreeSingleIndexParams());
        kdtree.buildIndex();

        auto emitFace = [&](vector<uint32_t>& output, uint32_t face)
        {
            for (auto&& faceIndex : { m_triangles[face].index[0], m_triangles[face].index[1], m_triangles[face].index[2] })
            {
                output.emplace_back(faceIndex);
            }
            auto pr = std::equal_range(std::begin(m_notEmittedTriangle), std::end(m_notEmittedTriangle), face);
            m_notEmittedTriangle.erase(pr.first, pr.second);
        };

        auto getAnyNotEmittedFace = [&]()->int
        {
            if (m_notEmittedTriangle.size() > 0)
                return m_notEmittedTriangle[0];
            return -1;
        };

        colorOutput.resize(color.size());
        uvOutput.resize(uv.size());

        size_t outputCount = 0;
        vector<uint8_t> indexFilled(ClusterMaxSize);
        auto face = 0;
        while (outputCount < index.size())
        {
            vector<uint32_t> cluster;

            face = getAnyNotEmittedFace();

            if (face == -1)
                break;

            float* p = kdtree.getPoint(face);
            Vector3f point;
            memcpy(&point, p, sizeof(Vector3f));
            flann::Matrix<float> searchPoint(&point, 1, 3);

            while (cluster.size() < ClusterMaxSize)
            {
                emitFace(cluster, face);
                kdtree.removePoint(face);

                engine::vector<engine::vector<int>> res;
                engine::vector<engine::vector<float>> dist;
                flann::SearchParams searchParams;
                searchParams.checks = flann::FLANN_CHECKS_UNLIMITED;
                kdtree.knnSearch(searchPoint, res, dist, 1, searchParams);

                if (res[0].size() == 0)
                    break;
                face = res[0][0];
            }

            clusterVertexStartPointers.emplace_back(vertexOutput.size());

#if 0
            uint16_t currentVertex = 0;
            memset(&indexFilled[0], 0, sizeof(uint8_t) * ClusterMaxSize);
            for(int i = 0; i < cluster.size(); ++i)
            {
                if (indexFilled[i] == 0)
                {
                    // this is a source index. need to translate
                    auto currentIndex = cluster[i];

                    indexOutput.emplace_back(currentVertex);
                    vertexOutput.emplace_back(vertex[currentIndex]);
                    normalOutput.emplace_back(normal[currentIndex]);
                    tangentOutput.emplace_back(tangent[currentIndex]);
                    for(int i = 0; i < color.size(); ++i)
                        colorOutput[i].emplace_back(color[i][currentIndex]);
                    for (int i = 0; i < uv.size(); ++i)
                        uvOutput[i].emplace_back(uv[i][currentIndex]);

                    for (int a = i; a < cluster.size(); ++a)
                    {
                        if (!indexFilled[a] && cluster[a] == currentIndex)
                        {
                            cluster[a] = currentVertex;
                            indexFilled[a] = 1;
                        }
                    }

                    ++currentVertex;
                }
                else
                {
                    // this index has already been translated
                    indexOutput.emplace_back(cluster[i]);
                }
            }
#else

            for (int i = 0; i < cluster.size(); ++i)
            {
                auto cindex = cluster[i];
                indexOutput.emplace_back(i);
                vertexOutput.emplace_back(vertex[cindex]);
                normalOutput.emplace_back(normal[cindex]);
                tangentOutput.emplace_back(tangent[cindex]);
                for (int a = 0; a < color.size(); ++a)
                    colorOutput[a].emplace_back(color[a][cindex]);
                for (int a = 0; a < uv.size(); ++a)
                    uvOutput[a].emplace_back(uv[a][cindex]);
            }
#endif

            outputCount += cluster.size();

            //output.insert(output.end(), cluster.begin(), cluster.end());
        }
    }

    void Clusterize::clusterize32to16Metis(
        const vector<Vector3f>& vertex,
        const vector<Vector3f>& normal,
        const vector<Vector3f>& tangent,
        const engine::vector<engine::vector<engine::Vector4f>>& color,
        const engine::vector<engine::vector<engine::Vector2f>>& uv,
        const vector<uint32_t>& index,
        engine::vector<Vector3f>& vertexOutput,
        engine::vector<Vector3f>& normalOutput,
        engine::vector<Vector3f>& tangentOutput,
        engine::vector<engine::vector<engine::Vector4f>>& colorOutput,
        engine::vector<engine::vector<engine::Vector2f>>& uvOutput,
        engine::vector<uint16_t>& indexOutput,
        engine::vector<uint32_t>& clusterIndexStartPointers,
        engine::vector<uint32_t>& clusterVertexStartPointers)
    {
        struct Edge
        {
            uint32_t ends[2];
        };

        struct Tri
        {
            uint32_t index[3];

            vector<Edge> edges()
            {
                return {
                    Edge{ index[0], index[1] },
                    Edge{ index[1], index[2] },
                    Edge{ index[2], index[0] } };
            }
        };

        struct Adjacent
        {
            vector<uint32_t> triangle;
        };
        using Hits = Adjacent;
        
        vector<Tri> triangles;
        vector<Hits> vertexHits;
        triangles.resize(index.size() / 3);
        vertexHits.resize(vertex.size());

        // create triangles
        for (uint32_t i = 0, triId = 0; i < index.size(); i += 3, ++triId)
        {
            triangles[triId] = Tri{ index[i], index[i + 1], index[i + 2] };
            vertexHits[index[i]].triangle.emplace_back(triId);
            vertexHits[index[i + 1]].triangle.emplace_back(triId);
            vertexHits[index[i + 2]].triangle.emplace_back(triId);
        }

        vector<Adjacent> adjacents;
        adjacents.resize(triangles.size() * 3);
        for (size_t i = 0; i < triangles.size(); ++i)
        {
            auto edges = triangles[i].edges();
            int edgeIndex = 0;
            for (auto&& edge : edges)
            {
                // find another triangle that has the same ends
                auto end1 = edge.ends[0];
                auto end2 = edge.ends[1];

                for (auto tri : vertexHits[end1].triangle)
                {
                    // we're certainly in the list, so ignore
                    if (tri != i)
                    {
                        auto triEdges = triangles[tri].edges();
                        for (auto triEdge : triEdges)
                        {
                            if ((triEdge.ends[0] == end1 && triEdge.ends[1] == end2) ||
                                (triEdge.ends[1] == end1 && triEdge.ends[0] == end2))
                            {
                                // found another triangle that shares an edge
                                adjacents[(i * 3) + edgeIndex].triangle.emplace_back(tri);
                            }
                        }
                    }
                }
                ++edgeIndex;
            }
        }


        vector<int> vertexOffset;
        vector<int> adjacency;
        vector<int> triangleToCluster;
        int triangleCount = triangles.size();
        vertexOffset.resize(triangleCount + 1);
        memset(vertexOffset.data(), 0, vertexOffset.size() * sizeof(int));
        adjacency.resize(triangleCount * 3);
        memset(adjacency.data(), 0, adjacency.size() * sizeof(int));
        triangleToCluster.resize(triangleCount);
        memset(triangleToCluster.data(), 0, triangleToCluster.size() * sizeof(int));

        int offset = 0;
        for (size_t triangleIndex = 0; triangleIndex < static_cast<int>(triangleCount); ++triangleIndex)
        {
            vertexOffset[triangleIndex] = offset;
            auto edges = triangles[triangleIndex].edges();
            int edgeIndex = 0;
            for (size_t i = 0; i < edges.size(); ++i)
            {
                if (adjacents[(triangleIndex * 3) + edgeIndex].triangle.size() > 0)
                {
                    adjacency[offset] = adjacents[(triangleIndex * 3) + edgeIndex].triangle[0];
                    ++offset;
                    ++edgeIndex;
                }
            }
        }
        vertexOffset[triangleCount] = offset;

        auto numPartitions = std::max(triangleCount / 64, 2);

        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        //options[METIS_OPTION_UFACTOR] = 178;
        //options[METIS_OPTION_UFACTOR] = 200;
        idx_t ncon = 1; //The number of balancing constraints. It should be at least 1.
        int objval = 0; //Number of edgecuts

        auto res = METIS_PartGraphKway(
            &triangleCount, 
            &ncon,
            vertexOffset.data(), 
            adjacency.data(), 
            nullptr, nullptr,  nullptr,
            &numPartitions, nullptr, nullptr, options, &objval, triangleToCluster.data());

        ASSERT(res == METIS_OK, "METIS clusterization failed!");

        struct Cluster
        {
            vector<uint32_t> originalIndex;

            vector<uint16_t> index;
            vector<Vector3f> vertexOutput;
            vector<Vector3f> normalOutput;
            vector<Vector3f> tangentOutput;
            vector<engine::vector<engine::Vector4f>> colorOutput;
            vector<engine::vector<engine::Vector2f>> uvOutput;
        };

        vector<Cluster> clusters;
        clusters.resize(numPartitions);

        for (int triangle = 0; triangle < triangleToCluster.size(); ++triangle)
        {
            auto tri = triangles[triangle];
            auto toCluster = triangleToCluster[triangle];
            clusters[toCluster].originalIndex.emplace_back(tri.index[0]);
            clusters[toCluster].originalIndex.emplace_back(tri.index[1]);
            clusters[toCluster].originalIndex.emplace_back(tri.index[2]);
        }

        ASSERT(clusters.size() == numPartitions, "Issue with clusterization");
        for (int i = 0; i < numPartitions; ++i)
        {
            auto& cluster = clusters[i];
            cluster.colorOutput.resize(color.size());
            cluster.uvOutput.resize(uv.size());

            unordered_map<uint32_t, uint16_t> reorder;

            for (int a = 0; a < cluster.originalIndex.size(); ++a)
            {
                auto origIndex = cluster.originalIndex[a];
                auto exists = reorder.find(origIndex);
                if (exists != reorder.end())
                {
                    cluster.index.emplace_back((*exists).second);
                }
                else
                {
                    auto newIndex = cluster.vertexOutput.size();
                    cluster.index.emplace_back(newIndex);
                    cluster.vertexOutput.emplace_back(vertex[origIndex]);
                    cluster.normalOutput.emplace_back(normal[origIndex]);
                    if(origIndex < tangent.size())
                    cluster.tangentOutput.emplace_back(tangent[origIndex]);
                    for (int b = 0; b < color.size(); ++b)
                        cluster.colorOutput[b].emplace_back(color[b][origIndex]);
                    for (int b = 0; b < uv.size(); ++b)
                        cluster.uvOutput[b].emplace_back(uv[b][origIndex]);

                    reorder[origIndex] = newIndex;
                }
            }
        }

        colorOutput.resize(color.size());
        uvOutput.resize(uv.size());
        for (int i = 0; i < numPartitions; ++i)
        {
            auto& cluster = clusters[i];
            
            clusterIndexStartPointers.emplace_back(indexOutput.size());
            clusterVertexStartPointers.emplace_back(vertexOutput.size());

            for (int a = 0; a < cluster.index.size(); ++a)
                indexOutput.emplace_back(cluster.index[a]);

            for (int a = 0; a < cluster.vertexOutput.size(); ++a)
                vertexOutput.emplace_back(cluster.vertexOutput[a]);
            for (int a = 0; a < cluster.normalOutput.size(); ++a)
                normalOutput.emplace_back(cluster.normalOutput[a]);
            for (int a = 0; a < cluster.tangentOutput.size(); ++a)
                tangentOutput.emplace_back(cluster.tangentOutput[a]);
            
            
            for (int a = 0; a < cluster.colorOutput.size(); ++a)
            {
                for (int b = 0; b < cluster.colorOutput[a].size(); ++b)
                    colorOutput[a].emplace_back(cluster.colorOutput[a][b]);
            }

            for (int a = 0; a < cluster.uvOutput.size(); ++a)
            {
                for (int b = 0; b < cluster.uvOutput[a].size(); ++b)
                    uvOutput[a].emplace_back(cluster.uvOutput[a][b]);
            }
        }


        /*
        * engine::vector<Vector3f>& vertexOutput,
        engine::vector<Vector3f>& normalOutput,
        engine::vector<Vector3f>& tangentOutput,
        engine::vector<engine::vector<engine::Vector4f>>& colorOutput,
        engine::vector<engine::vector<engine::Vector2f>>& uvOutput,
        engine::vector<uint16_t>& indexOutput,
        engine::vector<uint32_t>& clusterVertexStartPointers)
        */



        LOG("should have result now");
    }

}
