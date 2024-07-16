
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


cbuffer CrushingMaterials : register(b2)
{
	float2 toCrushPos_01;
	float dist_01;
	float mult_01;
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

VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT o;
	
	float3 newPosition = i.position;


	float2 temp = float2(-50.0f, 105.0f);
	float dist = distance(i.position.xy, temp);

	float maxDist = 15.0f;
	float crushEffect = saturate(1.0f - dist / maxDist);

	float height = 30.0f;
	float3 addPos = float3(0, 0, -1) * height * crushEffect;
	newPosition += addPos;
	

	float xFactor =  clamp((temp.x - i.position.x) / maxDist, -1.0f, 1.0f) * 2.0f;
	float yFactor =  clamp((temp.y - i.position.y) / maxDist, -1.0f, 1.0f) * 2.0f;
	
	float3 newNormal = normalize((RotateVectorOnZ(i.normal, xFactor) + RotateVectorOnX(i.normal, -yFactor)));
	newNormal = lerp(i.normal, newNormal, ceil(crushEffect));

	o.positionW = (float3) mul(float4(newPosition, 1.0f), localMatrix);
	o.normalW = normalize(mul(newNormal, (float3x3) localMatrix));
	o.tangentW = normalize(mul(i.tangent.xyz, (float3x3) localMatrix));
	o.position = mul(mul(float4(o.positionW, 1.0f), viewMatrix), projectionMatrix);
	
	o.uv = i.uv;
	
	return o;
}