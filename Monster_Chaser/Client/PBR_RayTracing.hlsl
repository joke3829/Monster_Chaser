
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

#define MAX_RAY_DEPTH               4
#define RADIANCE_OFFSET             0
#define RADIANCE_MISS_OFFSET        0
#define SHADOW_OFFSET               1
#define SHADOW_MISS_OFFSET          1
#define GEOMETRY_STRIDE             2           // num RayType

#define PI                          3.14159265358979323846264

#define SHADER_TYPE_METALLIC        0
#define SHADER_TYPE_SPECULAR        1
#define SHADER_TYPE_METALLIC_MAP    2
#define SHADER_TYPE_SPECULAR_MAP    3
#define SHADER_TYPE_UNKNOWN         4

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
    if (currentRayDepth + 1 > MAX_RAY_DEPTH)
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


float3 GetFresnelusingSchlick(in float3 r0, in float VdotH)
{
    //float3 f0 = pow((1 - 0.3) / (1 + 0.3), 2);
    return r0 + (1 - r0) * pow((1 - VdotH), 5);
}

float D_GGX(in float roughness, in float NdotH)
{
    //if (NdotH <= 0.0f)
        //return 0.0f;
    float a2 = pow(roughness, 4);
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

float3 SmithJointApprox(in float roughness, in float NdotV, in float NdotL)
{
    float r = roughness + 1;
    float k = pow(r, 2) / 8;
    //float v1 = NdotL * (NdotV * (1 - roughness) + roughness);
    //float v2 = NdotV * (NdotL * (1 - roughness) + roughness);
    //return 0.5 * rcp(v1 + v2);
    float g1L = NdotL / (NdotL * (1 - k) + k);
    float g1V = NdotV / (NdotV * (1 - k) + k);
    return g1L * g1V;

}

float4 CalculateLighting(inout RadiancePayload payload, in float3 N, uint ShaderType = 0, float2 uv = float2(0.0, 0.0))
{
    // 조명 개수 만큼
    float3 V = normalize(-WorldRayDirection());
    float NdotV = saturate(dot(N, V));
    float roughness = 0.0f;
    
    float3 albedoColor = float3(1.0, 1.0, 1.0);
    if(0!= l_Material.bHasAlbedoColor) 
        albedoColor = l_Material.AlbedoColor.xyz;
    if(0 != l_Material.bHasAlbedoMap)
        albedoColor *= l_AlbedoMap.SampleLevel(g_Sampler, uv, 0).xyz;
    
    float3 finalColor = float3(0.0, 0.0, 0.0);
    switch (ShaderType)
    {
        case SHADER_TYPE_SPECULAR_MAP:{
                float3 resultColor = float3(0.0, 0.0, 0.0);
                float4 smSample = l_SpecularMap.SampleLevel(g_Sampler, uv, 0);
                roughness = smSample.a;
                for (int i = 0; i < 1; ++i)
                {
                    float3 L = normalize(float3(1.0, 3.0, 1.0));
                    float3 lightColor = float3(1.0, 1.0, 1.0);
                
                    float3 H = normalize(V + L);
                    float NdotH = saturate(dot(N, H));
                    float NdotL = saturate(dot(N, L));
                                
                    float3 F = GetFresnelusingSchlick(smSample.rgb, NdotV);
                    float3 D = D_GGX(roughness, NdotH);
                    float3 G = SmithJointApprox(roughness, NdotV, NdotL);
                    //float G = GeoS(NdotH, NdotV, NdotL, VdotH);
                    
                    float3 Specular = (F * G * D) / (4 * NdotL * NdotV);
                    
                    //Specular = float3(D, D, D);
                    //Specular = F;
                    //resultColor += Specular;
                
                    float3 diffuseTerm = NdotL * albedoColor.rgb * lightColor;
                    resultColor += NdotL * (albedoColor.rgb + (1 - albedoColor.rgb) * Specular);
                    //resultColor = Specular + (float3(0.6, 0.6, 0.6) * 0.4);
                }
                finalColor = resultColor;
                break;
            }
        case SHADER_TYPE_SPECULAR:{
                if (0 != l_Material.bHasGlossiness) roughness = max(1.0f - l_Material.Glossiness, 0.001f);
                else if (0 != l_Material.bHasSmoothness) roughness = max(1.0f - l_Material.Smoothness, 0.001f);
                float3 resultColor = float3(0.0, 0.0, 0.0);
                roughness = 1.0f;
                for (int i = 0; i < 1; ++i)
                {
                    float3 L = normalize(float3(1.0, 1.0, 1.0));
                    float3 lightColor = float3(1.0, 1.0, 1.0);
                
                    float3 H = normalize(V + L);
                    float NdotH = saturate(dot(N, H));
                    float NdotL = saturate(dot(N, L));
                                
                    float3 F = GetFresnelusingSchlick(l_Material.SpecularColor.rgb, NdotV);
                    float3 D = D_GGX(roughness, NdotH);
                    float3 G = SmithJointApprox(roughness, NdotV, NdotL);
                    //float G = GeoS(NdotH, NdotV, NdotL, VdotH);
                    
                    float3 Specular = (F * G * D) / (4 * NdotL * NdotV);
                    
                    //Specular = float3(D, D, D);
                    //Specular = F;
                    //resultColor += Specular;
                
                    float3 diffuseTerm = NdotL * albedoColor.rgb * lightColor;
                    resultColor += NdotL * (albedoColor.rgb + (1 - albedoColor.rgb) * Specular);
                    //resultColor = F;
                }
                finalColor = resultColor;
                break;
            }
        case SHADER_TYPE_METALLIC_MAP:{
                float3 resultColor = float3(0.0, 0.0, 0.0);
                float4 metallicSample = l_MetallicMap.SampleLevel(g_Sampler, uv, 0);
                roughness = metallicSample.a;
                for (int i = 0; i < 1; ++i)
                {
                    float3 L = normalize(float3(1.0, 3.0, 1.0));
                    float3 lightColor = float3(1.0, 1.0, 1.0);
                
                    float3 H = normalize(V + L);
                    float NdotH = saturate(dot(N, H));
                    float NdotL = saturate(dot(N, L));
                                
                    float3 F = GetFresnelusingSchlick(metallicSample.rrr, NdotV);
                    float3 D = D_GGX(roughness, NdotH);
                    float3 G = SmithJointApprox(roughness, NdotV, NdotL);
                    //float G = GeoS(NdotH, NdotV, NdotL, VdotH);
                    
                    float3 Specular = (F * G * D) / (4 * NdotL * NdotV);
                    
                    //Specular = float3(D, D, D);
                    //Specular = F;
                    //resultColor += Specular;
                
                    float3 diffuseTerm = NdotL * albedoColor.rgb * lightColor;
                    resultColor += NdotL * (albedoColor.rgb + (1 - albedoColor.rgb) * Specular);
                    //resultColor = Specular + (float3(0.6, 0.6, 0.6) * 0.4);
                }
                finalColor = resultColor + (albedoColor.rgb * 0.5);
                break;
            }
        case SHADER_TYPE_METALLIC:{
                break;
            }
    }
    
    
    return float4(finalColor, 1.0);
}

float4 CalculatePhongModel(float4 diffuseColor, float3 normal, bool isShadow, in float2 uv = float2(0.0, 0.0))
{
    
    float3 normalW = normalize(mul(normal, (float3x3) ObjectToWorld4x3()));
    float shadowFactor = 1.0f; //isShadow ? 0.35f : 1.0f;
    
    float3 PhongE = float3(0.0, 0.0, 0.0);
    float3 emissiveColor = float3(0.0, 0.0, 0.0);
    float3 emissiveMapColor = float3(0.0, 0.0, 0.0);
    bool bEmission = false;
    if (0 != l_Material.bHasEmissionMap)
    {
        emissiveMapColor = l_EmissionMap.SampleLevel(g_Sampler, uv, 0).xyz;
        if (emissiveMapColor.x >= 0.02 || emissiveMapColor.y >= 0.02 || emissiveMapColor.z >= 0.02)
            bEmission = true;
    }
    if (0 != l_Material.bHasEmissiveColor)
    {
        /*if (l_Material.EmissiveColor.x >= 0.02 || l_Material.EmissiveColor.y >= 0.02 || l_Material.EmissiveColor.z >= 0.02)
        {
            bEmission = true;
            emissiveColor = l_Material.EmissiveColor;
        }*/
        emissiveColor = l_Material.EmissiveColor;
    }
    
    PhongE = emissiveColor * emissiveMapColor;
    
    //float3 lightColor = float3(0.8, 0.4, 0.2);
    float3 lightColor = float3(1.0, 1.0, 1.0);
    //float3 lightColor = float3(0.2, 0.2, 0.2);
    float3 light = normalize(float3(1.0, 1.0, 1.0));
    //float3 light = normalize(float3(0.0, 10.0, 0.0) - GetWorldPosition());
    
    float Diffuse = max(dot(normalW, light), 0.0f);
    // Half-Lambert
    //float Diffuse = dot(normalW, light) * 0.5 + 0.5;
    //Diffuse = pow(Diffuse, 3);
    if (isShadow && !bEmission && Diffuse > 0.0f)
        shadowFactor = 0.35f;
    
    float3 PhongD = shadowFactor * Diffuse * lightColor * diffuseColor.xyz;
    
    float3 PhongS = float3(0.0, 0.0, 0.0);
    if (l_Material.bHasSpecularHighlight && !isShadow)
    {
        //float3 Ref1 = 2.0f * normalW * dot(normalW, light) - light;
        float3 View = normalize(g_CameraInfo.cameraEye - GetWorldPosition());
        float3 halfV = normalize(View + light);
        float rh = 1.0f;
        if (0 != l_Material.bHasGlossiness)
        {
            rh = l_Material.Glossiness;
        }
        else if (0 != l_Material.bHasSmoothness)
        {
            rh = l_Material.Smoothness;
        }
        //float Specular = pow(max(dot(Ref1, View), 0.0f), 256.0);
        float Specular = pow(max(0.0f, dot(normalW, halfV)), 512);

        PhongS = Specular * l_Material.SpecularHighlight * l_Material.SpecularColor.xyz * lightColor;
    }
    
    float3 PhongA = 0.2f * diffuseColor.xyz;
    
    
    return saturate(float4(PhongD + PhongS + PhongA + PhongE, 1.0f));
}

bool CheckTheShadow(in RayDesc ray, uint currentRayDepth)
{
    if (currentRayDepth + 1 > MAX_RAY_DEPTH)
        return false;
    
    // 조명 개수에 따라 검사하도록 코드 예정
    
    ShadowPayload payload = { false };
    TraceRay(g_Scene,
    RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
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
    uint ShaderType = 0;
    if (0 != l_Material.bHasSpecularMap) ShaderType = SHADER_TYPE_SPECULAR_MAP;
    else if (0 != l_Material.bHasMetallicMap) ShaderType = SHADER_TYPE_METALLIC_MAP;
    else if (0 != l_Material.bHasSpecularColor) ShaderType = SHADER_TYPE_SPECULAR;
    else if (0 != l_Material.bHasMetallic) ShaderType = SHADER_TYPE_METALLIC;
    else ShaderType = SHADER_TYPE_UNKNOWN;

    float2 bary = float2(attrib.barycentrics.x, attrib.barycentrics.y);

    uint idx = PrimitiveIndex() * 3;
    float2 uvs[3];
    float3 normals[3];
    
    float2 texCoord0;
    float3 lightNormal;
    if (0 != l_Mesh.bHasTex0)
    {
        GetTex0FromBuffer(uvs, idx);
        texCoord0 = GetInterpolationHitFloat2(uvs, bary);
    }
    if (0 != l_Mesh.bHasNormals)
    {
        GetNormalFromBuffer(normals, idx);
        lightNormal = normalize(GetInterpolationHitFloat3(normals, bary));
    }
    
    if (0 != l_Material.bHasNormalMap && 0 != g_CameraInfo.bNormalMapping)
    {
        float3 tangents[3], bitangents[3];
        GetTangentFromBuffer(tangents, idx);
        GetBiTangentFromBuffer(bitangents, idx);
        float3 HitTangent = GetInterpolationHitFloat3(tangents, bary);
        float3 HitBiTangent = GetInterpolationHitFloat3(bitangents, bary);
        lightNormal = GetHitNormalFromNormalMap(HitTangent, HitBiTangent, lightNormal, texCoord0);
    }
    lightNormal = normalize(mul(lightNormal, (float3x3) ObjectToWorld4x3()));
    //float3 testColor = float3(lightNormal.x * 0.5 + 0.5, lightNormal.y * 0.5 + 0.5, lightNormal.z * 0.5 + 0.5);
    
    //payload.RayColor = float4(testColor, 1.0f);
    
    payload.RayColor = CalculateLighting(payload, lightNormal, ShaderType, texCoord0);
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attrib)
{
    payload.bShadow = true;
}