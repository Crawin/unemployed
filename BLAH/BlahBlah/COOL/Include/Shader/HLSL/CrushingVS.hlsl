
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

struct CrushData{
	float2 m_CrushPosition;
	float m_Power;
	float m_Distance;
};

cbuffer Material : register(b0)
{
	CrushData shaderdatapadding[2];
	CrushData g_CrushData[2];
};
float3 RotateVectorOnZ(float3 v, float theta)
{
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);

	float3 rotatedVector;
	rotatedVector.x = cosTheta * v.x - sinTheta * v.y;
	rotatedVector.y = sinTheta * v.x + cosTheta * v.y;
	rotatedVector.z = v.z;

	return rotatedVector;
}

float3 RotateVectorOnX(float3 v, float theta)
{
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);

	float3 rotatedVector;
	rotatedVector.x = v.x;
	rotatedVector.y = cosTheta * v.y - sinTheta * v.z;
	rotatedVector.z = sinTheta * v.y + cosTheta * v.z;

	return rotatedVector;
}

VS_OUTPUT vs(VS_INPUT input)
{
	VS_OUTPUT o;
	
	float3 newPosition = input.position;

	float3 curNormal = input.normal;

	[unroll(2)]
	for(int i = 0; i < 2; ++i) {
		float2 hitPos = g_CrushData[i].m_CrushPosition;
		float dist = distance(input.position.xy, hitPos);

		float maxDist = g_CrushData[i].m_Distance;
		float crushEffect = saturate(1.0f - dist / maxDist);

		float height = g_CrushData[i].m_Power;
		float3 addPos = float3(0, 0, -1) * height * crushEffect;
		newPosition += addPos;

		float xFactor =  clamp((hitPos.x - input.position.x) / maxDist, -1.0f, 1.0f) * 2.0f;
		float yFactor =  clamp((hitPos.y - input.position.y) / maxDist, -1.0f, 1.0f) * 2.0f;

		float3 newNormal = normalize((RotateVectorOnZ(input.normal, xFactor) + RotateVectorOnX(input.normal, -yFactor)));
		curNormal = lerp(curNormal, newNormal, ceil(crushEffect));
	}
	o.positionW = (float3) mul(float4(newPosition, 1.0f), localMatrix);
	o.normalW = normalize(mul(curNormal, (float3x3) localMatrix));
	o.tangentW = normalize(mul(input.tangent.xyz, (float3x3) localMatrix));
	o.position = mul(mul(float4(o.positionW, 1.0f), viewMatrix), projectionMatrix);
	
	o.uv = input.uv;
	
	return o;
}