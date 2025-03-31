
struct meshInfo {
    uint vertexCount;
    uint BonePerVertex;
};

struct bufferInfo
{
    int bHasNormal;
    int bHasTangent;
    int bHasBiTangnet;
};

// Skinning Range
ConstantBuffer<meshInfo> g_MeshInfo : register(b0, space0);        
StructuredBuffer<float4x4> g_OffsetMatrix : register(t0, space0);
StructuredBuffer<uint> g_BoneIndices : register(t1, space0);
StructuredBuffer<float> g_BoneWeight : register(t2, space0);
StructuredBuffer<uint> g_AnimationMatrixIndex : register(t3, space0);
StructuredBuffer<float4x4> g_AnimationMatrix : register(t4, space0);

ConstantBuffer<bufferInfo> g_BufferInfo : register(b1, space0);
StructuredBuffer<float3> g_InsertVertexBuffer : register(t5, space0);
StructuredBuffer<float3> g_InsertNormalBuffer : register(t6, space0);
StructuredBuffer<float3> g_InsertTangentBuffer : register(t7, space0);
StructuredBuffer<float3> g_InsertBiTangentBuffer : register(t8, space0);
// RW
RWStructuredBuffer<float3> g_OutputVertexBuffer : register(u0, space0);
RWStructuredBuffer<float3> g_OutputNormalBuffer : register(u1, space0);
RWStructuredBuffer<float3> g_OutputTangentBuffer : register(u2, space0);
RWStructuredBuffer<float3> g_OutputBiTangentBuffer : register(u3, space0);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_MeshInfo.vertexCount)
    {
        uint BonePerVertex = g_MeshInfo.BonePerVertex;
        float4x4 weightMatrix = (float4x4) 0.0f;
        if (0 != g_BufferInfo.bHasTangent)  // exist TBN coordinate
        {
            for (int i = 0; i < BonePerVertex; ++i)
            {
                uint boneIndex = g_BoneIndices[(DTid.x * BonePerVertex) + i];
                weightMatrix += g_BoneWeight[(DTid.x * BonePerVertex) + i] * mul(g_OffsetMatrix[boneIndex],
            g_AnimationMatrix[g_AnimationMatrixIndex[boneIndex]]);
            }
            g_OutputVertexBuffer[DTid.x] = mul(float4(g_InsertVertexBuffer[DTid.x], 1.0), weightMatrix).xyz;
            g_OutputNormalBuffer[DTid.x] = mul(g_InsertNormalBuffer[DTid.x], (float3x3)weightMatrix);
            g_OutputTangentBuffer[DTid.x] = mul(g_InsertTangentBuffer[DTid.x], (float3x3) weightMatrix);
            g_OutputBiTangentBuffer[DTid.x] = mul(g_InsertBiTangentBuffer[DTid.x], (float3x3) weightMatrix);
        }
        else if (0 != g_BufferInfo.bHasNormal)  // exist NormaBuffer
        {
            for (int i = 0; i < BonePerVertex; ++i)
            {
                uint boneIndex = g_BoneIndices[(DTid.x * BonePerVertex) + i];
                weightMatrix += g_BoneWeight[(DTid.x * BonePerVertex) + i] * mul(g_OffsetMatrix[boneIndex],
            g_AnimationMatrix[g_AnimationMatrixIndex[boneIndex]]);
            }
            g_OutputVertexBuffer[DTid.x] = mul(float4(g_InsertVertexBuffer[DTid.x], 1.0), weightMatrix).xyz;
            g_OutputNormalBuffer[DTid.x] = mul(g_InsertNormalBuffer[DTid.x], (float3x3) weightMatrix);
        }
        else                                    // only Vertex
        {
            for (int i = 0; i < BonePerVertex; ++i)
            {
                uint boneIndex = g_BoneIndices[(DTid.x * BonePerVertex) + i];
                weightMatrix += g_BoneWeight[(DTid.x * BonePerVertex) + i] * mul(g_OffsetMatrix[boneIndex],
            g_AnimationMatrix[g_AnimationMatrixIndex[boneIndex]]);
            }
            g_OutputVertexBuffer[DTid.x] = mul(float4(g_InsertVertexBuffer[DTid.x], 1.0), weightMatrix).xyz;
        }
    }
}

/*
for (int i = 0; i < BonePerVertex; ++i)
            {
                uint boneIndex = g_BoneIndices[(DTid.x * BonePerVertex) + i];
                float boneWeight = g_BoneWeight[(DTid.x * BonePerVertex) + i];
            
                float4 input = float4(g_InsertVertexBuffer[DTid.x], 1.0f);
                float4x4 boneMatrix = mul(g_OffsetMatrix[boneIndex],
            g_AnimationMatrix[g_AnimationMatrixIndex[boneIndex]]);
                weightPos += boneWeight * mul(input, boneMatrix).xyz;
            }
            g_OutputVertexBuffer[DTid.x] = weightPos;
*/