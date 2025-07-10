
#define PARTICLE_EMITTER 0
#define PARTICLE_FLAME 1

struct ParticleVertex
{
    float3 position : POSITION;
    float3 direction : DIRECTION;
    float lifeTime : LIFETIME;
    uint particleType : TYPE;
};

struct BillBoardOutput1
{
    float3 position : POSITION;
};

struct BillBoardOutput2
{
    float2 uv : TEXCOORD;
};

struct BillBoardOutput3
{
    float4 color : COLOR;
};

struct CameraInfo
{
    matrix mtxViewProj;
    matrix mtxInverseViewProj;
    float3 cameraEye;
    float fElapsedTime;
    int bNormalMapping;
    int bReflection;
};


ConstantBuffer<CameraInfo> g_camera : register(b0, space0);



ParticleVertex VSMain(ParticleVertex input)
{
    return input;
}

[maxvertexcount(42)]
void GSOnePath(point ParticleVertex input[1], inout PointStream<ParticleVertex> outStream)
{
    ParticleVertex particle = input[0];
    particle.lifeTime -= g_camera.fElapsedTime;
    switch (particle.particleType)
    {
        case PARTICLE_EMITTER:{
                if (particle.lifeTime <= 0.0f)
                {
                    particle.lifeTime = 0.5f;
                    outStream.Append(particle);
                
                    ParticleVertex output;
                    output.direction = float3(0.0, 3.0, 0.0);
                    output.lifeTime = 4.0f;
                    output.particleType = PARTICLE_FLAME;
                    output.position = particle.position;
                    outStream.Append(output);
                }
                else
                {
                    outStream.Append(particle);
                }
            }
            break;
        case PARTICLE_FLAME:{
                if (particle.lifeTime > 0.0f)
                {
                    particle.position += (particle.direction * g_camera.fElapsedTime);
                    outStream.Append(particle);
                }
            }
            break;
    }
}

[maxvertexcount(252)]
void GSTwoPath(point ParticleVertex input[1], inout PointStream<BillBoardOutput1> outStream1, 
inout PointStream<BillBoardOutput2> outStream2, inout PointStream<BillBoardOutput3> outStream3)
{
    switch (input[0].particleType)
    {
        case PARTICLE_EMITTER:{
                BillBoardOutput1 output1;
                BillBoardOutput2 output2;
                BillBoardOutput3 output3;
                output1.position = float3(0.0, 0.0, 0.0);
                output2.uv = float2(0.0, 0.0);
                output3.color = float4(0.0, 0.0, 0.0, 1.0);
                for (int i = 0; i < 6; ++i)
                {
                    outStream1.Append(output1);
                    outStream2.Append(output2);
                    outStream3.Append(output3);
                }
            }
            break;
        case PARTICLE_FLAME:{
                float3 vUp = float3(0.0, 1.0, 0.0);
                float3 vLook = g_camera.cameraEye - input[0].position;
                //vLook.y = 0.0f;
                vLook = normalize(vLook);
                float3 vRight = cross(vUp, vLook);
                vUp = cross(vLook, vRight);
                float3 Vertices[4];
                float fHalf = 8.0f - (input[0].lifeTime * 2.0f);
                Vertices[0] = input[0].position + fHalf * vRight - fHalf * vUp;
                Vertices[1] = input[0].position + fHalf * vRight + fHalf * vUp;
                Vertices[2] = input[0].position - fHalf * vRight - fHalf * vUp;
                Vertices[3] = input[0].position - fHalf * vRight + fHalf * vUp;
                float2 UVs[4] = { float2(0.0, 1.0), float2(0.0, 0.0), float2(1.0, 1.0), float2(1.0, 0.0) };
                BillBoardOutput1 output1;
                BillBoardOutput2 output2;
                BillBoardOutput3 output3; 
                output3.color = float4(0.0, 0.0, 0.0, 1.0);
                for (int i = 0; i < 6; ++i)
                    outStream3.Append(output3);
            
                output1.position = Vertices[0];
                output2.uv = UVs[0];
                outStream1.Append(output1);
                outStream2.Append(output2);
                output1.position = Vertices[1];
                output2.uv = UVs[1];
                outStream1.Append(output1);
                outStream2.Append(output2);
                output1.position = Vertices[2];
                output2.uv = UVs[2];
                outStream1.Append(output1);
                outStream2.Append(output2);
            
                output1.position = Vertices[1];
                output2.uv = UVs[1];
                outStream1.Append(output1);
                outStream2.Append(output2);
                output1.position = Vertices[3];
                output2.uv = UVs[3];
                outStream1.Append(output1);
                outStream2.Append(output2);
                output1.position = Vertices[2];
                output2.uv = UVs[2];
                outStream1.Append(output1);
                outStream2.Append(output2);
            }
            break;
    }
}