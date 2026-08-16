struct Uniforms {
    uint2 resolution;
    float time;
};

[[vk::binding(0, 0)]]
ConstantBuffer<Uniforms> inp;

[[vk::binding(1, 0)]]
[format("rgba8")]
RWTexture2D<float4> outImg;

[shader("compute")]
[numthreads(16,16,1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    uint2 pixel = threadId.xy;
    if (pixel.x >= inp.resolution.x || pixel.y >= inp.resolution.y) return;

    float2 uv = (float2)pixel / (float2)inp.resolution;
    float4 out_color = float4(uv, sin(inp.time) * .5 + .5, 1.0);
    outImg[pixel] = out_color;
}
