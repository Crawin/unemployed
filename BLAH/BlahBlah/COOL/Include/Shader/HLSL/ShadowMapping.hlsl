
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
};

VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT o;
	
	o.position = mul(mul(mul(float4(i.position, 1.0f), localMatrix), viewMatrix), projectionMatrix);
	
	return o;
}

float4 ps(VS_OUTPUT i) : SV_Target
{
	return float4(i.position.z, i.position.z, i.position.z, 1.0f);
}
