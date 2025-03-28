
struct RadiancePayload
{
    float4 RayColor;
    uint RayDepth;
};

struct ShadowPayload
{
    bool bShadow;
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

#define MAX_RAY_DEPTH 3
#define RADIANCE_OFFSET 0
#define RADIANCE_MISS_OFFSET 0
#define SHADOW_OFFSET 1
#define SHADOW_MISS_OFFSET 1
#define GEOMETRY_STRIDE 2       // num RayType

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
// =============================================================================

inline float3 GetWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float2 GetTexCoord(in float2 uvs[3], in float2 barycentrics)
{
    return uvs[0] * (1.0f - barycentrics.x - barycentrics.y) +
    uvs[1] * barycentrics.x + uvs[2] * barycentrics.y;
}

float3 GetHitNormal(in float3 normals[3], in float2 barycentrics)
{
    return normals[0] * (1.0f - barycentrics.x - barycentrics.y) +
    normals[1] * barycentrics.x + normals[2] * barycentrics.y;
}

float4 TraceRadianceRay(in RayDesc ray, uint currentRayDepth)
{
    if (currentRayDepth >= MAX_RAY_DEPTH)
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    RadiancePayload payload = { float4(0.0f, 0.0f, 0.0f, 0.0f), currentRayDepth + 1 };
    TraceRay(g_Scene,
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
    ~0,
    RADIANCE_OFFSET,
    GEOMETRY_STRIDE,
    RADIANCE_MISS_OFFSET,
    ray, payload);

    return payload.RayColor;
}

float4 CalculatePhongModel(float4 diffuseColor, float3 normal)
{
    
    float3 normalW = normalize(mul(normal, (float3x3) ObjectToWorld4x3()));
    
    //float3 lightColor = float3(0.8, 0.4, 0.2);
    float3 lightColor = float3(0.8, 0.8, 0.8);
    float3 light = normalize(float3(1.0, 1.0, 1.0));
    
    float Diffuse = max(dot(normalW, light), 0.0f);
    float3 PhongD = Diffuse * lightColor * diffuseColor.xyz;
    
    float3 PhongS = float3(0.0, 0.0, 0.0);
    if (l_Material.bHasSpecularHighlight)
    {
        float3 Ref1 = 2.0f * normalW * dot(normalW, light) - light;
        float3 View = normalize(g_CameraInfo.cameraEye - GetWorldPosition());
        float Specular = pow(max(dot(Ref1, View), 0.0f), l_Material.SpecularHighlight);
        if(Diffuse <= 0.0f)
            Specular = 0.0f;
        PhongS = Specular * l_Material.SpecularColor.xyz * lightColor;
    }
    
    float3 PhongA = 0.4 * diffuseColor.xyz;
    
    return float4(PhongD + PhongS + PhongA, 1.0f);
}

bool CheckTheShadow(in RayDesc ray, uint currentRayDepth)
{
    if (currentRayDepth >= MAX_RAY_DEPTH)
        return false;
    
    // 조명 개수에 따라 검사하도록 코드 예정
    
    ShadowPayload payload = { false };
    TraceRay(g_Scene,
    RAY_FLAG_FORCE_OPAQUE,
    ~0,
    SHADOW_OFFSET,
    GEOMETRY_STRIDE,
    SHADOW_MISS_OFFSET,
    ray, payload);
    
    return payload.bShadow;
}

// =============================================================================

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
    ray.TMax = 1000;
    
    float4 color = TraceRadianceRay(ray, 0);
    
    uav[DispatchRaysIndex().xy] = float4(color.xyz, 1.0f);
}

[shader("miss")]
void RadianceMiss(inout RadiancePayload payload)
{
    float slope = normalize(WorldRayDirection()).y;
    float t = saturate(slope * 5 + 0.5);
    
    float3 skyTop = float3(0.24, 0.44, 0.72);
    float3 skyBottom = float3(0.75, 0.86, 0.93);
    float3 skycolor = lerp(skyBottom, skyTop, t);
        
        
    payload.RayColor.xyz = skycolor;
}

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload)
{
    payload.bShadow = false;
}

[shader("anyhit")]
void RadianceAnyHit(inout RadiancePayload payload, in BuiltInTriangleIntersectionAttributes attrib)
{
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
    
    float2 bary = float2(attrib.barycentrics.x, attrib.barycentrics.y);
    float2 texCoord = GetTexCoord(uvs, bary);

    float AlphaValue;
        
    if (l_Material.bHasAlbedoMap != 0)
    {
        AlphaValue = l_AlbedoMap.SampleLevel(g_Sampler, texCoord, 0).a;
    }
    else if (l_Material.bHasAlbedoColor != 0)
        AlphaValue = l_Material.AlbedoColor.a;
    else
        AlphaValue = 1.0f;
    if(AlphaValue <= 0.01)
        IgnoreHit();
}

[shader("closesthit")]
void RadianceClosestHit(inout RadiancePayload payload, in BuiltInTriangleIntersectionAttributes attrib)
{
    float2 uvs[3] = { float2(0.0, 0.0), float2(0.0, 0.0), float2(0.0, 0.0) };
    float3 normals[3] = { float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0) };
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
        if (l_Mesh.bHasNormals)
        {
            normals[0] = l_Normals[index[0]];
            normals[1] = l_Normals[index[1]];
            normals[2] = l_Normals[index[2]];
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
        if (l_Mesh.bHasNormals)
        {
            normals[0] = l_Normals[idx];
            normals[1] = l_Normals[idx + 1];
            normals[2] = l_Normals[idx + 2];
        }
    }
    
    float2 bary = float2(attrib.barycentrics.x, attrib.barycentrics.y);
    float2 texCoord = GetTexCoord(uvs, bary);

    float4 objectColor;
        
    if (l_Material.bHasAlbedoMap != 0)
    {
        objectColor = l_AlbedoMap.SampleLevel(g_Sampler, texCoord, 0);
    }
    else if (l_Material.bHasAlbedoColor != 0)
        objectColor = l_Material.AlbedoColor;
    else
        objectColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
    
    RayDesc shadowray;
    shadowray.Origin = GetWorldPosition();
    shadowray.Direction = normalize(float3(1.0, 1.0, 1.0));
    shadowray.TMin = 0.001;
    shadowray.TMax = 1000;
    
    bool bShadow = CheckTheShadow(shadowray, payload.RayDepth + 1);
    
    objectColor = CalculatePhongModel(objectColor, GetHitNormal(normals, bary));
    if (bShadow)
        objectColor.xyz /= 2;
    
    payload.RayColor = objectColor;
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attrib)
{
    payload.bShadow = true;
}