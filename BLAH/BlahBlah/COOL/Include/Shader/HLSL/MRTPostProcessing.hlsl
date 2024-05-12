
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

float ShadowCalculate(float4 worldPos, float dotNormal, int camIdx, int mapIdx)
{
	StructuredBuffer<CameraData> shadowCams = ShadowCameraDataList[camIdx];
	CameraData cam = shadowCams[0];
	float4 fragPos = mul(mul(worldPos, cam.viewMatrix), cam.projectionMatrix);
	//float3 ndc = fragPos.xyz / fragPos.w * 0.5f + 0.5f;
	float3 ndc = fragPos.xyz / fragPos.w;
	ndc.xy *= 0.5f;
	ndc.xy += 0.5f;
	//ndc.z *= 0.5f;
	//ndc.z += 0.5f;
	ndc.y = 1 - ndc.y;
	float shadow = 0.0;

	float bias = max(0.00005 * (1.0 - dotNormal), 0.000005);

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			float depth = Tex2DList[mapIdx].Sample(samplerClamp, ndc.xy/*).r;// */+ (float2(i, j) / 8192.0f)).r;
			shadow += (depth + bias) < ndc.z ? 0.0 : 1.0;  
		}
	}

	return shadow / 9.0f;
}

float4 DirectionalLight(int idx, float3 viewDir, float4 albedo, float4 roughness, float4 metalic, float4 specular, float4 worldNormal, float4 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	//lights[i];
	// if on light (light map)
	// ambient
	float3 ambient = lights[idx].m_LightAmbient.rgb * 0.2f * albedo.rgb;

	// diffuse
	float3 lightDir = -lights[idx].m_Direction;

	float dotNormal = dot(worldNormal.rgb, lightDir);
	float3 diff = max(dotNormal, float3(0.0f, 0.0f, 0.0f));
	float3 diffuse = lights[idx].m_LightColor.rgb * diff * 0.7f * albedo.rgb;

	float3 refl = reflect(lightDir, worldNormal.rgb);
	float specInt = pow(max(dot(viewDir, refl), 0.0f), 32.0f);
	float3 specularres = lights[idx].m_LightColor.rgb * specInt * 0.1f * albedo.rgb;

	float shadow = 1;
	if (lights[idx].m_CameraIdx > 0)
		shadow = ShadowCalculate(worldPosition, dotNormal, lights[idx].m_CameraIdx, lights[idx].m_ShadowMapResults.x);

	shadow = shadow * worldPosition.w + (1 - worldPosition.w);

	return float4(ambient + shadow * (diffuse + specularres), 0.0f);;
}

float4 SpotLight(int idx, float3 viewDir, float4 albedo, float4 roughness, float4 metalic, float4 specular, float4 worldNormal, float4 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightToPos = worldPosition.xyz - lights[idx].m_Position;

	float distance = length(lightToPos);
	float distFactor = max(1 - distance / lights[idx].m_Distance, 0);

	float padding = 32.0f;
	float epsilon = 0.2f;
	float theta = dot(normalize(lightToPos), lights[idx].m_Direction);
	float maxCos = cos(lights[idx].m_Angle);
	float spotLightFactor = pow(clamp((theta - maxCos) / epsilon, 0.0f, 1.0f), padding);    

	result = DirectionalLight(idx, viewDir, albedo, roughness, metalic, specular, worldNormal, worldPosition) * spotLightFactor * distFactor;

	return result;
}

float4 PointLight(int idx, float3 viewDir, float4 albedo, float4 roughness, float4 metalic, float4 specular, float4 worldNormal, float4 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightToPos = worldPosition.xyz - lights[idx].m_Position;
	float distance = length(lightToPos);

	// distance factor
	float factor = max(1 - distance / lights[idx].m_Distance, 0);

	result = DirectionalLight(idx, viewDir, albedo, roughness, metalic, specular, worldNormal, worldPosition) * factor;

	return result;
}

float4 Lighting(float4 albedo, float4 roughness, float4 metalic, float4 specular, float4 worldNormal, float4 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float3 viewDir = viewMatrix[2].xyz;
	float4 result = float4(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < lightSize; ++i) {
		if (lights[i].m_LightType == 0)			result += DirectionalLight(i, viewDir, albedo, roughness, metalic, specular, worldNormal, worldPosition);
		else if (lights[i].m_LightType == 1)	result += SpotLight(i, viewDir, albedo, roughness, metalic, specular, worldNormal, worldPosition);
		else if (lights[i].m_LightType == 2)	result += PointLight(i, viewDir, albedo, roughness, metalic, specular, worldNormal, worldPosition);
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
