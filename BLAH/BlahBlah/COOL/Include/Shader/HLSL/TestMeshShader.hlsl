
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
	// fbx파일을 유니티에서 뽑아와서 해둔 임시 코드
	i.uv.y = 1 - i.uv.y;
	
	//float3 lightPosition = float3(1000.0f, 1000.0f, 0.0f);
	float3 lightDir = normalize(float3(1.0f, 1.0f, -1.0f));
	float3 lightColor = float3(1.0f, 1.0f, 1.0f);
	
	
	
	int albedoIdx = matIdx0.x;
	float3 smpl = float4(Tex2DList[albedoIdx].Sample(samplerWarp, i.uv));
	
	// ambient
	float3 ambient = lightColor * 0.2f * smpl.rgb;
	
	// diffuse
	float3 diff = max(dot(i.normalW, lightDir), float3(0.0f, 0.0f, 0.0f));
	float3 diffuse = lightColor * diff * 0.7f * smpl.rgb;
	
	// specular
	float3 viewDir = cameraPosition;
	float3 refl = reflect(-lightDir, i.normalW);
	float specInt = pow(max(dot(viewDir, refl), 0.0f), 32.0f);
	float3 specular = lightColor * specInt * 0.1f * smpl.rgb;
	
	//i.uv = tempUV.yx;
	//return float4(i.tangentW, 1.0f);
	
	float3 result = ambient + diffuse; + specular;
	return float4(ambient + diffuse, 1.0f);
}
