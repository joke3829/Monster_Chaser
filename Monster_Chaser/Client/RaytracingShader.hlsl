
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

// Global Root Signature ============================================
RaytracingAccelerationStructure g_Scene : register(t0, space0);


SamplerState g_Sampler : register(s0);
// ==================================================================

// Local Root Signature =============================================
ConstantBuffer<HasMaterial> l_Material : register(b0);

// ==================================================================
[shader("raygenshader")]
void Raygen()
{
    
}