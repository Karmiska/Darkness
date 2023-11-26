#include "GlobalTestFixture.h"
#include "containers/BitSet.h"
#include <bit>
#include <bitset>


TEST(TestBitSetDynamic, BitSetDynamicIterator)
{
    // check that empty bitset works
    {
        BitSetDynamic test(8);
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that size 0 bitset works
    {
        BitSetDynamic test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSetDynamic test(8);
        for (int i = 0; i < 8; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 8);
        for (int i = 0; i < 8; ++i)
            EXPECT_EQ(bits[i], i);
    }

    // test all iterator values
    {
        int set = 8;
        int maxn = 1 << set;
        for (int i = 0; i < maxn; ++i)
        {
            BitSetDynamic test(8);

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

    // check that empty bitset works
    {
        BitSetDynamic test(16);
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSetDynamic test(16);
        for (int i = 0; i < 16; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 16);
        for (int i = 0; i < 16; ++i)
            EXPECT_EQ(bits[i], i);
    }

    // test all iterator values
    {
        int set = 16;
        int maxn = 1 << set;
        for (int i = 0; i < maxn; ++i)
        {
            BitSetDynamic test(16);

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

    // check that empty bitset works
    {
        BitSetDynamic test(768);
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSetDynamic test(768);
        for (int i = 0; i < 768; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 768);
        for (int i = 0; i < 768; ++i)
            EXPECT_EQ(bits[i], i);
    }

    {
        // test iterator starting position and resize
        BitSetDynamic test(65536);
        test.set(12345);
        test.set(23456);
        test.set(34567);
        test.set(45678);
        test.set(56789);
        {
            engine::vector<uint64_t> vals;
            auto iterator = BitSetDynamic::Iterator(&test, 34000);
            for (; iterator != test.end(); ++iterator)
                vals.emplace_back(*iterator);

            EXPECT_EQ(vals.size(), 3);
            EXPECT_EQ(vals[0], 34567);
            EXPECT_EQ(vals[1], 45678);
            EXPECT_EQ(vals[2], 56789);
        }
        {
            test.resize(100000);
            engine::vector<uint64_t> vals;
            auto iterator = BitSetDynamic::Iterator(&test, 34000);
            for (; iterator != test.end(); ++iterator)
                vals.emplace_back(*iterator);

            EXPECT_EQ(vals.size(), 3);
            EXPECT_EQ(vals[0], 34567);
            EXPECT_EQ(vals[1], 45678);
            EXPECT_EQ(vals[2], 56789);
        }
    }

    // test some edge cases
    {
        BitSetDynamic test(768);
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
        test.set(766);
        test.set(767);

        engine::vector<int> vals;
        for (auto&& v : test)
            vals.emplace_back(v);

        EXPECT_EQ(vals.size(), 14);
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
        EXPECT_EQ(vals[12], 766);
        EXPECT_EQ(vals[13], 767);
    }
}

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

    // check that full bitset works
    {
        BitSet<8> test;
        for (int i = 0; i < 8; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 8);
        for (int i = 0; i < 8; ++i)
            EXPECT_EQ(bits[i], i);
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
    // check that empty bitset works
    {
        BitSet<16> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSet<16> test;
        for (int i = 0; i < 16; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 16);
        for (int i = 0; i < 16; ++i)
            EXPECT_EQ(bits[i], i);
    }

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
    // check that empty bitset works
    {
        BitSet<32> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSet<32> test;
        for (int i = 0; i < 32; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 32);
        for (int i = 0; i < 32; ++i)
            EXPECT_EQ(bits[i], i);
    }

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
    // check that empty bitset works
    {
        BitSet<64> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSet<64> test;
        for (int i = 0; i < 64; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 64);
        for (int i = 0; i < 64; ++i)
            EXPECT_EQ(bits[i], i);
    }

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
    // check that empty bitset works
    {
        BitSet<128> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSet<128> test;
        for (int i = 0; i < 128; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 128);
        for (int i = 0; i < 128; ++i)
            EXPECT_EQ(bits[i], i);
    }

    // test iterator starting position
    {
        BitSet<128> test;
        test.set(20);
        test.set(30);
        test.set(80);
        test.set(90);

        engine::vector<int> vals;
        auto iterator = BitSet<128>::Iterator(&test, 70);
        for (; iterator != test.end(); ++iterator)
            vals.emplace_back(*iterator);

        EXPECT_EQ(vals.size(), 2);
        EXPECT_EQ(vals[0], 80);
        EXPECT_EQ(vals[1], 90);
    }

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
    // check that empty bitset works
    {
        BitSet<512> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that size 0 bitset works
    {
        BitSet<0> test;
        int i = 0;
        for (auto&& bit : test)
            ++i;
        EXPECT_EQ(i, 0);
    }

    // check that full bitset works
    {
        BitSet<512> test;
        for (int i = 0; i < 512; ++i)
            test.set(i);
        engine::vector<int> bits;
        for (auto&& bit : test)
            bits.emplace_back(bit);
        EXPECT_EQ(bits.size(), 512);
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(bits[i], i);
    }

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
}
