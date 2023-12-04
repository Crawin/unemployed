
#include "ShaderBaseParameters.hlsl"


struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
};

VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT o;
	
	o.positionW = (float3) mul(float4(i.position, 1.0f), localMatrix);
	o.normalW = normalize(mul(i.normal, (float3x3) localMatrix));
	o.tangentW = (float3) mul(float4(i.tangent, 1.0f), localMatrix);
	o.position = mul(mul(float4(o.positionW, 1.0f), viewMatrix), projectionMatrix);
	
	o.uv = i.uv;
	
	return o;
}

float4 ps(VS_OUTPUT i) : SV_Target
{
	int albedoIdx = materialIndex[ALBEDO];
	
	i.uv.y = 1 - i.uv.y;
	//i.uv = tempUV.yx;
	//return float4(i.normalW, 1.0f);
	return Tex2DList[albedoIdx].Sample(samplerWarp, i.uv);
}
