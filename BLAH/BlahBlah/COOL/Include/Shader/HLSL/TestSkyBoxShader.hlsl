

#include "ShaderBaseParameters.hlsl"

cbuffer t
{
	float4 tttt;
};

struct VSOUT
{
	float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOUT vs(uint instancID : SV_VertexID)
{
	
	VSOUT op;
	
	op.pos = float4(0.0, 0.8, 0.99, 1.0);
    op.uv = float2(0, 0);
	if (instancID == 0)
	{
		op.pos = float4(-1.0, 1.0, 0.99, 1.0);
		op.uv = float2(0, 0);
    }
	else if (instancID == 1)
	{
		op.pos = float4(1.0, 1.0, 0.99, 1.0);
		op.uv = float2(1, 0);
    }
	else if (instancID == 2)
	{
		op.pos = float4(1.0, -1.0, 0.99, 1.0);
		op.uv = float2(1, 1);
    }
	
	else if (instancID == 3)
	{
		op.pos = float4(-1.0, 1.0, 0.99, 1.0);
		op.uv = float2(0, 0);
    }
	else if (instancID == 4)
	{
		op.pos = float4(1.0, -1.0, 0.99, 1.0);
		op.uv = float2(1, 1);
    }
	else if (instancID == 5)
	{
		op.pos = float4(-1.0, -1.0, 0.99, 1.0);
		op.uv = float2(0, 1);
    }
	
	return op;
}

float4 ps(VSOUT input) : SV_TARGET
{
	int albedoIdx = materialIndex[ALBEDO];

	// 임시라서 텍스쳐큐브는 안쓰고 그냥 투디 이미지만 그림
	
	
    //return float4(uv, input.pos.z, 1);
	
    return Tex2DList[albedoIdx].Sample(samplerClamp, input.uv);
	
	//return TexCubeList[texCubeIdx].Sample(samplerClamp, input.pos.xyz);
}
