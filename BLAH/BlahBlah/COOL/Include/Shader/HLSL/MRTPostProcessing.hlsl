
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	
	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return output; 
}

cbuffer PostMaterial : register(b0)
{
	int albedoIdx;
	int roughIdx;
	int metalIdx;
	int specIdx;
	int normalIdx;
	int positionIdx;
	int lightSize;
	int lightIdx;
};

float4 Lighting(float4 albedo, float4 roughness, float4 metalic, float4 specular, float4 worldNormal, float4 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float3 viewDir = viewMatrix[2].xyz;
	float4 result = float4(0.0f, 0.0f, 0.0f, 1.0f);

	for(int i = 0; i < lightSize; ++i) {
		lights[i];
		// if on light (light map)
		// ambient
		float3 ambient = lights[i].m_LightColor.rgb * 0.2f * albedo.rgb;

		// diffuse
		float3 diff = max(dot(worldNormal.rgb, lights[i].m_Direction), float3(0.0f, 0.0f, 0.0f));
		float3 diffuse = lights[i].m_LightColor.rgb * diff * 0.7f * albedo.rgb;

		float3 refl = reflect(-lights[i].m_Direction, worldNormal.rgb);
		float specInt = pow(max(dot(viewDir, refl), 0.0f), 32.0f);
		float3 specularres = lights[i].m_LightColor.rgb * specInt * 0.1f * albedo.rgb;

		result += float4(ambient + diffuse + specularres, 0.0f);
	}

	return result;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
	// sample all
	float4 albedoColor = float4(Tex2DList[albedoIdx].Sample(samplerWarp, input.uv));
	float4 roughness = float4(Tex2DList[roughIdx].Sample(samplerWarp, input.uv));
	float4 metalic = float4(Tex2DList[metalIdx].Sample(samplerWarp, input.uv));
	float4 specular = float4(Tex2DList[specIdx].Sample(samplerWarp, input.uv));
	float4 normalW = float4(Tex2DList[normalIdx].Sample(samplerWarp, input.uv));
	//normalW *= 2;
	//normalW -= 1;
	float4 positionW = float4(Tex2DList[positionIdx].Sample(samplerWarp, input.uv));
	
	return Lighting(albedoColor, roughness, metalic, specular, normalW, positionW);

	// do something for post processing
	
	// ambient
	//float3 ambient = lightColor * 0.2f * albedoColor.rgb;
	
	//// diffuse
	//float3 diff = max(dot(normalW.rgb, lightDir), float3(0.0f, 0.0f, 0.0f));
	//float3 diffuse = lightColor * diff * 0.7f * albedoColor.rgb;
	
	//// specular
	//float3 viewDir = viewMatrix[2].xyz;
	//float3 refl = reflect(-lightDir, normalW.rgb);
	//float specInt = pow(max(dot(viewDir, refl), 0.0f), 32.0f);
	//float3 specularres = lightColor * specInt * 0.1f * albedoColor.rgb;
	
	//i.uv = tempUV.yx;
	//return float4(i.tangentW, 1.0f);
	//float3 result = ambient + diffuse + specularres;

	//return float4(result, 1.0f);
}
