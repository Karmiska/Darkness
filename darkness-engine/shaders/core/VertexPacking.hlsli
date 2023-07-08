
uint2 packVertex(float3 vertex, VertexScale vertexScale)
{
    // 21 bits per vertex value = 63 bits = 8 bytes
    float3 memBits = float3( 2097151.0f, 2097151.0f, 2097151.0f );
    float3 scaledVert = ((vertex - vertexScale.origo) / vertexScale.range) * memBits;
    uint xpart = (uint)(scaledVert.x);
    uint ypart = (uint)(scaledVert.y);
    uint zpart = (uint)(scaledVert.z);
    return uint2(
        ((xpart & 0x1fffff) << 11) | (ypart & 0x7ff),               // xpart 21 bits, ypart 11 low bits
        ((zpart & 0x1fffff) << 11) | ((ypart & 0x1ff800) >> 10) );   // zpart 21 bits, ypart 10 high bits, 1 FREE BIT
};

float3 unpackVertex(uint2 vertex, VertexScale vertexScale)
{
    uint xpart = (vertex.x & 0xfffff800) >> 11;
    uint zpart = (vertex.y & 0xfffff800) >> 11;
    uint ypart = (vertex.x & 0x7ff) | ((vertex.y & 0x7fe) << 10);

    float3 memBits = float3( 2097151.0f, 2097151.0f, 2097151.0f );

    return float3(
        (((float)xpart / memBits.x) * vertexScale.range.x) + vertexScale.origo.x,
        (((float)ypart / memBits.y) * vertexScale.range.y) + vertexScale.origo.y,
        (((float)zpart / memBits.z) * vertexScale.range.z) + vertexScale.origo.z
    );
};
