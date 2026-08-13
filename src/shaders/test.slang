// struct Uniforms {
//     float2 resolution;
//     float time;
// };

// [[vk::binding(0, 0)]]
// ConstantBuffer<Uniforms> inp;

[[vk::binding(1, 0)]]
[format("rgba8")]
RWTexture2D<float4> outImg;

[shader("compute")]
[numthreads(16,16,1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    uint2 pixel = threadId.xy;
    // float2 uv = pixel / inp.resolution;
    float2 uv = (float2)pixel / 1024.0;
    float4 out_color = float4(uv, .5, 1.0);
    outImg[pixel] = out_color;
}
