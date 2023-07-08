
struct PSInput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
};

sampler tex_sampler;

Texture2D<float4> tex;
//Texture2D<float4> tex2;

//Buffer<float4> data;

/*cbuffer data_info
{
    float2 dataSize;
};*/



float4 main(PSInput input) : SV_Target
{
    return tex.SampleLevel(tex_sampler, input.uv.xy, 0.0f);

    /*float4 tex1color = tex.SampleLevel(tex_sampler, input.uv.xy, 0);
    float4 tex2color = tex2.SampleLevel(tex_sampler, input.uv.xy, 0);

    float4 tex3color = tex.Sample(tex_sampler, input.uv.xy);
    float4 tex4color = tex2.Sample(tex_sampler, input.uv.xy);

    float4 tex5color = tex.Load(int3(input.position.xy, 0));
    float4 tex6color = tex2.Load(int3(input.position.xy, 0));

    float4 uvcolor = float4(input.uv.x, input.uv.y, 0.0f, 1.0f);*/

    /*
    float2 dataSize = float2(512.0f, 512.0f);
    float2 coords = dataSize * input.uv.xy;
    int width = (int)floor(dataSize.x);
    int height = (int)floor(dataSize.y);
    int x = (int)floor(coords.x);
    int y = (int)floor(coords.y);
    return data[(y * width) + x];
    */
}
