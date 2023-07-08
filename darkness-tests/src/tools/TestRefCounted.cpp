#include "gtest/gtest.h"
#include "tools/PathTools.h"
#include "tools/Debug.h"
#include "tools/RefCounted.h"
#include "shaders/ShaderTypes.h"
#include <chrono>
#include "containers/vector.h"

using namespace engine;
using namespace engine;

#if 0
TEST(TestRefCounted, DISABLED_RefCountedPerformance)
{
    struct tempstruct
    {
        Float4x4 a;
        Float4x4 b;
        Float4x4 c;
    };

    // ####################################################################
    engine::vector<engine::shared_ptr<tempstruct>> shared;
    for (int i = 0; i < 1000000; ++i)
    {
        shared.emplace_back(engine::make_shared<tempstruct>(tempstruct{ Float4x4::zero(), Float4x4::zero(), Float4x4::zero() }));
        //shared.emplace_back(new tempstruct({ Float4x4::zero(), Float4x4::zero(), Float4x4::zero() }));
    }

    engine::vector<engine::shared_ptr<tempstruct>> shared_dst;

    auto shared_start = std::chrono::high_resolution_clock::now();
    shared_dst = shared;
    auto shared_stop = std::chrono::high_resolution_clock::now();

    auto shared_start_access = std::chrono::high_resolution_clock::now();
    for (auto&& sha : shared)
    {
        ASSERT(sha->a.data[0] == 0 && sha->b.data[0] == 0 && sha->c.data[0] == 0, "");
    }
    auto shared_stop_access = std::chrono::high_resolution_clock::now();
    shared.clear();

    // ####################################################################
    engine::vector<tools::RefCounted<tempstruct>> eshared;

    for (int i = 0; i < 1000000; ++i)
    {
        eshared.emplace_back(new tempstruct({ Float4x4::zero(), Float4x4::zero(), Float4x4::zero() }));
    }

    engine::vector<tools::RefCounted<tempstruct>> eshared_dst;

    auto eshared_start = std::chrono::high_resolution_clock::now();
    eshared_dst = eshared;
    auto eshared_stop = std::chrono::high_resolution_clock::now();

    auto eshared_start_access = std::chrono::high_resolution_clock::now();
    for (auto&& esha : eshared)
    {
        ASSERT(esha->a.data[0] == 0 && esha->b.data[0] == 0 && esha->c.data[0] == 0, "");
    }
    auto eshared_stop_access = std::chrono::high_resolution_clock::now();
    eshared.clear();

    // ####################################################################

    engine::vector<tempstruct*> ptr;

    for (int i = 0; i < 1000000; ++i)
    {
        ptr.emplace_back(new tempstruct({ Float4x4::zero(), Float4x4::zero(), Float4x4::zero() }));
    }

    engine::vector<tempstruct*> ptr_dst;

    auto ptr_start = std::chrono::high_resolution_clock::now();
    ptr_dst = ptr;
    auto ptr_stop = std::chrono::high_resolution_clock::now();

    auto ptr_start_access = std::chrono::high_resolution_clock::now();
    for (auto&& p : ptr)
    {
        ASSERT(p->a.data[0] == 0 && p->b.data[0] == 0 && p->c.data[0] == 0, "");
    }
    auto ptr_stop_access = std::chrono::high_resolution_clock::now();
    ptr.clear();
    // ####################################################################

    engine::vector<tempstruct> inst;

    for (int i = 0; i < 1000000; ++i)
    {
        inst.emplace_back(tempstruct({ Float4x4::zero(), Float4x4::zero(), Float4x4::zero() }));
    }

    engine::vector<tempstruct> inst_dst;

    auto inst_start = std::chrono::high_resolution_clock::now();
    inst_dst = inst;
    auto inst_stop = std::chrono::high_resolution_clock::now();

    auto inst_start_access = std::chrono::high_resolution_clock::now();
    for (auto&& i : inst)
    {
        ASSERT(i.a.data[0] == 0 && i.b.data[0] == 0 && i.c.data[0] == 0, "");
    }
    auto inst_stop_access = std::chrono::high_resolution_clock::now();
    inst.clear();
    // ####################################################################

    LOG_INFO("shared copy = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(shared_stop - shared_start).count() / 1000000.0);
    LOG_INFO("engine shared copy = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(eshared_stop - eshared_start).count() / 1000000.0);
    LOG_INFO("ptr copy = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(ptr_stop - ptr_start).count() / 1000000.0);
    LOG_INFO("inst copy = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(inst_stop - inst_start).count() / 1000000.0);

    LOG_INFO("shared access = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(shared_stop_access - shared_start_access).count() / 1000000.0);
    LOG_INFO("engine shared access = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(eshared_stop_access - eshared_start_access).count() / 1000000.0);
    LOG_INFO("ptr access = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(ptr_stop_access - ptr_start_access).count() / 1000000.0);
    LOG_INFO("inst access = %05.5f ms", std::chrono::duration_cast<std::chrono::nanoseconds>(inst_stop_access - inst_start_access).count() / 1000000.0);
    LOG_WARNING("");
}
#endif