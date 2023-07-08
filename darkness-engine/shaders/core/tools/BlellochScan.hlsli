
groupshared uint2 bucket[64];

uint scan(uint3 threadId, uint groupThreadId, uint x)         // Change the type of x here if scan other types
{
    bucket[groupThreadId].x = x;
    bucket[groupThreadId].y = 0;

    // Up sweep
    [unroll]
    for (uint strideA = 2; strideA <= 64; strideA <<= 1)
    {
        GroupMemoryBarrierWithGroupSync();

        if ((groupThreadId & (strideA - 1)) == (strideA - 1))
        {
            bucket[groupThreadId].x += bucket[groupThreadId - strideA / 2].x;
        }
    }

    if (groupThreadId == (64 - 1))
    {
        bucket[groupThreadId].x = 0;
    }

    // Down sweep
    bool n = true;
    [unroll]
    for (uint stride = 64 / 2; stride >= 1; stride >>= 1)
    {
        GroupMemoryBarrierWithGroupSync();

        uint a = stride - 1;
        uint b = stride | a;

        if (n)        // ping-pong between passes
        {
            if ((groupThreadId & b) == b)
            {
                bucket[groupThreadId].y = bucket[groupThreadId - stride].x + bucket[groupThreadId].x;
            }
            else
                if ((groupThreadId & a) == a)
                {
                    bucket[groupThreadId].y = bucket[groupThreadId + stride].x;
                }
                else
                {
                    bucket[groupThreadId].y = bucket[groupThreadId].x;
                }
        }
        else
        {
            if ((groupThreadId & b) == b)
            {
                bucket[groupThreadId].x = bucket[groupThreadId - stride].y + bucket[groupThreadId].y;
            }
            else
                if ((groupThreadId & a) == a)
                {
                    bucket[groupThreadId].x = bucket[groupThreadId + stride].y;
                }
                else
                {
                    bucket[groupThreadId].x = bucket[groupThreadId].y;
                }
        }

        n = !n;
    }

    return bucket[groupThreadId].y + x;
}
