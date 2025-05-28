
struct ParicleVertex
{
    float3 position : POSITION;
    float3 direction : DIRECTION;
    float size : SIZE;
    float lifeTime : LIFETIME;
    uint particleType : TYPE;
};

struct BillBoardOutput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};