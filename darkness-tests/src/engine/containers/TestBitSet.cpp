#include "GlobalTestFixture.h"
#include "containers/BitSet.h"
#include <bit>
#include <bitset>




TEST(TestBitSet, BitSetIterator8BitSpecialization)
{
    

    {
        BitSet<8> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    {
        BitSet<8> test;
        test.set(0);
        engine::vector<int> tmp;
        for (auto&& bit : test)
            tmp.emplace_back(bit);
        EXPECT_EQ(tmp.size(), 1);
        EXPECT_EQ(tmp[0], 0);
    }

    // test all iterator settings
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

#if 0
    {
        for (int i = 0; i < 9; ++i)
        {
            uint8_t _mask8 = mask8((uint8_t)i);
            LOG("index: %i = %s", i, std::bitset<8>(_mask8).to_string().c_str());
        }

        for (int i = 0; i < 17; ++i)
        {
            uint16_t _mask16 = mask16((uint16_t)i);
            LOG("index: %i = %s", i, std::bitset<16>(_mask16).to_string().c_str());
        }

        for (int i = 0; i < 33; ++i)
        {
            uint32_t _mask32 = mask32((uint32_t)i);
            LOG("index: %i = %s", i, std::bitset<32>(_mask32).to_string().c_str());
        }

        for (int i = 0; i < 65; ++i)
        {
            uint64_t _mask64 = mask64((uint64_t)i);
            LOG("index: %i = %s", i, std::bitset<64>(_mask64).to_string().c_str());
        }
    }
#endif

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
