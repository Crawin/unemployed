
/*
enum ROOT_SIGNATURE_IDX
{
	DESCRIPTOR_HEAP = 0,
	DESCRIPTOR_IDX_CBV,
	CAMERA_DATA_CBV,
	SHADER_DATAS_CBV,
	ROOT_SIGNATURE_IDX_MAX
};
*/

Texture2D Tex2DList[] : register(t0, space0);
TextureCube TexCubeList[] : register(t0, space1);
ByteAddressBuffer RawDataList[] : register(t0, space2);
Buffer<uint> BufferUintList[] : register(t0, space3);
Buffer<float> BufferFloatList[] : register(t0, space3);

SamplerState samplerWarp : register(s0);
SamplerState samplerClamp : register(s1);


struct VSOUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

VSOUT vs(uint instancID : SV_VertexID)
{
	VSOUT op;
	
	op.pos = float4(0.0, 0.5, 0.5, 1.0);
	op.color = float4(1.0, 0.0, 0.0, 1.0);

	if (instancID == 0)
	{
		op.pos = float4(0.0, 0.5, 0.5, 1.0);
		op.color = float4(1.0, 0.0, 0.0, 1.0);
	}
	else if (instancID == 1)
	{
		op.pos = float4(0.5, -0.5, 0.5, 1.0);
		op.color = float4(0.0, 1.0, 0.0, 1.0);
	}
	else if (instancID == 2)
	{
		op.pos = float4(-0.5, -0.5, 0.5, 1.0);
		op.color = float4(0.0, 0.0, 1.0, 1.0);
	}

	return op;
}

float4 ps(VSOUT input) : SV_TARGET
{
	return input.color;
}
