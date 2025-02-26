
struct meshInfo {
    uint vertexCount;
    uint BonePerVertex;
};

ConstantBuffer<meshInfo> g_MeshInfo : register(b0);        
StructuredBuffer<matrix> g_OffsetMatrix : register(t0);
StructuredBuffer<uint> g_BoneIndices : register(t1);
StructuredBuffer<float> g_BoneWeight : register(t2);
StructuredBuffer<uint> g_AnimationMatrixIndex : register(t3);
StructuredBuffer<matrix> g_AnimationMatrix : register(t4);


StructuredBuffer<float3> g_InsertVertexBuffer : register(t5);
// RW
RWStructuredBuffer<float3> g_OutputVertexBuffer : register(u0);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_MeshInfo.vertexCount)
    {
        uint BonePerVertex = g_MeshInfo.BonePerVertex;
        float3 weightPos = float3(0.0, 0.0, 0.0);
        for (int i = 0; i < g_MeshInfo.BonePerVertex; ++i)
        {
            weightPos += (g_BoneWeight[(DTid.x * BonePerVertex) + i] *
            (mul(float4(g_InsertVertexBuffer[DTid.x], 1.0f),
            mul(g_OffsetMatrix[g_BoneIndices[(DTid.x * BonePerVertex) + i]],
            g_AnimationMatrix[g_AnimationMatrixIndex[g_BoneIndices[(DTid.x * BonePerVertex) + i]]])).xyz));
        }
        g_OutputVertexBuffer[DTid.x] = weightPos;
    }
}