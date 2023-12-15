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
    engine::vector<ComponentTypeId> vec2{ typeAId, typeBId, typeCId };
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
