
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

float4 ps(VS_OUTPUT input) : SV_Target
{
	// hardcoded
	float3 lightDir = normalize(float3(1.0f, 1.0f, -1.0f));
	float3 lightColor = float3(1.0f, 1.0f, 1.0f);
	

	// sample all
	float4 albedoColor = float4(Tex2DList[matIdx0.x].Sample(samplerWarp, input.uv));
	float4 roughness = float4(Tex2DList[matIdx0.y].Sample(samplerWarp, input.uv));
	float4 metalic = float4(Tex2DList[matIdx0.z].Sample(samplerWarp, input.uv));
	float4 specular = float4(Tex2DList[matIdx0.w].Sample(samplerWarp, input.uv));
	float4 normalW = float4(Tex2DList[matIdx1.x].Sample(samplerWarp, input.uv));
	//normalW *= 2;
	//normalW -= 1;
	float4 positionW = float4(Tex2DList[matIdx0.y].Sample(samplerWarp, input.uv));
	
	// do something for post processing
	
	// ambient
	float3 ambient = lightColor * 0.2f * albedoColor.rgb;
	
	// diffuse
	float3 diff = max(dot(normalW.rgb, lightDir), float3(0.0f, 0.0f, 0.0f));
	float3 diffuse = lightColor * diff * 0.7f * albedoColor.rgb;
	
	// specular
	float3 viewDir = viewMatrix[2].xyz;
	float3 refl = reflect(-lightDir, normalW.rgb);
	float specInt = pow(max(dot(viewDir, refl), 0.0f), 32.0f);
	float3 specularres = lightColor * specInt * 0.1f * albedoColor.rgb;
	
	//i.uv = tempUV.yx;
	//return float4(i.tangentW, 1.0f);
	float3 result = ambient + diffuse + specularres;

	return float4(result, 1.0f);
}
