
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

// 0 == false, 1 == true
struct HasMaterial
{
    int bHasAlbedoColor;
    int bHasEmissiveColor;
    int bHasSpecularColor;
    int bHasGlossiness;
    int bHasSmoothness;
    int bHasMetallic;
    int bHasSpecularHighlight;
    int bHasGlossyReflection;
    
    int bHasAlbedoMap;
    int bHasSpecularMap;
    int bHasNormalMap;
    int bHasMetallicMap;
    int bHasEmissionMap;
    int bHasDetailAlbedoMap;
    int bHasDetailNormalMap;
    
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
    int bHasVertex;
    int bHasColor;
    int bHasTex0;
    int bHasTex1;
    int bHasNormals;
    int bHasTangenrs;
    int bHasBiTangents;
    int bHasSubMeshes;
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
    float2 xy = DispatchRaysIndex().xy + 0.5f;
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
    screenPos.y = -screenPos.y;
    
    float4 world = mul(float4(screenPos, 0, 1), g_CameraInfo.mtxInverseViewProj);
    world.xyz /= world.w;
    
    RayDesc ray;
    ray.Origin = g_CameraInfo.cameraEye;
    ray.Direction = normalize(world.xyz - ray.Origin);
    ray.TMin = 0.001;
    ray.TMax = 10000;
    
    Payload payload;
    
    TraceRay(g_Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 1, 0, ray, payload);
    
    uav[DispatchRaysIndex().xy] = float4(payload.RayColor, 1.0f);
}

[shader("miss")]
void Miss(inout Payload payload)
{
    float slope = normalize(WorldRayDirection()).y;
    float t = saturate(slope * 5 + 0.5);
    
    float skyTop = float3(0.24, 0.44, 0.72);
    float skyBottom = float3(0.75, 0.86, 0.93);
    
    payload.RayColor = lerp(skyBottom, skyTop, t);
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attrib)
{
    //payload.RayColor = float3(1.0, 0.0, 0.0);
    
    float2 uvs[3] = { float2(0.0, 0.0), float2(0.0, 0.0), float2(0.0, 0.0) };
    uint index[3];
    uint idx;
    idx = PrimitiveIndex() * 3;
    if (l_Mesh.bHasSubMeshes != 0)
    {
        index[0] = l_Indices[idx];
        index[1] = l_Indices[idx + 1];
        index[2] = l_Indices[idx + 2];
        if (l_Mesh.bHasTex0 != 0)
        {
            uvs[0] = l_Tex0[index[0]];
            uvs[1] = l_Tex0[index[1]];
            uvs[2] = l_Tex0[index[2]];
        }
    }
    else
    {
        if (l_Mesh.bHasTex0 != 0)
        {
            uvs[0] = l_Tex0[idx];
            uvs[1] = l_Tex0[idx + 1];
            uvs[2] = l_Tex0[idx + 2];
        }
    }
    
    
    float2 texCoord = uvs[0] * (1.0f - attrib.barycentrics.x - attrib.barycentrics.y) +
    uvs[1] * attrib.barycentrics.x + uvs[2] * attrib.barycentrics.y;

    if (l_Material.bHasAlbedoMap != 0)
    {
        payload.RayColor = l_AlbedoMap.SampleLevel(g_Sampler, texCoord, 0).xyz;
    }
    else if(l_Material.bHasAlbedoColor != 0)
        payload.RayColor = l_Material.AlbedoColor.xyz;
    else
        payload.RayColor = float3(0.0, 0.0, 0.0);
    
}