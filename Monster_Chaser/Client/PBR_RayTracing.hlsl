
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
    int bNormalMapping;
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

#define MAX_RAY_DEPTH 6
#define RADIANCE_OFFSET 0
#define RADIANCE_MISS_OFFSET 0
#define SHADOW_OFFSET 1
#define SHADOW_MISS_OFFSET 1
#define GEOMETRY_STRIDE 2       // num RayType

#define PI 3.141592

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

void GetTex0FromBuffer(inout float2 uvs[3], in uint idx)
{
    if (0 != l_Mesh.bHasSubMeshes)
    {
        uint index[3] = { l_Indices[idx], l_Indices[idx + 1], l_Indices[idx + 2] };
        uvs[0] = l_Tex0[index[0]];
        uvs[1] = l_Tex0[index[1]];
        uvs[2] = l_Tex0[index[2]];
    }
    else
    {
        uvs[0] = l_Tex0[idx];
        uvs[1] = l_Tex0[idx + 1];
        uvs[2] = l_Tex0[idx + 2];
    }
}

void GetNormalFromBuffer(inout float3 normals[3], in uint idx)
{
    if (0 != l_Mesh.bHasSubMeshes)
    {
        uint index[3] = { l_Indices[idx], l_Indices[idx + 1], l_Indices[idx + 2] };
        normals[0] = l_Normals[index[0]];
        normals[1] = l_Normals[index[1]];
        normals[2] = l_Normals[index[2]];
    }
    else
    {
        normals[0] = l_Normals[idx];
        normals[1] = l_Normals[idx + 1];
        normals[2] = l_Normals[idx + 2];
    }
}

void GetTangentFromBuffer(inout float3 tangent[3], in uint idx)
{
    if (0 != l_Mesh.bHasSubMeshes)
    {
        uint index[3] = { l_Indices[idx], l_Indices[idx + 1], l_Indices[idx + 2] };
        tangent[0] = l_Tangents[index[0]];
        tangent[1] = l_Tangents[index[1]];
        tangent[2] = l_Tangents[index[2]];
    }
    else
    {
        tangent[0] = l_Tangents[idx];
        tangent[1] = l_Tangents[idx + 1];
        tangent[2] = l_Tangents[idx + 2];
    }
}

void GetBiTangentFromBuffer(inout float3 biTangent[3], in uint idx)
{
    if (0 != l_Mesh.bHasSubMeshes)
    {
        uint index[3] = { l_Indices[idx], l_Indices[idx + 1], l_Indices[idx + 2] };
        biTangent[0] = l_BiTangents[index[0]];
        biTangent[1] = l_BiTangents[index[1]];
        biTangent[2] = l_BiTangents[index[2]];
    }
    else
    {
        biTangent[0] = l_BiTangents[idx];
        biTangent[1] = l_BiTangents[idx + 1];
        biTangent[2] = l_BiTangents[idx + 2];
    }
}

inline float3 GetWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float2 GetInterpolationHitFloat2(in float2 f2[3], in float2 barycentrics)
{
    return f2[0] * (1.0f - barycentrics.x - barycentrics.y) +
    f2[1] * barycentrics.x + f2[2] * barycentrics.y;
}

float3 GetInterpolationHitFloat3(in float3 f3[3], in float2 barycentrics)
{
    return f3[0] * (1.0f - barycentrics.x - barycentrics.y) +
    f3[1] * barycentrics.x + f3[2] * barycentrics.y;
}

float3 GetHitNormalFromNormalMap(float3 T, float3 B, float3 N, float2 uv)
{
    float3x3 TBN = float3x3(normalize(T), normalize(B), normalize(N));
    float3 MappedNormal = normalize(l_NormalMap.SampleLevel(g_Sampler, uv, 0).rgb * 2.0f - 1.0f);
    return normalize(mul(MappedNormal, TBN));
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

float4 CalculatePhongModel(float4 diffuseColor, float3 normal, bool isShadow)
{
    
    float3 normalW = normalize(mul(normal, (float3x3) ObjectToWorld4x3()));
    float shadowFactor = isShadow ? 0.35f : 1.0f;
    
    //float3 lightColor = float3(0.8, 0.4, 0.2);
    float3 lightColor = float3(0.8, 0.8, 0.8);
    //float3 lightColor = float3(1.0, 0.2, 0.2);
    float3 light = normalize(float3(0.5, 2.0, 0.7));
    
    float Diffuse = max(dot(normalW, light), 0.0f);
    float3 PhongD = shadowFactor * Diffuse * lightColor * diffuseColor.xyz;
    
    float3 PhongS = float3(0.0, 0.0, 0.0);
    if (l_Material.bHasSpecularHighlight && !isShadow)
    {
        float3 Ref1 = 2.0f * normalW * dot(normalW, light) - light;
        float3 View = normalize(g_CameraInfo.cameraEye - GetWorldPosition());
        float rh = 1.0f;
        if (0 != l_Material.bHasGlossiness)
        {
            rh = l_Material.Glossiness;
        }
        else if (0 != l_Material.bHasSmoothness)
        {
            rh = l_Material.Smoothness;
        }
        float Specular = pow(max(dot(Ref1, View), 0.0f), rh);
        if (Diffuse <= 0.0f)
            Specular = 0.0f;
        PhongS = Specular * l_Material.SpecularHighlight * l_Material.SpecularColor.xyz * lightColor;
    }
    
    float3 PhongA = 0.4f * diffuseColor.xyz;
    
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

/*float D_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;
    
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float3 F_Schlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float G_Smith(float3 N, float3 V, float3 L, float roughness)
{
    float k = pow(roughness + 1.0, 2.0) / 8.0;
    
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));

    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);

    return G_V * G_L;
}

float3 CookTorranceBRDF(float3 N, float3 V, float3 L, float3 albedo, float roughness, float metallic)
{
    float3 H = normalize(V + L);
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    float D = D_GGX(N, H, roughness);
    float3 F = F_Schlick(F0, saturate(dot(H, V)));
    float G = G_Smith(N, V, L, roughness);

    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.001);

    float3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL + 0.001;

    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    return (kD * albedo / PI + specular) * NdotL;
}*/

// Schlick의 근사에 의한 Fresnel
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX NDF (Normal Distribution Function)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

// Geometry function (Smith의 Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float3 CalculateCookTorranceLighting(float3 albedo, float3 N, float3 V, float3 L, float3 lightColor, float metallic, float roughness)
{
    float3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 방지용 offset
    float3 specular = nominator / denominator;

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * lightColor * NdotL;
}

float3 ImportanceSampleHemisphere(float2 xi, float3 normal)
{
    // xi는 0~1 사이의 난수 (각 샘플마다 다르게 줘야 함)

    float phi = 2.0f * PI * xi.x;
    float cosTheta = sqrt(1.0f - xi.y);
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // 로컬 방향
    float3 localDir = float3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    // 로컬 -> 월드 변환 (Tangent Space to World)
    float3 up = abs(normal.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);

    float3 worldDir = localDir.x * tangent + localDir.y * bitangent + localDir.z * normal;

    return normalize(worldDir);
}

uint ReverseBits(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
    return bits;
}

float2 Hammersley(uint i, uint N)
{
    return float2((float) i / (float) N, ReverseBits(i) * 2.3283064365386963e-10); // 2^-32
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
    if (0 != l_Mesh.bHasTex0)
        GetTex0FromBuffer(uvs, idx);
    
    float2 bary = float2(attrib.barycentrics.x, attrib.barycentrics.y);
    float2 texCoord = GetInterpolationHitFloat2(uvs, bary);

    float AlphaValue;
        
    if (l_Material.bHasAlbedoMap != 0)
    {
        AlphaValue = l_AlbedoMap.SampleLevel(g_Sampler, texCoord, 0).a;
    }
    else if (l_Material.bHasAlbedoColor != 0)
        AlphaValue = l_Material.AlbedoColor.a;
    else
        AlphaValue = 1.0f;
    if (AlphaValue <= 0.01)
        IgnoreHit();
}

[shader("closesthit")]
void RadianceClosestHit(inout RadiancePayload payload, in BuiltInTriangleIntersectionAttributes attrib)
{
    float2 uvs[3] = { float2(0.0, 0.0), float2(0.0, 0.0), float2(0.0, 0.0) };
    float3 normals[3] = { float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0) };
    float3 tangent[3] = { float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0) };
    float3 bitangent[3] = { float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0) };
    uint index[3];
    uint idx;
    idx = PrimitiveIndex() * 3;

    if (0 != l_Mesh.bHasTex0)
        GetTex0FromBuffer(uvs, idx);
    if (0 != l_Mesh.bHasNormals)
        GetNormalFromBuffer(normals, idx);
    if (0 != l_Mesh.bHasTangenrs)
        GetTangentFromBuffer(tangent, idx);
    if (0 != l_Mesh.bHasBiTangents)
        GetBiTangentFromBuffer(bitangent, idx);
    
    float2 bary = float2(attrib.barycentrics.x, attrib.barycentrics.y);
    float2 texCoord = GetInterpolationHitFloat2(uvs, bary);

    float4 albedoMapColor = float4(0.0, 0.0, 0.0, 1.0);
    float4 albedoColor = float4(1.0, 1.0, 1.0, 1.0);
    float4 objectColor;
    
    if (0 != l_Material.bHasAlbedoColor)
        albedoColor = l_Material.AlbedoColor;
        
    if (l_Material.bHasAlbedoMap != 0)
    {
        albedoMapColor = l_AlbedoMap.SampleLevel(g_Sampler, texCoord, 0);
        objectColor = saturate(albedoColor * albedoMapColor);
    }
    else
        objectColor = albedoColor;
    
    
    
    float3 lightingNormal;
    if ((0 != l_Material.bHasNormalMap) && (0 != g_CameraInfo.bNormalMapping))
        lightingNormal = GetHitNormalFromNormalMap(GetInterpolationHitFloat3(tangent, bary), GetInterpolationHitFloat3(bitangent, bary), GetInterpolationHitFloat3(normals, bary), texCoord);
    else
        lightingNormal = normalize(GetInterpolationHitFloat3(normals, bary));

    float3 normalW = normalize(mul(lightingNormal, (float3x3) ObjectToWorld4x3()));
    
    RayDesc shadowray;
    shadowray.Origin = GetWorldPosition() + normalW * 0.001f;
    shadowray.Direction = normalize(float3(1.0, 1.0, 1.0));
    shadowray.TMin = 0.001;
    shadowray.TMax = 1000;
    
    bool bShadow = CheckTheShadow(shadowray, payload.RayDepth + 1);
    
    float3 viewDir = normalize(g_CameraInfo.cameraEye - GetWorldPosition());
    float3 lightDir = normalize(float3(1.0, 1.0, 1.0)); // 실제 조명 방향으로 변경 가능
    float3 lightColor = float3(4.0, 4.0, 4.0);

    float roughness = 1.0 - l_Material.Smoothness;
    float metallic = l_Material.Metallic;

    float3 color = CalculateCookTorranceLighting(objectColor.rgb, normalW, viewDir, lightDir, lightColor, metallic, roughness);

// 그림자 적용
    if (bShadow)
        color *= 0.1; // 그림자 강도 조절

    float ambientStrength = 0.1 + 0.2 * pow(saturate(dot(normalW, float3(0, 1, 0))), 1.5);
    float3 ambientColor = float3(0.4, 0.5, 0.7);
    color += objectColor.rgb * ambientColor * ambientStrength;
    
    payload.RayColor = float4(color, 1.0);
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attrib)
{
    payload.bShadow = true;
}