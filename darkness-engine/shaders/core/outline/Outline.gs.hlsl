
struct VsData
{
    float3 position : POSITION0;
    float3 normal   : NORMAL0;
};

struct GsData
{
    float4 position : SV_Position0;
    float4 colora   : COLOR0;
    float edgeFlag  : PSIZE;
};

cbuffer ConstData
{
    float4x4 modelMatrix;
    float4x4 modelViewMatrix;
    float4x4 jitterProjectionMatrix;
    float4x4 jitterModelViewProjectionMatrix;
    float3 cameraWorldSpacePosition;
    float lineThickness;
    float2 texelSize;
};

float3 getNormal(float3 A, float3 B, float3 C)
{
    float3 AB = normalize(A - B);
    float3 AC = normalize(A - C);
    return normalize(cross(AB, AC));
};

[maxvertexcount(12)]
void main(
    triangleadj VsData vertices[6],
    inout TriangleStream<GsData> triStream)
{
    float3 wp1 = mul(modelMatrix, float4(vertices[0].position, 1.0)).xyz;
    float3 wp2 = mul(modelMatrix, float4(vertices[2].position, 1.0)).xyz;
    float3 wp3 = mul(modelMatrix, float4(vertices[4].position, 1.0)).xyz;

    float3 faceNormal = getNormal(wp1, wp2, wp3);
    float3 basePoint = (wp1 + wp2 + wp3) / 3.0;
    float3 viewDirection = normalize(cameraWorldSpacePosition - basePoint);
    float dotView = dot(faceNormal, viewDirection);

    if (dotView > 0)
    {
        // triangle face is on front
        for (uint i = 0; i < 6; i += 2)
        {
            uint iNextTri = (i + 2) % 6;
            wp1 = mul(modelMatrix, float4(vertices[i].position, 1.0)).xyz;
            wp2 = mul(modelMatrix, float4(vertices[i+1].position, 1.0)).xyz;
            wp3 = mul(modelMatrix, float4(vertices[iNextTri].position, 1.0)).xyz;

            faceNormal = getNormal(wp1, wp2, wp3);
            float3 basePoint = (wp1 + wp2 + wp3) / 3.0;
            viewDirection = normalize(cameraWorldSpacePosition - basePoint);
            dotView = dot(faceNormal, viewDirection);

            if (dotView < 0)
            {
                float2 thickness = lineThickness * 0.7;

                float3 fromView = mul(modelViewMatrix, float4(vertices[i].position, 1)).xyz;
                float3 toView = mul(modelViewMatrix, float4(vertices[iNextTri].position, 1)).xyz;

                float4 fromScreen = mul(jitterProjectionMatrix, float4(fromView, 1.0));
                float4 toScreen = mul(jitterProjectionMatrix, float4(toView, 1.0));

                fromScreen.xyz /= fromScreen.w;
                toScreen.xyz /= toScreen.w;

                if ((fromScreen.z > 0 && fromScreen.z < 1) &&
                    (toScreen.z > 0 && toScreen.z < 1))
                {
                    float2 edge = normalize(toScreen.xy - fromScreen.xy);
                    float2 tangent = float2(edge.y, -edge.x);

                    float2 move = tangent * thickness * texelSize;
                    //float2 move = normalize(tangent) / 10;

                    GsData data;
                    data.position = float4(fromScreen.xy + move, fromScreen.z, 1.0);
                    data.colora = float4(1.0, 0.0, 1.0, 1.0);
                    data.edgeFlag = 0;
                    triStream.Append(data);

                    GsData data2;
                    data2.position = float4(toScreen.xy + move, toScreen.z, 1.0);
                    data2.colora = float4(1.0, 0.0, 1.0, 1.0);
                    data2.edgeFlag = 0;
                    triStream.Append(data2);

                    GsData data3;
                    data3.position = float4(toScreen.xy - move, toScreen.z, 1.0);
                    data3.colora = float4(1.0, 0.0, 1.0, 1.0);
                    data3.edgeFlag = 0;
                    triStream.Append(data3);

                    GsData data4;
                    data4.position = float4(fromScreen.xy + move, fromScreen.z, 1.0);
                    data4.colora = float4(1.0, 0.0, 1.0, 1.0);
                    data4.edgeFlag = 0;
                    triStream.Append(data4);

                    GsData data5;
                    data5.position = float4(fromScreen.xy - move, fromScreen.z, 1.0);
                    data5.colora = float4(1.0, 0.0, 1.0, 1.0);
                    data5.edgeFlag = 0;
                    triStream.Append(data5);

                    GsData data6;
                    data6.position = float4(toScreen.xy - move, fromScreen.z, 1.0);
                    data6.colora = float4(1.0, 0.0, 1.0, 1.0);
                    data6.edgeFlag = 0;
                    triStream.Append(data6);

                    triStream.RestartStrip();
                }
            }
        }
    }
}
