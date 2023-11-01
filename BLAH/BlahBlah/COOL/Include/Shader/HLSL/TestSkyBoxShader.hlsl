

#include "ShaderBaseParameters.hlsl"

cbuffer t
{
	float4 tttt;
};

struct VSOUT
{
	float4 pos : SV_POSITION;
};

VSOUT vs(uint instancID : SV_VertexID)
{
	
	VSOUT op;
	
	op.pos = float4(0.0, 0.8, 0.8, 1.0);

	if (instancID == 0)
	{
		op.pos = float4(0.0, 1.0, 0.8, 1.0);
	}
	else if (instancID == 1)
	{
		op.pos = float4(1.0, 1.0, 0.8, 1.0);
	}
	else if (instancID == 2)
	{
		op.pos = float4(1.0, -1.0, 0.8, 1.0);
	}
	
	else if (instancID == 3)
	{
		op.pos = float4(0.0, 1.0, 0.8, 1.0);
	}
	else if (instancID == 4)
	{
		op.pos = float4(1.0, -1.0, 0.8, 1.0);
	}
	else if (instancID == 5)
	{
		op.pos = float4(0.0, -1.0, 0.8, 1.0);
	}
	
	return op;
}

float4 ps(VSOUT input) : SV_TARGET
{
	int albedoIdx = materialIndex[ALBEDO];

	// 임시라서 텍스쳐큐브는 안쓰고 그냥 투디 이미지만 그림
	
	float2 uv = (input.pos.xy + 1) / 2;
	
	return float4((input.pos.z), 0, 0, 1);
	
	return Tex2DList[albedoIdx].Sample(samplerClamp, uv);
	
	//return TexCubeList[texCubeIdx].Sample(samplerClamp, input.pos.xyz);
}