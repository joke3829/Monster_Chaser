
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
    int i;
    i = g_MeshInfo.BonePerVertex;
}