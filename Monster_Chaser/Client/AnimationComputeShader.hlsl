
struct meshInfo {
    uint vertexCount;
    uint BonePerVertex;
};

ConstantBuffer<meshInfo> g_MeshInfo : register(b0, space0);        
StructuredBuffer<float4x4> g_OffsetMatrix : register(t0, space0);
StructuredBuffer<uint> g_BoneIndices : register(t1, space0);
StructuredBuffer<float> g_BoneWeight : register(t2, space0);
StructuredBuffer<uint> g_AnimationMatrixIndex : register(t3, space0);
StructuredBuffer<float4x4> g_AnimationMatrix : register(t4, space0);

StructuredBuffer<float3> g_InsertVertexBuffer : register(t5, space0);
// RW
RWStructuredBuffer<float3> g_OutputVertexBuffer : register(u0, space0);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_MeshInfo.vertexCount)
    {
        uint BonePerVertex = g_MeshInfo.BonePerVertex;
        float3 weightPos = float3(0.0, 0.0, 0.0);
        for (int i = 0; i < BonePerVertex; ++i)
        {
            uint boneIndex = g_BoneIndices[(DTid.x * BonePerVertex) + i];
            float boneWeight = g_BoneWeight[(DTid.x * BonePerVertex) + i];
            
            float4 input = float4(g_InsertVertexBuffer[DTid.x], 1.0f);
            float4x4 boneMatrix = mul(g_OffsetMatrix[boneIndex], g_AnimationMatrix[g_AnimationMatrixIndex[boneIndex]]);
            //float4x4 boneMatrix = g_AnimationMatrix[g_AnimationMatrixIndex[boneIndex]];
            weightPos += boneWeight * mul(input, boneMatrix).xyz;
        }
        g_OutputVertexBuffer[DTid.x] = weightPos;
    }
}