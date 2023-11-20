#include "GlobalTestFixture.h"
#include "containers/BitSet.h"
#include <bit>
#include <bitset>




TEST(TestBitSet, BitSetIterator8BitSpecialization)
{
    // check that empty bitset works
    {
        BitSet<8> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // test all iterator values
    {
        int set = 8;
        int maxn = 1 << set;
        for (int i = 0; i < maxn; ++i)
        {
            BitSet<8> test;

            for (int a = 0; a < set; ++a)
            {
                if (i & (1 << a)) test.set(a);
            }

            engine::vector<int> tmp;
            for (auto&& bit : test)
                tmp.emplace_back(bit);

            uint32_t res = 0;
            for (auto&& t : tmp)
                res |= 1 << t;

            EXPECT_EQ(tmp.size(), __popcnt(i));
            EXPECT_EQ(i, res);
        }
    }

    // test invalid sets
#ifdef RETAIL
    {
        BitSet<8> test;
        test.set(10);
        test.set(-3);
        for (int i = 0; i < 8; ++i)
            EXPECT_EQ(test.get(i), 0);
    }
#endif
}

TEST(TestBitSet, BitSetIterator16BitSpecialization)
{
    // test all iterator values
    {
        int set = 16;
        int maxn = 1 << set;
        for (int i = 0; i < maxn; ++i)
        {
            BitSet<16> test;

            for (int a = 0; a < set; ++a)
            {
                if (i & (1 << a)) test.set(a);
            }

            engine::vector<int> tmp;
            for (auto&& bit : test)
                tmp.emplace_back(bit);

            uint32_t res = 0;
            for (auto&& t : tmp)
                res |= 1 << t;

            EXPECT_EQ(tmp.size(), __popcnt(i));
            EXPECT_EQ(i, res);
        }
    }

    // test invalid sets
#ifdef RETAIL
    {
        BitSet<16> test;
        test.set(70000);
        test.set(-3);
        for (int i = 0; i < 8; ++i)
            EXPECT_EQ(test.get(i), 0);
    }
#endif
}

TEST(TestBitSet, BitSetIterator32BitSpecialization)
{
    // some values
    {
        uint64_t set = 32;
        uint64_t maxn = (uint64_t)1 << set;
        for (int i = 0; i < maxn; i += 65432)
        {
            BitSet<32> test;

            for (int a = 0; a < set; ++a)
            {
                if (i & (1 << a)) test.set(a);
            }

            engine::vector<int> tmp;
            for (auto&& bit : test)
                tmp.emplace_back(bit);

            uint32_t res = 0;
            for (auto&& t : tmp)
                res |= 1 << t;

            EXPECT_EQ(tmp.size(), __popcnt(i));
            EXPECT_EQ(i, res);
        }
    }

    // test some edge cases
    {
        BitSet<32> test;
        test.set(0);
        test.set(1);
        test.set(30);
        test.set(31);

        engine::vector<int> vals;
        for (auto&& v : test)
            vals.emplace_back(v);

        EXPECT_EQ(vals.size(), 4);
        EXPECT_EQ(vals[0], 0);
        EXPECT_EQ(vals[1], 1);
        EXPECT_EQ(vals[2], 30);
        EXPECT_EQ(vals[3], 31);

    }
}

TEST(TestBitSet, BitSetIterator64BitSpecialization)
{
    // test some edge cases
    {
        BitSet<64> test;
        test.set(0);
        test.set(1);
        test.set(62);
        test.set(63);

        engine::vector<int> vals;
        for (auto&& v : test)
            vals.emplace_back(v);

        EXPECT_EQ(vals.size(), 4);
        EXPECT_EQ(vals[0], 0);
        EXPECT_EQ(vals[1], 1);
        EXPECT_EQ(vals[2], 62);
        EXPECT_EQ(vals[3], 63);
    }
}

TEST(TestBitSet, BitSetIterator128BitSpecialization)
{
    // test some edge cases
    {
        BitSet<128> test;
        test.set(0);
        test.set(1);
        test.set(63);
        test.set(64);
        test.set(65);
        test.set(126);
        test.set(127);

        engine::vector<int> vals;
        for (auto&& v : test)
            vals.emplace_back(v);

        EXPECT_EQ(vals.size(), 7);
        EXPECT_EQ(vals[0], 0);
        EXPECT_EQ(vals[1], 1);
        EXPECT_EQ(vals[2], 63);
        EXPECT_EQ(vals[3], 64);
        EXPECT_EQ(vals[4], 65);
        EXPECT_EQ(vals[5], 126);
        EXPECT_EQ(vals[6], 127);

    }
}

TEST(TestBitSet, BitSetIterator512Bits)
{
    // test some edge cases
    {
        BitSet<512> test;
        test.set(0);
        test.set(1);
        test.set(63);
        test.set(64);
        test.set(65);
        test.set(126);
        test.set(127);
        test.set(254);
        test.set(255);
        test.set(256);
        test.set(510);
        test.set(511);

        engine::vector<int> vals;
        for (auto&& v : test)
            vals.emplace_back(v);

        EXPECT_EQ(vals.size(), 12);
        EXPECT_EQ(vals[0], 0);
        EXPECT_EQ(vals[1], 1);
        EXPECT_EQ(vals[2], 63);
        EXPECT_EQ(vals[3], 64);
        EXPECT_EQ(vals[4], 65);
        EXPECT_EQ(vals[5], 126);
        EXPECT_EQ(vals[6], 127);
        EXPECT_EQ(vals[7], 254);
        EXPECT_EQ(vals[8], 255);
        EXPECT_EQ(vals[9], 256);
        EXPECT_EQ(vals[10], 510);
        EXPECT_EQ(vals[11], 511);
    }

#if 0
    {
        uint64_t set = 64;
        uint64_t maxn = 0xffffffffffffffff;
        for (uint64_t i = 0; i < maxn; i += 65432ull * 65432ull * 65432ull * 65432ull)
        {
            BitSet<64> test;

            for (uint64_t a = 0; a < set; ++a)
            {
                if (i & (1ull << a)) {
                    test.set(a);
                }
            }

            engine::vector<uint64_t> tmp;
            for (auto&& bit : test)
                tmp.emplace_back(bit);

            uint64_t res = 0;
            for (auto&& t : tmp)
                res |= 1ull << t;

            EXPECT_EQ(tmp.size(), __popcnt64(i));
            EXPECT_EQ(i, res);
        }
    }

    {
        uint64_t set = 128;
        uint64_t maxn = (uint64_t)1 << set;
        for (int i = 0; i < maxn; i += 65432 * 65432 * 65432 * 65432)
        {
            BitSet<128> test;

            for (int a = 0; a < set; ++a)
            {
                if (i & (1 << a)) test.set(a);
            }

            engine::vector<int> tmp;
            for (auto&& bit : test)
                tmp.emplace_back(bit);

            uint32_t res = 0;
            for (auto&& t : tmp)
                res |= 1 << t;

            EXPECT_EQ(tmp.size(), __popcnt64(i));
            EXPECT_EQ(i, res);
        }
    }
#endif
}
