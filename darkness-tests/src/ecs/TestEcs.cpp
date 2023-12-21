#include "GlobalTestFixture.h"
#include "ecs/TypeStorage.h"
#include "ecs/ArcheTypeStorage.h"

using namespace ecs;

TEST(TestEcs, TestTypeStorage)
{
    TypeStorage typeStorage;

    auto typeId = typeStorage.typeId<int>();
    EXPECT_EQ(typeId, 0);

    struct A
    {
        int blaa;
    };
    typeId = typeStorage.typeId<A>();
    EXPECT_EQ(typeId, 1);

    struct B : public A
    {
    };
    typeId = typeStorage.typeId<B>();
    EXPECT_EQ(typeId, 2);

    typeId = typeStorage.typeId<int>();
    EXPECT_EQ(typeId, 0);
}

TEST(TestEcs, TestTypeData)
{
    // test default instance
    {
        TypeData<int> typeDataInt;
        const auto const_ptr = typeDataInt.data();
        EXPECT_EQ(const_ptr, nullptr);

        auto mutable_ptr = typeDataInt.data();
        EXPECT_EQ(mutable_ptr, nullptr);

        EXPECT_EQ(typeDataInt.elements(), 0);
    }

    // test for memory under/over write
    {
        auto ptr = _aligned_malloc(sizeof(uint32_t) * 12, 64);
        memset(ptr, 0, sizeof(uint32_t) * 12);
        *static_cast<uint32_t*>(ptr) = 0xffffffff;
        *(static_cast<uint32_t*>(ptr) + 11) = 0xffffffff;

        auto typeDataPtr = static_cast<uint32_t*>(ptr) + 1;

        TypeData<uint32_t> typeDataInt(typeDataPtr, 10);
        EXPECT_EQ(typeDataInt.elements(), 10);

        for (int i = 0; i < typeDataInt.elements(); ++i)
            typeDataInt.data()[i] = 0;
        
        EXPECT_EQ(*static_cast<uint32_t*>(ptr), 0xffffffff);
        EXPECT_EQ(*(static_cast<uint32_t*>(ptr) + 11), 0xffffffff);

        *static_cast<uint32_t*>(ptr) = 0;
        *(static_cast<uint32_t*>(ptr) + 11) = 0;
        for (int i = 0; i < typeDataInt.elements(); ++i)
            typeDataInt.data()[i] = 0xFFFFFFFF;

        EXPECT_EQ(*static_cast<uint32_t*>(ptr), 0);
        EXPECT_EQ(*(static_cast<uint32_t*>(ptr) + 11), 0);

        _aligned_free(ptr);
    }

    // test trivial construction (should not initialize data)
    {
        class SomeType
        {
        public:
            int A;
            int B;
        };

        auto ptr = _aligned_malloc(sizeof(SomeType) * 10, 64);
        for (int i = 0; i < 10; ++i)
        {
            (static_cast<SomeType*>(ptr) + i)->A = i;
            (static_cast<SomeType*>(ptr) + i)->B = i+1;
        }

        TypeData<SomeType> tmp(static_cast<SomeType*>(ptr), 10);
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_EQ(tmp.data()[i].A, i);
            EXPECT_EQ(tmp.data()[i].B, i + 1);
        }

        _aligned_free(ptr);
    }

    // test default value construction
    {
        class SomeType
        {
        public:
            int A = 12345;
            int B = 12345;
        };

        auto ptr = _aligned_malloc(sizeof(SomeType) * 10, 64);
        for (int i = 0; i < 10; ++i)
        {
            (static_cast<SomeType*>(ptr) + i)->A = i;
            (static_cast<SomeType*>(ptr) + i)->B = i + 1;
        }

        TypeData<SomeType> tmp(static_cast<SomeType*>(ptr), 10);
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_EQ(tmp.data()[i].A, 12345);
            EXPECT_EQ(tmp.data()[i].B, 12345);
        }

        _aligned_free(ptr);
    }

    // test default constructor
    {
        class SomeType
        {
        public:
            SomeType()
                : A{ 12345 }
                , B{ 12345 }
            {}

            int A;
            int B;
        };

        auto ptr = _aligned_malloc(sizeof(SomeType) * 10, 64);
        for (int i = 0; i < 10; ++i)
        {
            auto ptri = static_cast<SomeType*>(ptr) + i;
            if (ptri) ptri->A = i;
            if (ptri) ptri->B = i + 1;
        }

        TypeData<SomeType> tmp(static_cast<SomeType*>(ptr), 10);
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_EQ(tmp.data()[i].A, 12345);
            EXPECT_EQ(tmp.data()[i].B, 12345);
        }

        _aligned_free(ptr);
    }

    // test something wonky
    {
        class SomeType
        {
        public:
            SomeType() { B = 10; };

            SomeType(int a, int b)
                : A{ a }
                , B{ b }
            {}

            int A = 12345;
            int B = 12345;
        };

        auto ptr = _aligned_malloc(sizeof(SomeType) * 10, 64);
        EXPECT_TRUE(ptr != nullptr);
        for (int i = 0; i < 10; ++i)
        {
            auto p = static_cast<SomeType*>(ptr) + i;
            EXPECT_TRUE(p != nullptr);
            if (p) p->A = i;
            if (p) p->B = i + 1;
        }

        TypeData<SomeType> tmp(static_cast<SomeType*>(ptr), 10);
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_EQ(tmp.data()[i].A, 12345);
            EXPECT_EQ(tmp.data()[i].B, 10);
        }

        _aligned_free(ptr);
    }

    // test swap
    {
        struct SomeType
        {
            int A;
            int B;
        };

        auto ptr = _aligned_malloc(sizeof(SomeType) * 10, 64);
        EXPECT_TRUE(ptr != nullptr);
        
        TypeData<SomeType> tmp(static_cast<SomeType*>(ptr), 10);
        for (int i = 0; i < 10; ++i)
        {
            tmp.data()[i].A = i;
            tmp.data()[i].B = 9-i;
        }

        for (int i = 0; i < 5; ++i)
            tmp.swap(i, 9 - i);

        for (int i = 0; i < 10; ++i)
        {
            EXPECT_EQ(tmp.data()[i].A, 9 - i);
            EXPECT_EQ(tmp.data()[i].B, i);
        }

        _aligned_free(ptr);
    }

    // test copy
    {
        struct SomeType
        {
            int A;
            int B;
        };

        auto ptrSrc = _aligned_malloc(sizeof(SomeType) * 10, 64);
        EXPECT_TRUE(ptrSrc != nullptr);
        auto ptrDst = _aligned_malloc(sizeof(SomeType) * 10, 64);
        EXPECT_TRUE(ptrDst != nullptr);
        if(ptrDst)
            memset(ptrDst, 0, sizeof(SomeType) * 10);

        TypeData<SomeType> tmpSrc(static_cast<SomeType*>(ptrSrc), 10);
        for (int i = 0; i < 10; ++i)
        {
            tmpSrc.data()[i].A = i;
            tmpSrc.data()[i].B = 9 - i;
        }

        TypeData<SomeType> tmpDst(static_cast<SomeType*>(ptrDst), 10);
        tmpDst.copy(&tmpSrc, 0, 5, 5);
        tmpDst.copy(&tmpSrc, 5, 0, 5);

        for (int i = 0; i < 5; ++i)
        {
            EXPECT_EQ(tmpDst.data()[i].A, 5 + i);
            EXPECT_EQ(tmpDst.data()[i].B, 4 - i);
        }
        for (int i = 0; i < 5; ++i)
        {
            EXPECT_EQ(tmpDst.data()[5 + i].A, i);
            EXPECT_EQ(tmpDst.data()[5 + i].B, 9-i);
        }

        _aligned_free(ptrSrc);
        _aligned_free(ptrDst);
    }
}

TEST(TestEcs, TestArcheTypeStorage)
{
    TypeStorage typeStorage;
    ArcheTypeStorage archeTypeStorage(typeStorage);

    struct TypeA { uint64_t value; };
    struct TypeB { uint32_t value; };
    struct TypeC { uint64_t valueA; uint64_t valueB; };
    struct TypeD { uint8_t value; };

    auto typeAId = typeStorage.typeId<TypeA>();
    auto typeBId = typeStorage.typeId<TypeB>();
    auto typeCId = typeStorage.typeId<TypeC>();
    auto typeDId = typeStorage.typeId<TypeD>();

    // check that ArcheType contains given types
    auto archeType = archeTypeStorage.archeType({ typeAId, typeBId, typeCId });
    EXPECT_TRUE(archeType.contains(typeAId));
    EXPECT_TRUE(archeType.contains(typeBId));
    EXPECT_TRUE(archeType.contains(typeCId));
    EXPECT_FALSE(archeType.contains(typeDId));

    // check that type order doesn't matter
    auto archeType2 = archeTypeStorage.archeType({ typeBId, typeCId, typeAId });
    EXPECT_EQ(archeType.id(), archeType2.id());

    // test that we get the correct archetype from ArcheTypeSet
    ArcheTypeSet typeSet;
    typeSet.set(typeAId);
    typeSet.set(typeBId);
    typeSet.set(typeCId);
    auto archeType3 = archeTypeStorage.archeType(typeSet);
    EXPECT_EQ(archeType.id(), archeType3.id());

    typeSet.clear(typeBId);
    auto archeType4 = archeTypeStorage.archeType(typeSet);
    EXPECT_NE(archeType.id(), archeType4.id());

    // test ArcheType extension
    auto archeType5 = archeTypeStorage.archeType(archeType4.id(), typeBId);
    EXPECT_EQ(archeType5.id(), archeType.id());

    // test archeTypeSet
    typeSet.set(typeBId);
    auto set = archeTypeStorage.archeTypeSet({ typeAId, typeBId, typeCId });
    EXPECT_EQ(set, typeSet);

    // test archeTypeIdFromSet
    EXPECT_EQ(archeTypeStorage.archeTypeIdFromSet(archeType.typeSet()), archeType.id());

    // test typeIdVectorFromArcheType
    auto vec = archeTypeStorage.typeIdVectorFromArcheType(archeType.id());
    engine::vector<TypeId> vec2{ typeAId, typeBId, typeCId };
    EXPECT_EQ(vec, vec2);

    // get all archetypes that contain types
    auto archeType00 = archeTypeStorage.archeType({ typeBId, typeCId });
    auto archeType01 = archeTypeStorage.archeType({ typeDId, typeCId });
    auto archeType02 = archeTypeStorage.archeType({ typeBId, typeCId, typeDId });
    auto archeType03 = archeTypeStorage.archeType({ typeAId, typeDId });
    auto archeType04 = archeTypeStorage.archeType({ typeBId });

    ArcheTypeSet includeSet;
    includeSet.set(typeBId);
    auto archeTypeList = archeTypeStorage.archeTypesThatContain(includeSet);
    EXPECT_EQ(archeTypeList.size(), 4);
    EXPECT_EQ(archeTypeList[0], 0);
    EXPECT_EQ(archeTypeList[1], 2);
    EXPECT_EQ(archeTypeList[2], 4);
    EXPECT_EQ(archeTypeList[3], 6);

    // test Container
    auto containerA = archeTypeStorage.archeTypeInfo(archeType.id());
    auto containerB = archeTypeStorage.archeTypeInfo(archeType4.id());
    auto containerC = archeTypeStorage.archeTypeInfo(archeType04.id());

    EXPECT_EQ(containerA.sizeBytes, 28);
    EXPECT_EQ(containerA.typeCount, 3);
    EXPECT_TRUE(containerA.set.get(typeAId));
    EXPECT_TRUE(containerA.set.get(typeBId));
    EXPECT_TRUE(containerA.set.get(typeCId));

    EXPECT_EQ(containerB.sizeBytes, 24);
    EXPECT_EQ(containerB.typeCount, 2);
    EXPECT_TRUE(containerB.set.get(typeAId));
    EXPECT_TRUE(containerB.set.get(typeCId));

    EXPECT_EQ(containerC.sizeBytes, 4);
    EXPECT_EQ(containerC.typeCount, 1);
    EXPECT_TRUE(containerC.set.get(typeBId));
}

class AlignedData
{
public:
    AlignedData()
        : m_data{ nullptr }
    {}

    AlignedData(size_t bytes, size_t alignment)
        : m_data{ _aligned_malloc(bytes, alignment) }
    {
        ASSERT(m_data != nullptr, "Failed to allocate memory!");
    }

    AlignedData(AlignedData&& data) noexcept
        : m_data{ nullptr }
    {
        std::swap(m_data, data.m_data);
    }

    AlignedData& operator=(AlignedData&& data) noexcept
    {
        std::swap(m_data, data.m_data);
        return *this;
    }

    AlignedData(const AlignedData&) = delete;
    AlignedData& operator=(const AlignedData&) = delete;

    ~AlignedData()
    {
        if(m_data)
            _aligned_free(m_data);
    }

    void* data() { return m_data; }
    const void* data() const { return m_data; }
private:
    void* m_data;
};

TEST(TestEcs, TestChunk)
{
    TypeStorage typeStorage;
    ArcheTypeStorage archeTypeStorage(typeStorage);

    using GameTranslation = engine::Vector3f;
    using GameRotation = engine::Vector4f;

    // let's allocate some backing memory for our data
    auto chunkData = AlignedData(PreferredChunkSizeBytes, ChunkAlignment);
    
    // let's get the type data
    auto typeIdA = typeStorage.typeId<GameTranslation>();
    auto typeIdB = typeStorage.typeId<GameRotation>();
    auto archeType = archeTypeStorage.archeType({ typeIdA, typeIdB });
    auto archeTypeInfo = archeTypeStorage.archeTypeInfo(archeType.id());

    // and create the chunk
    auto chunk = new Chunk(
        typeStorage, 
        archeTypeInfo, 
        StorageAllocation{ chunkData.data(), nullptr });

    EXPECT_EQ(chunk->empty(), true);
    EXPECT_EQ(chunk->full(), false);

    // allocate entities from the chunk
    engine::vector<uint64_t> entities;
    for (int i = 0; i < 10; ++i)
    {
        entities.emplace_back(chunk->allocate());
    }

    EXPECT_EQ(chunk->empty(), false);
    EXPECT_EQ(chunk->full(), false);

    // write some data to the chunk
    int index = 0;
    for (auto&& entity : entities)
    {
        chunk->componentDataPointer<GameTranslation>(typeIdA)[entity] = GameTranslation{
            static_cast<float>(index),
            static_cast<float>(index + 1),
            static_cast<float>(index + 2) };
        chunk->componentDataPointer<GameRotation>(typeIdB)[entity] = GameRotation{
            static_cast<float>(index),
            static_cast<float>(index + 1),
            static_cast<float>(index + 2),
            static_cast<float>(index + 3) };
        ++index;
    }

    // note: the capacity depends on the size of the type and
    // chunk entity allocation algorithm. (in ArcheTypeStorage.cpp)
    EXPECT_EQ(chunk->capacity(), 1820);
    EXPECT_EQ(chunk->size(), 10);
    EXPECT_EQ(chunk->available(), 1810);

    // let's allocate the rest of them
    engine::vector<uint64_t> rest;
    for (int i = 0; i < 1810; ++i)
        rest.emplace_back(chunk->allocate());

    EXPECT_EQ(chunk->full(), true);
    EXPECT_EQ(chunk->available(), 0);
    EXPECT_EQ(chunk->size(), 1820);

    // create another chunk to copy to
    auto chunkDataDst = AlignedData(PreferredChunkSizeBytes, ChunkAlignment);
    auto chunkDst = new Chunk(
        typeStorage,
        archeTypeInfo,
        StorageAllocation{ chunkDataDst.data(), nullptr });

    // copy over the first 10 entities
    chunkDst->copy(*chunk, 0, 0, 10);

    for (int i = 0; i < 10; ++i)
    {
        auto piece = chunkDst->componentDataPointer<GameTranslation>(typeIdA)[i];
        EXPECT_EQ(piece.x, static_cast<float>(i));
        EXPECT_EQ(piece.y, static_cast<float>(i + 1));
        EXPECT_EQ(piece.z, static_cast<float>(i + 2));

        auto pieceB = chunkDst->componentDataPointer<GameRotation>(typeIdB)[i];
        EXPECT_EQ(pieceB.x, static_cast<float>(i));
        EXPECT_EQ(pieceB.y, static_cast<float>(i + 1));
        EXPECT_EQ(pieceB.z, static_cast<float>(i + 2));
        EXPECT_EQ(pieceB.w, static_cast<float>(i + 3));
    }

    // swap some data
    for (int i = 0; i < 10; ++i)
        chunk->swap(i, 10 + i);
    chunkDst->copy(*chunk, 10, 20, 10);
    for (int i = 0; i < 10; ++i)
    {
        auto piece = chunkDst->componentDataPointer<GameTranslation>(typeIdA)[20 + i];
        EXPECT_EQ(piece.x, static_cast<float>(i));
        EXPECT_EQ(piece.y, static_cast<float>(i + 1));
        EXPECT_EQ(piece.z, static_cast<float>(i + 2));

        auto pieceB = chunkDst->componentDataPointer<GameRotation>(typeIdB)[20 + i];
        EXPECT_EQ(pieceB.x, static_cast<float>(i));
        EXPECT_EQ(pieceB.y, static_cast<float>(i + 1));
        EXPECT_EQ(pieceB.z, static_cast<float>(i + 2));
        EXPECT_EQ(pieceB.w, static_cast<float>(i + 3));
    }

    for (int i = 0; i < 1820; ++i)
        chunk->freeLast();

    EXPECT_EQ(chunk->size(), 0);
    EXPECT_EQ(chunk->available(), 1820);
    EXPECT_EQ(chunk->full(), false);
    EXPECT_EQ(chunk->empty(), true);

    delete chunk;
    delete chunkDst;
}

TEST(TestEcs, TestChunkStorageAllocation)
{
    ChunkStorageAllocation csa(ChunkStorageAllocationSize, PreferredChunkSizeBytes);
    EXPECT_EQ(csa.empty(), true);

    engine::vector<void*> allocations;
    while(void* mem = csa.allocate())
        allocations.emplace_back(mem);

    EXPECT_EQ(allocations.size(), ChunkStorageAllocationSize / PreferredChunkSizeBytes);
    EXPECT_EQ(csa.empty(), false);
    EXPECT_EQ(csa.full(), true);
    
    for (auto&& chunk : allocations)
        csa.deallocate(chunk);
    allocations.clear();

    EXPECT_EQ(csa.empty(), true);
    EXPECT_EQ(csa.full(), false);

    while (void* mem = csa.allocate())
        allocations.emplace_back(mem);

    void* allocPtr[4] = { allocations[12], allocations[34], allocations[56], allocations[78] };

    csa.deallocate(allocations[12]);
    csa.deallocate(allocations[34]);
    csa.deallocate(allocations[56]);
    csa.deallocate(allocations[78]);

    allocations.erase(allocations.begin() + 78);
    allocations.erase(allocations.begin() + 56);
    allocations.erase(allocations.begin() + 34);
    allocations.erase(allocations.begin() + 12);

    void* allocPtrCmp[4] = { csa.allocate(), csa.allocate(), csa.allocate(), csa.allocate() };
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(allocPtr[i], allocPtrCmp[i]);
}

TEST(TestEcs, TestChunkStorage)
{
    TypeStorage typeStorage;
    ArcheTypeStorage archeTypeStorage(typeStorage);
    ChunkStorage chunkStorage(typeStorage, archeTypeStorage);

    struct A
    {
        int val;
        int val2;
    };

    struct B
    {
        float value;
        uint64_t test;
    };

    struct C
    {
        int temp;
    };

    auto archeType1 = archeTypeStorage.archeType({ typeStorage.typeId<A>(), typeStorage.typeId<B>() });
    auto archeType2 = archeTypeStorage.archeType(
        { typeStorage.typeId<A>(), typeStorage.typeId<B>(), typeStorage.typeId<C>() });

    engine::vector<Chunk*> chunks1;
    for (int i = 0; i < 50000; ++i)
        chunks1.emplace_back(chunkStorage.allocateChunk(archeType1.id()));

    engine::vector<Chunk*> chunks2;
    for (int i = 0; i < 50000; ++i)
        chunks2.emplace_back(chunkStorage.allocateChunk(archeType2.id()));

    for (int i = 0; i < 50000; ++i)
        chunks1.emplace_back(chunkStorage.allocateChunk(archeType1.id()));
    for (int i = 0; i < 50000; ++i)
        chunks2.emplace_back(chunkStorage.allocateChunk(archeType2.id()));

}
