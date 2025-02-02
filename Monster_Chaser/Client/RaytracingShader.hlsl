
struct Payload
{
    float3 RayColor;
};

struct CameraInfo
{
    matrix mtxViewProj;
    matrix mtxInverseViewProj;
    float3 cameraEye;
};

struct HasMaterial
{
    bool bHasAlbedoColor;
    bool bHasEmissiveColor;
    bool bHasSpecularColor;
    bool bHasGlossiness;
    bool bHasSmoothness;
    bool bHasMetallic;
    bool bHasSpecularHighlight;
    bool bHasGlossyReflection;
    
    bool bHasAlbedoMap;
    bool bHasSpecularMap;
    bool bHasNormalMap;
    bool bHasMetallicMap;
    bool bHasEmissionMap;
    bool bHasDetailAlbedoMap;
    bool bHasDetailNormalMap;
    
    float4 AlbedoColor;
    float4 EmissiveColor;
    float4 SpecularColor;
    float Glossiness;
    float Smoothness;
    float Metallic;
    float SpecularHighlight;
    float GlossyReflection;
};

struct HasMesh
{
    bool bHasVertex;
    bool bHasColor;
    bool bHasTex0;
    bool bHasTex1;
    bool bHasNormals;
    bool bHasTangenrs;
    bool bHasBiTangents;
    bool bHasSubMeshes;
};

// Global Root Signature ============================================
RaytracingAccelerationStructure g_Scene : register(t0, space0);

RWTexture2D<float4> uav : register(u0);

ConstantBuffer<CameraInfo> g_CameraInfo : register(b0);

SamplerState g_Sampler : register(s0);
// ==================================================================

// Local Root Signature =============================================
ConstantBuffer<HasMaterial> l_Material : register(b1, space0);
ConstantBuffer<HasMesh> l_Mesh : register(b1, space1);

StructuredBuffer<float3> l_Vertices : register(t1, space0);
StructuredBuffer<float4> l_Colors : register(t1, space1);
StructuredBuffer<float2> l_Tex0 : register(t1, space2);
StructuredBuffer<float2> l_Tex1 : register(t1, space3);
StructuredBuffer<float3> l_Normals : register(t1, space4);
StructuredBuffer<float3> l_Tangents : register(t1, space5);
StructuredBuffer<float3> l_BiTangents : register(t1, space6);
StructuredBuffer<uint> l_Indices : register(t1, space7);

Texture2D l_AlbedoMap : register(t2, space0);
Texture2D l_SpecularMap : register(t2, space1);
Texture2D l_NormalMap : register(t2, space2);
Texture2D l_MetallicMap : register(t2, space3);
Texture2D l_EmissionMap : register(t2, space4);
Texture2D l_DetailAlbedoMap : register(t2, space5);
Texture2D l_DetailNormalMap : register(t2, space6);
// ==================================================================

[shader("raygeneration")]
void RayGenShader()
{
    uint2 idx = DispatchRaysIndex().xy;
    float2 size = DispatchRaysDimensions().xy;
    float2 uv = idx / size;
    float3 target = float3((uv.x * 2 - 1) * 1.8 * (size.x / size.y), (1 - uv.y) * 4 - 2 + camera.y, 0);
    RayDesc ray;
    ray.Origin = camera;
    ray.Direction = target - camera;
    ray.TMin = 0.001;
    ray.TMax = 1000;
    
    Payload payload;
    
    TraceRay(g_Scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    
    uav[idx] = float4(payload.RayColor, 1.0f);
}

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.RayColor = float3(0.0, 0.8, 1.0);
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attrib)
{
    float2 uvs[3] = { float2(0.0, 0.0), float2(0.0, 0.0), float2(0.0, 0.0) };
    uint index[3];
    uint idx;
    if (l_Mesh.bHasSubMeshes) {
        idx = PrimitiveIndex() * 3;
        index[0] = l_Indices[idx];
        index[1] = l_Indices[idx + 1];
        index[2] = l_Indices[idx + 2];
        uvs[0] = l_Tex0[index[0]];
        uvs[1] = l_Tex0[index[1]];
        uvs[2] = l_Tex0[index[2]];
    }
    
    float2 texCoord = uvs[0] * (1.0f - attrib.barycentrics.x - attrib.barycentrics.y) +
    uvs[1] * attrib.barycentrics.x + uvs[2] * attrib.barycentrics.y;
    
    if (l_Material.bHasAlbedoMap) {
        float4 color = l_AlbedoMap.SampleLevel(g_Sampler, texCoord, 0);
        payload.RayColor = color.xyz;
    }
    else
    {
        payload.RayColor = float3(1.0, 1.0, 0.0);
    }
}