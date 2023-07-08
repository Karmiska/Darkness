#include "GlobalTestFixture.h"
#include "engine/rendering/ModelCpu.h"
#include "engine/primitives/Vector3.h"

#include <random>

TEST(TestVertexPacking, VertexPacking)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0, 100.0);

    constexpr int VertexCount = 100000;

    engine::vector<engine::Vector3f> vertexes;
    for (int i = 0; i < VertexCount; ++i)
    {
        vertexes.emplace_back(engine::Vector3f{
            static_cast<float>(dis(gen)) - 50.0f,
            static_cast<float>(dis(gen)) - 50.0f,
            static_cast<float>(dis(gen)) - 50.0f });
    }

    engine::Vector3f minVertex{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    engine::Vector3f maxVertex{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

    for (int i = 0; i < VertexCount; ++i)
    {
        if (vertexes[i].x < minVertex.x) minVertex.x = vertexes[i].x;
        if (vertexes[i].y < minVertex.y) minVertex.y = vertexes[i].y;
        if (vertexes[i].z < minVertex.z) minVertex.z = vertexes[i].z;
        if (vertexes[i].x > maxVertex.x) maxVertex.x = vertexes[i].x;
        if (vertexes[i].y > maxVertex.y) maxVertex.y = vertexes[i].y;
        if (vertexes[i].z > maxVertex.z) maxVertex.z = vertexes[i].z;
    }
    VertexScale vertexScale;
    vertexScale.range.x = maxVertex.x - minVertex.x;
    vertexScale.range.y = maxVertex.y - minVertex.y;
    vertexScale.range.z = maxVertex.z - minVertex.z;
    vertexScale.origo.x = minVertex.x;
    vertexScale.origo.y = minVertex.y;
    vertexScale.origo.z = minVertex.z;

    for (int i = 0; i < VertexCount; ++i)
    {
        auto packed = engine::packVertex(vertexes[i], vertexScale);
        auto unpacked = engine::unpackVertex(packed, vertexScale);

        float abs_error = 0.0001f;
        EXPECT_NEAR(vertexes[i].x, unpacked.x, abs_error);
        EXPECT_NEAR(vertexes[i].y, unpacked.y, abs_error);
        EXPECT_NEAR(vertexes[i].z, unpacked.z, abs_error);
    }
}
