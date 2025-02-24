
struct meshInfo {
    uint vertexCount;
    uint BonePerVertex;
};

ConstantBuffer<meshInfo> g_MeshInfo : register(c0, space0);        
StructuredBuffer<matrix> g_OffsetMatrix : register(t0, space0);
StructuredBuffer<uint> g_BoneIndices : register(t1, space0);
StructuredBuffer<float> g_BoneWeight : register(t2, space0);
StructuredBuffer<uint> g_AnimationMatrixIndex : register(t3, space0);
StructuredBuffer<matrix> g_AnimationMatrix : register(t4, space0);


StructuredBuffer<float3> g_InsertVertexBuffer : register(t5, space0);
// RW
RWStructuredBuffer<float3> g_OutputVertexBuffer : register(u6, space0);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    
}